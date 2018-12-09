/////////////////////////////////////////////////////////////////////////////
// spiklist.cpp : implementation file
//
// CFromChan
// CSpikeElement
// CSpikeList

#include "StdAfx.h"
#include "Acqdatad.h"
#include "Spikelist.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CFromChan

// default spike detection init
CFromChan::CFromChan()
{
	wversion=5;
}

CFromChan::~CFromChan()
{
}

IMPLEMENT_SERIAL(CFromChan, CObject, 0 /* schema number*/ )

void CFromChan::Serialize(CArchive& ar)
{
	// store elements
	if (ar.IsStoring())
	{
		ar << wversion;
		ar << binzero;
		ar << encoding;
		ar << voltsperbin;
		ar << samprate;
		parm.Serialize(ar);
		int nitems = 1;
		ar << nitems;
		ar << comment;
	}
	// load data
	 else
	{
		WORD w; 
		WORD version;
		ar >> version;				// version number
		if (version < 5)
		{
			ar >> w; binzero = w;
		}
		else
			ar >> binzero;			// long (instead of word)
		if (version < 3)
		{
			ar >> w; parm.extractChan = w;
		}
		ar >> encoding;
		ar >> voltsperbin;
		ar >> samprate;
		if (version < 3)
		{
			ar >> w; parm.detectTransform=w;
			ar >> w; parm.detectFrom=w;
			ar >> w; parm.detectThreshold=w;
			ar >> w;	// unused parameter, removed at version 3
			ar >> w; parm.extractNpoints = w;
			ar >> w; parm.prethreshold=w;
			ar >> w; parm.refractory=w;
		}
		else
		{
			parm.Serialize(ar);
		}
		if (version >3)
		{
			int nitems;
			ar >> nitems;
			ar >> comment;
		}
	}
}

void CFromChan::operator =(const CFromChan& arg)
{
	wversion	= arg.wversion;
	binzero 	= arg.binzero;
	encoding 	= arg.encoding;
	samprate 	= arg.samprate;
	voltsperbin = arg.voltsperbin;
	parm		= arg.parm;
	comment		= arg.comment;
}


/////////////////////////////////////////////////////////////////////////////
// CSpikeBuffer
// maintains a buffer where raw spikes are stored
// create a buffer with filtered (transformed) spikes
// use associated parameters to manage buffer

IMPLEMENT_SERIAL(CSpikeBuffer, CObject, 0 /* schema number*/ )

CSpikeBuffer::CSpikeBuffer()
{
	m_pspkbuffer= nullptr;
	m_binzero = 2048;
	SetSpklen(1);	// init with spike len = 1	
	m_idata.SetSize(0, 128);
}

CSpikeBuffer::CSpikeBuffer(int lenspk)
{
	m_pspkbuffer= nullptr;
	m_binzero = 2048;
	SetSpklen(lenspk);
	m_idata.SetSize(0, 128);
}

CSpikeBuffer::~CSpikeBuffer()
{
	RemoveAllBuffers();
}


/////////////////////////////////////////////////////////////////////////////
void CSpikeBuffer::Serialize(CArchive& ar)
{
	// store elements
	if (ar.IsStoring())
	{   
	}
	// load data
	 else
	{
	}
}

/////////////////////////////////////////////////////////////////////////////
void CSpikeBuffer::SetSpklen(int lenspik)
{
	m_lenspk = lenspik;
	RemoveAllBuffers();
	if (m_lenspk <= 0)
		return;

	// allocate memory by 64 Kb chunks
	m_spkbufferincrement = (WORD)32767;
	m_spkbufferincrement = (m_spkbufferincrement/m_lenspk)*m_lenspk;
	m_spkbufferlength = m_spkbufferincrement;
	m_pspkbuffer = (short*) malloc(sizeof(short)*m_spkbufferlength);
	ASSERT(m_pspkbuffer != NULL);
	m_nextindex = 0;
	m_lastindex = m_spkbufferlength/m_lenspk -1;
	return;
}

// AddSpikeBuf(WORD spkindex)
// add an empty array for spike "spkindex" and insert address of
// this array inside m_lpspkdata which is an ordered array of pointers

short* CSpikeBuffer::AddSpikeBuf(WORD spkindex)
{
	// get pointer to next available buffer area for this spike
	// CAUTION: spkindex != m_nextindex
	if (m_nextindex > m_lastindex)
	{
		m_spkbufferlength += m_spkbufferincrement;		
		short* pspkbuffer = (short*) realloc(m_pspkbuffer, sizeof(short)*m_spkbufferlength);
		if (pspkbuffer != nullptr)
			m_pspkbuffer = pspkbuffer;
		m_lastindex = m_spkbufferlength/m_lenspk -1;
	}

	// compute destination address
	int offset = m_nextindex*m_lenspk;	
	short* lpDest = m_pspkbuffer + offset;
	m_idata.InsertAt(spkindex, offset);
	m_nextindex++;	
	return lpDest;
}

// prepare for loading contiguous nspikes
short*	CSpikeBuffer::AddNumbersofSpikes(WORD nspikes)
{
	// get pointer to next available buffer area for these spikes
	int currentindex = m_nextindex;
	m_nextindex += nspikes;
	while (m_nextindex > m_lastindex)
	{
		m_spkbufferlength += m_spkbufferincrement;		
		short* pspkbuffer = (short*) realloc(m_pspkbuffer, sizeof(short)*m_spkbufferlength);
		if (pspkbuffer != nullptr)
			m_pspkbuffer = pspkbuffer;
		m_lastindex = m_spkbufferlength/m_lenspk -1;
	}

	// compute destination address
	short* lpDest = m_pspkbuffer + (currentindex*m_lenspk);
	int offset = m_lenspk;
	int ioffset = currentindex*m_lenspk;
	for (int i=currentindex; i<m_nextindex; i++)
	{
		m_idata.InsertAt(i, ioffset);
		ioffset += offset;
	}
	return lpDest;
}

/////////////////////////////////////////////////////////////////////////////

void CSpikeBuffer::RemoveAllBuffers()
{
	// delete handle array and liberate corresponding memory
	if (m_pspkbuffer != nullptr)
		free(m_pspkbuffer);
	m_pspkbuffer = nullptr;

	// delete array of pointers
	m_idata.RemoveAll();
	m_nextindex = 0;
}

