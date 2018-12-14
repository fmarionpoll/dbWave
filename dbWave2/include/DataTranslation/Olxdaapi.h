#if !defined(__OLAPI__)
#define __OLAPI__

/*-----------------------------------------------------------------------

 Copyright  (C)  1992.   Data  Translation,  Inc.,  100  Locke   Drive,
 Marlboro Massachusetts 01752-1192.

 All rights reserved.  This software is furnished to purchaser under  a
 license  for  use  on a single computer system and can be copied (with
 the inclusion of DTI's copyright notice) only for use in such  system,
 except  as  may  be otherwise provided in writing by Data Translation,
 Inc., 100 Locke Drive, Marlboro, Massachusetts 01752-1192.

 The information in this document is subject to change  without  notice
 and  should not be construed as a commitment by Data Translation, Inc.
 Data Translation, Inc.  assumes no responsibility for any errors  that
 may appear in this document.

 Data Translation cannot assume any responsibility for the use  of  any
 portion  of  this  software  on  any  equipment  not  supplied by Data
 Translation, Inc.

FILE NAME: oldaapi.h

ABSTRACT:

   This unit contains declarations for the DT Open Layers Data Acquisition
   Library.


10/13/92
---------------------------------------------------------------------------*/

#include "./include/DataTranslation/oltypes.h"
#include "./include/DataTranslation/olmem.h"
#include "./include/DataTranslation/olxdadefs.h"

#ifdef WIN32
#include <mmsystem.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Data Acquisition SubSystem

DECLARE_HANDLE (HDASS);
typedef HDASS FAR *LPHDASS;

DECLARE_HANDLE (HSSLIST);
typedef HSSLIST FAR *LPHSSLIST;

typedef HDRVR HDEV;
typedef HDEV FAR *LPHDEV;

// conversion macros for measure frequency

#define LongToFreq(l)    *(float*) &(l)
#define FreqToLong(f)    *(long*) &(f)

typedef struct DASSInfotag {
   HDEV   hdev;
   OLSS   Olss;
   UINT   uiElement;
} DASSINFO;

typedef DASSINFO FAR *LPDASSINFO;

#ifdef STRICT
typedef BOOL (CALLBACK *SSLISTPROC) (HSSLIST hSSList, HDASS hDass, LPARAM lParam);
#else
typedef FARPROC SSLISTPROC;
#endif


// parameters for olDaEnumSSCaps
#define OLx_ENUM_FILTERS       0
#define OLx_ENUM_RANGES        1
#define OLx_ENUM_GAINS         2
#define OLx_ENUM_RESOLUTIONS   3

// function prototypes

#ifndef _OLACI_        // if oldaaci module is not included

ECODE WINAPI olDaGetVersion(LPSTR lpszVersion,UINT uiMaxChars);
ECODE WINAPI olDaGetDriverVersion(HDEV hDev, LPSTR lpszVersion, UINT uiMaxChars);

#endif
LPSTR WINAPI olDaGetErrorString (ECODE eCode, LPSTR lpStr, UINT uiMaxSize);

ECODE WINAPI olDaSetChannelRange (HDASS hDass, UINT uiChan, DBL dMaxVoltage, DBL dMinVoltage);
ECODE WINAPI olDaGetChannelRange (HDASS hDass, UINT uiChan, LPDBL lpdMaxVoltage, LPDBL dMinVoltage);

ECODE WINAPI olDaGetSSList (LPHSSLIST lphSSList);
ECODE WINAPI olDaReleaseSSList (HSSLIST hSSList);
ECODE WINAPI olDaPutDassToSSList (HSSLIST hSSList, HDASS hDass);
ECODE WINAPI olDaSimultaneousPrestart (HSSLIST hSSList);
ECODE WINAPI olDaSimultaneousStart (HSSLIST hSSList);
ECODE WINAPI olDaEnumSSList (HSSLIST hSSList, SSLISTPROC lpfnSSListProc, LPARAM lParam);


#ifdef __cplusplus
}
#endif


#endif
