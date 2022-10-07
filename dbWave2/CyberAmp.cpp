// CyberAmp.cpp : implementation file
//

#include "StdAfx.h"
#include "CyberAmp.h"

#include "AcqWaveChan.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#define szLEN 100

char CyberAmp::m_C300szCommands[];
char CyberAmp::m_C300szRcvText[];
int CyberAmp::m_C300nLastError;
int CyberAmp::m_C300nDevNumber;
int CyberAmp::m_C300nOutputPort;
int CyberAmp::m_C300nOutputSpeed;
int CyberAmp::m_C300nDebugVersion;
DWORD CyberAmp::m_C300fReceiveDelay;

CyberAmp::CyberAmp()
{
	C300_ResetParameters();
	m_hComm = nullptr;
}

void CyberAmp::C300_ResetParameters()
{
	m_C300nLastError = C300_SUCCESS;
	m_C300nOutputPort = AXOBUS_CLOSED;
	m_C300nOutputSpeed = AXOBUS_SPEEDDEFAULT;
	m_C300nDevNumber = C300_DEVICENULL;
	m_C300nDebugVersion = FALSE;
	m_C300fReceiveDelay = DEFAULTRCVDELAY;
}

CyberAmp::~CyberAmp()
{
	if (m_hComm != nullptr)
		CloseHandle(m_hComm);
}

// Do not edit the following lines, which are needed by ClassWizard.
#if 0
BEGIN_MESSAGE_MAP(CyberAmp, CFile)

END_MESSAGE_MAP()
#endif	// 0

/////////////////////////////////////////////////////////////////////////////
// CyberAmp member functions

int CyberAmp::Initialize()
{
	C300_ResetParameters();
	if (m_hComm != nullptr)
		CloseHandle(m_hComm);
	m_hComm = nullptr;

	m_C300nLastError = AXOBUS_Initialize();

	return (m_C300nLastError);
}

int CyberAmp::C300_SetOutputPortAndSpeed(int nWhichPort, int nWhichSpeed)
{
	m_C300nLastError = C300_SUCCESS;
	C300_INT_TranslateAXOBUS_COMSettings(nWhichPort, nWhichSpeed,&m_C300nOutputPort, &m_C300nOutputSpeed);
	m_C300nLastError = AXOBUS_SetOutput(m_C300nOutputPort, m_C300nOutputSpeed);
	if (m_C300nLastError)
		m_C300nLastError = C300_INT_TranslateAXOBUS_Error(m_C300nLastError);

	return (m_C300nLastError);
}

int CyberAmp::C300_INT_TranslateAXOBUS_Error(int error)
{
	switch (error)
	{
	case AXOBUS_SUCCESS: m_C300nLastError = C300_SUCCESS;
		break;
	case AXOBUS_BADDEVICE: m_C300nLastError = C300_BADDEVICE;
		break;
	case AXOBUS_BADSERIAL: m_C300nLastError = C300_BADSERIAL;
		break;
	case AXOBUS_XMITTIMEOUT: m_C300nLastError = C300_XMITTIMEOUT;
		break;
	case AXOBUS_RCVTIMEOUT: m_C300nLastError = C300_RCVTIMEOUT;
		break;
	case AXOBUS_RCVBUFOVERFLOW: m_C300nLastError = C300_RCVBUFOVERFLOW;
		break;
	case AXOBUS_RCVOVERRUN: m_C300nLastError = C300_RCVOVERRUN;
		break;
	default: m_C300nLastError = C300_BADAXOBUS_ERROR;
		break;
	}

	return (m_C300nLastError);
}

int CyberAmp::C300_INT_TranslateAXOBUS_COMSettings(int nWhichPort, int nWhichSpeed, int* pnOutputPort, int* pnOutputSpeed)
{
	m_C300nLastError = C300_SUCCESS;
	switch (nWhichPort)
	{
	case C300_COM1: *pnOutputPort = AXOBUS_COM1;
		break;
	case C300_COM2: *pnOutputPort = AXOBUS_COM2;
		break;
	case C300_COM3: *pnOutputPort = AXOBUS_COM3;
		break;
	case C300_COM4: *pnOutputPort = AXOBUS_COM4;
		break;
	default: *pnOutputPort = AXOBUS_CLOSED;
		break;
	}

	switch (nWhichSpeed)
	{
	case C300_SPEED19200: *pnOutputSpeed = AXOBUS_SPEED19200;
		break;
	case C300_SPEED9600: *pnOutputSpeed = AXOBUS_SPEED9600;
		break;
	case C300_SPEED4800: *pnOutputSpeed = AXOBUS_SPEED4800;
		break;
	case C300_SPEED2400: *pnOutputSpeed = AXOBUS_SPEED2400;
		break;
	case C300_SPEED1200: *pnOutputSpeed = AXOBUS_SPEED1200;
		break;
	case C300_SPEED600: *pnOutputSpeed = AXOBUS_SPEED600;
		break;
	case C300_SPEED300: *pnOutputSpeed = AXOBUS_SPEED300;
		break;
	case C300_SPEEDDEFAULT: 
	default: *pnOutputSpeed = AXOBUS_SPEEDDEFAULT;
		break;
	}

	return (m_C300nLastError);
}

