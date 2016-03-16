/* IBM_PROLOG_BEGIN_TAG */
/* 
 * Copyright 2003,2016 IBM International Business Machines Corp.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * 		 http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/* IBM_PROLOG_END_TAG */
/* @(#)57	1.8  src/htx/usr/lpp/htx/bin/htxd/htxd_option_method_run_mdt.c, htxd, htxfedora 8/23/15 23:34:35 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "htxd.h"
#include "htxd_ecg.h"
#include "htxd_define.h"
#include "htxd_profile.h"
#include "htxd_instance.h"
#include "htxd_util.h"
#include "htxd_dr.h"
#include "htxd_equaliser.h"
#include "htxd_trace.h"


extern volatile int htxd_ecg_shutdown_flag;
extern int init_syscfg();


/* start equaliser process */
int htxd_start_equaliser(void)
{

	pid_t equaliser_pid;
	char	temp_str[128];

	equaliser_pid = htxd_create_child_process();
	switch(equaliser_pid)
	{
	case 0:
		htxd_equaliser();
		exit(0);
		break;

	case -1:
		sprintf(temp_str, "Unable to fork for equaliser process.  errno = %d", errno);
		htxd_send_message(temp_str, errno, HTX_SYS_HARD_ERROR, HTX_SYS_MSG);
		break;

	default:
		printf("DEBUG: equaliser process started with pid <%d>\n", equaliser_pid);
		htxd_set_equaliser_pid(equaliser_pid);
		break;

	}

	return 0;
}



/* check for equaliser start */
int htxd_check_for_equaliser_start(htxd_ecg_manager *p_ecg_manager)
{
	int equaliser_start_flag = FALSE;
	
	if( (p_ecg_manager->current_loading_ecg_info->ecg_equaliser_info.enable_flag != 0) && (htxd_get_equaliser_pid() == 0) ) {
		equaliser_start_flag = TRUE;
	}

	return equaliser_start_flag;	
}



/* run setup script for the ECG */
void htxd_execute_run_setup_script(char *ecg_name)
{
	char command_string[512];

	sprintf(command_string, "%s %s 2> %s", RUN_SETUP_SCRIPT, ecg_name, RUN_SETUP_ERROR_OUTPUT);
	system(command_string);
}



/* hxsmsg : picks messages from HTX message queue and stores the messages in files */
int htxd_load_hxsmsg(htxd_profile *p_profile)
{
	int hxsmsg_pid;
	char hxsmsg_path[512];
	char auto_start_flag_string[8];


	hxsmsg_pid = htxd_create_child_process();
	switch(hxsmsg_pid)
	{
	case 0:
		strcpy(hxsmsg_path, HTX_PATH);
		strcat(hxsmsg_path, "/bin/hxsmsg");

		if(p_profile->auto_start == 0) {
			strcpy(auto_start_flag_string, "no");
		} else {
			strcpy(auto_start_flag_string, "yes");
		}
		
		execl(	hxsmsg_path, 
				"hxsmsg", 
				p_profile->max_htxerr_size, 
				p_profile->max_htxmsg_size, 
				p_profile->max_htxerr_save_size,  
				p_profile->max_htxmsg_save_size, 
				p_profile->htxerr_wrap, 
				p_profile->htxmsg_wrap, 
				p_profile->htxmsg_archive, 
				auto_start_flag_string,
				p_profile->stress_device,
				p_profile->stress_cycle,
				(char *) 0); 	
		printf("DEBUG: hxsmsg load failed <errno : %d> !!!\n", errno); fflush(stdout);
		exit(errno);

	case -1:
		return -1;
		break;

	default:
		htxd_set_htx_msg_pid(hxsmsg_pid);
		break;
	}

	htxd_reset_FD_close_on_exec_flag();
	
	return 0;
}



/* hxsstas : read exerciser info from shared memory and store it /tmp/htxstats file */
int htxd_load_hxstats(void)
{
	int hxstats_pid;
	char hxsstats_path[512];


	hxstats_pid = htxd_create_child_process();
	switch(hxstats_pid)
	{
	case 0:
		strcpy(hxsstats_path, HTX_PATH);
		strcat(hxsstats_path, "/bin/hxstats");

		execl(hxsstats_path, "hxstats", "/tmp/htxstats", "30", (char *) 0 );

		printf("DEBUG: hxstats load failed <errno : %d> !!!\n", errno); fflush(stdout);
		exit(errno);

	case -1:
		exit(-1);

	default:
		htxd_set_htx_stats_pid(hxstats_pid);
	}

	htxd_reset_FD_close_on_exec_flag();	

	return 0;
}



