#include "StdAfx.h"
#include "Spikelist.h"
#include "AcqDataDoc.h"
#include "SpikeFromChan.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



IMPLEMENT_SERIAL(SpikeList, CObject, 0)

SpikeList::SpikeList()
{
	flagged_spikes_.SetSize(0);
	class_properties_.SetSize(0);
}

SpikeList::~SpikeList()
{
	clear_flagged_spikes();
	delete_arrays();
}

void SpikeList::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << id_string_;
		w_version_ = 9;
		ar << w_version_;
		serialize_version_9(ar);
	}
	else
	{
		delete_arrays();
		CString cs_id; ar >> cs_id;
		if (cs_id == id_string_)
		{
			WORD version; ar >> version;
			if (version == 9)
				serialize_version_9(ar);
			//else if (version == 8)
			//	serialize_version_8(ar);
			else if (version == 7 || version == 6)
				serialize_version_7(ar);
			else if (version == 5)
				read_file_version_5(ar);
			else if (version > 0 && version < 5)
				read_file_version_before_5(ar, version);
			else
			{
				CString message;
				message.Format(_T("reading spike list: unrecognizable version %i"), version);
				AfxMessageBox(message, MB_OK);
			}
		}
		else
			read_file_version_1(ar);
	}
}

void SpikeList::read_file_version_1(CArchive& ar)
{
	m_i_center_1_sl = 0;
	m_i_center_2_sl = options_detect_spk_.detect_pre_threshold;
	shape_t1 = m_i_center_2_sl;
	shape_t2 = get_spike_length() - 1;
}

void SpikeList::remove_artefacts()
{
	auto n_spikes = spikes_.GetSize();
	auto n_spikes_ok = n_spikes;

	for (auto i = n_spikes - 1; i >= 0; i--)
	{
		if (spikes_.GetAt(i)->get_class_id() < 0)
		{
			const auto se = spikes_.GetAt(i);
			delete se;
			spikes_.RemoveAt(i);
			n_spikes_ok--;
		}
	}
	n_spikes = spikes_.GetSize();
	ASSERT(n_spikes_ok == n_spikes);
	update_class_list();
}

void SpikeList::serialize_version_9(CArchive& ar)
{
	// TODO - finalize
	CString id_string =  _T("dbwave spike list v9") ;
	int n_items = 15;
	
	if (ar.IsStoring())
	{
		int n_stored = 0;
		ar << n_items;
		// ---
		ar << id_string; n_stored++;
		ar << data_encoding_mode_; n_stored++;
		ar << bin_zero_; n_stored++;
		ar << sampling_rate_; n_stored++;
		ar << volts_per_bin_; n_stored++;
		ar << channel_description_; n_stored++;
		ar << m_i_center_1_sl; n_stored++;
		ar << m_i_center_2_sl; n_stored++;
		ar << shape_t1; n_stored++;
		ar << shape_t2; n_stored++;

		ar << data_encoding_mode_; n_stored++;
		ar << bin_zero_; n_stored++;
		ar << sampling_rate_; n_stored++;
		ar << volts_per_bin_; n_stored++;
		ar << channel_description_; n_stored++;

		ASSERT(n_items  == n_stored);
	}
	else
	{
		ar >> n_items;
		// --
		if (n_items > 0) ar >> id_string; n_items--;
		if (n_items > 0) ar >> data_encoding_mode_; n_items--;
		if (n_items > 0) ar >> bin_zero_; n_items--;
		if (n_items > 0) ar >> sampling_rate_; n_items--;
		if (n_items > 0) ar >> volts_per_bin_; n_items--;
		if (n_items > 0) ar >> channel_description_; n_items--;
		if (n_items > 0) ar >> m_i_center_1_sl; n_items--;
		if (n_items > 0) ar >> m_i_center_2_sl; n_items--;
		if (n_items > 0) ar >> shape_t1; n_items--;
		if (n_items > 0) ar >> shape_t2; n_items--;

		if (n_items > 0) ar >> data_encoding_mode_; n_items--;
		if (n_items > 0) ar >> bin_zero_; n_items--;
		if (n_items > 0) ar >> sampling_rate_; n_items--;
		if (n_items > 0) ar >> volts_per_bin_; n_items--;
		if (n_items > 0) ar >> channel_description_; n_items--;
	}

	int n_objects = 4;
	options_detect_spk_.Serialize(ar); n_objects--;
	wave_channel_.Serialize(ar); n_objects--;
	serialize_spikes(ar); n_objects--;
	serialize_spike_class_properties(ar); n_objects--;

	ASSERT(n_objects == 0);

	if (ar.IsStoring())
		ar.Flush();
}