//  Internal routine to add the necessary command prefix to the  command string

void CyberAmp::C300_INT_StartCommand() const
{
	strcat_s(m_C300szCommands, 4 * MAXCMDLEN, COMMANDPREFIX);

	// If AXOBUS_ device numbers are in use, specify which device
	if (m_C300nDevNumber >= 0)
	{
		char sz_buffer[20];
		sprintf_s(sz_buffer, 19, "%d", m_C300nDevNumber);
		strcat_s(m_C300szCommands, 4 * MAXCMDLEN, sz_buffer);
	}
}

//  Internal routine to append a command sequence to the current  command string

void CyberAmp::C300_INT_AddCommand(const char* lpszCommandText)
{
	// If this is the start of a new command, then flush the
	//     hardware receive buffer and apply the necessary command prefix
	if (strlen(m_C300szCommands) == 0)
	{
		AXOBUS_FlushReceiveBuffer(m_C300nOutputPort, m_C300fReceiveDelay);
		C300_INT_StartCommand();
	}

	// Look for the last portion of the command string
	auto lpsz_end_of_command = strrchr(m_C300szCommands, ENDCOMMAND);
	if (lpsz_end_of_command != nullptr)
		++lpsz_end_of_command;
	else
		lpsz_end_of_command = m_C300szCommands;

	// If this command would overflow the maximum command length, start a new portion
	if (strlen(lpsz_end_of_command) + strlen(lpszCommandText) > MAXCMDLEN)
	{
		// Terminate the current command portion and start a new portion
		C300_StringConcatChar(m_C300szCommands, ENDCOMMAND);
		C300_INT_StartCommand();
	}
	strcat_s(m_C300szCommands, 4 * MAXCMDLEN, lpszCommandText);
}

//  Flushes any pending command portions one at a time and awaits the response from the CyberAmp
int CyberAmp::C300_FlushCommandsAndAwaitResponse()
{
	char sz_text[szLEN + 1];

	m_C300nLastError = C300_SUCCESS;
	if (strlen(m_C300szCommands) > 0)
	{
		char* lpszEndCommand;
		do
		{
			if ((lpszEndCommand = strchr(m_C300szCommands, ENDCOMMAND))
				!= nullptr)
			{
				char szRestOfCommand[szLEN + 1];
				// copy the first half of this string into sz_text and terminate it
				*lpszEndCommand = '\0';
				strcpy_s(sz_text, szLEN, m_C300szCommands);

				// copy the second half into szRestOfCommand
				strcpy_s(szRestOfCommand, szLEN, lpszEndCommand + 1);
				strcpy_s(m_C300szCommands, szLEN, szRestOfCommand);
			}
			else
			{
				strcpy_s(sz_text, szLEN, m_C300szCommands);
				*m_C300szCommands = '\0';
			}

			// Add the required command terminator
			C300_StringConcatChar(sz_text, ENDCOMMAND);

			// Send the string and wait for the response
			if (AXOBUS_SetOutput(m_C300nOutputPort, m_C300nOutputSpeed) == AXOBUS_SUCCESS)
			{
				if (AXOBUS_SendString(m_C300nOutputPort, &sz_text[0], m_C300fReceiveDelay) == AXOBUS_SUCCESS)
				{
					if (AXOBUS_ReceiveString(&m_C300szRcvText[0], TRUE, m_C300fReceiveDelay) == AXOBUS_SUCCESS)
					{
						if (strlen(m_C300szRcvText) > 0 && *m_C300szRcvText == '?')
							m_C300nLastError = C300_BADCOMMAND;
					}
				}
				else
					*m_C300szRcvText = '\0';
			}
		}
		while (strlen(m_C300szCommands) > 0 && m_C300nLastError == C300_SUCCESS);
	}

	// Return the most recent error flag
	if (m_C300nLastError != 0)
	{
		CString cs_error;
		cs_error.Format(
			_T("Error returned by CyberAmp= %i\n when issuing command:\n%s\noutput port: %i\noutput speed: %d\n"),
			m_C300nLastError, (LPCTSTR)sz_text, m_C300nOutputPort, m_C300nOutputSpeed
		);
		AfxMessageBox(cs_error, MB_OK);
	}

	return (m_C300nLastError);
}

