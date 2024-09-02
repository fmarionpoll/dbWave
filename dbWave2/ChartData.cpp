#include "StdAfx.h"
#include <cstdlib>
#include <strsafe.h>
#include "resource.h"
#include "ChartData.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_SERIAL(ChartData, ChartWnd, 1)

ChartData::ChartData()
{
	add_channel_list_item(0, 0);
	resize_channels(m_n_pixels_, 1024);
	cs_empty_ = _T("no data to display");
}

ChartData::~ChartData()
{
	remove_all_channel_list_items();
}

BEGIN_MESSAGE_MAP(ChartData, ChartWnd)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_SIZE()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

void ChartData::remove_all_channel_list_items()
{
	for (auto i = envelope_ptr_array_.GetUpperBound(); i >= 0; i--)
		delete envelope_ptr_array_[i];
	envelope_ptr_array_.RemoveAll();

	for (auto i = chan_list_item_ptr_array_.GetUpperBound(); i >= 0; i--)
		delete chan_list_item_ptr_array_[i];
	chan_list_item_ptr_array_.RemoveAll();
}

int ChartData::add_channel_list_item(int ns, int mode)
{
	// first time??	create Envelope(0) with abscissa series
	if (chan_list_item_ptr_array_.GetSize() == 0)
	{
		m_poly_points_.SetSize(m_n_pixels_ * 4);
		m_scale_.set_scale(m_n_pixels_, m_lx_size_);
		m_data_per_pixel_ = 2;
		auto* p_envelope = new CEnvelope(static_cast<WORD>(m_n_pixels_ * m_data_per_pixel_), m_data_per_pixel_, 0, -1, 0);
		ASSERT(p_envelope != NULL);
		envelope_ptr_array_.Add(p_envelope);
		p_envelope->fill_envelope_with_abscissa(m_n_pixels_ * m_data_per_pixel_, m_lx_size_);
	}

	// create new Envelope and store pointer into Envelopes_list
	auto span = 0;
	if (m_p_data_file_ != nullptr)
		span = AcqDataDoc::get_transformed_data_span(mode);
	auto* p_envelope_y = new CEnvelope(static_cast<WORD>(m_n_pixels_), m_data_per_pixel_, ns, mode, span);
	ASSERT(p_envelope_y != NULL);
	const auto j = envelope_ptr_array_.Add(p_envelope_y);

	// create new chan_list_item with x=Envelope(0) and y=the new Envelope
	auto* p_chan_list_item = new CChanlistItem(envelope_ptr_array_.GetAt(0), 0, p_envelope_y, j);
	ASSERT(p_chan_list_item != NULL);
	const auto index_new_chan = chan_list_item_ptr_array_.Add(p_chan_list_item);

	// init display parameters
	p_chan_list_item->init_display_parameters(1, RGB(0, 0, 0), 2048, 4096);

	if (m_p_data_file_ != nullptr)
	{
		float volts_per_bin;
		m_p_data_file_->get_volts_per_bin(ns, &volts_per_bin, mode);
		const auto wave_chan_array = m_p_data_file_->get_wave_channels_array();
		const auto wave_format = m_p_data_file_->get_wave_format();
		p_chan_list_item->set_data_bin_format(wave_format->bin_zero, wave_format->bin_span);
		p_chan_list_item->set_data_volts_format(volts_per_bin, wave_format->full_scale_volts);
		if (ns >= wave_chan_array->chan_array_get_size())
			ns = 0;
		const auto p_channel = wave_chan_array->get_p_channel(ns);
		p_chan_list_item->dl_comment = p_channel->am_csComment; 
		update_chan_list_max_span(); 
		if (mode > 0) 
			p_chan_list_item->dl_comment = 
			(AcqDataDoc::get_transform_name(mode)).Left(8) + ": " + p_chan_list_item->dl_comment;
	}
	return index_new_chan;
}

int ChartData::remove_channel_list_item(int i_chan)
{
	const auto chan_list_size = chan_list_item_ptr_array_.GetSize(); 
	if (chan_list_size > 0) 
	{
		const auto pa = chan_list_item_ptr_array_[i_chan]->p_envelope_ordinates;
		// step 1: check that this envelope is not used by another channel
		auto b_used_only_once = TRUE;
		for (auto lj = chan_list_size; lj >= 0; lj--)
		{
			const auto p_envelope_y = chan_list_item_ptr_array_[i_chan]->p_envelope_ordinates;
			if (pa == p_envelope_y && lj != i_chan)
			{
				b_used_only_once = FALSE; 
				break; 
			}
		}
		// step 2: delete corresponding envelope only if envelope used only once.
		if (b_used_only_once)
		{
			for (auto k = envelope_ptr_array_.GetUpperBound(); k >= 0; k--)
			{
				const auto pb = envelope_ptr_array_[k];
				if (pa == pb) 
				{
					delete pa; 
					envelope_ptr_array_.RemoveAt(k);
					break; 
				}
			}
		}
		// step 3: delete channel
		delete chan_list_item_ptr_array_[i_chan];
		chan_list_item_ptr_array_.RemoveAt(i_chan);
	}
	update_chan_list_max_span();
	return chan_list_size - 1;
}

void ChartData::update_chan_list_max_span()
{
	if (envelope_ptr_array_.GetSize() <= 1)
		return;

	auto imax = 0;
	for (auto i = envelope_ptr_array_.GetUpperBound(); i > 0; i--)
	{
		const auto j = envelope_ptr_array_[i]->get_doc_buffer_span();
		if (j > imax)
			imax = j;
	}
	envelope_ptr_array_[0]->set_doc_buffer_span(imax);
}

void ChartData::update_chan_list_from_doc()
{
	for (auto i = chan_list_item_ptr_array_.GetUpperBound(); i >= 0; i--)
	{
		const auto chan_list_item = chan_list_item_ptr_array_[i];
		const auto p_ord = chan_list_item->p_envelope_ordinates;
		const auto ns = p_ord->get_source_chan();
		const auto mode = p_ord->get_source_mode();
		p_ord->set_doc_buffer_span(AcqDataDoc::get_transformed_data_span(mode));
		const auto wave_chan_array = m_p_data_file_->get_wave_channels_array();
		const auto p_chan = wave_chan_array->get_p_channel(ns);
		chan_list_item->dl_comment = p_chan->am_csComment;
		if (mode > 0)
			chan_list_item->dl_comment = 
			 (AcqDataDoc::get_transform_name(mode)).Left(6) + ": " + chan_list_item->dl_comment;
		update_gain_settings(i);
	}
	update_chan_list_max_span(); 
}

void ChartData::update_gain_settings(const int i_chan)
{
	CChanlistItem* p_chan = chan_list_item_ptr_array_[i_chan];
	const auto p_ord = p_chan->p_envelope_ordinates;
	const auto ns = p_ord->get_source_chan();
	const auto mode = p_ord->get_source_mode();
	float doc_volts_per_bin;
	m_p_data_file_->get_volts_per_bin(ns, &doc_volts_per_bin, mode);
	const auto volts_per_data_bin = p_chan->get_volts_per_bin();
	const auto wave_format = m_p_data_file_->get_wave_format();
	if (doc_volts_per_bin > volts_per_data_bin || doc_volts_per_bin < volts_per_data_bin)
	{
		p_chan->set_data_bin_format(wave_format->bin_zero, wave_format->bin_span);
		p_chan->set_data_volts_format(doc_volts_per_bin, wave_format->full_scale_volts);
		auto i_extent = p_chan->get_y_extent();
		i_extent = static_cast<int>(static_cast<float>(i_extent) / doc_volts_per_bin * volts_per_data_bin);
		p_chan->set_y_extent(i_extent);
	}
}

int ChartData::set_channel_list_source_channel(const int i_channel, const int acq_channel)
{
	// check if channel is allowed
	const auto wave_format = m_p_data_file_->get_wave_format();
	if (wave_format->scan_count <= acq_channel || acq_channel < 0)
		return -1;

	// make sure we have enough data channels...
	if (get_channel_list_size() <= i_channel)
		for (auto j = get_channel_list_size(); j <= i_channel; j++)
			add_channel_list_item(j, 0);

	// change channel
	const auto chan_list_item = chan_list_item_ptr_array_[i_channel];
	const auto p_ord = chan_list_item->p_envelope_ordinates;
	p_ord->set_source_chan(acq_channel);
	const auto mode = p_ord->get_source_mode();
	// modify comment
	const auto wave_chan_array = m_p_data_file_->get_wave_channels_array();
	const auto p_channel = wave_chan_array->get_p_channel(acq_channel);
	chan_list_item->dl_comment = p_channel->am_csComment;
	if (mode > 0)
		chan_list_item->dl_comment = (AcqDataDoc::get_transform_name(mode)).Left(6) + _T(": ") + chan_list_item->
			dl_comment;
	update_gain_settings(i_channel);
	return acq_channel;
}

