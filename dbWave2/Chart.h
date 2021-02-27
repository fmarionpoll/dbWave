#pragma once

// chart.h : header file

// display parameters: m_rangemode
constexpr auto RANGE_ALL = 0;
constexpr auto RANGE_TIMEINTERVALS = 1;
constexpr auto RANGE_INDEX = 2;
// display parameters: m_plotmode
constexpr auto PLOT_BLACK = 0;
constexpr auto PLOT_ONECLASSONLY = 1;
constexpr auto PLOT_ONECLASS = 2;
constexpr auto PLOT_CLASSCOLORS = 3;
constexpr auto PLOT_SINGLESPIKE = 4;
constexpr auto PLOT_ALLGREY = 5;
constexpr auto PLOT_WITHINBOUNDS = 6;
constexpr auto PLOT_ONECOLOR = 7;
constexpr auto NB_COLORS = 17;
constexpr auto BLACK_COLOR = 0;
constexpr auto SILVER_COLOR = 2;
constexpr auto RED_COLOR = 4;

constexpr auto NB_CURSORS = 6;

#include "RulerBar.h"

// CChartWnd window

class CChartWnd : public CWnd
{
public:
	CChartWnd();
	virtual			 ~CChartWnd();
	DECLARE_SERIAL(CChartWnd)
	void			Serialize(CArchive& archive) override;

	virtual BOOL	Create(LPCTSTR lpszWindowName, DWORD dw_style, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = nullptr);
	virtual SCOPESTRUCT* GetScopeParameters();
	virtual void	SetScopeParameters(SCOPESTRUCT* pStruct);
	virtual int		SetMouseCursorType(int cursormode);	// change mouse cursor on button
	virtual	void	ZoomData(CRect* prevRect, CRect* newRect);
	virtual void	DisplayVTtags_Value(CDC* p_dc);
	virtual void	DisplayHZtags(CDC* p_dc);
	virtual void	PlotDatatoDC(CDC* p_dc);

	void			EraseBkgnd(CDC* p_dc);
	void			PlotToBitmap(CDC* p_dc);
	COLORREF		GetColor(int i) const { return m_colorTable[i]; }
	void			SetColor(int i, COLORREF ccolor) { m_colorTable[i] = ccolor; }
	void			SetString(CString cs) { m_csEmpty = cs; }
	int				FindColor(COLORREF ccolor);

	float			ChangeUnit(float xVal, CString* xUnit, float* xScalefactor);
	int				NiceUnit(float y);

	void			SetbUseDIB(BOOL bsetPlot);				// use DIB or not
	void			SetDisplayAreaSize(int cx, int cy);		// set size of the display area

	CSize			GetRectSize() const { return { m_displayRect.Width() + 1, m_displayRect.Height() + 1 }; }
	int				GetRectHeight() const { return m_displayRect.Height() + 1; }
	int				GetRectWidth() const { return m_displayRect.Width() + 1; }
	int				GetMouseCursorType() const { return m_cursorType; }
	void			SetMouseCursor(int cursorm);

	// scale
	void			SetYWExtOrg(int extent, int zero) { m_yWE = extent; m_yWO = zero; }
	void			SetXWExtOrg(int extent, int zero) { m_xWE = extent; m_xWO = zero; }
	int				GetYWExtent() const { return m_yWE; }
	int				GetYWOrg() const { return m_yWO; }
	int				GetXWExtent() const { return m_xWE; }
	int				GetXWOrg() const { return m_xWO; }

	void			SetNxScaleCells(int iCells, int iTicks = 0, int iTickLine = 0);
	void			SetNyScaleCells(int iCells, int iTicks = 0, int iTickLine = 0);
	int				GetNxScaleCells() { return m_scopestruct.iXCells; }
	int				GetNyScaleCells() { return m_scopestruct.iYCells; }
	void			SetxScaleUnitValue(float x) { m_scopestruct.xScaleUnitValue = x; }
	void			SetyScaleUnitValue(float y) { m_scopestruct.yScaleUnitValue = y; }
	void			AttachExternalXRuler(CRulerBar* pXRuler) { m_pXRulerBar = pXRuler; }
	void			AttachExternalYRuler(CRulerBar* pYRuler) { m_pYRulerBar = pYRuler; }

	// Operations
	void			XorVTtag(int xpoint);
	void			XorTempVTtag(int xpoint);
	void			XorHZtag(int ypoint);
	void			ResetXortag() { m_ptLast.x = -1; m_ptLast.y = -1; }
	void			ReflectMouseMoveMessg(HWND hwnd) { m_hwndReflect = hwnd; }
	void			PlotToBitmap(CBitmap* pBitmap);

	void			DrawGrid(CDC* p_dc);
	void			AdjustDisplayRect(CRect* rect);

