#pragma once

// lineview.h : header file
//
/////////////////////////////////////////////////////////////////////////////


#include "Envelope.h" 
#include "chanlistitem.h"
#include "scopescr.h"
#include "acqdatad.h"

/////////////////////////////////////////////////////////////////////////////
// CLineViewWnd button
class CLineViewWnd : public CScopeScreen
{
// Construction
public:
	CLineViewWnd();
	virtual ~CLineViewWnd();
	DECLARE_SERIAL( CLineViewWnd)
	virtual void Serialize( CArchive& archive );

/////////////////////////////////////////////////////////////////////////////
// data display operations    
public:

	// document operations

	BOOL GetDataFromDoc();							// load data from document
	BOOL GetDataFromDoc(long lFirst);				// load data from doc, from index lFirst, same length
	BOOL GetDataFromDoc(long lFirst, long lLast);	// load data from doc: lFirst to lLast
	BOOL ScrollDataFromDoc(WORD nSBCode);			// load data relative to current data
	BOOL GetSmoothDataFromDoc(int ioption);

	int  ResizeChannels(int npixels, long lSize);	// change size of display chans, load data if necessary	
	BOOL AttachDataFile(CAcqDataDoc* pDataFile, long lSize);
	BOOL IsDefined() const {return (m_pChanArray.GetSize()>0);} // is data defined?
	CAcqDataDoc* GetpDataFile() const {return m_pDataFile;}

	// display given buffer operation	
	void ADdisplayStart(int chsamplesw);
	void ADdisplayStop();	
	void ADdisplayBuffer(short* lpBuf, long nchsamples);
			
	// export representation of data to the clipboard

	BOOL  CopyAsText(int ioption, int iunit, int nabcissa);
	LPTSTR GetAsciiLine (LPTSTR lpCopy, int iunit);
	LPTSTR GetAsciiEnvelope (LPTSTR lpCopy, int iunit);

	// Helper functions	

	long GetNxPixels() const	{return m_npixels;}	// number of pixels defined in this window
	long GetDataFirst() const	{return m_lxFirst;}	// document index of first abcissa
	long GetDataLast() const	{return m_lxLast;}	// document index of last abcissa
	long GetDataSize() const	{return m_lxSize;}	// nb of data pts displayed
	long GetPageSize() const	{return m_lxPage;}	// size of page increment used to browse through doc
	long GetLineSize() const	{return m_lxLine;}	// size of line increment used to browse through doc	
	void UpdatePageLineSize();						// update page and line size parameters
	long GetDocumentLast() const {return m_lxVeryLast;}// index of document's last pt
	long GetDataOffsetfromPixel(int pix) const
								{return ((long)pix)*(m_lxLast-m_lxFirst+1)/((long)m_displayRect.right) + m_lxFirst;}

// Attributes
protected:
	// these variables define the curves displayed on the screen (data from doc)
	CDWordArray m_PolyPoints;		// array with abcissa & ordinates
	CPtrArray	m_pChanArray;		// list of display items (abcissa, Envelope, disp. parms)
	CPtrArray	m_pEnvelopesArray;	// list of Envelopes
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ChanList operations
////////////////////////////////////////////////////////////////////////////////////////////////////////////

public:
	int  	GetChanlistSize() const {return m_pChanArray.GetSize();} // nb of displ channels
	void 	RemoveAllChanlistItems();			// remove all channels displayed
	int  	AddChanlistItem(int ns, int mode);	// add one channel
	int  	RemoveChanlistItem(WORD i);			// remove chan(i)
	
	// GET/set binary parms
	void 	SetChanlistZero(WORD i, int zero);
	void 	SetChanlistYextent(WORD i, int yextent);
	void 	SetChanlistComment(WORD i, CString& comment);
	void	SetChanlistColor(WORD i, int color);
	void	SetChanlistPenWidth(WORD i, WORD penwidth);
	void	SetChanlistflagPrintVisible(WORD i, WORD drawmode);
	int		SetChanlistTransformMode(WORD i, int imode);
	int		SetChanlistSourceChan(WORD i, int ns);	

	void	SetChanlistOrdinates(WORD i, int chan, int transform);
	void	SetChanlistVoltsExtent(int chan, float* pvalue);
	void	SetChanlistVoltsMaxMin(int ichan, float vMax, float vMin);
	void	SetChanlistmilliVoltsMaxMin (int chan, float vMax, float vMin);

	float	GetChanlistVoltsExtent(int chan); 
	CString	GetChanlistComment(WORD i);
	float 	GetChanlistVoltsperBin(WORD i);
	int 	GetChanlistYextent(WORD i);
	int 	GetChanlistYzero(WORD i);
	float 	GetChanlistVoltsperPixel(WORD i);
	float 	GetTimeperPixel();
	int		GetChanlistColor(WORD i);
	WORD	GetChanlistPenWidth(WORD i);
	int		GetChanlistYsource(WORD i);
	int 	GetChanlistYmode(WORD i);
	WORD	GetChanlistflagPrintVisible(WORD i);
	int		GetChanlistBinZero(WORD i);
	int 	GetChanlistSourceChan(WORD i);
	int 	GetChanlistTransformMode(WORD i);
	int		GetChanlistBinAt(WORD i, int index);
	void  	GetChanlistMaxMin(WORD i, int* max, int* min);
	float	GetChanlistBintoVolts(WORD i, int bins);
	float	GetChanlistBintoMilliVolts(WORD i, int bins);
	int		GetChanlistVoltstoBins(WORD i, float nvolts);
	int		GetChanlistMilliVoltstoBins(WORD i, float millivolts);
	
	int		FromChanlistBintoPixel(WORD chan, int bin);
	int		FromChanlistPixeltoBin(WORD chan, int pixels);

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

					
////////////////////////////////////////////////////////////////////////////////////////////////////////////
// mouse cursor operations
////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	void 	MoveHZtagtoVal(int itag, int ival);	

protected:
	// implementation
	void	ZoomData(CRect* prevRect, CRect* newRect);	// zoom display	
public:
	void	PlotDatatoDC(CDC* pDC);
protected:
	int		DoesCursorHitCurve(CPoint point);	
	void	XORcurve();
	void	XORHZcur(CPoint point);	

////////////////////////////////////////////////////////////////////////////////////////////////////////////
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

