#pragma once
//#include <afxdao.h>
#pragma warning(disable : 4995)
#include "CSpkListTabCtrl.h"

class CViewDAO : public CDaoRecordView
{
	DECLARE_DYNAMIC(CViewDAO)

protected:
	explicit		CViewDAO(LPCTSTR lpszTemplateName);
	explicit		CViewDAO(UINT nIDTemplate);
	virtual			~CViewDAO();

public:
	CdbMainTable*	m_pSet = nullptr;
	boolean			m_autoDetect = false;
	boolean			m_autoIncrement = false;

	CdbWaveDoc*		GetDocument();
	virtual CDaoRecordset* OnGetRecordset();
	virtual BOOL	OnMove(UINT nIDMoveCommand) override;
	virtual void	OnDraw(CDC* pDC);
	virtual BOOL	PreCreateWindow(CREATESTRUCT& cs)  override;

	CSpikeDoc*		m_pSpkDoc = nullptr;
	CSpikeList*		m_pSpkList = nullptr;
	void			saveCurrentSpkFile();
	CSpkListTabCtrl	m_tabCtrl{};

#ifdef _DEBUG
	virtual void	AssertValid() const;
#ifndef _WIN32_WCE
	virtual void	Dump(CDumpContext& dc) const;
#endif
#endif
	// print view
protected:
	CRect			m_Margin;				// margins (pixels)
	int				m_file0 = 0;			// current file
	long			m_lFirst0 = 0;
	long			m_lLast0 = 0;
	int				m_npixels0 = 0;
	int				m_nfiles = 0;			// nb of files in doc
	int 			m_nbrowsperpage = 0;	// USER: nb files/page
	long			m_lprintFirst = 0;		// file index of first pt
	long 			m_lprintLen = 0;		// nb pts per line
	long			m_printFirst = 0;
	long 			m_printLast = 0;
	BOOL			m_bIsPrinting = false;
	CRect			m_rData;
	CRect			m_rSpike;

	// specific printer parameters
	TEXTMETRIC		m_tMetric{};			// onbegin/onendPrinting
	LOGFONT			m_logFont{};			// onbegin/onendPrinting
	CFont*			m_pOldFont = nullptr;	// onbegin/onendPrinting
	CFont			m_fontPrint;			// onbegin/onendPrinting
	CRect			m_printRect;

	virtual BOOL	OnPreparePrinting(CPrintInfo* pInfo) override;
	virtual void	OnBeginPrinting(CDC* p_dc, CPrintInfo* pInfo) override;
	virtual void	OnEndPrinting(CDC* p_dc, CPrintInfo* pInfo) override;
	virtual void	OnPrint(CDC* p_dc, CPrintInfo* pInfo) override;
	virtual void	OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) override;

	// class and parameter for OnSize
	CStretchControl m_stretch{};
	BOOL			m_binit = false;

protected:
	afx_msg void	OnSize(UINT nType, int cx, int cy);
	afx_msg void	OnDestroy();
	afx_msg void	OnNMClickTab1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void	OnTcnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in dataView.cpp
inline CdbWaveDoc* CViewDAO::GetDocument()
{
	return (CdbWaveDoc*)m_pDocument;
}
#endif

