#include "StdAfx.h"
#include "ChartDataAD.h"



void ChartDataAD::start_display(int points_per_channel)
{
	// init parameters related to AD display
	m_b_ad_buffers_ = TRUE; // yes, display AD buffers
	m_l_ad_buffer_done_ = 0; // length of data already displayed
	const auto envelope = envelope_ptr_array_.GetAt(0);
	envelope->fill_envelope_with_abscissa_ex(1, display_rect_.right - 1, points_per_channel);
	envelope->export_to_abscissa(m_poly_points_);
	set_b_use_dib(FALSE);

	// clear window before acquiring data
	CClientDC dc(this);
	erase_background(&dc);

	// print comment
	dc.SelectObject(GetStockObject(DEFAULT_GUI_FONT));
	auto rect = display_rect_;
	rect.DeflateRect(1, 1);
	CString cs = _T("Waiting for trigger");
	const auto text_length = cs.GetLength();
	dc.DrawText(cs, text_length, rect, DT_LEFT);
}

void ChartDataAD::display_buffer(short* samples_buffer, long samples_number)
{
	// check data wrap
	if (m_l_ad_buffer_done_ + samples_number > m_lx_size_)
	{
		const long pixels_left_until_end_of_Display = m_lx_size_ - m_l_ad_buffer_done_;
		if (pixels_left_until_end_of_Display > 0)
		{
			display_buffer(samples_buffer, pixels_left_until_end_of_Display);
			samples_number -= pixels_left_until_end_of_Display;
			samples_buffer += (pixels_left_until_end_of_Display * m_p_data_file_->get_wave_format()->scan_count);
		}
		m_l_ad_buffer_done_ = 0;
	}

	// create device context and prepare bitmap to receive drawing commands
	CClientDC dc(this);
	CDC dc_mem;
	dc_mem.CreateCompatibleDC(&dc);
	CBitmap bitmap_plot;
	const auto bitmap_old = dc_mem.SelectObject(&bitmap_plot);
	const auto dc_old = dc_mem.SaveDC();
	CDC* p_dc = &dc;
	if (b_use_dib_)
		p_dc = &dc_mem;

	// get first and last pixels of the interval to display
	const int ad_pixel_first = MulDiv(m_l_ad_buffer_done_, m_n_pixels_, m_lx_size_);
	int ad_pixel_last = MulDiv(m_l_ad_buffer_done_ + samples_number - 1, m_n_pixels_, m_lx_size_);
	if (ad_pixel_last > display_rect_.right - 2)
		ad_pixel_last = display_rect_.right - 2;

	const int display_pixels = ad_pixel_last - ad_pixel_first + 1;
	const int display_data_points = display_pixels * m_data_per_pixel_;


	const auto points_to_display = &m_poly_points_[ad_pixel_first * m_data_per_pixel_ * 2];

	CRect rect(ad_pixel_first, display_rect_.top, ad_pixel_last, display_rect_.bottom);
	if (ad_pixel_first == 0)
		rect.left = 1;
	rect.DeflateRect(0, 1);
	p_dc->IntersectClipRect(rect);
	p_dc->FillSolidRect(&rect, scope_structure_.cr_scope_fill);

	auto* pen_old = static_cast<CPen*>(p_dc->SelectStockObject(BLACK_PEN));

	p_dc->SetMapMode(MM_ANISOTROPIC);
	p_dc->SetViewportExt(x_viewport_extent_, y_ve_);
	p_dc->SetViewportOrg(x_viewport_origin_, y_vo_);
	p_dc->SetWindowExt(m_n_pixels_, y_ve_); 
	p_dc->SetWindowOrg(0, 0); 
	const auto yVE = y_ve_;

	for (int channel_number = 0; channel_number < chan_list_item_ptr_array_.GetSize(); channel_number++)
	{
		// load channel descriptors
		const auto channel_item = chan_list_item_ptr_array_[channel_number];
		CPen temp_pen;
		temp_pen.CreatePen(PS_SOLID, 0, color_spike_class[channel_item->get_color_index()]);
		p_dc->SelectObject(&temp_pen);

		// compute max min and plot array
		auto ppt = points_to_display;
		auto p_data = samples_buffer + channel_number;
		const auto y_zero = channel_item->get_y_zero();
		const auto y_extent = channel_item->get_y_extent();
		const int n_channels = m_p_data_file_->get_wave_format()->scan_count;

		// only one data point per pixel
		if (m_data_per_pixel_ > 1)
		{
			int number_of_elements_displayed = 0;
			for (int i = 0; i < display_pixels; i++)
			{
				auto i_max = *p_data;
				auto i_min = i_max;
				int number_of_elements = ((i + 1) * samples_number / display_pixels) - number_of_elements_displayed;
				number_of_elements_displayed += number_of_elements;
				while (number_of_elements > 0)
				{
					const auto value_to_display = *p_data;
					if (value_to_display < i_min)
						i_min = value_to_display;
					else if (value_to_display > i_max)
						i_max = value_to_display;
					p_data += n_channels;
					number_of_elements--;
				}

				ppt->y = MulDiv(i_max - y_zero, yVE, y_extent);
				ppt->x = i + ad_pixel_first;
				ppt++;
				ppt->y = MulDiv(i_min - y_zero, yVE, y_extent);
				ppt->x = i + ad_pixel_first;
				ppt++;
			}
		}

		// 1 point per pixel
		else
		{
			for (auto i = 0; i < display_pixels; i++, ppt++, p_data += n_channels)
			{
				ppt->y = MulDiv(*p_data - y_zero, yVE, y_extent);
				ppt->x = i + ad_pixel_first;
			}
		}
		p_dc->MoveTo(static_cast<int>(points_to_display->x - 2), static_cast<int>(points_to_display->y));
		p_dc->PolylineTo(points_to_display, display_data_points);
		temp_pen.DeleteObject();
	}

	// restore the old pen and exit
	p_dc->SelectObject(pen_old);
	p_dc->RestoreDC(dc_old);
	p_dc->SetMapMode(MM_TEXT);
	p_dc->SetViewportOrg(0, 0);
	p_dc->SetWindowOrg(0, 0);

	if (b_use_dib_)
		dc.BitBlt(rect.left, rect.top,
			rect.Width(), rect.Height(),
			&dc_mem,
			rect.left, rect.top,
			SRCCOPY);

	dc_mem.SelectObject(bitmap_old);
	m_l_ad_buffer_done_ += samples_number;
}
