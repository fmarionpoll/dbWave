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

	DlgADInputs(CWnd* p_parent = nullptr);
	~DlgADInputs() override;

	// Dialog Data
	enum { IDD = IDD_AD_INPUTPARMS };

protected:
	void DoDataExchange(CDataExchange* pDX) override;

	// number of input channels
	int m_n_acq_channels_  {1};					// number of acquisition channels
	int m_max_channels_  {32};					// max nb for channel input number (0-7 or 0-15; exception for channel 16DIN)
	int m_input_list_max_  {32};				// dt9800  32 (set when creating the object)
	CSize m_old_size_  {CSize(-1, -1)}; // used to resize the form

public:
	GridCtrl m_grid;

	// parameters passed:
	CWaveFormat* m_pw_format  {nullptr};	// acquisition parameters
	CWaveChanArray* m_pch_array  {nullptr};	// acquisition channels
	BOOL b_chain_dialog  {false};			// chain dialog (no FALSE)
	WORD m_postmessage  {0};				// launch assoc dialog

	BOOL channel_type { OLx_CHNT_SINGLEENDED}; // flag TRUE single ended; false differential - default: false
	int n_channels_max_di  {8};				//  m_Analog.GetSSCaps(OLSSC_MAXDICHANS); default  8
	int n_channels_max_se  {16};				//  m_Analog.GetSSCaps(OLSSC_MAXSECHANS); default  16
	BOOL b_command_amplifier  {false};		// change ampli settings on the fly (if present); default  none

	CUSBPxxS1* m_alligator_amplifier  {nullptr};
	boolean is_ad_changed  {false};

	// Implementation
protected:
	static const TCHAR* psz_row_title_[];
	static const TCHAR* psz_high_pass_[];
	static const TCHAR* psz_ad_gains_[];
	static const TCHAR* psz_amplifier_[];
	static const TCHAR* psz_probe_type_[];
	static const TCHAR* psz_encoding_[];
	static int i_encoding_[];

	int m_row_ad_channel_  {0};
	int m_row_ad_gain_  {0};
	int m_row_head_stage_gain_  {1};
	int m_row_amp_gain_  {1};
	int m_row_readonly_  {1};
	UINT m_i_n_bins_  {4096};
	float m_x_volts_max_  {10.0f};

	BOOL init_grid_column_defaults(int col);
	void adjust_grid_size();
	void init_ad_channel_combo(int col, int i_select) const;
	void load_grid_with_wavechan_data(int col);
	void save_grid_to_wavechan_data(int col) const;
	void save_data();
	void set_amplifier_parameters(int col) const;
	void get_amplifier_parameters(CWaveChan* p_chan) const;

	void load_ad_parameters_from_pw_format();
	void init_row_headers();
	void init_columns();
	void display_channel_ad_card(GV_ITEM& item, const CWaveChan* p_chan);
	void display_channel_amplifier(GV_ITEM& item, const CWaveChan* p_chan);
	void display_channel_probe(GV_ITEM& item, const CWaveChan* p_chan);
	void display_channel_read_only_fields(int col);

public:
	BOOL OnInitDialog() override;

	CComboBox m_resolution_combo; // A/D resolution: 8, 12, 16 bits (?)
	CComboBox m_encoding_combo; // encoding mode (binary offset/straight/2's complement)

	afx_msg void on_en_change_n_acq_channels();
	afx_msg void OnSize(UINT n_type, int cx, int cy);
	afx_msg void on_bn_clicked_single_ended();
	afx_msg void on_bn_clicked_differential();
	afx_msg void on_grid_end_edit(NMHDR* p_notify_struct, LRESULT* p_result);
	afx_msg void on_cbn_sel_change_resolution();
	afx_msg void on_bn_clicked_ok();
	afx_msg void on_bn_clicked_ad_intervals();

	DECLARE_MESSAGE_MAP()
};
