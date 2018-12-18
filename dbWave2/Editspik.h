#pragma once

// editspik.h : header file

/////////////////////////////////////////////////////////////////////////////
// CSpikeEditDlg dialog
#include "Lineview.h"
#include "spikeshape.h"

class CSpikeEditDlg : public CDialog
{
// Construction
public:
	CSpikeEditDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
	enum { IDD = IDD_EDITSPIKE };
	int		m_spikeclass;
	int		m_spikeno;
	BOOL	m_bartefact;
	int		m_displayratio;
	int		m_yvextent;

	CEditCtrl mm_spikeno;
	CEditCtrl mm_spikeclass;
	CEditCtrl mm_displayratio;
	CEditCtrl mm_yvextent;
		
	CSpikeList*		m_pSpkList;		// spike list
	CAcqDataDoc*	m_dbDoc;			// source data doc cDocument
	int				m_spikeChan;	// source channel
	CWnd* 			m_parent;		// post messages to parent
	int				m_xextent;
	int 			m_yextent;
	int 			m_xzero;
	int 			m_yzero;
	BOOL 			m_bchanged;

protected:
	int				m_spkpretrig;
	int				m_spklen;
	int				m_viewdatalen;
	CDWordArray		m_DWintervals;	// intervals to highlight spikes / CLineviewWnd
	CSpikeShapeWnd	m_spkForm;		// all spikes in displayspikes
	CLineViewWnd	m_sourceView;	// source data	
	long			m_iitimeold;
	long			m_iitime;
	CScrollBar		m_HScroll;
	CScrollBar		m_VScroll;
	SCROLLINFO		m_HScroll_infos;	// infos for scrollbar
	SCROLLINFO		m_VScroll_infos;

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	void LoadSpikeParms();
	void LoadSourceData();
	void LoadSpikeFromData(int shift);
	void UpdateSpikeScroll();

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnEnChangeSpikeno();
	afx_msg void OnEnChangeSpikeclass();
	afx_msg void OnArtefact();
	afx_msg void OnEnChangeDisplayratio();
	afx_msg void OnEnChangeYextent();
	afx_msg void OnDestroy();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	DECLARE_MESSAGE_MAP()
};