//  Returns the status text for a single channel

int CyberAmp::C300_GetChannelStatus(int nChannel, char* lpszStatusText)
{
	if (C300_FlushCommandsAndAwaitResponse() == C300_SUCCESS)
	{
		char sz_text[RCVBUFSIZE+1];
		sprintf_s(sz_text, 99, "S%d", nChannel);
		C300_INT_AddCommand(sz_text);
		if (C300_FlushCommandsAndAwaitResponse() == C300_SUCCESS)
			C300_GetLastReception(lpszStatusText);
	}
	return (m_C300nLastError);
}

//  Set the AXOBUS device number of the CyberAmp

int CyberAmp::C300_SetDeviceNumber(int which_device)
{
	// Check the device number for validity
	if ((which_device < 0 || which_device > 9) && which_device != C300_DEVICENULL)
		m_C300nLastError = C300_BADDEVICENUMBER;
	else
	{
		m_C300nLastError = C300_SUCCESS;

		// If we are changing the current device number, flush any pending commands
		if (m_C300nDevNumber != which_device)
			C300_FlushCommandsAndAwaitResponse();
		m_C300nDevNumber = which_device;
	}
	return (m_C300nLastError);
}

//   Set the gain for a given channel

int CyberAmp::SetGain(int nChannel, int nGainValue)
{
	int nPreAmp, nGainDivisor;
	// Clear the last error flag
	m_C300nLastError = C300_SUCCESS;

	// Check the channel number for validity
	if (nChannel < C300_MINCHANNEL || nChannel > C300_MAXCHANNEL)
		return (m_C300nLastError = C300_BADCHANNEL);

	switch (nGainValue) // Build the gain command and channel value
	{
	case 1:
	case 2:
	case 5:
		nPreAmp = 1;
		nGainDivisor = 1;
		break;

	case 10:
	case 20:
	case 50:
		nPreAmp = 10;
		nGainDivisor = 10;
		break;

	case 100:
	case 200:
	case 500:
	case 1000:
	case 2000:
	case 5000:
	case 10000:
	case 20000:
		nPreAmp = 100;
		nGainDivisor = 100;
		break;

	default:
		nPreAmp = 1;
		nGainDivisor = 1;
		m_C300nLastError = C300_BADVALUE;
	}

	// Add the current command to the command string
	if (m_C300nLastError == C300_SUCCESS)
	{
		char sz_text[szLEN];
		sprintf_s(sz_text, 99, "G%dP%dG%dO%d", nChannel, nPreAmp, nChannel, nGainValue / nGainDivisor);
		C300_INT_AddCommand(sz_text);
	}

	return (m_C300nLastError);
}

//   Set the pre-amp and output gains

int CyberAmp::C300_SetAmpGains(int nChannel, int nPreAmpGain, int nOutputGain)
{
	char sz_text[szLEN];

	// This buffer must be cleared here, as it will be
	//     strcat()'ed below, and will not be initialized
	//     properly
	strcpy_s(sz_text, 99, "");

	// Clear the last error flag
	m_C300nLastError = C300_SUCCESS;

	// Check the channel number for validity
	if (nChannel < C300_MINCHANNEL || nChannel > C300_MAXCHANNEL)
		return (m_C300nLastError = C300_BADCHANNEL);

	// Build the pre-amp gain command if the value is to be changed
	if (nPreAmpGain != C300_NOCHANGE)
	{
		char szPreAmp[20];
		switch (nPreAmpGain)
		{
		case 1:
		case 10:
		case 100:
			sprintf_s(szPreAmp, 19, "G%dP%d", nChannel, nPreAmpGain);
			strcat_s(sz_text, 99, szPreAmp);
			break;

		default:
			m_C300nLastError = C300_BADVALUE;
		}
	}

	// Build the output gain command if the value is to be changed
	if (nOutputGain != C300_NOCHANGE)
	{
		char szOutput[20];
		switch (nOutputGain)
		{
		case 1:
		case 2:
		case 5:
		case 10:
		case 20:
		case 50:
		case 100:
		case 200:
			sprintf_s(szOutput, 19, "G%dO%d", nChannel, nOutputGain);
			strcat_s(sz_text, 99, szOutput);
			break;

		default:
			m_C300nLastError = C300_BADVALUE;
		}
	}

	// Add the current command to the command string
	if (m_C300nLastError == C300_SUCCESS && strlen(sz_text) > 0)
		C300_INT_AddCommand(sz_text);

	return (m_C300nLastError);
}

//   Set the input coupling for a given channel