void SpikeList::serialize_version_8(CArchive& ar)
{
	serialize_data_parameters(ar);
	serialize_spikes(ar);

	serialize_spike_class_properties(ar);
	serialize_additional_data(ar);
	if (ar.IsStoring())
		ar.Flush();
}

void SpikeList::serialize_version_7(CArchive& ar)
{
	serialize_data_parameters(ar);
	serialize_spikes(ar);
	serialize_spike_data_short(ar);

	serialize_spike_class_properties(ar);
	serialize_additional_data(ar);
	if (ar.IsStoring())
		ar.Flush();
}

void SpikeList::serialize_data_parameters(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << data_encoding_mode_;
		ar << bin_zero_;
		ar << sampling_rate_;
		ar << volts_per_bin_;
		ar << channel_description_;
	}
	else
	{
		ar >> data_encoding_mode_;
		ar >> bin_zero_;
		ar >> sampling_rate_;
		ar >> volts_per_bin_;
		ar >> channel_description_;
	}
	options_detect_spk_.Serialize(ar);
	wave_channel_.Serialize(ar);
}

void SpikeList::serialize_spikes(CArchive& ar)
{
	if (ar.IsStoring())
	{
		if (!b_save_artefacts_)
			remove_artefacts();
		const auto n_spikes = spikes_.GetSize();
		ar << static_cast<WORD>(n_spikes);
		for (auto i = 0; i < n_spikes; i++)
		{
			auto* spike = spikes_.GetAt(i);
			spike->Serialize(ar);
		}
	}
	else
	{
		WORD w1; ar >> w1; const int n_spikes = w1;
		spikes_.SetSize(n_spikes);
		for (auto i = 0; i < n_spikes; i++)
		{
			auto* spike = new Spike();
			ASSERT(spike != NULL);
			spike->Serialize(ar);
			spikes_.SetAt(i, spike);
		}
	}
}

void SpikeList::serialize_spike_data_short(CArchive& ar) 
{
	const auto n_spikes = spikes_.GetSize();
	if (ar.IsStoring())
	{
		const WORD spike_length = static_cast<WORD>(get_spike_length());
		ar << spike_length;

		const auto n_bytes_short = spike_length * sizeof(short);
		if (n_bytes_short > 0)
		{
			const auto buffer = static_cast<short*>(malloc(n_bytes_short * n_spikes));
			if (buffer != nullptr)
			{
				short* p_buffer = buffer;
				for (auto i = 0; i < n_spikes; i++)
				{
					Spike* spike = get_spike(i);
					int* p_data = spike->get_p_data(spike_length);

					for (int j = 0; j < spike_length; j++, p_buffer++, p_data++)
					{
						const short val = static_cast<short>(*p_data);
						*p_buffer = static_cast<short>(val);
					}
				}
				ar.Write(buffer, n_bytes_short * n_spikes);
				free(buffer);
			}
			else
				printf("Serialize SpikeList: not enough memory to write spike data\n");
		}
	}
	else
	{
		WORD spike_length;
		ar >> spike_length;
		spike_length_ = spike_length;

		const auto n_bytes_short = spike_length * sizeof(short) ;
		const auto buffer = static_cast<short*>(malloc(n_bytes_short * n_spikes));
		if (buffer)
		{
			ar.Read(buffer, n_bytes_short * n_spikes);
			short* p_buffer = buffer;
			for (auto i = 0; i < n_spikes; i++)
			{
				Spike* spike = get_spike(i);
				int* p_data = spike->get_p_data(spike_length);
				spike->set_spike_length (spike_length);
				//spike->buffer_spike_length_ = spike_length;
				
				for (int j = 0; j < spike_length; j++, p_buffer++, p_data++ )
				{
					const int val = *p_buffer;
					*p_data = val;
				}
			}
			free(buffer);
		}
		else
			printf("Serialize SpikeList: not enough memory to read spike data\n");
	}
}

