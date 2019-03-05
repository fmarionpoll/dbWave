
#include "StdAfx.h"
#include "Acqdatad.h"
#include "SpikeFromChan.h"
#include "Spikelist.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CSpikeList

IMPLEMENT_SERIAL(CSpikeList,CObject,0)

CSpikeList::CSpikeList()
{
	//m_spkelmts.SetSize(0, 128);
	//m_classArray.SetSize(0, 128);
}

CSpikeList::~CSpikeList()
{   
	DeleteArrays();	
}

void CSpikeList::Serialize(CArchive& ar)
{  
	m_wversion=6;				// version=6 - aug 2013 change spike element
	if (ar.IsStoring())
	{
		WritefileVersion6(ar);
	}

	else
	 {
		DeleteArrays();
		CString cs_id;
		ar >> cs_id;
		if (cs_id != m_IDstring)
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

void CSpikeList::RemoveArtefacts()
{
	auto nspikes = m_spkelmts.GetSize();
	auto save_nbspikes = nspikes;

	for (auto i = nspikes - 1; i >= 0; i--)
	{
		if (m_spkelmts.GetAt(i)->get_class() < 0)
		{
			const auto se = m_spkelmts.GetAt(i);
			delete se;
			m_spkelmts.RemoveAt(i);
			m_spikebuffer.DeleteSpike(i);
			save_nbspikes--;
		}
	}
	nspikes = m_spkelmts.GetSize();
	ASSERT(save_nbspikes == nspikes);
	UpdateClassList();

}

void CSpikeList::WritefileVersion6(CArchive& ar)
{
	ar << m_IDstring;								// (1) save version ID
	ar << m_wversion;								// current version: 4 (aug 2005)

	ar << m_encoding;								// data encoding mode  
	ar << m_binzero;								// 
	ar << m_samprate;								// data acq sampling rate (Hz)
	ar << m_voltsperbin;							// nb volts per bin (data)
	ar << m_cscomment;								// spike channel descriptor

	m_parm.Serialize(ar);							// spike detection parameters
	m_acqchan.Serialize(ar);						// data acq def

	// save spike elements
	if (!m_bsaveartefacts)
		RemoveArtefacts();
	const auto nspikes = m_spkelmts.GetSize();
	ar << static_cast<WORD>(nspikes);
	for (auto i=0;i<nspikes;i++)
		m_spkelmts.GetAt(i)->Serialize(ar);	

	ar << static_cast<WORD>(m_spikebuffer.GetSpklen());	// (4) save spike raw data
	const auto nbytes = m_spikebuffer.GetSpklen() * sizeof(short);
	if (nbytes > 0)									// store data
	{
		for (auto i=0; i<nspikes; i++)				// loop through all spikes
			ar.Write(m_spikebuffer.GetSpike(i), nbytes);
	}

	// save spike classes
	if (m_classArray.GetAt(0) < static_cast<int>(0))
		m_bvalidclasslist=FALSE;					// (5) save class list if parameters are valid
	ar << static_cast<long>(m_bvalidclasslist);
	if (m_bvalidclasslist)							// save class list if valid
	{
		ar << static_cast<long>(m_nbclasses);		// number of classes
		auto j=0;
		for (auto i=0; i<m_nbclasses; i++)
		{											// // skip artefacts if option set
			if (m_classArray.GetAt(j) >= 0)
			{
				long idummy = m_classArray.GetAt(j);
				ar << idummy;
				idummy = m_classArray.GetAt(j+1);
				ar << idummy;
			}
			j += 2;
		}
	}
	const auto nparms = 4;							// save misc additional data (version 3 - feb 2003)
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
	const int nspikes = w1;
	m_spkelmts.SetSize(nspikes);					// change size of the list
	for (auto i=0;i<nspikes;i++)						// loop to save spike descriptors
	{
		auto*se=new(CSpikeElemt);
		ASSERT(se != NULL);
		se->Serialize(ar);
		m_spkelmts[i]=se;        	
	}
	m_spikebuffer.DeleteAllSpikes();				// delete data buffers
	ar >>  w1;										// read spike length
	m_spikebuffer.SetSpklen(w1);					// reset parms/buffer
	m_spikebuffer.m_binzero = GetAcqBinzero();
	const auto nbytes = w1 * sizeof(short) * nspikes;
	const auto lp_dest = m_spikebuffer.AllocateSpaceForSeveralSpikes(nspikes);
	
	ar.Read(lp_dest, nbytes);						// read data from disk
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
			auto j=0;								// index to array elements
			for (auto i=0; i<m_nbclasses; i++)
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
	const int nspikes = w1;
	m_spkelmts.SetSize(nspikes);					// change size of the list
	for (auto i=0;i<nspikes;i++)						// loop to save spike descriptors
	{
		auto*se=new(CSpikeElemt);
		ASSERT(se != NULL);
		se->Read0(ar);								// read spike element before version 6
		m_spkelmts[i]=se;        	
	}
	m_spikebuffer.DeleteAllSpikes();				// delete data buffers
	ar >>  w1;										// read spike length
	m_spikebuffer.SetSpklen(w1);					// reset parms/buffer
	m_spikebuffer.m_binzero = GetAcqBinzero();
	const auto nbytes = w1 * sizeof(short) * nspikes;
	const auto lp_dest = m_spikebuffer.AllocateSpaceForSeveralSpikes(nspikes);
	
	ar.Read(lp_dest, nbytes);						// read data from disk
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
	// (1) version ID already loaded
	
	// (2) load parameters associated to data acquisition
	auto* pf_c = new CSpikeFromChan;
	auto iobjects=1;
	if (iversion == 4)
		ar >> iobjects;
	for (auto i=0; i<iobjects; i++)
	{
		pf_c->Serialize(ar);
		if (i > 0)
			continue;
		m_encoding=pf_c->encoding ;			// data encoding mode  
		m_binzero=pf_c->binzero;				// 2048
		m_samprate=pf_c->samprate;			// data acq sampling rate (Hz)
		m_voltsperbin=pf_c->voltsperbin;		// nb volts per bin (data)
		m_cscomment=pf_c->comment;			// spike channel descriptor
		m_parm= pf_c->parm;					// spike detection parameters
	}
	delete pf_c;
	// acquisition parameters
	m_acqchan.Serialize(ar);

	// ----------------------------------------------------
	// (3) load SpikeElement items
	WORD w1;
	ar >> w1;
	const int nspikes = w1;
	m_spkelmts.SetSize(nspikes);// change size of the list
	for (auto i=0;i<nspikes;i++)
	{
		auto se=new(CSpikeElemt);
		ASSERT(se != NULL);
		se->Read0(ar);						// read spike element before version 1
		m_spkelmts[i]=se;        	
	}

	// ----------------------------------------------------
	// (4) load spike raw data
	m_spikebuffer.DeleteAllSpikes();	// delete data buffers
	ar >>  w1;							// read spike length
	m_spikebuffer.SetSpklen(w1);		// reset parms/buffer
	m_spikebuffer.m_binzero = GetAcqBinzero();
	
	// loop through all data buffers
	const auto nbytes = w1 * sizeof(short) * nspikes;
	auto lp_dest = m_spikebuffer.AllocateSpaceForSeveralSpikes(nspikes);
	ar.Read(lp_dest, nbytes);				// read data from disk		
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

void CSpikeList::DeleteArrays()
{   	
	if (m_spkelmts.GetSize() > 0)
	{
		for (auto i=m_spkelmts.GetUpperBound(); i>= 0; i--)
		{
			auto* pspk_element = m_spkelmts.GetAt(i);
			delete pspk_element;
		}
		m_spkelmts.RemoveAll();   
	}
}

int CSpikeList::RemoveSpike(int spikeindex)
{
	if (spikeindex <= m_spkelmts.GetUpperBound())
	{
		const auto pspk_element = m_spkelmts.GetAt(spikeindex);
		delete pspk_element;
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
		for (auto j = m_spkelmts.GetUpperBound(); j >= 0; j--)	
		{										// assumed ordered list
			if (iitime >= m_spkelmts[j]->get_time())
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
		deltatime = m_spkelmts[jspk-1]->get_time() - iitime;
		if (labs(deltatime) <= jitter)		// allow a detection jitter of 2 (?)
		{
			jspk--;							// exit if condition met
			return TRUE;
		}
	}

	if (jspk <= m_spkelmts.GetUpperBound())	// deal only with case of jspk within the list
	{	
		deltatime = m_spkelmts[jspk]->get_time() - iitime;
		const auto ichan2 = m_spkelmts[jspk]->get_source_channel();
		if (ichan2 == ichan &&  labs(deltatime) <= jitter)		// allow a detection jitter of 2 (?)
			return TRUE;					// exit if condition met
	}
	
	return FALSE;
}

int CSpikeList::AddSpike(short* lp_source, const int nchans, const long iitime, const int sourcechan, const int iclass,
                         const BOOL b_check)
{   
	// search spike index of first spike which time is > to present one
	int jspk;
	auto jitter = 0;
	if (b_check)
		jitter = m_jitterSL;
	const auto bfound = IsAnySpikeAround(iitime, jitter, jspk, sourcechan);

	if (!bfound)
	{
		// create spike element and add pointer to array	
		auto* se = new CSpikeElemt(iitime, sourcechan, m_totalmax, m_totalmin, 0, iclass, 0);
		ASSERT(se != NULL);
		m_spkelmts.InsertAt(jspk, se);					// add element to the list of spikes

		if (lp_source != nullptr)
		{
			m_spikebuffer.AllocateSpaceForSpikeAt(jspk);		// add buffer area
			TransferDataToSpikeBuffer(jspk, lp_source, nchans, TRUE);	// add spike
		}
	}
	return jspk;
}

BOOL CSpikeList::TransferDataToSpikeBuffer(const int no, short* lp_source, const int nchans, const BOOL b_adjust)
{
	if (no <0 || no >= m_spkelmts.GetSize())
		return FALSE;

	// compute avg from m_icenter1SL to m_icenter2SL
	auto lp_b = lp_source + nchans*m_icenter1SL;
	long lavg = 0;				// average
	for (auto i = m_icenter1SL; i <= m_icenter2SL; i++, lp_b += nchans)
	{
		const int val = *lp_b;
		lavg += val;			// compute avg
	}

	// search max min
	lp_b = lp_source + nchans*m_imaxmin1SL;
	int max = *lp_b;				// provisional max
	auto min = max;				// provisional min
	auto imin = 0;
	auto imax = 0;

	for (auto i = m_imaxmin1SL+1; i <= m_imaxmin2SL; i++)
	{
		const int val = *lp_b;
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
		lp_b += nchans;			// update pointer
	}
	const auto dmaxmin = imin - imax;  // assume that min comes after max in a 'normal' spike

	// get offset over zero at the requested index
	auto offset = 0;
	if (b_adjust)
	{
		if (m_icenter1SL - m_icenter2SL != 0)
		{
			offset = lavg / (m_icenter2SL - m_icenter1SL+1);
			offset -= m_spikebuffer.m_binzero;
		}
	}

	// save values computed here within spike element structure
	auto* se = m_spkelmts.GetAt(no);
	se->SetSpikeMaxMin(max-offset, min-offset, dmaxmin);
	se->SetSpikeAmplitudeOffset(offset);
	auto lp_dest = m_spikebuffer.GetSpike(no);		// get pointer to buffer
	for (auto i = m_spikebuffer.GetSpklen(); i>0; i--)	// loop to copy data
	{
		*lp_dest = *lp_source - offset;				// copy data
		lp_dest++;									// update destination pointer
		lp_source += nchans;							// update source pointer
	}
	return true;
}

void CSpikeList::MeasureSpikeMaxMin(const int index, int* max, int* imax, int* min, int* imin)
{
	auto lp_buffer = m_spikebuffer.GetSpike(index); // get pointer to buffer
	int val = *lp_buffer;
	*max = val;		// assume offset between points = 1 (short)
	*min = val;        // init max and min val
	*imin = *imax = 0;
	for (auto i = 1; i <m_spikebuffer.GetSpklen(); i++)// loop to scan data
	{
		lp_buffer++;
		val = *lp_buffer;
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

void CSpikeList::MeasureSpikeMaxThenMin(const int index, int* max, int* imax, int* min, int* imin)
{
	auto lp_buffer = m_spikebuffer.GetSpike(index); // get pointer to buffer
	auto lp_bmax = lp_buffer;
	int val = *lp_buffer;
	*max = val;		// assume offset between points = 1 (short)
	*imax = 0;

	// first search for max
	for (auto i = 1; i <m_spikebuffer.GetSpklen(); i++)// loop to scan data
	{
		lp_buffer++;
		val = *lp_buffer;
		if (val > *max)	// search max
		{
			*max = val;	// change max and imax
			*imax = i;
			lp_bmax = lp_buffer;
		}
	}

	// then search for min
	lp_buffer = lp_bmax;
	*min = *max;
	*imin = *imax;
	for (auto i = *imax; i <m_spikebuffer.GetSpklen(); i++)// loop to scan data
	{
		lp_buffer++;
		val = *lp_buffer;
		if (val < *min)	// search max
		{
			*min = val;	// change max and imax
			*imin = i;
		}
	}
}

void CSpikeList::MeasureSpikeMaxMinEx(const int index, int* max, int* imax, int* min, int* imin, const int ifirst,
                                      const int ilast)
{
	auto lp_buffer = m_spikebuffer.GetSpike(index);
	lp_buffer += ifirst;					// get pointer to buffer
	int val = *lp_buffer;
	*max = val;						// assume offset between points = 1 (short)
	*min = *max;					// init max and min val
	*imin = *imax = ifirst;
	for (auto i = ifirst+1; i <= ilast; i++)	// loop to scan data
	{
		lp_buffer++;
		val = *lp_buffer;
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

void CSpikeList::MeasureSpikeMaxThenMinEx(const int index, int* max, int* imax, int* min, int* imin, const int ifirst,
                                          const int ilast)
{
	auto lp_buffer = m_spikebuffer.GetSpike(index);
	lp_buffer += ifirst;					// get pointer to buffer
	auto lp_bmax = lp_buffer;
	int val = *lp_buffer;
	*max = val;						// assume offset between points = 1 (short)
	*imax = ifirst;

	// first search for max
	for (auto i = ifirst+1; i <= ilast; i++)	// loop to scan data
	{
		lp_buffer++;
		val = *lp_buffer;
		if (val > *max)				// search max
		{
			*max = val;				// change max and imax
			*imax = i;
			lp_bmax = lp_buffer;
		}			
	}

	// search for min
	lp_buffer = lp_bmax;
	*min = *max;
	*imin = *imax;
	for (int i = *imin+1; i <= ilast; i++)	// loop to scan data
	{
		lp_buffer++;
		val = *lp_buffer;
		if (val < *min)				// search max
		{
			*min = val;				// change max and imax
			*imin = i;
		}
	}
}

void CSpikeList::GetTotalMaxMin(const BOOL b_recalc, int* max, int* min)
{
	if (b_recalc || !m_bextrema)
	{
		auto lmax = 0, lmin=65535;

		// first find valid max and min
		for (auto index = 0; index < m_spkelmts.GetSize(); index++)
		{
			if (GetSpikeClass(index) >= 0)
			{
				GetSpikeExtrema(0, &lmax, &lmin);
				break;
			}
		}
		m_totalmin = lmin;	// init max with first value
		m_totalmax = lmax;	// init min with first value
		for (auto index=0; index < m_spkelmts.GetSize(); index++)
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

BOOL CSpikeList::InitSpikeList(CAcqDataDoc* p_data_file, SPKDETECTPARM* pFC)
{
	// remove data from spike list
	EraseData();
	RemoveAllSpikeFlags();

	// copy data from CObArray
	if (pFC != nullptr)
		m_parm = *pFC;

	auto flag = false;
	if (p_data_file != nullptr)
	{
		const auto pwave_format = p_data_file->GetpWaveFormat();
		m_encoding = pwave_format->mode_encoding;
		m_binzero  = pwave_format->binzero;
		m_samprate = pwave_format->chrate;
		flag = p_data_file->GetWBVoltsperBin(m_parm.detectChan, &m_voltsperbin);
	}

	if (!flag)
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

void CSpikeList::EraseData()
{
	DeleteArrays();
	m_spikebuffer.DeleteAllSpikes();
	m_selspike = -1;
}

int	CSpikeList::SetSpikeFlag(int spikeno, BOOL bFlag)
{
	// set spike flag: add spike to the array
	if (TRUE == bFlag)
	{
		// first look if spikeno is already flagged
		if (!GetSpikeFlag (spikeno))
			m_spike_flagged.Add(spikeno);
	}

	// remove flag
	else
	{
		// find spikeno within the array and remove it
		auto index = -1;
		for (auto i= m_spike_flagged.GetCount()-1; i >=0 ; i--)
		{
			if (m_spike_flagged.GetAt(i) == spikeno)
			{
				index = i;
				break;
			}
		}
		if (index >= 0)
			m_spike_flagged.RemoveAt(index);
		
	}
	// return the number of elements within the array
	return GetSpikeFlagArrayCount();
}

int	CSpikeList::ToggleSpikeFlag(int spikeno)
{
	// find spike within array
	auto index = -1;
	for (auto i= m_spike_flagged.GetCount()-1; i >=0 ; i--)
	{
		if (m_spike_flagged.GetAt(i) == spikeno)
		{
			index = i;
			break;
		}
	}

	// if found: remove it
	if (index >= 0)
		m_spike_flagged.RemoveAt(index);

	// if not found, add it
	else
		m_spike_flagged.Add(spikeno);

	return GetSpikeFlagArrayCount();
}

void CSpikeList::SetSingleSpikeFlag(int spikeno)
{
	if ( m_spike_flagged.GetCount()!= 1)	
		m_spike_flagged.SetSize(1);
	m_spike_flagged.SetAt(0, spikeno);
}

BOOL CSpikeList::GetSpikeFlag(int spikeno)
{
	BOOL bFlag = FALSE;
	// search if spikeno is in the list
	for (int i= m_spike_flagged.GetCount()-1; i >= 0 ; i--)
	{
		if (m_spike_flagged.GetAt(i) == spikeno)
		{
			bFlag = TRUE;
			break;
		}
	}
	return bFlag;
}

void CSpikeList::FlagRangeOfSpikes(long l_first, long l_last, BOOL bSet)
{
	// first clear flags of spikes within the flagged array which fall within limits
	long l_time;
	for (auto i= m_spike_flagged.GetCount()-1; i >= 0 ; i--)
	{
		const int ispik= m_spike_flagged.GetAt(i);
		l_time = GetSpikeTime(ispik);
		if (l_time < l_first || l_time > l_last)
			continue;
		m_spike_flagged.RemoveAt(i);
	}
	// if bSet was set to FALSE, the job is done
	if (bSet == FALSE)
		return;

	// then if bSet is ON, search spike file for spikes falling within this range and flag them
	for (auto i=0; i < GetTotalSpikes(); i++)
	{
		l_time = GetSpikeTime(i);
		if (l_time < l_first || l_time > l_last)
			continue;
		m_spike_flagged.Add(i);
	}
}

void CSpikeList::SelectSpikeswithinRect(int vmin, int vmax, long l_first, long l_last, BOOL bAdd)
{
	if (!bAdd)
		RemoveAllSpikeFlags();
	for (auto i=0; i < GetTotalSpikes(); i++)
	{
		const auto lTime = GetSpikeTime(i);
		if (lTime < l_first || lTime > l_last)
			continue;

		int max, min;
		GetSpikeExtrema(i, &max, &min);
		if (max > vmax) continue;
		if (min < vmin) continue;
		// found within boundaries= remove spike from array
		m_spike_flagged.Add(i);
	}
}

void CSpikeList::GetRangeOfSpikeFlagged(long& l_first, long& l_last)
{
	// no spikes flagged, return dummy values
	if (m_spike_flagged.GetCount() < 1)
	{
		l_first = -1;
		l_last = -1;
		return;
	}

	// spikes flagged: init max and min to the first spike time
	l_first = GetSpikeTime(m_spike_flagged.GetAt(0));
	l_last = l_first;

	// search if spikeno is in the list
	for (auto i= m_spike_flagged.GetCount()-1; i >= 0 ; i--)
	{
		const auto lTime = GetSpikeTime(m_spike_flagged.GetAt(i));
		if (lTime < l_first)
			l_first = lTime;
		if (lTime > l_last)
			l_last = lTime;
	}
}
		
void CSpikeList::OffsetSpikeAmplitude(int index, int valfirst, int vallast, int center)
{
	auto lp_buffer = m_spikebuffer.GetSpike(index); // get pointer to buffer	
	const auto diff = vallast - valfirst;		// difference
	int max=*lp_buffer; 						// compute max/min on the fly
	auto min = max;						// provisional max and minimum
	auto imax= 0;
	auto imin = vallast-valfirst;
	for (auto i = 0; i <m_spikebuffer.GetSpklen(); i++)// loop to scan data
	{								// offset point i
		*lp_buffer += valfirst + MulDiv(diff, i, m_spikebuffer.GetSpklen()); 
		if (*lp_buffer > max)					// new max?
		{
			max = *lp_buffer;
			imax = i;
		}
		else if (*lp_buffer < min)			// new min?
		{
			min = *lp_buffer;
			imin = i;
		}
		lp_buffer++;							// update pointer
	}
	const auto dmaxmin = imin - imax;			// assume that min comes after max in a "normal" spike

	auto offset = m_spkelmts[index]->get_amplitude_offset();
	offset -= valfirst;					// change spike offset
	m_spkelmts[index]->SetSpikeAmplitudeOffset(offset);
	m_bextrema = FALSE;
	m_spkelmts[index]->SetSpikeMaxMin(max, min, dmaxmin);
}

void CSpikeList::CenterSpikeAmplitude(int spkindex, int ifirst, int ilast, WORD method)
{
	auto lp_buffer = m_spikebuffer.GetSpike(spkindex); // get pointer to buffer	
	lp_buffer += ifirst;
	int valfirst;								// contains offset
	auto imax = 0;
	auto imin = 0;
	auto dmaxmin=0;
	int max, min;
	auto p_se = GetSpikeElemt(spkindex);
	p_se->GetSpikeMaxMin(&max, &min, &dmaxmin);
	
	switch (method)
	{
	case 0:		// ........................ center (max + min )/2
		{
		max=*lp_buffer; 						// compute max/min on the fly
		min = max;						// provisional max and minimum	
		for (auto i = ifirst; i <ilast; i++, lp_buffer++)
			{								// offset point i		
			if (*lp_buffer > max)					// new max?
			{
				max = *lp_buffer;
				imax = i;
			}
			else if (*lp_buffer < min)			// new min?
			{
				min = *lp_buffer;	
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
			const auto ioffset = GetAcqBinzero();
			for (auto i = ifirst; i <ilast; i++, lp_buffer++)
				lsum += (*lp_buffer - ioffset);
			valfirst = static_cast<int>(lsum / static_cast<long>(ilast - ifirst + 1));
		}
		break;
	default:
		valfirst = 0;
		break;
	}

	// change spike offset	
	p_se->SetSpikeAmplitudeOffset(p_se->get_amplitude_offset()+valfirst);
	// then offset data (max & min ibidem)
	p_se->SetSpikeMaxMin(max-valfirst, min-valfirst, dmaxmin);
	lp_buffer = m_spikebuffer.GetSpike(spkindex); // get pointer to buffer	
	for (auto i = 0; i <m_spikebuffer.GetSpklen(); i++, lp_buffer++)
		*lp_buffer -= valfirst;		
	m_bextrema = FALSE;	
}

long CSpikeList::UpdateClassList()
{
	const auto nspikes = GetTotalSpikes();		// total nb of spikes within list
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
	for (auto i=1; i<nspikes; i++)
	{
		const auto cla = GetSpikeClass(i);		// class of spike i
		auto arrayclass=0;

		// loop over all existing classes to find if there is one
		for (auto j=0; j<m_nbclasses; j++)
		{
			arrayclass = GetclassID(j);	// class ID
			if (cla == arrayclass)		// class OK?
			{
				m_classArray[j*2+1]++;	// increment spike count
				break;					// exit spk class loop
			}
			if (cla < arrayclass)		// insert new class?
			{
				m_classArray.InsertAt(j*2, cla); // build new item
				m_classArray.InsertAt(j*2+1, 1); // set nb spk
				m_nbclasses++;			// exit spk class loop
				break;
			}
		}
		// test if cla not found within array
		if (cla > arrayclass)
		{			
			m_classArray.Add(cla); 		// build new item
			m_classArray.Add(1);		// set nb spk
			m_nbclasses++;				// update nb classes
		}
	}
	return m_nbclasses;
}

void CSpikeList::ChangeSpikeClassID(int oldclaID, int newclaID)
{
	// first find valid max and min
	for (auto index = 0; index < m_spkelmts.GetSize(); index++)
	{
		if (GetSpikeClass(index) == oldclaID)
			SetSpikeClass(index, newclaID);
	}	
}

void CSpikeList::Measure_case0_AmplitudeMinToMax(const int t1, const int t2)
{
	const auto nspikes = GetTotalSpikes();

	for (auto ispike = 0; ispike < nspikes; ispike++)
	{
		const auto spike_element = GetSpikeElemt(ispike);
		auto lp_buffer = m_spikebuffer.GetSpike(ispike);
		lp_buffer += t1;
		int val = *lp_buffer;
		auto max = val;	
		auto min = val;	
		auto imin = t1;
		auto imax = t1;
		for (auto i = t1 + 1; i <= t2; i++)	
		{
			lp_buffer++;
			val = *lp_buffer;
			if (val > max)
			{
				max = val;
				imax = i;
			}
			else if (val < min)	
			{
				min = val;
				imin = i;
			}
		}

		spike_element->SetSpikeMaxMin(max, min, imin - imax);
		spike_element->set_y1(max - min);
	}
}


void CSpikeList::Measure_case1_AmplitudeAtT(const int t)
{
	const auto nspikes = GetTotalSpikes();

	for (auto ispike = 0; ispike < nspikes; ispike++)
	{
		const auto spike_element = GetSpikeElemt(ispike);
		auto lp_buffer = m_spikebuffer.GetSpike(ispike);
		lp_buffer += t;
		const int val = *lp_buffer;
		spike_element->set_y1(val);
	}
}

void CSpikeList::Measure_case2_AmplitudeAtT2MinusAtT1(const int t1, const int t2)
{
	const auto nspikes = GetTotalSpikes();

	for (auto ispike = 0; ispike < nspikes; ispike++)
	{
		const auto spike_element = GetSpikeElemt(ispike);
		const auto lp_buffer = m_spikebuffer.GetSpike(ispike);
		const int val1 = *(lp_buffer + t1);
		const int val2 = *(lp_buffer + t2);
		spike_element->set_y1(val2-val1);
	}
}

CSize CSpikeList::Measure_Y1_MaxMin()
{
	const auto nspikes = GetTotalSpikes();
	int max = GetSpikeElemt(0)->get_y1() ;
	int min = max;
	for (auto ispike = 0; ispike < nspikes; ispike++)
	{
		const auto val = GetSpikeElemt(ispike)->get_y1();
		if (val > max) max = val;
		if (val < min) min = val;
	}

	return CSize(max, min);
}

BOOL CSpikeList::SortSpikeWithY1(const CSize fromclass_toclass, const CSize timewindow, const CSize limits)
{
	const auto nspikes = GetTotalSpikes();

	const auto from_class = fromclass_toclass.cx;
	const auto to_class = fromclass_toclass.cy;
	const auto first = timewindow.cx;
	const auto last = timewindow.cy;
	const int upper = limits.cy;
	const int lower = limits.cx;
	BOOL bchanged = false;

	for (auto ispike = 0; ispike < nspikes; ispike++)
	{
		const auto spike_element = GetSpikeElemt(ispike);
		if (spike_element->get_class() != from_class)
			continue;
		const auto iitime = spike_element->get_time();
		if (iitime < first || iitime > last)
			continue;
		const auto value = spike_element->get_y1();
		if (value >= lower && value <= upper) {
			spike_element->set_class(to_class);
			bchanged = true;
		}
	}

	return bchanged;
}

BOOL CSpikeList::SortSpikeWithY1AndY2(const CSize fromclass_toclass, const CSize timewindow, const CSize limits1,
                                      const CSize limits2)
{
	const auto nspikes = GetTotalSpikes();

	const auto from_class = fromclass_toclass.cx;
	const auto to_class = fromclass_toclass.cy;
	const auto first = timewindow.cx;
	const auto last = timewindow.cy;
	const int upper1 = limits1.cy;
	const int lower1 = limits1.cx;
	const int upper2 = limits2.cy;
	const int lower2 = limits2.cx;
	BOOL bchanged = false;

	for (auto ispike = 0; ispike < nspikes; ispike++)
	{
		const auto spike_element = GetSpikeElemt(ispike);
		if (spike_element->get_class() != from_class)
			continue;
		const auto iitime = spike_element->get_time();
		if (iitime < first || iitime > last)
			continue;
		const auto value1 = spike_element->get_y1();
		const auto value2 = spike_element->get_y2();
		if ((value1 >= lower1 && value1 <= upper1) && (value2 >= lower2 && value2 <= upper2))
		{
			spike_element->set_class(to_class);
			bchanged = true;
		}
	}

	return bchanged;
}