/* load exerciser : start exerciser process */
int htxd_load_exerciser(struct htxshm_HE *p_HE)
{

	int exerciser_pid;
	int temp_int;
	char exerciser_path[512];
	char exerciser_name[64];
	char device_name[64];
	char run_mode[64];
	char rule_path[64];
	int emc_mode;
	char trace_str[256];


	HTXD_FUNCTION_TRACE(FUN_ENTRY, "htxd_load_exerciser");
	sprintf(trace_str, "loading exerciser <%s>", p_HE->sdev_id);
	HTXD_TRACE(LOG_OFF, trace_str);
	exerciser_pid = htxd_create_child_process();
	switch(exerciser_pid)
	{
	case 0:
		setsid();
		
		temp_int = p_HE->priority;
		nice(temp_int);

		sleep(5);   /* let daemon update shared memory */
		
		strcpy(exerciser_name, p_HE->HE_name);

		if (strcmp(p_HE->HE_name, "hxemem64") == 0) {
			putenv("CORE_NOSHM=true");
		}

		if (strcmp(p_HE->HE_name, "hxepowermixer") == 0) {
			putenv("MEMORY_AFFINITY=MCM");
		}

		strcpy(exerciser_path, HTX_PATH);
		strcat(exerciser_path, "/bin/");
		strcat(exerciser_path, exerciser_name);

		strcpy(device_name, "/dev/");
		strcat(device_name, p_HE->sdev_id);

		emc_mode = htxd_get_emc_mode();
		if(emc_mode == 1) {
			strcpy(run_mode, "EMC");
		} else {
			strcpy(run_mode, "REG");
		}

		strcpy(rule_path, HTX_PATH);
		strcat(rule_path, "/rules/");
		if(emc_mode == 1) {
			strcat(rule_path, "emc/");
			strcat(rule_path, p_HE->emc_rules);
		} else {
			strcat(rule_path, "reg/");
			strcat(rule_path, p_HE->reg_rules);
		}
	
/* system("export EXTSHM=OFF"); */
unsetenv("EXTSHM");
		if ( (execl(exerciser_path, exerciser_name, device_name, run_mode, rule_path, (char *) 0) ) == -1) {
			sprintf(trace_str, "execl() failed  exerciser_path <%s> exerciser_name <%s> errno = <%d>\n", exerciser_path, exerciser_name, errno);
			htxd_send_message (trace_str, 0, HTX_SYS_SOFT_ERROR, HTX_SYS_MSG);	
			HTXD_TRACE(LOG_ON, trace_str);
			exit(-1);
		} 	

	case -1:
		sprintf(trace_str, "exerciser <%s> fork failed with error <%d>", p_HE->sdev_id, errno);
		HTXD_TRACE(LOG_ON, trace_str);
		return -1;

	default:
		p_HE->PID = exerciser_pid;
		htxd_update_exer_pid_in_exer_list(htxd_get_exer_table(), p_HE->sdev_id, exerciser_pid);
		sprintf(trace_str, "exerciser <%s> forked with PID <%d>", p_HE->sdev_id, p_HE->PID);
		HTXD_TRACE(LOG_OFF, trace_str);
		break;
	}
	htxd_reset_FD_close_on_exec_flag();

	HTXD_FUNCTION_TRACE(FUN_EXIT, "htxd_load_exerciser");
	return 0;
}



/* to start all exercisers */
int htxd_activate_all_ecg_devices(htxd_ecg_manager *this_ecg_manager)
{
	htxd_ecg_info *p_current_ecg_info;
	int number_of_exercisers_to_run;
	struct htxshm_HE *p_HE;
	int i;


	HTXD_FUNCTION_TRACE(FUN_ENTRY, "htxd_activate_all_ecg_devices");

	p_current_ecg_info = this_ecg_manager->current_loading_ecg_info;	
	/* number_of_exercisers_to_run = p_current_ecg_info->ecg_number_of_exercisers_to_run; */
	number_of_exercisers_to_run = p_current_ecg_info->ecg_shm_exerciser_entries;

	p_HE = (struct htxshm_HE *)(p_current_ecg_info->ecg_shm_addr.hdr_addr + 1); /* skipping shm header and point to first HE */

	for(i = 0; i < number_of_exercisers_to_run; i++) {
		htxd_load_exerciser(p_HE + i);	
	}	
	sleep(10);  /* let all exercisers to start */
	
	(p_current_ecg_info->ecg_shm_addr.hdr_addr)->started = 1;
	p_current_ecg_info->ecg_status = ECG_ACTIVE;
	this_ecg_manager->loaded_device_count += number_of_exercisers_to_run;
	
	HTXD_FUNCTION_TRACE(FUN_EXIT, "htxd_activate_all_ecg_devices");
	return 0;
}



