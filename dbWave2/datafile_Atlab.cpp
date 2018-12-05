// AtlabFile.cpp : implementation file
//

#include "stdafx.h"
#include "dataheader_Atlab.h"
#include "datafile_Atlab.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CDataFileATLAB

IMPLEMENT_DYNCREATE(CDataFileATLAB, CDataFileX)

CDataFileATLAB::CDataFileATLAB()
{
	m_bHeaderSize = 1024;
	m_ulOffsetData = m_bHeaderSize;
	m_ulOffsetHeader = 0;
	m_idType = DOCTYPE_ATLAB;
	m_csType= _T("ATLAB");
	m_pWFormat = nullptr;
}

CDataFileATLAB::~CDataFileATLAB()
{
}


/////////////////////////////////////////////////////////////////////////////
// CDataFileATLAB diagnostics

#ifdef _DEBUG
void CDataFileATLAB::AssertValid() const
{
	CDataFileX::AssertValid();
}

void CDataFileATLAB::Dump(CDumpContext& dc) const
{
	CDataFileX::Dump(dc);
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CDataFileATLAB commands

BOOL CDataFileATLAB::ReadDataInfos(CWaveFormat* pWFormat, CWaveChanArray* pArray)
{
	// Read file header
	BOOL bflag = TRUE;
	m_pWFormat = pWFormat;
	m_pArray = pArray;

	char* pHeader = new (char[m_bHeaderSize]);
	ASSERT(pHeader != NULL);
	Seek(m_ulOffsetHeader, CFile::begin);	// position pointer
	Read(pHeader, m_bHeaderSize);			// read header

	// get A/D card type		
	char* pchar = pHeader + DEVID;
	short devid = *( (short*) pchar);
	switch (devid)
	{
		case 1:		pWFormat->csADcardName= _T("DT2828");		break;
		case 2:		pWFormat->csADcardName= _T("DT2827");		break;
		case 4:		pWFormat->csADcardName= _T("DT2821_F_DI");	break;
		case 8:		pWFormat->csADcardName= _T("DT2821_F_SE");	break;
		case 16:	pWFormat->csADcardName= _T("DT2821");		break;
		case 32:	pWFormat->csADcardName= _T("DT2821_G_DI");	break;
		case 64:	pWFormat->csADcardName= _T("DT2821_G_SE");	break;
		case 128:	pWFormat->csADcardName= _T("DT2823");		break;
		case 256:	pWFormat->csADcardName= _T("DT2825");		break;
		case 512:	pWFormat->csADcardName= _T("DT2824");		break;
		default:	pWFormat->csADcardName= _T("DT_OTHER");		break;
	}

	// A/D acquisition mode -- not implemented
	//unsigned short devflags = (short) *(pHeader+DEVFLAGS);

	// number of data acquisition channels	
	pchar = pHeader + SCNCNT;
	pWFormat->scan_count = *((short*)pchar);	

	// check if file is not corrupted
	pchar = pHeader + SAMCNT;
	long* plong = (long*) pchar;
	long len1 = (long) (GetLength() - m_bHeaderSize)/2;	
	if (len1 != *plong)
	{
		*plong=len1;
		SeekToBegin();
		Write(pHeader, m_bHeaderSize);
		bflag = 2;			
	}
	pWFormat->sample_count = *plong;

	// decode ATLAB header and fill in windows structure
	// ---------------- specific DT2821 differential mode
	pWFormat->fullscale_Volts	= 20.f;	// 20 mv full scale (+10 V to -10 V)
	pWFormat->binspan	= 4096;	// 12 bits resolution
	pWFormat->binzero			= 2048;			// offset binary encoding
	
	// 1) version independent parameters
	//pWFormat->wversion = 1;			// initial version

	// get device flags & encoding	
	//DEV_FLAGS devflags = (DEV_FLAGS)((short) *(pHeader+DEVFLAGS));

	// load data acquisition comment
	char* pcomment = pHeader + ACQCOM;					// get pointer to acqcom
	int i=0;
	for (i=0; i<ACQCOM_LEN; i++)						// loop over acqcom (80 chars)
		if (*(pcomment+i) == 0) 
			*(pcomment+i) = ' ';	// erase zeros
	pcomment = pHeader + ACQCOM;						// restore pointer
	CStringA ATLcomment = CStringA(pcomment, ACQCOM_LEN);	// load comment into string
	pWFormat->csComment = ATLcomment;
	pWFormat->csComment.TrimRight();

	// assume that comment is standardized and has the following fields
	pWFormat->csStimulus		= ATLcomment.Mid( 0, 20);
	pWFormat->csStimulus.TrimRight();
	pWFormat->csConcentration	= ATLcomment.Mid(20, 10);
	pWFormat->csConcentration.TrimRight();
	pWFormat->csSensillum		= ATLcomment.Mid(30, 10);
	pWFormat->csSensillum.TrimRight();
	pWFormat->csComment.Empty();

	// convert acquisition time stamp (date & time) into a CTime variable	
	char cdummy;
	char* pfirst = pHeader + ACQDATE;
	char* plast = pfirst+2; cdummy = *plast; *plast = 0;
	short month = atoi(pfirst);	*plast = cdummy; pfirst = plast+1; plast = pfirst+2; cdummy = *plast; *plast=0;
	short day = atoi(pfirst);	*plast = cdummy; pfirst = plast+1; plast = pfirst+2; cdummy = *plast; *plast=0;
	short year = 1900 + atoi(pfirst); *plast = cdummy;
	
	// convert data acquisition time
	pfirst = pHeader +ACQTIME; 
	plast = pfirst+2; cdummy = *plast; *plast=0;
	short hour = atoi(pfirst); *plast = cdummy; pfirst = plast+1; plast = pfirst+2; cdummy = *plast; *plast=0;
	short min = atoi(pfirst); *plast = cdummy; pfirst = plast+1; plast = pfirst+2; cdummy = *plast; *plast=0;
	short sec = atoi(pfirst); *plast = cdummy;
	pWFormat->acqtime = CTime(year, month, day, hour, min, sec);
	
	// clock period, sampling rate/chan and file duration	
	pchar = pHeader+ CLKPER; plong = (long*) pchar;	
	float clock_rate = 4.0E6f/ ((float)*plong);
	pWFormat->chrate = clock_rate / pWFormat->scan_count;
	pWFormat->duration =  pWFormat->sample_count / clock_rate;

	// fill channel description with minimal infos	
	pchar = pHeader + CHANLST; short* pchanlist = (short*) pchar;
	pchar = pHeader + GAINLST; short* pgainlist = (short*) pchar;
	char*  pcomtlist = pHeader + CHANCOM;
	pchar = pHeader + XGAIN;
	float* pxgainlist= (float*) pchar;
	for (i = 0; i< pWFormat->scan_count; i++)
	{
		pArray->ChannelAdd();
		CWaveChan* pChan = (CWaveChan*) pArray->GetWaveChan(i);
		pChan->am_adchannel = *pchanlist; pchanlist++;	// acq chan
		pChan->am_gainAD = *pgainlist; pgainlist++;		// gain on the A/D card
		short j = (short) strlen(pcomtlist);
		if (j > 40 || j <0)
			j = 40;
		pChan->am_csComment = CStringA(pcomtlist, j);			// chan comment
		pChan->am_gainamplifier = *pxgainlist;				// total gain (ampli + A/D card)
		pChan->am_gaintotal = pChan->am_gainamplifier *((float)pChan->am_gainAD);
		// TODO: check if resolution is computed correctly
		//pChan->am_resolutionV = 20. / (double) (pWFormat->binspan)  / pChan->am_gaintotal;
		pChan->am_resolutionV = pWFormat->fullscale_Volts / pChan->am_gaintotal / pWFormat->binspan;
		pgainlist++;		
	}

	// init as if no amplifier were present
	for (i = 0; i < pWFormat->scan_count; i++)
		InitDummyChansInfo(i);

	// 2) version dependent parameters
	pchar = pHeader + VERSION;
	short version = *((short*) pchar);
	pchar = pHeader+ CYBERA320;
	CYBERAMP* pcyberA320= (CYBERAMP*) pchar;
	short* pshort;	
	switch (version)
	{
		case 0:	// version 0: transfer parms from xgain && cyber to channel_instrm
			//AfxMessageBox("Atlab header version 0", MB_OK);

			if (pcyberA320->ComSpeed == 0)	// cyberAmp??
				break;
			InitChansFromCyberA320(pHeader, version);
			pWFormat->trig_mode = 0; //OLx_TRG_SOFT;
			pchar = pHeader+ TIMING;
			pshort = (short*) pchar;
			if (*pshort & EXTERNAL_TRIGGER)
				pWFormat->trig_mode = 1; //OLx_TRG_EXTERN;
			bflag = 2;
			break;

		case 1:	// version 1: trig parameters explicitely set			
			if (pcyberA320->ComSpeed == 0)	// cyberAmp??
				break;
			InitChansFromCyberA320(pHeader, version);
			pchar = pHeader + TRIGGER_MODE; pshort = (short*) pchar;
			pWFormat->trig_mode = *pshort;
			pchar = pHeader + TRIGGER_CHAN; pshort = (short*) pchar;
			pWFormat->trig_chan = *pshort;
			pchar = pHeader + TRIGGER_THRESHOLD; pshort = (short*) pchar;
			pWFormat->trig_threshold = *pshort;			
			break;

		default: // version before version 0
			pWFormat->trig_mode = 0; //OLx_TRG_SOFT;
			pchar = pHeader+ TIMING;
			pshort = (short*) pchar;
			if (*pshort & EXTERNAL_TRIGGER)
				pWFormat->trig_mode = 1; //OLx_TRG_EXTERN;
			break;
	}

	// init additional parameters
	delete [] pHeader;
	return bflag;
}

void CDataFileATLAB::InitChansFromCyberA320(char *pHeader, short version)
{
	char* pchar= pHeader+CYBER_1;
	CYBERCHAN* pcyber1 = (CYBERCHAN*) pchar;
	pchar= pHeader+CYBER_2;
	CYBERCHAN* pcyber2 = (CYBERCHAN*) pchar;

	// ATLAB(fred) version 0 did not set chan_cyber properly...	
	// if cyber_chanx equal on both channels, assume that parameters are not correctly set
	// look for the first xgain that equals the Cyber gain
	if ((pcyber1->acqchan == pcyber2->acqchan) 
		&& m_pWFormat->scan_count >1)
	{
		AfxMessageBox(_T("cyber chan not correctly set"), MB_OK);
	}

	// default cyberamp description	
	else
	{		
		if (0 <= pcyber1->acqchan && pcyber1->acqchan <= m_pWFormat->scan_count)
		{
			short chan = pcyber1->acqchan-1;
			if (chan < 0) chan = 0;
			LoadChanFromCyber(chan, (char*) pcyber1);
			if ((short)1 == m_pWFormat->scan_count)
				pcyber2->acqchan= (unsigned char)255;
		}
		if (0 <= pcyber2->acqchan && pcyber2->acqchan <= m_pWFormat->scan_count)
		{
			short chan = pcyber2->acqchan-1;
			if (chan <0)
				chan = 0;
			LoadChanFromCyber(chan, (char*) pcyber2);
		}
	}
}

void CDataFileATLAB::LoadChanFromCyber(short i, char* pcyberchan)
{
	CYBERCHAN* pcyb = (CYBERCHAN*) pcyberchan;
	// special case if probe == "none"	: exit
	CStringA probe = CStringA(&(pcyb->probe[0]), 8);	
	if ((  probe.CompareNoCase("none    ") == 0)
	   || (probe.CompareNoCase("0       ") == 0))
		return;		
	CWaveChan* pChan = (CWaveChan*) m_pArray->GetWaveChan(i);
	pChan->am_csheadstage		= probe;
	pChan->am_gainheadstage	= pcyb->gainprobe;	
	pChan->am_csamplifier	= CString (_T("CyberAmp"));
	pChan->am_amplifierchan		= pcyb->acqchan;
	pChan->am_gainpre	= pcyb->gainpre; 	// pre-filter amplification
	pChan->am_gainpost	= pcyb->gainpost;	// post-filter amplification
	pChan->am_notchfilt	= pcyb->notchfilt;// notch filter ON/off
	pChan->am_lowpass	= pcyb->lowpass;	// low pass filter 0=DC 4....30000
	pChan->am_offset	= pcyb->offset;		// input offset
	pChan->am_csInputpos= GetCyberA320filter(pcyb->inputpos);
	pChan->am_csInputneg= GetCyberA320filter(pcyb->inputneg);
	pChan->am_gainamplifier = 1.;
	pChan->am_gaintotal = (double) pcyb->gainpre * (double) pcyb->gainpost * (double) pcyb->gainprobe * (double) pChan->am_gainAD;
}


// assume that code is filter cutoff * 10

CString CDataFileATLAB::GetCyberA320filter(int ncode)
{
	CString csCoupling;
	switch (ncode)
	{
	case -10:	csCoupling= _T("GND");break;
	case 0:		csCoupling= _T("DC" ); break;		// DC
	case 1:		csCoupling= _T("0.1"); break;		// 0.1 Hz
	case 10:	csCoupling= _T("1" ); break;		// 1 Hz
	case 100:	csCoupling= _T("10"); break;		// 10 Hz
	case 300:	csCoupling= _T("30"); break;		// 30 Hz
	case 1000:	csCoupling= _T("100"); break;		// 100 Hz
	case 3000:	csCoupling= _T("300"); break;		// 300 Hz
	default:	csCoupling= _T("undefined"); break;
	}
	return csCoupling;
}

void CDataFileATLAB::InitDummyChansInfo(short chanlistindex)
{
	CWaveChan* pChan = (CWaveChan*) m_pArray->GetWaveChan(chanlistindex);
	pChan->am_csamplifier = CStringA("Unknown");
	pChan->am_csheadstage = pChan->am_csamplifier;	
	pChan->am_gainheadstage =1;
	pChan->am_amplifierchan = 1;
	pChan->am_gainpre =1;	// pre-filter amplification
	pChan->am_gainpost=1;	// post-filter amplification
	pChan->am_notchfilt=0;	// notch filter ON/off
	pChan->am_lowpass=0;	// low pass filter 0=DC 4....30000
	pChan->am_offset=0.0f;	// input offset
	pChan->am_csInputpos= CStringA( "DC");
	pChan->am_csInputneg= CStringA( "GND");
}


BOOL CDataFileATLAB::CheckFileType(CFile* f, int bfileoffset)
{
	// ------lit octets testes pour determiner type de fichier
	WORD wAtlab;			// struct for ATLab file
	BOOL flag = DOCTYPE_UNKNOWN;	
	f->Seek(m_ulOffsetHeader, CFile::begin);	// position pointer to start of file
	f->Read(&wAtlab, sizeof(wAtlab));		// Read data

	// test Atlab 
	if (wAtlab == 0xAAAA) //	//&&( tab[2] == 0x07 || tab[2] == 0x06)	    	
		flag = m_idType;
	return flag;
}