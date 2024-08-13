#include "stdafx.h"
#include "options_spk_classification.h"

#include "TemplateListWnd.h"


IMPLEMENT_SERIAL(options_spk_classification, CObject, 0 /* schema number*/)

options_spk_classification::options_spk_classification() : b_changed(0), n_int_parameters(0), n_float_parameters(0)
{
}

options_spk_classification::~options_spk_classification()
{
	if (p_template)
		delete static_cast<CTemplateListWnd*>(p_template);
}

options_spk_classification& options_spk_classification::operator =(const options_spk_classification& arg)
{
	if (this != &arg)
	{
		data_transform = arg.data_transform; // transform mode
		i_parameter = arg.i_parameter; // type of parameter measured
		shape_t1 = arg.shape_t1; // position of first cursor
		shape_t2 = arg.shape_t2; // position of second cursor
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

void options_spk_classification::Serialize(CArchive& ar)
{
	BOOL b_tpl_is_present = FALSE;
	if (ar.IsStoring())
	{
		w_version = 2;
		ar << w_version;
		ar << static_cast<WORD>(data_transform);
		ar << static_cast<WORD>(i_parameter);
		ar << static_cast<WORD>(shape_t1);
		ar << static_cast<WORD>(shape_t2);
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
		shape_t1 = w1;
		ar >> w1;
		shape_t2 = w1;
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


void options_spk_classification::create_tpl()
{
	p_template = new (CTemplateListWnd);
}