int CyberAmp::SetHPFilter(int nChannel, int nInput, const CString& cs_coupling)
{
	char sz_text[szLEN];

	m_C300nLastError = C300_SUCCESS;
	// Check the channel number for validity
	if (nChannel < C300_MINCHANNEL || nChannel > C300_MAXCHANNEL)
		return (m_C300nLastError = C300_BADCHANNEL);

	// Build the coupling command and the channel number
	sprintf_s(sz_text, 99, "C%d", nChannel);
	// Set the positive or negative input command
	if (nInput == C300_NEGINPUT)
		strcat_s(sz_text, 99, "-");
	else
		strcat_s(sz_text, 99, "+");

	// set filtering value
	const CStringA cs_a_coupling(cs_coupling);
	strcat_s(sz_text, 99, cs_a_coupling);

	// list of valid commands
	/*
	switch (coupling)
	{
	case -10:	strcat( sz_text, "GND" );break;
	case 0:		strcat( sz_text, "DC" ); break;		// DC
	case 1:		strcat( sz_text, "0.1" ); break;	// 0.1 Hz
	case 10:	strcat( sz_text, "1" ); break;		// 1 Hz
	case 100:	strcat( sz_text, "10" ); break;		// 10 Hz
	case 300:	strcat( sz_text, "30" ); break;		// 30 Hz
	case 1000:	strcat( sz_text, "100" ); break;	// 100 Hz
	case 3000:	strcat( sz_text, "300" ); break;	// 300 Hz
	default:	m_C300nLastError = C300_BADVALUE; break;
	}
	*/
	// Add the current command to the command string
	if (m_C300nLastError == C300_SUCCESS)
		C300_INT_AddCommand(sz_text);

	return (m_C300nLastError);
}

int CyberAmp::GetHPFilterToken(CString value)
{
	int filter_item = 0;
	if (value.IsEmpty() || value.Find(_T("DC")) >= 0)
		return filter_item;
	if (value.Find(_T("GND") >= 0))
		filter_item = -10;
	else
	{
		const double dValue = _ttof(value)* 10;
		filter_item = static_cast<int>(dValue);
	}
	return filter_item;
}

int CyberAmp::SetWaveChanParms(CWaveChan* pchan)
{
	// chan, gain, filter +, lowpass, notch
	SetHPFilter(pchan->am_amplifierchan, C300_POSINPUT, pchan->am_csInputpos);
	SetHPFilter(pchan->am_amplifierchan, C300_NEGINPUT, pchan->am_csInputneg);
	SetmVOffset(pchan->am_amplifierchan, pchan->am_offset);
	SetNotchFilter(pchan->am_amplifierchan, pchan->am_notchfilt);
	const auto gain = static_cast<double>(pchan->am_gaintotal)
		/ (static_cast<double>(pchan->am_gainheadstage) 
		* static_cast<double>(pchan->am_gainAD));
	SetGain(pchan->am_amplifierchan, static_cast<int>(gain));
	SetLPFilter(pchan->am_amplifierchan, static_cast<int>(pchan->am_lowpass));
	return C300_FlushCommandsAndAwaitResponse();
}

int CyberAmp::GetWaveChanParms(CWaveChan* pchan)
{
	char sz_text[RCVBUFSIZE];
	C300_GetChannelStatus(pchan->am_amplifierchan, &sz_text[0]);
	const CString result(sz_text);

	const CString string1 = get_token_value(result, _T("X="));
	if (!string1.IsEmpty() && string1.Find(_T("0")) > 0)
		pchan->am_csheadstage = string1;

	const CString string2 = get_token_value(result, _T("+="));
	pchan->am_csInputpos = get_coupling_value(string2);

	const CString string3 = get_token_value(result, _T("-="));
	pchan->am_csInputneg = get_coupling_value(string3);

	CString string4 = get_token_value(result, _T("P="));
	pchan->am_gainpre = _ttoi(string4);

	string4 = get_token_value(result, _T("O="));
	pchan->am_gainpost = _ttoi(string4);

	string4 = get_token_value(result, _T("N="));
	pchan->am_notchfilt = _ttoi(string4);

	string4 = get_token_value(result, _T("D="));
	pchan->am_offset = static_cast<float>(_ttof(string4)/1000.);

	string4 = get_token_value(result, _T("F="));
	pchan->am_lowpass = _ttoi(string4);

	const short gain = pchan->am_gainpost * pchan->am_gainpre;
	pchan->am_amplifiergain = gain;

	return 0;
}

CString CyberAmp::get_coupling_value(const CString& result)
{
	if (result.CompareNoCase(_T("GND")) == 0 || result.CompareNoCase(_T("DC")) == 0)
		return result;
	const double hp_filter = _ttof(result);
	CString cs_value;
	if (hp_filter < 1.)
		cs_value.Format(_T("%.1f"), hp_filter);
	else 
		cs_value.Format(_T("%i"), static_cast<int>(hp_filter));
	return cs_value;
}

