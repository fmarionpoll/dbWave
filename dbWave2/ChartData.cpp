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
	resize_channels(m_npixels, 1024);
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
	for (auto i = envelope_ptr_array.GetUpperBound(); i >= 0; i--)
		delete envelope_ptr_array[i];
	envelope_ptr_array.RemoveAll();

	for (auto i = chanlistitem_ptr_array.GetUpperBound(); i >= 0; i--)
		delete chanlistitem_ptr_array[i];
	chanlistitem_ptr_array.RemoveAll();
}

int ChartData::add_channel_list_item(int ns, int mode)
{
	// first time??	create Envelope(0) with abscissa series
	if (chanlistitem_ptr_array.GetSize() == 0)
	{
		m_PolyPoints.SetSize(m_npixels * 4);
		m_scale.SetScale(m_npixels, m_lxSize);
		m_dataperpixel = 2;
		auto* p_envelope = new CEnvelope(static_cast<WORD>(m_npixels * m_dataperpixel), m_dataperpixel, 0, -1, 0);
		ASSERT(p_envelope != NULL);
		envelope_ptr_array.Add(p_envelope);
		p_envelope->FillEnvelopeWithAbcissa(m_npixels * m_dataperpixel, m_lxSize);
	}

	// create new Envelope and store pointer into Envelopes_list
	auto span = 0;
	if (m_pDataFile != nullptr)
		span = m_pDataFile->get_transformed_data_span(mode);
	auto* p_envelope_y = new CEnvelope(static_cast<WORD>(m_npixels), m_dataperpixel, ns, mode, span);
	ASSERT(p_envelope_y != NULL);
	const auto j = envelope_ptr_array.Add(p_envelope_y);

	// create new chan_list_item with x=Envelope(0) and y=the new Envelope
	auto* p_chan_list_item = new CChanlistItem(envelope_ptr_array.GetAt(0), 0, p_envelope_y, j);
	ASSERT(p_chan_list_item != NULL);
	const auto index_new_chan = chanlistitem_ptr_array.Add(p_chan_list_item);

	// init display parameters
	p_chan_list_item->InitDisplayParms(1, RGB(0, 0, 0), 2048, 4096);

	if (m_pDataFile != nullptr)
	{
		float volts_per_bin;
		m_pDataFile->get_volts_per_bin(ns, &volts_per_bin, mode);
		const auto wave_chan_array = m_pDataFile->get_wavechan_array();
		const auto wave_format = m_pDataFile->get_waveformat();
		p_chan_list_item->SetDataBinFormat(wave_format->bin_zero, wave_format->bin_span);
		p_chan_list_item->SetDataVoltsFormat(volts_per_bin, wave_format->full_scale_volts);
		if (ns >= wave_chan_array->chan_array_get_size())
			ns = 0;
		const auto p_channel = wave_chan_array->get_p_channel(ns);
		p_chan_list_item->dl_comment = p_channel->am_csComment; 
		update_chan_list_max_span(); 
		if (mode > 0) 
			p_chan_list_item->dl_comment = 
			(m_pDataFile->get_transform_name(mode)).Left(8) + ": " + p_chan_list_item->dl_comment;
	}
	return index_new_chan;
}

int ChartData::remove_channel_list_item(int i_chan)
{
	const auto chan_list_size = chanlistitem_ptr_array.GetSize(); 
	if (chan_list_size > 0) 
	{
		const auto pa = chanlistitem_ptr_array[i_chan]->pEnvelopeOrdinates;
		// step 1: check that this envelope is not used by another channel
		auto b_used_only_once = TRUE;
		for (auto lj = chan_list_size; lj >= 0; lj--)
		{
			const auto p_envelope_y = chanlistitem_ptr_array[i_chan]->pEnvelopeOrdinates;
			if (pa == p_envelope_y && lj != i_chan)
			{
				b_used_only_once = FALSE; 
				break; 
			}
		}
		// step 2: delete corresponding envelope only if envelope used only once.
		if (b_used_only_once)
		{
			for (auto k = envelope_ptr_array.GetUpperBound(); k >= 0; k--)
			{
				const auto pb = envelope_ptr_array[k];
				if (pa == pb) 
				{
					delete pa; 
					envelope_ptr_array.RemoveAt(k);
					break; 
				}
			}
		}
		// step 3: delete channel
		delete chanlistitem_ptr_array[i_chan];
		chanlistitem_ptr_array.RemoveAt(i_chan);
	}
	update_chan_list_max_span();
	return chan_list_size - 1;
}

void ChartData::update_chan_list_max_span()
{
	if (envelope_ptr_array.GetSize() <= 1)
		return;

	auto imax = 0;
	for (auto i = envelope_ptr_array.GetUpperBound(); i > 0; i--)
	{
		const auto j = envelope_ptr_array[i]->GetDocbufferSpan();
		if (j > imax)
			imax = j;
	}
	envelope_ptr_array[0]->SetDocbufferSpan(imax);
}

void ChartData::update_chan_list_from_doc()
{
	for (auto i = chanlistitem_ptr_array.GetUpperBound(); i >= 0; i--)
	{
		const auto chan_list_item = chanlistitem_ptr_array[i];
		const auto p_ord = chan_list_item->pEnvelopeOrdinates;
		const auto ns = p_ord->GetSourceChan();
		const auto mode = p_ord->GetSourceMode();
		p_ord->SetDocbufferSpan(m_pDataFile->get_transformed_data_span(mode));
		const auto wave_chan_array = m_pDataFile->get_wavechan_array();
		const auto p_chan = wave_chan_array->get_p_channel(ns);
		chan_list_item->dl_comment = p_chan->am_csComment;
		if (mode > 0)
			chan_list_item->dl_comment = 
			 (m_pDataFile->get_transform_name(mode)).Left(6) + ": " + chan_list_item->dl_comment;
		update_gain_settings(i);
	}
	update_chan_list_max_span(); 
}

void ChartData::update_gain_settings(const int i_chan)
{
	CChanlistItem* p_chan = chanlistitem_ptr_array[i_chan];
	const auto p_ord = p_chan->pEnvelopeOrdinates;
	const auto ns = p_ord->GetSourceChan();
	const auto mode = p_ord->GetSourceMode();
	float doc_volts_per_bin;
	m_pDataFile->get_volts_per_bin(ns, &doc_volts_per_bin, mode);
	const auto volts_per_data_bin = p_chan->GetVoltsperDataBin();
	const auto wave_format = m_pDataFile->get_waveformat();
	if (doc_volts_per_bin != volts_per_data_bin)
	{
		p_chan->SetDataBinFormat(wave_format->bin_zero, wave_format->bin_span);
		p_chan->SetDataVoltsFormat(doc_volts_per_bin, wave_format->full_scale_volts);
		auto i_extent = p_chan->GetYextent();
		i_extent = static_cast<int>(static_cast<float>(i_extent) / doc_volts_per_bin * volts_per_data_bin);
		p_chan->SetYextent(i_extent);
	}
}

int ChartData::set_channel_list_source_channel(const int i_channel, const int acq_channel)
{
	// check if channel is allowed
	const auto wave_format = m_pDataFile->get_waveformat();
	if (wave_format->scan_count <= acq_channel || acq_channel < 0)
		return -1;

	// make sure we have enough data channels...
	if (get_channel_list_size() <= i_channel)
		for (auto j = get_channel_list_size(); j <= i_channel; j++)
			add_channel_list_item(j, 0);

	// change channel
	const auto chan_list_item = chanlistitem_ptr_array[i_channel];
	const auto p_ord = chan_list_item->pEnvelopeOrdinates;
	p_ord->SetSourceChan(acq_channel);
	const auto mode = p_ord->GetSourceMode();
	// modify comment
	const auto wave_chan_array = m_pDataFile->get_wavechan_array();
	const auto p_channel = wave_chan_array->get_p_channel(acq_channel);
	chan_list_item->dl_comment = p_channel->am_csComment;
	if (mode > 0)
		chan_list_item->dl_comment = (m_pDataFile->get_transform_name(mode)).Left(6) + _T(": ") + chan_list_item->
			dl_comment;
	update_gain_settings(i_channel);
	return acq_channel;
}