void SpikeList::serialize_spike_class_properties(CArchive& ar)
{
	if (ar.IsStoring())
	{
		keep_only_valid_classes_ = FALSE;
		ar << static_cast<long>(keep_only_valid_classes_);
		const auto n_properties = class_properties_.GetCount();
		ar << static_cast<long>(n_properties);
		for (auto i = 0; i < n_properties; i++)
		{
			SpikeClassProperties* item = class_properties_.GetAt(i);
			item->Serialize(ar);
		}
	}
	else
	{
		extrema_valid_ = FALSE;
		class_properties_.RemoveAll();
		long dummy;
		ar >> dummy; keep_only_valid_classes_ = dummy;
		ar >> dummy; n_classes_ = dummy;
		class_properties_.SetSize(n_classes_);
		for (auto i = 0; i < n_classes_; i++)
		{
			const auto item = new SpikeClassProperties();
			item->Serialize(ar);
			class_properties_.SetAt(i, item);
		}
	}
}

void SpikeList::serialize_additional_data(CArchive& ar)
{
	if (ar.IsStoring())
	{
		constexpr int n_parameters = 4;
		ar << n_parameters;
		ar << m_i_center_1_sl;
		ar << m_i_center_2_sl;
		ar << shape_t1;
		ar << shape_t2;
	}
	else
	{
		int n_parameters;
		ar >> n_parameters;
		if (n_parameters < 5) {
			ar >> m_i_center_1_sl; n_parameters--;
			ar >> m_i_center_2_sl; n_parameters--;
			ar >> shape_t1; n_parameters--;
			ar >> shape_t2; n_parameters--;
			ASSERT(n_parameters < 1);
		}
	}
}

void SpikeList::read_file_version_5(CArchive& ar)
{
	serialize_data_parameters(ar);

	WORD w1;
	ar >> w1;
	const int n_spikes = w1;
	spikes_.SetSize(n_spikes); 
	for (auto i = 0; i < n_spikes; i++) 
	{
		auto* se = new(Spike);
		ASSERT(se != NULL);
		se->read_version0(ar);
		spikes_[i] = se;
	}

	serialize_spike_data_short(ar);

	extrema_valid_ = FALSE;
	keep_only_valid_classes_ = FALSE; 
	n_classes_ = 1;
	class_properties_.SetSize(1);
	class_properties_.SetAt(0, new SpikeClassProperties()); 
	long dummy;
	ar >> dummy;
	keep_only_valid_classes_ = dummy;
	if (keep_only_valid_classes_) 
	{
		ar >> dummy;
		n_classes_ = dummy;
		if (n_classes_ != 0)
		{
			class_properties_.SetSize(n_classes_);
			for (int i = 0; i < n_classes_; i++)
			{
				read_class_descriptors_v5(ar, i);
			}
		}
	}

	serialize_additional_data(ar);
}

void SpikeList::read_class_descriptors_v5(CArchive& ar, const int index)
{
	int dummy1, dummy2;
	CString cs;
	ar >> dummy1;
	ar >> dummy2;
	cs.Format(_T("class %i"), dummy1);

	class_properties_.SetAt(index, new SpikeClassProperties(dummy1, dummy2, cs));
}

void SpikeList::read_file_version_before_5(CArchive& ar, const int version)
{
	// (1) version ID already loaded

	// (2) load parameters associated to data acquisition
	auto* pf_c = new SpikeFromChan;
	auto i_objects = 1;
	if (version == 4)
		ar >> i_objects;
	for (auto i = 0; i < i_objects; i++)
	{
		pf_c->Serialize(ar);
		if (i > 0)
			continue;
		data_encoding_mode_ = pf_c->encoding; 
		bin_zero_ = pf_c->bin_zero; 
		sampling_rate_ = pf_c->sampling_rate; 
		volts_per_bin_ = pf_c->volts_per_bin;
		channel_description_ = pf_c->comment; 
		options_detect_spk_ = pf_c->detect_spikes_parameters; 
	}
	delete pf_c;
	wave_channel_.Serialize(ar);

	// ----------------------------------------------------
	// (3) load SpikeElements 
	WORD w1;
	ar >> w1;
	const int n_spikes = w1;
	spikes_.SetSize(n_spikes); 
	for (auto i = 0; i < n_spikes; i++)
	{
		const auto se = new(Spike);
		ASSERT(se != NULL);
		se->read_version0(ar); 
		spikes_[i] = se;
	}

	// ----------------------------------------------------
	// (4) load spike raw data
	ar >> w1; 
	set_spike_length(w1); 
	bin_zero_ = get_acq_bin_zero();

	// loop through all data buffers
	const auto n_bytes = w1 * sizeof(short);
	for (int i = 0; i < n_spikes; i++)
	{
		const auto lp_dest = get_spike(i)->get_p_data(get_spike_length());
		ar.Read(lp_dest, n_bytes); 
	}
	
	extrema_valid_ = FALSE;

	// ----------------------------------------------------
	// (5) load class array if valid flag
	// reset elements of the list
	keep_only_valid_classes_ = FALSE; // default: no valid array
	n_classes_ = 1;
	class_properties_.SetSize(1); // default size - some functions
	class_properties_.SetAt(0, new SpikeClassProperties());

	// load flag and load elements only if valid
	long dummy;
	ar >> dummy;
	keep_only_valid_classes_ = dummy;
	if (keep_only_valid_classes_) // read class list
	{
		ar >> dummy;
		n_classes_ = dummy;
		if (n_classes_ != 0)
		{
			class_properties_.SetSize(n_classes_);
			for (int i = 0; i < n_classes_; i++)
			{
				read_class_descriptors_v5(ar, i);
			}
		}
	}
	if (version > 2)
	{
		int n_parameters;
		ar >> n_parameters;
		ar >> m_i_center_1_sl;
		n_parameters--;
		ar >> m_i_center_2_sl;
		n_parameters--;
		ar >> shape_t1;
		n_parameters--;
		ar >> shape_t2;
		n_parameters--;
	}
	if (version < 3)
	{
		m_i_center_1_sl = 0;
		m_i_center_2_sl = options_detect_spk_.detect_pre_threshold;
		shape_t1 = m_i_center_2_sl;
		shape_t2 = get_spike_length() - 1;
	}
}

