#include "StdAfx.h"
#include "options_scope_struct.h"


IMPLEMENT_SERIAL(options_scope_struct, CObject, 0 /* schema number*/)

options_scope_struct::options_scope_struct()
= default;

options_scope_struct::~options_scope_struct()
= default;

options_scope_struct& options_scope_struct::operator =(const options_scope_struct& arg)
{
	if (this != &arg)
	{
		i_id = arg.i_id;
		i_x_cells = arg.i_x_cells;
		i_y_cells = arg.i_y_cells;
		i_x_ticks = arg.i_x_ticks;
		i_y_ticks = arg.i_y_ticks;
		i_x_tick_line = arg.i_x_tick_line;
		i_y_tick_line = arg.i_y_tick_line;
		cr_scope_fill = arg.cr_scope_fill;
		cr_scope_grid = arg.cr_scope_grid;
		b_draw_frame = arg.b_draw_frame;
		x_scale_unit_value = arg.x_scale_unit_value;
		y_scale_unit_value = arg.y_scale_unit_value;

		channels.SetSize(arg.channels.GetSize());
		for (int i = 0; i < arg.channels.GetSize(); i++)
			channels[i] = arg.channels[i];
	}
	return *this;
}

void options_scope_struct::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		w_version = 3;
		ar << w_version;
		constexpr int n_int = 8;
		ar << n_int;
		ar << i_id;
		ar << i_x_cells;
		ar << i_y_cells;
		ar << i_x_ticks;
		ar << i_y_ticks;
		ar << i_x_tick_line;
		ar << i_y_tick_line;
		ar << b_draw_frame;
		constexpr int n_color_items = 2;
		ar << n_color_items;
		ar << cr_scope_fill;
		ar << cr_scope_grid;
		// save scale values
		constexpr int n_float = 2;
		ar << n_float;
		ar << x_scale_unit_value;
		ar << y_scale_unit_value;
	}
	else
	{
		WORD w_version;
		ar >> w_version;
		int n_int;
		ar >> n_int;
		ar >> i_id;
		n_int--;
		ar >> i_x_cells;
		n_int--;
		ar >> i_y_cells;
		n_int--;
		ar >> i_x_ticks;
		n_int--;
		ar >> i_y_ticks;
		n_int--;
		ar >> i_x_tick_line;
		n_int--;
		ar >> i_y_tick_line;
		n_int--;
		ar >> b_draw_frame;
		n_int--;
		ASSERT(n_int == 0);

		int n_color_items;
		ar >> n_color_items;
		ar >> cr_scope_fill;
		n_color_items--;
		ar >> cr_scope_grid;
		n_color_items--;
		ASSERT(n_color_items == 0);

		if (w_version > 1)
		{
			int n_float;
			ar >> n_float;
			ar >> x_scale_unit_value;
			n_float--;
			ar >> y_scale_unit_value;
			n_float--;
			while (n_float > 0)
			{
				float x;
				ar >> x;
				n_float--;
			}
		}
	}
}
