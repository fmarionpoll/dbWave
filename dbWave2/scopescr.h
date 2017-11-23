#pragma once

// scopescr.h : header file
//

// display parameters: m_rangemode
#define RANGE_ALL			0
#define RANGE_TIMEINTERVALS 1
#define RANGE_INDEX			2
// display parameters: m_plotmode
#define PLOT_BLACK			0
#define PLOT_ONECLASSONLY	1
#define PLOT_ONECLASS 		2
#define PLOT_CLASSCOLORS	3
#define PLOT_SINGLESPIKE	4
#define PLOT_ALLGREY		5
#define PLOT_WITHINBOUNDS	6
#define PLOT_ONECOLOR		7

#define NB_COLORS			17
#define BLACK_COLOR			0
#define SILVER_COLOR		2
#define RED_COLOR			4

#include "RulerBar.h"

/////////////////////////////////////////////////////////////////////////////
// CScopeScreen window

class CScopeScreen : public CWnd
{

// Implementation
public:
	CScopeScreen();								// create assoc object
	virtual ~CScopeScreen();					// delete assoc object
	DECLARE_SERIAL( CScopeScreen )
	virtual void Serialize( CArchive& archive );

public:
	virtual BOOL Create(LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);  	
	void	EraseBkgnd(CDC *pDC);
	void	PlotToBitmap(CDC *pDC);
	inline	COLORREF GetColor(int i) const {return m_colorTable[i];}
	inline	void	SetColor(int i, COLORREF ccolor) { m_colorTable[i] = ccolor; }
	inline	void	SetString(CString cs) {m_csEmpty =cs;}
	int				FindColor(COLORREF ccolor);

	float	ChangeUnit(float xVal, CString* xUnit, float* xScalefactor);
	int		NiceUnit(float y);
	
	// Implementation
protected:
	// statics : created only once - associated with a counter
	static int		m_countcurs;					// objects counter
#define NB_CURSORS 4								// nb of cursors loaded
	static HCURSOR	m_cursor		[NB_CURSORS];	// array with cursor handles
	static int		m_cursordragmode[NB_CURSORS];	// cursor mode: 0=invert rect; 1=catch object	
	static COLORREF m_colorTable	[NB_COLORS];	// array with colorref
	CPen			m_penTable		[NB_COLORS];	// table with CPen objects (same colors as color table
	BOOL			m_bLmouseDown;
	BOOL			m_bUseDIB;	
	CDC				m_PlotDC;
	CWordArray		m_arrayMark;
	CPen			m_blackDottedPen;
	CString			m_csEmpty;
	
public:
	SCOPESTRUCT		m_parms;

// ---------------------------------------------------------------
// Attributes
public:
	void SetbUseDIB(BOOL bsetPlot);				// use DIB or not
	void SetDisplayAreaSize(int cx, int cy);	// set size of the display area

	inline CSize Size() const						{return CSize(m_displayRect.Width()+1, m_displayRect.Height()+1);}
	inline int	Height() const						{return m_displayRect.Height()+1;}
	inline int	Width() const						{return m_displayRect.Width()+1;}
	inline int	GetMouseCursorType() const			{return m_cursorType;}
	virtual int SetMouseCursorType(int cursormode);	// change mouse cursor on button

	// scale
	inline void SetYWExtOrg(int extent, int zero)	{m_yWE = extent;	m_yWO = zero;}
	inline void SetXWExtOrg(int extent, int zero)	{m_xWE = extent;	m_xWO = zero;}
	inline int  GetYWExtent() const					{return m_yWE;}
	inline int  GetYWOrg() const					{return m_yWO;}
	inline int  GetXWExtent() const					{return m_xWE;}
	inline int  GetXWOrg() const					{return m_xWO;}

	void		SetNxScaleCells(int iCells, int iTicks = 0, int iTickLine = 0);
	void		SetNyScaleCells(int iCells, int iTicks = 0, int iTickLine = 0);
	inline int  GetNxScaleCells()					{return m_parms.iXCells;}
	inline int  GetNyScaleCells()					{return m_parms.iYCells;}
	inline void SetxScaleUnitValue(float x)			{m_parms.xScaleUnitValue=x;}
	inline void SetyScaleUnitValue(float y)			{m_parms.yScaleUnitValue=y;}

	// new x scale -----------------------------------------------------------------
public:
	CRuler		m_xRuler;
	CRuler		m_yRuler;
	CFont		m_hFont;
	BOOL		m_bNiceGrid;
	int			m_abcissaheight;
	int			m_ordinateswidth;
public:
	void		DrawGrid(CDC *pDC);
	void		AdjustDisplayRect(CRect* rect);
protected:
	void		DrawGridEvenlySpaced(CDC *pDC);
	void		DrawGridNicelySpaced(CDC *pDC);
	void		DrawGridfromScale(CDC *pDC, CRuler* scale);

	// tags ------------------------------------------------------------------------
public:
	inline int  GetHZtagVal(int i) 					{return m_HZtags.GetTagVal(i);}
	inline int  GetHZtagChan(int i) 				{return m_HZtags.GetTagChan(i);}
	inline int  GetHZtagPix(int i) 					{return m_HZtags.GetTagPix(i);}	
		   int  GetNHZtags();
	inline int  AddHZtag(int val, int chan)			{return m_HZtags.AddTag(val, chan);}
	inline int  DelHZtag(int index) 				{return m_HZtags.RemoveTag(index);}
	inline void DelAllHZtags()						{m_HZtags.RemoveAllTags();}

