#pragma once

#include "Envelope.h" 
#include "chanlistitem.h"
#include "scopescr.h"
#include "acqdatad.h"

class CLineViewWnd : public CScopeScreen
{
// Construction
public:
	CLineViewWnd();
	virtual ~CLineViewWnd();
	DECLARE_SERIAL( CLineViewWnd)
	virtual void Serialize( CArchive& archive );

// data display operations    
public:
	BOOL GetDataFromDoc();							// load data from document
	BOOL GetDataFromDoc(long lFirst);				// load data from doc, from index lFirst, same length
	BOOL GetDataFromDoc(long lFirst, long lLast);	// load data from doc: lFirst to lLast
	BOOL ScrollDataFromDoc(WORD nSBCode);			// load data relative to current data
	BOOL GetSmoothDataFromDoc(int ioption);

	int  ResizeChannels(int npixels, long lSize);	// change size of display chans, load data if necessary	
	BOOL AttachDataFile(CAcqDataDoc* pDataFile, long lSize);
	BOOL IsDefined() const {return (m_pChanlistItemArray.GetSize()>0);} // is data defined?
	CAcqDataDoc* GetpDataFile() const {return m_pDataFile;}

	// display given buffer operation	
	void ADdisplayStart(int chsamplesw);
	void ADdisplayBuffer(short* lpBuf, long nchsamples);
	inline void ADdisplayStop() { m_bADbuffers = FALSE; }
			
	// export representation of data to the clipboard
	BOOL	CopyAsText(int ioption, int iunit, int nabcissa);
	LPTSTR	GetAsciiLine (LPTSTR lpCopy, int iunit);
	LPTSTR	GetAsciiEnvelope (LPTSTR lpCopy, int iunit);

	// Helper functions	
	inline long GetNxPixels()		const {return m_npixels;}	// number of pixels defined in this window
	inline long GetDataFirst()		const {return m_lxFirst;}	// document index of first abcissa
	inline long GetDataLast()		const {return m_lxLast;}	// document index of last abcissa
	inline long GetDataSize()		const {return m_lxSize;}	// nb of data pts displayed
	inline long GetPageSize()		const {return m_lxPage;}	// size of page increment used to browse through doc
	inline long GetLineSize()		const {return m_lxLine;}	// size of line increment used to browse through doc	
	inline long GetDocumentLast()	const {return m_lxVeryLast;}// index of document's last pt
	inline long GetDataOffsetfromPixel(int pix) const {return ((long)pix)*(m_lxLast-m_lxFirst+1)/((long)m_displayRect.right) + m_lxFirst;}
	void UpdatePageLineSize();									// update page and line size parameters

// Attributes
protected:
	// these variables define the curves displayed on the screen (data from doc)
	CDWordArray m_PolyPoints;		// array with abcissa & ordinates
	CArray<CChanlistItem*, CChanlistItem*>	m_pChanlistItemArray;	// list of display items (abcissa, Envelope, disp. parms)
	CArray<CEnvelope*, CEnvelope*>			m_pEnvelopesArray;		// list of Envelopes
	CScale 		m_scale;			// scale (array and a few parameters)   
	CDWordArray* m_pDWintervals;	// intervals of data that are highlighted
	CAcqDataDoc* m_pDataFile;		// pointer to data source file

	int		m_npixels;				// nb pixels displayed horizontally
	int		m_dataperpixel;			// nb of data point per pixel	
	long	m_lxVeryLast;			// end of document
	long	m_lxPage;				// size of page increment / file index
	long	m_lxLine;				// size of line increment / file index
	long	m_lxSize;				// nb of data pts represented in a Envelope
	long	m_lxFirst;				// file index of 1rst pt in the Envelopes
	long	m_lxLast;				// file index of last pt in the Envelopes
	BOOL	m_btrackCurve;			// track curve if hit	

	int		m_XORnelmts;			// curve tracking parameters
	int		m_hitcurve;				// ibid  which curve was hitted if any
	int		m_XORxext;				// ibid  x extent
	int		m_XORyext;				// ibid  y extent
	int		m_zero;					// ibid. zero

	BOOL	m_bADbuffers;			// flag when AD buffers are in displayed
	long	m_lADbufferdone;		// position of incoming data along m_lxSize

	BOOL	m_bPrintHZcursor;		// default=FALSE
	BOOL	m_btrackspike;			// track spike with vertic cursor
	int		m_tracklen;				// length of waveform to track
	int		m_trackoffset;			// offset of waveform from cursor
	int		m_trackchannel;			// lineview channel	

// ChanList operations
public:
	int  	GetChanlistSize() const {return m_pChanlistItemArray.GetSize();} // nb of displ channels
	void 	RemoveAllChanlistItems();			// remove all channels displayed
	int  	AddChanlistItem(int ns, int mode);	// add one channel
	int  	RemoveChanlistItem(WORD i);			// remove chan(i)
	inline CChanlistItem* GetChanlistItem(int i) const {return m_pChanlistItemArray.GetAt(i); }
	
	// GET/set binary parms
	inline void SetChanlistYzero(WORD i, int zero) {m_pChanlistItemArray[i]->SetYzero(zero);}
	inline void SetChanlistYextent(WORD i, int yextent) {m_pChanlistItemArray[i]->SetYextent(yextent);}
	inline void SetChanlistComment(WORD i, CString& comment) {m_pChanlistItemArray[i]->dl_comment = comment;}
	inline void	SetChanlistColor(WORD i, int color) {m_pChanlistItemArray[i]->SetColor(color);}
	inline void	SetChanlistPenWidth(WORD i, WORD penwidth) {m_pChanlistItemArray[i]->SetPenWidth(penwidth);}
	inline void	SetChanlistflagPrintVisible(WORD i, WORD drawmode) {m_pChanlistItemArray[i]->SetflagPrintVisible(drawmode);}
	int		SetChanlistTransformMode(WORD i, int imode);
	int		SetChanlistSourceChan(WORD i, int ns);	

