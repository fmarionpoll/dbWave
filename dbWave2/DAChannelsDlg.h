#pragma once

// CDAChannelsDlg dialog

#define DA_SEQUENCEWAVE	0
#define DA_SINEWAVE		1
#define DA_SQUAREWAVE	2
#define DA_TRIANGLEWAVE	3
#define DA_MSEQWAVE		4
#define DA_NOISEWAVE	5
#define	DA_FILEWAVE		6
#define DA_LINEWAVE		7
#define DA_CONSTANT		8

class CDAChannelsDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CDAChannelsDlg)

public:
	CDAChannelsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDAChannelsDlg();

// Dialog Data
	enum { IDD = IDD_DA_CHANNELS };
	OPTIONS_OUTPUTDATA	m_outD;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	BOOL	m_bChannel0;
	BOOL	m_bChannel1;
	BOOL	m_bChannel2;
	
	double	m_famplitude0;
	double	m_famplitudelow0;
	double	m_ffrequence0;

	int		m_waveformChannel0;
	int		m_waveformChannel1;
	int		m_waveformChannel2;

	double	m_famplitude1;
	double	m_famplitudelow1;
	double	m_ffrequence1;

	CIntervalsArray m_stimsaved;
	float	m_samplingRate;

	virtual BOOL OnInitDialog();
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
};
