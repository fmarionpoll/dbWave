// CyberAmp.cpp : implementation file
//

#include "StdAfx.h"
#include "CyberAmp.h"

#include "AcqWaveChan.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CCyberAmp

#define szLEN 100

char CCyberAmp::m_C300szCommands[];
char CCyberAmp::m_C300szRcvText[];
int CCyberAmp::m_C300nLastError;
int CCyberAmp::m_C300nDevNumber;
int CCyberAmp::m_C300nOutputPort;
int CCyberAmp::m_C300nOutputSpeed;
int CCyberAmp::m_C300nDebugVersion;
DWORD CCyberAmp::m_C300fReceiveDelay;

CCyberAmp::CCyberAmp()
{
	C300_ResetParms();
	m_hComm = nullptr;
}

void CCyberAmp::C300_ResetParms()
{
	m_C300nLastError = C300_SUCCESS;
	m_C300nOutputPort = ABUS_CLOSED;
	m_C300nOutputSpeed = ABUS_SPEEDDEFAULT;
	m_C300nDevNumber = C300_DEVICENULL;
	m_C300nDebugVersion = FALSE;
	m_C300fReceiveDelay = DEFAULTRCVDELAY;
}

CCyberAmp::~CCyberAmp()
{
	if (m_hComm != nullptr)
		CloseHandle(m_hComm);
}

// Do not edit the following lines, which are needed by ClassWizard.
#if 0
BEGIN_MESSAGE_MAP(CCyberAmp, CFile)

END_MESSAGE_MAP()
#endif	// 0

/////////////////////////////////////////////////////////////////////////////
// CCyberAmp member functions

int CCyberAmp::Initialize()
{
	C300_ResetParms();
	if (m_hComm != nullptr)
		CloseHandle(m_hComm);
	m_hComm = nullptr;

	m_C300nLastError = ABUS_Initialize();

	return (m_C300nLastError);
}

int CCyberAmp::C300_SetOutputPortAndSpeed(int nWhichPort, int nWhichSpeed)
{
	// Clear the last error flag
	m_C300nLastError = C300_SUCCESS;
	C300_INT_TranslateABUSCOMSettings(nWhichPort, nWhichSpeed,
	                                  &m_C300nOutputPort, &m_C300nOutputSpeed);
	m_C300nLastError = ABUS_SetOutput(m_C300nOutputPort, m_C300nOutputSpeed);
	if (m_C300nLastError)
		m_C300nLastError = C300_INT_TranslateABUSError(m_C300nLastError);

	// Return the most recent error flag
	return (m_C300nLastError);
}

int CCyberAmp::C300_INT_TranslateABUSError(int ABUSError)
{
	switch (ABUSError)
	{
	case ABUS_SUCCESS: m_C300nLastError = C300_SUCCESS;
		break;
	case ABUS_BADDEVICE: m_C300nLastError = C300_BADDEVICE;
		break;
	case ABUS_BADSERIAL: m_C300nLastError = C300_BADSERIAL;
		break;
	case ABUS_XMITTIMEOUT: m_C300nLastError = C300_XMITTIMEOUT;
		break;
	case ABUS_RCVTIMEOUT: m_C300nLastError = C300_RCVTIMEOUT;
		break;
	case ABUS_RCVBUFOVERFLOW: m_C300nLastError = C300_RCVBUFOVERFLOW;
		break;
	case ABUS_RCVOVERRUN: m_C300nLastError = C300_RCVOVERRUN;
		break;
	default: m_C300nLastError = C300_BADABUSERROR;
		break;
	}

	// Return the most recent error flag
	return (m_C300nLastError);
}

