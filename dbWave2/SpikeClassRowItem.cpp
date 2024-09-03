#include "StdAfx.h"
#include "SpikeClassRowItem.h"

SpikeClassRowItem::SpikeClassRowItem()
= default;

SpikeClassRowItem::~SpikeClassRowItem()
{
	delete chart_spike_shape_;
	delete chart_spike_bar_;
}

void SpikeClassRowItem::create_item(CWnd* parent_wnd, CdbWaveDoc* pdb_doc, SpikeList* p_spike_list, const int i_class, int i_id, SpikeClassListBoxContext* context)
{
	const auto rect_spikes = CRect(0, 0, 0, 0); 
	const auto rect_bars = CRect(0, 0, 0, 0);
	parent_context_ = context;
	class_id_ = i_class;
	row_id_ = i_id;
	spike_list_ = p_spike_list;

	// 1) create chart_spike_shape
	if (spike_list_->get_spike_length() > 0)
	{
		chart_spike_shape_ = new (ChartSpikeShape);
		ASSERT(chart_spike_shape_ != NULL);
		chart_spike_shape_->sub_item_create(parent_wnd, rect_spikes, i_id, i_class, pdb_doc, spike_list_);
	}

	// 2) create chart_spike_bar with spike height
	i_id++;
	chart_spike_bar_ = new (ChartSpikeBar);
	ASSERT(chart_spike_bar_ != NULL);
	chart_spike_bar_->sub_item_create(parent_wnd, rect_bars, i_id, i_class, pdb_doc, spike_list_);

	// 3) create text
	row_comment_ = get_class_comment(i_class);
}

CString SpikeClassRowItem::get_class_comment(const int class_id) const
{
	SpikeClassProperties* p_desc = spike_list_->get_class_descriptor_from_id(class_id_);
	CString cs; 
	cs.Format(_T("class %i\n%s\nn=%i"),
		p_desc->get_class_id(),
		(LPCTSTR)p_desc->get_class_text(),
		p_desc->get_class_n_items());

	return cs;
}

void SpikeClassRowItem::set_class_id(const int new_class_id)
{
	class_id_ = new_class_id;
	chart_spike_bar_->set_plot_mode(PLOT_ONE_CLASS_ONLY, class_id_);
	chart_spike_shape_->set_plot_mode(PLOT_ONE_CLASS_ONLY, class_id_);
	row_comment_ = get_class_comment(new_class_id);
}

void SpikeClassRowItem::draw_item(const LPDRAWITEMSTRUCT lp_dis) const
{
	CDC dc;
	dc.Attach(lp_dis->hDC);

	if (lp_dis->itemAction & ODA_DRAWENTIRE)
	{
		// get data
		CRect rc_text = lp_dis->rcItem;
		rc_text.right = rc_text.left + parent_context_->m_width_text;
	

		// display text
		const auto text_length = row_comment_.GetLength();
		dc.DrawText(row_comment_, text_length, rc_text, DT_LEFT | DT_WORDBREAK);

		// display spikes
		const auto col1 = parent_context_->m_width_text + parent_context_->m_width_separator;
		const auto col2 = col1 + parent_context_->m_width_spikes + parent_context_->m_width_separator;
		auto rect_spikes = CRect(col1 + 1, lp_dis->rcItem.top + 1, col1 + parent_context_->m_width_spikes, lp_dis->rcItem.bottom - 1);
		chart_spike_shape_->sub_item_draw(dc, rect_spikes);

		// display bars
		auto rect_bars = CRect(col2 + 1, lp_dis->rcItem.top + 1, col2 + parent_context_->m_width_bars, lp_dis->rcItem.bottom - 1);
		chart_spike_bar_->sub_item_draw(dc, rect_bars);
	}

	// item is selected -- add frame
	if ((lp_dis->itemState & ODS_SELECTED) &&
		(lp_dis->itemAction & (ODA_SELECT | ODA_DRAWENTIRE)))
	{
		CBrush br(col_blue);
		dc.FrameRect(&(lp_dis->rcItem), &br);
	}

	// Item has been de-selected -- remove frame
	if (!(lp_dis->itemState & ODS_SELECTED) && // item not selected
		(lp_dis->itemAction & ODA_SELECT))
	{
		dc.FrameRect(&(lp_dis->rcItem), &parent_context_->m_brush_background);
	}
	dc.Detach();
}

