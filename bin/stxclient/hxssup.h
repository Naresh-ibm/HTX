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
/* @(#)40	1.3  src/htx/usr/lpp/htx/bin/stxclient/hxssup.h, eserv_gui, htxubuntu 7/10/03 12:40:59 */
#ifndef HXSSUP_H
#define HXSSUP_H

/*
 ******************************************************************************
 ***  hxssup.h -- "hxssup" HTX Supervisor Include File  ***********************
 ******************************************************************************
 */


//#include <curses.h>

#ifdef	__HTX_LINUX__
#include <sys/param.h>
#include <hxiipc.h>
#include <htx_local.h>
#include <hxihtx.h>
#include <hxiconv.h>

#else
#include <htx_local.h>
#include <hxiipc.h>
#include <hxihtx.h>
#include "hxiconv_gui.h"
#endif


/* undefine TRUE and FALSE to prevent lint errors -- redefined in cfgdb.h */
#undef TRUE
#undef FALSE

#ifdef	__HTX_LINUX__
#define	TRUE	1
#define	FALSE	0
#endif

#ifndef	__HTX_LINUX__
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#endif

#include "hxssupdef.h"
#include "scr_info.h"

#define htx_strcpy strcpy
#define htx_strncpy strncpy
#define htx_strlen strlen

#endif				/* HXSSUP_H */
