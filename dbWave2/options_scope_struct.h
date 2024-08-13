#pragma once
#include "OPTIONS_SCOPE_CHANNEL.h"

class options_scope_struct final : public CObject
{
	DECLARE_SERIAL(options_scope_struct)
	options_scope_struct();
	~options_scope_struct() override;
	options_scope_struct& operator =(const options_scope_struct& arg);
	void Serialize(CArchive& ar) override;

public:
	WORD w_version{2};
	int i_id{0};
	int i_x_cells{1};
	int i_y_cells{1};
	int i_x_ticks{0};
	int i_y_ticks{0};
	int i_x_tick_line{0};
	int i_y_tick_line{0};
	COLORREF cr_scope_fill{RGB(255, 255, 255)};
	COLORREF cr_scope_grid{RGB(255, 255, 255)};
	BOOL b_draw_frame{true};
	BOOL b_clip_rect{false};
	float x_scale_unit_value{0.f};
	float y_scale_unit_value{0.f};
	float x_scale_span_s{0.f};
	float y_scale_span_v{0.f};

	CArray<options_scope_channel, options_scope_channel> channels{};
};
