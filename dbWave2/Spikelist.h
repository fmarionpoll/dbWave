#pragma once

// spikelist.h : header file


#include "Acqparam.h"		// data acquisition struct: wave format, wave chans
#include "Taglines.h"		// tags
#include "datafile_X.h"		// generic data file
#include "wavebuf.h"		// data acquisition buffer
#include "awavepar.h"		// user parameters

class CAcqDataDoc;

/////////////////////////////////////////////////////////////////////////////
// CFromChan structure
// parameters used for detection of spikes from data acquisition
// simple object - serialized

class CFromChan : public CObject
{
	DECLARE_SERIAL(CFromChan)
public :
	CFromChan();					// constructor used by dynamic creation
public:
	WORD			wversion;		// version number of the structure    
	WORD  			encoding;		// data encoding mode  
	long  			binzero;		// 2048
	float 			samprate;		// data acq sampling rate (Hz)
	float 			voltsperbin;	// nb volts per bin (data)
	CString			comment;		// spike channel descriptor

	SPKDETECTPARM	parm;			// detection parameters

// Implementation
public:
	virtual ~CFromChan();
	virtual void Serialize(CArchive& ar);	// overridden for document i/o
	void operator= (const CFromChan& arg);	// redefinition operator =
};


/////////////////////////////////////////////////////////////////////////////
// CSpikeBuffer command target

class CSpikeBuffer : public CObject
{   
	DECLARE_SERIAL(CSpikeBuffer)
public:
	CSpikeBuffer();			// protected constructor used by dynamic creation
	CSpikeBuffer(int lenspik);
	virtual ~CSpikeBuffer();

// Attributes
public:
	CDWordArray	m_idata;			// pointers to individual spike data
	WORD		m_lenspk;			// length of one spike
	int			m_spkbufferincrement;//
	int			m_spkbufferlength;	// current buffer length	
	short*		m_pspkbuffer;		// buffer address
	int			m_currentfilter;	// ID of last transform
	int			m_nextindex;		// next available index	
	int			m_lastindex;		// index last free space for spike data
	int			m_binzero;			// zero... default = 2048

// Operations
public:
	void 		SetSpklen(int lenspik);
	inline int 	GetSpklen() const {return m_lenspk;}

// Implementation
public:
	virtual void Serialize(CArchive& ar);	// overridden for document i/o    
	void 	RemoveAllBuffers();
	short* 	AddSpikeBuf(WORD spkindex);
	short*	AddNumbersofSpikes(WORD spkindex);
	BOOL  	DeleteSpike(WORD spkindex);
	BOOL  	ExchangeSpikes(WORD spk1, WORD spk2);
	inline 	short* GetSpike(int index) const {return (m_pspkbuffer +m_idata[index]);}

protected:
	void	LPD2f3();
};


/////////////////////////////////////////////////////////////////////////////
// CSpikeElemt CObject
// this object is a serialized structure containing parameters associated to
// each spike detected from a data document
// stores:  time of occurence, initial data acq chan and a modifiable parameter
// the class
// this basic object is part of a CSpikelist object that stores parameters 
// concerning the source document (file, date, acq rate and parms, etc) and
// eventually the source data extracted from the data file
// real data are stored in a separate object managing data buffers

class CSpikeElemt : public CObject
{
	DECLARE_SERIAL(CSpikeElemt)
public :
	CSpikeElemt();			// protected constructor used by dynamic creation
	CSpikeElemt(LONG time, WORD channel); // create element and init
	CSpikeElemt(LONG time, WORD channel, int max, int min, int offset, int iclass, int dmaxmin);

// Attributes
private:
	long	m_iitime;		// occurence time - multiply by rate to get time in seconds
	int		m_class;		// spike class - init to zero at first
	int		m_chanparm;		// spike detection array index
	int		m_max;			// spike max	(used to scan rapidly to adjust display)
	int		m_min;			// min val		(used to scan rapidly to adjust display)
	int		m_dmaxmin;
	int		m_offset;		// offset voltage pt 1

// Operations set/change elements of spikeele
public:
	inline long		GetSpikeTime() const				{return m_iitime;}
	inline int		GetSpikeClass() const				{return m_class;}
	inline int		GetSpikeChannel() const				{return m_chanparm;}
	inline short	GetSpikeMax() const					{return m_max;}
	inline short	GetSpikeMin() const					{return m_min;}
	inline int		GetSpikeAmplitudeOffset() const		{return m_offset;}
	inline void		GetSpikeMaxMin  (int *max, int *min, int *dmaxmin) {*max = m_max; *min = m_min; *dmaxmin= m_dmaxmin;}
	inline void		GetSpikeExtrema (int *max, int *min)	{*max = m_max; *min = m_min;}