CString CyberAmp::get_token_value(const CString& result, CString token)
{
	const int first = result.Find(token, 0);
	CString bout = result.Mid(first+2);
	CString resToken = result.Mid(first+2).SpanExcluding(_T(" \n\r"));
	return resToken;
}

//  Set the input offset for a given channel (in mV)

int CyberAmp::SetmVOffset(int nChannel, float fOffset)
{
	// Clear the last error flag
	m_C300nLastError = C300_SUCCESS;

	// Check the channel number for validity
	if (nChannel < C300_MINCHANNEL || nChannel > C300_MAXCHANNEL)
		return (m_C300nLastError = C300_BADCHANNEL);

	// Flush out any pending command string
	if ((m_C300nLastError = C300_FlushCommandsAndAwaitResponse()) != C300_SUCCESS)
		return (m_C300nLastError);

	// Build the offset command and the channel no.
	// Check the offset for valid range 
	if (fOffset >= -3000.0F && fOffset <= 3000.0F)
	{
		char sz_text[szLEN];
		sprintf_s(sz_text, 99, "D%d%.0f", nChannel, 1000000.0F * fOffset / 1000.0F);
		C300_INT_AddCommand(sz_text);
		if (C300_FlushCommandsAndAwaitResponse() == C300_SUCCESS)
		{
			if (*m_C300szRcvText == 'D' && *(m_C300szRcvText + 3) == '!')
				m_C300nLastError = C300_BADOFFSET;
		}
	}
	else
		m_C300nLastError = C300_BADVALUE;

	return (m_C300nLastError);
}

//   Set the filter for a given channel

static int nFilterValueList[] =
{
	2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30,
	40, 60, 80, 100, 120, 140, 160, 180, 200, 220, 240,
	260, 280, 300, 400, 600, 800, 1000, 1200, 1400, 1600,
	1800, 2000, 2200, 2400, 2600, 2800, 3000, 3200, 4000,
	6000, 8000, 10000, 12000, 14000, 16000, 18000, 20000,
	22000, 24000, 26000, 28000, 30000
};

#define NUM_ENTRIES     58
#define NOT_FOUND       -1

int CyberAmp::SetLPFilter(int nChannel, int nFilterValue)
{
	char sz_text[szLEN];

	m_C300nLastError = C300_SUCCESS;

	// Check the channel number for validity
	if (nChannel < C300_MINCHANNEL || nChannel > C300_MAXCHANNEL)
		return (m_C300nLastError = C300_BADCHANNEL);

	// Build the filter command and the channel number
	sprintf_s(sz_text, 99, "F%d", nChannel);

	// Add the filter value to the command
	//     If disabled, send out the disable command
	//     Otherwise, search the list of possible values for
	//     a match
	if (nFilterValue == C300_FILTERDISABLED)
	{
		strcat_s(sz_text, 99, "-");
		C300_INT_AddCommand(sz_text);
	}
	else if (C300_FoundListMatch(nFilterValue, nFilterValueList, NUM_ENTRIES)
		!= NOT_FOUND)
	{
		char sz_buffer[20];
		sprintf_s(sz_buffer, 19, "%d", nFilterValue);
		strcat_s(sz_text, 99, sz_buffer);
		C300_INT_AddCommand(sz_text);
	}
	else
		m_C300nLastError = C300_BADVALUE;

	return (m_C300nLastError);
}

//  Set the notch filter ON or OFF for a given channel

int CyberAmp::SetNotchFilter(int nChannel, int nEnabled)
{
	char sz_text[szLEN];
	m_C300nLastError = C300_SUCCESS;

	// Check the channel number for validity
	if (nChannel < C300_MINCHANNEL || nChannel > C300_MAXCHANNEL)
		return (m_C300nLastError = C300_BADCHANNEL);

	// Build the notch filter command and the channel number
	if (nEnabled == C300_DISABLED)
		sprintf_s(sz_text, 99, "N%d-", nChannel);
	else
		sprintf_s(sz_text, 99, "N%d+", nChannel);

	C300_INT_AddCommand(sz_text);

	return (m_C300nLastError);
}