int CCyberAmp::C300_INT_TranslateABUSCOMSettings(int nWhichPort, int nWhichSpeed,
                                                 int* pnOutputPort, int* pnOutputSpeed)
{
	// Clear the last error flag
	m_C300nLastError = C300_SUCCESS;

	switch (nWhichPort)
	{
	case C300_COM1: *pnOutputPort = ABUS_COM1;
		break;
	case C300_COM2: *pnOutputPort = ABUS_COM2;
		break;
	case C300_COM3: *pnOutputPort = ABUS_COM3;
		break;
	case C300_COM4: *pnOutputPort = ABUS_COM4;
		break;
	default: *pnOutputPort = ABUS_CLOSED;
		break;
	}

	switch (nWhichSpeed)
	{
	case C300_SPEED19200: *pnOutputSpeed = ABUS_SPEED19200;
		break;
	case C300_SPEED9600: *pnOutputSpeed = ABUS_SPEED9600;
		break;
	case C300_SPEED4800: *pnOutputSpeed = ABUS_SPEED4800;
		break;
	case C300_SPEED2400: *pnOutputSpeed = ABUS_SPEED2400;
		break;
	case C300_SPEED1200: *pnOutputSpeed = ABUS_SPEED1200;
		break;
	case C300_SPEED600: *pnOutputSpeed = ABUS_SPEED600;
		break;
	case C300_SPEED300: *pnOutputSpeed = ABUS_SPEED300;
		break;
	case C300_SPEEDDEFAULT: *pnOutputSpeed = ABUS_SPEEDDEFAULT;
		break;
	default: *pnOutputSpeed = ABUS_SPEEDDEFAULT;
		break;
	}

	// Return the most recent error flag
	return (m_C300nLastError);
}

//  Internal routine to add the necessary command prefix to the   command string

void CCyberAmp::C300_INT_StartCommand()
{
	char sz_buffer[20];
	strcat_s(m_C300szCommands, 4 * MAXCMDLEN, COMMANDPREFIX);

	// If AXOBUS device numbers are in use, specify which device
	if (m_C300nDevNumber >= 0)
	{
		sprintf_s(sz_buffer, 19, "%d", m_C300nDevNumber);
		strcat_s(m_C300szCommands, 4 * MAXCMDLEN, sz_buffer);
	}
}

//  Internal routine to append a command sequence to the current  command string

void CCyberAmp::C300_INT_AddCommand(char* lpszCommandText)
{
	// If this is the start of a new command, then flush the
	//     hardware receive buffer and apply the necessary
	//     command prefix
	if (strlen(m_C300szCommands) == 0)
	{
		ABUS_FlushReceiveBuffer(m_C300nOutputPort, m_C300fReceiveDelay);
		C300_INT_StartCommand();
	}

	// Look for the last portion of the command string
	auto lpsz_end_of_command = strrchr(m_C300szCommands, ENDCOMMAND);
	if (lpsz_end_of_command != nullptr)
		++lpsz_end_of_command;
	else
		lpsz_end_of_command = m_C300szCommands;

	// If this command would overflow the maximum command length,
	//     start a new portion
	if (strlen(lpsz_end_of_command) + strlen(lpszCommandText) > MAXCMDLEN)
	{
		// Terminate the current command portion and start a new
		//    portion
		C300_StringConcatChar(m_C300szCommands, ENDCOMMAND);
		C300_INT_StartCommand();
	}
	strcat_s(m_C300szCommands, 4 * MAXCMDLEN, lpszCommandText);
}

