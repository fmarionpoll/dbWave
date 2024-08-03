#include "StdAfx.h"
#include "dbWaveDoc.h"
#include "AcqDataDoc.h"
#include "Editctrl.h"
#include "ChartWnd.h"
#include "ChartData.h"
#include "ChartSpikeShape.h"
#include "DlgSpikeEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



DlgSpikeEdit::DlgSpikeEdit(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
}

void DlgSpikeEdit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_SPIKECLASS, spike_class);
	DDX_Text(pDX, IDC_SPIKENO, spike_index);
	DDX_Check(pDX, IDC_ARTEFACT, b_artefact);
	DDX_Text(pDX, IDC_DISPLAYRATIO, display_ratio);
	DDX_Text(pDX, IDC_YEXTENT, yv_extent);
}

BEGIN_MESSAGE_MAP(DlgSpikeEdit, CDialog)
	ON_EN_CHANGE(IDC_SPIKENO, OnEnChangespike_index)
	ON_EN_CHANGE(IDC_SPIKECLASS, OnEnChangeSpikeclass)
	ON_BN_CLICKED(IDC_ARTEFACT, OnArtefact)
	ON_EN_CHANGE(IDC_DISPLAYRATIO, OnEnChangeDisplayratio)
	ON_EN_CHANGE(IDC_YEXTENT, OnEnChangeYextent)
	ON_WM_DESTROY()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
END_MESSAGE_MAP()


void DlgSpikeEdit::load_spike_parameters()
{
	if (spike_index < 0)
		spike_index = 0;
	spike_sel_.spike_index = spike_index;
	chart_spike_shape_.set_selected_spike(spike_sel_);
	
	const Spike* spike = p_spk_list_->get_spike(spike_index); 
	spike_class = spike->get_class_id(); 
	b_artefact = (spike_class < 0);
	ii_time_ = spike->get_time();
	chart_spike_shape_.draw_spike(spike);

	if (p_acq_data_doc_ != nullptr)
		load_source_view_data();
	UpdateData(FALSE); 
}

BOOL DlgSpikeEdit::OnInitDialog()
{
	CDialog::OnInitDialog(); 
	p_acq_data_doc_ = db_wave_doc->m_p_dat;
	p_spk_list_ = db_wave_doc->m_p_spk->get_spike_list_current();
	if (p_spk_list_ == nullptr || p_spk_list_->get_spikes_count() == 0)
	{
		EndDialog(FALSE); 
		return TRUE;
	}
	spike_sel_.database_position = -1;
	spike_sel_.spike_list_index = -1;

	// subclass edit controls
	VERIFY(mm_spike_class.SubclassDlgItem(IDC_SPIKECLASS, this));
	VERIFY(mm_spike_index.SubclassDlgItem(IDC_SPIKENO, this));
	VERIFY(m_h_scroll_.SubclassDlgItem(IDC_SCROLLBAR1, this));
	VERIFY(m_v_scroll_.SubclassDlgItem(IDC_SCROLLBAR2, this));

	// add scrollbar to the left of edit controls
	mm_spike_class.ShowScrollBar(SB_VERT);
	mm_spike_index.ShowScrollBar(SB_VERT);

	// attach spike buffer
	VERIFY(chart_spike_shape_.SubclassDlgItem(IDC_CHART_SHAPE_buttn, this));
	chart_spike_shape_.set_source_data(p_spk_list_, db_wave_doc);
	chart_spike_shape_.set_range_mode(RANGE_ALL); // display mode (lines)
	chart_spike_shape_.set_plot_mode(PLOT_BLACK, 0); // display also artefacts

	if (p_acq_data_doc_ != nullptr)
	{
		VERIFY(chart_data_.SubclassDlgItem(IDC_DISPLAREA_buttn, this));
		chart_data_.set_b_use_dib(FALSE);
		chart_data_.attach_data_file(p_acq_data_doc_);
		const auto lv_size = chart_data_.get_rect_size();
		chart_data_.resize_channels(lv_size.cx, 0); 
		chart_data_.remove_all_channel_list_items();
		chart_data_.add_channel_list_item(p_spk_list_->get_detection_parameters()->extract_channel, p_spk_list_->get_detection_parameters()->extract_transform);

		if (p_spk_list_->get_detection_parameters()->compensate_Baseline)
		{
			chart_data_.add_channel_list_item(p_spk_list_->get_detection_parameters()->extract_channel, MOVAVG30);
			chart_data_.get_channel_list_item(1)->SetColor(6);
			chart_data_.get_channel_list_item(1)->SetPenWidth(1);
			static_cast<CButton*>(GetDlgItem(IDC_CHECK1))->SetCheck(1);
		}
		intervals_to_highlight_spikes_.SetSize(3 + 2); // total size
		intervals_to_highlight_spikes_.SetAt(0, 0); // source channel
		intervals_to_highlight_spikes_.SetAt(1, RGB(255, 0, 0)); // red color
		intervals_to_highlight_spikes_.SetAt(2, 1); // pen size
		chart_data_.set_highlight_data(&intervals_to_highlight_spikes_); // tell sourceview to highlight spk

		// validate associated controls
		VERIFY(mm_yv_extent.SubclassDlgItem(IDC_YEXTENT, this));
		VERIFY(mm_display_ratio.SubclassDlgItem(IDC_DISPLAYRATIO, this));
		mm_yv_extent.ShowScrollBar(SB_VERT);
		mm_display_ratio.ShowScrollBar(SB_VERT);
	}
	else
	{
		GetDlgItem(IDC_YEXTENT)->EnableWindow(FALSE);
		GetDlgItem(IDC_DISPLAYRATIO)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON1)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON2)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON3)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON4)->EnableWindow(FALSE);
	}

	display_ratio = 20; // how much spike versus source data
	spk_pre_trigger_ = p_spk_list_->get_detection_parameters()->detect_pre_threshold; 
	spk_length_ = p_spk_list_->get_spike_length();
	view_data_len_ = MulDiv(spk_length_, 100, display_ratio); 
	if (y_extent == 0)
	{
		short max, min;
		p_spk_list_->get_total_max_min(TRUE, &max, &min);
		y_extent = (max - min);
		y_zero = (max + min) / 2;
	}
	yv_extent = y_extent; 

	// display data and init parameters
	load_spike_parameters(); 
	ii_time_old_ = ii_time_;
	b_changed = FALSE; 

	// adjust scroll bar (size of button and left/right limits)
