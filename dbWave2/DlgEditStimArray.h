#pragma once
#include "Spikedoc.h"

// CEditStimArrayDlg dialog

class CDlgEditStimArray : public CDialog
{
	DECLARE_DYNAMIC(CDlgEditStimArray)

public:
	CDlgEditStimArray(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CDlgEditStimArray();

	// data passed by caller
	CTagList*			m_pTagList = nullptr;
	float				m_rate;
	CIntervalsAndLevels* m_pstimsaved;
	CArray < CIntervalsAndLevels*, CIntervalsAndLevels*> intervalsandlevels_ptr_array{};

	// Dialog Data
	enum { IDD = IDD_EDITSTIMULUS };

protected:
	CIntervalsAndLevels* m_pstim;

	virtual void	DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	CImageList* m_pimagelist;
	CStretchControl m_stretch;
	BOOL			m_binit;
	CEdit			m_csEdit;		// IDC_EDIT1
	float			m_value;		// IDC_EDIT1
	int				m_iItem;
	CListCtrl		m_stimarrayCtrl;// IDC_LISTSTIM

	void			selectItem(int i);
	void			resetListOrder();
	void			addNewItem(long lInterval);
	LVITEM			getItemDescriptor(int i);
	void			transferIntervalsArrayToDialogList(CIntervalsAndLevels* pstim);

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedEdit();
	afx_msg void OnEnKillfocusEdit1();
	afx_msg void OnBnClickedDelete();
	afx_msg void OnBnClickedInsert();
	afx_msg void OnBnClickedDelete3();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedCopy();
	afx_msg void OnBnClickedPaste();
	afx_msg void OnBnClickedExport();
	afx_msg void OnBnClickedImportfromdata();
};