void ChartData::set_channel_list_y(const int i_chan, int acq_chan, const int transform)
{
	// change channel
	const auto chan_list_item = chanlistitem_ptr_array[i_chan];
	chan_list_item->SetOrdinatesSourceData(acq_chan, transform);
	// modify comment
	const auto p_channelArray = m_pDataFile->get_wavechan_array();
	if (acq_chan >= p_channelArray->chan_array_get_size())
		acq_chan = 0;
	const auto p_channel = p_channelArray->get_p_channel(acq_chan);
	chan_list_item->dl_comment = p_channel->am_csComment;
	if (transform > 0)
		chan_list_item->dl_comment = (m_pDataFile->get_transform_name(transform)).Left(6) + _T(": ") + chan_list_item->
			dl_comment;
}

void ChartData::set_channel_list_volts_extent(const int i_chan, const float* p_value)
{
	auto i_chan_first = i_chan;
	auto i_chan_last = i_chan;
	if (i_chan < 0)
	{
		i_chan_first = 0;
		i_chan_last = chanlistitem_ptr_array.GetUpperBound();
	}
	auto volts_extent = 0.f;
	if (p_value != nullptr)
		volts_extent = *p_value;
	for (auto i = i_chan_first; i <= i_chan_last; i++)
	{
		const auto chan_list_item = chanlistitem_ptr_array[i];
		const auto y_volts_per_bin = chan_list_item->GetVoltsperDataBin();
		if (p_value == nullptr)
			volts_extent = y_volts_per_bin * static_cast<float>(chan_list_item->GetYextent());

		const auto y_extent = static_cast<int>(volts_extent / y_volts_per_bin);
		chan_list_item->SetYextent(y_extent);
	}
}

void ChartData::set_channel_list_volts_zero(const int i_chan, const float* p_value)
{
	auto i_chan_first = i_chan;
	auto i_chan_last = i_chan;
	if (i_chan < 0)
	{
		i_chan_first = 0;
		i_chan_last = chanlistitem_ptr_array.GetUpperBound();
	}

	auto volts_extent = 0.f;
	if (p_value != nullptr)
		volts_extent = *p_value;
	for (auto i = i_chan_first; i <= i_chan_last; i++)
	{
		const auto chan_list_item = chanlistitem_ptr_array[i];
		const auto y_volts_per_bin = chan_list_item->GetVoltsperDataBin();
		if (p_value == nullptr)
			volts_extent = y_volts_per_bin * static_cast<float>(chan_list_item->GetDataBinZero());

		const auto i_y_zero = static_cast<int>(volts_extent / y_volts_per_bin);
		chan_list_item->SetYzero(i_y_zero);
	}
}

int ChartData::set_channel_list_transform_mode(const int i_chan, const int i_mode)
{
	// check if transform is allowed
	if (!m_pDataFile->is_wb_transform_allowed(i_mode) || // ? is transform allowed
		!m_pDataFile->init_wb_transform_buffer()) // ? is init OK
	{
		AfxMessageBox(IDS_LNVERR02, MB_OK);
		return -1;
	}

	// change transform mode
	const auto chan_list_item = chanlistitem_ptr_array[i_chan];
	const auto p_ord = chan_list_item->pEnvelopeOrdinates;
	const auto ns = p_ord->GetSourceChan();
	p_ord->SetSourceMode(i_mode, m_pDataFile->get_transformed_data_span(i_mode));

	// modify comment
	const auto wave_chan_array = m_pDataFile->get_wavechan_array();
	const auto p_channel = wave_chan_array->get_p_channel(ns);
	chan_list_item->dl_comment = p_channel->am_csComment;
	if (i_mode > 0)
		chan_list_item->dl_comment = (m_pDataFile->get_transform_name(i_mode)).Left(8)
			+ _T(": ") + chan_list_item->dl_comment;
	update_gain_settings(i_chan);
	update_chan_list_max_span();
	return i_mode;
}

SCOPESTRUCT* ChartData::get_scope_parameters()
{
	const auto n_channels = chanlistitem_ptr_array.GetSize();
	scope_structure_.channels.SetSize(n_channels);
	for (auto i = 0; i < n_channels; i++)
	{
		const auto chan_list_item = chanlistitem_ptr_array[i];
		scope_structure_.channels[i].izero = chan_list_item->GetYzero();
		scope_structure_.channels[i].iextent = chan_list_item->GetYextent();
	}

	return &scope_structure_;
}

void ChartData::set_scope_parameters(SCOPESTRUCT* pStruct)
{
	const auto n_channels_struct = pStruct->channels.GetSize();
	const auto n_channels_chan_list = chanlistitem_ptr_array.GetSize();
	for (auto i = 0; i < n_channels_struct; i++)
	{
		if (i >= n_channels_chan_list)
			break;
		const auto chan_list_item = chanlistitem_ptr_array[i];
		chan_list_item->SetYzero(pStruct->channels[i].izero);
		chan_list_item->SetYextent(pStruct->channels[i].iextent);
	}
}

void ChartData::AutoZoomChan(int j) const
{
	auto i1 = j;
	auto i2 = j;
	if (j < 0)
	{
		i1 = 0;
		i2 = get_channel_list_size() - 1;
	}

	int max, min;
	for (auto i = i1; i <= i2; i++)
	{
		CChanlistItem* chan = get_channel_list_item(i);
		chan->GetMaxMin(&max, &min);
		chan->SetYzero((max + min) / 2);
		chan->SetYextent(MulDiv(max - min + 1, 10, 8));
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
		chan->GetMaxMin(&max, &min);
		const auto amplitude_span = MulDiv((max - min), 12 * n_channels, 10);
		chan->SetYextent(amplitude_span);
		const auto offset = (max + min) / 2 + MulDiv(amplitude_span, i_cur, n_channels * 2);
		chan->SetYzero(offset);
	}
}

void ChartData::center_chan(int j) const
{
	auto i1 = j;
	auto i2 = j;
	if (j < 0)
	{
		i1 = 0;
		i2 = get_channel_list_size() - 1;
	}

	int max, min;
	for (auto i = i1; i <= i2; i++)
	{
		CChanlistItem* chan = get_channel_list_item(i);
		chan->GetMaxMin(&max, &min);
		const auto y_zero = (max + min) / 2;
		chan->SetYzero(y_zero);
	}
}

void ChartData::max_gain_chan(const int j) const
{
	auto i1 = j;
	auto i2 = j;
	if (j < 0)
	{
		i1 = 0;
		i2 = get_channel_list_size() - 1;
	}

	int max, min;
	for (auto i = i1; i <= i2; i++)
	{
		CChanlistItem* p_channel = get_channel_list_item(i);
		p_channel->GetMaxMin(&max, &min);
		const auto y_extent = MulDiv(max - min + 1, 10, 8);
		p_channel->SetYextent(y_extent);
	}
}

int ChartData::resize_channels(const int n_pixels, const long l_size)
{
	const auto old_n_pixels = m_npixels;

	// trap dummy values and return if no operation is necessary
	if (n_pixels != 0) // horizontal resolution
		m_npixels = n_pixels;
	if (l_size != 0) // size of data to display
		m_lxSize = l_size;

	// change horizontal resolution	: m_Poly_points receives abscissa and ordinates
	// make provision for max and min points
	if (m_PolyPoints.GetSize() != m_npixels * 4)
		m_PolyPoints.SetSize(m_npixels * 4);

	// compute new scale and change size of Envelopes
	m_scale.SetScale(m_npixels, m_lxSize);
	m_dataperpixel = 2;

	// change Envelopes size
	auto n_points = m_npixels; // set Envelope size to the nb of pixels
	if (m_npixels > m_lxSize)
		n_points = static_cast<int>(m_lxSize); // except when there is only one data pt
	ASSERT(n_points > 0);

	const auto n_envelopes = envelope_ptr_array.GetSize(); // loop through all Envelopes
	if (n_envelopes > 0)
	{
		CEnvelope* p_envelope;
		for (auto i_envelope = 0; i_envelope < n_envelopes; i_envelope++)
		{
			p_envelope = envelope_ptr_array.GetAt(i_envelope);
			p_envelope->SetEnvelopeSize(n_points, m_dataperpixel);
		}
		p_envelope = envelope_ptr_array.GetAt(0);
		p_envelope->FillEnvelopeWithAbcissa(m_npixels, m_lxSize); // store data series
	}

	// read data and update page and line sizes / file browse
	update_page_line_size();
	m_lxLast = m_lxFirst + m_lxSize - 1;
	return old_n_pixels;
}