	BOOL			GetbDrawframe() const { return m_scopestruct.bDrawframe; }
	void			SetbDrawframe(BOOL flag) { m_scopestruct.bDrawframe = flag; }
	CRect			GetDefinedRect() { return CRect(m_ptFirst.x, m_ptFirst.y, m_ptLast.x, m_ptLast.y); }
	void			SetBottomComment(BOOL flag, CString cs) { m_csBottomComment = cs; m_bBottomComment = flag; }

public:
	CRuler			m_xRuler;
	CRuler			m_yRuler;
	CFont			m_hFont;
	BOOL			m_bNiceGrid = false;
	int				m_abcissaheight = 10;
	int				m_ordinateswidth = 25;
	CRulerBar*		m_pXRulerBar = nullptr;
	CRulerBar*		m_pYRulerBar = nullptr;

	CTagList		m_HZtags{};				// List of horizontal tag lines
	CTagList		m_VTtags{};				// List of vertical tag lines
	CTagList*		GetVTtagList();
	CTagList*		GetHZtagList();
	
	CString			m_csBottomComment{};
	BOOL			m_bBottomComment = false;
	
	// Implementation
protected:
	static int		m_countcurs;					// objects counter
	static HCURSOR	m_cursor[NB_CURSORS];			// array with cursor handles
	static int		m_cursordragmode[NB_CURSORS];	// cursor mode: 0=invert rect; 1=catch object
	int				m_cursorType = 0;				// current cursor
	int				m_oldcursorType = 0;
	HCURSOR			m_currCursor{};					// handle to current cursor
	int				m_currCursorMode = 0;			// current cursor drag mode

	static COLORREF m_colorTable[NB_COLORS];		// array with colorref
	static TCHAR	csUnit[];
	static int		dUnitsPower[];
	static int		dmaxIndex;
	static int		dniceIntervals[];

	CPen			m_penTable[NB_COLORS];	// table with CPen objects (same colors as color table
	BOOL			m_bLmouseDown = false;
	BOOL			m_bUseDIB = false;
	CDC				m_PlotDC{};
	CWordArray		m_arrayMark{};
	CPen			m_blackDottedPen{};
	CString			m_csEmpty;
	SCOPESTRUCT		m_scopestruct{};

	int				m_plotmode = 0;
	int				m_colorbackgr = SILVER_COLOR;
	int				m_colorselected = BLACK_COLOR;
	BOOL			m_erasebkgnd = true;	// erase backgroung (flag)
	BOOL			m_bVTtagsLONG = false;	// flag: TRUE if VTtags are defined as long
	long			m_liFirst = 0;			// file position of first left pixel
	long			m_liLast = 0;			// file position of last right pixel
	long			m_liJitter{};			// file position range corresponding mouse jitter

	// plotting options - parameters for PLOT_WITHINBOUNDS
	int				m_plotwithin_mode{};
	int				m_lowerlimit{};
	int				m_upperlimit{};
	int				m_color_withinlimits{};
	int				m_color_outsidelimits{};

	// mouse tracking modes
	int				m_HCtrapped{};			// cursor index trapped by the mouse
	int				m_trackMode = TRACK_OFF;

	int 			m_xWO = 0;				// x origin, extent / window & view
	int				m_xWE = 1;
	int				m_xVO = 0;
	int				m_xVE = 1;
	int 			m_yWO = 0;				// y origin, extent / window & view
	int				m_yWE = 1;
	int				m_yVO = 0;
	int				m_yVE = 1;

	int 			m_curTrack{};			// threshold  tracked
	CPoint			m_ptFirst{};
	CPoint			m_ptCurr{};
	CPoint			m_ptLast{};
	CRect			m_clientRect{};
	CRect			m_displayRect{};

	int 			m_cxjitter;				// mouse horizontal hitter
	int				m_cyjitter;				// mouse vertical jitter
	CRect			m_ZoomFrom;				// temp rect
	CRect			m_ZoomTo;				// temp rect
	int				m_iUndoZoom = 0;		// 1: rect+ stored; -1: rect- stored; 0: none stored (not implemented)

	BOOL			m_bAllowProps = true;
	HWND			m_hwndReflect = nullptr;
	CTag*			m_tempVTtag = nullptr;

protected:
	virtual void	PreSubclassWindow();
	virtual int		hitCurve(CPoint point);

	void			prepareDC(CDC* p_dc, CPrintInfo* pInfo = nullptr);
	void			captureCursor();
	void			releaseCursor();
	void			lbuttonUp_HzTag(UINT nFlags, CPoint point);
	void			sendMyMessage(int code, int codeparm);
	void			postMyMessage(int code, int codeparm);
	void			invertTracker(CPoint point);	// invert rectangle when mouse-left is down
	int				hitHZtag(int y);				// test if point is on an horizontal tag line
	int				hitVTtagPix(int x);				// test if point is on a vertical tag line
	int				hitVTtagLong(long lx); 			// test if point is on a vertical tag line
	void			zoomIn();
	void			zoomOut();
	void			zoomPop();
	void			drawGridEvenlySpaced(CDC* p_dc);
	void			drawGridFromRuler(CDC* p_dc, CRuler* pRuler);
	void			drawGridNicelySpaced(CDC* p_dc);
	void			drawScalefromRuler(CDC* p_dc, CRuler* scale);

	// Generated message map functions
protected:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* p_dc);
	afx_msg void OnPaint();
	afx_msg BOOL OnSetCursor(CWnd* p_wnd, UINT nHitTest, UINT message);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()
};
