#pragma once
#include "afxwin.h"

// CDAChannelsDlg dialog

#define DA_SEQUENCEWAVE	0
#define DA_SINEWAVE		1
#define DA_SQUAREWAVE	2
#define DA_TRIANGLEWAVE	3
#define DA_MSEQWAVE		4
#define DA_NOISEWAVE	5
#define	DA_FILEWAVE		6
#define DA_CONSTANT		7
#define DA_LINEWAVE		8
#define DA_ZERO			9
#define DA_ONE			10


class CDAChannelsDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CDAChannelsDlg)

public:
	CDAChannelsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDAChannelsDlg();

// Dialog Data
	enum { IDD = IDD_DA_CHANNELS };
	OPTIONS_OUTPUTDATA	m_outD;
	BOOL				m_bChannel2;
	CArray <CComboBox*, CComboBox*> m_pcombos;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void EditSequence(int isel, int channel);
	void FillCombo(CComboBox* pCombo, int channel);
	void SelectComboItem(CComboBox * pCombo, DWORD val);
	void SetDigitalParmsToDlg(int channel);
	void GetDigitalParmsFromDlg(int channel);

	void OnBnClickedButtonsource(int ID, int channel);
	void OnCbnSelchangeCombosource(int IDCOMBO, int IDBUTTON, int IDFREQ);

	static CString	comboText[];
	static DWORD	comboVal[];

public:
	CIntervalsArray m_stimsaved;
	float	m_samplingRate;
	int		m_iseldigital;

	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedOk();
	
	afx_msg void OnBnClickedCheckchan0();
	afx_msg void OnBnClickedCheckchan1();
	afx_msg void OnBnClickedCheckchan2();

	afx_msg void OnCbnSelchangeCombosource0(); 
	afx_msg void OnCbnSelchangeCombosource1();
	afx_msg void OnCbnSelchangeCombosource2();

	afx_msg void OnBnClickedButtonsource0();
	afx_msg void OnBnClickedButtonsource1();
	afx_msg void OnBnClickedButtonsource2();
	// TODO
	//afx_msg void OnCbnSelchangeCombochandigital();
	afx_msg void OnBnClickedButtonsource3();
	afx_msg void OnBnClickedButtonsource4();
	afx_msg void OnBnClickedButtonsource5();
	afx_msg void OnBnClickedButtonsource6();
	afx_msg void OnBnClickedButtonsource7();
	afx_msg void OnBnClickedButtonsource8();
	afx_msg void OnBnClickedButtonsource9();
	afx_msg void OnCbnSelchangeCombosource3();
	afx_msg void OnCbnSelchangeCombosource4();
	afx_msg void OnCbnSelchangeCombosource5();
	afx_msg void OnCbnSelchangeCombosource6();
	afx_msg void OnCbnSelchangeCombosource7();
	afx_msg void OnCbnSelchangeCombosource8();
	afx_msg void OnCbnSelchangeCombosource9();
};
