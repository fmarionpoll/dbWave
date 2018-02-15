// aqcparam.cpp    implementation file
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "dataheader_Atlab.h"
#include "acqparam.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//------------------ class CWaveFormat ---------------------------------

IMPLEMENT_SERIAL(CWaveFormat, CObject, 0 /* schema number*/ )


CWaveFormat::CWaveFormat()
{
	csADcardName	= _T("undefined");
	csComment		= _T("");		// file annotation
	csStimulus		= _T("");		// stimulation (chemical)
	csConcentration = _T("");		// stimulus concentration
	csStimulus2		= _T("");		// stimulation (chemical 2)
	csConcentration2= _T("");		// stimulus 2 concentration
	csInsectname 	= _T("");		// insect type
	csLocation		= _T("");		// sensillum location (organ)
	csSensillum		= _T("");		// type of sensillum
	csMoreComment	= _T("");		// additional free comment
	csOperator		= _T("");		// operator
	csStrain		= _T("");		// strain
	csSex			= _T("");		// sex
	acqtime = CTime::GetCurrentTime();

	wversion=8;						// header version number 
	fullscale_Volts=20.0f;			// volts full scale, gain 1	
	binspan=4096;			// nb of bins (4096)
	binzero=2048;
	mode_encoding = OLx_ENC_BINARY;
	mode_clock = INTERNAL_CLOCK;	// internal, external
	mode_trigger = INTERNAL_TRIGGER;// internal, external, trigg_scan

	chrate=10000.0f;				// channel sampling rate (Hz)
	scan_count=1;					// number of channels in scan list
	sample_count=10000;				// sample count
	duration=0.0f;					// file duration (s)
	trig_mode=0;					// soft, keyboard, trig_ext, input+, input-
	trig_chan=0;					// (trig_mode=input) A/D channel input
	trig_threshold=2048;			// (trig_mode=input) A/D channel input threshold value	
	data_flow=0;					// 0 = continuous, 1 = burst
	bOnlineDisplay=TRUE;			// TRUE = online data display
	bADwritetofile=FALSE;			// TRUE = write data to disk
	bufferNitems=10;				// n buffers declared
	buffersize=1000;				// total size of one buffer	
	insectID=0;
	sensillumID=0;
	repeat = 0;
	repeat2 = 0;
	bUnicode = TRUE;
}

CWaveFormat::~CWaveFormat()
{
}

void CWaveFormat::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << (WORD)9;				// 1
		ar << acqtime;				// 2
		ar << fullscale_Volts;		// 3
		ar << binspan;		// 4
		ar << binzero;				// 5
		ar << (WORD)mode_encoding;	// 6
		ar << (WORD)mode_clock;		// 7
		ar << (WORD)mode_trigger;	// 8
		ar << chrate;				// 9
		ar << (WORD)scan_count;		// 10
		ar << sample_count;
		ar << duration;
		ar << (WORD)trig_mode;
		ar << (WORD)trig_chan;
		ar << (WORD)trig_threshold;
		ar << (WORD)data_flow;
		ar << bOnlineDisplay;
		ar << bADwritetofile;
		ar << bufferNitems;
		ar << buffersize;

		int ncomments = 14;
		ar << ncomments;
		// save "CString"
		ar << csComment;
		ar << csStimulus;
		ar << csConcentration;
		ar << csInsectname;
		ar << csOperator;
		ar << csLocation;
		ar << csSensillum;
		ar << csStrain;
		ar << csMoreComment;
		ar << csADcardName;
		ar << csStimulus2;
		ar << csConcentration2;
		ar << csSex;

		int nitems = 4;
		ar << nitems;
		// save "long"
		ar << insectID;
		ar << sensillumID;
		ar << repeat;
		ar << repeat2;
	}
	else
	{
		WORD wversion; ar >> wversion;			// 1 version number
		ASSERT(wversion >= 3);					// assume higher version than 3
		if (wversion < 9)
			Read_v8andbefore(ar, wversion);
		else
		{
			WORD w;
			ar >> acqtime;						// 2
			ar >> fullscale_Volts;				// 3
			ar >> binspan;				// 4
			ar >> binzero;						// 5
			ar >> w; mode_encoding	= (short)w;	// 6         	    	
			ar >> w; mode_clock		= (short)w;	// 7    	    	
			ar >> w; mode_trigger	= (short)w;	// 8
			ar >> chrate;						// 9
			ar >> w; scan_count		= (short)w;	// 10
			ar >> sample_count;
			ar >> duration;
			ar >> w; trig_mode		= (short)w;
			ar >> w; trig_chan		= (short)w;
			ar >> w; trig_threshold = (short)w;
			ar >> w; data_flow		= (short)w;
			ar >> bOnlineDisplay;
			ar >> bADwritetofile;
			ar >> bufferNitems >> buffersize;

			int ncomments;
			ar >> ncomments; 
			ASSERT(ncomments == 14);
			ar >> csComment;
			ar >> csStimulus;
			ar >> csConcentration;
			ar >> csInsectname;
			ar >> csOperator;
			ar >> csLocation;
			ar >> csSensillum;
			ar >> csStrain;
			ar >> csMoreComment;
			ar >> csADcardName;
			ar >> csStimulus2;
			ar >> csConcentration2;
			ar >> csSex;

			int nitems;
			ar >> nitems;
			ASSERT(nitems == 4);
			ar >> insectID;
			ar >> sensillumID;
			ar >> repeat;
			ar >> repeat2;
		}
	}
}

