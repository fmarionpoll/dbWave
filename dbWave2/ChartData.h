#pragma once

#include "Envelope.h"
#include "chanlistitem.h"
#include "ChartWnd.h"
#include "AcqDataDoc.h"
#include "Cscale.h"

class ChartData : public ChartWnd
{
public:
	ChartData();
	~ChartData() override;
	DECLARE_SERIAL(ChartData)
	void	Serialize(CArchive& ar) override;

public:
	BOOL	GetDataFromDoc();
	BOOL	GetDataFromDoc(long l_first);
	BOOL	GetDataFromDoc(long l_first, long l_last);
	BOOL	GetSmoothDataFromDoc(int ioption);
	BOOL	ScrollDataFromDoc(WORD nSBCode);

	int		ResizeChannels(int npixels, long lSize); // change size of display chans, load data if necessary
	BOOL	AttachDataFile(AcqDataDoc* pDataFile);
	BOOL	IsDefined() const { return (chanlistitem_ptr_array.GetSize() > 0); } // is data defined?
	AcqDataDoc* GetpDataFile() const { return m_pDataFile; }

	// export representation of data to the clipboard
	BOOL	CopyAsText(int ioption, int iunit, int nabcissa);
	LPTSTR	GetAsciiLine(LPTSTR lpCopy, int iunit);
	LPTSTR	GetAsciiEnvelope(LPTSTR lpCopy, int iunit);

	// Helper functions
	long	GetNxPixels() const { return m_npixels; } // number of pixels defined in this window
	long	GetDataFirstIndex() const { return m_lxFirst; } // document index of first abscissa
	long	GetDataLastIndex() const { return m_lxLast; } // document index of last abscissa
	long	GetDataSize() const { return m_lxSize; } // nb of data pts displayed
	long	GetPageSize() const { return m_lxPage; } // size of page increment used to browse through doc
	long	GetLineSize() const { return m_lxLine; } // size of line increment used to browse through doc
	long	GetDocumentLast() const { return m_lxVeryLast; } // index of document's last pt
	long	GetDataOffsetfromPixel(int pix) const {
				return static_cast<long>(pix) * (m_lxLast - m_lxFirst + 1) / static_cast<long>(m_displayRect.right) + m_lxFirst; }

	void UpdatePageLineSize(); // update page and line size parameters

	// Attributes
protected:
	// these variables define the curves displayed on the screen (data from doc)
	AcqDataDoc*	m_pDataFile = nullptr; // pointer to data source file
	CArray<CChanlistItem*, CChanlistItem*> chanlistitem_ptr_array;
	// list of display items (abscissa, Envelope, disp. parms)
	CArray<CEnvelope*, CEnvelope*> envelope_ptr_array; // list of Envelopes
	CArray<CPoint, CPoint> m_PolyPoints; // array with abscissa & ordinates
	CHighLight m_highlighted;
	CScale m_scale;

	int m_npixels = 1; // nb pixels displayed horizontally
	int m_dataperpixel = 1; // nb of data point per pixel
	long m_lxVeryLast = 1; // end of document
	long m_lxPage{}; // size of page increment / file index
	long m_lxLine{}; // size of line increment / file index
	long m_lxSize = 1; // nb of data pts represented in a Envelope
	long m_lxFirst = 0; // file index of 1rst pt in the Envelopes
	long m_lxLast = 1; // file index of last pt in the Envelopes
	float m_samplingrate{};
	BOOL m_btrackCurve = false; // track curve if hit

	int m_XORnelmts{}; // curve tracking parameters
	int m_hitcurve{}; // ibid  which curve was hitted if any
	int m_XORxext{}; // ibid  x extent
	int m_XORyext{}; // ibid  y extent
	int m_zero{}; // ibid. zero

	BOOL m_bADbuffers = false; // flag when AD buffers are in displayed
	long m_lADbufferdone{}; // position of incoming data along m_lxSize

	BOOL m_bPrintHZcursor{}; // default=FALSE
	BOOL m_btrackspike = false; // track spike with vertic cursor
	int m_tracklen = 60; // length of waveform to track
	int m_trackoffset = 20; // offset of waveform from cursor
	int m_trackchannel = 0; // lineview channel

	// ChanList operations
public:
	int		GetChanlistSize() const { return chanlistitem_ptr_array.GetSize(); }
	void	RemoveAllChanlistItems();
	int		AddChanlistItem(int ns, int mode);
	int		RemoveChanlistItem(int ichan);
	CChanlistItem* GetChanlistItem(int i) const { return chanlistitem_ptr_array.GetAt(i); }

	int		SetChanlistTransformMode(int i, int imode);
	int		SetChanlistSourceChan(int ichan, int acqchan);
	void	SetChanlistOrdinates(int i, int chan, int transform);
	void	SetChanlistVoltsExtent(int ichan, const float* pvalue);
	void	SetChanlistVoltsZero(int ichan, const float* pvalue);

	float GetChanlistVoltsperPixel(int i) const
	{
		const CChanlistItem* pchan = GetChanlistItem(i);
		return (static_cast<float>(pchan->GetYextent()) * pchan->GetVoltsperDataBin() / static_cast<float>(-m_yVE));
	}

	float GetTimeperPixel() const
	{
		return static_cast<float>(GetDataSize()) / m_pDataFile->get_waveformat()->sampling_rate_per_channel / static_cast<float>(
			GetRectWidth());
	}

	int GetChanlistBintoYPixel(int chan, int bin)
	{
		return MulDiv(bin - chanlistitem_ptr_array[chan]->GetYzero(), m_yVE,
		              chanlistitem_ptr_array[chan]->GetYextent()) + m_yVO;
	}

	int GetChanlistYPixeltoBin(int chan, int ypixel_relative_to_top_clientwindow)
	{
		return chanlistitem_ptr_array[chan]->GetYzero() + 
				MulDiv(ypixel_relative_to_top_clientwindow - m_yVO,
					chanlistitem_ptr_array[chan]->GetYextent(), 
					m_yVE);
	}

	SCOPESTRUCT* GetScopeParameters() override;
	void	SetScopeParameters(SCOPESTRUCT* pStruct) override;
	void	AutoZoomChan(int i) const;
	void	CenterChan(int i) const;
	void	MaxgainChan(int i) const;
	void	SplitChans() const;
	void	UpdateChanlistFromDoc();
	void	UpdateChanlistMaxSpan();
	void	UpdateGainSettings(int i);
	void	SetHighlightData(CDWordArray* pIntervals);
	void	SetHighlightData(const CHighLight& source);
	void	SetTrackSpike(BOOL btrackspike, int tracklen, int trackoffset, int trackchannel);
	void	MoveHZtagtoVal(int itag, int ival);
	void	UpdateXRuler();
	void	UpdateYRuler();
	void	PlotDataToDC(CDC* p_dc) override;
	void	ZoomData(CRect* prevRect, CRect* newRect) override;

	void	Print(CDC* p_dc, CRect* rect, BOOL bCenterline = FALSE);

	void	adjust_gain(boolean set_mV_span, float mV_span) const;
	void	load_data_within_window(boolean set_time_span, float t_first, float t_last);
	void	load_all_channels(int data_transform);

protected:
	void	highlight_data(CDC* p_dc, int chan);
	int		does_cursor_hit_curve(CPoint point);
	void	curve_xor();
	void	display_vt_tags_long_value(CDC* p_dc);
	void	display_hz_tags_for_channel(CDC* p_dc, int ichan, const CChanlistItem* pChan);

	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP()
};