BOOL ChartData::AttachDataFile(AcqDataDoc* p_data_file)
{
	m_pDataFile = p_data_file;
	m_samplingrate = m_pDataFile->get_waveformat()->sampling_rate_per_channel;
	m_pDataFile->set_reading_buffer_dirty();
	ASSERT(m_pDataFile->get_doc_channel_length() > 0);

	const long l_size = m_pDataFile->get_doc_channel_length();

	// init parameters used to display Envelopes
	const int l_very_last = m_lxVeryLast;
	m_lxVeryLast = m_pDataFile->get_doc_channel_length() - 1;
	m_lxFirst = 0;
	m_lxLast = l_size - 1;
	if (m_lxSize != l_size || l_very_last != m_lxVeryLast)
	{
		m_lxSize = l_size;
		m_dataperpixel = m_scale.SetScale(m_npixels, m_lxSize);
		m_dataperpixel = 2;
	}

	//Remove irrelevant Chan_list items;
	const auto scan_count = m_pDataFile->get_waveformat()->scan_count - 1;
	const auto channel_list_count = chanlistitem_ptr_array.GetUpperBound();
	for (auto i = channel_list_count; i >= 0; i--)
	{
		if (get_channel_list_item(i)->GetSourceChan() > scan_count)
			remove_channel_list_item(i);
	}

	// Remove irrelevant Envelopes();
	const auto n_envelopes = envelope_ptr_array.GetUpperBound();
	for (auto i = n_envelopes; i > 0; i--) // ! Envelope(0)=abscissa
	{
		const auto p_envelope = envelope_ptr_array.GetAt(i);
		if (p_envelope->GetSourceChan() > scan_count)
		{
			delete p_envelope;
			envelope_ptr_array.RemoveAt(i);
		}
	}

	// update chan list
	update_chan_list_from_doc();
	return TRUE;
}