/* run command main function */
int htxd_option_method_run_mdt(char **result)
{
	htxd *htxd_instance;
	htxd_ecg_info * p_ecg_info_list;
	htxd_ecg_info * p_ecg_info_to_run;
	char command_ecg_name[MAX_ECG_NAME_LENGTH];
	htxd_ecg_manager *ecg_manager;
	char trace_str[512];
	int return_code;


	HTXD_FUNCTION_TRACE(FUN_ENTRY, "htxd_option_method_run_mdt");
	/* htxd instance will be created only first time */
	htxd_instance = htxd_get_instance();
	strcpy(command_ecg_name, htxd_get_command_ecg_name());
	
	sprintf(trace_str, "ECG name from command = <%s>", command_ecg_name);
	HTXD_TRACE(LOG_OFF, trace_str);

	htxd_ecg_shutdown_flag = FALSE;

	ecg_manager = htxd_get_ecg_manager();

	if(command_ecg_name[0] == '\0') {
		strcpy(command_ecg_name, DEFAULT_ECG_NAME);
	}

	*result = malloc(1024);

	if(htxd_is_file_exist(command_ecg_name) == FALSE) {
		sprintf(*result, "specified mdt(%s) is invalid", command_ecg_name);
		HTXD_TRACE(LOG_OFF, "htxd_is_file_exist() could not find the ECG file");
		return 1;
	}

	if(ecg_manager == NULL) {
		HTXD_TRACE(LOG_OFF, "ecg manager is creating");
		ecg_manager = create_ecg_manager();		
		if(ecg_manager == NULL) {
			HTXD_TRACE(LOG_OFF, "e_ecg_manager() failed to create ecg manager");
			return 1;
		}
		htxd_instance->p_ecg_manager = ecg_manager;
	}


	/* start hxsmsg process if it is not already started */
	if(htxd_get_htx_msg_pid() == 0) {
		htxd_truncate_error_file();	
		htxd_load_hxsmsg(htxd_instance->p_profile);
		HTXD_TRACE(LOG_ON, "run started htxsmsg process");
		htxd_send_message ("System wakeup for responding", 0, HTX_SYS_INFO, HTX_SYS_MSG);
	}

	if(htxd_is_daemon_selected()  != TRUE) {
		p_ecg_info_list = htxd_get_ecg_info_list(htxd_instance->p_ecg_manager);
		while(p_ecg_info_list != NULL) {
			if( strcmp(command_ecg_name, p_ecg_info_list->ecg_name) == 0) {
				sprintf(*result, "specified ECG (%s) is already running", command_ecg_name);
				HTXD_TRACE(LOG_OFF, "specified ECG is already running");
				return 1;
			}
			p_ecg_info_list = p_ecg_info_list->ecg_info_next;
		}
		sprintf(trace_str, "start activating  MDT <%s>", command_ecg_name);
		htxd_send_message (trace_str, 0, HTX_SYS_INFO, HTX_SYS_MSG);
		HTXD_TRACE(LOG_ON, trace_str);


		/* execute run setup script for this MDT */
		htxd_execute_run_setup_script(command_ecg_name);

		/* allocating required IPCs  and intialize the value at shared memory*/
		HTXD_TRACE(LOG_OFF, "run initializing ecg_info");
		htxd_init_ecg_info(ecg_manager, command_ecg_name);
	} else {
		if(strcmp(command_ecg_name, ecg_manager->selected_ecg_name) != 0) {
			sprintf(*result, "Failed to run specified ecg/mdt (%s), another ecg/mdt (%s) is already selected", command_ecg_name, ecg_manager->selected_ecg_name);
			return 1;
		}
		ecg_manager->selected_ecg_name[0] = '\0';
	}

	/* initializing syscfg */
	if(htxd_is_init_syscfg() != TRUE) {
		return_code = init_syscfg();
		if (return_code != 0) {
			sprintf(*result, "Internal error: failed to initialize syscfg with error code <%d>", return_code);
			HTXD_TRACE(LOG_ON, *result);
			return return_code;
		}
		htxd_set_init_syscfg_flag(TRUE);
	}

	/* start all devices in the ecg */
	HTXD_TRACE(LOG_OFF, "run activating all devices under the ECG");
	htxd_activate_all_ecg_devices(ecg_manager);

	/* start hxstats process if it is not already started */
	if(htxd_get_htx_stats_pid() == 0) {
		htxd_load_hxstats();
		HTXD_TRACE(LOG_ON, "run started htxstats process");
	}

#ifdef __HTXD_DR__
	/* start DR child process */
	if(htxd_get_dr_child_pid() == 0) {
		htxd_start_DR_child();
		HTXD_TRACE(LOG_ON, "run started DR child process");
	}
#endif

#ifdef __HTX_LINUX__
	/* start hotplug monitor */
	if( htxd_is_hotplug_monitor_initialized() != TRUE) {
		htxd_start_hotplug_monitor(&(htxd_instance->p_hotplug_monitor_thread));
		HTXD_TRACE(LOG_ON, "run started hotplug monitor");
	}
#endif

	/* start equaliser process */
	if( htxd_check_for_equaliser_start(ecg_manager) == TRUE) {
		htxd_start_equaliser();
		HTXD_TRACE(LOG_ON, "run started equaliser process");
	}

	/* start hang monitor thread */
	if(htxd_is_hang_monitor_initialized() != TRUE) {
		htxd_start_hang_monitor(&(htxd_instance->p_hang_monitor_thread));
		HTXD_TRACE(LOG_ON, "run started hang monitor thread");
	}

	htxd_instance->run_state = HTXD_DAEMON_RUNNING;
	strcpy(ecg_manager->running_ecg_name, command_ecg_name);

	/* to DEBUG */
/*	htxd_display_ecg_info_list();	
	htxd_display_exer_table();
*/
	p_ecg_info_to_run = ecg_manager->current_loading_ecg_info;
	if(p_ecg_info_to_run->ecg_exerciser_entries == 0){
		sprintf(*result, "No device is present in ECG <%s>, could not start any device", command_ecg_name);
	} else if(p_ecg_info_to_run->ecg_shm_exerciser_entries == 0){
		sprintf(*result, "No device is started under ECG <%s>, since all the devices are already loaded under another ECG(s)", command_ecg_name);
	} else if (p_ecg_info_to_run->ecg_exerciser_entries != p_ecg_info_to_run->ecg_shm_exerciser_entries) {
		sprintf(*result, "ECG (%s) Activated\nWARNING: few devices are unable to start since they may be already loaded under another ECG(s)", command_ecg_name);
	} else {
		sprintf(*result, "ECG (%s) Activated.", command_ecg_name);
		sprintf(trace_str, "date +\"ECG (%s) was activated on %%x at %%X %%Z\" >>/tmp/htxd.start.stop.time", command_ecg_name);
		system(trace_str);
	}

	htxd_send_message (*result, 0, HTX_SYS_INFO, HTX_SYS_MSG);
	HTXD_TRACE(LOG_ON, *result);
	HTXD_FUNCTION_TRACE(FUN_EXIT, "htxd_option_method_run_mdt");
	return 0;
}




