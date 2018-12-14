#if !defined(__OLOLDSP__)
#define __OLOLDSP__

/*-----------------------------------------------------------------------

 Copyright  (C)  1992-1996.   Data  Translation,  Inc.,  100  Locke   Drive,
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

FILE NAME: oldsp32.h

ABSTRACT:

   This unit contains declarations for the DT Open Layers DSP
   Library.


9/12/96
---------------------------------------------------------------------------*/
                
#include "./include/olmem.h"
#include "./include/olerrors.h"
                
//string resources
//note all error strings must be less then UNKNOWN_ERROR_STRING,
//other string resources (if any are ever added), must be > UNKNOWN_ERROR_STRING
#define UNKNOWN_ERROR_STRING   350

// error codes (and error strings)
#define OLDSP_BASE_ERROR 300
#define OLDSP_CANT_ALLOC_MEMORY               (OLDSP_BASE_ERROR + 0)
#define OLDSP_FFT_SIZE_TOO_LARGE              (OLDSP_BASE_ERROR + 1)
#define OLDSP_BAD_INPUT_DATA_WIDTH            (OLDSP_BASE_ERROR + 2)
#define OLDSP_BAD_FFT_SIZE                    (OLDSP_BASE_ERROR + 3)
#define OLDSP_NOT_ENOUGH_VALID_INPUT_DATA     (OLDSP_BASE_ERROR + 4)
#define OLDSP_WINDOW_SIZE_TOO_LARGE           (OLDSP_BASE_ERROR + 5)
#define OLDSP_BAD_DATA_TYPE                   (OLDSP_BASE_ERROR + 6)
#define OLDSP_BAD_WINDOW_TYPE                 (OLDSP_BASE_ERROR + 7)
#define OLDSP_NEGATIVE_INPUT_MAGNITUDE        (OLDSP_BASE_ERROR + 8)


typedef enum {
    UNSIGNED_DATA,
    SIGNED_DATA,
    FLOAT_DATA
} DATATYPE;
                                          
typedef enum {
    RECTANGLE,
    HANNING,
    HAMMING,
    BLACKMAN,
    BLACKMAN_HARRIS_67,
    BLACKMAN_HARRIS_92
} WINDOWTYPE;
                                          
//------------------ function prototypes-----------------------------
#ifdef __cplusplus
extern "C" {
#endif

ECODE WINAPI __export olDspRealFFT(HBUF in, HBUF mag, HBUF phase, int fftsize);


ECODE WINAPI __export olDspWindow(HBUF in, HBUF out, UINT WindowSize, WINDOWTYPE WindowType, 
								  DATATYPE DataType, ULNG InputOffset);

ECODE WINAPI __export olDspInputToVolts(HBUF hBufIn, HBUF hBufVolts,DATATYPE DataType,
										float MinRange, float MaxRange, WORD bits);

ECODE WINAPI __export olDspVoltsToOutput(HBUF hBufVolts, HBUF hBufOut,DATATYPE DataType,
										 float MinRange, float MaxRange, WORD bits);

ECODE WINAPI __export olDspMagToDB(HBUF in, HBUF out);
LPSTR WINAPI __export olDspGetErrorString(ECODE status,LPSTR str, UINT MaxSize);
ECODE WINAPI __export olDspInitFFT(void);  //can be used to pre-init FFT tables, not required

#ifdef __cplusplus
}
#endif

#endif //!defined(__OLOLDSP__)
