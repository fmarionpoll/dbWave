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

class CDlgDAChannels : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgDAChannels)

public:
	CDlgDAChannels(CWnd* pParent = nullptr); // standard constructor
	~CDlgDAChannels() override;

	// Dialog Data
	enum { IDD = IDD_DA_CHANNELS };

	CArray<OUTPUTPARMS, OUTPUTPARMS> outputparms_array;
	CArray<CComboBox*, CComboBox*> combobox_ptr_array;
	BOOL m_bChannel2;

protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support
	void EditSequence(int isel, int channel);
	void FillCombo(int channel);
	void SelectComboItemFromOutputParm(int ival);
	void OnBnClickedButtonsource(int channel);
	void OnCbnSelchangeCombosource(int ival);

	static CString comboText[];
	static DWORD comboVal[];

public:
	CIntervalsAndLevels m_stimsaved;
	float m_samplingRate;
	int m_iseldigital;

	BOOL OnInitDialog() override;

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

	DECLARE_MESSAGE_MAP()
};