	inline void		SetSpikeTime(long ii)				{m_iitime=ii;}
	inline void		SetSpikeClass(int cl)				{m_class=cl; }
	inline void		SetSpikeMaxMin(int max, int min, int dmaxmin)	{m_max=max; m_min=min; m_dmaxmin=dmaxmin;}
	inline void		SetSpikeAmplitudeOffset(int offset) {m_offset= offset;}
		
// Implementation
public:
	virtual ~CSpikeElemt();
	virtual void Serialize(CArchive& ar);	// overridden for document i/o
	void	Read0(CArchive& ar);
};


/////////////////////////////////////////////////////////////////////////////
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
	void Serialize(CArchive& ar);	// overridden for document i/o
	DECLARE_SERIAL(CSpikeList)

	CSpikeList();					// constructor used by dynamic creation
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
	WORD			m_wversion;
	CString			m_IDstring;

// (1) ---------------infos about data acquisition and spike detection ------------------
	CWaveChan		m_acqchan;			// description acquisition channel
	WORD  			m_encoding;			// data encoding mode  
	long  			m_binzero;			// 2048
	float 			m_samprate;			// data acq sampling rate (Hz)
	float 			m_voltsperbin;		// nb volts per bin (data)
	SPKDETECTPARM	m_parm;				// detection parameters
	CString			m_cscomment;		// spike channel descriptor (unused)

// (2) -------------ordered spike list with class, time, etc-----------------------------

	BOOL			m_bextrema;			// extrema valid / no
	int				m_totalmin;			// min of all spikes
	int				m_totalmax;			// max of all spikes
	CObArray		m_spkelmts;			// array of SpikeElemts

// (3) -------------unordered data buffers with spikes extracted from acq data-----------

	CSpikeBuffer	m_spikebuffer;		// spike data buffer

// (4) miscellaneous

	BOOL			m_bsaveartefacts;	// save (yes/no) artefacts - default = FALSE
	BOOL 			m_bvalidclasslist;	// class list (array with classnb & nb spikes/class)
	int				m_nbclasses;		//
	CUIntArray		m_classArray;		// CWordArray describing classes found and nb of spikes within them	