void CWaveFormat::Read_v8andbefore(CArchive& ar, WORD version)
{
	ASSERT(!ar.IsStoring());
	WORD w;
	ar >> acqtime;            			// 2
	ar >> fullscale_Volts;           	// 3
	if (version < 6)
	{
		ar >> w;  binspan = w;	// 4    	
		ar >> w;  binzero = w;			// 5
	}
	else
	{
		ar >> binspan;			// 4    	
		ar >> binzero;					// 5
	}
	ar >> w ; mode_encoding	=(short)w;	// 6         	    	
	ar >> w ; mode_clock	=(short)w;		// 7    	    	
	ar >> w ; mode_trigger	=(short)w;	// 8
	ar >> chrate;						// 9
	ar >> w; scan_count		= (short)w;	// 10
	ar >> sample_count;
	ar >> duration;
	ar >> w; trig_mode		=(short)w;
	ar >> w; trig_chan		=(short)w;
	ar >> w; trig_threshold	=(short)w;
	ar >> w; data_flow		=(short)w;
	ar >> bOnlineDisplay;
	ar >> bADwritetofile;
	ar >> bufferNitems >> buffersize;

	if (version >= 7)
	{
		ar >> csComment;
		ar >> csStimulus;
		ar >> csConcentration;
		ar >> csInsectname;
		ar >> csOperator;
		ar >> csLocation;
		ar >> csSensillum;
		ar >> csStrain;
		ar >> csMoreComment;
		ar >> csADcardName;
		ar >> csStimulus2;
		ar >> csConcentration2;
		if (version == 8)
			ar >> csSex;
	}
	else
	{
		ar >> csComment;
		ar >> csStimulus;
		ar >> csConcentration;
		ar >> csInsectname;
		ar >> csOperator;
		ar >> csLocation;
		ar >> csSensillum;
		ar >> csStrain;
		ar >> csMoreComment;
		if (version >= 3)
		{
			ar >> csADcardName;
		}
	}

	if (version > 4)
	{
		int nitems; ar >> nitems;
		ar >> insectID; nitems--;
		ar >> sensillumID; nitems--;
		repeat = 0;
		repeat2 = 0;
		if (nitems > 0) ar >> repeat; nitems--;
		if (nitems > 0) ar >> repeat2; nitems--;
	}
	else
	{
		insectID = 0;
		sensillumID = 0;
	}
}