void SpikeList::delete_arrays()
{
	const int n_spikes = spikes_.GetSize();
	if (n_spikes > 0)
	{
		for (auto i = n_spikes - 1; i >= 0; i--)
		{
			const Spike* spike = spikes_.GetAt(i);
			delete spike;
		}
		spikes_.RemoveAll();
	}

	const int n_classes = class_properties_.GetCount();
	if (n_classes > 0)
	{
		for (int i = 0; i < n_classes; i++)
			delete class_properties_.GetAt(i);
	}
	class_properties_.RemoveAll();
}

int SpikeList::remove_spike(const int spike_index)
{
	if (spike_index <= spikes_.GetUpperBound())
	{
		const auto element = spikes_.GetAt(spike_index);
		delete element;
		spikes_.RemoveAt(spike_index);
	}
	return spikes_.GetSize();
}

BOOL SpikeList::is_any_spike_around(const long ii_time, const int jitter, int& spike_index, const int channel_index)
{
	// search spike index of first spike which time is > to present one
	spike_index = 0;
	if (spikes_.GetSize() > 0)
	{
		// loop to find position of the new spike
		for (auto j = spikes_.GetUpperBound(); j >= 0; j--)
		{
			// assumed ordered list
			if (ii_time >= spikes_[j]->get_time())
			{
				spike_index = j + 1; 
				break; 
			}
		}
	}

	// check if no duplicate otherwise exit immediately
	long delta_time; 
	if (spike_index > 0) 
	{
		delta_time = spikes_[spike_index - 1]->get_time() - ii_time;
		if (labs(delta_time) <= jitter) 
		{
			spike_index--; 
			return TRUE;
		}
	}

	if (spike_index <= spikes_.GetUpperBound()) 
	{
		delta_time = spikes_[spike_index]->get_time() - ii_time;
		const auto i_channel_2 = spikes_[spike_index]->get_source_channel();
		if (i_channel_2 == channel_index && labs(delta_time) <= jitter) 
			return TRUE; 
	}

	return FALSE;
}

CString SpikeList::get_class_description_from_id(const int class_id)
{
	SpikeClassProperties* p_class_descriptor = get_class_descriptor_from_id(class_id);
	if (p_class_descriptor == nullptr)
		return {_T("undefined")};
	else
		return p_class_descriptor->get_class_text();
}

int SpikeList::get_class_id_index(const int class_id)
{
	int item_index = -1;
	const int n_properties = class_properties_.GetCount();
	if (n_properties > 0)
	{
		for (int i = 0; i < n_properties; i++)
		{
			if (class_properties_.GetAt(i)->get_class_id() != class_id)
				continue;
			item_index = i;
			break;
		}
	}
	return item_index;
}

SpikeClassProperties* SpikeList::get_class_descriptor_from_id(const int class_id)
{
	const int index = get_class_id_index(class_id);
	if (index < 0)
		return nullptr;
	return class_properties_.GetAt(index);
}

int SpikeList::increment_class_n_items(const int class_id)
{
	const int index = get_class_id_index(class_id);
	if (index < 0)
		return 0;
	return class_properties_.GetAt(index)->increment_n_items();
}

