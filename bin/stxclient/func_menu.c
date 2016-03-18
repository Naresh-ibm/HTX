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

/* @(#)55	1.4  src/htx/usr/lpp/htx/bin/stxclient/func_menu.c, eserv_gui, htxubuntu 5/24/04 17:05:38 */

#include "hxssup.h"

extern int COLS;      /* number of cols in CURSES screen   */
extern int LINES;      /* number of lines in CURSES screen  */
extern WINDOW *stdscr;      /* standard screen window       */

int  func_menu(void)
{
    char  input[32];    /* input string                      */
    //char  msg_text[MAX_TEXT_MSG];
    int    rc;
    static int col = 0;  /* column number                     */
    static int row = 0;  /* row number                        */

    rc = 0;
    for (;;) {
        /*
         * display fresh screen
         */
        clear();
        refresh();
        display_scn(stdscr, 0, 0, LINES, COLS, "func_scn", 1, &row, &col, 23, 81, 'n');

        strncpy(input, "", DIM(input));  /* clear input */
        get_string(stdscr, 19, 52, input, (size_t) (2), "123rRqQ", (tbool) TRUE);

        switch (input[0]) {
            case '1':  /* refresh */
                ART_device();
                break;

            case 'q':  /* refresh */
                return 0;
            default:
                beep();
                break;
        }    /* switch */
    }      /* for forever */

    return 0;
}        /* ART_device */