void ChartData::set_channel_list_y(const int i_chan, int acq_chan, const int transform)
{
	const auto chan_list_item = chan_list_item_ptr_array_[i_chan];
	chan_list_item->set_ordinates_source_data(acq_chan, transform);
	const auto p_channel_array = m_p_data_file_->get_wave_channels_array();
	if (acq_chan >= p_channel_array->chan_array_get_size())
		acq_chan = 0;
	const auto p_channel = p_channel_array->get_p_channel(acq_chan);
	chan_list_item->dl_comment = p_channel->am_csComment;
	if (transform > 0)
		chan_list_item->dl_comment = (AcqDataDoc::get_transform_name(transform)).Left(6) + _T(": ") + chan_list_item->
			dl_comment;
}

void ChartData::set_channel_list_volts_extent(const int i_chan, const float* p_value)
{
	auto i_chan_first = i_chan;
	auto i_chan_last = i_chan;
	if (i_chan < 0)
	{
		i_chan_first = 0;
		i_chan_last = chan_list_item_ptr_array_.GetUpperBound();
	}
	auto volts_extent = 0.f;
	if (p_value != nullptr)
		volts_extent = *p_value;
	for (auto i = i_chan_first; i <= i_chan_last; i++)
	{
		const auto chan_list_item = chan_list_item_ptr_array_[i];
		const auto y_volts_per_bin = chan_list_item->get_volts_per_bin();
		if (p_value == nullptr)
			volts_extent = y_volts_per_bin * static_cast<float>(chan_list_item->get_y_extent());

		const auto y_extent = static_cast<int>(volts_extent / y_volts_per_bin);
		chan_list_item->set_y_extent(y_extent);
	}
}

void ChartData::set_channel_list_volts_zero(const int i_chan, const float* p_value)
{
	auto i_chan_first = i_chan;
	auto i_chan_last = i_chan;
	if (i_chan < 0)
	{
		i_chan_first = 0;
		i_chan_last = chan_list_item_ptr_array_.GetUpperBound();
	}

	auto volts_extent = 0.f;
	if (p_value != nullptr)
		volts_extent = *p_value;
	for (auto i = i_chan_first; i <= i_chan_last; i++)
	{
		const auto chan_list_item = chan_list_item_ptr_array_[i];
		const auto y_volts_per_bin = chan_list_item->get_volts_per_bin();
		if (p_value == nullptr)
			volts_extent = y_volts_per_bin * static_cast<float>(chan_list_item->get_data_bin_zero());

		const auto i_y_zero = static_cast<int>(volts_extent / y_volts_per_bin);
		chan_list_item->set_y_zero(i_y_zero);
	}
}

int ChartData::set_channel_list_transform_mode(const int i_chan, const int i_mode)
{
	// check if transform is allowed
	if (!AcqDataDoc::is_wb_transform_allowed(i_mode) ||
		!m_p_data_file_->init_wb_transform_buffer()) 
	{
		AfxMessageBox(IDS_LNVERR02, MB_OK);
		return -1;
	}

	// change transform mode
	const auto chan_list_item = chan_list_item_ptr_array_[i_chan];
	const auto p_ord = chan_list_item->p_envelope_ordinates;
	const auto ns = p_ord->get_source_chan();
	p_ord->set_source_mode(i_mode, AcqDataDoc::get_transformed_data_span(i_mode));

	// modify comment
	const auto wave_chan_array = m_p_data_file_->get_wave_channels_array();
	const auto p_channel = wave_chan_array->get_p_channel(ns);
	chan_list_item->dl_comment = p_channel->am_csComment;
	if (i_mode > 0)
		chan_list_item->dl_comment = (AcqDataDoc::get_transform_name(i_mode)).Left(8)
			+ _T(": ") + chan_list_item->dl_comment;
	update_gain_settings(i_chan);
	update_chan_list_max_span();
	return i_mode;
}

options_scope_struct* ChartData::get_scope_parameters()
{
	const auto n_channels = chan_list_item_ptr_array_.GetSize();
	scope_structure_.channels.SetSize(n_channels);
	for (auto i = 0; i < n_channels; i++)
	{
		const auto chan_list_item = chan_list_item_ptr_array_[i];
		scope_structure_.channels[i].i_zero = chan_list_item->get_y_zero();
		scope_structure_.channels[i].i_extent = chan_list_item->get_y_extent();
	}

	return &scope_structure_;
}

void ChartData::set_scope_parameters(options_scope_struct* p_struct)
{
	const auto n_channels_struct = p_struct->channels.GetSize();
	const auto n_channels_chan_list = chan_list_item_ptr_array_.GetSize();
	for (auto i = 0; i < n_channels_struct; i++)
	{
		if (i >= n_channels_chan_list)
			break;
		const auto chan_list_item = chan_list_item_ptr_array_[i];
		chan_list_item->set_y_zero(p_struct->channels[i].i_zero);
		chan_list_item->set_y_extent(p_struct->channels[i].i_extent);
	}
}

void ChartData::auto_zoom_chan(const int channel_index) const
{
	auto i1 = channel_index;
	auto i2 = channel_index;
	if (channel_index < 0)
	{
		i1 = 0;
		i2 = get_channel_list_size() - 1;
	}

	int max, min;
	for (auto i = i1; i <= i2; i++)
	{
		CChanlistItem* chan = get_channel_list_item(i);
		chan->get_max_min(&max, &min);
		chan->set_y_zero((max + min) / 2);
		chan->set_y_extent(MulDiv(max - min + 1, 10, 8));
	}
}

void ChartData::split_channels() const
{
	int max, min;
	const auto n_channels = get_channel_list_size();
	auto i_cur = n_channels - 1;
	for (auto i = 0; i < n_channels; i++, i_cur -= 2)
	{
		CChanlistItem* chan = get_channel_list_item(i);
		chan->get_max_min(&max, &min);
		const auto amplitude_span = MulDiv((max - min), 12 * n_channels, 10);
		chan->set_y_extent(amplitude_span);
		const auto offset = (max + min) / 2 + MulDiv(amplitude_span, i_cur, n_channels * 2);
		chan->set_y_zero(offset);
	}
}

void ChartData::center_chan(const int channel_index) const
{
	auto i1 = channel_index;
	auto i2 = channel_index;
	if (channel_index < 0)
	{
		i1 = 0;
		i2 = get_channel_list_size() - 1;
	}

	int max, min;
	for (auto item = i1; item <= i2; item++)
	{
		CChanlistItem* chan = get_channel_list_item(item);
		chan->get_max_min(&max, &min);
		const auto y_zero = (max + min) / 2;
		chan->set_y_zero(y_zero);
	}
}

void ChartData::max_gain_chan(const int channel_index) const
{
	auto i1 = channel_index;
	auto i2 = channel_index;
	if (channel_index < 0)
	{
		i1 = 0;
		i2 = get_channel_list_size() - 1;
	}

	int max, min;
	for (auto i = i1; i <= i2; i++)
	{
		CChanlistItem* p_channel = get_channel_list_item(i);
		p_channel->get_max_min(&max, &min);
		const auto y_extent = MulDiv(max - min + 1, 10, 8);
		p_channel->set_y_extent(y_extent);
	}
}

int ChartData::resize_channels(const int n_pixels, const long l_size)
{
	const auto old_n_pixels = m_n_pixels_;

	// trap dummy values and return if no operation is necessary
	if (n_pixels != 0) // horizontal resolution
		m_n_pixels_ = n_pixels;
	if (l_size != 0) // size of data to display
		m_lx_size_ = l_size;

	// change horizontal resolution	: m_Poly_points receives abscissa and ordinates
	// make provision for max and min points
	if (m_poly_points_.GetSize() != m_n_pixels_ * 4)
		m_poly_points_.SetSize(m_n_pixels_ * 4);

	// compute new scale and change size of Envelopes
	m_scale_.set_scale(m_n_pixels_, m_lx_size_);
	m_data_per_pixel_ = 2;

	// change Envelopes size
	auto n_points = m_n_pixels_; // set Envelope size to the nb of pixels
	if (m_n_pixels_ > m_lx_size_)
		n_points = static_cast<int>(m_lx_size_); // except when there is only one data pt
	ASSERT(n_points > 0);

	const auto n_envelopes = envelope_ptr_array_.GetSize(); // loop through all Envelopes
	if (n_envelopes > 0)
	{
		CEnvelope* p_envelope;
		for (auto i_envelope = 0; i_envelope < n_envelopes; i_envelope++)
		{
			p_envelope = envelope_ptr_array_.GetAt(i_envelope);
			p_envelope->set_envelope_size(n_points, m_data_per_pixel_);
		}
		p_envelope = envelope_ptr_array_.GetAt(0);
		p_envelope->fill_envelope_with_abscissa(m_n_pixels_, m_lx_size_); // store data series
	}

	// read data and update page and line sizes / file browse
	update_page_line_size();
	m_lx_last_ = m_lx_first_ + m_lx_size_ - 1;
	return old_n_pixels;
}