/* select mdt command main function */
int htxd_option_method_select_mdt(char **result)
{
	htxd *htxd_instance;
	htxd_ecg_info * p_ecg_info_list;
	htxd_ecg_info * p_ecg_info_to_select;
	char command_ecg_name[MAX_ECG_NAME_LENGTH];
	htxd_ecg_manager *ecg_manager;
	char trace_str[512];


	HTXD_FUNCTION_TRACE(FUN_ENTRY, "htxd_option_method_select_mdt");
	/* htxd instance will be created only first time */
	htxd_instance = htxd_get_instance();

	*result = malloc(1024);
	ecg_manager = htxd_get_ecg_manager();
	strcpy(command_ecg_name, htxd_get_command_ecg_name());

	if(htxd_instance->run_state == HTXD_DAEMON_SELECTED) {
		if(strcmp(command_ecg_name, ecg_manager->selected_ecg_name) == 0) {
			sprintf(*result, "Failed to select specified ecg/mdt(%s), same ecg/mdt is already selected", command_ecg_name);
		} else {
			sprintf(*result, "Failed to select specified ecg/mdt(%s), another ecg/mdt(%s) is already selected", command_ecg_name, ecg_manager->selected_ecg_name);
		}
		return 1;
	}
	
	if(htxd_instance->run_state == HTXD_DAEMON_RUNNING) {
		if(strcmp(command_ecg_name, ecg_manager->running_ecg_name) == 0) {
			sprintf(*result, "Failed to select specified ecg/mdt(%s), same ecg/mdt is being run", command_ecg_name);
		} else {
			sprintf(*result, "Failed to select specified ecg/mdt(%s), another ecg/mdt(%s) is being run", command_ecg_name, ecg_manager->running_ecg_name);
		}
		return 1;
	}
	
	sprintf(trace_str, "ECG name from command = <%s>", command_ecg_name);
	HTXD_TRACE(LOG_OFF, trace_str);


	if(command_ecg_name[0] == '\0') {
		strcpy(command_ecg_name, DEFAULT_ECG_NAME);
	}

	if(htxd_is_file_exist(command_ecg_name) == FALSE) {
		sprintf(*result, "specified mdt(%s) is invalid", command_ecg_name);
		HTXD_TRACE(LOG_OFF, "htxd_is_file_exist() could not find the ECG file");
		return 1;
	}

	if(ecg_manager == NULL) {
		HTXD_TRACE(LOG_OFF, "ecg manager is creating");
		ecg_manager = create_ecg_manager();		
		if(ecg_manager == NULL) {
			HTXD_TRACE(LOG_OFF, "e_ecg_manager() failed to create ecg manager");
			return 1;
		}
		htxd_instance->p_ecg_manager = ecg_manager;
		htxd_send_message ("System wakeup for responding", 0, HTX_SYS_INFO, HTX_SYS_MSG);
	}

	p_ecg_info_list = htxd_get_ecg_info_list(htxd_instance->p_ecg_manager);
	while(p_ecg_info_list != NULL) {
		if( strcmp(command_ecg_name, p_ecg_info_list->ecg_name) == 0) {
			sprintf(*result, "specified ECG (%s) is already running", command_ecg_name);
			HTXD_TRACE(LOG_OFF, "specified ECG is already running");
			return 1;
		}
		p_ecg_info_list = p_ecg_info_list->ecg_info_next;
	}

	/* execute run setup script for this MDT */
	htxd_execute_run_setup_script(command_ecg_name);

	/* allocating required IPCs  and intialize the value at shared memory*/
	HTXD_TRACE(LOG_OFF, "run initializing ecg_info");
	htxd_init_ecg_info(ecg_manager, command_ecg_name);

	/* start hxsmsg process if it is not already started */
	if(htxd_get_htx_msg_pid() == 0) {
		htxd_truncate_error_file();	
		htxd_load_hxsmsg(htxd_instance->p_profile);
		HTXD_TRACE(LOG_ON, "run started htxsmsg process");
	}


	htxd_instance->run_state = HTXD_DAEMON_SELECTED;
	strcpy(ecg_manager->selected_ecg_name, command_ecg_name);

	/* to DEBUG */
/*	htxd_display_ecg_info_list();	
	htxd_display_exer_table();
*/
	p_ecg_info_to_select = ecg_manager->current_loading_ecg_info;
	if(p_ecg_info_to_select->ecg_exerciser_entries == 0){
		sprintf(*result, "No device is present in ECG <%s>, could not start any device", command_ecg_name);
	} else if(p_ecg_info_to_select->ecg_shm_exerciser_entries == 0){
		sprintf(*result, "No device is started under ECG <%s>, since all the devices are already loaded under another ECG(s)", command_ecg_name);
	} else if (p_ecg_info_to_select->ecg_exerciser_entries != p_ecg_info_to_select->ecg_shm_exerciser_entries) {
		sprintf(*result, "ECG (%s) is slected\nWARNING: few devices are unable to start since they may be already loaded under another ECG(s)", command_ecg_name);
	} else {
		sprintf(*result, "ECG (%s) is selected", command_ecg_name);
		sprintf(trace_str, "date +\"ECG (%s) was selected on %%x at %%X %%Z\" >>/tmp/htxd.start.stop.time", command_ecg_name);
		system(trace_str);
	}

	htxd_send_message (*result, 0, HTX_SYS_INFO, HTX_SYS_MSG);
	HTXD_TRACE(LOG_ON, *result);
	HTXD_FUNCTION_TRACE(FUN_EXIT, "htxd_option_method_select_mdt");
	return 0;

}