#define SCROLLMAX 200
#define SCROLLCENTER 100
	m_h_scroll_infos_.fMask = SIF_ALL;
	m_h_scroll_infos_.nMin = 0;
	m_h_scroll_infos_.nMax = SCROLLMAX;
	m_h_scroll_infos_.nPos = SCROLLCENTER;
	m_h_scroll_infos_.nPage = 10;
	m_h_scroll_.SetScrollInfo(&m_h_scroll_infos_);

	m_v_scroll_infos_ = m_h_scroll_infos_;
	m_v_scroll_.SetScrollInfo(&m_v_scroll_infos_);

	return TRUE; // return TRUE unless you set the focus to a control
}

void DlgSpikeEdit::OnEnChangespike_index()
{
	if (mm_spike_index.m_bEntryDone)
	{
		const auto i_spike_index = spike_index;
		mm_spike_index.OnEnChange(this, spike_index, 1, -1);

		// check boundaries
		if (spike_index < 0)
			spike_index = 0;
		if (spike_index >= p_spk_list_->get_spikes_count())
			spike_index = p_spk_list_->get_spikes_count() - 1;

		if (spike_index != i_spike_index) 
		{
			load_spike_parameters();
			ii_time_old_ = ii_time_;
			update_spike_scroll();
		}
		else
			UpdateData(FALSE);
	}
}

void DlgSpikeEdit::OnEnChangeSpikeclass()
{
	if (mm_spike_class.m_bEntryDone)
	{
		mm_spike_class.OnEnChange(this, spike_class, 1, -1);
		p_spk_list_->get_spike(spike_index)->set_class_id(spike_class);
		b_artefact = (spike_class < 0);
		UpdateData(FALSE);
		b_changed = TRUE;
	}
}

void DlgSpikeEdit::OnArtefact()
{
	UpdateData(TRUE); 
	spike_class = (b_artefact) ? -1 : 0;
	p_spk_list_->get_spike(spike_index)->set_class_id(spike_class);
	UpdateData(FALSE); 
	b_changed = TRUE;
}

void DlgSpikeEdit::OnEnChangeDisplayratio()
{
	if (mm_display_ratio.m_bEntryDone)
	{
		mm_display_ratio.OnEnChange(this, display_ratio, 1, -1);

		if (display_ratio < 1)
			display_ratio = 1;
		UpdateData(FALSE);
		view_data_len_ = MulDiv(spk_length_, 100, display_ratio);
		if (p_acq_data_doc_ != nullptr)
			load_source_view_data();
	}
}

void DlgSpikeEdit::OnEnChangeYextent()
{
	if (mm_yv_extent.m_bEntryDone)
	{
		mm_yv_extent.OnEnChange(this, yv_extent, 1, -1);
		UpdateData(FALSE);
		ASSERT(yv_extent != 0);
		if (yv_extent != y_extent)
		{
			y_extent = yv_extent;
			chart_data_.get_channel_list_item(0)->SetYextent(y_extent);
			if (p_spk_list_->get_detection_parameters()->compensate_Baseline)
				chart_data_.get_channel_list_item(1)->SetYextent(y_extent);
			chart_spike_shape_.set_yw_ext_org(y_extent, y_zero);
			chart_data_.Invalidate();
			chart_spike_shape_.Invalidate();
		}
	}
}