//  Zero the DC offset for a given channel.  Returns the actual offset (in mV)
int CyberAmp::C300_ZeroDCOffset(int nChannel, float* pfOffsetReturned)
{
	m_C300nLastError = C300_SUCCESS; // Clear the last error flag
	// Check the channel number for validity
	if (nChannel < C300_MINCHANNEL || nChannel > C300_MAXCHANNEL)
		return (m_C300nLastError = C300_BADCHANNEL);

	// Flush out any pending command string
	if (C300_FlushCommandsAndAwaitResponse() == C300_SUCCESS)
	{
		char sz_text[szLEN];
		// Build the zero command and the channel number
		sprintf_s(sz_text, 99, "Z%d", nChannel);

		// Append the command to the command string and flush it out
		C300_INT_AddCommand(sz_text);
		if (C300_FlushCommandsAndAwaitResponse() == C300_SUCCESS)
		{
			char* lpsz_offset;
			// Check the returned text for any error.  Pick out the
			//    offset value and return it to the caller.
			if (*m_C300szRcvText == 'D'
				&& (lpsz_offset = strchr(m_C300szRcvText, '=')) != nullptr)
			{
				if (*(lpsz_offset + 1) == '!')
					if (C300_GetChannelStatus(nChannel, sz_text) == C300_SUCCESS)
					{
						*pfOffsetReturned = static_cast<float>(atof(sz_text + 44)) / 1000.0F;
						SetmVOffset(nChannel, *pfOffsetReturned);
					}
					else
						m_C300nLastError = C300_COULDNOTDO;
				else
					*pfOffsetReturned = static_cast<float>(atof(lpsz_offset + 1)) / 1000.0F;
			}
			else
				m_C300nLastError = C300_BADVALUE;
		}
	}

	return (m_C300nLastError);
}

//  Reload the factory defaults  for all settings

int CyberAmp::C300_LoadFactoryDefaults(void)
{
	m_C300nLastError = C300_SUCCESS;

	// Output the initialization command
	C300_INT_AddCommand("L");

	return (m_C300nLastError);
}

//  CyberAmp to save the current state in the EEPROM

int CyberAmp::C300_SaveCurrentState(void)
{
	// Output the write EEPROM command
	C300_INT_AddCommand("W");

	// Return the value returned from C300_FlushCommands()
	return (C300_FlushCommandsAndAwaitResponse());
}

//  Enter or exit the electrode test mode

int CyberAmp::C300_ElectrodeTest(int nEnabled)
{
	m_C300nLastError = C300_SUCCESS;

	// Output the appropriate electrode test command
	if (nEnabled == C300_DISABLED)
		C300_INT_AddCommand("TO-");
	else
		C300_INT_AddCommand("TO+");

	return (m_C300nLastError);
}

//  Set the receive timeout value

int CyberAmp::C300_SetReceiveTimeout(DWORD fTimeoutVal)
{
	// Check the timeout value for a return to the default
	if (fTimeoutVal > 0.0)
		m_C300fReceiveDelay = fTimeoutVal;
	else
		m_C300fReceiveDelay = DEFAULTRCVDELAY;

	return (m_C300nLastError = C300_SUCCESS);
}

//  Return the overload status of all channels

int CyberAmp::C300_GetOverloadStatus(int* lpnChannelMask)
{
	int n_channel;

	// Clear the last error flag
	m_C300nLastError = C300_SUCCESS;

	// Clear the passed channel overload mask
	*lpnChannelMask = 0;

	// Flush any pending command string
	if (C300_FlushCommandsAndAwaitResponse() == C300_SUCCESS)
	{
		// Build and flush the overload command
		C300_INT_AddCommand("O");
		if (C300_FlushCommandsAndAwaitResponse() == C300_SUCCESS)
		{
			char* lpsz_temp;
			auto lpsz_string = m_C300szRcvText;
			while (sscanf_s(lpsz_string, "%d", &n_channel) > 0)
			{
				*lpnChannelMask |= (2 << (n_channel - 1));
				if ((lpsz_temp = strchr(lpsz_string, ' ')) != nullptr)
					lpsz_string = lpsz_temp + 1;
				else
					lpsz_string = m_C300szRcvText + strlen(m_C300szRcvText);
			}
		}
	}
	return (m_C300nLastError);
}

//  Return the last error generated by a call to the CyberAmp 300 support module

int CyberAmp::C300_GetLastError(void)
{
	return (m_C300nLastError);
}

//  Return the raw text from the last communication with theCyberAmp

int CyberAmp::C300_GetLastReception(char* lpszResultText)
{
	auto p_char_end = strchr(m_C300szRcvText, ENDCOMMAND);
	if (p_char_end != nullptr)
		*(p_char_end + 1) = '\0';
	else
	{
		p_char_end = strchr(m_C300szRcvText, 0);
		const auto len_rcv = strlen(m_C300szRcvText);
		constexpr size_t receiving_buffer_size = RCVBUFSIZE;
		if (len_rcv > receiving_buffer_size)
		{
			p_char_end = &m_C300szRcvText[0] + receiving_buffer_size - 1;
			*(p_char_end + 1) = '\0';
		}
	}

	constexpr size_t receiving_buffer_size = RCVBUFSIZE;
	strcpy_s(lpszResultText, receiving_buffer_size, m_C300szRcvText);
	return (m_C300nLastError = C300_SUCCESS);
}

