#pragma once

//  spikepar.h  acquisition parameters
//

////////////////////////////////////////////////////////////////////////
// STIMDETECT				version 1
// SPKDETECTPARM			version 1
// SPKDETECTARRAY			version 1
// SPKCLASSIF				version 1
////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------
// stimulus detection

class STIMDETECT : public CObject
{
	DECLARE_SERIAL(STIMDETECT)
public:
	BOOL bChanged;			// flag set TRUE if contents has changed
	WORD wversion;			// version number

	int	 nItems;			// number of items/line (nb of chans/detection) ?
	int  SourceChan;		// source channel
	int	 TransformMethod;	// detect from data transformed - i = transform method cf CAcqDataDoc
	int  DetectMethod;		// unused
	int	 Threshold1;		// value of threshold 1	
	int	 bMode;				// 0: ON/OFF (up/down); 1: OFF/ON (down/up); 2: ON/ON (up/up); 3: OFF/OFF (down, down); 

public :
	STIMDETECT();			// constructor
	~STIMDETECT();			// destructor
	STIMDETECT& operator = (const STIMDETECT& arg);// operator redefinition
	virtual void Serialize(CArchive& ar);	// overridden for document i/o
};

// ---------------------------------------------------------------------
// spike detection parameters

class SPKDETECTPARM : public CObject
{
	DECLARE_SERIAL(SPKDETECTPARM)
public:
	BOOL	bChanged;			// flag set TRUE if contents has changed
	WORD	wversion;			// version number

	CString	comment;			// free comment (for ex describing the method)

	int		detectWhat;			// 0=spikes, 1=stimulus
	int		detectMode;			// stim: 0: ON/OFF (up/down); 1: OFF/ON (down/up); 2: ON/ON (up/up); 3: OFF/OFF (down, down); 
	int		detectFrom;			// detection method - (stored as WORD): 0=chan; 1 = tag
	int		detectChan;			// source channel
	int		detectTransform;	// detect from data transformed - i = transform method cf CAcqDataDoc
	int		detectThreshold;	// value of threshold
	float	detectThresholdmV;	// mV value of threshold
	BOOL	compensateBaseline;	// compensate for baseline shift?

	int		extractChan;		// channel number of data to extract
	int		extractTransform;	// transformed data extracted
	int		extractNpoints;		// spike length (n data pts)
	int		prethreshold;		// offset spike npts before threshold
	int		refractory;			// re-start detection n pts after threshold	

protected:
	void	ReadVersionlessthan6(CArchive& ar, int version);
	void	ReadVersion6(CArchive& ar);
	void	ReadVersion7(CArchive& ar);
	
public :
	SPKDETECTPARM();
	~SPKDETECTPARM();
	SPKDETECTPARM& operator = (const SPKDETECTPARM& arg);
	virtual void Serialize(CArchive& ar);
};

// ---------------------------------------------------------------------
// spike detection parameters

class CSpkDetectArray : public CObject
{
	DECLARE_SERIAL(CSpkDetectArray)
public:
	BOOL		bChanged;							// flag set TRUE if contents has changed
	int			AddItem();							// add one parameter array item
	int			RemoveItem(int i);
	SPKDETECTPARM* GetItem(int i) {return spkdetectparm_ptr_array.GetAt(i);}
	void		SetItem(int i, SPKDETECTPARM* pSD) {*(spkdetectparm_ptr_array[i]) = *pSD;}
	int			GetSize() {return spkdetectparm_ptr_array.GetSize();}
	void		SetSize(int nitems);

protected:
	WORD		wversion;							// version number
	void		DeleteArray();
	CArray <SPKDETECTPARM*, SPKDETECTPARM*> spkdetectparm_ptr_array;							// array with SPKDETECTPARM objects

public :
	CSpkDetectArray();
	virtual			~CSpkDetectArray();
	CSpkDetectArray& operator	= (const CSpkDetectArray& arg);	// operator redefinition
	virtual void	Serialize(CArchive& ar);		// overridden for document i/o
	void			Serialize_Load(CArchive& ar, WORD wversion);
};

// ---------------------------------------------------------------------
// array of spike detection parameters
// 2 dimensions

class SPKDETECTARRAY : public CObject
{
	DECLARE_SERIAL(SPKDETECTARRAY)
public:
	BOOL bChanged;										// flag set TRUE if contents has changed
	void				SetChanArray(int acqchan, CSpkDetectArray* pspk);
	CSpkDetectArray*	GetChanArray(int acqchan);

protected:
	WORD				wversion;						// version number
	CMapWordToPtr		chanArrayMap;					// array of CPtrArray
	void				DeleteAll();					// erase all arrays of parmItems (and all parmItems within them)

public :
	SPKDETECTARRAY();
	virtual		~SPKDETECTARRAY();
	virtual void Serialize(CArchive& ar);				// overridden for document i/o
};


// ---------------------------------------------------------------------
// spike classification parameters

class SPKCLASSIF : public CObject
{
	DECLARE_SERIAL(SPKCLASSIF)
public:
	BOOL	bChanged;			// flag set TRUE if contents has changed
	WORD	wversion;			// version number

	// sort with parameters
	int		dataTransform;		// dummy
	int		iparameter;			// type of parameter measured
	int		ileft;				// position of first cursor
	int		iright;				// position of second cursor
	int		ilower;				// binary val of lower
	int		iupper;				// binary val of upper
	int		ixyleft;			// position of left cursor on xy display
	int		ixyright;			// position of right cursor on xy display
	int		sourceclass;		// source class
	int		destclass;			// destination class

	// display parameters for spikeview
	int		rowheight;
	int		coltext;
	int		colspikes;
	int		colseparator;
	int		vsourceclass;
	int		vdestclass;
	float	fjitter_ms;
	BOOL	bresetzoom;

	// sort with templates
	int		nintparms;
	int		hitrate;
	int		hitratesort;
	int		kleft;
	int		kright;

	int		nfloatparms;
	float	ktolerance;
	float	mvmin;			// display limits
	float	mvmax;
	void*	ptpl;

public :
	SPKCLASSIF();	// constructor
	~SPKCLASSIF();	// destructor
	SPKCLASSIF& operator = (const SPKCLASSIF& arg);	// operator redefinition
	virtual void Serialize(CArchive& ar);	// overridden for document i/o
	void CreateTPL();	
};