int SpikeList::decrement_class_n_items(const int class_id)
{
	const int index = get_class_id_index(class_id);
	return class_properties_.GetAt(index)->decrement_n_items();
}

void SpikeList::change_spike_class_id(const int spike_no, const int class_id)
{
	Spike* spike = get_spike(spike_no);
	decrement_class_n_items(spike->get_class_id());

	spike->set_class_id(class_id);
	increment_class_n_items(class_id);
}

int SpikeList::add_spike(short* source_data, const int n_channels, const long ii_time, const int source_channel, const int i_class, const BOOL b_check)
{
	// search spike index of first spike for which time is > to present one
	int index_added_spike;
	auto jitter = 0;
	if (b_check)
		jitter = m_jitter_sl;
	const auto b_found = is_any_spike_around(ii_time, jitter, index_added_spike, source_channel);

	if (!b_found)
	{
		auto* se = new Spike(ii_time, source_channel, 0, i_class, spike_length_);
		ASSERT(se != NULL);
		spikes_.InsertAt(index_added_spike, se);

		if (source_data != nullptr)
		{
			se->transfer_data_to_spike_buffer(source_data, n_channels, spike_length_);
			se->measure_amplitude_min_to_max(1, spike_length_-1);
		}
	}
	return index_added_spike;
}

int SpikeList::get_index_first_spike(const int index_start, const boolean reject_artefacts)
{
	int index_found = -1;
	const int n_spikes = spikes_.GetCount();
	if (n_spikes == 0)
		return index_found;
	for (auto index = index_start; index < n_spikes; index++)
	{
		const Spike* spike = get_spike(index);
		if (!reject_artefacts)
		{
			index_found = index_start;
			break;
		}

		if (spike->get_class_id() >= 0)
		{
			index_found = index;
			break;
		}
	}
	return index_found;
}

void SpikeList::get_total_max_min(const BOOL b_recalculate, int* max, int* min)
{
	if (b_recalculate || !extrema_valid_)
		get_total_max_min_measure();
	*max = maximum_over_all_spikes_;
	*min = minimum_over_all_spikes_;
}

void SpikeList::get_total_max_min_read()
{
	const int index0 = get_index_first_spike(0, true);
	minimum_over_all_spikes_ = bin_zero_;
	maximum_over_all_spikes_ = minimum_over_all_spikes_;
	if (index0 < 0)
		return;

	int max1, min1;
	const int n_spikes = spikes_.GetCount();
	boolean initialized = false;

	for (auto index = index0; index < n_spikes; index++)
	{
		const Spike* spike = get_spike(index);
		if (spike->get_class_id() >= 0)
		{
			spike->get_max_min(&max1, &min1);
			if (! initialized || max1 > maximum_over_all_spikes_)
				maximum_over_all_spikes_ = max1;
			if (!initialized || min1 < minimum_over_all_spikes_)
				minimum_over_all_spikes_ = min1;
			initialized = true;
		}
	}
}

void SpikeList::get_total_max_min_measure()
{
	const int index0 = get_index_first_spike(0, true);
	minimum_over_all_spikes_ = static_cast<short>(bin_zero_);
	maximum_over_all_spikes_ = minimum_over_all_spikes_;
	if (index0 < 0)
		return;

	const int n_spikes = spikes_.GetCount();
	if (n_spikes < 1)
		return;
	int value_max, value_min;
	int index_max, index_min;
	const int i_last = get_spike_length() - 1;
	boolean initialized = false;

	for (auto spike_index = index0; spike_index < n_spikes; spike_index++)
	{
		const Spike* spike = get_spike(spike_index);
		if (spike->get_class_id() >= 0)
		{
			spike->measure_max_min_ex(&value_max, &index_max, &value_min, &index_min, 1, i_last);
			if (!initialized || value_max > maximum_over_all_spikes_)
				maximum_over_all_spikes_ = value_max;
			if (!initialized || value_min < minimum_over_all_spikes_)
				minimum_over_all_spikes_ = value_min;
			initialized = true;
		}
	}
}

int SpikeList::get_total_max_min_of_y1(int* max, int* min)
{
	const int n_spikes_found = get_total_max_min_of_y1_measure();
	*max = max_y1_over_all_spikes_;
	*min = min_y1_over_all_spikes_;
	return n_spikes_found;
}