BOOL ChartData::attach_data_file(AcqDataDoc* p_data_file)
{
	m_p_data_file_ = p_data_file;
	m_sampling_rate_ = m_p_data_file_->get_wave_format()->sampling_rate_per_channel;
	m_p_data_file_->set_reading_buffer_dirty();
	ASSERT(m_p_data_file_->get_doc_channel_length() > 0);

	const long l_size = m_p_data_file_->get_doc_channel_length();

	// init parameters used to display Envelopes
	const int l_very_last = m_lx_very_last_;
	m_lx_very_last_ = m_p_data_file_->get_doc_channel_length() - 1;
	m_lx_first_ = 0;
	m_lx_last_ = l_size - 1;
	if (m_lx_size_ != l_size || l_very_last != m_lx_very_last_)
	{
		m_lx_size_ = l_size;
		m_data_per_pixel_ = m_scale_.set_scale(m_n_pixels_, m_lx_size_);
		m_data_per_pixel_ = 2;
	}

	//Remove irrelevant Chan_list items;
	const auto scan_count = m_p_data_file_->get_wave_format()->scan_count - 1;
	const auto channel_list_count = chan_list_item_ptr_array_.GetUpperBound();
	for (auto i = channel_list_count; i >= 0; i--)
	{
		if (get_channel_list_item(i)->get_source_chan() > scan_count)
			remove_channel_list_item(i);
	}

	// Remove irrelevant Envelopes();
	const auto n_envelopes = envelope_ptr_array_.GetUpperBound();
	for (auto i = n_envelopes; i > 0; i--) // ! Envelope(0)=abscissa
	{
		const auto p_envelope = envelope_ptr_array_.GetAt(i);
		if (p_envelope->get_source_chan() > scan_count)
		{
			delete p_envelope;
			envelope_ptr_array_.RemoveAt(i);
		}
	}

	// update chan list
	update_chan_list_from_doc();
	return TRUE;
}

BOOL ChartData::get_data_from_doc()
{
	// get document parameters: exit if empty document
	if (m_b_ad_buffers_ || m_p_data_file_ == nullptr)
		return FALSE;
	if (m_p_data_file_->get_doc_channel_length() <= 0)
		return FALSE;

	// check intervals	(assume m_lxSize OK)
	if (m_lx_first_ < 0)
		m_lx_first_ = 0; // avoid negative start
	m_lx_last_ = m_lx_first_ + m_lx_size_ - 1; // test end
	if (m_lx_last_ > m_lx_very_last_) // past end of file?
	{
		if (m_lx_size_ >= m_lx_very_last_ + 1)
			m_lx_size_ = m_lx_very_last_ + 1;
		m_lx_last_ = m_lx_very_last_; // clip to end
		m_lx_first_ = m_lx_last_ - m_lx_size_ + 1; // change start
	}

	// max nb of points spanning around raw data pt stored in array(0)
	if (envelope_ptr_array_.GetSize() < 1)
		return FALSE;
	auto p_cont = envelope_ptr_array_.GetAt(0);
	const auto n_span = p_cont->get_doc_buffer_span(); // additional pts necessary

	// loop through all pixels if data buffer is longer than data displayed
	// within one pixel...
	auto l_first = m_lx_first_; // start
	const auto scale_n_intervals = m_scale_.get_n_intervals(); // max pixel
	const auto scan_count = m_p_data_file_->get_scan_count();

	for (auto pixel_i = 0; pixel_i < scale_n_intervals; pixel_i++)
	{
		// compute file index of pts within current pixel
		const auto data_within_1_pixel = m_scale_.get_interval_size(pixel_i); // size first pixel
		const int l_last = l_first + data_within_1_pixel - 1;
		auto b_new = TRUE; // flag to tell routine that it should start from new data

		while (l_first <= l_last)
		{
			auto index_first_point_in_pixel = l_first; // index very first pt within pixel
			long index_last_point_in_pixel = l_last; // index very last pixel

			// ask document to read raw data, document returns index of data loaded within the buffer
			if (!m_p_data_file_->load_raw_data(&index_first_point_in_pixel, &index_last_point_in_pixel, n_span))
				break; // exit if error reported

			// build Envelopes  .................
			if (index_last_point_in_pixel > l_last)
				index_last_point_in_pixel = l_last;
			const int n_points = index_last_point_in_pixel - l_first + 1;
			if (n_points <= 0)
				break;

			// loop over each envelope
			for (auto i_envelope = envelope_ptr_array_.GetUpperBound(); i_envelope > 0; i_envelope--)
			{
				p_cont = envelope_ptr_array_.GetAt(i_envelope);

				const auto source_chan = p_cont->get_source_chan(); 
				const auto source_mode = p_cont->get_source_mode(); 
				if (source_mode > 0)
				{
					const auto lp_data = m_p_data_file_->load_transformed_data(l_first, index_last_point_in_pixel, source_mode, source_chan);
					p_cont->fill_envelope_with_max_min(pixel_i, lp_data, 1, n_points, b_new);
				}
				else 
				{
					const auto lp_data = m_p_data_file_->get_raw_data_element(source_chan, l_first);
					p_cont->fill_envelope_with_max_min(pixel_i, lp_data, scan_count, n_points, b_new);
				}
			}
			b_new = FALSE;
			l_first = index_last_point_in_pixel + 1;
		}
	}
	return TRUE;
}

BOOL ChartData::get_smooth_data_from_doc(const int i_option)
{
	// get document parameters: exit if empty document
	if (m_b_ad_buffers_ || m_p_data_file_ == nullptr)
		return FALSE;
	if (m_p_data_file_->get_doc_channel_length() <= 0)
		return FALSE;

	// check intervals	(assume m_lxSize OK)
	if (m_lx_first_ < 0)
		m_lx_first_ = 0; // avoid negative start
	m_lx_last_ = m_lx_first_ + m_lx_size_ - 1; // test end
	if (m_lx_last_ > m_lx_very_last_) // past end of file?
	{
		if (m_lx_size_ >= m_lx_very_last_ + 1)
			m_lx_size_ = m_lx_very_last_ + 1;
		m_lx_last_ = m_lx_very_last_; // clip to end
		m_lx_first_ = m_lx_last_ - m_lx_size_ + 1; // change start
	}
	const auto n_channels = m_p_data_file_->get_scan_count(); // n raw channels
	short* lp_data; // pointer used later
	// max nb of points spanning around raw data pt stored in array(0)
	auto p_cont = envelope_ptr_array_.GetAt(0);
	const auto doc_buffer_span = p_cont->get_doc_buffer_span(); // additional pts necessary

	// loop through all pixels if data buffer is longer than data displayed
	// within one pixel...
	auto l_first = m_lx_first_; // start
	const auto n_intervals = m_scale_.get_n_intervals(); // max pixel

	for (int pixel = 0; pixel < n_intervals; pixel++)
	{
		// compute file index of pts within current pixel
		const auto data_within_1_pixel = m_scale_.get_interval_size(pixel); // size first pixel
		const auto l_last = l_first + data_within_1_pixel - 1;
		auto b_new = TRUE; // flag to tell routine that it should start from new data

		while (l_first <= l_last)
		{
			auto l_buf_chan_first = l_first; // index very first pt within pixel
			auto l_buf_chan_last = l_last; // index very last pixel

			// ask document to read raw data, document returns index of data loaded within the buffer
			if (!m_p_data_file_->load_raw_data(&l_buf_chan_first, &l_buf_chan_last, doc_buffer_span))
				break; // exit if error reported

			// build Envelopes  .................
			if (l_buf_chan_last > l_last)
				l_buf_chan_last = l_last;
			const int n_points = l_buf_chan_last - l_first + 1;
			if (n_points <= 0)
				break;

			// loop over each envelope
			for (auto i_envelope = envelope_ptr_array_.GetUpperBound(); i_envelope > 0; i_envelope--)
			{
				p_cont = envelope_ptr_array_.GetAt(i_envelope);

				const auto source_chan = p_cont->get_source_chan();
				const auto source_mode = p_cont->get_source_mode();
				int intervals = n_channels;
				if (source_mode > 0) 
				{
					lp_data = m_p_data_file_->load_transformed_data(l_first, l_buf_chan_last, source_mode, source_chan);
					intervals = 1;
				}
				else 
					lp_data = m_p_data_file_->get_raw_data_element(source_chan, l_first);
				p_cont->fill_envelope_with_smooth_mx_mi(pixel, lp_data, intervals, n_points, b_new, i_option);
			}
			b_new = FALSE;
			l_first = l_buf_chan_last + 1;
		}
	}
	return TRUE;
}

BOOL ChartData::get_data_from_doc(const long l_first)
{
	if (l_first == m_lx_first_)
		return TRUE;
	m_lx_first_ = l_first;
	return get_data_from_doc();
}

