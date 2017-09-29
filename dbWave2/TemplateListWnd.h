#pragma once

// TemplateListWnd.h : header file

/////////////////////////////////////////////////////////////////////////////
// CTemplateListWnd window

class CTemplateListWnd : public CListCtrl
{
	DECLARE_SERIAL(CTemplateListWnd)

// Construction
public:
	CTemplateListWnd();
	void operator = (const CTemplateListWnd& arg);	// operator redefinition
	virtual void Serialize(CArchive& ar);			// overridden for document i/o

// Attributes
public:
	int		InsertTemplate(int i, int classID);
	int		InsertTemplateData(int i, int classID);
	void	TransferTemplateData();
	BOOL	DeleteAllTemplates();
	BOOL	DeleteItem(int i);
	void	SortTemplatesByClass(BOOL bUp);
	void	SortTemplatesByNumberofSpikes(BOOL bUp, BOOL bUpdateClasses, int minclassnb);
	inline	CTemplateWnd* GetTemplateWnd(int i) const {return (CTemplateWnd*) m_ptpl.GetAt(i);}
	int		GetTemplateDataSize() const {return m_ptpl.GetSize();}

	double	m_globalstd;
	double	m_globaldist;

	BOOL	tInit(int i);
	BOOL	tAdd(short* pSource);
	BOOL	tAdd(int i, short* pSource);
	BOOL	tPower(int i, double* power);
	BOOL	tWithin(int i, short* pSource);
	BOOL	tMinDist(int i, short* pSource, int* offsetmin, double* distmin);
	void	tGlobalstats();

	void	SetTemplateLength(int spklen, int tpleft, int tpright);
	void	SetHitRate_Tolerance(int* phitrate, float* ptolerance);
	inline int GetTemplateLength() const {return m_tpllen;}
	inline int GetNtemplates() const {return GetItemCount();}
	void	SetYWExtOrg(int extent, int zero);
	void	UpdateTemplateLegends(LPCSTR pszType);
	void	UpdateTemplateBaseClassID(int inewlowestclass);
	void	SetTemplateclassID(int item, LPCTSTR pszType, int classID);
	inline	int GetTemplateclassID(int item) const {return ((CTemplateWnd*) m_ptpl.GetAt(item))->m_classID;}

protected:
	CPtrArray	m_ptpl;
	CTemplateWnd m_tpl0;
	int			m_tpllen;
	int			m_tpleft;
	int			m_tpright;
	float		m_ktolerance;
	int			m_hitrate;	

	int			m_yextent;
	int			m_yzero;
	BOOL		m_bDragging;
	int			m_iItemDrag;
	int			m_iItemDrop;
	CPoint		m_ptHotSpot;
	CPoint		m_ptOrigin;
	CSize		m_sizeDelta;
	CImageList*	m_pimageListDrag;

	void OnButtonUp(CPoint point);

// Operations
public:
	CImageList	m_imageList;

// Implementation
public:
	virtual ~CTemplateListWnd();
	
protected:

	// Generated message map functions
protected:
	afx_msg void OnGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeleteitem(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
};