	inline void SetHZtagVal(int i, int val)			{m_HZtags.SetTagVal(i, val);}
	inline void SetHZtagPix(int i, int val)			{m_HZtags.SetTagPix(i, val);}
	inline void SetHZtagChan(int i, int chan)		{m_HZtags.SetTagChan(i, chan);}

	inline int	AddVTtag(int val)					{return m_VTtags.AddTag(val, 0);}
	inline int	AddVTLtag(long lval)				{return m_VTtags.AddLTag(lval, 0);}
	inline int	DelVTtag(int index) 				{return m_VTtags.RemoveTag(index);}
	inline void DelAllVTtags()						{m_VTtags.RemoveAllTags();}
	inline int	GetVTtagVal(int itag) 				{return m_VTtags.GetTagVal(itag);}	
	inline int	GetVTtagPix(int itag) 				{return m_VTtags.GetTagPix(itag);}	
		   int	GetNVTtags();
	inline void SetVTtagVal(int itag, int val)		{m_VTtags.SetTagVal(itag, val);}
	inline void SetVTtagPix(int itag, int val)		{m_VTtags.SetTagPix(itag, val);}

	inline void SetVTtagLval(int itag, long lval)	{m_VTtags.SetTagLVal(itag, lval);}
	inline long GetVTtagLval(int itag) 				{return m_VTtags.GetTagLVal(itag);}

	inline BOOL GetbDrawframe() const				{return m_parms.bDrawframe;}
	inline void SetbDrawframe(BOOL flag)			{m_parms.bDrawframe=flag;}
	inline CRect GetDefinedRect()					{return CRect(m_ptFirst.x, m_ptFirst.y, m_ptLast.x, m_ptLast.y);}

	inline void SetVTtagList(CTagList* pList)		{m_VTtags.CopyTagList(pList);}
	inline void SetHZtagList(CTagList* pList)		{m_HZtags.CopyTagList(pList);}
	CTagList*	GetVTtagList();
	CTagList*	GetHZtagList();	

	CString		m_csBottomComment;
	BOOL		m_bBottomComment;
	inline void SetBottomComment(BOOL flag, CString cs) {m_csBottomComment=cs; m_bBottomComment=flag;}

protected:
	int		m_plotmode;
	int		m_colorbackgr;
	int		m_colorselected;
	BOOL	m_erasebkgnd;		// erase backgroung (flag)
	CTagList m_HZtags;			// List of horizontal tag lines
	CTagList m_VTtags;			// List of vertical tag lines
	BOOL	m_bVTtagsLONG;		// flag: TRUE if VTtags are defined as long
	long	m_liFirst;			// file position of first left pixel
	long	m_liLast;			// file position of last right pixel
	long	m_liJitter;			// file position range corresponding mouse jitter

	// plotting options - parameters for PLOT_WITHINBOUNDS
	int		m_plotwithin_mode;
	int		m_lowerlimit;
	int		m_upperlimit;
	int		m_color_withinlimits;
	int		m_color_outsidelimits;

	// mouse tracking modes
	int		m_HCtrapped;		// cursor index trapped by the mouse
	int		m_trackMode;

	int 	m_xWO;				// x origin, extent / window & view
	int		m_xWE;
	int		m_xVO;
	int		m_xVE;
	int 	m_yWO;				// y origin, extent / window & view
	int		m_yWE;
	int		m_yVO;
	int		m_yVE;

	int 	m_curTrack;			// threshold  tracked
	CPoint	m_ptFirst;
	CPoint	m_ptCurr;
	CPoint	m_ptLast;
	CRect	m_clientRect;
	CRect	m_displayRect;

	int		m_cursorType;		// current cursor
	int		m_oldcursorType;
	HCURSOR m_currCursor;		// handle to current cursor
	int		m_currCursorMode;   // current cursor drag mode

	int 	m_cxjitter;			// mouse horizontal hitter
	int		m_cyjitter;			// mouse vertical jitter
	CRect	m_ZoomFrom;			// temp rect
	CRect	m_ZoomTo;			// temp rect
	int		m_iUndoZoom;		// 1: rect+ stored; -1: rect- stored; 0: none stored

	BOOL	m_bAllowProps;
	HWND	m_hwndReflect;
	CTag*	m_tempVTtag;

// Operations
public:
	void DisplayVTtags(CDC* pDC);
	void DisplayHZtags(CDC* pDC);
	void XorVTtag(int xpoint);		// xor vertical line at point.x	
	void XorTempVTtag(int xpoint);
	void XorHZtag(int ypoint);	// xor horizontal line at point.y	
	void ResetXortag() {m_ptLast.x = -1; m_ptLast.y = -1;}
	void ReflectMouseMoveMessg(HWND hwnd) {m_hwndReflect = hwnd;}
	void PlotToBitmap(CBitmap* pBitmap);

protected:
	void		PrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);
	void		CaptureCursor();
	void		ReleaseCursor();
	void		SendMyMessage(int code, int codeparm);
	void		PostMyMessage(int code, int codeparm);
	void		InvertTracker(CPoint point);	// invert rectangle when mouse-left is down
	int			HitTestHZtag(int y);			// test if point is on an horizontal tag line
	int			HitTestVTtagPix(int x);		// test if point is on a vertical tag line
	int			HitTestVTtagLong(long lx); 	// test if point is on a vertical tag line
	virtual void ZoomData(CRect* prevRect, CRect* newRect);	// zoom display	
	void		ZoomIn();
	void		ZoomOut();
	void		ZoomPop();	
	
// overloaded functions	
protected:
	virtual void PreSubclassWindow( );

public:
	virtual void PlotDatatoDC(CDC* pDC);

	// Generated message map functions
protected:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);	
	DECLARE_MESSAGE_MAP()
};