BOOL ChartData::get_data_from_doc(long l_first, long l_last)
{
	if (l_first < 0) 
		l_first = 0;
	if (l_last > m_lx_very_last_) 
		l_last = m_lx_very_last_;
	if ((l_first > m_lx_very_last_) || (l_last < l_first))
	{
		// shuffled intervals
		l_first = 0;
		if (m_lx_size_ > m_lx_very_last_ + 1)
			m_lx_size_ = m_lx_very_last_ + 1;
		l_last = l_first + m_lx_size_ - 1;
	}

	if (m_lx_size_ != (l_last - l_first + 1))
	{
		m_lx_first_ = l_first;
		resize_channels(0, (l_last - l_first + 1));
	}
	// load data
	m_lx_first_ = l_first;
	m_lx_last_ = l_last;
	return get_data_from_doc();
}

BOOL ChartData::scroll_data_from_doc(const UINT n_sb_code)
{
	auto l_first = m_lx_first_;
	switch (n_sb_code)
	{
	case SB_LEFT: 
		l_first = 0;
		break;
	case SB_LINELEFT:
		l_first -= m_lx_line_;
		break;
	case SB_LINERIGHT: 
		l_first += m_lx_line_;
		break;
	case SB_PAGELEFT:
		l_first -= m_lx_page_;
		break;
	case SB_PAGERIGHT:
		l_first += m_lx_page_;
		break;
	case SB_RIGHT:
		l_first = m_lx_very_last_ - m_lx_size_ + 1;
		break;
	default:
		return FALSE;
	}
	m_lx_first_ = l_first;
	return get_data_from_doc();
}

void ChartData::update_page_line_size()
{
	if (m_p_data_file_ != nullptr)
		m_lx_page_ = m_lx_size_;
	else
		m_lx_page_ = m_lx_size_ / 10;
	if (m_lx_page_ == 0)
		m_lx_page_ = 1;
	m_lx_line_ = m_lx_page_ / m_n_pixels_;
	if (m_lx_line_ == 0)
		m_lx_line_ = 1;
}

void ChartData::zoom_data(CRect* previous_rect, CRect* new_rect)
{
	previous_rect->NormalizeRect();
	new_rect->NormalizeRect();

	// change gain & offset of all channels:
	for (auto i = chan_list_item_ptr_array_.GetUpperBound(); i >= 0; i--)
	{
		const auto chan_list_item = chan_list_item_ptr_array_[i];
		const auto extent = chan_list_item->get_y_extent();
		const auto new_ext = MulDiv(extent, new_rect->Height(), previous_rect->Height());
		chan_list_item->set_y_extent(new_ext);
		const auto zero = chan_list_item->get_y_zero();
		const auto new_zero = zero - (previous_rect->Height() - new_rect->Height()) / 2;
		chan_list_item->set_y_zero(new_zero);
	}

	// change index of first and last pt displayed
	auto l_size = m_lx_last_ - m_lx_first_ + 1;
	auto l_first = m_lx_first_ + (l_size * (new_rect->left - previous_rect->left)) / previous_rect->Width();
	l_size = (l_size * new_rect->Width()) / previous_rect->Width();
	if (l_size > m_lx_very_last_)
	{
		l_size = m_lx_very_last_;
		l_first = 0;
	}
	const auto l_last = l_first + l_size - 1;

	// call for new data only if indexes are different
	if (l_first != m_lx_first_ || l_last != m_lx_last_)
	{
		resize_channels(0, l_last - l_first + 1);
		get_data_from_doc(l_first, l_last);
	}
	send_my_message(HINT_VIEW_SIZE_CHANGED, NULL); // post?
}

void ChartData::update_x_ruler()
{
	if (x_ruler_bar == nullptr) 
		return;

	x_ruler.update_range(static_cast<double>(m_lx_first_) / static_cast<double>(m_sampling_rate_), static_cast<double>(m_lx_last_) / static_cast<double>(m_sampling_rate_));
	x_ruler_bar->Invalidate();
}

void ChartData::update_y_ruler()
{
	if (y_ruler_bar == nullptr)
		return;
	
	const auto bin_low = get_channel_list_y_pixels_to_bin(0, 0);
	const auto bin_high = get_channel_list_y_pixels_to_bin(0, client_rect_.Height());

	const CChanlistItem* p_channel = get_channel_list_item(0);
	const auto y_first = static_cast<double>(p_channel->convert_data_bins_to_volts(bin_low));
	const auto y_last = static_cast<double>(p_channel->convert_data_bins_to_volts(bin_high));
	
	y_ruler.update_range(y_first, y_last);
	y_ruler_bar->Invalidate();
}

void ChartData::plot_data_to_dc(CDC* p_dc)
{
	if (m_b_ad_buffers_)
		return;

	if (b_erase_background_)
		erase_background(p_dc);

	//if (b_nice_grid) 
	//{
		update_x_ruler();
		update_y_ruler();
	//}

	auto rect = display_rect_;
	rect.DeflateRect(1, 1);
	p_dc->SelectObject(&h_font);

	// exit if no data defined
	if (!is_defined() || m_p_data_file_ == nullptr)
	{
		const auto text_length = cs_empty_.GetLength();
		p_dc->DrawText(cs_empty_, text_length, rect, DT_TOP | DT_LEFT);
		return;
	}

	if (b_bottom_comment)
		display_text_bottom_left(p_dc, cs_bottom_comment, col_blue);

	// save DC
	const auto n_saved_dc = p_dc->SaveDC();
	ASSERT(n_saved_dc != 0);
	p_dc->IntersectClipRect(&display_rect_);

	// prepare DC
	p_dc->SetMapMode(MM_ANISOTROPIC);
	p_dc->SetViewportOrg(display_rect_.left, display_rect_.Height() / 2);
	p_dc->SetViewportExt(display_rect_.Width(), -display_rect_.Height());
	p_dc->SetWindowExt(display_rect_.Width(), display_rect_.Height());
	p_dc->SetWindowOrg(0, 0);

	const auto y_ve = display_rect_.Height();
	CEnvelope* p_x = nullptr;
	auto n_elements = 0;
	constexpr auto color = BLACK_COLOR;
	const auto old_pen = p_dc->SelectObject(&pen_table_[color]);

	// display loop:
	for (auto i_channel = chan_list_item_ptr_array_.GetUpperBound(); i_channel >= 0; i_channel--) 
	{
		const auto chan_list_item = chan_list_item_ptr_array_[i_channel];
		const auto w_ext = chan_list_item->get_y_extent();
		const auto w_org = chan_list_item->get_y_zero();
		p_dc->SelectObject(&pen_table_[chan_list_item->get_color_index()]);
		if (chan_list_item->get_pen_width() == 0)
			continue;

		if (p_x != chan_list_item->p_envelope_abscissa)
		{
			p_x = chan_list_item->p_envelope_abscissa;
			n_elements = p_x->get_envelope_size();
			if (m_poly_points_.GetSize() != n_elements * 2)
				m_poly_points_.SetSize(n_elements * 2);
			p_x->export_to_abscissa(m_poly_points_);
		}

		const auto p_y = chan_list_item->p_envelope_ordinates;
		p_y->export_to_ordinates(m_poly_points_);

		for (auto j = 0; j < n_elements; j++)
		{
			const auto p_point = &m_poly_points_[j];
			p_point->y = MulDiv(static_cast<short>(p_point->y) - w_org, y_ve, w_ext);
		}
		p_dc->MoveTo(m_poly_points_[0]);
		p_dc->Polyline(&m_poly_points_[0], n_elements);

		if (hz_tags.get_tag_list_size() > 0)
			display_hz_tags_for_channel(p_dc, i_channel, chan_list_item);

		highlight_data(p_dc, i_channel);
	}

	// restore DC
	p_dc->SelectObject(old_pen);
	p_dc->RestoreDC(n_saved_dc);

	if (vt_tags.get_tag_list_size() > 0)
		display_vt_tags_long_value(p_dc);

	// temp tag
	if (h_wnd_reflect_ != nullptr && temp_vertical_tag_ != nullptr)
	{
		const auto p_pen = p_dc->SelectObject(&black_dotted_pen_);
		const auto old_rop2 = p_dc->SetROP2(R2_NOTXORPEN);
		p_dc->MoveTo(temp_vertical_tag_->pixel, 2);
		p_dc->LineTo(temp_vertical_tag_->pixel, display_rect_.bottom - 2);
		p_dc->SetROP2(old_rop2);
		p_dc->SelectObject(p_pen);
	}
}

void ChartData::display_hz_tags_for_channel(CDC* p_dc, const int i_chan, const CChanlistItem* p_channel)
{
	const auto old_pen = p_dc->SelectObject(&black_dotted_pen_);
	const auto old_rop2 = p_dc->SetROP2(R2_NOTXORPEN);
	const auto w_ext = p_channel->get_y_extent();
	const auto w_org = p_channel->get_y_zero();
	const auto y_ve = display_rect_.Height();
	for (auto i = hz_tags.get_tag_list_size() - 1; i >= 0; i--)
	{
		if (hz_tags.get_channel(i) != i_chan)
			continue;
		auto k = hz_tags.get_value_int(i);
		k = MulDiv(static_cast<short>(k) - w_org, y_ve, w_ext);
		p_dc->MoveTo(display_rect_.left, k);
		p_dc->LineTo(display_rect_.right, k);
	}
	p_dc->SetROP2(old_rop2);
	p_dc->SelectObject(old_pen);
}

