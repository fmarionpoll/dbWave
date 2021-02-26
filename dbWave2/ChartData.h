#pragma once

#include "Envelope.h"
#include "chanlistitem.h"
#include "chart.h"
#include "AcqDataDoc.h"
#include "Cscale.h"

class CChartDataWnd : public CChartWnd
{
	// Construction
public:
	CChartDataWnd();
	virtual		~CChartDataWnd();
	DECLARE_SERIAL(CChartDataWnd)
	void		Serialize(CArchive& ar) override;

	// data display operations
public:
	BOOL		GetDataFromDoc();
	BOOL		GetDataFromDoc(long l_first);
	BOOL		GetDataFromDoc(long l_first, long l_last);
	BOOL		GetSmoothDataFromDoc(int ioption);
	BOOL		ScrollDataFromDoc(WORD nSBCode);

	int			ResizeChannels(int npixels, long lSize);	// change size of display chans, load data if necessary
	BOOL		AttachDataFile(CAcqDataDoc* pDataFile);
	BOOL		IsDefined() const { return (chanlistitem_ptr_array.GetSize() > 0); } // is data defined?
	CAcqDataDoc* GetpDataFile() const { return m_pDataFile; }

	// display given buffer operation
	void		ADdisplayStart(int chsamplesw);
	void		ADdisplayBuffer(short* lpBuf, long nchsamples);
	void		ADdisplayStop() { m_bADbuffers = FALSE; }

	// export representation of data to the clipboard
	BOOL		CopyAsText(int ioption, int iunit, int nabcissa);
	LPTSTR		GetAsciiLine(LPTSTR lpCopy, int iunit);
	LPTSTR		GetAsciiEnvelope(LPTSTR lpCopy, int iunit);

	// Helper functions
	long		GetNxPixels()		const { return m_npixels; }	// number of pixels defined in this window
	long		GetDataFirst()		const { return m_lxFirst; }	// document index of first abcissa
	long		GetDataLast()		const { return m_lxLast; }	// document index of last abcissa
	long		GetDataSize()		const { return m_lxSize; }	// nb of data pts displayed
	long		GetPageSize()		const { return m_lxPage; }	// size of page increment used to browse through doc
	long		GetLineSize()		const { return m_lxLine; }	// size of line increment used to browse through doc
	long		GetDocumentLast()	const { return m_lxVeryLast; }// index of document's last pt
	long		GetDataOffsetfromPixel(int pix) const { return ((long)pix) * (m_lxLast - m_lxFirst + 1) / ((long)m_displayRect.right) + m_lxFirst; }
	void		UpdatePageLineSize();							// update page and line size parameters

// Attributes
protected:
	// these variables define the curves displayed on the screen (data from doc)
	CAcqDataDoc* m_pDataFile = nullptr;	// pointer to data source file
	CArray<CChanlistItem*, CChanlistItem*>	chanlistitem_ptr_array;	// list of display items (abcissa, Envelope, disp. parms)
	CArray<CEnvelope*, CEnvelope*>			envelope_ptr_array;	// list of Envelopes
	CArray <CPoint, CPoint>					m_PolyPoints;		// array with abcissa & ordinates
	CHighLight								m_highlighted;
	CScale 									m_scale;

	int			m_npixels = 1;			// nb pixels displayed horizontally
	int			m_dataperpixel = 1;		// nb of data point per pixel
	long		m_lxVeryLast = 1;		// end of document
	long		m_lxPage{};				// size of page increment / file index
	long		m_lxLine{};				// size of line increment / file index
	long		m_lxSize = 1;			// nb of data pts represented in a Envelope
	long		m_lxFirst = 0;			// file index of 1rst pt in the Envelopes
	long		m_lxLast = 1;			// file index of last pt in the Envelopes
	float		m_samplingrate{};
	BOOL		m_btrackCurve = false;	// track curve if hit

	int			m_XORnelmts{};			// curve tracking parameters
	int			m_hitcurve{};			// ibid  which curve was hitted if any
	int			m_XORxext{};			// ibid  x extent
	int			m_XORyext{};			// ibid  y extent
	int			m_zero{};				// ibid. zero

	BOOL		m_bADbuffers = false;			// flag when AD buffers are in displayed
	long		m_lADbufferdone{};		// position of incoming data along m_lxSize

	BOOL		m_bPrintHZcursor{};		// default=FALSE
	BOOL		m_btrackspike = false;	// track spike with vertic cursor
	int			m_tracklen = 60;			// length of waveform to track
	int			m_trackoffset = 20;		// offset of waveform from cursor
	int			m_trackchannel = 0;		// lineview channel

// ChanList operations
public:
	int  		GetChanlistSize() const { return chanlistitem_ptr_array.GetSize(); }
	void 		RemoveAllChanlistItems();
	int  		AddChanlistItem(int ns, int mode);
	int  		RemoveChanlistItem(WORD i);

	void		SetChanlistYzero(WORD i, int zero) { chanlistitem_ptr_array[i]->SetYzero(zero); }
	void		SetChanlistYextent(WORD i, int yextent) { chanlistitem_ptr_array[i]->SetYextent(yextent); }
	void		SetChanlistComment(WORD i, CString& comment) { chanlistitem_ptr_array[i]->dl_comment = comment; }
	void		SetChanlistColor(WORD i, int color) { chanlistitem_ptr_array[i]->SetColor(color); }
	void		SetChanlistPenWidth(WORD i, WORD penwidth) { chanlistitem_ptr_array[i]->SetPenWidth(penwidth); }
	void		SetChanlistflagPrintVisible(WORD i, WORD drawmode) { chanlistitem_ptr_array[i]->SetflagPrintVisible(drawmode); }
	int			SetChanlistTransformMode(WORD i, int imode);
	int			SetChanlistSourceChan(WORD i, int ns);
	void		SetChanlistOrdinates(WORD i, int chan, int transform);
	void		SetChanlistVoltsExtent(int chan, const float* pvalue);
	void		SetChanlistVoltsZero(int chan, const float* pvalue);