//  Flushes any pending command portions one at a time and awaits the response from the CyberAmp
int CCyberAmp::C300_FlushCommandsAndAwaitResponse()
{
	char* lpszEndCommand;

	char sz_text[szLEN + 1];
	char szRestOfCommand[szLEN + 1];

	// Clear the last error flag
	m_C300nLastError = C300_SUCCESS;
	if (strlen(m_C300szCommands) > 0)
	{
		do
		{
			if ((lpszEndCommand = strchr(m_C300szCommands, ENDCOMMAND))
				!= nullptr)
			{
				// copy the first half of this string into sz_text
				//     and terminate it
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
			if (ABUS_SetOutput(m_C300nOutputPort, m_C300nOutputSpeed) == ABUS_SUCCESS)
			{
				if (ABUS_SendString(m_C300nOutputPort, &sz_text[0], m_C300fReceiveDelay) == ABUS_SUCCESS)
				{
					if (ABUS_ReceiveString(&m_C300szRcvText[0], TRUE, m_C300fReceiveDelay) == ABUS_SUCCESS)
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
		CString csError;
		csError.Format(
			_T("Error returned by CyberAmp= %i\n when issuing command:\n%hs\noutput port: %i\noutput speed: %d\n"),
			m_C300nLastError, sz_text, m_C300nOutputPort, m_C300nOutputSpeed
		);
		AfxMessageBox(csError, MB_OK);
	}

	return (m_C300nLastError);
}

//  Returns the status text for a single channel

int CCyberAmp::C300_GetChannelStatus(int nChannel, char* lpszStatusText)
{
	char sz_text[szLEN];
	if (C300_FlushCommandsAndAwaitResponse() == C300_SUCCESS)
	{
		// Output the channel status command and flush it out
		sprintf_s(sz_text, 99, "S%d", nChannel);
		C300_INT_AddCommand(sz_text);
		if (C300_FlushCommandsAndAwaitResponse() == C300_SUCCESS)
			C300_GetLastReception(lpszStatusText);
	}
	// Return the most recent error flag
	return (m_C300nLastError);
}

//  Set the AXOBUS device number of the CyberAmp

int CCyberAmp::C300_SetDeviceNumber(int nDevNum)
{
	// Check the device number for validity
	if ((nDevNum < 0 || nDevNum > 9) && nDevNum != C300_DEVICENULL)
		m_C300nLastError = C300_BADDEVICENUMBER;
	else
	{
		m_C300nLastError = C300_SUCCESS;

		// If we are changing the current device number, flush any
		//    pending commands
		if (m_C300nDevNumber != nDevNum)
			C300_FlushCommandsAndAwaitResponse();
		m_C300nDevNumber = nDevNum;
	}
	// Return the most recent error flag
	return (m_C300nLastError);
}

//   Set the gain for a given channel

int CCyberAmp::SetGain(int nChannel, int nGainValue)
{
	char sz_text[szLEN];
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
		sprintf_s(sz_text, 99, "G%dP%dG%dO%d", nChannel, nPreAmp, nChannel,
		          nGainValue / nGainDivisor);
		C300_INT_AddCommand(sz_text);
	}

	// Return the most recent error flag
	return (m_C300nLastError);
}

//   Set the pre-amp and output gains

int CCyberAmp::C300_SetAmpGains(int nChannel, int nPreAmpGain, int nOutputGain)
{
	char szPreAmp[20], szOutput[20];
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

	// Return the most recent error flag
	return (m_C300nLastError);
}

//   Set the input coupling for a given channel

int CCyberAmp::SetHPFilter(int nChannel, int nInput, const CString& cs_coupling)
{
	char sz_text[szLEN];

	// Clear the last error flag
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

	// Return the most recent error flag
	return (m_C300nLastError);
}

int CCyberAmp::SetWaveChanParms(CWaveChan* pchan)
{
	// chan, gain, filter +, lowpass, notch
	SetHPFilter(pchan->am_amplifierchan, C300_POSINPUT, pchan->am_csInputpos);
	SetHPFilter(pchan->am_amplifierchan, C300_NEGINPUT, pchan->am_csInputneg);
	SetmVOffset(pchan->am_amplifierchan, pchan->am_offset);
	SetNotchFilter(pchan->am_amplifierchan, pchan->am_notchfilt);
	const auto gain = static_cast<double>(pchan->am_gaintotal) / (static_cast<double>(pchan->am_gainheadstage) *
		static_cast<double>(pchan->am_gainAD));
	SetGain(pchan->am_amplifierchan, static_cast<int>(gain));
	SetLPFilter(pchan->am_amplifierchan, static_cast<int>(pchan->am_lowpass));
	return C300_FlushCommandsAndAwaitResponse();
}

int CCyberAmp::GetWaveChanParms(CWaveChan* pchan)
{
	// chan, gain, filter +, lowpass, notch
	//cyberAmp.SetHPFilter(pchan->am_amplifierchan, C300_POSINPUT, pchan->am_csInputpos);
	//cyberAmp.SetHPFilter(pchan->am_amplifierchan, C300_NEGINPUT, pszHighPass[0]);
	//cyberAmp.SetmVOffset(pchan->am_amplifierchan, pchan->am_offset);
	//cyberAmp.SetNotchFilter(pchan->am_amplifierchan, pchan->am_notchfilt);
	//double gain = pchan->am_gaintotal / (pchan->am_gainheadstage*pchan->am_gainAD);
	//cyberAmp.SetGain(pchan->am_amplifierchan, (int)gain);
	//cyberAmp.SetLPFilter(pchan->am_amplifierchan, (int)(pchan->am_lowpass));
	//int errorcode = cyberAmp.C300_FlushCommandsAndAwaitResponse();
	return 0;
}

//  Set the input offset for a given channel (in mV)

int CCyberAmp::SetmVOffset(int nChannel, float fOffset)
{
	char sz_text[szLEN];

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

	// Return the most recent error flag
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

int CCyberAmp::SetLPFilter(int nChannel, int nFilterValue)
{
	char sz_text[szLEN];
	char sz_buffer[20];

	// Clear the last error flag
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
		sprintf_s(sz_buffer, 19, "%d", nFilterValue);
		strcat_s(sz_text, 99, sz_buffer);
		C300_INT_AddCommand(sz_text);
	}
	else
		m_C300nLastError = C300_BADVALUE;

	// Return the most recent error flag
	return (m_C300nLastError);
}

//  Set the notch filter ON or OFF for a given channel

int CCyberAmp::SetNotchFilter(int nChannel, int nEnabled)
{
	char sz_text[szLEN];
	// Clear the last error flag
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

	// Return the most recent error flag
	return (m_C300nLastError);
}

//  Zero the DC offset for a given channel.  Returns the actual offset (in mV)
int CCyberAmp::C300_ZeroDCOffset(int nChannel, float* pfOffsetReturned)
{
	char* lpsz_offset;
	char sz_text[szLEN];
	m_C300nLastError = C300_SUCCESS; // Clear the last error flag
	// Check the channel number for validity
	if (nChannel < C300_MINCHANNEL || nChannel > C300_MAXCHANNEL)
		return (m_C300nLastError = C300_BADCHANNEL);

	// Flush out any pending command string
	if (C300_FlushCommandsAndAwaitResponse() == C300_SUCCESS)
	{
		// Build the zero command and the channel number
		sprintf_s(sz_text, 99, "Z%d", nChannel);

		// Append the command to the command string and flush it out
		C300_INT_AddCommand(sz_text);
		if (C300_FlushCommandsAndAwaitResponse() == C300_SUCCESS)
		{
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

	// Return the most recent error flag
	return (m_C300nLastError);
}

//  Reload the factory defaults  for all settings

int CCyberAmp::C300_LoadFactoryDefaults(void)
{
	// Clear the last error flag
	m_C300nLastError = C300_SUCCESS;

	// Output the initialization command
	C300_INT_AddCommand("L");

	// Return the most recent error flag
	return (m_C300nLastError);
}

//  CyberAmp to save the current state in the EEPROM

int CCyberAmp::C300_SaveCurrentState(void)
{
	// Output the write EEPROM command
	C300_INT_AddCommand("W");

	// Return the value returned from C300_FlushCommands()
	return (C300_FlushCommandsAndAwaitResponse());
}

//  Enter or exit the electrode test mode

int CCyberAmp::C300_ElectrodeTest(int nEnabled)
{
	// Clear the last error flag
	m_C300nLastError = C300_SUCCESS;

	// Output the appropriate electrode test command
	if (nEnabled == C300_DISABLED)
		C300_INT_AddCommand("TO-");
	else
		C300_INT_AddCommand("TO+");

	// Return the most recent error flag
	return (m_C300nLastError);
}

//  Set the receive timeout value

int CCyberAmp::C300_SetReceiveTimeout(DWORD fTimeoutVal)
{
	// Check the timeout value for a return to the default
	if (fTimeoutVal > 0.0)
		m_C300fReceiveDelay = fTimeoutVal;
	else
		m_C300fReceiveDelay = DEFAULTRCVDELAY;

	// Return the success state
	return (m_C300nLastError = C300_SUCCESS);
}

//  Return the overload status of all channels

int CCyberAmp::C300_GetOverloadStatus(int* lpnChannelMask)
{
	char* lpsz_temp;
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
	// Return the most recent error flag
	return (m_C300nLastError);
}

//  Return the last error generated by a call to the CyberAmp 300 support module

int CCyberAmp::C300_GetLastError(void)
{
	// Return the most recent error flag
	return (m_C300nLastError);
}

//  Return the raw text from the last communication with theCyberAmp

int CCyberAmp::C300_GetLastReception(char* lpszResultText)
{
	auto p_charend = strchr(m_C300szRcvText, ENDCOMMAND);
	if (p_charend != nullptr)
		*(p_charend + 1) = '\0';
	else
	{
		p_charend = strchr(m_C300szRcvText, 0);
		const auto len_rcv = strlen(m_C300szRcvText);
		const size_t len_res = RCVBUFSIZE;
		if (len_rcv > len_res)
		{
			p_charend = &m_C300szRcvText[0] + len_res - 1;
			*(p_charend + 1) = '\0';
		}
	}

	const size_t ilen = RCVBUFSIZE;
	strcpy_s(lpszResultText, ilen, m_C300szRcvText);
	return (m_C300nLastError = C300_SUCCESS);
}

//  Return the library version number as a string

int CCyberAmp::C300_GetLibraryVersion(char* sz_txt)
{
	strcpy_s(sz_txt, strlen(sz_txt), VERSIONCYB);
	return (m_C300nLastError = C300_SUCCESS);
}

//  Internal routine to concatenate a single char to a string

void CCyberAmp::C300_StringConcatChar(char* lpsz_string, int c)
{
	int n_len = strlen(lpsz_string);
	lpsz_string[n_len++] = static_cast<char>(c);
	lpsz_string[n_len] = '\0';
}

//  Internal routine which finds a match using a binary search

int CCyberAmp::C300_FoundListMatch(int nFilterValue, int* lpn_list, const int nListItems)
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

int CCyberAmp::ABUS_Initialize()
{
	const auto iCOM4 = 4;
	char sz_status_text[RCVBUFSIZE + 1]; //512];
	sz_status_text[0] = '\0';
	m_C300fReceiveDelay = 2;

	for (auto i = 1; i <= iCOM4; i++)
	{
		const auto error = ABUS_SetOutput(i, 9600);
		if (error == 0 || m_hComm != INVALID_HANDLE_VALUE)
		{
			// get status of the amplifier
			if (C300_FlushCommandsAndAwaitResponse() != C300_SUCCESS)
				return (m_C300nLastError);

			// Output the general status command and flush it out
			C300_INT_AddCommand("S0");
			if (C300_FlushCommandsAndAwaitResponse() == C300_SUCCESS)
				C300_GetLastReception(sz_status_text);

			const int ilen = strlen(sz_status_text);
			if (ilen == 0)
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

					const int ilen2 = strlen(sz_status_text);
					if (ilen2 == 0)
						m_C300nLastError = C300_BADDEVICE;

					// Return the most recent error flag
					return (m_C300nLastError);
				}
				return C300_SUCCESS;
			}
		}
	}
	return C300_BADDEVICE;
}

int CCyberAmp::ABUS_SetOutput(int nWhichPort, int nWhichSpeed)
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
	case ABUS_COM1: cs_comchan = "COM1";
		break;
	case ABUS_COM2: cs_comchan = "COM2";
		break;
	case ABUS_COM3: cs_comchan = "COM3";
		break;
	case ABUS_COM4: cs_comchan = "COM4";
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
	/*fsuccess = SetupComm(m_hComm, RCVBUFSIZE, RCVBUFSIZE);
	if (!fsuccess)
		return GetLastError();*/

	fsuccess = GetCommTimeouts(m_hComm, &m_CommTimeOuts);
	if (!fsuccess)
		return GetLastError();

	return 0;
}

int CCyberAmp::ABUS_SendString(int nOutputPort, char* lpszCmdString, DWORD fDelay)
{
	if (nOutputPort != m_C300nOutputPort)
	{
		const auto error = ABUS_SetOutput(nOutputPort, m_C300nOutputSpeed);
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

int CCyberAmp::ABUS_ReceiveString(char* lpszCmdString, int nWaitOK, DWORD fDelay)
{
	DWORD dw_number_of_bytes_read;
	const auto dw_number_of_bytes_to_read = 256; //RCVBUFSIZE; //strlen(lpszCmdString);
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

void CCyberAmp::ABUS_FlushReceiveBuffer(int nOutputPort, DWORD fDelay)
{
	const DWORD flag = PURGE_RXABORT | PURGE_RXCLEAR;

	//PURGE_TXABORT Terminates all outstanding overlapped write operations and returns immediately, even if the write operations have not been completed.
	//PURGE_RXABORT Terminates all outstanding overlapped read operations and returns immediately, even if the read operations have not been completed.
	//PURGE_TXCLEAR Clears the output buffer (if the device driver has one).
	//PURGE_RXCLEAR Clears the input buffer (if the device driver has one).
	PurgeComm(m_hComm, flag);
}
