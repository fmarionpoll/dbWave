#include "StdAfx.h"
#include "RowItem.h"

RowItem::RowItem()
= default;

RowItem::~RowItem()
{
	delete row_comment;
	delete chart_spike_shape;
	delete chart_spike_bar;
}

void RowItem::CreateItem(CWnd* parentWnd, CdbWaveDoc* pdbDoc, SpikeList* spike_list, int i_class, int i_id, SpikeClassListBoxContext* context)
{
	const auto rect_spikes = CRect(0, 0, 0, 0); 
	const auto rect_bars = CRect(0, 0, 0, 0);
	parent_context = context;
	class_id = i_class;
	row_id = i_id;

	// 1) create chart_spike_shape
	if (spike_list->get_spike_length() > 0)
	{
		chart_spike_shape = new (ChartSpikeShape);
		ASSERT(chart_spike_shape != NULL);
		chart_spike_shape->sub_item_create(parentWnd, rect_spikes, i_id, i_class, pdbDoc, spike_list);
		i_id++;
	}

	// 2) create chart_spike_bar with spike height
	chart_spike_bar = new (ChartSpikeBar);
	ASSERT(chart_spike_bar != NULL);
	chart_spike_bar->sub_item_create(parentWnd, rect_bars, i_id, i_class, pdbDoc, spike_list);

	// 3) create text
	row_comment = new CString();
	ASSERT(row_comment != NULL);
	row_comment->Format(_T("class %i\nn=%i"), i_class, spike_list->get_class_id_n_items(i_class));
}

void RowItem::DrawItem(LPDRAWITEMSTRUCT lpDIS) const
{
	CDC dc;
	dc.Attach(lpDIS->hDC);

	if (lpDIS->itemAction & ODA_DRAWENTIRE)
	{
		// get data
		CRect rc_text = lpDIS->rcItem;
		rc_text.right = rc_text.left + parent_context->m_widthText;
	

		// display text
		const auto text_length = row_comment->GetLength();
		dc.DrawText(*row_comment, text_length, rc_text, DT_LEFT | DT_WORDBREAK);

		// display spikes
		const auto col1 = parent_context->m_widthText + parent_context->m_widthSeparator;
		const auto col2 = col1 + parent_context->m_widthSpikes + parent_context->m_widthSeparator;
		auto rect_spikes = CRect(col1 + 1, lpDIS->rcItem.top + 1, col1 + parent_context->m_widthSpikes, lpDIS->rcItem.bottom - 1);
		chart_spike_shape->sub_item_draw(dc, rect_spikes);

		// display bars
		auto rect_bars = CRect(col2 + 1, lpDIS->rcItem.top + 1, col2 + parent_context->m_widthBars, lpDIS->rcItem.bottom - 1);
		chart_spike_bar->sub_item_draw(dc, rect_bars);
	}

	// item is selected -- add frame
	if ((lpDIS->itemState & ODS_SELECTED) &&
		(lpDIS->itemAction & (ODA_SELECT | ODA_DRAWENTIRE)))
	{
		constexpr auto color_blue = RGB(0, 0, 255);
		CBrush br(color_blue);
		dc.FrameRect(&(lpDIS->rcItem), &br);
	}

	// Item has been de-selected -- remove frame
	if (!(lpDIS->itemState & ODS_SELECTED) && // item not selected
		(lpDIS->itemAction & ODA_SELECT))
	{
		dc.FrameRect(&(lpDIS->rcItem), &parent_context->m_brush_background);
	}
	dc.Detach();
}

void RowItem::set_time_intervals(long l_first, long l_last) const
{
	if (chart_spike_shape != nullptr)
	{
		chart_spike_shape->SetRangeMode(RANGE_TIMEINTERVALS);
		chart_spike_shape->SetTimeIntervals(l_first, l_last);
	}
	chart_spike_bar->SetRangeMode(RANGE_TIMEINTERVALS);
	chart_spike_bar->SetTimeIntervals(l_first, l_last);
}

void RowItem::set_spk_list(SpikeList* p_spike_list) const
{
	chart_spike_shape->SetSpkList(p_spike_list);
	chart_spike_bar->SetSpkList(p_spike_list);
}

int RowItem::set_mouse_cursor_type(int cursor_m) const
{
	if (chart_spike_shape != nullptr)
		chart_spike_shape->SetMouseCursorType(cursor_m);
	return chart_spike_bar->SetMouseCursorType(cursor_m);
}

void RowItem::move_row_out_of_the_way() const
{
	CRect rect(0, 0, 0, 0);
	if (chart_spike_shape != nullptr)
		chart_spike_shape->MoveWindow(rect, FALSE);
	chart_spike_bar->MoveWindow(rect, FALSE);
}

void RowItem::set_y_zoom(int y_we, int y_wo) const
{
	if (chart_spike_shape != nullptr)
		chart_spike_shape->SetYWExtOrg(y_we, y_wo);
	chart_spike_bar->SetYWExtOrg(y_we, y_wo);
}

void RowItem::set_x_zoom(int x_we, int x_wo) const
{
	if (chart_spike_shape != nullptr)
		chart_spike_shape->SetXWExtOrg(x_we, x_wo);
}

void RowItem::get_time_intervals(long& first, long& last) const
{
	first = chart_spike_bar->GetTimeFirst();
	last = chart_spike_bar->GetTimeLast();
}

void RowItem::get_zoom_y(int& we, int& wo) const
{
	we = chart_spike_bar->GetYWExtent();
	wo = chart_spike_bar->GetYWOrg();
}

void RowItem::get_zoom_x_shapes(int& we, int& wo) const
{
	if (chart_spike_shape == nullptr) return;

	we = chart_spike_shape->GetXWExtent();
	wo = chart_spike_shape->GetXWOrg();
}

float RowItem::get_zoom_y_shapes_mv() const
{
	if (chart_spike_shape == nullptr) return 0.f;
	return chart_spike_shape->GetExtent_mV();
}

int RowItem::select_individual_spike(int no_spike)
{
	if (chart_spike_shape != nullptr)
		chart_spike_shape->SelectSpikeShape(no_spike);
	return chart_spike_bar->SelectSpike(no_spike);
}

void RowItem::print(CDC* p_dc, CRect* rect1, CRect* rect2, CRect* rect3) const
{
	// print text
	const auto text_length = row_comment->GetLength();
	p_dc->DrawText(*row_comment, text_length, rect1, DT_LEFT | DT_WORDBREAK);

	// spike shape
	if (chart_spike_shape != nullptr)
		chart_spike_shape->Print(p_dc, rect2);

	// spike bars
	if (chart_spike_bar != nullptr)
		chart_spike_bar->Print(p_dc, rect3);
}

void RowItem::update_string(int i_class, int n_spikes)
{
	delete row_comment;
	const auto c_string = new CString;
	ASSERT(c_string != NULL);
	c_string->Format(_T("class %i\nn=%i"), i_class, n_spikes);
	class_id = i_class;

	row_comment = c_string;
}