int SpikeList::get_total_max_min_of_y1_measure()
{
	const int index0 = get_index_first_spike(0, true);
	int n_spikes_found = 0;
	min_y1_over_all_spikes_ = 0;
	max_y1_over_all_spikes_ = 0;
	if (index0 < 0)
		return n_spikes_found;

	const int n_spikes = spikes_.GetCount();
	boolean initialized = false;

	for (auto index = index0; index < n_spikes; index++)
	{
		const Spike* spike = get_spike(index);
		if (spike->get_class_id() >= 0)
		{
			const int value = spike->get_y1();
			n_spikes_found++;
			if (!initialized)
			{
				max_y1_over_all_spikes_ = value;
				min_y1_over_all_spikes_ = value;
				initialized = true;
				continue;
			}
			if (value > max_y1_over_all_spikes_)
				max_y1_over_all_spikes_ = value;
			if (value < min_y1_over_all_spikes_)
				min_y1_over_all_spikes_ = value;
		}
	}
	return n_spikes_found;
}

BOOL SpikeList::init_spike_list(const AcqDataDoc* acq_data_doc, const options_detect_spikes* spk_detect_parameters)
{
	// remove data from spike list
	erase_data();
	clear_flagged_spikes();

	// copy data from CObArray
	if (spk_detect_parameters != nullptr)
		options_detect_spk_ = *spk_detect_parameters;

	auto flag = false;
	if (acq_data_doc != nullptr)
	{
		const auto wave_format = acq_data_doc->get_wave_format();
		data_encoding_mode_ = wave_format->mode_encoding;
		bin_zero_ = wave_format->bin_zero;
		sampling_rate_ = wave_format->sampling_rate_per_channel;
		flag = acq_data_doc->get_volts_per_bin(options_detect_spk_.detect_channel, &volts_per_bin_);
	}

	if (!flag)
	{
		extrema_valid_ = TRUE;
		minimum_over_all_spikes_ = 2048;
		maximum_over_all_spikes_ = 2100;
	}

	// reset buffers, list, spk params
	set_spike_length(options_detect_spk_.extract_n_points);

	// reset classes
	keep_only_valid_classes_ = FALSE; // default: no valid array
	n_classes_ = 0;
	class_properties_.SetSize(1); 
	class_properties_.SetAt(0, new SpikeClassProperties());
	return TRUE;
}

void SpikeList::erase_data()
{
	delete_arrays();
	m_selected_spike = -1;
}

int SpikeList::toggle_spike_flag(const int spike_index) 
{
	auto index = -1;
	for (auto i = 0; i <flagged_spikes_.GetCount() ; i ++)
	{
		if (flagged_spikes_.GetAt(i) == spike_index)
		{
			index = i;
			break;
		}
	}

	if (index >= 0)
		flagged_spikes_.RemoveAt(index);
	else
		flagged_spikes_.Add(spike_index);

	return flagged_spikes_.GetCount();
}

void SpikeList::set_spike_flag(const int spike_index, const boolean exclusive)
{
	if (exclusive)
		clear_flagged_spikes();
	if (!get_spike_flag(spike_index))
		flagged_spikes_.Add(spike_index);
}

boolean SpikeList::get_spike_flag(const int spike_index) const
{
	boolean b_flag = false;
	if (flagged_spikes_.GetCount() > 0)
	{
		// search if spike_index is in the list
		for (int i = 0; i < flagged_spikes_.GetCount(); i++)
		{
			if (flagged_spikes_.GetAt(i) == spike_index)
			{
				b_flag = true;
				break;
			}
		}
	}
	return b_flag;
}

void SpikeList::clear_flagged_spikes()
{
	flagged_spikes_.RemoveAll();
}

void SpikeList::flag_range_of_spikes(const long l_first, const long l_last, const boolean b_set)
{
	// first clear flags of spikes within the flagged array which fall within limits
	if (flagged_spikes_ .GetCount() > 0)
	{
		for (auto i = flagged_spikes_.GetCount() - 1; i >= 0; i--)
		{
			const int spike_index = flagged_spikes_.GetAt(i);
			const long l_time = get_spike(spike_index)->get_time();
			if (l_time < l_first || l_time > l_last)
				continue;
			flagged_spikes_.RemoveAt(i);
		}
	}
	// if bSet was set to FALSE, the job is done
	if (!b_set)
		return;

	// then if bSet is ON, search spike file for spikes falling within this range and flag them
	for (auto i = 0; i < get_spikes_count(); i++)
	{
		const long l_time = get_spike(i)->get_time();
		if (l_time < l_first || l_time > l_last)
			continue;
		flagged_spikes_.Add(i);
	}
}

