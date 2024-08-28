// exportsp.cpp : implementation file
//

#include "StdAfx.h"
#include "DlgExportSpikeInfos.h"

#include "options_view_spikes.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


DlgExportSpikeInfos::DlgExportSpikeInfos(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
	m_bacqchsettings = FALSE;
	m_bacqcomments = FALSE;
	m_bacqdate = FALSE;
	m_timeend = 0.0f;
	m_timebin = 0.0f;
	m_timestart = 0.0f;
	m_classnb = 0;
	m_classnb2 = 0;
	m_btotalspikes = FALSE;
	m_bspkcomments = FALSE;
	m_ispikeclassoptions = -1;
	m_nbins = 1;
	m_histampl_vmax = 0.0f;
	m_histampl_vmin = 0.0f;
	m_histampl_nbins = 0;
	m_iexportoptions = -1;
	m_bexportzero = FALSE;
	m_bexportPivot = TRUE;
	m_bexporttoExcel = TRUE;
	m_istimulusindex = 0;
	m_bhist = SW_SHOW;
	m_bhistampl = SW_SHOW;
	m_brelation = TRUE;
	m_pvdS = nullptr;
}

void DlgExportSpikeInfos::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_ACQCHSETTING, m_bacqchsettings);
	DDX_Check(pDX, IDC_ACQCOMMENTS, m_bacqcomments);
	DDX_Check(pDX, IDC_ACQDATE, m_bacqdate);
	DDX_Text(pDX, IDC_TIMEEND, m_timeend);
	DDX_Text(pDX, IDC_BINSIZE, m_timebin);
	DDX_Text(pDX, IDC_TIMESTART, m_timestart);
	DDX_Check(pDX, IDC_CHECK4, m_brelation);
	DDX_Text(pDX, IDC_SPIKECLASS, m_classnb);
	DDX_Text(pDX, IDC_SPIKECLASS2, m_classnb2);
	DDX_Check(pDX, IDC_TOTALSPIKES, m_btotalspikes);
	DDX_Check(pDX, IDC_SPKFILECOMMENTS, m_bspkcomments);

	DDX_Radio(pDX, IDC_RADIO1, m_ispikeclassoptions);
	DDX_Text(pDX, IDC_NBINS, m_nbins);
	DDX_Text(pDX, IDC_HISTMAX, m_histampl_vmax);
	DDX_Text(pDX, IDC_HISTMIN, m_histampl_vmin);
	DDX_Text(pDX, IDC_HISTNBINS, m_histampl_nbins);
	DDX_Radio(pDX, IDC_BPSTH, m_iexportoptions);
	DDX_Check(pDX, IDC_CHECK1, m_bexportzero);
	DDX_Check(pDX, IDC_CHECK2, m_bexportPivot);
	DDX_Check(pDX, IDC_CHECK3, m_bexporttoExcel);
	DDX_Text(pDX, IDC_EDIT1, m_istimulusindex);
}

BEGIN_MESSAGE_MAP(DlgExportSpikeInfos, CDialog)
	ON_BN_CLICKED(IDC_RADIO1, OnClassFilter)
	ON_EN_CHANGE(IDC_NBINS, OnEnChangeNbins)
	ON_EN_CHANGE(IDC_BINSIZE, OnEnChangeBinsize)
	ON_BN_CLICKED(IDC_BPSTH, OnclickPSTH)
	ON_BN_CLICKED(IDC_BISI, OnclickISI)
	ON_BN_CLICKED(IDC_BAUTOCORRELATION, OnclickAUTOCORR)
	ON_BN_CLICKED(IDC_BINTERVALS, OnclickOthers)
	ON_BN_CLICKED(IDC_AMPLIHIST, OnclickAmplHistog)
	ON_BN_CLICKED(IDC_BEXTREMA, OnclickOthers)
	ON_BN_CLICKED(IDC_RADIO2, OnClassFilter)
	ON_BN_CLICKED(IDC_RADIO3, OnClassFilter)
	ON_BN_CLICKED(IDC_SPIKEMAXTOMIN, OnclickOthers)
	ON_BN_CLICKED(IDC_BSPIKELATENCY, OnclickOthers)
	ON_BN_CLICKED(IDC_CHECKAVERAGE, OnclickOthers)
	ON_BN_CLICKED(IDC_RADIO5, OnBnClickedRadio5)
	ON_BN_CLICKED(IDC_RADIO4, OnBnClickedRadio4)
	ON_BN_CLICKED(IDC_SPIKEPOINTS, OnBnClickedSpikepoints)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// DlgExportSpikeInfos message handlers

