// CyberAmp.h : header file
// adapted from C300LibC from Axon Instruments - 1992
// and AxonBuscC
#pragma once

#define VERSIONCYB      "Version 1.1 - FMP may 1999/july 2017"
#define RCVBUFSIZE      2048

#define C300_CLOSED     0

#define MAXCMDLEN       63

#define COMMANDPREFIX   "AT"
#define ENDCOMMAND      13
#define ENDRESPONSE     '>'

#define DEFAULTRCVDELAY			100		//20
#define C300_SUCCESS  			0

#define C300_BADDEVICE       	1
#define C300_BADSERIAL       	2
#define C300_BADDEVICENUMBER	3
#define C300_BADCHANNEL      	4
#define C300_BADVALUE        	5
#define C300_BADCOMMAND      	6
#define C300_COULDNOTDO      	7
#define C300_XMITTIMEOUT     	8
#define C300_RCVTIMEOUT      	9
#define C300_RCVBUFOVERFLOW    10
#define C300_NOTOPEN           11
#define C300_TOOFAST           12
#define C300_RCVOVERRUN        13
#define C300_BADOFFSET		   14
#define C300_BADABUSERROR	   15

#define C300_COM1			1
#define C300_COM2          	2
#define C300_COM3          	3
#define C300_COM4          	4

#define C300_SPEEDDEFAULT  	0

#define C300_SPEED300      	1
#define C300_SPEED600      	2
#define C300_SPEED1200     	3
#define C300_SPEED2400     	4
#define C300_SPEED4800     	5
#define C300_SPEED9600     	6
#define C300_SPEED19200    	7

#define C300_POSINPUT      -1
#define C300_NEGINPUT       0

#define C300_ENABLED       -1
#define C300_DISABLED      	0

#define C300_COUPLINGGND   	0
#define C300_COUPLINGDC    -1
#define C300_COUPLINGAC    -2

#define C300_FILTERDISABLED	0
#define C300_NOCHANGE      	0

#define C300_MINCHANNEL    	1
#define C300_MAXCHANNEL    16

#define C300_DEVICENULL    -1

//////////////////////////////////////////////////////////////////////////////
//																			//
// Constant definitions														//
//																			//
//////////////////////////////////////////////////////////////////////////////

#define ABUS_COM1			1
#define ABUS_COM2          	2
#define ABUS_COM3          	3
#define ABUS_COM4          	4

#define ABUS_SPEEDDEFAULT  	0
#define ABUS_SPEED300      	1
#define ABUS_SPEED600      	2
#define ABUS_SPEED1200     	3
#define ABUS_SPEED2400     	4
#define ABUS_SPEED4800     	5
#define ABUS_SPEED9600     	6
#define ABUS_SPEED19200    	7

#define  ABUS_CLOSED		0

//////////////////////////////////////////////////////////////////////////////
//																			//
// Definitions for error results returned by module							//
//																			//
//////////////////////////////////////////////////////////////////////////////

#define ABUS_SUCCESS  			0

#define ABUS_BADDEVICE       	1
#define ABUS_BADSERIAL       	2
#define ABUS_XMITTIMEOUT     	3
#define ABUS_RCVTIMEOUT      	4
#define ABUS_RCVBUFOVERFLOW		5
#define ABUS_RCVOVERRUN			6
#include "AcqWaveChan.h"

// CCyberAmp command target

class CyberAmp : public CFile
{
public:
	CyberAmp();
	~CyberAmp() override;

	int Initialize(void);
	int SetGain(int nChannel, int nGainValue);
	int SetmVOffset(int nChannel, float fOffset);
	int SetLPFilter(int nChannel, int nFilterValue);
	int SetNotchFilter(int nChannel, int nEnabled);
	int SetHPFilter(int nChannel, int nInput, const CString& csCoupling);

	int SetWaveChanParms(CWaveChan* pchan);
	int GetWaveChanParms(CWaveChan* pchan);

	int C300_SetDeviceNumber(int nWhichDev);
	int C300_SetOutputPortAndSpeed(int nWhichPort, int nWhichSpeed);
	int C300_SetReceiveTimeout(DWORD fTimeoutVal);
	int C300_SetAmpGains(int nChannel, int nPreAmpGain, int nOutputGain);
	int C300_LoadFactoryDefaults(void);
	int C300_ElectrodeTest(int nEnabled);
	int C300_ZeroDCOffset(int nChannel, float* pfOffsetReturned);
	int C300_GetChannelStatus(int nChannel, char* lpStatusText);
	int C300_GetOverloadStatus(int* lpnChannelMask);
	int C300_SaveCurrentState(void);
	int C300_FlushCommandsAndAwaitResponse(void);
	int C300_GetLastError(void);
	int C300_GetLastReception(char* lpLastText);
	int C300_GetLibraryVersion(char* lpVersionText);

	// Implementation
protected:
	static char m_C300szCommands[4 * MAXCMDLEN + 1];
	static char m_C300szRcvText[RCVBUFSIZE + 1];
	static int m_C300nLastError;
	static int m_C300nDevNumber;
	static int m_C300nOutputPort;
	static int m_C300nOutputSpeed;
	static int m_C300nDebugVersion;
	static DWORD m_C300fReceiveDelay;
	COMMTIMEOUTS m_CommTimeOuts{};

	DCB m_dcb{}; // DCB structure containing COM parameters
	HANDLE m_hComm; // handle to file used to communicate with COM

	static void C300_ResetParms();
	static void C300_StringConcatChar(char* lpsz_string, int c);
	static int C300_FoundListMatch(int nFilterValue, int* lpnList, int nListItems);

	int C300_INT_TranslateABUSError(int ABUSError);
	int C300_INT_TranslateABUSCOMSettings(int nWhichPort, int nWhichSpeed,
	                                      int* pnOutputPort, int* pnOutputSpeed);
	void C300_INT_StartCommand(void);
	void C300_INT_AddCommand(char* lpszCommandText);

	int ABUS_Initialize(void);
	int ABUS_SetOutput(int nWhichPort, int nWhichSpeed);
	int ABUS_SendString(int nOutputPort, char* lpszCmdString, DWORD fDelay);
	int ABUS_ReceiveString(char* lpszCmdString, int nWaitOK, DWORD fDelay);
	void ABUS_FlushReceiveBuffer(int nOutputPort, DWORD fDelay);
};
