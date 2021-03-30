#pragma once

#include "TemplateWnd.h"


class CTemplateListWnd : public CListCtrl
{
	DECLARE_SERIAL(CTemplateListWnd)

	// Construction
public:
	CTemplateListWnd();
	virtual ~CTemplateListWnd() override;
	CTemplateListWnd& operator = (const CTemplateListWnd& arg);	// operator redefinition
	void		Serialize(CArchive& ar) override;
	
	// Attributes
public:
	CImageList	m_imageList;
	double		m_globalstd = 0.;
	double		m_globaldist = 0.;
protected:
	CImageList* m_pimageListDrag = nullptr;
	CArray < CTemplateWnd*, CTemplateWnd*> templatewnd_ptr_array;
	CTemplateWnd m_tpl0{};
	int			m_tpllen		= 60;
	int			m_tpleft		= 0;
	int			m_tpright		= 1;
	float		m_ktolerance	= 1.96f;
	int			m_hitrate		= 75;
	int			m_yextent		= 0;
	int			m_yzero			= 0;
	BOOL		m_bDragging		= false;
	int			m_iItemDrag		= 0;
	int			m_iItemDrop		= 0;
	CPoint		m_ptHotSpot{};
	CPoint		m_ptOrigin{};
	CSize		m_sizeDelta{};
	
public:
	int			InsertTemplate(int i, int classID);
	int			InsertTemplateData(int i, int classID);
	void		TransferTemplateData();
	void		DeleteAllTemplates();
	BOOL		DeleteItem(int i);
	void		SortTemplatesByClass(BOOL bUp);
	void		SortTemplatesByNumberofSpikes(BOOL bUp, BOOL bUpdateClasses, int minclassnb);
	int			GetTemplateDataSize() const { return templatewnd_ptr_array.GetSize(); }

	BOOL		tInit(int i);
	BOOL		tAdd(short* p_source);
	BOOL		tAdd(int i, short* p_source);
	BOOL		tPower(int i, double* power);
	BOOL		tWithin(int i, short* p_source);
	BOOL		tMinDist(int i, short* p_source, int* offsetmin, double* distmin);
	void		tGlobalstats();

	void		SetTemplateLength(int spklen, int tpleft, int tpright);
	void		SetHitRate_Tolerance(int* phitrate, float* ptolerance);
	void		SetYWExtOrg(int extent, int zero);
	void		UpdateTemplateLegends(LPCSTR pszType);
	void		UpdateTemplateBaseClassID(int inewlowestclass);
	void		SetTemplateclassID(int item, LPCTSTR pszType, int classID);

	inline CTemplateWnd* GetTemplateWnd(int i) const { return templatewnd_ptr_array.GetAt(i); }
	inline int	GetTemplateLength() const { return m_tpllen; }
	inline int	GetNtemplates() const { return GetItemCount(); }
	inline int	GetTemplateclassID(int item) const { return templatewnd_ptr_array.GetAt(item)->m_classID; }

	void		OnButtonUp(CPoint point);

	// Generated message map functions
protected:
	afx_msg void OnGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeleteitem(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
public:
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP()
};
