#include "StdAfx.h"
#include "ChartSpike.h"


ChartSpike::ChartSpike()
{
	cs_empty_ = _T("no spikes");
	set_b_use_dib(FALSE);
}

ChartSpike::~ChartSpike()
= default;

int ChartSpike::get_color_according_to_plot_mode(const Spike* spike, int plot_mode) const
{
	const auto no_spike_class = spike->get_class_id();
	auto color = BLACK_COLOR;
	switch (plot_mode)
	{
	case PLOT_ONE_CLASS_ONLY:
	case PLOT_ONE_CLASS:
		if (no_spike_class != selected_class_)
			color = SILVER_COLOR;
		break;
	case PLOT_CLASS_COLORS:
		color = no_spike_class % nb_colors;
		break;
	case PLOT_BLACK:
	default:
		color = BLACK_COLOR;
		break;
	}
	return color;
}

boolean ChartSpike::get_spike_list_from_file(const int i_file)
{
	boolean success = true;
	if (b_display_all_files_)
	{
		success = static_cast<boolean>(dbwave_doc_->db_set_current_record_position(i_file));
		if (success) 
		{
			 dbwave_doc_->open_current_spike_file();
			 success = (dbwave_doc_->m_p_spk_doc != nullptr);
			 p_spike_doc_ = dbwave_doc_->m_p_spk_doc;
		}

		if (success)
			p_spike_list_ = dbwave_doc_->m_p_spk_doc->get_spike_list_current();
	}

	if (success)
	{
		if (p_spike_list_ == nullptr || p_spike_list_->get_spikes_count() <= 0)
			success = false;
	}
	return success;
}

void ChartSpike::set_plot_mode(const int mode, const int selected_class)
{
	plot_mode_ = mode;
	selected_class_ = selected_class;
}

void ChartSpike::set_source_data(SpikeList* p_spk_list, CdbWaveDoc* p_document)
{
	dbwave_doc_ = p_document;
	p_spike_doc_ = dbwave_doc_->m_p_spk_doc;
	p_spike_list_ = p_spk_list;
	spike_selected_.clear();
}

void ChartSpike::sub_item_draw(CDC& dc, CRect& rect)
{
	MoveWindow(rect, FALSE); 
	CDC dc_mem;
	CBitmap bmp_mem;
	dc_mem.CreateCompatibleDC(&dc);
	bmp_mem.CreateCompatibleBitmap(&dc, rect.Width(), rect.Height());
	CBitmap* p_old_bmp = dc_mem.SelectObject(&bmp_mem);

	// draw actual data
	set_display_area_size(rect.Width(), rect.Height());
	plot_data_to_dc(&dc_mem); 

	// transfer data to DC and clean the memory DC
	dc.BitBlt(rect.left, rect.top, rect.Width(), rect.Height(), &dc_mem, 0, 0, SRCCOPY);
	dc_mem.SelectObject(p_old_bmp);
	dc_mem.DeleteDC(); 
}

void ChartSpike::sub_item_create(CWnd* parent_wnd, const CRect& rect, int i_id, int i_class, CdbWaveDoc* pdb_doc, SpikeList* spike_list)
{
	Create(_T(""), WS_CHILD | WS_VISIBLE, rect, parent_wnd, i_id);
	const auto n_spikes = spike_list->get_spikes_count();

	set_source_data(spike_list, pdb_doc);
	set_plot_mode(PLOT_ONE_CLASS_ONLY, i_class);
	set_range_mode(RANGE_INDEX);
	set_spk_indexes(0, n_spikes - 1);
	set_b_draw_frame(TRUE);
	set_cursor_max_on_dbl_click(cursor_index_max_);
}

boolean ChartSpike::is_spike_within_range(db_spike& spike_selected) const
{
	if (range_mode_ == RANGE_INDEX
		&& (spike_selected.spike_index > index_last_spike_ || spike_selected.spike_index < index_first_spike_))
		return false;

	const auto spike = dbwave_doc_->get_spike(spike_selected);
	if (spike == nullptr)
		return false;

	const auto ii_time = spike->get_time();
	if (range_mode_ == RANGE_TIME_INTERVALS
		&& (ii_time < l_first_ || ii_time > l_last_))
		return false;

	if (plot_mode_ == PLOT_ONE_CLASS_ONLY
		&& spike->get_class_id() != selected_class_)
		return false;

	return true;
}

db_spike ChartSpike::hit_curve_in_doc(const CPoint point)
{
	const long n_files = b_display_all_files_ ? dbwave_doc_->db_get_records_count() : 1;
	db_spike result(1, -1, -1);

	for (long i_file = 0; i_file < n_files; i_file++)
	{
		if (b_display_all_files_)
		{
			if (dbwave_doc_->db_set_current_record_position(i_file))
				dbwave_doc_->open_current_spike_file();
			p_spike_list_ = dbwave_doc_->m_p_spk_doc->get_spike_list_current();
		}
		if (p_spike_list_ == nullptr || p_spike_list_->get_spikes_count() == 0)
			continue;

		result.record_id = dbwave_doc_->db_get_current_record_id();
		result.spike_list_index = dbwave_doc_->m_p_spk_doc->get_index_current_spike_list();

		result.spike_index = hit_curve(point);
		if (result.spike_index >= 0)
			break;
	}

	return result;
}