void ChartData::display_vt_tags_long_value(CDC* p_dc)
{
	const auto old_pen = p_dc->SelectObject(&black_dotted_pen_);
	const auto old_rop2 = p_dc->SetROP2(R2_NOTXORPEN);
	const int y1 = display_rect_.bottom;

	for (auto j = vt_tags.get_tag_list_size() - 1; j >= 0; j--)
	{
		constexpr auto y0 = 0;
		const auto lk = vt_tags.get_tag_value_long(j);
		if (lk < m_lx_first_ || lk > m_lx_last_)
			continue;
		const auto k = MulDiv(lk - m_lx_first_, display_rect_.Width(), m_lx_last_ - m_lx_first_ + 1);
		p_dc->MoveTo(k, y0);
		p_dc->LineTo(k, y1);
	}

	p_dc->SelectObject(old_pen);
	p_dc->SetROP2(old_rop2);
}

void ChartData::OnSize(UINT n_type, int cx, int cy)
{
	ChartWnd::OnSize(n_type, cx, cy);
	if (!is_defined() || m_p_data_file_ == nullptr)
		return;

	resize_channels(cx - 1, 0);
	if (!m_b_ad_buffers_)
	{
		get_data_from_doc();
	}
}

void ChartData::print(CDC* p_dc, const CRect* p_rect, const BOOL b_center_line)
{
	// save DC & old client rect
	const auto n_saved_dc = p_dc->SaveDC();
	ASSERT(n_saved_dc != 0);
	const auto old_rect = client_rect_;

	// prepare DC
	const auto old_map_mode = p_dc->SetMapMode(MM_TEXT); 
	client_rect_ = *p_rect; 
	adjust_display_rect(p_rect);
	erase_background(p_dc);
	// clip curves
	if (scope_structure_.b_clip_rect)
		p_dc->IntersectClipRect(display_rect_);
	else
		p_dc->SelectClipRgn(nullptr);

	// adjust coordinates for anisotropic mode
	const auto y_ve = -display_rect_.Height();
	const int y_vo = display_rect_.top + display_rect_.Height() / 2;
	const auto x_ve = display_rect_.Width();
	const int x_vo = display_rect_.left;

	// exit if no data defined
	if (!is_defined())
	{
		p_dc->TextOut(10, 10, _T("No data"));
		return;
	}

	// change horizontal resolution;
	resize_channels(display_rect_.Width(), m_lx_size_);
	if (!b_center_line)
		get_data_from_doc();
	else
		get_smooth_data_from_doc(b_center_line);

	const auto p_envelope = envelope_ptr_array_.GetAt(0);
	p_envelope->fill_envelope_with_abscissa_ex(x_vo, x_ve, m_lx_size_);

	// display all channels
	auto n_elements = 0;
	auto p_x = chan_list_item_ptr_array_[0]->p_envelope_abscissa;
	const BOOL b_poly_line = (p_dc->m_hAttribDC == nullptr) || (p_dc->GetDeviceCaps(LINECAPS) & LC_POLYLINE);
	auto color = BLACK_COLOR;
	const auto old_pen = p_dc->SelectObject(&pen_table_[color]);

	// display loop:
	for (auto i_chan = chan_list_item_ptr_array_.GetUpperBound(); i_chan >= 0; i_chan--) // scan all channels
	{
		const auto chan_list_item = chan_list_item_ptr_array_[i_chan];
		if (chan_list_item->is_print_visible() == FALSE)
			continue;

		// display: load abscissa   ----------------------------------------------
		if (p_x != chan_list_item->p_envelope_abscissa)
		{
			p_x = chan_list_item->p_envelope_abscissa; 
			p_x->export_to_abscissa(m_poly_points_);
			n_elements = p_x->get_envelope_size(); 
		}
		// display: load ordinates ---------------------------------------------
		const auto p_y = chan_list_item->p_envelope_ordinates; // load pointer to ordinates
		p_y->export_to_ordinates(m_poly_points_); 

		// change extent, org and color ----------------------------------------
		const auto y_extent = chan_list_item->get_y_extent();
		const auto y_zero = chan_list_item->get_y_zero();
		if (chan_list_item->get_color_index() != color)
		{
			color = chan_list_item->get_color_index();
			p_dc->SelectObject(&pen_table_[color]);
		}
		// transform ordinates ------------------------------------------------
		for (auto j = 0; j < n_elements; j++)
		{
			const auto p_point = &m_poly_points_[j];
			p_point->y = MulDiv(p_point->y - y_zero, y_ve, y_extent) + y_vo;
		}
		//  display points ----------------------------------------------------
		if (b_poly_line)
			p_dc->Polyline(&m_poly_points_[0], n_elements); 
		else
		{
			p_dc->MoveTo(m_poly_points_[0]); 
			for (auto j = 0; j < n_elements; j++)
				p_dc->LineTo(m_poly_points_[j]); 
		}

		//display associated cursors ------------------------------------------
		if (hz_tags.get_tag_list_size() > 0) 
		{
			// select pen and display mode
			CPen pen_light_grey(PS_SOLID, 0, color_spike_class[SILVER_COLOR]);
			const auto old_pen2 = p_dc->SelectObject(&pen_light_grey);
			// iterate through HZ cursor list
			const int x0 = p_rect->left;
			const int x1 = p_rect->right;
			for (auto j = hz_tags.get_tag_list_size() - 1; j >= 0; j--)
			{
				if (hz_tags.get_channel(j) != i_chan) // next tag if not associated with
					continue; // current channel
				auto k = hz_tags.get_value_int(j);
				k = MulDiv(k - y_zero, y_ve, y_extent) + y_vo;
				p_dc->MoveTo(x0, k); // set initial pt
				p_dc->LineTo(x1, k); // HZ line
			}
			p_dc->SelectObject(old_pen2);
		}
		// highlight data ------------------------------------------------------
		highlight_data(p_dc, i_chan);
	}

	// display vertical cursors ------------------------------------------------
	if (vt_tags.get_tag_list_size() > 0)
	{
		// select pen and display mode
		CPen pen_light_grey(PS_SOLID, 0, color_spike_class[SILVER_COLOR]);
		const auto p_old_pen = p_dc->SelectObject(&pen_light_grey);
		// iterate through VT cursor list
		const int y0 = p_rect->top;
		const int y1 = p_rect->bottom;
		const int k0 = p_rect->left;
		const int k_size = p_rect->right - k0;
		for (auto j = vt_tags.get_tag_list_size() - 1; j >= 0; j--)
		{
			const auto lk = vt_tags.get_tag_value_long(j); // get value
			if (lk < m_lx_first_ || lk > m_lx_last_)
				continue;
			const int k = k0 + (lk - m_lx_first_) * k_size / (m_lx_last_ - m_lx_first_ + 1);
			p_dc->MoveTo(k, y0); // set initial pt
			p_dc->LineTo(k, y1); // VT line
		}
		p_dc->SelectObject(p_old_pen);
	}

	// restore DC ----------------------------------------------------------------
	p_dc->SelectObject(old_pen); // restore old pen
	p_dc->RestoreDC(n_saved_dc); // restore DC
	p_dc->SetMapMode(old_map_mode); // restore map mode
	client_rect_ = old_rect;
	adjust_display_rect(&client_rect_);
}