BOOL CSpikeBuffer::DeleteSpike(WORD spkindex)
{
	if (spkindex > m_idata.GetUpperBound() || spkindex <0)
		return FALSE;
	m_idata.RemoveAt(spkindex);
	return TRUE;
}

BOOL CSpikeBuffer::ExchangeSpikes(WORD spk1, WORD spk2)
{
	if (spk1 > m_idata.GetUpperBound() || spk1 <0
	||  spk2 > m_idata.GetUpperBound() || spk2 <0)
		return FALSE;
	DWORD dummy =	m_idata[spk1];
	m_idata[spk1] = m_idata[spk2];
	m_idata[spk2] = dummy;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CSpikeElemt


// default spike element init
CSpikeElemt::CSpikeElemt()
{
	m_iitime=0;
	m_class=0;
	m_chanparm=0;
	m_min = 0;
	m_max = 4096;
	m_offset = 2048;
	m_dmaxmin = 0;
}

// init spike element with time and chan
CSpikeElemt::CSpikeElemt( long time, WORD channel)
{
	m_iitime=time;
	m_chanparm=channel;
	m_class=0;
	m_min = 0;
	m_max = 4096;
	m_offset = 2048;
	m_dmaxmin =0;
}

CSpikeElemt::CSpikeElemt(LONG time, WORD channel, int max, int min, int offset, int iclass, int dmaxmin)
{
	m_iitime=time;
	m_chanparm=channel;
	m_min = min;
	m_max = max;
	m_offset = offset;
	m_class= iclass;
	m_dmaxmin = dmaxmin;
}

CSpikeElemt::~CSpikeElemt()
{
}

IMPLEMENT_SERIAL(CSpikeElemt, CObject, 0 /* schema number*/ )

void CSpikeElemt::Serialize(CArchive& ar)
{
	WORD w1;
	WORD wVersion = 1;

	if (ar.IsStoring())
	{
		ar << wVersion;
		ar << m_iitime;
		ar << (WORD) m_class;
		ar << (WORD) m_chanparm;
		ar << WORD (m_max);
		ar << WORD (m_min);
		ar << WORD (m_offset);
		ar << WORD (m_dmaxmin);
	}
	else
	{
		ar >> wVersion;
		ar >> m_iitime; 
		ar >> w1; m_class		= (int) w1;
		ar >> w1; m_chanparm	= (int) w1;
		ar >> w1; m_max			= (short) w1;
		ar >> w1; m_min			= (short) w1;
		ar >> w1; m_offset		= (short) w1;
		ar >> w1; m_dmaxmin		= (short) w1;
	}
}

void CSpikeElemt::Read0(CArchive& ar)
{
	WORD w1;
	
	ASSERT(ar.IsStoring() == FALSE);
	
	ar >> m_iitime; 
	ar >> w1; m_class	 = (int)  w1;
	ar >> w1; m_chanparm = (int)  w1;
	ar >> w1; m_max		=  (short) w1;
	ar >> w1; m_min		 = (short) w1;
	ar >> w1; m_offset	 = (short) w1;
	m_dmaxmin = 0;
}

///////////////////////////////////////////////////////////////////////////
// CSpikeList	implementation 
///////////////////////////////////////////////////////////////////////////

IMPLEMENT_SERIAL(CSpikeList,CObject,0)

/**************************************************************************
 function:  CSpikeList()
 purpose:	constructor
 parameters:
 returns:	
 comments:
 **************************************************************************/

CSpikeList::CSpikeList()
{
	m_bsaveartefacts = FALSE;	
	m_nbclasses = 1;
	m_bvalidclasslist=FALSE;	// default: no valid array
	//m_classArray.SetSize(2);	// default size - some functions
	//m_classArray.SetAt(0,0);	// access this array anyway so provide dummy value
	//m_classArray.SetAt(1,0);	// access this array anyway so provide dummy value
	m_IDstring = "Awave Spike file v";
	m_selspike=-1;
	m_lFirstSL=0;
	m_lLastSL=0;
	m_bextrema = FALSE;
	m_totalmin = 2048;
	m_totalmax = 2100;
	m_jitterSL = 2;
	m_icenter1SL = 0;
	m_icenter2SL = 60;
	m_imaxmin1SL = 0;
	m_imaxmin2SL = 60;
	m_spkelmts.SetSize(0, 128);
	m_classArray.SetSize(0, 128);
}

/**************************************************************************
 function:  ~CSpikeList()
 purpose:	destructor
 parameters:
 returns:	
 comments:
 **************************************************************************/
CSpikeList::~CSpikeList()
{   
	DeleteArrays();	
}


/**************************************************************************
 function:   Serialize
 purpose:	 Save or Load a SpikeList 
 parameters: CArchive& ar
 returns:	 
 comments:   overridden for document i/o      
 **************************************************************************/

void CSpikeList::Serialize(CArchive& ar)
{  
	m_wversion=6;				// version=6 - aug 2013 change spike element
	
	// store elements ...................................
	if (ar.IsStoring())
	{
		WritefileVersion6(ar);
	}

	// load data  ...................................
	 else
	 {
		DeleteArrays();
		CString csID;
		ar >> csID;
		if (csID != m_IDstring)
			ReadfileVersion1(ar);
		else 
		{
			WORD iversion;
			ar >> iversion;
			if (iversion > 0 && iversion < 5)
				ReadfileVersion_before5(ar, iversion);
			else if (iversion == 5)
				ReadfileVersion5(ar);
			else if (iversion == 6)
				ReadfileVersion6(ar);
			else
			{
				ASSERT(FALSE);
				ATLTRACE2("reading spike list: unrecognizable version %i\n", iversion);
			}
		}
	 }
}

void CSpikeList::ReadfileVersion1(CArchive& ar)
{
	m_icenter1SL = 0;
	m_icenter2SL = m_parm.prethreshold;;
	m_imaxmin1SL = m_icenter2SL;
	m_imaxmin2SL = m_spikebuffer.GetSpklen()-1;
}

void CSpikeList::WritefileVersion6(CArchive& ar)
{
	ar << m_IDstring;								// (1) save version ID
	ar << m_wversion;								// current version: 4 (aug 2005)
	// description of channel source
	ar << m_encoding;								// data encoding mode  
	ar << m_binzero;								// 
	ar << m_samprate;								// data acq sampling rate (Hz)
	ar << m_voltsperbin;							// nb volts per bin (data)
	ar << m_cscomment;								// spike channel descriptor
	m_parm.Serialize(ar);							// spike detection parameters
	m_acqchan.Serialize(ar);						// data acq def

	int nspikes = m_spkelmts.GetSize();				// count number of spikes
	int saveNbspikes = nspikes;
	// remove artefacts if necessary
	if (!m_bsaveartefacts)
	{
		for (int i=nspikes-1; i>=0; i--)				// loop through all spikes
		{
			if (((CSpikeElemt *)m_spkelmts.GetAt(i))->GetSpikeClass() <0) 
			{
				CSpikeElemt* se=(CSpikeElemt *)m_spkelmts.GetAt(i);	// new
				delete se;											// new
				m_spkelmts.RemoveAt(i);								// new
				m_spikebuffer.DeleteSpike(i);
				saveNbspikes--;
			}
		}
		nspikes = m_spkelmts.GetSize();								// new
		ASSERT(saveNbspikes == nspikes);
		UpdateClassList();											// new
	}

	// save number of spikes
	ar << (WORD) saveNbspikes;						// save total nb of spk
	// save spike descriptors
	for (int i=0;i<nspikes;i++)						// loop over all spikes
	{        	
		CSpikeElemt* se=(CSpikeElemt *)m_spkelmts.GetAt(i);	// get pointer
		se->Serialize(ar);							// store parameter
	}

	// save spike data
	ar << (WORD) m_spikebuffer.GetSpklen();			// (4) save spike raw data
	UINT nbytes = m_spikebuffer.GetSpklen() * sizeof(short);
	if (nbytes > 0)									// store data
	{
		for (int i=0; i<nspikes; i++)					// loop through all spikes
		{
			CSpikeElemt* se=(CSpikeElemt *)m_spkelmts.GetAt(i);
			ar.Write(m_spikebuffer.GetSpike(i), nbytes);
		}
	}

	// save spike classes
	if ((int)m_classArray.GetAt(0) < (int)0)
		m_bvalidclasslist=FALSE;					// (5) save class list if parameters are valid
	ar << (long) m_bvalidclasslist;
	if (m_bvalidclasslist)							// save class list if valid
	{
		ar << (long) m_nbclasses;					// number of classes
		long idummy;								// dummy item
		short j=0;
		for (int i=0; i<m_nbclasses; i++)
		{											// // skip artefacts if option set
			if (m_classArray.GetAt(j) < 0)
			{
				j +=2;
				continue;
			}										// class nb then nb spikes
			idummy = m_classArray.GetAt(j); j++; ar << idummy;
			idummy = m_classArray.GetAt(j); j++; ar << idummy;
		}
	}
	int nparms = 4;									// save misc additional data (version 3 - feb 2003)
	ar << nparms;
	ar << m_icenter1SL;
	ar << m_icenter2SL;
	ar << m_imaxmin1SL;
	ar << m_imaxmin2SL;

	ar.Flush();										// end of storing -- flush data
}

void CSpikeList::ReadfileVersion6(CArchive& ar)
{
	// read spike detection parameters array
	ar >> m_encoding;								// data encoding mode  
	ar >> m_binzero;								// 
	ar >> m_samprate;								// data acq sampling rate (Hz)
	ar >> m_voltsperbin;							// nb volts per bin (data)
	ar >> m_cscomment;								// spike channel descriptor
	m_parm.Serialize(ar);							// spike detection parameters
	m_acqchan.Serialize(ar);
	
	WORD w1;
	ar >> w1; 
	int nspikes = w1;
	m_spkelmts.SetSize(nspikes);					// change size of the list
	for (int i=0;i<nspikes;i++)						// loop to save spike descriptors
	{
		CSpikeElemt *se=new(CSpikeElemt);
		ASSERT(se != NULL);
		se->Serialize(ar);
		m_spkelmts[i]=se;        	
	}
	m_spikebuffer.RemoveAllBuffers();				// delete data buffers
	ar >>  w1;										// read spike length
	m_spikebuffer.SetSpklen(w1);					// reset parms/buffer
	m_spikebuffer.m_binzero = GetAcqBinzero();
	UINT nbytes = w1 * sizeof(short) * nspikes;
	short* lpDest = m_spikebuffer.AddNumbersofSpikes(nspikes);
	
	ar.Read(lpDest, nbytes);						// read data from disk
	m_bextrema = FALSE;
	m_bvalidclasslist=FALSE;						// default: no valid array
	m_nbclasses = 1;
	m_classArray.SetSize(2);						// default size - some functions
	m_classArray.SetAt(0,0);						// access this array anyway so provide
	m_classArray.SetAt(1,0);						// dummy values for them
	long dummy; ar >> dummy; m_bvalidclasslist=dummy;
	if (m_bvalidclasslist)							// read class list
	{
		ar >> dummy; m_nbclasses=dummy;				
		if (m_nbclasses != 0)
		{
			m_classArray.SetSize(m_nbclasses*2);
			int j=0;								// index to array elements
			for (int i=0; i<m_nbclasses; i++)
			{						
				ar >> dummy; m_classArray.SetAt(j, dummy); j++; // class nb						
				ar >> dummy; m_classArray.SetAt(j, dummy); j++; // nb spikes
			}
		}
	}
	int nparms;
	ar >> nparms;
	ar >> m_icenter1SL; nparms--;
	ar >> m_icenter2SL; nparms--;
	ar >> m_imaxmin1SL; nparms--;
	ar >> m_imaxmin2SL; nparms--;
}

void CSpikeList::ReadfileVersion5(CArchive& ar)
{
	// read spike detection parameters array
	ar >> m_encoding;								// data encoding mode  
	ar >> m_binzero;								// 
	ar >> m_samprate;								// data acq sampling rate (Hz)
	ar >> m_voltsperbin;							// nb volts per bin (data)
	ar >> m_cscomment;								// spike channel descriptor
	m_parm.Serialize(ar);							// spike detection parameters
	m_acqchan.Serialize(ar);
	
	WORD w1;
	ar >> w1; 
	int nspikes = w1;
	m_spkelmts.SetSize(nspikes);					// change size of the list
	for (int i=0;i<nspikes;i++)						// loop to save spike descriptors
	{
		CSpikeElemt *se=new(CSpikeElemt);
		ASSERT(se != NULL);
		se->Read0(ar);								// read spike element before version 6
		m_spkelmts[i]=se;        	
	}
	m_spikebuffer.RemoveAllBuffers();				// delete data buffers
	ar >>  w1;										// read spike length
	m_spikebuffer.SetSpklen(w1);					// reset parms/buffer
	m_spikebuffer.m_binzero = GetAcqBinzero();
	UINT nbytes = w1 * sizeof(short) * nspikes;
	short* lpDest = m_spikebuffer.AddNumbersofSpikes(nspikes);
	
	ar.Read(lpDest, nbytes);						// read data from disk
	m_bextrema = FALSE;
	m_bvalidclasslist=FALSE;						// default: no valid array
	m_nbclasses = 1;
	m_classArray.SetSize(2);						// default size - some functions
	m_classArray.SetAt(0,0);						// access this array anyway so provide
	m_classArray.SetAt(1,0);						// dummy values for them
	long dummy; ar >> dummy; m_bvalidclasslist=dummy;
	if (m_bvalidclasslist)							// read class list
	{
		ar >> dummy; m_nbclasses=dummy;				
		if (m_nbclasses != 0)
		{
			m_classArray.SetSize(m_nbclasses*2);
			int j=0;								// index to array elements
			for (int i=0; i<m_nbclasses; i++)
			{						
				ar >> dummy; m_classArray.SetAt(j, dummy); j++; // class nb						
				ar >> dummy; m_classArray.SetAt(j, dummy); j++; // nb spikes
			}
		}
	}
	int nparms;
	ar >> nparms;
	ar >> m_icenter1SL; nparms--;
	ar >> m_icenter2SL; nparms--;
	ar >> m_imaxmin1SL; nparms--;
	ar >> m_imaxmin2SL; nparms--;
}


void CSpikeList::ReadfileVersion_before5(CArchive& ar, int iversion)
{
	// ----------------------------------------------------
	// (1) version ID already loaded
	
	// ----------------------------------------------------
	// (2) load parameters associated to data acquisition
	CFromChan* pfC = new CFromChan;
	int iobjects=1;
	if (iversion == 4)
		ar >> iobjects;
	for (int i=0; i<iobjects; i++)
	{
		pfC->Serialize(ar);
		if (i > 0)
			continue;
		m_encoding=pfC->encoding ;			// data encoding mode  
		m_binzero=pfC->binzero;				// 2048
		m_samprate=pfC->samprate;			// data acq sampling rate (Hz)
		m_voltsperbin=pfC->voltsperbin;		// nb volts per bin (data)
		m_cscomment=pfC->comment;			// spike channel descriptor
		m_parm= pfC->parm;					// spike detection parameters
	}
	delete pfC;
	// acquisition parameters
	m_acqchan.Serialize(ar);

	// ----------------------------------------------------
	// (3) load SpikeElement items
	WORD w1;
	ar >> w1; int nspikes = w1;
	m_spkelmts.SetSize(nspikes);// change size of the list
	for (int i=0;i<nspikes;i++)
	{
		CSpikeElemt *se=new(CSpikeElemt);
		ASSERT(se != NULL);
		se->Read0(ar);						// read spike element before version 1
		m_spkelmts[i]=se;        	
	}

	// ----------------------------------------------------
	// (4) load spike raw data
	m_spikebuffer.RemoveAllBuffers();	// delete data buffers
	ar >>  w1;							// read spike length
	m_spikebuffer.SetSpklen(w1);		// reset parms/buffer
	m_spikebuffer.m_binzero = GetAcqBinzero();
	
	// loop through all data buffers
	UINT nbytes = w1 * sizeof(short) * nspikes;
	short* lpDest = m_spikebuffer.AddNumbersofSpikes(nspikes);
	ar.Read(lpDest, nbytes);				// read data from disk		
	m_bextrema = FALSE;
	
	// ----------------------------------------------------
	// (5) load class array if valid flag
	// reset elements of the list
	m_bvalidclasslist=FALSE;	// default: no valid array
	m_nbclasses = 1;
	m_classArray.SetSize(2);	// default size - some functions
	m_classArray.SetAt(0,0);	// access this array anyway so provide
	m_classArray.SetAt(1,0);	// dummy values for them
	
	// load flag and load elements only if valid
	long dummy; ar >> dummy; m_bvalidclasslist=dummy;
	if (m_bvalidclasslist)	// read class list
	{
		ar >> dummy; m_nbclasses=dummy;				
		if (m_nbclasses != 0)
		{
			m_classArray.SetSize(m_nbclasses*2);
			int j=0;	// index to array elements
			for (int i=0; i<m_nbclasses; i++)
			{						
				ar >> dummy; m_classArray.SetAt(j, dummy); j++; // class nb						
				ar >> dummy; m_classArray.SetAt(j, dummy); j++; // nb spikes
			}
		}
	}
	if (iversion > 2)
	{
		int nparms;
		ar >> nparms;
		ar >> m_icenter1SL; nparms--;
		ar >> m_icenter2SL; nparms--;
		ar >> m_imaxmin1SL; nparms--;
		ar >> m_imaxmin2SL; nparms--;
	}
	if (iversion < 3)
	{
		m_icenter1SL = 0;
		m_icenter2SL = m_parm.prethreshold;
		m_imaxmin1SL = m_icenter2SL;
		m_imaxmin2SL = m_spikebuffer.GetSpklen()-1;
	}
}


/**************************************************************************
function: 	DeleteArrays
purpose :	Delete array of spike parameters
parameters 
return void
**************************************************************************/

void CSpikeList::DeleteArrays()
{   	
	if (m_spkelmts.GetSize() > 0)
	{
		for (int i=m_spkelmts.GetUpperBound(); i>= 0; i--)
		{
			CSpikeElemt* pSpkE = (CSpikeElemt *) m_spkelmts.GetAt(i);
			delete pSpkE;
		}
		m_spkelmts.RemoveAll();   
	}
}


int CSpikeList::RemoveSpike(int spikeindex)
{
	if (spikeindex <= m_spkelmts.GetUpperBound())
	{
		CSpikeElemt* pSpkE = (CSpikeElemt *) m_spkelmts.GetAt(spikeindex);
		delete pSpkE;
		m_spkelmts.RemoveAt(spikeindex);
	}
	return m_spkelmts.GetSize();
}


BOOL CSpikeList::IsAnySpikeAround(long iitime, int jitter, int& jspk, int ichan)
{
	// search spike index of first spike which time is > to present one
	jspk = 0;
	if (m_spkelmts.GetSize() > 0)
	{
		// loop to find position of the new spike
		for (int j = m_spkelmts.GetUpperBound(); j >= 0; j--)	
		{										// assumed ordered list
			if (iitime >= ((CSpikeElemt*)m_spkelmts[j])->GetSpikeTime())
			{
				jspk = j+1;						// new spike should be inserted in the array
				break;							// break loop, keep jspk as final index
			}
		}
	}

	// check if no duplicate otherwise exit immediately
	long deltatime;							// estimate dist of new spike / neighbours
	if (jspk > 0)							// skip case where no spikes are in the list
	{
		deltatime = ((CSpikeElemt*)m_spkelmts[jspk-1])->GetSpikeTime() - iitime;
		if (labs(deltatime) <= jitter)		// allow a detection jitter of 2 (?)
		{
			jspk--;							// exit if condition met
			return TRUE;
		}
	}

	if (jspk <= m_spkelmts.GetUpperBound())	// deal only with case of jspk within the list
	{	
		deltatime = ((CSpikeElemt*)m_spkelmts[jspk])->GetSpikeTime() - iitime;
		int ichan2 = ((CSpikeElemt*)m_spkelmts[jspk])->GetSpikeChannel();
		if (ichan2 == ichan &&  labs(deltatime) <= jitter)		// allow a detection jitter of 2 (?)
			return TRUE;					// exit if condition met
	}
	
	return FALSE;
}

/**************************************************************************
 function:  AddSpike
 purpose:	add a spike to the List 
			adjust voltage level so that 
			average from X(first, center) = 2048
 parameters: 	
	lpSource	= buff pointer to the buffer to copy
	nchans		= nb of interleaved channels
	time		= file index of first pt of the spk
	detectChan	= data source chan index
	iClass		= class of the spike to add
	bCheck		= check if a spike is present within m_jitterSL (or do not check and force add)
	//center		= upper index of spike array used to adjust baseline level
 returns:	index of spike added, 0 if failed
 comments:
 **************************************************************************/

int CSpikeList::AddSpike(short* lpSource, WORD nchans, long iitime, int sourcechan, int iclass, BOOL bCheck)
{   
	// search spike index of first spike which time is > to present one
	int jspk;
	int jitter = 0;
	if (bCheck)
		jitter = m_jitterSL;
	BOOL bfound = IsAnySpikeAround(iitime, jitter, jspk, sourcechan);

	if (!bfound)
	{
		// create spike element and add pointer to array	
		CSpikeElemt* se = new CSpikeElemt(iitime, sourcechan, m_totalmax, m_totalmin, 0, iclass, 0);
		ASSERT(se != NULL);
		m_spkelmts.InsertAt(jspk, se);					// add element to the list of spikes

		if (lpSource != nullptr)
		{
			m_spikebuffer.AddSpikeBuf(jspk);			// add buffer area
			SetSpikeData(jspk, lpSource, nchans, TRUE);	// add spike but try to center it
		}
	}
	return jspk;
}


//************************************************************************** 
// SetSpikeData
// replace data of spike element
// in:
// no = spike list index
// lpSource = address of source array
// nchans = nb interleaved data channels in source data
// center = pretrig offset; used to center spike
//**************************************************************************

BOOL CSpikeList::SetSpikeData(WORD no, short* lpSource, int nchans, BOOL bAdjust)
{
	BOOL flag = TRUE;
	if (no <0 || no >= m_spkelmts.GetSize())
		return FALSE;

	// --------------------------------
	// search maximum and minimum to center spike within buffer
	short* lpB = lpSource;		// duplicate pointer
	
	// compute avg from m_icenter1SL to m_icenter2SL
	lpB = lpSource + nchans*m_icenter1SL;
	long lavg = 0;			// average
	for (int i = m_icenter1SL; i <= m_icenter2SL; i++, lpB += nchans)
	{
		int val = *lpB;
		lavg += val;		// compute avg
	}

	// search max min
	lpB = lpSource + nchans*m_imaxmin1SL;
	int max = *lpB;				// provisional max
	int min = max;				// provisional min
	int imin = 0;
	int imax = 0;

	for (int i = m_imaxmin1SL+1; i <= m_imaxmin2SL; i++)
	{
		int val = *lpB;
		if (val > max)			// search max
		{
			max = val;			// change max and imax
			imax = i;
		}
		else if (val < min)		// search min
		{
			min = val;			// change min and imin
			imin = i;
		}
		lpB += nchans;			// update pointer
	}
	int dmaxmin = imin - imax;  // assume that min comes after max in a 'normal' spike

	// get offset over zero at the requested index
	int offset = 0;
	if (bAdjust)
	{
		if (m_icenter1SL - m_icenter2SL != 0)
		{
			offset = lavg / (m_icenter2SL - m_icenter1SL+1);
			offset -= m_spikebuffer.m_binzero;
		}
	}

	// save values computed here within spike element structure
	CSpikeElemt* se = (CSpikeElemt *) m_spkelmts.GetAt(no);
	se->SetSpikeMaxMin(max-offset, min-offset, dmaxmin);
	se->SetSpikeAmplitudeOffset(offset);
	short* lpDest = m_spikebuffer.GetSpike(no);		// get pointer to buffer
	for (int i = m_spikebuffer.GetSpklen(); i>0; i--)	// loop to copy data
	{
		*lpDest = *lpSource - offset;				// copy data
		lpDest++;									// update destination pointer
		lpSource += nchans;							// update source pointer
	}
	return flag;
}


/**************************************************************************
function:      	GetSpikeMaxMin()
purpose :		search max and min of spike i
parameters 		short index = index of spike to scan
				int *max, = pointer to max value (modified)
				int *imax = index max within spike (modified)
				int *min  = min value (modified)
				int *imin = index min value (modified)
return void
**************************************************************************/

void CSpikeList::MeasureSpikeMaxMin(WORD index, int* max, int* imax, int* min, int* imin)
{
	short* lpB = m_spikebuffer.GetSpike(index); // get pointer to buffer
	int val = *lpB;
	*max = val;		// assume offset between points = 1 (short)
	*min = val;        // init max and min val
	*imin = *imax = 0;
	for (int i = 1; i <m_spikebuffer.GetSpklen(); i++)// loop to scan data
	{
		lpB++;
		val = *lpB;
		if (val > *max)	// search max
		{
			*max = val;	// change max and imax
			*imax = i;
		}
		else if (val < *min) // search min
		{
			*min = val;	// change min and imin
			*imin = i;
		}			
	}
}

void CSpikeList::MeasureSpikeMaxThenMin(WORD index, int* max, int* imax, int* min, int* imin)
{
	short* lpB = m_spikebuffer.GetSpike(index); // get pointer to buffer
	short* lpBmax = lpB;
	int val = *lpB;
	*max = val;		// assume offset between points = 1 (short)
	*imax = 0;

	// first search for max
	for (int i = 1; i <m_spikebuffer.GetSpklen(); i++)// loop to scan data
	{
		lpB++;
		val = *lpB;
		if (val > *max)	// search max
		{
			*max = val;	// change max and imax
			*imax = i;
			lpBmax = lpB;
		}
	}

	// then search for min
	lpB = lpBmax;
	*min = *max;
	*imin = *imax;
	for (int i = *imax; i <m_spikebuffer.GetSpklen(); i++)// loop to scan data
	{
		lpB++;
		val = *lpB;
		if (val < *min)	// search max
		{
			*min = val;	// change max and imax
			*imin = i;
		}
	}
}

void CSpikeList::MeasureSpikeMaxMinEx(WORD index, int* max, int* imax, int* min, int* imin, int ifirst, int ilast)
{
	short* lpB = m_spikebuffer.GetSpike(index);
	lpB += ifirst;					// get pointer to buffer
	int val = *lpB;
	*max = val;						// assume offset between points = 1 (short)
	*min = *max;					// init max and min val
	*imin = *imax = ifirst;
	for (int i = ifirst+1; i <= ilast; i++)	// loop to scan data
	{
		lpB++;
		val = *lpB;
		if (val > *max)				// search max
		{
			*max = val;				// change max and imax
			*imax = i;
		}
		else if (val < *min)		// search min
		{
			*min = val;				// change min and imin
			*imin = i;
		}			
	}
}

void CSpikeList::MeasureSpikeMaxThenMinEx(WORD index, int* max, int* imax, int* min, int* imin, int ifirst, int ilast)
{
	short* lpB = m_spikebuffer.GetSpike(index);
	lpB += ifirst;					// get pointer to buffer
	short* lpBmax = lpB;
	int val = *lpB;
	*max = val;						// assume offset between points = 1 (short)
	*imax = ifirst;

	// first search for max
	for (int i = ifirst+1; i <= ilast; i++)	// loop to scan data
	{
		lpB++;
		val = *lpB;
		if (val > *max)				// search max
		{
			*max = val;				// change max and imax
			*imax = i;
			lpBmax = lpB;
		}			
	}

	// search for min
	lpB = lpBmax;
	*min = *max;
	*imin = *imax;
	for (int i = *imin+1; i <= ilast; i++)	// loop to scan data
	{
		lpB++;
		val = *lpB;
		if (val < *min)				// search max
		{
			*min = val;				// change max and imax
			*imin = i;
		}
	}
}

/**************************************************************************
function:      	GetTotalMaxMin()
purpose :		compute max and min of all spikes
				discard spike with class < 0
parameters 		bRecalc : TRUE=calculate all, FALSE=read param m_totalmax,..
				max	= maximum amplitude
				min	= minimum amplitude
return void
**************************************************************************/

void CSpikeList::GetTotalMaxMin(BOOL bRecalc, int* max, int* min)
{
	if (bRecalc || !m_bextrema)
	{
		int lmax = 0, lmin=0;
		int index;
		// first find valid max and min
		for (index = 0; index < m_spkelmts.GetSize(); index++)
		{
			if (GetSpikeClass(index) >= 0)
			{
				GetSpikeExtrema(0, &lmax, &lmin);
				break;
			}
		}
		m_totalmin = lmin;	// init max with first value
		m_totalmax = lmax;	// init min with first value
		for (index; index < m_spkelmts.GetSize(); index++)
		{
			if (GetSpikeClass(index) >= 0)
			{
				GetSpikeExtrema(index, &lmax, &lmin);
				if (lmax > m_totalmax) m_totalmax = lmax;
				if (lmin < m_totalmin) m_totalmin = lmin;
			}
		}
		m_bextrema = TRUE;
	}
	*max = m_totalmax;
	*min = m_totalmin;		
}


/**************************************************************************
function:      	BOOL InitSpikeList()
purpose :		define source data before adding spikes
				erase old list (if flag set)
parameters 		CAcqDataDoc* pDoc
				CFromChan* CFromChan
return 			TRUE if OK
modif 20-12-94 use of SPKDETECT & chan instead of CFromChan struct as param
**************************************************************************/

BOOL CSpikeList::InitSpikeList(CAcqDataDoc* pDataFile, SPKDETECTPARM* pFC)
{
	// remove data from spike list
	EraseData();
	RemoveAllSpikeFlags();

	// copy data from CObArray
	if (pFC != nullptr)
		m_parm = *pFC;
		
	if (pDataFile != nullptr)
	{
		CWaveFormat* pwaveFormat = pDataFile->GetpWaveFormat();
		m_encoding = pwaveFormat->mode_encoding;
		m_binzero  = pwaveFormat->binzero;
		m_samprate = pwaveFormat->chrate;
		pDataFile->GetWBVoltsperBin(m_parm.detectChan, &m_voltsperbin);
	}
	else
	{
		m_bextrema = TRUE;
		m_totalmin = 2048;
		m_totalmax = 2100;
	}

	// reset buffers, list, spk params	
	m_spikebuffer.SetSpklen(m_parm.extractNpoints);
	m_spikebuffer.m_binzero = m_binzero;

	// reset classes
	m_bvalidclasslist=FALSE;	// default: no valid array
	m_nbclasses = 0;
	m_classArray.SetSize(2);	// default size - some functions
	m_classArray.SetAt(0,0);	// access this array anyway so provide
	m_classArray.SetAt(1,0);	// dummy values for them
	return TRUE;	
}

/**************************************************************************
function:      	Get/SetDetectParms()
purpose :		load data from/to Cfromchan				
parameters 		SPKDETEC structure
				CFromChan* CFromChan
created 10-5-95 to update save detection parms at the moment of the detection
				and to restore them when the spk file is read
**************************************************************************/

//void CSpikeList::SetDetectParms(SPKDETECTPARM* pSd)
//{
//	m_parm = *pSd;	
//}
//
//SPKDETECTPARM* CSpikeList::GetDetectParms()
//{
//	return &m_parm;	
//}

void CSpikeList::EraseData()
{
	DeleteArrays();
	m_spikebuffer.RemoveAllBuffers();
	m_selspike = -1;
}

/**************************************************************************
function:       SetSpikeFlag(int spikeno, bFlag=TRUE/FALSE)
purpose :		set/erase flag of a given spike within the list
parameters 		spikeno= spike index; if -1= all spikes
				bFlag : TRUE=selected (added to the list)
						FALSE=de-selected (removed from the list)
return the number of spikes selected
**************************************************************************/

int	CSpikeList::SetSpikeFlag(int spikeno, BOOL bFlag)
{
	// set spike flag: add spike to the array
	if (TRUE == bFlag)
	{
		// first look if spikeno is already flagged
		if (!GetSpikeFlag (spikeno))
			m_bSpikeFlagArray.Add(spikeno);
	}

	// remove flag
	else
	{
		// find spikeno within the array and remove it
		int index = -1;
		for (int i= m_bSpikeFlagArray.GetCount()-1; i >=0 ; i--)
		{
			if (m_bSpikeFlagArray.GetAt(i) == spikeno)
			{
				index = i;
				break;
			}
		}
		if (index >= 0)
			m_bSpikeFlagArray.RemoveAt(index);
		
	}
	// return the number of elements within the array
	return GetSpikeFlagArrayCount();
}

/**************************************************************************
function:       ToggleSpikeFlag(int spikeno, bFlag=TRUE/FALSE)
purpose :		set/erase flag of a given spike within the list
parameters 		spikeno= spike index
				if flag=TRUE: remove from the list, if flag=FALSE: add it
return the number of spikes selected
**************************************************************************/

int	CSpikeList::ToggleSpikeFlag(int spikeno)
{
	// find spike within array
	int index = -1;
	for (int i= m_bSpikeFlagArray.GetCount()-1; i >=0 ; i--)
	{
		if (m_bSpikeFlagArray.GetAt(i) == spikeno)
		{
			index = i;
			break;
		}
	}

	// if found: remove it
	if (index >= 0)
		m_bSpikeFlagArray.RemoveAt(index);

	// if not found, add it
	else
		m_bSpikeFlagArray.Add(spikeno);

	return GetSpikeFlagArrayCount();
}

void CSpikeList::SetSingleSpikeFlag(int spikeno)
{
	if ( m_bSpikeFlagArray.GetCount()!= 1)	
		m_bSpikeFlagArray.SetSize(1);
	m_bSpikeFlagArray.SetAt(0, spikeno);
}

/**************************************************************************
function:       GetSpikeFlag(int spikeno)
purpose :		get the flag of spikeno
parameters 		spikeno= spike index; if -1= all spikes

return the flag value
**************************************************************************/
BOOL CSpikeList::GetSpikeFlag(int spikeno)
{
	BOOL bFlag = FALSE;
	// search if spikeno is in the list
	for (int i= m_bSpikeFlagArray.GetCount()-1; i >= 0 ; i--)
	{
		if (m_bSpikeFlagArray.GetAt(i) == spikeno)
		{
			bFlag = TRUE;
			break;
		}
	}
	return bFlag;
}

// flag, unflag all spikes from the list of spikes that fall within time boundaries
void CSpikeList::FlagRangeOfSpikes(long lFirst, long lLast, BOOL bSet)
{
	// first clear flags of spikes within the flagged array which fall within limits
	long lTime;
	for (int i= m_bSpikeFlagArray.GetCount()-1; i >= 0 ; i--)
	{
		int ispik= m_bSpikeFlagArray.GetAt(i);
		lTime = GetSpikeTime(ispik);
		if (lTime < lFirst || lTime > lLast)
			continue;
		// found within boundaries= remove spike from array
		m_bSpikeFlagArray.RemoveAt(i);
	}
	// if bSet was set to FALSE, the job is done
	if (bSet == FALSE)
		return;

	// then if bSet is ON, search spike file for spikes falling within this range and flag them
	for (int i=0; i < GetTotalSpikes(); i++)
	{
		lTime = GetSpikeTime(i);
		if (lTime < lFirst || lTime > lLast)
			continue;
		// found within boundaries= remove spike from array
		m_bSpikeFlagArray.Add(i);
	}
}
void CSpikeList::SelectSpikeswithinRect(int vmin, int vmax, long lFirst, long lLast, BOOL bAdd)
{
	if (!bAdd)
		RemoveAllSpikeFlags();
	for (int i=0; i < GetTotalSpikes(); i++)
	{
		long lTime = GetSpikeTime(i);
		if (lTime < lFirst || lTime > lLast)
			continue;

		int max, min;
		GetSpikeExtrema(i, &max, &min);
		if (max > vmax) continue;
		if (min < vmin) continue;
		// found within boundaries= remove spike from array
		m_bSpikeFlagArray.Add(i);
	}
}
// get the range of spikes flagged within the array

void CSpikeList::GetRangeOfSpikeFlagged(long& lFirst, long& lLast)
{
	// no spikes flagged, return dummy values
	if (m_bSpikeFlagArray.GetCount() < 1)
	{
		lFirst = -1;
		lLast = -1;
		return;
	}
	// spikes flagged: init max and min to the first spike time
	else
	{
		lFirst = GetSpikeTime(m_bSpikeFlagArray.GetAt(0));
		lLast = lFirst;
	}
	long lTime;

	// search if spikeno is in the list
	for (int i= m_bSpikeFlagArray.GetCount()-1; i >= 0 ; i--)
	{
		lTime = GetSpikeTime(m_bSpikeFlagArray.GetAt(i));
		if (lTime < lFirst)
			lFirst = lTime;
		if (lTime > lLast)
			lLast = lTime;
	}
}
		

/**************************************************************************
function:       OffsetSpikeAmplitude()
purpose :		offset spike amplitude by a vertical offset (horiz or diag line)
parameters 		spikeno= spike index
				valfirst = offset for spike at index 0
				vallast  = offset for spike at last point
return void
**************************************************************************/

void CSpikeList::OffsetSpikeAmplitude(WORD index, int valfirst, int vallast, int center)
{
	short* lpB = m_spikebuffer.GetSpike(index); // get pointer to buffer	
	int diff = vallast - valfirst;		// difference
	int max=*lpB; 						// compute max/min on the fly
	int min = max;						// provisional max and minimum
	int imax= 0;
	int imin = vallast-valfirst;
	for (int i = 0; i <m_spikebuffer.GetSpklen(); i++)// loop to scan data
	{								// offset point i
		*lpB += valfirst + MulDiv(diff, i, m_spikebuffer.GetSpklen()); 
		if (*lpB > max)					// new max?
		{
			max = *lpB;
			imax = i;
		}
		else if (*lpB < min)			// new min?
		{
			min = *lpB;
			imin = i;
		}
		lpB++;							// update pointer
	}
	int dmaxmin = imin - imax;			// assume that min comes after max in a "normal" spike
	
	int offset = ((CSpikeElemt*)m_spkelmts[index])->GetSpikeAmplitudeOffset();
	offset -= valfirst;					// change spike offset
	((CSpikeElemt*)m_spkelmts[index])->SetSpikeAmplitudeOffset(offset);
	m_bextrema = FALSE;
	((CSpikeElemt*)m_spkelmts[index])->SetSpikeMaxMin(max, min, dmaxmin);
}


/**************************************************************************
function:       CenterSpikeAmplitude()
purpose :		offset spike amplitude by a vertical offset (horiz or diag line)
parameters 		spikeno= spike index
				valfirst = offset for spike at index 0
				vallast  = offset for spike at last point
				method: 0=center (max + min )/2; 1=center average
return void
**************************************************************************/

void CSpikeList::CenterSpikeAmplitude(WORD spkindex, int ifirst, int ilast, WORD method)
{
	short* lpB = m_spikebuffer.GetSpike(spkindex); // get pointer to buffer	
	lpB += ifirst;
	int valfirst;								// contains offset
	int imax = 0;
	int imin = 0;
	int dmaxmin=0;
	int max, min;
	CSpikeElemt* pSE = GetSpikeElemt(spkindex);
	pSE->GetSpikeMaxMin(&max, &min, &dmaxmin);
	
	switch (method)
	{
	case 0:		// ........................ center (max + min )/2
		{
		max=*lpB; 						// compute max/min on the fly
		min = max;						// provisional max and minimum	
		for (int i = ifirst; i <ilast; i++, lpB++)
			{								// offset point i		
			if (*lpB > max)					// new max?
			{
				max = *lpB;
				imax = i;
			}
			else if (*lpB < min)			// new min?
			{
				min = *lpB;	
				imin = i;
			}
			}
		valfirst = (max+min)/2 -GetAcqBinzero();
		dmaxmin = imin - imax;
		}
		break;
	case 1: 	// ........................ center average
		{
		long lsum = 0;
		int ioffset = GetAcqBinzero();
		for (int i = ifirst; i <ilast; i++, lpB++)
			lsum += (*lpB - ioffset);
		valfirst = (int) (lsum / (long)(ilast-ifirst+1));
		}
		break;
	default:
		valfirst = 0;
		break;
	}

	// change spike offset	
	pSE->SetSpikeAmplitudeOffset(pSE->GetSpikeAmplitudeOffset()+valfirst);
	// then offset data (max & min ibidem)
	pSE->SetSpikeMaxMin(max-valfirst, min-valfirst, dmaxmin);
	lpB = m_spikebuffer.GetSpike(spkindex); // get pointer to buffer	
	for (int i = 0; i <m_spikebuffer.GetSpklen(); i++, lpB++)
		*lpB -= valfirst;		
	m_bextrema = FALSE;	
}


/**************************************************************************
function:      	UpdateClassList()
purpose :		browse all spikes and generate array with spk class 
				&& nb items /class (spk class is sorted by growing numbers)
parameters 		none
return void
**************************************************************************/

long CSpikeList::UpdateClassList()
{
	int nspikes = GetTotalSpikes();		// total nb of spikes within list
	m_nbclasses = 1;					// erase nb of classes
	m_classArray.SetSize(0);			// erase array
	m_bvalidclasslist = TRUE;			// class list OK when exit
	if (nspikes == 0)
	{									// no spikes? dummy parameters
		m_classArray.Add(0);			// dummy class
		m_classArray.Add(0);			// zero spikes
		return 0L;						// done
	}
		
	m_classArray.Add(GetSpikeClass(0));	// init with class of first spike
	m_classArray.Add(1);				// nb items found yet = 1
	m_nbclasses = 1;					// total nb of classes

	// loop over all spikes of the list
	for (int i=1; i<nspikes; i++)
	{
		int cla = GetSpikeClass(i);		// class of spike i
		int Arrayclass=0;

		// loop over all existing classes to find if there is one
		for (int j=0; j<m_nbclasses; j++)
		{
			Arrayclass = GetclassID(j);	// class ID
			if (cla == Arrayclass)		// class OK?
			{
				m_classArray[j*2+1]++;	// increment spike count
				break;					// exit spk class loop
			}
			if (cla < Arrayclass)		// insert new class?
			{
				m_classArray.InsertAt(j*2, cla); // build new item
				m_classArray.InsertAt(j*2+1, 1); // set nb spk
				m_nbclasses++;			// exit spk class loop
				break;
			}
		}
		// test if cla not found within array
		if (cla > Arrayclass)
		{			
			m_classArray.Add(cla); 		// build new item
			m_classArray.Add(1);		// set nb spk
			m_nbclasses++;				// update nb classes
		}
	}
	return m_nbclasses;
}

/**************************************************************************
function:      	ChangeSpikeClassID
purpose :		change all spike class ID from oldID -> newID
parameters		oldID, newID
return void
**************************************************************************/
void CSpikeList::ChangeSpikeClassID(int oldclaID, int newclaID)
{
	// first find valid max and min
	for (int index = 0; index < m_spkelmts.GetSize(); index++)
	{
		if (GetSpikeClass(index) == oldclaID)
			SetSpikeClass(index, newclaID);
	}	
}

/**************************************************************************
function:      	
purpose :		
parameters		
return void
**************************************************************************/