void CWaveFormat::operator = (const CWaveFormat& arg)
{
	acqtime=arg.acqtime;
	wversion=arg.wversion;
	fullscale_Volts = arg.fullscale_Volts;
	binspan = arg.binspan;
	binzero = arg.binzero;

	mode_encoding=arg.mode_encoding;
	mode_clock=arg.mode_clock;
	mode_trigger=arg.mode_trigger;

	chrate = arg.chrate;
	scan_count=arg.scan_count;
	sample_count=arg.sample_count;	
	duration=arg.duration;

	trig_mode=arg.trig_mode;
	trig_chan=arg.trig_chan;
	trig_threshold=arg.trig_threshold;

	data_flow=arg.data_flow;
	bOnlineDisplay=arg.bOnlineDisplay;
	bADwritetofile=arg.bADwritetofile;
	bufferNitems=arg.bufferNitems;
	buffersize=arg.buffersize;	

	csComment=arg.csComment;			// 1
	csStimulus=arg.csStimulus;
	csConcentration=arg.csConcentration;
	csStimulus2=arg.csStimulus2;
	csConcentration2=arg.csConcentration2;

	csInsectname=arg.csInsectname;
	csLocation = arg.csLocation;
	csSensillum = arg.csSensillum;
	csStrain = arg.csStrain;
	csSex = arg.csSex;
	csMoreComment = arg.csMoreComment;
	csADcardName = arg.csADcardName;	// 10
	csOperator=arg.csOperator;
	insectID = arg.insectID;
	sensillumID=arg.sensillumID;
	repeat =arg.repeat;
	repeat2 = arg.repeat2;
}


// Write CWaveFormat in a binary file
long CWaveFormat::Write(CFile* datafile)
{
	ULONGLONG p1 = datafile->GetPosition();
	CArchive ar(datafile, CArchive::store);
	Serialize(ar);
	ar.Close();
	ULONGLONG p2 = datafile->GetPosition();
	return (long) (p2-p1);
}

// Read CWaveFormat from a binary file
BOOL CWaveFormat::Read(CFile* datafile)
{
	CArchive ar(datafile, CArchive::load);
	BOOL flag = TRUE;
	try 
	{
		Serialize(ar);
	}
	catch (CException* e)
	{
		e->Delete();
		flag= FALSE;
	}
	ar.Close();
	return flag;
}

CString CWaveFormat::GetComments (CString psep, BOOL bExpl)
{
	CString csOut;
	csOut += AddComments(psep, bExpl, _T("comment1="),	csComment);
	csOut += AddComments(psep, bExpl, _T("stim1="),		csStimulus);
	csOut += AddComments(psep, bExpl, _T("conc1= #"),	csConcentration);
	csOut += AddComments(psep, bExpl, _T("stim2="),		csStimulus2);
	csOut += AddComments(psep, bExpl, _T("com2= #"),	csConcentration2);
	
	csOut += AddComments(psep, bExpl, _T("insect="),	csInsectname);
	csOut += AddComments(psep, bExpl, _T("location="),	csLocation);
	csOut += AddComments(psep, bExpl, _T("sensillum="), csSensillum);
	csOut += AddComments(psep, bExpl, _T("strain="),	csStrain);
	csOut += AddComments(psep, bExpl, _T("sex="),		csSex);
	csOut += AddComments(psep, bExpl, _T("operator="),	csOperator);
	csOut += AddComments(psep, bExpl, _T("comment2="),	csMoreComment);

	return csOut;
}

CString CWaveFormat::AddComments(CString psep, BOOL bExpl, CString csExpl, CString cscomment)
{
	CString csOut;
	csOut += psep; 
	if (bExpl) 
		csOut += csExpl;	
	if (!cscomment.IsEmpty())		
		csOut += cscomment;
	return csOut;
}

//------------------ class CWaveChan ---------------------------------

IMPLEMENT_SERIAL(CWaveChan, CObject, 0 /* schema number*/ )

