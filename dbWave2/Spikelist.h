#pragma once

// spikelist.h : header file

#include "Acqparam.h"		// data acquisition struct: wave format, wave chans
#include "WaveBuf.h"		// data acquisition buffer
#include "SpikeBuffer.h"
#include "SpikeElement.h"

// CSpikeList CObject
// this object stores spikes from one data acquisition channel
// it contains a description of the original data acqusition set,
// how spikes were initially detected
// parameters associated to each spike and data describing the spikes
// only one data spike buffer (64Kb max) opened at a time
//
// this object has procedures to manage the list of spikes and to
// measure certain parameters on them
// as well as storing/reading them
//
// Two independent structures are associated to manage infos related to individual spikes

class CSpikeList : public CObject
{
public:
	void Serialize(CArchive& ar) override;
	DECLARE_SERIAL(CSpikeList)

	CSpikeList();
	~CSpikeList();
	
	int				m_selspike;
	long			m_lFirstSL;
	long			m_lLastSL;
	int				m_jitterSL;
	int				m_icenter1SL;
	int				m_icenter2SL;
	int				m_imaxmin1SL;
	int				m_imaxmin2SL;
	
// Description 
protected:
// (0) ---------------infos about file version and ID
	WORD			m_wversion{};
	CString			m_IDstring;

// (1) ---------------infos about data acquisition and spike detection ------------------
	CWaveChan		m_acqchan;			// description acquisition channel
	WORD  			m_encoding{};		// data encoding mode  
	long  			m_binzero{};		// 2048
	float 			m_samprate{};		// data acq sampling rate (Hz)
	float 			m_voltsperbin{};	// nb volts per bin (data)
	SPKDETECTPARM	m_parm;				// detection parameters
	CString			m_cscomment;		// spike channel descriptor (unused)

// (2) -------------ordered spike list with class, time, etc-----------------------------

	BOOL			m_bextrema;			// extrema valid / no
	int				m_totalmin;			// min of all spikes
	int				m_totalmax;			// max of all spikes
	CArray<CSpikeElemt*, CSpikeElemt*>	m_spkelmts;	// array of SpikeElemts

// (3) -------------unordered data buffers with spikes extracted from acq data-----------

	CSpikeBuffer	m_spikebuffer;		// spike data buffer

// (4) miscellaneous

	BOOL			m_bsaveartefacts;	// save (yes/no) artefacts - default = FALSE
	BOOL 			m_bvalidclasslist;	// class list (array with classnb & nb spikes/class)
	int				m_nbclasses;		//
	CArray <int, int>	m_classArray;		// CWordArray describing classes found and nb of spikes within them	

// Operations
public:
	WORD	GetVersion()			const {return m_wversion;}
	BOOL	IsClassListValid()		const {return m_bvalidclasslist;}

	int		GetNbclasses()			const {if (m_bvalidclasslist) return m_nbclasses; else return (int) -1;}
	int		GetclassID(int i)		const {return m_classArray.GetAt(i*2);}
	int		GetclassNbspk(int i)	const {return m_classArray.GetAt(i*2+1);}
	void	SetclassNbspk(int no, int nbspk) { m_classArray.SetAt(no*2+1, nbspk);}
	
	int		GetSpikeClass(int no)	const {return m_spkelmts[no]->get_class();}
	long	GetSpikeTime(int no)	const {return m_spkelmts[no]->get_time();}
	int		GetSpikeChan(int no)	const {return m_spkelmts[no]->get_source_channel();}
	void	GetSpikeExtrema(int no, int *max, int *min) {m_spkelmts[no]->GetSpikeExtrema(max, min);}
	void	GetSpikeMaxmin(int no, int *max, int *min, int *dmaxmin) {m_spkelmts[no]->GetSpikeMaxMin(max, min, dmaxmin);}
	int		GetSpikeAmplitudeOffset(int no) const {return m_spkelmts[no]->get_amplitude_offset();}
	int		GetSpikeValAt(int no, int index) const {return *(GetpSpikeData(no)+index);}
	int		GetSpikeLength()		const {return m_spikebuffer.GetSpklen();}
	int		GetTotalSpikes()		const {return m_spkelmts.GetSize();}
	
	void	SetSpikeClass(int no, int nclass) {m_spkelmts[no]->set_class(nclass); m_bvalidclasslist=FALSE;}
	void	SetSpikeTime(int no, long iitime) {m_spkelmts[no]->set_time(iitime);}
	
	CSpikeElemt* GetSpikeElemt(int no) {return m_spkelmts.GetAt(no);}
	