//  Return the library version number as a string

int CyberAmp::C300_GetLibraryVersion(char* sz_txt)
{
	strcpy_s(sz_txt, strlen(sz_txt), VERSIONCYB);
	return (m_C300nLastError = C300_SUCCESS);
}

//  Internal routine to concatenate a single char to a string

void CyberAmp::C300_StringConcatChar(char* lpsz_string, int c)
{
	int n_len = strlen(lpsz_string);
	lpsz_string[n_len++] = static_cast<char>(c);
	lpsz_string[n_len] = '\0';
}

//  Internal routine which finds a match using a binary search

int CyberAmp::C300_FoundListMatch(int nFilterValue, int* lpn_list, const int nListItems)
{
	// Test for a match with the 0 offset element of the list;
	//     the following algorithm will not find this entry
	if (nFilterValue == *lpn_list)
		return (0);

	// Find the smallest multiple of two greater than half the
	//     number of list items
	auto n_jump = 1;
	while (n_jump < nListItems / 2)
		n_jump <<= 1;

	// Now search the list for a match
	lpn_list += n_jump;
	auto n_offset = n_jump;
	while (nFilterValue != *lpn_list && n_jump >= 1)
	{
		n_jump >>= 1;
		if (nFilterValue < *lpn_list)
		{
			lpn_list -= n_jump;
			n_offset -= n_jump;
		}
		else
		{
			while (n_offset + n_jump > nListItems - 1)
				n_jump >>= 1;

			lpn_list += n_jump;
			n_offset += n_jump;
		}
	}

	if (nFilterValue == *lpn_list)
		return (n_offset);
	return (NOT_FOUND);
}

// Initialize bus

int CyberAmp::AXOBUS_Initialize()
{
	constexpr auto iCOM4 = 4;
	char sz_status_text[RCVBUFSIZE + 1]; 
	sz_status_text[0] = '\0';
	m_C300fReceiveDelay = 2;

	for (auto i = 1; i <= iCOM4; i++)
	{
		const auto error = AXOBUS_SetOutput(i, 9600);
		if (error == 0 || m_hComm != INVALID_HANDLE_VALUE)
		{
			// get status of the amplifier
			if (C300_FlushCommandsAndAwaitResponse() != C300_SUCCESS)
				return (m_C300nLastError);

			// Output the general status command and flush it out
			C300_INT_AddCommand("S0");
			if (C300_FlushCommandsAndAwaitResponse() == C300_SUCCESS)
				C300_GetLastReception(sz_status_text);

			const int sz_status_length = strlen(sz_status_text);
			if (sz_status_length == 0)
				m_C300nLastError = C300_BADDEVICE;

			if (m_C300nLastError == NULL)
			{
				m_C300fReceiveDelay = DEFAULTRCVDELAY;
				if (C300_FlushCommandsAndAwaitResponse() == C300_SUCCESS)
				{
					// Output the general status command and flush it out
					C300_INT_AddCommand("S0");
					if (C300_FlushCommandsAndAwaitResponse() == C300_SUCCESS)
						C300_GetLastReception(sz_status_text);

					const int sz_status_length2 = strlen(sz_status_text);
					if (sz_status_length2 == 0)
						m_C300nLastError = C300_BADDEVICE;

					return (m_C300nLastError);
				}
				return C300_SUCCESS;
			}
		}
	}
	return C300_BADDEVICE;
}

