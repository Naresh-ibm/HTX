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


/* @(#)56	1.3  src/htx/usr/lpp/htx/bin/hxssup/config.h, htx_sup, htxubuntu 1/4/16 05:08:28 */

#define UTILIZATION_QUEUE_LENGTH     20
#define MAX_UTIL_SEQUENCE_LENGTH     10
#define FILENAME	"/usr/lpp/htx/htx_eq.cfg"
#define LOGFILE       "/tmp/eq_status"
#define LOGFILE_SAVE  "/tmp/eq_status_save"

#define UTIL_LEFT	0
#define UTIL_RIGHT	1
#define UTIL_RANDOM	2
#define UTIL_PATTERN	3

typedef char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

struct run_time_data {
    uint32 target_utilization;
    uint32 current_seq_step;
    uint8 current_step;
};
typedef struct run_time_data run_time_data;

struct thread_config_parameters {
    char        dev_name[16];
    int         lcpu;
    int         pcpu;
    uint32      util_pattern;
    uint32      utilization_pattern;
    uint16      pattern_length;
    uint16      sequence_length;
    uint16      utilization_sequence[MAX_UTIL_SEQUENCE_LENGTH];
};
typedef struct thread_config_parameters thread_config_params;

struct run_time_thread_config_structure {
    thread_config_params    th_config;
    run_time_data           data;
};
typedef struct run_time_thread_config_structure run_time_thread_config;

struct test_config_structure {
    uint32                      time_quantum;
    uint16                      num_tests_configured;
    uint32                      wof_test;
    uint32                      startup_time_delay;
    uint32                      log_duration;
    run_time_thread_config      *thread;
};
typedef struct test_config_structure test_config_struct;