	void	SetChanlistOrdinates(WORD i, int chan, int transform);
	void	SetChanlistVoltsExtent(int chan, float* pvalue);
	void	SetChanlistVoltsZero(int chan, float * pvalue);

	inline float	GetChanlistVoltsExtent(int chan) {return m_pChanlistItemArray[chan]->GetDataVoltsSpan(); }
	inline CString	GetChanlistComment(WORD i) {return m_pChanlistItemArray[i]->dl_comment;}
	inline float 	GetChanlistVoltsperDataBin(WORD i) {return m_pChanlistItemArray[i]->GetVoltsperDataBin();}
	inline int 		GetChanlistYextent(WORD i) {return m_pChanlistItemArray[i]->GetYextent();}
	inline int 		GetChanlistYzero(WORD i) {return m_pChanlistItemArray[i]->GetYzero();}

	inline float 	GetChanlistVoltsperPixel(WORD i) {return ((float)m_pChanlistItemArray[i]->GetYextent() * m_pChanlistItemArray[i]->GetVoltsperDataBin() / -m_yVE);}
	inline float 	GetTimeperPixel() {return ((float)(GetDataSize() / m_pDataFile->GetpWaveFormat()->chrate)) / (float)Width();}
	inline int		GetChanlistColor(WORD i) {return m_pChanlistItemArray[i]->GetColor();}
	inline WORD		GetChanlistPenWidth(WORD i) {return m_pChanlistItemArray[i]->GetPenWidth();}
	inline int		GetChanlistYsource(WORD i) {return m_pChanlistItemArray[i]->pEnvelopeOrdinates->GetSourceChan();}
	inline int 		GetChanlistYmode(WORD i) {return m_pChanlistItemArray[i]->pEnvelopeOrdinates->GetSourceMode();}
	inline WORD		GetChanlistflagPrintVisible(WORD i) {return m_pChanlistItemArray[i]->GetflagPrintVisible();}

	inline int		GetChanlistBinZero(WORD i) {return m_pChanlistItemArray[i]->GetDataBinZero();}
	inline int		GetChanlistBinSpan(WORD i) { return m_pChanlistItemArray[i]->GetDataBinSpan(); }

	inline int 		GetChanlistSourceChan(WORD i) {return m_pChanlistItemArray[i]->pEnvelopeOrdinates->GetSourceChan();}
	inline int 		GetChanlistTransformMode(WORD i) {return m_pChanlistItemArray[i]->pEnvelopeOrdinates->GetSourceMode();}
	inline int		GetChanlistBinAt(WORD i, int index) {return m_pChanlistItemArray[i]->pEnvelopeOrdinates->GetPointAt(index);}
	inline void  	GetChanlistMaxMin(WORD i, int* pmax, int* pmin) {m_pChanlistItemArray[i]->pEnvelopeOrdinates->GetEnvelopeMaxMin(pmax, pmin);}
	inline float	GetChanlistBinsToVolts(WORD i, int bins) {return m_pChanlistItemArray[i]->ConvertDataBinsToVolts(bins);}
	inline float	GetChanlistBinsToMilliVolts(WORD i, int bins) {return (m_pChanlistItemArray[i]->ConvertDataBinsToVolts(bins)*1000.f);}
	inline int		GetChanlistVoltstoBins(WORD i, float nvolts) {return m_pChanlistItemArray[i]->ConvertVoltsToDataBins(nvolts);}
	
	inline int		GetChanlistBintoPixel(WORD chan, int bin) { return MulDiv(bin - m_pChanlistItemArray[chan]->GetYzero(), m_yVE, m_pChanlistItemArray[chan]->GetYextent()) + m_yVO; }
	inline int		GetChanlistPixeltoBin(WORD chan, int pixels) {return MulDiv(pixels - m_yVO, m_pChanlistItemArray[chan]->GetYextent(), m_yVE) + m_pChanlistItemArray[chan]->GetYzero();}

	void	AutoZoomChan(int i);	// maximize gain and offset to center curve
	void	CenterChan(int i);		// center curve within button area
	void	MaxgainChan(int i);		// maximize gain
	void	SplitChans();			// split chans - adjust gain and offset

	void 	UpdateChanlistFromDoc();
	void 	UpdateChanlistMaxSpan();
	void 	UpdateGainSettings(int i);	

	void	SetHighlightData(CDWordArray* pIntervals);
	void	SetTrackSpike(BOOL btrackspike, int tracklen, int trackoffset, int trackchannel);

protected:
	void	HighlightData(CDC* pDC, int chan);	
		
// mouse cursor operations
public:
	void 	MoveHZtagtoVal(int itag, int ival);	
	void	PlotDatatoDC(CDC* pDC);
protected:
	int		DoesCursorHitCurve(CPoint point);	
	void	XORcurve();
	void	ZoomData(CRect* prevRect, CRect* newRect);	// zoom display

// overloaded functions	
public:	
	void Print(CDC* pDC, CRect* rect, BOOL bCenterline=FALSE);	// print curve inside a rectangle

protected:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()
};

