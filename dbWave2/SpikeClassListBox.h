#pragma once

class CSpikeClassListBox : public CListBox
{
public:
	CSpikeClassListBox();
	~CSpikeClassListBox() override;
protected:
	BOOL m_bText = true;
	BOOL m_bSpikes = true;
	BOOL m_bBars = true;
	int m_leftcolwidth = 20;
	int m_rowheight = 20;
	int m_widthSeparator = 5;
	int m_widthText = -1;
	int m_widthSpikes = -1;
	int m_widthBars = -1;
	int m_topIndex = -1;
	COLORREF m_clrText = RGB(0, 0, 0);
	COLORREF m_clrBkgnd;
	CBrush m_brBkgnd;
	int m_cursorIndexMax = 3;

	// logic to drag spikes
	BOOL m_bHitspk = false; // one spike is hit
	int m_selspikeLB = 0; // spike selected & hit
	int m_oldsel = 0; // selected row from which spike is hit

	long m_lFirst = 0;
	long m_lLast = 0;
	SpikeList* p_spikelist_ = nullptr;
	CSpikeDoc* p_spike_doc_ = nullptr;
	CdbWaveDoc* p_dbwave_doc_ = nullptr;
	HWND m_hwndBarsReflect = nullptr;

public:
	void SetSourceData(SpikeList* pSList, CdbWaveDoc* pdbDoc);
	void SetSpkList(SpikeList* p_spike_list);

	void SetTimeIntervals(long l_first, long l_last);
	int GetHowManyClassesInCurrentSpikeList();
	long GetTimeFirst() const { return m_lFirst; }
	long GetTimeLast() const { return m_lLast; }

	void SetRowHeight(int rowheight);
	void SetLeftColWidth(int rowwidth);
	void SetColsWidth(int coltext, int colspikes, int colseparator);

	int GetRowHeight() const { return m_rowheight; }
	int GetLeftColWidth() const { return m_leftcolwidth; }
	int GetColsTextWidth() const { return m_widthText; }
	int GetColsSpikesWidth() const { return m_widthSpikes; }
	int GetColsTimeWidth() const { return m_widthBars; }
	int GetColsSeparatorWidth() const { return m_widthSeparator; }
	float GetExtent_mV();

	void SetYzoom(int y_we, int y_wo);
	void SetXzoom(int x_we, int x_wo);

	int GetYWExtent(); // load display parameters
	int GetYWOrg(); // ordinates
	int GetXWExtent(); // and
	int GetXWOrg(); // abscissa

	int SelectSpike(int spikeno);
	void ChangeSpikeClass(int spikeno, int newclass);
	int SetMouseCursorType(int cursorm);
	void XorTempVTtag(int xpoint);
	void ResetBarsXortag();
	void ReflectBarsMouseMoveMessg(HWND hwnd);
	void SetCursorMaxOnDblClick(int imax) { m_cursorIndexMax = imax; }

	void PrintItem(CDC* p_dc, CRect* prect1, CRect* prect2, CRect* prect3, int i);

protected:
	void UpdateString(void* pptr, int iclass, int nbspikes);

	// Implementation
public:
	void MeasureItem(LPMEASUREITEMSTRUCT lpMIS) override;
	void DrawItem(LPDRAWITEMSTRUCT lpDIS) override;
	int CompareItem(LPCOMPAREITEMSTRUCT lpCIS) override;
	void DeleteItem(LPDELETEITEMSTRUCT lpDI) override;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnMyMessage(WPARAM wParam, LPARAM lParam);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg HBRUSH CtlColor(CDC* p_dc, UINT nCtlColor);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP()
};