CWaveChan::CWaveChan()
{
	am_csComment.Empty();
	am_gaintotal=0.0f;
	am_resolutionV = 0.;
	am_adchannel = 0;
	am_gainAD = 1;
	am_csheadstage= _T("none");
	am_gainheadstage = 1;
	am_csamplifier= _T("Unknown");
	am_amplifierchan=0; 
	am_gainpre=1; 
	am_gainpost=1; 
	am_notchfilt=0;
	am_lowpass=0; 
	am_inputpos=0; 
	am_inputneg=0; 
	am_csInputpos = _T("DC");
	am_csInputneg = _T("GND");
	am_offset=0.0f;	
	am_gainamplifier=1.0f;
	am_version = 1;
	am_csversion= _T("mcid");
}

CWaveChan::CWaveChan(CWaveChan& arg)
{
	am_csComment = arg.am_csComment;
	am_adchannel = arg.am_adchannel;
	am_gainAD = arg.am_gainAD;
	am_csheadstage = arg.am_csheadstage;
	am_gainheadstage = arg.am_gainheadstage;
	am_csamplifier=arg.am_csamplifier;
	am_amplifierchan=arg.am_amplifierchan;
	am_gainpre=arg.am_gainpre;
	am_gainpost=arg.am_gainpost;
	am_notchfilt=arg.am_notchfilt;
	am_lowpass=arg.am_lowpass;
	am_offset=arg.am_offset;
	am_csInputpos = arg.am_csInputpos;
	am_csInputneg= arg.am_csInputneg;
	am_gainamplifier = arg.am_gainamplifier;
	am_gaintotal= arg.am_gaintotal;
	am_resolutionV = arg.am_resolutionV;
}

CWaveChan::~CWaveChan()
{
}

void CWaveChan::operator = (const CWaveChan& arg)
{
	am_csComment=arg.am_csComment;
	am_adchannel=arg.am_adchannel;
	am_gainAD=arg.am_gainAD;

	am_csheadstage=arg.am_csheadstage;
	am_gainheadstage=arg.am_gainheadstage;

	am_csamplifier=arg.am_csamplifier;
	am_amplifierchan=arg.am_amplifierchan;
	am_gainpre=arg.am_gainpre;
	am_gainpost=arg.am_gainpost;
	am_notchfilt=arg.am_notchfilt;
	am_lowpass=arg.am_lowpass;
	am_offset=arg.am_offset;
	am_csInputpos = arg.am_csInputpos;
	am_csInputneg= arg.am_csInputneg;
	am_gainamplifier = arg.am_gainamplifier;
	am_gaintotal = arg.am_gaintotal;
	am_resolutionV = arg.am_resolutionV;
}

void CWaveChan::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		CString csdummy = am_csversion + am_csComment;	// modif 5 nov 2010 to introduce versioning in this structure
		ar << csdummy;
		float xgain = 0.0f;
		ar << xgain;		// dummy
		ar << (WORD) am_adchannel << (WORD) am_gainAD;
		ar << am_csheadstage;
		ar << (WORD) am_gainheadstage;
		ar << am_csamplifier << (WORD) am_amplifierchan;
		ar << (WORD) am_gainpre << (WORD) am_gainpost;
		ar << (WORD) am_notchfilt << (WORD) am_lowpass;
		am_inputpos = 0;	// dummy value (version april 2004 to add csInput)
		am_inputneg = 0;	// dummy value (version april 2004 to add csInput)
		ar << am_inputpos << am_inputneg;
		ar << am_offset;
		ar << am_csInputpos;
		ar << am_csInputneg;
		// addition 5 nov 2010
		ar << am_version;
		ar << am_gainamplifier;
		ar << am_gaintotal;
		ar << am_resolutionV;
	} 
	else
	{
		WORD w1, w2, w3, w4;
		ar >> am_csComment;
		float xgain;
		ar >> xgain ;				// dummy
		ar >> w1 >> w2; am_adchannel = (short) w1; am_gainAD = (short) w2;
		ar >> am_csheadstage;
		ar >> w1; am_gainheadstage = w1;
		ar >> am_csamplifier >> w1 >> w2 >> w3;
		am_amplifierchan = (short) w1; am_gainpre = (short) w2; am_gainpost = (short) w3;
		ar >> w1 >> w2 >> w3 >> w4;
		am_notchfilt	= (short) w1; 
		am_lowpass		= (short) w2;
		am_inputpos		= (short) w3;  
		am_inputneg		= (short) w4;
		ar >> am_offset;
		if (am_inputpos == 0 && am_inputneg == 0)
		{
			ar >> am_csInputpos;
			ar >> am_csInputneg;
		}
		else
		{
			am_csInputpos = GetCyberAmpInput(am_inputpos);
			am_csInputneg = GetCyberAmpInput(am_inputneg);
		}

		// patch to read values stored after version 0
		int nchars = am_csversion.GetLength();
		if (am_csComment.GetLength() >= nchars && am_csComment.Find(am_csversion, 0) >= 0)
		{
			// extract the dummy sequence ("mcid")
			CString csdummy = am_csComment;
			am_csComment = csdummy.Right(csdummy.GetLength()-nchars);
			// read the rest of the archive
			ar >> w1;
			ar >> am_gainamplifier;
			ar >> am_gaintotal;
			ar >> am_resolutionV;
		}
	}
}