BOOL ChartData::get_data_from_doc()
{
	// get document parameters: exit if empty document
	if (m_bADbuffers || m_pDataFile == nullptr)
		return FALSE;
	if (m_pDataFile->get_doc_channel_length() <= 0)
		return FALSE;

	// check intervals	(assume m_lxSize OK)
	if (m_lxFirst < 0)
		m_lxFirst = 0; // avoid negative start
	m_lxLast = m_lxFirst + m_lxSize - 1; // test end
	if (m_lxLast > m_lxVeryLast) // past end of file?
	{
		if (m_lxSize >= m_lxVeryLast + 1)
			m_lxSize = m_lxVeryLast + 1;
		m_lxLast = m_lxVeryLast; // clip to end
		m_lxFirst = m_lxLast - m_lxSize + 1; // change start
	}

	// max nb of points spanning around raw data pt stored in array(0)
	if (envelope_ptr_array.GetSize() < 1)
		return FALSE;
	auto p_cont = envelope_ptr_array.GetAt(0);
	const auto n_span = p_cont->GetDocbufferSpan(); // additional pts necessary

	// loop through all pixels if data buffer is longer than data displayed
	// within one pixel...
	auto l_first = m_lxFirst; // start
	const auto scale_n_intervals = m_scale.GetnIntervals(); // max pixel
	const auto scan_count = m_pDataFile->get_scan_count();

	for (auto pixel_i = 0; pixel_i < scale_n_intervals; pixel_i++)
	{
		// compute file index of pts within current pixel
		const auto data_within_1_pixel = m_scale.GetIntervalSize(pixel_i); // size first pixel
		const int l_last = l_first + data_within_1_pixel - 1;
		auto b_new = TRUE; // flag to tell routine that it should start from new data

		while (l_first <= l_last)
		{
			auto index_first_point_in_pixel = l_first; // index very first pt within pixel
			long index_last_point_in_pixel = l_last; // index very last pixel

			// ask document to read raw data, document returns index of data loaded within the buffer
			if (!m_pDataFile->load_raw_data(&index_first_point_in_pixel, &index_last_point_in_pixel, n_span))
				break; // exit if error reported

			// build Envelopes  .................
			if (index_last_point_in_pixel > l_last)
				index_last_point_in_pixel = l_last;
			const int n_points = index_last_point_in_pixel - l_first + 1;
			if (n_points <= 0)
				break;

			// loop over each envelope
			for (auto i_envelope = envelope_ptr_array.GetUpperBound(); i_envelope > 0; i_envelope--)
			{
				p_cont = envelope_ptr_array.GetAt(i_envelope);

				const auto source_chan = p_cont->GetSourceChan(); 
				const auto source_mode = p_cont->GetSourceMode(); 
				if (source_mode > 0)
				{
					const auto lp_data = m_pDataFile->load_transformed_data(l_first, index_last_point_in_pixel, source_mode, source_chan);
					p_cont->FillEnvelopeWithMxMi(pixel_i, lp_data, 1, n_points, b_new);
				}
				else 
				{
					const auto lp_data = m_pDataFile->get_raw_data_element(source_chan, l_first);
					p_cont->FillEnvelopeWithMxMi(pixel_i, lp_data, scan_count, n_points, b_new);
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
	if (m_bADbuffers || m_pDataFile == nullptr)
		return FALSE;
	if (m_pDataFile->get_doc_channel_length() <= 0)
		return FALSE;

	// check intervals	(assume m_lxSize OK)
	if (m_lxFirst < 0)
		m_lxFirst = 0; // avoid negative start
	m_lxLast = m_lxFirst + m_lxSize - 1; // test end
	if (m_lxLast > m_lxVeryLast) // past end of file?
	{
		if (m_lxSize >= m_lxVeryLast + 1)
			m_lxSize = m_lxVeryLast + 1;
		m_lxLast = m_lxVeryLast; // clip to end
		m_lxFirst = m_lxLast - m_lxSize + 1; // change start
	}
	const auto n_channels = m_pDataFile->get_scan_count(); // n raw channels
	short* lp_data; // pointer used later
	// max nb of points spanning around raw data pt stored in array(0)
	auto p_cont = envelope_ptr_array.GetAt(0);
	const auto doc_buffer_span = p_cont->GetDocbufferSpan(); // additional pts necessary

	// loop through all pixels if data buffer is longer than data displayed
	// within one pixel...
	auto l_first = m_lxFirst; // start
	const auto n_intervals = m_scale.GetnIntervals(); // max pixel

	for (int pixel = 0; pixel < n_intervals; pixel++)
	{
		// compute file index of pts within current pixel
		const auto data_within_1_pixel = m_scale.GetIntervalSize(pixel); // size first pixel
		const auto l_last = l_first + data_within_1_pixel - 1;
		auto b_new = TRUE; // flag to tell routine that it should start from new data

		while (l_first <= l_last)
		{
			auto l_buf_chan_first = l_first; // index very first pt within pixel
			auto l_buf_chan_last = l_last; // index very last pixel

			// ask document to read raw data, document returns index of data loaded within the buffer
			if (!m_pDataFile->load_raw_data(&l_buf_chan_first, &l_buf_chan_last, doc_buffer_span))
				break; // exit if error reported

			// build Envelopes  .................
			if (l_buf_chan_last > l_last)
				l_buf_chan_last = l_last;
			const int n_points = l_buf_chan_last - l_first + 1;
			if (n_points <= 0)
				break;

			// loop over each envelope
			for (auto i_envelope = envelope_ptr_array.GetUpperBound(); i_envelope > 0; i_envelope--)
			{
				p_cont = envelope_ptr_array.GetAt(i_envelope);

				const auto source_chan = p_cont->GetSourceChan();
				const auto source_mode = p_cont->GetSourceMode();
				int intervals = n_channels;
				if (source_mode > 0) 
				{
					lp_data = m_pDataFile->load_transformed_data(l_first, l_buf_chan_last, source_mode, source_chan);
					intervals = 1;
				}
				else 
					lp_data = m_pDataFile->get_raw_data_element(source_chan, l_first);
				p_cont->FillEnvelopeWithSmoothMxMi(pixel, lp_data, intervals, n_points, b_new, i_option);
			}
			b_new = FALSE;
			l_first = l_buf_chan_last + 1;
		}
	}
	return TRUE;
}

BOOL ChartData::get_data_from_doc(const long l_first)
{
	if (l_first == m_lxFirst)
		return TRUE;
	m_lxFirst = l_first;
	return get_data_from_doc();
}

BOOL ChartData::get_data_from_doc(long l_first, long l_last)
{
	if (l_first < 0) 
		l_first = 0;
	if (l_last > m_lxVeryLast) 
		l_last = m_lxVeryLast;
	if ((l_first > m_lxVeryLast) || (l_last < l_first))
	{
		// shuffled intervals
		l_first = 0;
		if (m_lxSize > m_lxVeryLast + 1)
			m_lxSize = m_lxVeryLast + 1;
		l_last = l_first + m_lxSize - 1;
	}

	if (m_lxSize != (l_last - l_first + 1))
	{
		m_lxFirst = l_first;
		resize_channels(0, (l_last - l_first + 1));
	}
	// load data
	m_lxFirst = l_first;
	m_lxLast = l_last;
	return get_data_from_doc();
}

BOOL ChartData::scroll_data_from_doc(const WORD n_sb_code)
{
	auto l_first = m_lxFirst;
	switch (n_sb_code)
	{
	case SB_LEFT: 
		l_first = 0;
		break;
	case SB_LINELEFT:
		l_first -= m_lxLine;
		break;
	case SB_LINERIGHT: 
		l_first += m_lxLine;
		break;
	case SB_PAGELEFT:
		l_first -= m_lxPage;
		break;
	case SB_PAGERIGHT:
		l_first += m_lxPage;
		break;
	case SB_RIGHT:
		l_first = m_lxVeryLast - m_lxSize + 1;
		break;
	default:
		return FALSE;
	}
	m_lxFirst = l_first;
	return get_data_from_doc();
}

void ChartData::update_page_line_size()
{
	if (m_pDataFile != nullptr)
		m_lxPage = m_lxSize;
	else
		m_lxPage = m_lxSize / 10;
	if (m_lxPage == 0)
		m_lxPage = 1;
	m_lxLine = m_lxPage / m_npixels;
	if (m_lxLine == 0)
		m_lxLine = 1;
}

void ChartData::zoom_data(CRect* previous_rect, CRect* new_rect)
{
	previous_rect->NormalizeRect();
	new_rect->NormalizeRect();

	// change gain & offset of all channels:
	for (auto i = chanlistitem_ptr_array.GetUpperBound(); i >= 0; i--)
	{
		const auto chan_list_item = chanlistitem_ptr_array[i];
		const auto extent = chan_list_item->GetYextent();
		const auto new_ext = MulDiv(extent, new_rect->Height(), previous_rect->Height());
		chan_list_item->SetYextent(new_ext);
		const auto zero = chan_list_item->GetYzero();
		const auto new_zero = zero - (previous_rect->Height() - new_rect->Height()) / 2;
		chan_list_item->SetYzero(new_zero);
	}

	// change index of first and last pt displayed
	auto l_size = m_lxLast - m_lxFirst + 1;
	auto l_first = m_lxFirst + (l_size * (new_rect->left - previous_rect->left)) / previous_rect->Width();
	l_size = (l_size * new_rect->Width()) / previous_rect->Width();
	if (l_size > m_lxVeryLast)
	{
		l_size = m_lxVeryLast;
		l_first = 0;
	}
	const auto l_last = l_first + l_size - 1;

	// call for new data only if indexes are different
	if (l_first != m_lxFirst || l_last != m_lxLast)
	{
		resize_channels(0, l_last - l_first + 1);
		get_data_from_doc(l_first, l_last);
	}
	post_my_message(HINT_VIEWSIZECHANGED, NULL);
}

void ChartData::update_x_ruler()
{
	if (x_ruler_bar == nullptr) return;

	x_ruler.UpdateRange(static_cast<double>(m_lxFirst) / static_cast<double>(m_samplingrate), static_cast<double>(m_lxLast) / static_cast<double>(m_samplingrate));
	x_ruler_bar->Invalidate();
}

void ChartData::update_y_ruler()
{
	if (y_ruler_bar == nullptr)
		return;
	
	const auto bin_low = get_channel_list_y_pixels_to_bin(0, 0);
	const auto bin_high = get_channel_list_y_pixels_to_bin(0, m_client_rect_.Height());

	const CChanlistItem* p_channel = get_channel_list_item(0);
	const auto y_first = static_cast<double>(p_channel->ConvertDataBinsToVolts(bin_low));
	const auto y_last = static_cast<double>(p_channel->ConvertDataBinsToVolts(bin_high));
	
	y_ruler.UpdateRange(y_first, y_last);
	y_ruler_bar->Invalidate();
}

void ChartData::plot_data_to_dc(CDC* p_dc)
{
	if (m_bADbuffers)
		return;

	if (b_erase_background_)
		erase_background(p_dc);

	if (b_nice_grid) 
	{
		update_x_ruler();
		update_y_ruler();
	}

	auto rect = m_display_rect_;
	rect.DeflateRect(1, 1);
	p_dc->SelectObject(&h_font);

	// exit if no data defined
	if (!IsDefined() || m_pDataFile == nullptr)
	{
		const auto text_length = cs_empty_.GetLength();
		p_dc->DrawText(cs_empty_, text_length, rect, DT_LEFT);
		return;
	}

	// plot comment at the bottom
	if (b_bottom_comment)
	{
		const auto text_length = cs_bottom_comment.GetLength();
		p_dc->SetTextColor(RGB(0, 0, 255)); // BLUE
		p_dc->DrawText(cs_bottom_comment, text_length, rect, DT_RIGHT | DT_BOTTOM | DT_SINGLELINE);
	}

	// save DC
	const auto n_saved_dc = p_dc->SaveDC();
	ASSERT(n_saved_dc != 0);
	p_dc->IntersectClipRect(&m_display_rect_);

	// prepare DC
	p_dc->SetMapMode(MM_ANISOTROPIC);
	p_dc->SetViewportOrg(m_display_rect_.left, m_display_rect_.Height() / 2);
	p_dc->SetViewportExt(m_display_rect_.Width(), -m_display_rect_.Height());
	p_dc->SetWindowExt(m_display_rect_.Width(), m_display_rect_.Height());

	p_dc->SetWindowOrg(0, 0);

	const auto y_ve = m_display_rect_.Height();
	CEnvelope* p_x = nullptr;
	auto n_elements = 0;

	constexpr auto color = BLACK_COLOR;
	const auto old_pen = p_dc->SelectObject(&pen_table_[color]);

	// display loop:
	for (auto i_channel = chanlistitem_ptr_array.GetUpperBound(); i_channel >= 0; i_channel--) // scan all channels
	{
		const auto chan_list_item = chanlistitem_ptr_array[i_channel];
		const auto w_ext = chan_list_item->GetYextent();
		const auto w_org = chan_list_item->GetYzero();
		p_dc->SelectObject(&pen_table_[chan_list_item->GetColorIndex()]);
		if (chan_list_item->GetPenWidth() == 0)
			continue;

		if (p_x != chan_list_item->pEnvelopeAbcissa)
		{
			p_x = chan_list_item->pEnvelopeAbcissa;
			n_elements = p_x->GetEnvelopeSize();
			if (m_PolyPoints.GetSize() != n_elements * 2)
				m_PolyPoints.SetSize(n_elements * 2);
			p_x->ExportToAbcissa(m_PolyPoints);
		}

		const auto pY = chan_list_item->pEnvelopeOrdinates;
		pY->ExportToOrdinates(m_PolyPoints);

		for (auto j = 0; j < n_elements; j++)
		{
			const auto p_point = &m_PolyPoints[j];
			p_point->y = MulDiv(static_cast<short>(p_point->y) - w_org, y_ve, w_ext);
		}
		p_dc->MoveTo(m_PolyPoints[0]);
		p_dc->Polyline(&m_PolyPoints[0], n_elements);

		if (horizontal_tags.get_tag_list_size() > 0)
			display_hz_tags_for_channel(p_dc, i_channel, chan_list_item);

		highlight_data(p_dc, i_channel);
	}

	// restore DC
	p_dc->SelectObject(old_pen);
	p_dc->RestoreDC(n_saved_dc);

	if (vertical_tags.get_tag_list_size() > 0)
		display_vt_tags_long_value(p_dc);

	// temp tag
	if (m_hwnd_reflect_ != nullptr && m_temp_vertical_tag_ != nullptr)
	{
		const auto p_pen = p_dc->SelectObject(&black_dotted_pen_);
		const auto old_rop2 = p_dc->SetROP2(R2_NOTXORPEN);
		p_dc->MoveTo(m_temp_vertical_tag_->m_pixel, 2);
		p_dc->LineTo(m_temp_vertical_tag_->m_pixel, m_display_rect_.bottom - 2);
		p_dc->SetROP2(old_rop2);
		p_dc->SelectObject(p_pen);
	}
}

void ChartData::display_hz_tags_for_channel(CDC* p_dc, const int i_chan, const CChanlistItem* p_channel)
{
	const auto old_pen = p_dc->SelectObject(&black_dotted_pen_);
	const auto old_rop2 = p_dc->SetROP2(R2_NOTXORPEN);
	const auto w_ext = p_channel->GetYextent();
	const auto w_org = p_channel->GetYzero();
	const auto y_ve = m_display_rect_.Height();
	for (auto i = horizontal_tags.get_tag_list_size() - 1; i >= 0; i--)
	{
		if (horizontal_tags.get_channel(i) != i_chan)
			continue;
		auto k = horizontal_tags.get_value(i);
		k = MulDiv(static_cast<short>(k) - w_org, y_ve, w_ext);
		p_dc->MoveTo(m_display_rect_.left, k);
		p_dc->LineTo(m_display_rect_.right, k);
	}
	p_dc->SetROP2(old_rop2);
	p_dc->SelectObject(old_pen);
}

void ChartData::display_vt_tags_long_value(CDC* p_dc)
{
	const auto old_pen = p_dc->SelectObject(&black_dotted_pen_);
	const auto old_rop2 = p_dc->SetROP2(R2_NOTXORPEN);
	const int y1 = m_display_rect_.bottom;

	for (auto j = vertical_tags.get_tag_list_size() - 1; j >= 0; j--)
	{
		constexpr auto y0 = 0;
		const auto lk = vertical_tags.get_tag_l_val(j);
		if (lk < m_lxFirst || lk > m_lxLast)
			continue;
		const auto k = MulDiv(lk - m_lxFirst, m_display_rect_.Width(), m_lxLast - m_lxFirst + 1);
		p_dc->MoveTo(k, y0);
		p_dc->LineTo(k, y1);
	}

	p_dc->SelectObject(old_pen);
	p_dc->SetROP2(old_rop2);
}

void ChartData::OnSize(UINT nType, int cx, int cy)
{
	ChartWnd::OnSize(nType, cx, cy);
	if (!IsDefined() || m_pDataFile == nullptr)
		return;

	resize_channels(cx - 1, 0);
	if (!m_bADbuffers)
	{
		get_data_from_doc();
	}
}

void ChartData::Print(CDC* p_dc, CRect* pRect, BOOL bCenterLine)
{
	// save DC & old client rect
	const auto n_saved_dc = p_dc->SaveDC();
	ASSERT(n_saved_dc != 0);
	const auto old_rect = m_client_rect_;

	// prepare DC
	const auto old_map_mode = p_dc->SetMapMode(MM_TEXT); // change map mode to text (1 pixel = 1 logical point)
	m_client_rect_ = *pRect; //CRect(0,0, pRect->GetRectWidth(), pRect->GetRectHeight());
	adjust_display_rect(pRect);
	erase_background(p_dc);
	// clip curves
	if (scope_structure_.bClipRect)
		p_dc->IntersectClipRect(m_display_rect_);
	else
		p_dc->SelectClipRgn(nullptr);

	// adjust coordinates for anisotropic mode
	const auto y_ve = -m_display_rect_.Height();
	const int y_vo = m_display_rect_.top + m_display_rect_.Height() / 2;
	const auto x_ve = m_display_rect_.Width();
	const int x_vo = m_display_rect_.left;

	// exit if no data defined
	if (!IsDefined())
	{
		p_dc->TextOut(10, 10, _T("No data"));
		return;
	}

	// change horizontal resolution;
	resize_channels(m_display_rect_.Width(), m_lxSize);
	if (!bCenterLine)
		get_data_from_doc();
	else
		get_smooth_data_from_doc(bCenterLine);

	const auto p_envelope = envelope_ptr_array.GetAt(0);
	p_envelope->FillEnvelopeWithAbcissaEx(x_vo, x_ve, m_lxSize);

	// display all channels
	auto n_elements = 0;
	auto p_x = chanlistitem_ptr_array[0]->pEnvelopeAbcissa;
	const BOOL b_poly_line = (p_dc->m_hAttribDC == nullptr) || (p_dc->GetDeviceCaps(LINECAPS) & LC_POLYLINE);
	auto color = BLACK_COLOR;
	const auto old_pen = p_dc->SelectObject(&pen_table_[color]);

	// display loop:
	for (auto i_chan = chanlistitem_ptr_array.GetUpperBound(); i_chan >= 0; i_chan--) // scan all channels
	{
		const auto chan_list_item = chanlistitem_ptr_array[i_chan];
		if (chan_list_item->GetflagPrintVisible() == FALSE)
			continue;

		// display: load abscissa   ----------------------------------------------
		if (p_x != chan_list_item->pEnvelopeAbcissa)
		{
			p_x = chan_list_item->pEnvelopeAbcissa; // load pointer to abscissa
			p_x->ExportToAbcissa(m_PolyPoints); // copy abscissa to polypts buffer
			n_elements = p_x->GetEnvelopeSize(); // update nb of elements
		}
		// display: load ordinates ---------------------------------------------
		const auto p_y = chan_list_item->pEnvelopeOrdinates; // load pointer to ordinates
		p_y->ExportToOrdinates(m_PolyPoints); // copy ordinates to polypts buffer

		// change extent, org and color ----------------------------------------
		const auto y_extent = chan_list_item->GetYextent();
		const auto y_zero = chan_list_item->GetYzero();
		if (chan_list_item->GetColorIndex() != color)
		{
			color = chan_list_item->GetColorIndex();
			p_dc->SelectObject(&pen_table_[color]);
		}
		// transform ordinates ------------------------------------------------
		for (auto j = 0; j < n_elements; j++)
		{
			const auto p_point = &m_PolyPoints[j];
			p_point->y = MulDiv(p_point->y - y_zero, y_ve, y_extent) + y_vo;
		}
		//  display points ----------------------------------------------------
		if (b_poly_line)
			p_dc->Polyline(&m_PolyPoints[0], n_elements); // draw curve
		else
		{
			p_dc->MoveTo(m_PolyPoints[0]); // move pen to first pair of coords
			for (auto j = 0; j < n_elements; j++)
				p_dc->LineTo(m_PolyPoints[j]); // draw lines
		}

		//display associated cursors ------------------------------------------
		if (horizontal_tags.get_tag_list_size() > 0) // print HZ cursors if any?
		{
			// select pen and display mode
			CPen pen_light_grey(PS_SOLID, 0, color_table_[SILVER_COLOR]);
			const auto old_pen2 = p_dc->SelectObject(&pen_light_grey);
			// iterate through HZ cursor list
			const int x0 = pRect->left;
			const int x1 = pRect->right;
			for (auto j = horizontal_tags.get_tag_list_size() - 1; j >= 0; j--)
			{
				if (horizontal_tags.get_channel(j) != i_chan) // next tag if not associated with
					continue; // current channel
				auto k = horizontal_tags.get_value(j);
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
	if (vertical_tags.get_tag_list_size() > 0)
	{
		// select pen and display mode
		CPen pen_light_grey(PS_SOLID, 0, color_table_[SILVER_COLOR]);
		const auto p_old_pen = p_dc->SelectObject(&pen_light_grey);
		// iterate through VT cursor list
		const int y0 = pRect->top;
		const int y1 = pRect->bottom;
		const int k0 = pRect->left;
		const int k_size = pRect->right - k0;
		for (auto j = vertical_tags.get_tag_list_size() - 1; j >= 0; j--)
		{
			const auto lk = vertical_tags.get_tag_l_val(j); // get value
			if (lk < m_lxFirst || lk > m_lxLast)
				continue;
			const int k = k0 + (lk - m_lxFirst) * k_size / (m_lxLast - m_lxFirst + 1);
			p_dc->MoveTo(k, y0); // set initial pt
			p_dc->LineTo(k, y1); // VT line
		}
		p_dc->SelectObject(p_old_pen);
	}

	// restore DC ----------------------------------------------------------------
	p_dc->SelectObject(old_pen); // restore old pen
	p_dc->RestoreDC(n_saved_dc); // restore DC
	p_dc->SetMapMode(old_map_mode); // restore map mode
	m_client_rect_ = old_rect;
	adjust_display_rect(&m_client_rect_);
}

BOOL ChartData::copy_as_text(const int i_option, const int i_unit, const int n_abcissa)
{
	// Clean clipboard of contents, and copy the text
	auto flag = FALSE;
	if (OpenClipboard())
	{
		BeginWaitCursor();
		const auto old_channels_size = resize_channels(n_abcissa, 0); 
		get_data_from_doc();
		EmptyClipboard(); 
		constexpr DWORD dw_len = 32768; // 32 Kb
		size_t characters_remaining = dw_len / sizeof(TCHAR);
		const auto h_copy = GlobalAlloc(GHND, dw_len);
		const auto wave_format = m_pDataFile->get_waveformat();

		if (h_copy != nullptr)
		{
			// memory allocated -- get pointer to it
			auto lp_copy = static_cast<LPTSTR>(GlobalLock(h_copy));

			// data file name, comment, header
			const auto date = (wave_format->acquisition_time).Format(_T("%#d %B %Y %X"));
			StringCchPrintfEx(lp_copy, characters_remaining, &lp_copy, &characters_remaining, STRSAFE_NULL_ON_FAILURE,
			                  _T("%s\t%s\r\n"), static_cast<LPCTSTR>(m_pDataFile->GetPathName()),
			                  static_cast<LPCTSTR>(date));
			StringCchPrintfEx(lp_copy, characters_remaining, &lp_copy, &characters_remaining, STRSAFE_NULL_ON_FAILURE,
			                  _T("%s\t\r\n"), static_cast<LPCTSTR>(wave_format->get_comments(_T("\t"), 0)));
			// time interval
			auto tt = static_cast<float>(GetDataFirstIndex()) / wave_format->sampling_rate_per_channel; 
			StringCchPrintfEx(lp_copy, characters_remaining, &lp_copy, &characters_remaining, STRSAFE_NULL_ON_FAILURE,
			                  _T("time start(s):\t%f\r\n"), tt);
			tt = static_cast<float>(GetDataLastIndex()) / wave_format->sampling_rate_per_channel; 
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
					                  static_cast<double>(get_channel_list_item(i)->GetVoltsperDataBin()) * 1000.);
				lp_copy--; // erase last tab
			}

			// comment for each channel
			StringCchPrintfEx(lp_copy, characters_remaining, &lp_copy, &characters_remaining, STRSAFE_NULL_ON_FAILURE,
			                  _T("\r\nchan title:\r\n"));
			for (auto i = 0; i < get_channel_list_size(); i++)
				StringCchPrintfEx(lp_copy, characters_remaining, &lp_copy, &characters_remaining, STRSAFE_NULL_ON_FAILURE,
				                  _T("%s\t"), static_cast<LPCTSTR>(get_channel_list_item(i)->GetComment()));
			lp_copy--; // erase last tab
			StringCchPrintfEx(lp_copy, characters_remaining, &lp_copy, &characters_remaining, STRSAFE_NULL_ON_FAILURE, _T("\r\n"));

			// data
			if (chanlistitem_ptr_array.GetSize() < 1)
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
	const auto i_channels = chanlistitem_ptr_array.GetUpperBound();
	const auto n_points = envelope_ptr_array.GetAt(0)->GetEnvelopeSize();
	// loop through all points
	for (auto j = 0; j < n_points; j++)
	{
		// loop through all channels
		for (auto i = 0; i <= i_channels; i++) // scan all channels
		{
			const auto chan_list_item = chanlistitem_ptr_array[i];
			const int k = (chan_list_item->pEnvelopeOrdinates)->GetPointAt(j);
			if (i_unit == 1)
			{
				lp_copy += wsprintf(lp_copy, _T("%f\t"),
				                   static_cast<double>(k) * static_cast<double>(get_channel_list_item(i)->GetVoltsperDataBin()) * 1000.);
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
	const auto i_channels = chanlistitem_ptr_array.GetUpperBound();
	const auto n_points = envelope_ptr_array.GetAt(0)->GetEnvelopeSize();
	// loop through all points
	for (auto j = 0; j < n_points; j += m_dataperpixel)
	{
		// loop through all channels
		for (auto i = 0; i <= i_channels; i++) // scan all channels
		{
			const auto chan_list_item = chanlistitem_ptr_array[i];
			int k = (chan_list_item->pEnvelopeOrdinates)->GetPointAt(j);
			if (m_dataperpixel > 1)
			{
				k += (chan_list_item->pEnvelopeOrdinates)->GetPointAt(j + 1);
				k = k / 2;
			}
			if (i_unit == 1)
				lp_copy += wsprintf(lp_copy, _T("%f\t"),
				                   static_cast<double>(k) * static_cast<double>(get_channel_list_item(i)->GetVoltsperDataBin()) * 1000.);
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
// this curve has 2 times less points (median) to speed up the display
// Although CClientDC is claimed as attached to the client area of the button
// ("this"), moving the curve along the vertical direction will draw outside of
// the client area, suggesting that it is necessary to clip the client area...
// this code was also tested with m_dibSurf: XOR to DIB surface, then redraw the
// client area by fast BitBlt. This latter method was less efficient (slower)
// than XORing directly to the screen.

void ChartData::curve_xor()
{
	const auto p_dc = GetDC();
	const auto n_saved_dc = p_dc->SaveDC();
	CPen temp_pen;
	temp_pen.CreatePen(PS_SOLID, 0, color_table_[SILVER_COLOR]);
	const auto old_pen = p_dc->SelectObject(&temp_pen);
	p_dc->IntersectClipRect(&m_display_rect_);

	p_dc->SetMapMode(MM_ANISOTROPIC);
	p_dc->SetViewportOrg(m_display_rect_.left, m_y_vo_);
	p_dc->SetViewportExt(get_rect_width(), m_y_ve_);
	p_dc->SetWindowExt(m_XORxext, m_XORyext);
	p_dc->SetWindowOrg(0, 0);

	// display envelope store into p_data using XOR mode
	const auto old_rop2 = p_dc->SetROP2(R2_NOTXORPEN);
	p_dc->MoveTo(m_PolyPoints[0]);
	p_dc->Polyline(&m_PolyPoints[0], m_XORnelmts);
	p_dc->SetROP2(old_rop2);

	p_dc->SelectObject(old_pen); 
	p_dc->RestoreDC(n_saved_dc); 
	ReleaseDC(p_dc);
	temp_pen.DeleteObject();
}

void ChartData::OnLButtonDown(UINT nFlags, CPoint point)
{
	// convert chan values stored within HZ tags into pixels
	if (horizontal_tags.get_tag_list_size() > 0)
	{
		for (auto icur = 0; icur < horizontal_tags.get_tag_list_size(); icur++)
		{
			const auto pixval = get_channel_list_bin_to_y_pixel(
				WORD(horizontal_tags.get_channel(icur)),
				horizontal_tags.get_value(icur));
			horizontal_tags.set_tag_pixel(icur, pixval);
		}
	}

	if (vertical_tags.get_tag_list_size() > 0)
	{
		file_position_first_left_pixel_ = m_lxFirst;
		file_position_last_right_pixel_ = m_lxLast;
	}

	// call base class to test for horiz cursor or XORing rectangle
	ChartWnd::OnLButtonDown(nFlags, point);

	// if cursor mode = 0 and no tag hit detected, mouse mode=track rect
	// test curve hit -- specific to lineview, if hit curve, track curve instead
	if (current_cursor_mode_ == 0 && hc_trapped_ < 0) // test if cursor hits a curve
	{
		track_mode_ = TRACK_RECT;
		m_hitcurve = does_cursor_hit_curve(point);
		if (m_hitcurve >= 0)
		{
			// cancel track rect mode (cursor captured)
			track_mode_ = TRACK_CURVE; // flag: track curve

			// modify polypoint and prepare for XORing curve tracked with mouse
			const auto chan_list_item = chanlistitem_ptr_array[m_hitcurve];
			const auto p_x = chan_list_item->pEnvelopeAbcissa; // display: load abscissa
			p_x->GetMeanToAbcissa(m_PolyPoints);
			m_XORnelmts = p_x->GetEnvelopeSize() / 2; // nb of elements
			m_XORxext = p_x->GetnElements() / 2; // extent

			const auto p_y = chan_list_item->pEnvelopeOrdinates; // load ordinates
			p_y->GetMeanToOrdinates(m_PolyPoints);
			m_XORyext = chan_list_item->GetYextent(); // store extent
			m_zero = chan_list_item->GetYzero(); // store zero
			m_pt_first_ = point; // save first point
			m_cur_track_ = m_zero; // use m_curTrack to store zero

			curve_xor(); // xor curve
			post_my_message(HINT_HITCHANNEL, m_hitcurve); // tell parent chan selected
			return;
		}
	}

	// if horizontal cursor hit -- specific .. deal with variable gain
	if (track_mode_ == TRACK_HZTAG)
	{
		const auto chan_list_item = chanlistitem_ptr_array[horizontal_tags.get_channel(hc_trapped_)];
		m_y_we_ = chan_list_item->GetYextent(); // store extent
		m_y_wo_ = chan_list_item->GetYzero(); // store zero
	}
}

void ChartData::OnMouseMove(UINT nFlags, CPoint point)
{
	switch (track_mode_)
	{
	case TRACK_CURVE:
		curve_xor(); // erase past curve and compute new zero
		m_zero = MulDiv(point.y - m_pt_first_.y, m_XORyext, -m_y_ve_) + m_cur_track_;
		curve_xor(); // plot new curve
		break;

	default:
		ChartWnd::OnMouseMove(nFlags, point);
		break;
	}
}

void ChartData::OnLButtonUp(UINT nFlags, CPoint point)
{
	release_cursor();
	switch (track_mode_)
	{
	case TRACK_CURVE:
		{
			curve_xor(); 
			const auto chan_list_item = chanlistitem_ptr_array[m_hitcurve];
			chan_list_item->SetYzero(m_zero);
			track_mode_ = TRACK_OFF;
			post_my_message(HINT_HITCHANNEL, m_hitcurve);
			Invalidate();
		}
		break;

	case TRACK_HZTAG:
		left_button_up_horizontal_tag(nFlags, point);
		break;

	case TRACK_VTTAG:
		{
			// convert pix into data value and back again
			const auto l_val = static_cast<long>(point.x) * (file_position_last_right_pixel_ - file_position_first_left_pixel_ + 1) / static_cast<long>(m_display_rect_.
				right) + file_position_first_left_pixel_;
			vertical_tags.set_tag_l_value(hc_trapped_, l_val);
			point.x = static_cast<int>((l_val - file_position_first_left_pixel_) 
				* static_cast<long>(m_display_rect_.right) 
				/ (file_position_last_right_pixel_ - file_position_first_left_pixel_ + 1));
			xor_vertical_tag(point.x);
			post_my_message(HINT_CHANGEVERTTAG, hc_trapped_);
			track_mode_ = TRACK_OFF;
		}
		break;

	case TRACK_RECT:
		{
			// skip too small a rectangle (5 pixels?)
			CRect rect_out(m_pt_first_.x, m_pt_first_.y, m_pt_last_.x, m_pt_last_.y);
			constexpr auto jitter = 3;
			const BOOL b_rect_ok = ((abs(rect_out.Height()) > jitter) || (abs(rect_out.Width()) > jitter));

			// perform action according to cursor type
			auto rect_in = m_display_rect_;
			switch (cursor_type_)
			{
			case 0: // if no cursor, no curve track, then move display
				if (b_rect_ok)
				{
					invert_tracker(point);
					rect_out = rect_in;
					rect_out.OffsetRect(m_pt_first_.x - m_pt_last_.x, m_pt_first_.y - m_pt_last_.y);
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
				post_my_message(HINT_SETMOUSECURSOR, old_cursor_type_);
				break;
			case CURSOR_CROSS:
				post_my_message(HINT_DEFINEDRECT, NULL);
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
}

int ChartData::does_cursor_hit_curve(const CPoint point)
{
	auto channel_found = -1; // output value
	const auto i_channels = chanlistitem_ptr_array.GetUpperBound();
	auto chan_list_item = chanlistitem_ptr_array[0]->pEnvelopeAbcissa;
	const auto x_extent = chan_list_item->GetnElements();
	int index1 = point.x - cx_jitter_;
	auto index2 = index1 + cx_jitter_;
	if (index1 < 0) index1 = 0; 
	if (index2 > (get_rect_width() - 1)) index2 = get_rect_width() - 1;

	// convert index1 into Envelope indexes
	index1 = index1 * m_dataperpixel; // start from
	index2 = (index2 + 1) * m_dataperpixel; // stop at
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
		const auto i_jitter = MulDiv(cy_jitter_, get_channel_list_item(chan)->GetYextent(), -m_y_ve_);
		const auto val_max = i_val + i_jitter; // mouse max
		const auto val_min = i_val - i_jitter; // mouse min
		chan_list_item = chanlistitem_ptr_array[chan]->pEnvelopeOrdinates;

		// loop around horizontal jitter...
		for (auto index = index1; index < index2 && channel_found < 0; index++)
		{
			int k_max = chan_list_item->GetPointAt(index); // get chan Envelope data point
			// special case: one point per pixel
			if (m_dataperpixel == 1)
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
				int k_min = chan_list_item->GetPointAt(index); // get min
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

void ChartData::move_hz_tag_to_val(int i, int val)
{
	const auto chan = horizontal_tags.get_channel(i);
	const auto chan_list_item = chanlistitem_ptr_array[chan];
	m_XORyext = chan_list_item->GetYextent();
	m_zero = chan_list_item->GetYzero();
	m_pt_last_.y = MulDiv(horizontal_tags.get_value(i) - m_zero, m_y_ve_, m_XORyext) + m_y_vo_;
	CPoint point;
	point.y = MulDiv(val - m_zero, m_y_ve_, m_XORyext) + m_y_vo_;
	xor_horizontal_tag(point.y);
	horizontal_tags.set_tag_val(i, val);
}

void ChartData::set_highlight_data(const CHighLight& source)
{
	m_highlighted = source;
}

void ChartData::set_highlight_data(const CDWordArray* p_intervals)
{
	m_highlighted.l_first.RemoveAll();
	m_highlighted.l_last.RemoveAll();
	if (p_intervals == nullptr)
		return;

	m_highlighted.channel = static_cast<int>(p_intervals->GetAt(0));
	m_highlighted.color = p_intervals->GetAt(1);
	m_highlighted.pensize = static_cast<int>(p_intervals->GetAt(2));
	const auto size = (p_intervals->GetSize() - 3) / 2;
	m_highlighted.l_first.SetSize(size);
	m_highlighted.l_last.SetSize(size);

	for (auto i = 3; i < p_intervals->GetSize(); i += 2)
	{
		m_highlighted.l_first.Add(static_cast<long>(p_intervals->GetAt(i)));
		m_highlighted.l_last.Add(static_cast<long>(p_intervals->GetAt(i + 1)));
	}
}

void ChartData::highlight_data(CDC* p_dc, int chan)
{
	// skip if not the correct chan
	if (chan != m_highlighted.channel || m_highlighted.l_first.GetSize() < 2)
		return;

	// get color and pen size from array p_intervals
	CPen new_pen;
	new_pen.CreatePen(PS_SOLID, m_highlighted.pensize, m_highlighted.color);
	const auto old_pen = p_dc->SelectObject(&new_pen);
	const BOOL b_poly_line = (p_dc->m_hAttribDC == nullptr)
		|| (p_dc->GetDeviceCaps(LINECAPS) & LC_POLYLINE);

	// loop to display data

	// pointer to descriptor
	for (auto i = 0; i < m_highlighted.l_first.GetSize(); i++)
	{
		// load ith interval values
		auto l_first = m_highlighted.l_first[i]; // first value
		auto l_last = m_highlighted.l_last[i]; // last value

		if (l_last < m_lxFirst || l_first > m_lxLast)
			continue; // next if out of range

		// clip data if out of range
		if (l_first < m_lxFirst) // minimum interval
			l_first = m_lxFirst;
		if (l_last > m_lxLast) // maximum interval
			l_last = m_lxLast;

		// compute corresponding interval (assume same m_scale for all chans... (!!)
		auto i_first = m_scale.GetWhichInterval(l_first - m_lxFirst);
		if (i_first < 0)
			continue;
		auto i_last = m_scale.GetWhichInterval(l_last - m_lxFirst) + 1;
		if (i_last < 0)
			continue;

		if (m_dataperpixel != 1)
		{
			// envelope plotting
			i_first = i_first * m_dataperpixel;
			i_last = i_last * m_dataperpixel;
		}
		// display data
		const auto n_elements = i_last - i_first;
		if (b_poly_line)
			p_dc->Polyline(&m_PolyPoints[i_first], n_elements);
		else
		{
			p_dc->MoveTo(m_PolyPoints[i_first]);
			for (auto j = 0; j < n_elements; j++)
				p_dc->LineTo(m_PolyPoints[i_first + j]);
		}
	}

	// restore previous pen
	p_dc->SelectObject(old_pen);
}

void ChartData::Serialize(CArchive& ar)
{
	ChartWnd::Serialize(ar);
	m_PolyPoints.Serialize(ar);
	m_scale.Serialize(ar);

	if (ar.IsStoring())
	{
		ar << m_dataperpixel; // nb of data point per pixel
		ar << m_lxVeryLast; // end of document
		ar << m_lxPage; // size of page increment / file index
		ar << m_lxLine; // size of line increment / file index
		ar << m_lxSize; // nb of data pts represented in a Envelope
		ar << m_lxFirst; // file index of 1rst pt in the Envelopes
		ar << m_lxLast; // file index of last pt in the Envelopes
		ar << m_npixels; // nb pixels displayed horizontally

		const auto n_envelopes = envelope_ptr_array.GetSize();
		ar << n_envelopes;
		const auto n_channels = chanlistitem_ptr_array.GetSize();
		ar << n_channels;

		for (auto i = 0; i < n_envelopes; i++)
			envelope_ptr_array[i]->Serialize(ar);

		for (auto i = 0; i < n_channels; i++)
			chanlistitem_ptr_array[i]->Serialize(ar);
	}
	else
	{
		ar >> m_dataperpixel; // nb of data point per pixel
		ar >> m_lxVeryLast; // end of document
		ar >> m_lxPage; // size of page increment / file index
		ar >> m_lxLine; // size of line increment / file index
		ar >> m_lxSize; // nb of data pts represented in a Envelope
		ar >> m_lxFirst; // file index of 1rst pt in the Envelopes
		ar >> m_lxLast; // file index of last pt in the Envelopes
		ar >> m_npixels; // nb pixels displayed horizontally

		int n_envelopes;
		ar >> n_envelopes;
		int n_chan_list_items;
		ar >> n_chan_list_items;

		// CEnvelope array
		if (envelope_ptr_array.GetSize() > n_envelopes)
		{
			for (auto i = envelope_ptr_array.GetUpperBound(); i >= n_envelopes; i--)
				delete envelope_ptr_array[i];
			envelope_ptr_array.SetSize(n_envelopes);
		}
		else if (envelope_ptr_array.GetSize() < n_envelopes)
		{
			const auto n_envelope_0 = envelope_ptr_array.GetSize();
			envelope_ptr_array.SetSize(n_envelopes);
			for (auto i = n_envelope_0; i < n_envelopes; i++)
				envelope_ptr_array[i] = new CEnvelope; // (CEnvelope*)
		}
		for (auto i = 0; i < n_envelopes; i++)
			envelope_ptr_array[i]->Serialize(ar);

		// ChanList array
		if (chanlistitem_ptr_array.GetSize() > n_chan_list_items)
		{
			for (auto i = chanlistitem_ptr_array.GetUpperBound(); i >= n_chan_list_items; i--)
				delete chanlistitem_ptr_array[i];
			chanlistitem_ptr_array.SetSize(n_chan_list_items);
		}
		else if (chanlistitem_ptr_array.GetSize() < n_chan_list_items)
		{
			const auto n_chan_list_items_0 = chanlistitem_ptr_array.GetSize();
			chanlistitem_ptr_array.SetSize(n_chan_list_items);
			for (auto i = n_chan_list_items_0; i < n_chan_list_items; i++)
			{
				chanlistitem_ptr_array[i] = new CChanlistItem;
			}
		}
		auto ix = 0;
		auto iy = 0;
		for (auto i = 0; i < n_chan_list_items; i++)
		{
			chanlistitem_ptr_array[i]->Serialize(ar);
			chanlistitem_ptr_array[i]->GetEnvelopeArrayIndexes(ix, iy);
			chanlistitem_ptr_array[i]->SetEnvelopeArrays(envelope_ptr_array.GetAt(ix), ix, envelope_ptr_array.GetAt(iy),
			                                             iy);
		}
	}
}

void ChartData::set_track_spike(const BOOL b_track_spike, const int track_len, const int track_offset, const int track_channel)
{
	m_btrackspike = b_track_spike;
	m_tracklen = track_len;
	m_trackoffset = track_offset;
	m_trackchannel = track_channel;
}

void ChartData::adjust_gain(const boolean b_set_span_mv, const float span_mv_value) const
{
	const auto channels_list_size = get_channel_list_size();
	auto j = channels_list_size - 1;
	int max, min;
	for (auto i = j; i >= 0; i--)
	{
		CChanlistItem* chan = get_channel_list_item(i);
		chan->GetMaxMin(&max, &min);
		auto span = max - min + 1;
		int extent;
		int zero;
		if (b_set_span_mv)
		{
			span = chan->ConvertVoltsToDataBins(span_mv_value / 1000.f);
			zero = chan->GetDataBinZero();
			extent = span;
		}
		else
		{
			extent = MulDiv(span, 11 * channels_list_size, 10);
			zero = (max + min) / 2 - MulDiv(extent, j, channels_list_size * 2);
		}
		j -= 2;
		chan->SetYextent(extent);
		chan->SetYzero(zero);
	}
}

void ChartData::load_data_within_window(const boolean set_time_span, const float t_first, const float t_last)
{
	const auto n_pixels = get_rect_width();
	long l_first = 0;
	long l_last = m_pDataFile->get_doc_channel_length() - 1;
	if (set_time_span)
	{
		l_first = static_cast<long>(t_first * m_samplingrate);
		l_last = static_cast<long>(t_last * m_samplingrate);
		if (l_last == l_first)
			l_last++;
	}
	resize_channels(n_pixels, l_last - l_first + 1);
	get_data_from_doc(l_first, l_last);
}

void ChartData::load_all_channels(int data_transform)
{
	const int n_document_channels = m_pDataFile->get_waveformat()->scan_count;
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
			if (get_channel_list_item(j)->GetSourceChan() == channel)
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
		get_channel_list_item(channel)->SetColor(static_cast<WORD>(channel));
	}
}
