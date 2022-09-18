#pragma once
#include <afx.h>

#include "ChartSpikeBar.h"
#include "ChartSpikeShape.h"
#include "SpikeClassListBoxContext.h"

class RowItem :
    public CWnd
{
public:
	RowItem();
	~RowItem() override;

	void	CreateItem(CWnd* parentWnd, CdbWaveDoc* pdbDoc, SpikeList* spike_list, int i_class, int i_id, SpikeClassListBoxContext* context);
	void	DrawItem(LPDRAWITEMSTRUCT lpDIS) const;
	void	set_time_intervals(long l_first, long l_last) const;
	void	set_spk_list(SpikeList* p_spike_list) const;
	int		set_mouse_cursor_type(int cursor_m) const;

	int		get_class_id() const { return class_id; }
	void	set_class_id(int new_class_id) { class_id = new_class_id; }
	int		get_row_id() const { return row_id; }
	void	set_row_id(int new_row_id) { row_id = new_row_id; }

	void	move_row_out_of_the_way() const;
	void	set_y_zoom(int y_we, int y_wo) const;
	void	set_x_zoom(int x_we, int x_wo) const;
	void	get_time_intervals(long& first, long& last) const;
	void	get_zoom_y(int& we, int& wo) const;
	void	get_zoom_x_shapes(int& we, int& wo) const;
	float	get_zoom_y_shapes_mv() const;
	int		select_individual_spike(int no_spike);
	void	print(CDC* p_dc, CRect* rect1, CRect* rect2, CRect* rect3) const;
	void	update_string(int i_class, int n_spikes);

	ChartSpikeShape* get_chart_shape() const { return chart_shapes; };
	ChartSpikeBar* get_chart_bars() const { return chart_bars; }

protected:
	CString* row_comment = nullptr;
	ChartSpikeShape* chart_shapes = nullptr;
	ChartSpikeBar* chart_bars = nullptr;

	// TODO replace with spikeclassdescriptor?
	int class_id = 0;
	int row_id = 0;
	SpikeClassListBoxContext* parent_context = nullptr;
};