// Write CWaveChan in a binary file

long CWaveChan::Write(CFile *datafile)
{
	ULONGLONG p1 = datafile->GetPosition();
	CArchive ar(datafile, CArchive::store);
	Serialize(ar);
	ar.Close();
	ULONGLONG p2 = datafile->GetPosition();
	return (long) (p2-p1);
}

// Read CWaveChan from a binary file
BOOL CWaveChan::Read(CFile *datafile)
{
	CArchive ar(datafile, CArchive::load);
	BOOL flag = TRUE;
	try 
	{
		Serialize(ar);
	}
	catch (CException* e)
	{
		e->Delete();
		flag = FALSE;
	}
	ar.Close();
	return flag;
}

CString CWaveChan::GetCyberAmpInput(int value)
{
	CString csOut;
	// values:GND(-1) DC(0) .1 1 10 30 100 300 Hz (*10)
	switch (value)
	{
	case -10:
		csOut = _T("GND");
		break;
	case 0: 
		csOut = _T("DC");
		break;
	default: 
		csOut.Format(_T("%i"), value / 10);
		break;
	}
	return csOut;
}


//------------------ class CWaveChanArray ---------------------------------

IMPLEMENT_SERIAL(CWaveChanArray, CObject, 0 /* schema number*/ )

CWaveChanArray::CWaveChanArray()
{
}

CWaveChanArray::~CWaveChanArray()
{
	ChannelRemoveAll();
}

long CWaveChanArray::Write(CFile *datafile)
{
	ULONGLONG p1 = datafile->GetPosition();

	short arraySize = m_chanArray.GetSize();
	datafile->Write(&arraySize, sizeof(short));
	CWaveChan *pChannel;
	for (int Compteur = 0; Compteur < arraySize; Compteur ++)
	{
		pChannel = (CWaveChan*) m_chanArray[Compteur];
		pChannel->Write(datafile);
	}

	ULONGLONG p2 = datafile->GetPosition();
	return (long) (p2-p1);
}

BOOL CWaveChanArray::Read(CFile *datafile)
{
	short arraySize;
	datafile->Read(&arraySize, sizeof(short));
	CWaveChan* pChannel;
	int n = 0;	

	// if size = 0, create dummy & empty channel
	if (arraySize == 0)
	{
		ChannelRemoveAll();					// erase existing data	
		pChannel = new CWaveChan;
		ASSERT(pChannel != NULL);
		m_chanArray.Add(pChannel);
		return FALSE;
	}
	
	// same size, load data in the objects already created
	if (m_chanArray.GetSize() == arraySize)
	{
		do
		{
			pChannel = (CWaveChan*) m_chanArray.GetAt(n);
			ASSERT(pChannel != NULL);
			if (!pChannel->Read(datafile))
				return FALSE;
			n++;		
		} while (n < arraySize);
	}
	else
	{
		ChannelRemoveAll();					// erase existing data	
		do
		{
			pChannel = new CWaveChan;
			ASSERT(pChannel != NULL);
			if (!pChannel->Read(datafile))
				return FALSE;
			m_chanArray.Add(pChannel);
			n++;		
		} while (n < arraySize);
	}
	return TRUE;
}

