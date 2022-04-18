#include "StdAfx.h"
#include "RowItem.h"

RowItem::RowItem()
= default;

RowItem::~RowItem()
{
	delete comment;
	delete chart_shapes;
	delete chart_bars;
}

void RowItem::CreateItem(CdbWaveDoc* pdbDoc, SpikeList* spike_list, int i_class, int i_id)
{
	const auto rect_spikes = CRect(0, 0, 0, 0); //CRect(0, 0, size, size);
	const auto rect_bars = CRect(0, 0, 0, 0);

	// 1) create chart_spike_shape
	ChartSpikeShape* chart_spike_shape = nullptr;
	if (spike_list->get_spike_length() > 0)
	{
		chart_spike_shape = new (ChartSpikeShape);
		ASSERT(chart_spike_shape != NULL);

		chart_spike_shape->sub_item_create(rect_spikes, i_id, i_class, pdbDoc, spike_list);
		i_id++;
	}

	// 2) create chart_spike_bar with spike height
	auto* chart_spike_bar = new (ChartSpikeBar);
	ASSERT(chart_spike_bar != NULL);
	chart_spike_bar->sub_item_create(rect_bars, i_id, i_class, pdbDoc, spike_list);
	i_id++;

	// 3) create text
	auto* pcs = new CString();
	pcs->Format(_T("class %i\nn=%i"), i_class, spike_list->get_class_id_n_items(i_class));
	ASSERT(pcs != NULL);

	// 4) create array of 3 pointers and pass it to the listbox
	const auto row_item = new(RowItem);
	ASSERT(row_item != NULL);
	row_item->comment = pcs;
	row_item->chart_shapes = chart_spike_shape;
	row_item->chart_bars = chart_spike_bar;
}
