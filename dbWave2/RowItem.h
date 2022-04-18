#pragma once
#include <afx.h>

#include "ChartSpikeBar.h"
#include "ChartSpikeShape.h"

class RowItem :
    public CObject
{
public:
	RowItem();
	~RowItem() override;

	void CreateItem(CdbWaveDoc* pdbDoc, SpikeList* spike_list, int i_class, int i_id);

	CString* comment = nullptr;
	ChartSpikeShape* chart_shapes = nullptr;
	ChartSpikeBar* chart_bars = nullptr;
	int class_id = 0;
};