	WORD	GetAcqEncoding()		const {return m_encoding;}
	float	GetAcqSampRate()		const {return m_samprate;}
	float	GetAcqVoltsperBin()		const {return m_voltsperbin;}
	int		GetSpikePretrig()		const {return m_parm.prethreshold;}
	int		GetSpikeRefractory()	const {return m_parm.refractory;}
	int		GetAcqBinzero()			const {return m_binzero;}
	int		GetdetectTransform()	const {return m_parm.detectTransform;}
	int		GetextractNpoints()		const {return m_parm.extractNpoints;}
	int		GetextractChan()		const {return m_parm.extractChan;}
	int		GetextractTransform()	const {return m_parm.extractTransform;}
	int		GetcompensateBaseline() const {return m_parm.compensateBaseline;}
	CString	GetComment()			const {return m_parm.comment;}
	int		GetdetectWhat()			const {return m_parm.detectWhat;}
	int		GetdetectChan()			const {return m_parm.detectChan;}
	int		GetdetectThreshold()	const {return m_parm.detectThreshold;}
	float	GetdetectThresholdmV()	const {return m_parm.detectThresholdmV;}

	void	SetFlagSaveArtefacts(BOOL bflag) {m_bsaveartefacts=bflag;}

	void	SetextractChan(int echan)		{m_parm.extractChan = echan;}
	void	SetdetectChan(int dchan)		{m_parm.detectChan = dchan;}
	void	SetextractTransform(int extractTransform)	{m_parm.extractTransform = extractTransform;}

	void	SetDetectParms(SPKDETECTPARM* pSd) {m_parm = *pSd;}
	SPKDETECTPARM* GetDetectParms() {return &m_parm;}

	int  	AddSpike(short* lpsource, int nchans, long iitime, int sourcechan, int iclass, BOOL bCheck);
	BOOL	TransferDataToSpikeBuffer(int no, short* lpsource, int nchans, BOOL badjust= FALSE);
	short*	GetpSpikeData(int no) const	{return m_spikebuffer.GetSpike(no);}
	int  	RemoveSpike(int spikeindex);
	BOOL	IsAnySpikeAround(long iitime, int jitter, int &spikeindex, int ichan);
	
	void 	MeasureSpikeMaxMin(int no, int* max, int* imax, int* min, int* imin);
	void 	MeasureSpikeMaxMinEx(int no, int* max, int* imax, int* min, int* imin, int ifirst, int ilast);
	void 	MeasureSpikeMaxThenMin(int no, int* max, int* imax, int* min, int* imin);
	void 	MeasureSpikeMaxThenMinEx(int no, int* max, int* imax, int* min, int* imin, int ifirst, int ilast);

	void 	GetTotalMaxMin(BOOL bRecalc, int* max, int* min);
	void 	OffsetSpikeAmplitude(int no, int valfirst, int vallast, int center=0);
	void	CenterSpikeAmplitude(int spkindex, int ifirst, int ilast, WORD method=0);
	BOOL 	InitSpikeList(CAcqDataDoc* pDataFile, SPKDETECTPARM* pFC);
	long	UpdateClassList();
	void	EraseData();
	void	ChangeSpikeClassID(int oldclaID, int newclaID);

	// measure parameters
	void Measure_case0_AmplitudeMinToMax(int t1, int t2);
	void Measure_case1_AmplitudeAtT(int t);
	void Measure_case2_AmplitudeAtT2MinusAtT1(int t1, int t2);

	CSize Measure_Y1_MaxMin();

	BOOL SortSpikeWithY1(CSize fromclass_toclass, CSize timewindow, CSize limits);
	BOOL SortSpikeWithY1AndY2(CSize fromclass_toclass, CSize timewindow, CSize limits1, CSize limits2);

	// deal with list of spikes flagged
protected:
	CArray <int, int> m_spike_flagged;	// store spike number in this array

public:
	int		SetSpikeFlag(int spikeno, BOOL bFlag);
	int		ToggleSpikeFlag(int spikeno);
	void	SetSingleSpikeFlag(int spikeno);
	BOOL	GetSpikeFlag(int spikeno);
	void	RemoveAllSpikeFlags() {m_spike_flagged.RemoveAll();}
	void	FlagRangeOfSpikes(long l_first, long l_last, BOOL bSet);
	void	SelectSpikeswithinRect(int vmin, int vmax, long l_first, long l_ast, BOOL bAdd);

	void	GetRangeOfSpikeFlagged(long& l_first, long& l_last);
	BOOL	GetSpikeFlagArrayAt(int i) const {return static_cast<BOOL>(m_spike_flagged.GetAt(i));}
	int		GetSpikeFlagArrayCount() const {return m_spike_flagged.GetCount();}

protected:	
// Implementation
	void	ReadfileVersion1(CArchive& ar);
	void	RemoveArtefacts();
	void	ReadfileVersion_before5(CArchive& ar, int iversion);
	void	ReadfileVersion5(CArchive& ar);
	void	ReadfileVersion6(CArchive& ar);
	
	void	WritefileVersion6(CArchive& ar);
	void	DeleteArrays();
};
