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

#include <htx_local.h>
#include <hxihtx64.h>
#include <stdlib.h>


/*
 * NAME: hxfmsg()
 *
 * FUNCTION: Sends a message to the HTX system via the hxfupdate() ERROR call.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This function is called by any Hardware Exerciser (HE) program and
 *      is included as part of the libhtx.a library.
 *
 * NOTES:
 *
 *      operation:
 *      ---------
 *
 *      set htx_data error number and severity fields
 *
 *      copy message string to htx_data text field
 *
 *      call hxfupdate() ERROR to send message to HTX system
 *
 *      return(hxfupdate return code)
 *
 *
 * RETURNS:
 *
 *      Return Codes:
 *      ----------------------------------------------------------------------
 *                   0 -- Normal exit; buffers compare OK.
 *       exit_code > 0 -- Problem in hxfupdate() ERROR call.
 *
 *
 */

int hxfmsg(struct htx_data *p, int err, enum sev_code  sev, char *text)
     /*
      * p -- pointer to the Hardware Exerciser's htx_data data structure
      * err -- error code
      * sev -- severity code
      * text -- pointer to message text
      */
{
  /*
   ***  Beginning of Executable Code  *****************************************
   */
	if(p == NULL){ /* to handle cases when the htx_data pointer is not updated in hxfupdate*/
		printf("%s",text);
		return 0;
	}
	else{
		p->error_code = err;
		p->severity_code = sev;

		(void) strncpy(p->msg_text, text, MAX_TEXT_MSG);
		if (p->msg_text[MAX_TEXT_MSG - 1] != '\0')
		p->msg_text[MAX_TEXT_MSG -1] = '\0';  /* string MUST end with null char  */

		return(hxfupdate(ERROR, p));
	}

} /* hxfmsg() */