void SpikeList::flag_spikes_within_bounds(const int v_min, const int v_max, const long l_first, const long l_last, const boolean b_add)
{
	if (!b_add)
		clear_flagged_spikes();

	for (auto i = 0; i < get_spikes_count(); i++)
	{
		const auto l_time = get_spike(i)->get_time();
		if (l_time < l_first || l_time > l_last)
			continue;

		int max, min;
		get_spike(i)->get_max_min(&max, &min);
		if (max > v_max) continue;
		if (min < v_min) continue;
		// found within boundaries= remove spike from array
		flagged_spikes_.Add(i);
	}
}

void SpikeList::get_range_of_spike_flagged(long& l_first, long& l_last)
{
	// no spikes flagged, return dummy values
	if (flagged_spikes_.GetCount() < 1)
	{
		l_first = -1;
		l_last = -1;
		return;
	}

	// spikes flagged: init max and min to the first spike time
	l_first = get_spike(flagged_spikes_.GetAt(0))->get_time();
	l_last = l_first;

	// search if spike is in the list
	for (auto i = 0; i < flagged_spikes_.GetCount() ; i++)
	{
		const auto l_time = get_spike(flagged_spikes_.GetAt(i))->get_time();
		if (l_time < l_first)
			l_first = l_time;
		if (l_time > l_last)
			l_last = l_time;
	}
}

void SpikeList::change_class_of_flagged_spikes(const int new_class_id)
{
	const auto n_spikes = get_spike_flag_array_count();
	for (auto i = 0; i < n_spikes; i++)
	{
		const auto spike_no = get_spike_index_of_flag(i);
		Spike* spike = get_spike(spike_no);
		spike->set_class_id(new_class_id);
	}
}

long SpikeList::update_class_list()
{
	const auto n_spikes = get_spikes_count();
	clear_spike_classes_spikes_count();

	// loop over all spikes of the list
	for (auto i = 0; i < n_spikes; i++)
	{
		const auto spike_class = get_spike(i)->get_class_id(); 
		boolean found = false;
		for (auto j = 0; j < class_properties_.GetCount(); j++)
		{
			SpikeClassProperties* desc = class_properties_.GetAt(j);
			if (spike_class == desc->get_class_id()) 
			{
				desc->increment_n_items();
				found = true;
				break; 
			}
		}
		if (!found)
			add_class_id(spike_class);
	}

	remove_spike_classes_with_no_spikes();
	return n_classes_;
}

void SpikeList::remove_spike_classes_with_no_spikes()
{
	const int count = class_properties_.GetCount();
	for (int i = count-1; i >= 0; i--)
	{
		const auto* desc = class_properties_.GetAt(i);
		if( desc->get_class_n_items() < 1)
		{
			class_properties_.RemoveAt(i);
			delete desc;
		}
	}
}

void SpikeList::clear_spike_classes_spikes_count()
{
	const int count = class_properties_.GetCount();
	for (int i = 0; i < count; i++)
	{
		const auto desc = get_class_descriptor_from_index(i);
		if (desc != nullptr)
			desc->set_class_n_items(0);
	}
}

int SpikeList::add_class_id(const int id)
{
	int index = -1;
	for (int i = 0; i< class_properties_.GetCount(); i++)
	{
		if (id < class_properties_.GetAt(i)->get_class_id())
		{
			class_properties_.InsertAt(i, 
				new SpikeClassProperties(id, 1, SpikeClassProperties::get_class_default_descriptor_string(id)));
			index = i;
			break;
		}
	}
	if (index < 0)
		index = class_properties_.Add(
			new SpikeClassProperties(id, 1, SpikeClassProperties::get_class_default_descriptor_string(id)));
	n_classes_++;
	return index;
}

void SpikeList::change_all_spike_from_class_id_to_new_class_id(const int old_class_id, const int new_class_id)
{
	// first find valid max and min
	for (auto index = 0; index < spikes_.GetSize(); index++)
	{
		if (get_spike(index)->get_class_id() == old_class_id)
			get_spike(index)->set_class_id(new_class_id);
	}
}

void SpikeList::measure_amplitude_min_to_max(const int t1, const int t2)
{
	const auto n_spikes = get_spikes_count();
	for (auto spike_index = 0; spike_index < n_spikes; spike_index++)
	{
		const auto spike = get_spike(spike_index);
		spike->measure_amplitude_min_to_max(t1, t2);
	}
}

void SpikeList::measure_amplitude_vt(const int t)
{
	const auto n_spikes = get_spikes_count();

	for (auto spike_index = 0; spike_index < n_spikes; spike_index++)
	{
		const auto spike_element = get_spike(spike_index);
		auto lp_buffer = get_spike(spike_index)->get_p_data(get_spike_length());
		lp_buffer += t;
		const auto val = *lp_buffer;
		spike_element->set_y1(val);
	}
}

