#pragma once

#include <Olxdadefs.h>

#include "AcqWaveChanArray.h"
#include "AcqWaveFormat.h"
#include "CyberAmp.h"
#include "./GridCtrl/GridCtrl.h"
#include "USBPxxS1.h"

class DlgADInputs : public CDialog
{
	DECLARE_DYNAMIC(DlgADInputs)

public:
	DlgADInputs(CWnd* pParent = nullptr);
	~DlgADInputs() override;

	// Dialog Data
	enum { IDD = IDD_AD_INPUTPARMS };

protected:
	void DoDataExchange(CDataExchange* pDX) override;

	// number of input channels
	int m_nacqchans = 1; // number of acquisition channels
	int m_maxchans = 32; // max nb for channel input number (0-7 or 0-15; exception for channel 16=DIN)
	int m_inputlistmax = 32; // dt9800 = 32 (set when creating the object)
	CSize m_OldSize = CSize(-1, -1); // used to resize the form

public:
	CGridCtrl m_Grid;

	// parameters passed:
	CWaveFormat* m_pwFormat = nullptr; // acquisition parameters
	CWaveChanArray* m_pchArray = nullptr; // acquisition channels
	BOOL m_bchainDialog = false; // chain dialog (no= FALSE)
	// parameter set on exit to chain dialog
	WORD m_postmessage = 0; // launch assoc dialog

	BOOL m_bchantype = OLx_CHNT_SINGLEENDED; // flag TRUE=single ended; false=differential - default: false
	int m_numchansMAXDI = 8; // = m_Analog.GetSSCaps(OLSSC_MAXDICHANS); default = 8
	int m_numchansMAXSE = 16; // = m_Analog.GetSSCaps(OLSSC_MAXSECHANS); default = 16
	BOOL m_bcommandAmplifier = false; // change ampli settings on the fly (if present); default = none

	CUSBPxxS1* m_palligator = nullptr;
	CCyberAmp* m_pcyber_amp = nullptr;


	// Implementation
protected:
	static TCHAR* pszRowTitle[];
	static TCHAR* pszHighPass[];
	static TCHAR* pszADGains[];
	static TCHAR* pszAmplifier[];
	static TCHAR* pszProbeType[];
	static TCHAR* pszEncoding[];
	static int iEncoding[];

	int m_rowADchannel = 0;
	int m_row_ADgain = 0;
	int m_row_headstagegain = 1;
	int m_row_ampgain = 1;
	int m_row_readonly = 1;
	UINT m_iNBins = 4096;
	float m_xVoltsMax = 10.0f;

	BOOL InitGridColumnDefaults(int col);
	void AdjustGridSize();
	void InitADchannelCombo(int col, int iselect);
	void LoadGridWithWavechanData(int i);
	void SaveGridToWavechanData(int i);
	void SaveData();
	void SetAmplifierParms(int col);
	void GetAmplifierParms(CWaveChan* p_chan);

	void LoadADparameters_From_pwFormat();
	void InitRowHeaders();
	void InitColumns();
	void DisplayChannelADCard(GV_ITEM& item, const CWaveChan* p_chan);
	void DisplayChannelAmplifier(GV_ITEM& item, const CWaveChan* p_chan);
	void DisplayChannelProbe(GV_ITEM& item, const CWaveChan* p_chan);
	void DisplayChannelReadOnlyFields(int col);


public:
	BOOL OnInitDialog() override;

	CComboBox m_resolutionCombo; // A/D resolution: 8, 12, 16 bits (?)
	CComboBox m_encodingCombo; // encoding mode (binary offset/straight/2's complement)

	afx_msg void OnEnChangeNacqchans();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedSingleended();
	afx_msg void OnBnClickedDifferential();
	afx_msg void OnGridEndEdit(NMHDR* pNotifyStruct, LRESULT* pResult);
	afx_msg void OnCbnSelchangeResolution();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedAdintervals();

	DECLARE_MESSAGE_MAP()
};