BOOL ChartData::copy_as_text(const int i_option, const int i_unit, const int n_abscissa)
{
	// Clean clipboard of contents, and copy the text
	auto flag = FALSE;
	if (OpenClipboard())
	{
		BeginWaitCursor();
		const auto old_channels_size = resize_channels(n_abscissa, 0); 
		get_data_from_doc();
		EmptyClipboard(); 
		constexpr DWORD dw_len = 32768; // 32 Kb
		size_t characters_remaining = dw_len / sizeof(TCHAR);
		const auto h_copy = GlobalAlloc(GHND, dw_len);
		const auto wave_format = m_p_data_file_->get_wave_format();

		if (h_copy != nullptr)
		{
			// memory allocated -- get pointer to it
			auto lp_copy = static_cast<LPTSTR>(GlobalLock(h_copy));

			// data file name, comment, header
			const auto date = (wave_format->acquisition_time).Format(_T("%#d %B %Y %X"));
			StringCchPrintfEx(lp_copy, characters_remaining, &lp_copy, &characters_remaining, STRSAFE_NULL_ON_FAILURE,
			                  _T("%s\t%s\r\n"), static_cast<LPCTSTR>(m_p_data_file_->GetPathName()),
			                  static_cast<LPCTSTR>(date));
			StringCchPrintfEx(lp_copy, characters_remaining, &lp_copy, &characters_remaining, STRSAFE_NULL_ON_FAILURE,
			                  _T("%s\t\r\n"), static_cast<LPCTSTR>(wave_format->get_comments(_T("\t"), 0)));
			// time interval
			auto tt = static_cast<float>(get_data_first_index()) / wave_format->sampling_rate_per_channel; 
			StringCchPrintfEx(lp_copy, characters_remaining, &lp_copy, &characters_remaining, STRSAFE_NULL_ON_FAILURE,
			                  _T("time start(s):\t%f\r\n"), tt);
			tt = static_cast<float>(get_data_last_index()) / wave_format->sampling_rate_per_channel; 
			StringCchPrintfEx(lp_copy, characters_remaining, &lp_copy, &characters_remaining, STRSAFE_NULL_ON_FAILURE,
			                  _T("time end(s):\t%f"), tt);
			if (i_unit == 0)
			{
				// zero volt for document
				StringCchPrintfEx(lp_copy, characters_remaining, &lp_copy, &characters_remaining, STRSAFE_NULL_ON_FAILURE,
				                  _T("\r\nzero volts(bin):\t%i\r\n"), 0 );
				// unit for each channel
				StringCchPrintfEx(lp_copy, characters_remaining, &lp_copy, &characters_remaining, STRSAFE_NULL_ON_FAILURE,
				                  _T("mvolts per bin:\r\n"));
				for (auto i = 0; i < get_channel_list_size(); i++)
					StringCchPrintfEx(lp_copy, characters_remaining, &lp_copy, &characters_remaining, STRSAFE_NULL_ON_FAILURE,
					                  _T("%f\t"),
					                  static_cast<double>(get_channel_list_item(i)->get_volts_per_bin()) * 1000.);
				lp_copy--; // erase last tab
			}

			// comment for each channel
			StringCchPrintfEx(lp_copy, characters_remaining, &lp_copy, &characters_remaining, STRSAFE_NULL_ON_FAILURE,
			                  _T("\r\nchan title:\r\n"));
			for (auto i = 0; i < get_channel_list_size(); i++)
				StringCchPrintfEx(lp_copy, characters_remaining, &lp_copy, &characters_remaining, STRSAFE_NULL_ON_FAILURE,
				                  _T("%s\t"), static_cast<LPCTSTR>(get_channel_list_item(i)->get_comment()));
			lp_copy--; // erase last tab
			StringCchPrintfEx(lp_copy, characters_remaining, &lp_copy, &characters_remaining, STRSAFE_NULL_ON_FAILURE, _T("\r\n"));

			// data
			if (chan_list_item_ptr_array_.GetSize() < 1)
			{
				StringCchPrintfEx(lp_copy, characters_remaining, &lp_copy, &characters_remaining, STRSAFE_NULL_ON_FAILURE,
				                  _T("No data to display"));
			}
			else
			{
				switch (i_option)
				{
				case 0:
					get_ascii_envelope(lp_copy, i_unit);
					break;
				case 1:
					get_ascii_line(lp_copy, i_unit);
					break;
				default:
					get_ascii_envelope(lp_copy, i_unit);
					break;
				}
			}
			// un-alloc array
			GlobalUnlock(static_cast<HGLOBAL>(h_copy));
			flag = TRUE;
			SetClipboardData(CF_TEXT, h_copy);
		}
		CloseClipboard();
		resize_channels(old_channels_size, 0);
		get_data_from_doc();
		EndWaitCursor();
	}
	return flag;
}

LPTSTR ChartData::get_ascii_envelope(LPTSTR lp_copy, const int i_unit)
{
	// time intervals
	const auto i_channels = chan_list_item_ptr_array_.GetUpperBound();
	const auto n_points = envelope_ptr_array_.GetAt(0)->get_envelope_size();
	// loop through all points
	for (auto j = 0; j < n_points; j++)
	{
		// loop through all channels
		for (auto i = 0; i <= i_channels; i++) // scan all channels
		{
			const auto chan_list_item = chan_list_item_ptr_array_[i];
			const int k = (chan_list_item->p_envelope_ordinates)->get_point_at(j);
			if (i_unit == 1)
			{
				lp_copy += wsprintf(lp_copy, _T("%f\t"),
				                   static_cast<double>(k) * static_cast<double>(get_channel_list_item(i)->get_volts_per_bin()) * 1000.);
			}
			else
				lp_copy += wsprintf(lp_copy, _T("%i\t"), k);
		}
		lp_copy--;
		*lp_copy = 0xD; //CR
		*lp_copy++ = 0xA; //LF
	}
	*lp_copy = _T('\0');
	lp_copy++;
	return lp_copy;
}

LPTSTR ChartData::get_ascii_line(LPTSTR lp_copy, const int i_unit)
{
	// time intervals
	const auto i_channels = chan_list_item_ptr_array_.GetUpperBound();
	const auto n_points = envelope_ptr_array_.GetAt(0)->get_envelope_size();
	// loop through all points
	for (auto j = 0; j < n_points; j += m_data_per_pixel_)
	{
		// loop through all channels
		for (auto i = 0; i <= i_channels; i++) // scan all channels
		{
			const auto chan_list_item = chan_list_item_ptr_array_[i];
			int k = (chan_list_item->p_envelope_ordinates)->get_point_at(j);
			if (m_data_per_pixel_ > 1)
			{
				k += (chan_list_item->p_envelope_ordinates)->get_point_at(j + 1);
				k = k / 2;
			}
			if (i_unit == 1)
				lp_copy += wsprintf(lp_copy, _T("%f\t"),
				                   static_cast<double>(k) * static_cast<double>(get_channel_list_item(i)->get_volts_per_bin()) * 1000.);
			else
				lp_copy += wsprintf(lp_copy, _T("%i\t"), k);
		}
		lp_copy--;
		*lp_copy = 0xD; //CR
		*lp_copy++ = 0xA; //LF
	}
	*lp_copy = _T('\0');
	lp_copy++;
	return lp_copy;
}

// in order to move a curve vertically with the cursor, a special envelope is
// stored within the p_data array and displayed using XOR mode.
// this curve has 2 times fewer points (median) to speed up the display
// Although CClientDC is claimed as attached to the client area of the button
// ("this"), moving the curve along the vertical direction will draw outside
// the client area, suggesting that it is necessary to clip the client area...
// this code was also tested with m_dibSurf: XOR to DIB surface, then redraw the
// client area by fast BitBlt. This latter method was less efficient (slower)
// than XORing directly to the screen.

void ChartData::curve_xor()
{
	const auto p_dc = GetDC();
	const auto n_saved_dc = p_dc->SaveDC();
	CPen temp_pen;
	temp_pen.CreatePen(PS_SOLID, 0, color_spike_class[SILVER_COLOR]);
	const auto old_pen = p_dc->SelectObject(&temp_pen);
	p_dc->IntersectClipRect(&display_rect_);

	p_dc->SetMapMode(MM_ANISOTROPIC);
	p_dc->SetViewportOrg(display_rect_.left, y_vo_);
	p_dc->SetViewportExt(get_rect_width(), y_ve_);
	p_dc->SetWindowExt(m_xor_x_ext_, m_xor_y_ext_);
	p_dc->SetWindowOrg(0, 0);

	// display envelope store into p_data using XOR mode
	const auto old_rop2 = p_dc->SetROP2(R2_NOTXORPEN);
	p_dc->MoveTo(m_poly_points_[0]);
	p_dc->Polyline(&m_poly_points_[0], m_xor_n_elements_);
	p_dc->SetROP2(old_rop2);

	p_dc->SelectObject(old_pen); 
	p_dc->RestoreDC(n_saved_dc); 
	ReleaseDC(p_dc);
	temp_pen.DeleteObject();
}