void DlgSpikeEdit::load_source_view_data()
{
	const auto spike = p_spk_list_->get_spike(spike_index); 
	const auto spike_first = spike->get_time() - spk_pre_trigger_;
	intervals_to_highlight_spikes_.SetAt(3, spike_first); 
	const auto spike_last = spike_first + spk_length_; 
	intervals_to_highlight_spikes_.SetAt(4, spike_last);

	// compute limits of m_sourceView
	auto source_view_first = spike_first + spk_length_ / 2 - view_data_len_ / 2;
	if (source_view_first < 0) 
		source_view_first = 0; 
	auto source_view_last = source_view_first + view_data_len_ - 1; 
	if (source_view_last > chart_data_.get_document_last()) 
	{
		source_view_last = chart_data_.get_document_last();
		source_view_first = source_view_last - view_data_len_ + 1;
	}
	// get data from doc
	spike_chan = spike->get_source_channel();

	chart_data_.set_channel_list_source_channel(0, spike_chan);
	chart_data_.get_data_from_doc(source_view_first, source_view_last);

	const auto method = p_spk_list_->get_detection_parameters()->extract_transform;
	p_acq_data_doc_->load_transformed_data(source_view_first, source_view_last, method, spike_chan);

	// adjust offset (center spike) : use initial offset from spike
	CChanlistItem* chan0 = chart_data_.get_channel_list_item(0);
	chan0->SetYzero(y_zero + spike->get_amplitude_offset());
	chan0->SetYextent(y_extent);

	if (p_spk_list_->get_detection_parameters()->compensate_Baseline)
	{
		CChanlistItem* chan1 = chart_data_.get_channel_list_item(1);
		chan1->SetYzero(y_zero + spike->get_amplitude_offset());
		chan1->SetYextent(y_extent);
	}
	chart_data_.Invalidate();
}

void DlgSpikeEdit::load_spike_from_data(const int shift)
{
	if (p_acq_data_doc_ != nullptr)
	{
		Spike* p_spike = p_spk_list_->get_spike(spike_index);
		//auto offset = pSpike->get_amplitude_offset();
		ii_time_ += shift;
		p_spike->set_time(ii_time_);
		update_spike_scroll();

		load_source_view_data();
		const auto spike_first = ii_time_ - spk_pre_trigger_;

		auto lp_source = p_acq_data_doc_->get_transformed_data_buffer();
		const auto delta = spike_first - p_acq_data_doc_->get_t_buffer_first();
		lp_source += delta;
		p_spike->transfer_data_to_spike_buffer(lp_source, 1, spk_length_);

		short max, min;
		int i_max, i_min;
		p_spike->measure_max_min_ex(&max, &i_max, &min, &i_min, 0, p_spk_list_->get_spike_length() - 1);
		p_spike->set_max_min_ex(max, min, i_min - i_max);

		// copy data to spike buffer
		//offset += pSpike->get_amplitude_offset();
		//pSpike->OffsetSpikeDataToAverageEx(offset, offset);

		chart_spike_shape_.Invalidate();
		b_changed = TRUE;
	}
}

void DlgSpikeEdit::OnDestroy()
{
	CDialog::OnDestroy();
}

void DlgSpikeEdit::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar == nullptr)
	{
		CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
		return;
	}

	// assume that code comes from SCROLLBAR1
	int shift;
	switch (nSBCode)
	{
	case SB_LEFT: shift = -100;
		break; 
	case SB_LINELEFT: shift = -1;
		break; 
	case SB_LINERIGHT: shift = +1;
		break;
	case SB_PAGELEFT: shift = -10;
		break; 
	case SB_PAGERIGHT: shift = +10;
		break; 
	case SB_RIGHT: shift = +100;
		break; 
	case SB_THUMBPOSITION: // scroll to pos = nPos
	case SB_THUMBTRACK: // drag scroll box -- pos = nPos
		shift = static_cast<int>(nPos) - SCROLLCENTER - (ii_time_ - ii_time_old_);
		break;
	default: 
		return;
	}
	load_spike_from_data(shift);
}

void DlgSpikeEdit::update_spike_scroll()
{
	m_h_scroll_infos_.fMask = SIF_PAGE | SIF_POS;
	m_h_scroll_infos_.nPos = ii_time_ - ii_time_old_ + SCROLLCENTER;
	m_h_scroll_infos_.nPage = 10;
	m_h_scroll_.SetScrollInfo(&m_h_scroll_infos_);
}

void DlgSpikeEdit::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar == nullptr)
	{
		CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
		return;
	}

	// assume that code comes from SCROLLBAR2
	int shift;
	switch (nSBCode)
	{
	case SB_TOP: shift = -100;
		break; 
	case SB_LINEUP: shift = -1;
		break; 
	case SB_LINEDOWN: shift = 1;
		break; 
	case SB_PAGEUP: shift = -10;
		break; 
	case SB_PAGEDOWN: shift = 10;
		break; 
	case SB_BOTTOM: shift = 100;
		break; 
	case SB_THUMBPOSITION: // scroll to pos = nPos
	case SB_THUMBTRACK: // drag scroll box -- pos = nPos
		shift = nPos - SCROLLCENTER - 4096;
		break;
	default: 
		return;
	}

	const auto spike = p_spk_list_->get_spike(spike_index);
	spike->set_spike_length(p_spk_list_->get_spike_length());
	spike->offset_spike_data(static_cast<short>(shift));

	load_spike_parameters();
	b_changed = TRUE;
}
