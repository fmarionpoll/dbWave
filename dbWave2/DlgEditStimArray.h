#pragma once
#include "Spikedoc.h"
#include "StretchControls.h"

// CEditStimArrayDlg dialog

class CDlgEditStimArray : public CDialog
{
	DECLARE_DYNAMIC(CDlgEditStimArray)

public:
	CDlgEditStimArray(CWnd* pParent = nullptr);
	virtual ~CDlgEditStimArray();

	// data passed by caller
	CTagList*			m_pTagList = nullptr;
	CIntervalsAndLevels* m_pstimsaved = nullptr;
	CArray < CIntervalsAndLevels*, CIntervalsAndLevels*> intervalsandlevels_ptr_array{};
	float				m_rate = 0.f;

	// Dialog Data
	enum { IDD = IDD_EDITSTIMULUS };

protected:
	CIntervalsAndLevels* m_pstim = nullptr;

	virtual void	DoDataExchange(CDataExchange* pDX);
	CStretchControl m_stretch{};
	BOOL		m_binit = false;
	CEdit		m_csEdit{};
	float		m_value = 0;
	int			m_iItem = -1;
	CListCtrl	m_stimarrayCtrl{};
	CImageList* m_pimagelist = nullptr;

	void		selectItem(int i);
	void		resetListOrder();
	void		addNewItem(int i, long lInterval);
	void		transferIntervalsArrayToDialogList(CIntervalsAndLevels* pstim);
	void		setSubItem0(LVITEM& lvi, int i, CString& cs);
	void		setSubItem1(LVITEM& lvi, int iItem, long lInterval, CString& cs);

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedEdit();
	afx_msg void OnEnKillfocusReOrder();
	afx_msg void OnBnClickedDelete();
	afx_msg void OnBnClickedInsert();
	afx_msg void OnBnClickedDelete3();
	afx_msg void OnBnClickedReOrder();
	afx_msg void OnBnClickedCopy();
	afx_msg void OnBnClickedPaste();
	afx_msg void OnBnClickedExport();
	afx_msg void OnBnClickedImportfromdata();

	DECLARE_MESSAGE_MAP()
};