BOOL DlgExportSpikeInfos::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_bspkcomments = m_pvdS->b_spk_comments;
	m_bacqchsettings = m_pvdS->b_acq_ch_settings;
	m_bacqcomments = m_pvdS->b_acq_comments;
	m_bacqdate = m_pvdS->b_acq_date;
	m_classnb = m_pvdS->class_nb;
	m_classnb2 = m_pvdS->class_nb_2;
	m_btotalspikes = m_pvdS->b_total_spikes;
	m_bexportzero = m_pvdS->b_export_zero;
	m_bexportPivot = m_pvdS->b_export_pivot;
	m_bexporttoExcel = m_pvdS->b_export_to_excel;
	m_timeend = m_pvdS->time_end;
	m_timestart = m_pvdS->time_start;
	m_histampl_nbins = m_pvdS->hist_ampl_n_bins;
	m_histampl_vmax = m_pvdS->hist_ampl_v_max;
	m_histampl_vmin = m_pvdS->hist_ampl_v_min;
	m_istimulusindex = m_pvdS->i_stimulus_index;

	m_iexportoptions = m_pvdS->export_data_type;
	if (m_iexportoptions < 0)
		m_iexportoptions = EXPORT_PSTH;
	m_ispikeclassoptions = m_pvdS->spike_class_option + 1;
	auto disphist = SW_HIDE;
	auto disphistampl = SW_HIDE;
	switch (m_iexportoptions)
	{
	case EXPORT_PSTH:
		m_nbins = m_pvdS->n_bins;
		m_timebin = (m_timeend - m_timestart) / m_nbins;
		disphist = SW_SHOW;
		break;
	case EXPORT_ISI:
	case EXPORT_AUTOCORR:
		m_nbins = m_pvdS->n_bins_isi;
		m_timebin = m_pvdS->bin_isi;
		disphist = SW_SHOW;
		break;
	case EXPORT_HISTAMPL:
		disphistampl = SW_SHOW;
		break;
	default:
		break;
	}

	DisplayHistParms(disphist);
	DisplayHistAmplParms(disphistampl);

	static_cast<CButton*>(GetDlgItem(IDC_RADIO4))->SetCheck(m_pvdS->b_absolute_time);
	static_cast<CButton*>(GetDlgItem(IDC_RADIO5))->SetCheck(!m_pvdS->b_absolute_time);
	GetDlgItem(IDC_EDIT1)->ShowWindow(!m_pvdS->b_absolute_time);

	static_cast<CButton*>(GetDlgItem(IDC_RADIOALLCHANS))->SetCheck(m_pvdS->b_all_channels);
	static_cast<CButton*>(GetDlgItem(IDC_RADIOCURRCHAN))->SetCheck(!m_pvdS->b_all_channels);

	UpdateData(FALSE);
	OnClassFilter();
	return TRUE; // return TRUE  unless you set the focus to a control
}

void DlgExportSpikeInfos::OnOK()
{
	UpdateData(TRUE);

	if (m_timestart >= m_timeend)
	{
		MessageBox(_T("Current time intervals are incorrect!"), _T("Define exports"), MB_ICONSTOP);
		return;
	}

	// intervals OK - proceed
	m_pvdS->b_changed = TRUE;
	m_pvdS->b_acq_ch_settings = m_bacqchsettings;
	m_pvdS->b_acq_comments = m_bacqcomments;
	m_pvdS->b_acq_date = m_bacqdate;
	m_pvdS->export_data_type = m_iexportoptions;
	m_pvdS->b_export_zero = m_bexportzero;
	m_pvdS->b_export_pivot = m_bexportPivot;
	m_pvdS->b_export_to_excel = m_bexporttoExcel;
	if (m_iexportoptions == EXPORT_PSTH)
	{
		m_pvdS->n_bins = m_nbins;
		m_pvdS->time_bin = m_timebin;
	}
	else if (m_iexportoptions == EXPORT_ISI || m_iexportoptions == EXPORT_AUTOCORR)
	{
		m_pvdS->n_bins_isi = m_nbins;
		m_pvdS->bin_isi = m_timebin;
	}
	//m_pvdS->bartefacts = m_bartefacts;
	m_pvdS->class_nb = m_classnb;
	m_pvdS->class_nb_2 = m_classnb2;
	m_pvdS->b_column_header = TRUE;
	m_pvdS->b_total_spikes = m_btotalspikes;
	m_pvdS->b_spk_comments = m_bspkcomments;
	m_pvdS->spike_class_option = m_ispikeclassoptions - 1;

	m_pvdS->time_start = m_timestart;
	m_pvdS->time_end = m_timeend;

	m_pvdS->hist_ampl_n_bins = m_histampl_nbins;
	m_pvdS->i_stimulus_index = m_istimulusindex;
	m_pvdS->hist_ampl_v_max = m_histampl_vmax;
	m_pvdS->hist_ampl_v_min = m_histampl_vmin;

	m_pvdS->b_absolute_time = static_cast<CButton*>(GetDlgItem(IDC_RADIO4))->GetCheck();
	m_pvdS->b_all_channels = static_cast<CButton*>(GetDlgItem(IDC_RADIOALLCHANS))->GetCheck();

	CDialog::OnOK();
}

