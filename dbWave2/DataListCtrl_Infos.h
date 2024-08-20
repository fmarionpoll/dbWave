
#pragma once
#include "ChartWnd.h"

typedef struct tagDataListCtrlInfos
{
	CWnd* parent {nullptr};

	CImageList image_list;
	CBitmap* p_empty_bitmap{ nullptr};

	int image_width{ 400 };
	int image_height{ 50 };
	int data_transform{ 0 };
	int display_mode{ 1 };
	int spike_plot_mode{ PLOT_BLACK };
	int selected_class{ 0 };
	float t_first{ 0.f };
	float t_last{ 0.f };
	float mv_span{ 0.f };

	boolean b_set_time_span{ false };
	boolean b_set_mv_span{ false };
	boolean b_display_file_name{ false };

} DataListCtrlInfos;