void SpikeClassRowItem::set_time_intervals(const long l_first, const long l_last) const
{
	if (chart_spike_shape_ != nullptr)
	{
		chart_spike_shape_->set_range_mode(RANGE_TIME_INTERVALS);
		chart_spike_shape_->set_time_intervals(l_first, l_last);
	}
	chart_spike_bar_->set_range_mode(RANGE_TIME_INTERVALS);
	chart_spike_bar_->set_time_intervals(l_first, l_last);
}

void SpikeClassRowItem::set_spk_list(SpikeList* p_spike_list) const
{
	chart_spike_shape_->set_spike_list(p_spike_list);
	chart_spike_bar_->set_spike_list(p_spike_list);
}

int SpikeClassRowItem::set_mouse_cursor_type(const int cursor_m) const
{
	if (chart_spike_shape_ != nullptr)
		chart_spike_shape_->set_mouse_cursor_type(cursor_m);
	return chart_spike_bar_->set_mouse_cursor_type(cursor_m);
}

void SpikeClassRowItem::move_row_out_of_the_way() const
{
	CRect rect(0, 0, 0, 0);
	if (chart_spike_shape_ != nullptr)
		chart_spike_shape_->MoveWindow(rect, FALSE);
	chart_spike_bar_->MoveWindow(rect, FALSE);
}

void SpikeClassRowItem::set_y_zoom(const int y_we, const int y_wo) const
{
	if (chart_spike_shape_ != nullptr)
		chart_spike_shape_->set_yw_ext_org(y_we, y_wo);
	chart_spike_bar_->set_yw_ext_org(y_we, y_wo);
}

void SpikeClassRowItem::set_x_zoom(const int x_we, const int x_wo) const
{
	if (chart_spike_shape_ != nullptr)
		chart_spike_shape_->set_xw_ext_org(x_we, x_wo);
}

void SpikeClassRowItem::get_time_intervals(long& first, long& last) const
{
	first = chart_spike_bar_->get_time_first();
	last = chart_spike_bar_->get_time_last();
}

void SpikeClassRowItem::get_zoom_y(int& we, int& wo) const
{
	we = chart_spike_bar_->get_yw_extent();
	wo = chart_spike_bar_->get_yw_org();
}

void SpikeClassRowItem::get_zoom_x_shapes(int& we, int& wo) const
{
	if (chart_spike_shape_ == nullptr) return;

	we = chart_spike_shape_->get_xw_extent();
	wo = chart_spike_shape_->get_xw_org();
}

float SpikeClassRowItem::get_zoom_y_shapes_mv() const
{
	if (chart_spike_shape_ == nullptr) return 0.f;
	return chart_spike_shape_->get_extent_mv();
}

void SpikeClassRowItem::select_individual_spike(const int no_spike) const
{
	const CdbWaveDoc* p_doc = chart_spike_shape_->get_db_wave_doc();
	db_spike spike_sel(p_doc->db_get_current_record_id(), //db_get_current_record_position(),
		p_doc->m_p_spk_doc->get_index_current_spike_list(), no_spike);
	if (chart_spike_shape_ != nullptr)
		chart_spike_shape_->select_spike(spike_sel);
	return chart_spike_bar_->select_spike(spike_sel);
}

void SpikeClassRowItem::print(CDC* p_dc, CRect* rect1, const CRect* rect2, const CRect* rect3) const
{
	// print text
	const auto text_length = row_comment_.GetLength();
	p_dc->DrawText(row_comment_, text_length, rect1, DT_LEFT | DT_WORDBREAK);

	if (chart_spike_shape_ != nullptr) chart_spike_shape_->print(p_dc, rect2);
	if (chart_spike_bar_ != nullptr) chart_spike_bar_->print(p_dc, rect3);
}

void SpikeClassRowItem::update_string(const int i_class, const int n_spikes)
{
	delete row_comment_;
	const auto c_string = new CString;
	ASSERT(c_string != NULL);
	c_string->Format(_T("class %i\nn=%i"), i_class, n_spikes);
	class_id_ = i_class;

	row_comment_ = *c_string;
}