void SpikeList::measure_amplitude_vt2_minus_vt1(const int t1, const int t2)
{
	const auto n_spikes = get_spikes_count();

	for (auto spike_index = 0; spike_index < n_spikes; spike_index++)
	{
		const auto spike_element = get_spike(spike_index);
		const auto lp_buffer = get_spike(spike_index)->get_p_data(get_spike_length());
		const auto val1 = *(lp_buffer + t1);
		const auto val2 = *(lp_buffer + t2);
		spike_element->set_y1(val2 - val1);
	}
}

CSize SpikeList::measure_y1_max_min()
{
	const auto n_spikes = get_spikes_count();
	int max = get_spike(0)->get_y1();
	int min = max;
	for (auto spike_index = 0; spike_index < n_spikes; spike_index++)
	{
		const auto val = get_spike(spike_index)->get_y1();
		if (val > max) 
			max = val;
		if (val < min) 
			min = val;
	}

	return {max, min};
}

BOOL SpikeList::sort_spike_with_y1(const CSize from_class_id_to_class_id, const CSize time_bounds, const CSize limits)
{
	const auto n_spikes = get_spikes_count();

	const auto from_class = from_class_id_to_class_id.cx;
	const auto to_class = from_class_id_to_class_id.cy;
	const auto first = time_bounds.cx;
	const auto last = time_bounds.cy;
	const int upper = limits.cy;
	const int lower = limits.cx;
	BOOL b_changed = false;

	for (auto spike_index = 0; spike_index < n_spikes; spike_index++)
	{
		const auto spike_element = get_spike(spike_index);
		if (spike_element->get_class_id() != from_class)
			continue;
		const auto ii_time = spike_element->get_time();
		if (ii_time < first || ii_time > last)
			continue;
		const auto value = spike_element->get_y1();
		if (value >= lower && value <= upper)
		{
			spike_element->set_class_id(to_class);
			b_changed = true;
		}
	}

	return b_changed;
}

BOOL SpikeList::sort_spike_with_y1_and_y2(const CSize from_class_id_to_class_id, const CSize time_bounds, const CSize limits1,
                                      const CSize limits2)
{
	const auto n_spikes = get_spikes_count();

	const auto from_class = from_class_id_to_class_id.cx;
	const auto to_class = from_class_id_to_class_id.cy;
	const auto first = time_bounds.cx;
	const auto last = time_bounds.cy;
	const int upper1 = limits1.cy;
	const int lower1 = limits1.cx;
	const int upper2 = limits2.cy;
	const int lower2 = limits2.cx;
	BOOL b_changed = false;

	for (auto spike_index = 0; spike_index < n_spikes; spike_index++)
	{
		const auto spike_element = get_spike(spike_index);
		if (spike_element->get_class_id() != from_class)
			continue;
		const auto ii_time = spike_element->get_time();
		if (ii_time < first || ii_time > last)
			continue;
		const auto value1 = spike_element->get_y1();
		const auto value2 = spike_element->get_y2();
		if ((value1 >= lower1 && value1 <= upper1) && (value2 >= lower2 && value2 <= upper2))
		{
			spike_element->set_class_id(to_class);
			b_changed = true;
		}
	}

	return b_changed;
}

int SpikeList::get_valid_spike_number(int spike_index) const
{
	if (spike_index < 0)
		spike_index = -1;
	if (spike_index >= get_spikes_count())
		spike_index = get_spikes_count() - 1;
	return spike_index;
}

int SpikeList::get_next_spike(int spike_index, int delta, const BOOL b_keep_same_class) 
{
	const int spike_index_old = spike_index;
	int class_id_old = 0;
	if (spike_index_old >= 0 && spike_index_old < get_spikes_count() - 1)
		class_id_old = get_spike(spike_index)->get_class_id();
	if (delta >= 0)
		delta = 1;
	else
		delta = -1;
	if (b_keep_same_class)
	{
		do
		{
			spike_index += delta;
			if (spike_index < 0 || spike_index >= get_spikes_count())
			{
				spike_index = spike_index_old;
				break;
			}
		}
		while (spike_index < get_spikes_count() && get_spike(spike_index)->get_class_id() != class_id_old);
	}
	else
		spike_index += delta;

	if (spike_index >= get_spikes_count())
		spike_index = spike_index_old;
	return get_valid_spike_number(spike_index);
}
