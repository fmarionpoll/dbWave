// CyberAmp.h : header file
// adapted from C300LibC from Axon Instruments - 1992
// and AxonBuscC
#pragma once
#include "AcqWaveChan.h"

constexpr auto VERSIONCYB = "Version 1.1 - FMP may 1999/sept 2022";
constexpr auto RCVBUFSIZE = 2048;

constexpr auto C300_CLOSED = 0;

constexpr auto MAXCMDLEN = 63;

constexpr auto COMMANDPREFIX = "AT";
constexpr auto ENDCOMMAND = 13;
constexpr auto ENDRESPONSE = '>';

constexpr auto DEFAULTRCVDELAY = 100; 		//20;
constexpr auto C300_SUCCESS = 0;

constexpr auto C300_BADDEVICE = 1;
constexpr auto C300_BADSERIAL = 2;
constexpr auto C300_BADDEVICENUMBER = 3;
constexpr auto C300_BADCHANNEL = 4;
constexpr auto C300_BADVALUE = 5;
constexpr auto C300_BADCOMMAND = 6;
constexpr auto C300_COULDNOTDO = 7;
constexpr auto C300_XMITTIMEOUT = 8;
constexpr auto C300_RCVTIMEOUT = 9;
constexpr auto C300_RCVBUFOVERFLOW = 10;
constexpr auto C300_NOTOPEN = 11;
constexpr auto C300_TOOFAST = 12;
constexpr auto C300_RCVOVERRUN = 13;
constexpr auto C300_BADOFFSET = 14;
constexpr auto C300_BADAXOBUS_ERROR = 15;

constexpr auto C300_COM1 = 1;
constexpr auto C300_COM2 = 2;
constexpr auto C300_COM3 = 3;
constexpr auto C300_COM4 = 4;

constexpr auto C300_SPEEDDEFAULT = 0;

constexpr auto C300_SPEED300 = 1;
constexpr auto C300_SPEED600 = 2;
constexpr auto C300_SPEED1200 = 3;
constexpr auto C300_SPEED2400 = 4;
constexpr auto C300_SPEED4800 = 5;
constexpr auto C300_SPEED9600 = 6;
constexpr auto C300_SPEED19200 = 7;

constexpr auto C300_POSINPUT = -1;
constexpr auto C300_NEGINPUT = 0;

constexpr auto C300_ENABLED = -1;
constexpr auto C300_DISABLED = 0;

constexpr auto C300_COUPLINGGND = 0;
constexpr auto C300_COUPLINGDC = -1;
constexpr auto C300_COUPLINGAC = -2;

constexpr auto C300_FILTERDISABLED = 0;
constexpr auto C300_NOCHANGE = 0;

constexpr auto C300_MINCHANNEL = 1;
constexpr auto C300_MAXCHANNEL = 16;

constexpr auto C300_DEVICENULL = -1;

//////////////////////////////////////////////////////////////////////////////
//																			//
// Constant definitions														//
//																			//
//////////////////////////////////////////////////////////////////////////////

constexpr auto AXOBUS_COM1 = 1;
constexpr auto AXOBUS_COM2 = 2;
constexpr auto AXOBUS_COM3 = 3;
constexpr auto AXOBUS_COM4 = 4;

constexpr auto AXOBUS_SPEEDDEFAULT = 0;
constexpr auto AXOBUS_SPEED300 = 1;
constexpr auto AXOBUS_SPEED600 = 2;
constexpr auto AXOBUS_SPEED1200 = 3;
constexpr auto AXOBUS_SPEED2400 = 4;
constexpr auto AXOBUS_SPEED4800 = 5;
constexpr auto AXOBUS_SPEED9600 = 6;
constexpr auto AXOBUS_SPEED19200 = 7;

constexpr auto AXOBUS_CLOSED = 0;

//////////////////////////////////////////////////////////////////////////////
//																			//
// Definitions for error results returned by module							//
//																			//
//////////////////////////////////////////////////////////////////////////////

constexpr auto AXOBUS_SUCCESS = 0;

constexpr auto AXOBUS_BADDEVICE = 1;
constexpr auto AXOBUS_BADSERIAL = 2;
constexpr auto AXOBUS_XMITTIMEOUT = 3;
constexpr auto AXOBUS_RCVTIMEOUT = 4;
constexpr auto AXOBUS_RCVBUFOVERFLOW = 5;
constexpr auto AXOBUS_RCVOVERRUN = 6;


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
	int SetHPFilter(int nChannel, int nInput, const CString& cs_coupling);
	int GetHPFilterToken(CString value);
	int SetWaveChanParms(CWaveChan* pchan);
	int GetWaveChanParms(CWaveChan* pchan);

	int C300_SetDeviceNumber(int which_device);
	int C300_SetOutputPortAndSpeed(int nWhichPort, int nWhichSpeed);
	int C300_SetReceiveTimeout(DWORD fTimeoutVal);
	int C300_SetAmpGains(int nChannel, int nPreAmpGain, int nOutputGain);
	int C300_LoadFactoryDefaults(void);
	int C300_ElectrodeTest(int nEnabled);
	int C300_ZeroDCOffset(int nChannel, float* pfOffsetReturned);
	int C300_GetChannelStatus(int nChannel, char* lpszStatusText);
	int C300_GetOverloadStatus(int* lpnChannelMask);
	int C300_SaveCurrentState(void);
	int C300_FlushCommandsAndAwaitResponse(void);
	int C300_GetLastError(void);
	int C300_GetLastReception(char* lpszResultText);
	int C300_GetLibraryVersion(char* sz_txt);

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

	CString get_token_value(const CString& result, CString token);
	CString get_coupling_value(const CString& result);

	static void C300_ResetParameters();
	static void C300_StringConcatChar(char* lpsz_string, int c);
	static int C300_FoundListMatch(int nFilterValue, int* lpn_list, const int nListItems);

	int C300_INT_TranslateAXOBUS_Error(int error);
	int C300_INT_TranslateAXOBUS_COMSettings(int nWhichPort, int nWhichSpeed,
	                                      int* pnOutputPort, int* pnOutputSpeed);
	void C300_INT_StartCommand(void) const;
	void C300_INT_AddCommand(const char* lpszCommandText);

	int AXOBUS_Initialize(void);
	int AXOBUS_SetOutput(int nWhichPort, int nWhichSpeed);
	int AXOBUS_SendString(int nOutputPort, char* lpszCmdString, DWORD fDelay);
	int AXOBUS_ReceiveString(char* lpszCmdString, int nWaitOK, DWORD fDelay);
	void AXOBUS_FlushReceiveBuffer(int nOutputPort, DWORD fDelay);
};