// Operations
public:
	inline WORD		GetVersion()			const {return m_wversion;}
	inline BOOL		IsClassListValid()		const {return m_bvalidclasslist;}

	inline int		GetNbclasses()			const {if (m_bvalidclasslist) return m_nbclasses; else return (int) -1;}
	inline int		GetclassID(int i)		const {return m_classArray.GetAt(i*2);}
	inline int		GetclassNbspk(int i)	const {return m_classArray.GetAt(i*2+1);}
	inline void		SetclassNbspk(int no, int nbspk) { m_classArray.SetAt(no*2+1, nbspk);}
	
	inline int		GetSpikeClass(int no)	const {return ((CSpikeElemt *)m_spkelmts[no])->GetSpikeClass();}
	inline long		GetSpikeTime(int no)	const {return ((CSpikeElemt *)m_spkelmts[no])->GetSpikeTime();}
	inline int		GetSpikeChan(int no)	const {return ((CSpikeElemt *)m_spkelmts[no])->GetSpikeChannel();}
	inline void		GetSpikeExtrema(int no, int *max, int *min) {((CSpikeElemt*)m_spkelmts[no])->GetSpikeExtrema(max, min);}
	inline void		GetSpikeMaxmin(int no, int *max, int *min, int *dmaxmin) {((CSpikeElemt*)m_spkelmts[no])->GetSpikeMaxMin(max, min, dmaxmin);}
	inline int		GetSpikeAmplitudeOffset(int no) const {return ((CSpikeElemt*)m_spkelmts[no])->GetSpikeAmplitudeOffset();}
	inline int		GetSpikeValAt(int no, int index) const {return *(GetpSpikeData(no)+index);}
	inline int		GetSpikeLength()		const {return m_spikebuffer.GetSpklen();}
	inline int		GetTotalSpikes()		const {return m_spkelmts.GetSize();}
	
	inline void		SetSpikeClass(int no, int nclass) {((CSpikeElemt*)m_spkelmts[no])->SetSpikeClass(nclass); m_bvalidclasslist=FALSE;}
	inline void		SetSpikeTime(int no, long iitime) {((CSpikeElemt*)m_spkelmts[no])->SetSpikeTime(iitime);}
	
	inline CSpikeElemt* GetSpikeElemt(int no) {return (CSpikeElemt*) m_spkelmts.GetAt(no);}
	
	inline WORD		GetAcqEncoding()		const {return m_encoding;}
	inline float	GetAcqSampRate()		const {return m_samprate;}
	inline float	GetAcqVoltsperBin()		const {return m_voltsperbin;}
	inline int		GetSpikePretrig()		const {return m_parm.prethreshold;}
	inline int		GetSpikeRefractory()	const {return m_parm.refractory;}
	inline int		GetAcqBinzero()			const {return m_binzero;}
	inline int		GetdetectTransform()	const {return m_parm.detectTransform;}
	inline int		GetextractNpoints()		const {return m_parm.extractNpoints;}
	inline int		GetextractChan()		const {return m_parm.extractChan;}
	inline int		GetextractTransform()	const {return m_parm.extractTransform;}
	inline int		GetcompensateBaseline() const {return m_parm.compensateBaseline;}
	inline CString	GetComment()			const {return m_parm.comment;}
	inline int		GetdetectWhat()			const {return m_parm.detectWhat;}
	inline int		GetdetectChan()			const {return m_parm.detectChan;}
	inline int		GetdetectThreshold()	const {return m_parm.detectThreshold;}
	inline float	GetdetectThresholdmV()	const {return m_parm.detectThresholdmV;}

	inline void		SetFlagSaveArtefacts(BOOL bflag) {m_bsaveartefacts=bflag;}

	inline void		SetextractChan(int echan)		{m_parm.extractChan = echan;}
	inline void		SetdetectChan(int dchan)		{m_parm.detectChan = dchan;}
	inline void		SetextractTransform(int extractTransform)	{m_parm.extractTransform = extractTransform;}

	inline void		SetDetectParms(SPKDETECTPARM* pSd) {m_parm = *pSd;}
	inline SPKDETECTPARM* GetDetectParms() {return &m_parm;}

	int  			AddSpike(short* lpsource, WORD nchans, long iitime, int sourcechan, int iclass, BOOL bCheck);
	BOOL			SetSpikeData(WORD no, short* lpsource, int nchans, BOOL badjust= FALSE);
	inline short*	GetpSpikeData(WORD no) const	{return m_spikebuffer.GetSpike(no);}
	int  			RemoveSpike(int spikeindex);
	BOOL			IsAnySpikeAround(long iitime, int jitter, int &spikeindex, int ichan);
	
	void 			MeasureSpikeMaxMin(WORD no, int* max, int* imax, int* min, int* imin);
	void 			MeasureSpikeMaxMinEx(WORD no, int* max, int* imax, int* min, int* imin, int ifirst, int ilast);
	void 			MeasureSpikeMaxThenMin(WORD no, int* max, int* imax, int* min, int* imin);
	void 			MeasureSpikeMaxThenMinEx(WORD no, int* max, int* imax, int* min, int* imin, int ifirst, int ilast);

	void 			GetTotalMaxMin(BOOL bRecalc, int* max, int* min);
	void 			OffsetSpikeAmplitude(WORD no, int valfirst, int vallast, int center=0);
	void			CenterSpikeAmplitude(WORD spkindex, int ifirst, int ilast, WORD method=0);
	BOOL 			InitSpikeList(CAcqDataDoc* pDataFile, SPKDETECTPARM* pFC);
	long			UpdateClassList();
	void			EraseData();
	void			ChangeSpikeClassID(int oldclaID, int newclaID);

	// deal with list of spikes flagged
protected:
	CUIntArray		m_bSpikeFlagArray;	// store spike number in this array

public:
	int				SetSpikeFlag(int spikeno, BOOL bFlag);
	int				ToggleSpikeFlag(int spikeno);
	void			SetSingleSpikeFlag(int spikeno);
	BOOL			GetSpikeFlag(int spikeno);
	void			RemoveAllSpikeFlags() {m_bSpikeFlagArray.RemoveAll();}
	void			FlagRangeOfSpikes(long lFirst, long lLast, BOOL bSet);
	void			SelectSpikeswithinRect(int vmin, int vmax, long lFirst, long lLast, BOOL bAdd);

	void			GetRangeOfSpikeFlagged(long& lFirst, long& lLast);
	BOOL			GetSpikeFlagArrayAt(int i) const {return (BOOL) m_bSpikeFlagArray.GetAt(i);}
	inline int		GetSpikeFlagArrayCount() const {return m_bSpikeFlagArray.GetCount();}

protected:	
// Implementation
	void   ReadfileVersion1(CArchive& ar);
	void   ReadfileVersion_before5(CArchive& ar, int iversion);
	void   ReadfileVersion5(CArchive& ar);
	void   ReadfileVersion6(CArchive& ar);
	
	void   WritefileVersion6(CArchive& ar);

protected:
	void DeleteArrays();
};