	CChanlistItem* GetChanlistItem(int i) const { return chanlistitem_ptr_array.GetAt(i); }
	float		GetChanlistVoltsExtent(int chan) { return chanlistitem_ptr_array[chan]->GetDataVoltsSpan(); }
	CString		GetChanlistComment(WORD i) { return chanlistitem_ptr_array[i]->dl_comment; }
	float 		GetChanlistVoltsperDataBin(WORD i) { return chanlistitem_ptr_array[i]->GetVoltsperDataBin(); }
	int 		GetChanlistYextent(WORD i) { return chanlistitem_ptr_array[i]->GetYextent(); }
	int 		GetChanlistYzero(WORD i) { return chanlistitem_ptr_array[i]->GetYzero(); }

	float 		GetChanlistVoltsperPixel(WORD i) { return ((float)chanlistitem_ptr_array[i]->GetYextent() * chanlistitem_ptr_array[i]->GetVoltsperDataBin() / -m_yVE); }
	float 		GetTimeperPixel() { return ((float)(GetDataSize() / m_pDataFile->GetpWaveFormat()->chrate)) / (float)GetRectWidth(); }
	int			GetChanlistColor(WORD i) { return chanlistitem_ptr_array[i]->GetColor(); }
	WORD		GetChanlistPenWidth(WORD i) { return chanlistitem_ptr_array[i]->GetPenWidth(); }
	int			GetChanlistYsource(WORD i) { return chanlistitem_ptr_array[i]->pEnvelopeOrdinates->GetSourceChan(); }
	int 		GetChanlistYmode(WORD i) { return chanlistitem_ptr_array[i]->pEnvelopeOrdinates->GetSourceMode(); }
	WORD		GetChanlistflagPrintVisible(WORD i) { return chanlistitem_ptr_array[i]->GetflagPrintVisible(); }

	int			GetChanlistBinZero(WORD i) { return chanlistitem_ptr_array[i]->GetDataBinZero(); }
	int			GetChanlistBinSpan(WORD i) { return chanlistitem_ptr_array[i]->GetDataBinSpan(); }

	int 		GetChanlistSourceChan(WORD i) { return chanlistitem_ptr_array[i]->pEnvelopeOrdinates->GetSourceChan(); }
	int 		GetChanlistTransformMode(WORD i) { return chanlistitem_ptr_array[i]->pEnvelopeOrdinates->GetSourceMode(); }
	int			GetChanlistBinAt(WORD i, int index) { return chanlistitem_ptr_array[i]->pEnvelopeOrdinates->GetPointAt(index); }
	void  		GetChanlistMaxMin(WORD i, int* pmax, int* pmin) { chanlistitem_ptr_array[i]->pEnvelopeOrdinates->GetEnvelopeMaxMin(pmax, pmin); }

	float		ConvertChanlistDataBinsToVolts(WORD i, int bins) { return chanlistitem_ptr_array[i]->ConvertDataBinsToVolts(bins); }
	float		ConvertChanlistDataBinsToMilliVolts(WORD i, int bins) { return (chanlistitem_ptr_array[i]->ConvertDataBinsToVolts(bins) * 1000.f); }
	int			ConvertChanlistVoltstoDataBins(WORD i, float nvolts) { return chanlistitem_ptr_array[i]->ConvertVoltsToDataBins(nvolts); }

	int			GetChanlistBintoPixel(WORD chan, int bin) { return MulDiv(bin - chanlistitem_ptr_array[chan]->GetYzero(), m_yVE, chanlistitem_ptr_array[chan]->GetYextent()) + m_yVO; }
	int			GetChanlistPixeltoBin(WORD chan, int pixels) { return MulDiv(pixels - m_yVO, chanlistitem_ptr_array[chan]->GetYextent(), m_yVE) + chanlistitem_ptr_array[chan]->GetYzero(); }

	SCOPESTRUCT* GetScopeParameters() override;
	void		SetScopeParameters(SCOPESTRUCT* pStruct) override;

	void		AutoZoomChan(int i);
	void		CenterChan(int i);
	void		MaxgainChan(int i);
	void		SplitChans();
	void 		UpdateChanlistFromDoc();
	void 		UpdateChanlistMaxSpan();
	void 		UpdateGainSettings(int i);
	void		SetHighlightData(CDWordArray* pIntervals);
	void		SetHighlightData(CHighLight& source);
	void		SetTrackSpike(BOOL btrackspike, int tracklen, int trackoffset, int trackchannel);
	void 		MoveHZtagtoVal(int itag, int ival);
	void		UpdateXRuler();
	void		UpdateYRuler();
	void		PlotDatatoDC(CDC* p_dc) override;
	void		ZoomData(CRect* prevRect, CRect* newRect) override;

	void		Print(CDC* p_dc, CRect* rect, BOOL bCenterline = FALSE);

protected:
	void		highlightData(CDC* p_dc, int chan);
	int			doesCursorHitCurve(CPoint point);
	void		curveXOR();
	void		displayVTtags_LValue(CDC* p_dc);
	void		displayHZtags_Chan(CDC* p_dc, int ichan, CChanlistItem* pChan);

	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()
};