void DlgExportSpikeInfos::OnClassFilter()
{
	UpdateData(TRUE);
	GetDlgItem(IDC_SPIKECLASS)->EnableWindow(0 == m_ispikeclassoptions);
	GetDlgItem(IDC_SPIKECLASS2)->EnableWindow(0 == m_ispikeclassoptions);
}

void DlgExportSpikeInfos::OnEnChangeNbins()
{
	UpdateData(TRUE);
	if (m_nbins > 0 && m_iexportoptions == EXPORT_PSTH && m_brelation)
	{
		m_timebin = (m_timeend - m_timestart) / m_nbins;
		UpdateData(FALSE);
	}
}

void DlgExportSpikeInfos::OnEnChangeBinsize()
{
	UpdateData(TRUE);
	if (m_timebin > 0 && m_iexportoptions == EXPORT_PSTH && m_brelation)
	{
		m_nbins = static_cast<int>((m_timeend - m_timestart) / m_timebin);
		UpdateData(FALSE);
	}
}

void DlgExportSpikeInfos::OnclickPSTH()
{
	if (m_iexportoptions != EXPORT_PSTH)
	{
		m_iexportoptions = EXPORT_PSTH;
		m_nbins = m_pvdS->n_bins;
		m_timebin = m_pvdS->time_bin;
		UpdateData(FALSE);
	}
	DisplayHistAmplParms(SW_HIDE);
	DisplayHistParms(SW_SHOW);
}

void DlgExportSpikeInfos::OnclickISI()
{
	if (m_iexportoptions != EXPORT_ISI)
	{
		m_iexportoptions = EXPORT_ISI;
		m_nbins = m_pvdS->n_bins_isi;
		m_timebin = m_pvdS->bin_isi;
		UpdateData(FALSE);
	}
	DisplayHistAmplParms(SW_HIDE);
	DisplayHistParms(SW_SHOW);
}

void DlgExportSpikeInfos::OnclickAUTOCORR()
{
	if (m_iexportoptions != EXPORT_AUTOCORR)
	{
		m_iexportoptions = EXPORT_AUTOCORR;
		m_nbins = m_pvdS->n_bins_isi;
		m_timebin = m_pvdS->bin_isi;
		UpdateData(FALSE);
	}
	DisplayHistAmplParms(SW_HIDE);
	DisplayHistParms(SW_SHOW);
}

void DlgExportSpikeInfos::OnclickOthers()
{
	DisplayHistAmplParms(SW_HIDE);
	DisplayHistParms(SW_HIDE);
}

void DlgExportSpikeInfos::OnclickAmplHistog()
{
	DisplayHistAmplParms(SW_SHOW);
	DisplayHistParms(SW_HIDE);
}

void DlgExportSpikeInfos::DisplayHistParms(int bdisplay)
{
	if (m_bhist != bdisplay)
	{
		m_bhist = bdisplay;
		GetDlgItem(IDC_NBINS)->ShowWindow(bdisplay);
		GetDlgItem(IDC_BINSIZE)->ShowWindow(bdisplay);
		GetDlgItem(IDC_STATIC10)->ShowWindow(bdisplay);
		GetDlgItem(IDC_STATIC11)->ShowWindow(bdisplay);
		GetDlgItem(IDC_CHECK4)->ShowWindow(bdisplay);
	}
}

void DlgExportSpikeInfos::DisplayHistAmplParms(int bdisplay)
{
	if (m_bhistampl != bdisplay)
	{
		m_bhistampl = bdisplay;
		GetDlgItem(IDC_HISTNBINS_STATIC)->ShowWindow(bdisplay);
		GetDlgItem(IDC_HISTMIN_STATIC)->ShowWindow(bdisplay);
		GetDlgItem(IDC_HISTMAX_STATIC)->ShowWindow(bdisplay);
		GetDlgItem(IDC_HISTNBINS)->ShowWindow(bdisplay);
		GetDlgItem(IDC_HISTMIN)->ShowWindow(bdisplay);
		GetDlgItem(IDC_HISTMAX)->ShowWindow(bdisplay);
	}
}

void DlgExportSpikeInfos::OnBnClickedRadio5()
{
	m_pvdS->b_absolute_time = FALSE;
	GetDlgItem(IDC_EDIT1)->ShowWindow(TRUE);
}

void DlgExportSpikeInfos::OnBnClickedRadio4()
{
	m_pvdS->b_absolute_time = TRUE;
	GetDlgItem(IDC_EDIT1)->ShowWindow(FALSE);
}

void DlgExportSpikeInfos::OnBnClickedSpikepoints()
{
	m_iexportoptions = EXPORT_SPIKEPOINTS;
	DisplayHistAmplParms(SW_HIDE);
	DisplayHistParms(SW_HIDE);
}