// this routine must duplicate the content of the array
// otherwise the pointers would point to the same objects and
// one of the array would be undefined when the other is destroyed

void CWaveChanArray::operator = (const CWaveChanArray& arg)
{
	int nItems=arg.m_chanArray.GetSize();// source size
	ChannelRemoveAll();					// erase existing data
	for (int i = 0; i < nItems; i++)	// loop over n items
	{
		CWaveChan* pChannel = new CWaveChan;// create new object
		ASSERT(pChannel != NULL);
		*pChannel = *((CWaveChan*) arg.m_chanArray[i]);
		ChannelAdd(pChannel);		// store pointer into array
	}
}

CWaveChan& CWaveChanArray::operator[] (int index)
{	
	return *((CWaveChan*) m_chanArray.GetAt(index));
}

CWaveChan* CWaveChanArray::GetWaveChan(int index)
{	
	return (CWaveChan*) m_chanArray.GetAt(index);
}

void CWaveChanArray::ChannelRemoveAll()
{
	CWaveChan* p;
	for (int i = 1; i <= m_chanArray.GetSize(); i++)
	{
		p = (CWaveChan*) m_chanArray[i-1];
		delete p;
	}	
	m_chanArray.RemoveAll();
}

int CWaveChanArray::ChannelAdd(CWaveChan *arg)
{
	return m_chanArray.Add(arg);
}

int CWaveChanArray::ChannelAdd()
{
	CWaveChan* p = new CWaveChan;
	ASSERT(p != NULL);
	return ChannelAdd(p);
}

void CWaveChanArray::ChannelInsert(int Indice)
{
	CWaveChan* p = new CWaveChan;
	ASSERT(p != NULL);
	m_chanArray.InsertAt(Indice, p, 1);
}

void CWaveChanArray::ChannelRemove(int Indice)
{
	CWaveChan* p = (CWaveChan*) m_chanArray[Indice];
	delete p;
	m_chanArray.RemoveAt(Indice);
}

int CWaveChanArray::ChannelGetnum() 
{
	return m_chanArray.GetSize();
}

int CWaveChanArray::ChannelSetnum(int i)
{
	if (i < m_chanArray.GetSize())
	{
		for (int j = m_chanArray.GetUpperBound(); j>= i; j--)
			ChannelRemove(j);
	}
	else if (i > m_chanArray.GetSize())
		for (int j = m_chanArray.GetSize(); j< i; j++)
			ChannelAdd();
	return m_chanArray.GetSize();
}


void CWaveChanArray::Serialize(CArchive & ar)
{
	WORD wversion;
	if (ar.IsStoring())
	{
		wversion=1;
		ar << wversion;				// save version number
		int nitems = m_chanArray.GetSize();
		ar << nitems;				// save number of items
		if (nitems >0)				// loop to save each CWaveChan
		{
			for (int i=0; i<nitems; i++)
			{
				CWaveChan* pItem = (CWaveChan*) m_chanArray.GetAt(i);
				pItem->Serialize(ar);
			}
		}
		
	} 
	else
	{
		ar >> wversion;				// get version
		int nitems; ar >> nitems;	// get number of items
		if (nitems >0)				// loop to read all CWaveChan items
		{
			ChannelRemoveAll();
			for (int i=0; i<nitems; i++)
			{
				CWaveChan* pItem = new CWaveChan;
				ASSERT(pItem != NULL);
				pItem->Serialize(ar);
				m_chanArray.Add(pItem);
			}
		}
	}
}