void ChartData::OnLButtonDown(const UINT n_flags, const CPoint point)
{
	// convert chan values stored within HZ tags into pixels
	if (hz_tags.get_tag_list_size() > 0)
	{
		for (auto i_cur = 0; i_cur < hz_tags.get_tag_list_size(); i_cur++)
		{
			const auto pix_val = get_channel_list_bin_to_y_pixel(
				static_cast<WORD>(hz_tags.get_channel(i_cur)),
				hz_tags.get_value_int(i_cur));
			hz_tags.set_pixel(i_cur, pix_val);
		}
	}

	if (vt_tags.get_tag_list_size() > 0)
	{
		file_position_first_left_pixel_ = m_lx_first_;
		file_position_last_right_pixel_ = m_lx_last_;
	}

	// call base class to test for horizontal cursor or XORing rectangle
	ChartWnd::OnLButtonDown(n_flags, point);

	// if cursor mode = 0 and no tag hit detected, mouse mode=track rect
	// test curve hit -- specific to line_view, if hit curve, track curve instead
	if (current_cursor_mode_ == 0 && hc_trapped_ < 0) // test if cursor hits a curve
	{
		track_mode_ = TRACK_RECT;
		m_hit_curve_ = does_cursor_hit_curve(point);
		if (m_hit_curve_ >= 0)
		{
			// cancel track rect mode (cursor captured)
			track_mode_ = TRACK_CURVE; // flag: track curve

			// modify polypoint and prepare for XORing curve tracked with mouse
			const auto chan_list_item = chan_list_item_ptr_array_[m_hit_curve_];
			const auto p_x = chan_list_item->p_envelope_abscissa;
			p_x->get_mean_to_abscissa(m_poly_points_);
			m_xor_n_elements_ = p_x->get_envelope_size() / 2; 
			m_xor_x_ext_ = p_x->get_n_elements() / 2; 

			const auto p_y = chan_list_item->p_envelope_ordinates;
			p_y->get_mean_to_ordinates(m_poly_points_);
			m_xor_y_ext_ = chan_list_item->get_y_extent();
			m_zero_ = chan_list_item->get_y_zero();
			pt_first_ = point; 
			cur_track_ = m_zero_;

			curve_xor(); // xor curve
			send_my_message(HINT_HIT_CHANNEL, m_hit_curve_);  // post?
			return;
		}
	}

	// if horizontal cursor hit
	if (track_mode_ == TRACK_HZ_TAG)
	{
		const auto chan_list_item = chan_list_item_ptr_array_[hz_tags.get_channel(hc_trapped_)];
		y_we_ = chan_list_item->get_y_extent(); 
		y_wo_ = chan_list_item->get_y_zero(); 
	}
}

void ChartData::OnMouseMove(const UINT n_flags, const CPoint point)
{
	switch (track_mode_)
	{
	case TRACK_CURVE:
		curve_xor(); 
		m_zero_ = MulDiv(point.y - pt_first_.y, m_xor_y_ext_, -y_ve_) + cur_track_;
		curve_xor(); 
		break;

	default:
		ChartWnd::OnMouseMove(n_flags, point);
		break;
	}
}

void ChartData::OnLButtonUp(const UINT n_flags, CPoint point)
{
	
	switch (track_mode_)
	{
	case TRACK_CURVE:
		{
			curve_xor(); 
			const auto chan_list_item = chan_list_item_ptr_array_[m_hit_curve_];
			chan_list_item->set_y_zero(m_zero_);
			track_mode_ = TRACK_OFF;
			send_my_message(HINT_HIT_CHANNEL, m_hit_curve_); // post?
			Invalidate();
		}
		break;

	case TRACK_HZ_TAG:
		left_button_up_horizontal_tag(n_flags, point);
		break;

	case TRACK_VT_TAG:
		{
			const auto l_val = static_cast<long>(point.x)
				* (file_position_last_right_pixel_ - file_position_first_left_pixel_ + 1)
				/ static_cast<long>(display_rect_.right) + file_position_first_left_pixel_;
			point.x = static_cast<int>((l_val - file_position_first_left_pixel_)
				* static_cast<long>(display_rect_.right)
				/ (file_position_last_right_pixel_ - file_position_first_left_pixel_ + 1));

			Tag* p_tag = vt_tags.get_tag(hc_trapped_);
			p_tag->value_long = l_val;
			xor_vt_tag(point.x, p_tag->swap_pixel(point.x));

			send_my_message(HINT_CHANGE_VERT_TAG, hc_trapped_); // post?
			track_mode_ = TRACK_OFF;
		}
		break;

	case TRACK_RECT:
		{
			// skip too small a rectangle (5 pixels?)
			CRect rect_out(pt_first_.x, pt_first_.y, pt_last_.x, pt_last_.y);
			constexpr auto jitter = 3;
			const BOOL b_rect_ok = ((abs(rect_out.Height()) > jitter) || (abs(rect_out.Width()) > jitter));

			// perform action according to cursor type
			auto rect_in = display_rect_;
			switch (cursor_type_)
			{
			case 0: // if no cursor, no curve track, then move display
				if (b_rect_ok)
				{
					invert_tracker(point);
					rect_out = rect_in;
					rect_out.OffsetRect(pt_first_.x - pt_last_.x, pt_first_.y - pt_last_.y);
					zoom_data(&rect_in, &rect_out);
				}
				break;
			case CURSOR_ZOOM: // zoom operation
				if (b_rect_ok)
				{
					zoom_data(&rect_in, &rect_out);
					rect_zoom_from_ = rect_in;
					rect_zoom_to_ = rect_out;
					i_undo_zoom_ = 1;
				}
				else
					zoom_in();
				send_my_message(HINT_SET_MOUSE_CURSOR, old_cursor_type_); // post?
				break;
			case CURSOR_CROSS:
				send_my_message(HINT_DEFINED_RECT, NULL); // post?
				break;
			default:
				break;
			}
			track_mode_ = TRACK_OFF;
			Invalidate();
		}
		break;
	default:
		break;
	}
	release_cursor();
}

int ChartData::does_cursor_hit_curve(const CPoint point)
{
	auto channel_found = -1; // output value
	const auto i_channels = chan_list_item_ptr_array_.GetUpperBound();
	auto chan_list_item = chan_list_item_ptr_array_[0]->p_envelope_abscissa;
	const auto x_extent = chan_list_item->get_n_elements();
	int index1 = point.x - cx_mouse_jitter_;
	auto index2 = index1 + cx_mouse_jitter_;
	if (index1 < 0) index1 = 0; 
	if (index2 > (get_rect_width() - 1)) index2 = get_rect_width() - 1;

	// convert index1 into Envelope indexes
	index1 = index1 * m_data_per_pixel_; // start from
	index2 = (index2 + 1) * m_data_per_pixel_; // stop at
	// special case when less pt than pixels
	if (index1 == index2)
	{
		index1--;
		if (index1 < 0) index1 = 0;
		index2++;
		if (index2 > x_extent) index2 = x_extent;
	}
	// loop through all channels
	for (auto chan = 0; chan <= i_channels; chan++) // scan all channels
	{
		// convert device coordinates into value
		const auto i_val = get_channel_list_y_pixels_to_bin(chan, point.y);
		const auto i_jitter = MulDiv(cy_mouse_jitter_, get_channel_list_item(chan)->get_y_extent(), -y_ve_);
		const auto val_max = i_val + i_jitter; // mouse max
		const auto val_min = i_val - i_jitter; // mouse min
		chan_list_item = chan_list_item_ptr_array_[chan]->p_envelope_ordinates;

		// loop around horizontal jitter...
		for (auto index = index1; index < index2 && channel_found < 0; index++)
		{
			int k_max = chan_list_item->get_point_at(index); // get chan Envelope data point
			// special case: one point per pixel
			if (m_data_per_pixel_ == 1)
			{
				// more than min AND less than max
				if (k_max >= val_min && k_max <= val_max)
				{
					channel_found = chan;
					break;
				}
			}
			// second case: 2 pts per pixel - Envelope= max, min
			else
			{
				index++;
				int k_min = chan_list_item->get_point_at(index); // get min
				if (k_min > k_max) // ensure that k1=max
				{
					const auto k = k_max;
					k_max = k_min; 
					k_min = k;
				}
				// test if mouse interval crosses data interval!
				if (!(val_min > k_max || val_max < k_min))
				{
					channel_found = chan;
					break;
				}
			}
		}
	}
	return channel_found;
}

void ChartData::move_hz_tag_to_val(const int tag_index, const int val_int)
{
	const auto chan = hz_tags.get_channel(tag_index);
	const auto chan_list_item = chan_list_item_ptr_array_[chan];
	m_xor_y_ext_ = chan_list_item->get_y_extent();
	m_zero_ = chan_list_item->get_y_zero();
	//m_pt_last_.y = MulDiv(hz_tags.get_value_int(tag_index) - m_zero_, m_y_viewport_extent_, m_xor_y_ext_) + m_y_viewport_origin_;
	//CPoint point;
	//point.y = MulDiv(val_int - m_zero_, m_y_viewport_extent_, m_xor_y_ext_) + m_y_viewport_origin_;
	//hz_tags.set_value_int(tag_index, val_int);
	Tag* p_tag = hz_tags.get_tag(tag_index);
	p_tag->value_int = val_int;
	p_tag->pixel = MulDiv(val_int - m_zero_, y_ve_, m_xor_y_ext_) + y_vo_;
	xor_hz_tag(p_tag->pixel, p_tag->swap_pixel(p_tag->pixel));
	
}

void ChartData::set_highlight_data(const CHighLight& source)
{
	m_highlighted_ = source;
}

