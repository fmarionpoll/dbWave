// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxcview.h>
#include <afxcontrolbars.h> // MFC support for ribbons and control bars
#include <afxole.h>         // MFC OLE classes
//#include <afxodlgs.h>       // MFC OLE dialog classes
#include <afxdisp.h>        // MFC Automation classes
#include <afxdao.h>			// MFC DAO database classes
#include <afx.h>
#include <afxdb.h>
#include <afxtempl.h>
#include <afxrich.h>		// MFC rich edit classes
#include <afxadv.h>

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>       // MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>         // MFC support for Windows Common Controls  
#endif // _AFX_NO_AFXCMN_SUPPORT


#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

// commons for dbWave...
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <process.h>
#include <afxdisp.h>

#include "StretchControls.h"		// stretch controls
#include "dbwave_constants.h"
#include "Acqparam.h"		// structures describing data acquisition parameters
#include "Taglines.h"		// tag structures
#include "datafile_X.h"		// data file base class
#include "wavebuf.h"		// data file buffer structure
#include "StimLevelArray.h"
#include "awavepar.h"		// view data options
#include "spikepar.h"		// spike parameters options
#include "dbMultiDocTemplate.h"
#pragma warning(disable : 4995)
#include "dbwave.h"
#include "resource.h"



#define SAFE_DELETE(ptr) if (ptr) { delete ptr; ptr = NULL; }
#define SAFE_DELETE_ARRAY(p) if(p) {delete [] p ;p = NULL ;}

#pragma warning(disable : 4995)
