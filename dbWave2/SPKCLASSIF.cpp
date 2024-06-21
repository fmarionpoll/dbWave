#include "stdafx.h"
#include "SPKCLASSIF.h"

#include "TemplateListWnd.h"


IMPLEMENT_SERIAL(SPKCLASSIF, CObject, 0 /* schema number*/)

SPKCLASSIF::SPKCLASSIF() : b_changed(0), n_int_parameters(0), n_float_parameters(0)
{
	w_version = 2;
	data_transform = 0; // data transform method (0=raw data)
	i_parameter = 0; // type of parameter measured
	i_left = 10; // position of first cursor
	i_right = 40; // position of second cursor
	lower_threshold = 0; // second threshold
	upper_threshold = 10; // first threshold
	i_xy_left = 10;
	i_xy_right = 40;
	source_class = 0; // source class
	dest_class = 0; // destination class
	hit_rate = 50;
	hit_rate_sort = 75;
	k_tolerance = 1.96f;
	k_left = 10;
	k_right = 40;
	row_height = 100; // height of the spike row within spikeview
	col_text = -1;
	col_spikes = 100; // width of the spikes within one row
	col_separator = 5;
	p_template = nullptr;
	mv_min = 0.f;
	mv_max = 2.f;
	v_dest_class = 1;
	v_source_class = 0;
	f_jitter_ms = 1.f;
	b_reset_zoom = TRUE;
}

SPKCLASSIF::~SPKCLASSIF()
{
	if (p_template)
		delete static_cast<CTemplateListWnd*>(p_template);
}

SPKCLASSIF& SPKCLASSIF::operator =(const SPKCLASSIF& arg)
{
	if (this != &arg)
	{
		data_transform = arg.data_transform; // transform mode
		i_parameter = arg.i_parameter; // type of parameter measured
		i_left = arg.i_left; // position of first cursor
		i_right = arg.i_right; // position of second cursor
		lower_threshold = arg.lower_threshold; // second threshold
		upper_threshold = arg.upper_threshold; // first threshold
		i_xy_right = arg.i_xy_right;
		i_xy_left = arg.i_xy_left;
		source_class = arg.source_class; // source class
		dest_class = arg.dest_class; // destination class

		hit_rate = arg.hit_rate;
		hit_rate_sort = arg.hit_rate_sort;
		k_tolerance = arg.k_tolerance;
		k_left = arg.k_left;
		k_right = arg.k_right;
		row_height = arg.row_height;
		col_text = arg.col_text;
		col_spikes = arg.col_spikes;
		col_separator = arg.col_separator;
		v_source_class = arg.v_source_class; // source class
		v_dest_class = arg.v_dest_class; // destination class
		b_reset_zoom = arg.b_reset_zoom;
		f_jitter_ms = arg.f_jitter_ms;

		mv_min = arg.mv_min;
		mv_max = arg.mv_max;

		if (arg.p_template != nullptr)
		{
			p_template = new (CTemplateListWnd);
			*static_cast<CTemplateListWnd*>(p_template) = *static_cast<CTemplateListWnd*>(arg.p_template);
		}
	}
	return *this;
}