void ChartData::set_highlight_data(const CDWordArray* p_intervals)
{
	m_highlighted_.l_first.RemoveAll();
	m_highlighted_.l_last.RemoveAll();
	if (p_intervals == nullptr)
		return;

	m_highlighted_.channel = static_cast<int>(p_intervals->GetAt(0));
	m_highlighted_.color = p_intervals->GetAt(1);
	m_highlighted_.pen_size = static_cast<int>(p_intervals->GetAt(2));
	const auto size = (p_intervals->GetSize() - 3) / 2;
	m_highlighted_.l_first.SetSize(size);
	m_highlighted_.l_last.SetSize(size);

	for (auto i = 3; i < p_intervals->GetSize(); i += 2)
	{
		m_highlighted_.l_first.Add(static_cast<long>(p_intervals->GetAt(i)));
		m_highlighted_.l_last.Add(static_cast<long>(p_intervals->GetAt(i + 1)));
	}
}

void ChartData::highlight_data(CDC* p_dc, int chan)
{
	// skip if not the correct chan
	if (chan != m_highlighted_.channel || m_highlighted_.l_first.GetSize() < 2)
		return;

	// get color and pen size from array p_intervals
	CPen new_pen;
	new_pen.CreatePen(PS_SOLID, m_highlighted_.pen_size, m_highlighted_.color);
	const auto old_pen = p_dc->SelectObject(&new_pen);
	const BOOL b_poly_line = (p_dc->m_hAttribDC == nullptr)
		|| (p_dc->GetDeviceCaps(LINECAPS) & LC_POLYLINE);

	// loop to display data

	// pointer to descriptor
	for (auto i = 0; i < m_highlighted_.l_first.GetSize(); i++)
	{
		// load ith interval values
		auto l_first = m_highlighted_.l_first[i]; // first value
		auto l_last = m_highlighted_.l_last[i]; // last value

		if (l_last < m_lx_first_ || l_first > m_lx_last_)
			continue; // next if out of range

		// clip data if out of range
		if (l_first < m_lx_first_)
			l_first = m_lx_first_;
		if (l_last > m_lx_last_) 
			l_last = m_lx_last_;

		// compute corresponding interval (assume same m_scale for all channels... (!!)
		auto i_first = m_scale_.get_which_interval(l_first - m_lx_first_);
		if (i_first < 0)
			continue;
		auto i_last = m_scale_.get_which_interval(l_last - m_lx_first_) + 1;
		if (i_last < 0)
			continue;

		if (m_data_per_pixel_ != 1)
		{
			// envelope plotting
			i_first = i_first * m_data_per_pixel_;
			i_last = i_last * m_data_per_pixel_;
		}
		// display data
		const auto n_elements = i_last - i_first;
		if (b_poly_line)
			p_dc->Polyline(&m_poly_points_[i_first], n_elements);
		else
		{
			p_dc->MoveTo(m_poly_points_[i_first]);
			for (auto j = 0; j < n_elements; j++)
				p_dc->LineTo(m_poly_points_[i_first + j]);
		}
	}

	// restore previous pen
	p_dc->SelectObject(old_pen);
}

void ChartData::Serialize(CArchive& ar)
{
	ChartWnd::Serialize(ar);
	m_poly_points_.Serialize(ar);
	m_scale_.Serialize(ar);

	if (ar.IsStoring())
	{
		ar << m_data_per_pixel_; 
		ar << m_lx_very_last_;
		ar << m_lx_page_;
		ar << m_lx_line_; 
		ar << m_lx_size_; 
		ar << m_lx_first_; 
		ar << m_lx_last_;
		ar << m_n_pixels_; 

		const auto n_envelopes = envelope_ptr_array_.GetSize();
		ar << n_envelopes;
		const auto n_channels = chan_list_item_ptr_array_.GetSize();
		ar << n_channels;

		for (auto i = 0; i < n_envelopes; i++)
			envelope_ptr_array_[i]->Serialize(ar);

		for (auto i = 0; i < n_channels; i++)
			chan_list_item_ptr_array_[i]->Serialize(ar);
	}
	else
	{
		ar >> m_data_per_pixel_; 
		ar >> m_lx_very_last_; 
		ar >> m_lx_page_; 
		ar >> m_lx_line_; 
		ar >> m_lx_size_; 
		ar >> m_lx_first_; 
		ar >> m_lx_last_; 
		ar >> m_n_pixels_; 

		int n_envelopes;
		ar >> n_envelopes;
		int n_chan_list_items;
		ar >> n_chan_list_items;

		// CEnvelope array
		if (envelope_ptr_array_.GetSize() > n_envelopes)
		{
			for (auto i = envelope_ptr_array_.GetUpperBound(); i >= n_envelopes; i--)
				delete envelope_ptr_array_[i];
			envelope_ptr_array_.SetSize(n_envelopes);
		}
		else if (envelope_ptr_array_.GetSize() < n_envelopes)
		{
			const auto n_envelope_0 = envelope_ptr_array_.GetSize();
			envelope_ptr_array_.SetSize(n_envelopes);
			for (auto i = n_envelope_0; i < n_envelopes; i++)
				envelope_ptr_array_[i] = new CEnvelope; // (CEnvelope*)
		}
		for (auto i = 0; i < n_envelopes; i++)
			envelope_ptr_array_[i]->Serialize(ar);

		// ChanList array
		if (chan_list_item_ptr_array_.GetSize() > n_chan_list_items)
		{
			for (auto i = chan_list_item_ptr_array_.GetUpperBound(); i >= n_chan_list_items; i--)
				delete chan_list_item_ptr_array_[i];
			chan_list_item_ptr_array_.SetSize(n_chan_list_items);
		}
		else if (chan_list_item_ptr_array_.GetSize() < n_chan_list_items)
		{
			const auto n_chan_list_items_0 = chan_list_item_ptr_array_.GetSize();
			chan_list_item_ptr_array_.SetSize(n_chan_list_items);
			for (auto i = n_chan_list_items_0; i < n_chan_list_items; i++)
			{
				chan_list_item_ptr_array_[i] = new CChanlistItem;
			}
		}
		auto ix = 0;
		auto iy = 0;
		for (auto i = 0; i < n_chan_list_items; i++)
		{
			chan_list_item_ptr_array_[i]->Serialize(ar);
			chan_list_item_ptr_array_[i]->get_envelope_array_indexes(ix, iy);
			chan_list_item_ptr_array_[i]->set_envelope_arrays(envelope_ptr_array_.GetAt(ix), ix, envelope_ptr_array_.GetAt(iy),
			                                             iy);
		}
	}
}

void ChartData::set_track_spike(const BOOL b_track_spike, const int track_len, const int track_offset, const int track_channel)
{
	m_b_track_spike_ = b_track_spike;
	m_track_len_ = track_len;
	m_track_offset_ = track_offset;
	m_track_channel_ = track_channel;
}

void ChartData::adjust_gain(const boolean b_set_span_mv, const float span_mv_value) const
{
	const auto channels_list_size = get_channel_list_size();
	auto j = channels_list_size - 1;
	int max, min;
	for (auto i = j; i >= 0; i--)
	{
		CChanlistItem* chan = get_channel_list_item(i);
		chan->get_max_min(&max, &min);
		auto span = max - min + 1;
		int extent;
		int zero;
		if (b_set_span_mv)
		{
			span = chan->convert_volts_to_data_bins(span_mv_value / 1000.f);
			zero = chan->get_data_bin_zero();
			extent = span;
		}
		else
		{
			extent = MulDiv(span, 11 * channels_list_size, 10);
			zero = (max + min) / 2 - MulDiv(extent, j, channels_list_size * 2);
		}
		j -= 2;
		chan->set_y_extent(extent);
		chan->set_y_zero(zero);
	}
}

void ChartData::load_data_within_window(const boolean set_time_span, const float t_first, const float t_last)
{
	const auto n_pixels = get_rect_width();
	long l_first = 0;
	long l_last = m_p_data_file_->get_doc_channel_length() - 1;
	if (set_time_span)
	{
		l_first = static_cast<long>(t_first * m_sampling_rate_);
		l_last = static_cast<long>(t_last * m_sampling_rate_);
		if (l_last == l_first)
			l_last++;
	}
	resize_channels(n_pixels, l_last - l_first + 1);
	get_data_from_doc(l_first, l_last);
}

void ChartData::load_all_channels(int data_transform)
{
	const int n_document_channels = m_p_data_file_->get_wave_format()->scan_count;
	auto n_channels_to_plot = get_channel_list_size();

	// add channels if value is zero
	// channels were all removed if file was not found in an earlier round
	for (auto channel = 0; channel < n_document_channels; channel++)
	{
		// check if present in the list
		auto b_present = FALSE;
		for (auto j = n_channels_to_plot - 1; j >= 0; j--) // check all channels / display list
		{
			// test if this data chan is present + no transformation
			if (get_channel_list_item(j)->get_source_chan() == channel)
			{
				b_present = TRUE; // the wanted chan is present: stop looping through display list
				set_channel_list_transform_mode(j, data_transform);
				break; // and examine next doc channel
			}
		}
		if (b_present == FALSE)
		{
			add_channel_list_item(channel, data_transform);
			n_channels_to_plot++;
		}
		get_channel_list_item(channel)->set_color(static_cast<WORD>(channel));
	}
}
