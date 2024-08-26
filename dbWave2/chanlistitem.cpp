// chanlistitem.cpp
//

#include "StdAfx.h"
#include <stdlib.h>
#include "Envelope.h"
#include "chanlistitem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
//	CChanlistItem
//
// lineview keeps a list of channels to be displayed in a list
// each item of this list group together the parameters necessary to
// display the channel, ie color, etc and a pointer to structures
// describing the abscissa and the ordinates
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_SERIAL(CChanlistItem, CObject, 1)

CChanlistItem::CChanlistItem()
= default;

CChanlistItem::CChanlistItem(CEnvelope* p_envelope_X, const int index_x, CEnvelope* p_envelope_Y, const int index_y)
{
	p_envelope_abscissa = p_envelope_X;
	p_envelope_ordinates = p_envelope_Y;
	dl_index_abscissa_ = index_x;
	dl_index_ordinates_ = index_y;
}

void CChanlistItem::init_display_parameters(const WORD new_pen_width, const WORD new_color, const int new_zero, const int new_y_extent,
                                            const WORD new_draw_mode)
{
	dl_pen_width_ = new_pen_width;
	dl_color_ = new_color;
	dl_y_zero_ = new_zero;
	dl_y_extent_ = new_y_extent;
	dl_b_print_ = new_draw_mode;
	dl_data_volts_per_bin_ = static_cast<float>(20.) / dl_y_extent_;
	dl_data_bin_zero_ = 2048;
	dl_data_bin_span_ = 4096;
	dl_b_hz_tags_print_ = FALSE;
	dl_data_volt_span_ = 0.0f;
}

void CChanlistItem::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << dl_y_zero_; // zero volts
		ar << dl_y_extent_; // max to min extent
		ar << dl_pen_width_; // pen size
		ar << dl_color_; // color
		ar << dl_b_print_; // draw mode
		ar << dl_data_volts_per_bin_; // scale factor (1 unit (0-4095) -> y volts)
		ar << dl_data_bin_zero_; // value of zero volts
		ar << dl_data_bin_span_; // nb of bins encoding values within envelope
		ar << dl_b_hz_tags_print_; // print HZ tags flag
		ar << dl_index_abscissa_;
		ar << dl_index_ordinates_;
	}
	else
	{
		ar >> dl_y_zero_; // zero volts
		ar >> dl_y_extent_; // max to min extent
		ar >> dl_pen_width_; // pen size
		ar >> dl_color_; // color
		ar >> dl_b_print_; // draw mode
		ar >> dl_data_volts_per_bin_; // scale factor (1 unit (0-4095) -> y volts)
		ar >> dl_data_bin_zero_; // value of zero volts
		ar >> dl_data_bin_span_; // nb of bins encoding values within envelope
		ar >> dl_b_hz_tags_print_; // print HZ tags flag
		ar >> dl_index_abscissa_;
		ar >> dl_index_ordinates_;
	}
}

void CChanlistItem::get_envelope_array_indexes(int& x, int& y) const
{
	x = dl_index_abscissa_;
	y = dl_index_ordinates_;
}

void CChanlistItem::set_envelope_arrays(CEnvelope* px, const int x, CEnvelope* py, const int y)
{
	dl_index_abscissa_ = x;
	dl_index_ordinates_ = y;
	p_envelope_abscissa = px;
	p_envelope_ordinates = py;
}