void SPKCLASSIF::Serialize(CArchive& ar)
{
	BOOL b_tpl_is_present = FALSE;
	if (ar.IsStoring())
	{
		w_version = 2;
		ar << w_version;
		ar << static_cast<WORD>(data_transform);
		ar << static_cast<WORD>(i_parameter);
		ar << static_cast<WORD>(i_left);
		ar << static_cast<WORD>(i_right);
		ar << static_cast<WORD>(lower_threshold);
		ar << static_cast<WORD>(upper_threshold);
		constexpr auto dummy = static_cast<WORD>(0);
		ar << dummy;
		ar << dummy;

		n_float_parameters = 4;
		ar << n_float_parameters;
		ar << k_tolerance; // 1
		ar << mv_min;
		ar << mv_max;
		ar << f_jitter_ms;

		n_int_parameters = 16;
		ar << n_int_parameters;
		ar << k_left; // 1
		ar << k_right; // 2
		ar << row_height; // 3
		ar << hit_rate; // 4
		ar << hit_rate_sort; // 5
		b_tpl_is_present = p_template != nullptr; // test if template_list is present
		ar << b_tpl_is_present; // 6
		ar << col_text; // 7
		ar << col_spikes; // 8
		ar << col_separator; // 9
		ar << source_class; // 10
		ar << dest_class; // 11
		ar << v_source_class; // 12
		ar << v_dest_class; // 13
		ar << b_reset_zoom; // 14
		ar << i_xy_right; // 15
		ar << i_xy_left; // 16
	}
	else
	{
		WORD version;
		ar >> version;

		// version 1
		WORD w1;
		ar >> w1;
		data_transform = w1;
		ar >> w1;
		i_parameter = w1;
		ar >> w1;
		i_left = w1;
		ar >> w1;
		i_right = w1;
		ar >> w1;
		lower_threshold = w1;
		ar >> w1;
		upper_threshold = w1;
		ar >> w1;
		source_class = w1; // dummy in v4
		ar >> w1;
		dest_class = w1; // dummy in v4

		// version 2
		if (version > 1)
		{
			// float parameters
			int n_temp_float_parameters = 0;
			ar >> n_temp_float_parameters;
			if (n_temp_float_parameters > 0)
			{
				ar >> k_tolerance;
				n_temp_float_parameters--;
			}
			if (n_temp_float_parameters > 0)
			{
				ar >> mv_min;
				n_temp_float_parameters--;
			}
			if (n_temp_float_parameters > 0)
			{
				ar >> mv_max;
				n_temp_float_parameters--;
			}
			if (n_temp_float_parameters > 0)
			{
				ar >> f_jitter_ms;
				n_temp_float_parameters--;
			}
			if (n_temp_float_parameters > 0)
			{
				float dummy;
				do
				{
					ar >> dummy;
					n_temp_float_parameters--;
				} while (n_temp_float_parameters > 0);
			}
			ASSERT(n_temp_float_parameters == 0);

			// int parameters
			ar >> n_temp_float_parameters;
			if (n_temp_float_parameters > 0)
			{
				ar >> k_left;
				n_temp_float_parameters--;
			}
			if (n_temp_float_parameters > 0)
			{
				ar >> k_right;
				n_temp_float_parameters--;
			}
			if (n_temp_float_parameters > 0)
			{
				ar >> row_height;
				n_temp_float_parameters--;
			}
			if (n_temp_float_parameters > 0)
			{
				ar >> hit_rate;
				n_temp_float_parameters--;
			}
			if (n_temp_float_parameters > 0)
			{
				ar >> hit_rate_sort;
				n_temp_float_parameters--;
			}
			if (n_temp_float_parameters > 0)
			{
				ar >> b_tpl_is_present;
				n_temp_float_parameters--;
			}
			if (n_temp_float_parameters > 0)
			{
				ar >> col_text;
				n_temp_float_parameters--;
			}
			if (n_temp_float_parameters > 0)
			{
				ar >> col_spikes;
				n_temp_float_parameters--;
			}
			if (n_temp_float_parameters > 0)
			{
				ar >> col_separator;
				n_temp_float_parameters--;
			}
			if (n_temp_float_parameters > 0)
			{
				ar >> source_class;
				n_temp_float_parameters--;
			}
			if (n_temp_float_parameters > 0)
			{
				ar >> dest_class;
				n_temp_float_parameters--;
			}
			if (n_temp_float_parameters > 0)
			{
				ar >> v_source_class;
				n_temp_float_parameters--;
			}
			if (n_temp_float_parameters > 0)
			{
				ar >> v_dest_class;
				n_temp_float_parameters--;
			}
			if (n_temp_float_parameters > 0)
			{
				ar >> b_reset_zoom;
				n_temp_float_parameters--;
			}
			if (n_temp_float_parameters > 0)
			{
				ar >> i_xy_right;
				n_temp_float_parameters--;
			}
			if (n_temp_float_parameters > 0)
			{
				ar >> i_xy_left;
				n_temp_float_parameters--;
			}

			if (n_temp_float_parameters > 0)
			{
				int dummy;
				do
				{
					ar >> dummy;
					n_temp_float_parameters--;
				} while (n_temp_float_parameters > 0);
			}
			ASSERT(n_temp_float_parameters == 0);

			if (!b_tpl_is_present && p_template != nullptr)
				delete static_cast<CTemplateListWnd*>(p_template);
		}
	}

	// serialize templates
	if (b_tpl_is_present)
	{
		if (p_template == nullptr)
			p_template = new (CTemplateListWnd);
		static_cast<CTemplateListWnd*>(p_template)->Serialize(ar);
	}
}


void SPKCLASSIF::CreateTPL()
{
	p_template = new (CTemplateListWnd);
}
