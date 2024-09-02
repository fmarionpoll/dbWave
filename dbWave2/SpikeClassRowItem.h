#pragma once
#include <afx.h>

#include "ChartSpikeBar.h"
#include "ChartSpikeShape.h"
#include "SpikeClassListBoxContext.h"

class SpikeClassRowItem : public CWnd
{
public:
	SpikeClassRowItem();
	~SpikeClassRowItem() override;

	void	create_item(CWnd* parent_wnd, CdbWaveDoc* pdb_doc, SpikeList* p_spike_list, int i_class, int i_id, SpikeClassListBoxContext* context);
	CString get_class_comment(int class_id) const;
	void	draw_item(LPDRAWITEMSTRUCT lp_dis) const;

	void	set_time_intervals(long l_first, long l_last) const;
	void	set_spk_list(SpikeList* p_spike_list) const;
	int		set_mouse_cursor_type(int cursor_m) const;

	int		get_class_id() const { return class_id_; }
	void	set_class_id(int new_class_id);

	int		get_row_id() const { return row_id_; }
	void	set_row_id(const int new_row_id) { row_id_ = new_row_id; }

	void	move_row_out_of_the_way() const;
	void	set_y_zoom(int y_we, int y_wo) const;
	void	set_x_zoom(int x_we, int x_wo) const;
	void	get_time_intervals(long& first, long& last) const;
	void	get_zoom_y(int& we, int& wo) const;
	void	get_zoom_x_shapes(int& we, int& wo) const;
	float	get_zoom_y_shapes_mv() const;
	void	select_individual_spike(int no_spike) const;
	void	print(CDC* p_dc, CRect* rect1, const CRect* rect2, const CRect* rect3) const;
	void	update_string(int i_class, int n_spikes);

	ChartSpikeShape* get_chart_shape() const { return chart_spike_shape_; }
	ChartSpikeBar* get_chart_bars() const { return chart_spike_bar_; }

protected:
	CString row_comment_  {};
	ChartSpikeShape* chart_spike_shape_  {nullptr};
	ChartSpikeBar* chart_spike_bar_  {nullptr};
	SpikeList* spike_list_  {nullptr};

	// TODO replace with spike class descriptor?
	int class_id_  {0};
	int row_id_  {0};
	SpikeClassListBoxContext* parent_context_  {nullptr};
};