int CyberAmp::AXOBUS_SetOutput(int nWhichPort, int nWhichSpeed)
{
	// close file if already opened
	if (m_hComm != nullptr)
	{
		CloseHandle(m_hComm);
		m_hComm = nullptr;
	}
	m_C300nOutputPort = nWhichPort;
	m_C300nOutputSpeed = nWhichSpeed;

	// open file
	CString cs_comchan;
	switch (m_C300nOutputPort)
	{
	case AXOBUS_COM1: cs_comchan = "COM1";
		break;
	case AXOBUS_COM2: cs_comchan = "COM2";
		break;
	case AXOBUS_COM3: cs_comchan = "COM3";
		break;
	case AXOBUS_COM4: cs_comchan = "COM4";
		break;
	default: cs_comchan = "COM1";
		break;
	}

	// ! note: CreateFile fails when the CYBERAMP program is running
	// probably because the resource cannot be shared
	m_hComm = CreateFile(cs_comchan, // file name (com port)
	                     GENERIC_READ | GENERIC_WRITE, // dwDesiredAccess: generic RW
	                     0, // dwShareMode: exclusive access
	                     nullptr, // lpSecurityAttributes: no security attributes
	                     OPEN_EXISTING, // dwCreationDisposition: must exist
	                     0, // dwFlagsAndAttributes: not overlapped I/O
	                     nullptr); // hTemplateFile: handle to file with attrib to copy

	if (m_hComm == INVALID_HANDLE_VALUE)
	{
		return GetLastError();
	}

	// get the current configuration
	BOOL fsuccess = GetCommState(m_hComm, &m_dcb);
	if (!fsuccess)
		return GetLastError();

	// fill the DCB: baud: 9600, parity, etc.
	m_dcb.BaudRate = m_C300nOutputSpeed;
	m_dcb.ByteSize = 8;
	m_dcb.Parity = NOPARITY;
	m_dcb.StopBits = ONESTOPBIT;
	fsuccess = SetCommState(m_hComm, &m_dcb);
	if (!fsuccess)
		return GetLastError();

	// ! note: the following section code returns an error on TASTE3 under Windows XP2
	//// set com input and output buffer size
	fsuccess = SetupComm(m_hComm, RCVBUFSIZE, RCVBUFSIZE);
	if (!fsuccess)
		return GetLastError();

	fsuccess = GetCommTimeouts(m_hComm, &m_CommTimeOuts);
	if (!fsuccess)
		return GetLastError();

	return 0;
}

int CyberAmp::AXOBUS_SendString(int nOutputPort, char* lpszCmdString, DWORD fDelay)
{
	if (nOutputPort != m_C300nOutputPort)
	{
		const auto error = AXOBUS_SetOutput(nOutputPort, m_C300nOutputSpeed);
		if (error != 0)
			return error;
	}

	DWORD dw_written;
	OVERLAPPED os_write = {0};
	const int nbytes = strlen(lpszCmdString);
	m_CommTimeOuts.WriteTotalTimeoutMultiplier = fDelay;
	m_CommTimeOuts.WriteTotalTimeoutConstant = fDelay;
	SetCommTimeouts(m_hComm, &m_CommTimeOuts);

	WriteFile(m_hComm, lpszCmdString, nbytes, &dw_written, &os_write);
	return GetLastError();
}

int CyberAmp::AXOBUS_ReceiveString(char* lpszCmdString, int nWaitOK, DWORD fDelay)
{
	DWORD dw_number_of_bytes_read;
	constexpr auto dw_number_of_bytes_to_read = 256; //RCVBUFSIZE; //strlen(lpszCmdString);
	auto delay = fDelay;
	if (!nWaitOK)
		delay = 0;

	auto fsuccess = GetCommTimeouts(m_hComm, &m_CommTimeOuts);
	if (!fsuccess)
	{
		return GetLastError();
	}
	m_CommTimeOuts.ReadIntervalTimeout = delay; // 2
	m_CommTimeOuts.ReadTotalTimeoutMultiplier = delay;
	m_CommTimeOuts.ReadTotalTimeoutConstant = delay;
	SetCommTimeouts(m_hComm, &m_CommTimeOuts);
	fsuccess = GetCommTimeouts(m_hComm, &m_CommTimeOuts);

	const auto b_success = ReadFile(m_hComm, lpszCmdString, dw_number_of_bytes_to_read, &dw_number_of_bytes_read,
	                                nullptr);
	if (b_success)
	{
		const auto pchar = &lpszCmdString[dw_number_of_bytes_read];
		*pchar = 0;
	}

	//// force upper bit to zero here (pb: sometimes, the end char is -13)
	//if (bSuccess)
	//{
	//	char* pchar = &lpszCmdString[0];
	//	for (DWORD dwi=0; dwi < dwNumberOfBytesRead ; dwi++)
	//	{
	//		*pchar = *pchar & 0x8F;
	//		pchar++;
	//	}
	//}

	return GetLastError();
}

void CyberAmp::AXOBUS_FlushReceiveBuffer(int nOutputPort, DWORD fDelay)
{
	constexpr DWORD flag = PURGE_RXABORT | PURGE_RXCLEAR;
	//PURGE_TXABORT Terminates all outstanding overlapped write operations and returns immediately, even if the write operations have not been completed.
	//PURGE_RXABORT Terminates all outstanding overlapped read operations and returns immediately, even if the read operations have not been completed.
	//PURGE_TXCLEAR Clears the output buffer (if the device driver has one).
	//PURGE_RXCLEAR Clears the input buffer (if the device driver has one).
	PurgeComm(m_hComm, flag);
}
