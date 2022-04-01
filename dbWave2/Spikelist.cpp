#include "StdAfx.h"
#include "AcqDataDoc.h"
#include "SpikeFromChan.h"
#include "Spikelist.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



IMPLEMENT_SERIAL(SpikeList, CObject, 0)

SpikeList::SpikeList()
{
	m_spike_class_descriptor_array.SetSize(1);
	m_spike_class_descriptor_array.Add(SpikeClassDescriptor(0,0));
}

SpikeList::~SpikeList()
{
	delete_arrays();
}

void SpikeList::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << ID_string;
		m_version = 7;
		ar << m_version;
		serialize_version7(ar);
	}
	else
	{
		delete_arrays();
		CString cs_id; ar >> cs_id;
		if (cs_id == ID_string)
		{
			WORD version; ar >> version;
			if (version == 7 || version == 6)
				serialize_version7(ar);
			else if (version == 5)
				read_file_version5(ar);
			else if (version > 0 && version < 5)
				read_file_version_before5(ar, version);
			else
			{
				ASSERT(FALSE);
				CString message;
				message.Format(_T("reading spike list: unrecognizable version %i"), version);
				AfxMessageBox(message, MB_OK);
			}
		}
		else
			read_file_version1(ar);

	}
}

void SpikeList::read_file_version1(CArchive& ar)
{
	m_icenter1SL = 0;
	m_icenter2SL = m_detection_parameters.prethreshold;
	m_imaxmin1SL = m_icenter2SL;
	m_imaxmin2SL = GetSpikeLength() - 1;
}

void SpikeList::remove_artefacts()
{
	auto n_spikes = m_spike_elements.GetSize();
	auto n_spikes_ok = n_spikes;

	for (auto i = n_spikes - 1; i >= 0; i--)
	{
		if (m_spike_elements.GetAt(i)->get_class() < 0)
		{
			const auto se = m_spike_elements.GetAt(i);
			delete se;
			m_spike_elements.RemoveAt(i);
			n_spikes_ok--;
		}
	}
	n_spikes = m_spike_elements.GetSize();
	ASSERT(n_spikes_ok == n_spikes);
	UpdateClassList();
}


void SpikeList::serialize_version7(CArchive& ar)
{
	serialize_data_parameters(ar);
	serialize_spike_elements(ar);
	serialize_spike_data(ar);
	serialize_spike_class_descriptors(ar);
	serialize_additional_data(ar);
	if (ar.IsStoring())
		ar.Flush();
}

void SpikeList::serialize_data_parameters(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << m_data_encoding_mode;
		ar << m_bin_zero;
		ar << m_sampling_rate;
		ar << m_volts_per_bin;
		ar << m_spike_channel_description;
	}
	else
	{
		ar >> m_data_encoding_mode;
		ar >> m_bin_zero;
		ar >> m_sampling_rate;
		ar >> m_volts_per_bin;
		ar >> m_spike_channel_description;
	}
	m_detection_parameters.Serialize(ar);
	m_acquisition_channel.Serialize(ar);
}

void SpikeList::serialize_spike_elements(CArchive& ar)
{
	if (ar.IsStoring())
	{
		if (!m_b_save_artefacts)
			remove_artefacts();
		const auto n_spikes = m_spike_elements.GetSize();
		ar << static_cast<WORD>(n_spikes);
		for (auto i = 0; i < n_spikes; i++)
			m_spike_elements.GetAt(i)->Serialize(ar);
	}
	else
	{
		WORD w1; ar >> w1; const int n_spikes = w1;
		m_spike_elements.SetSize(n_spikes);
		for (auto i = 0; i < n_spikes; i++)
		{
			auto* se = new(Spike);
			ASSERT(se != NULL);
			se->Serialize(ar);
			m_spike_elements.SetAt(i, se);
		}
	}
}

void SpikeList::serialize_spike_data(CArchive& ar) 
{
	const auto n_spikes = m_spike_elements.GetSize();
	if (ar.IsStoring())
	{
		const WORD spike_length = static_cast<WORD>(GetSpikeLength());
		ar << spike_length;

		const auto n_bytes = spike_length * sizeof(short);
		if (n_bytes > 0)
		{
			for (auto i = 0; i < n_spikes; i++)
			{
				ar.Write(GetSpike(i)->GetpSpikeData(spike_length), n_bytes);
			}
		}
	}
	else
	{
		WORD spike_length = 1;
		ar >> spike_length;

		const auto n_bytes = spike_length * sizeof(short) ;
		for (auto i = 0; i < n_spikes; i++)
		{
			ar.Read(GetSpike(i)->GetpSpikeData(spike_length), n_bytes);
		}
	}
}

void SpikeList::serialize_spike_class_descriptors(CArchive& ar)
{
	if (ar.IsStoring())
	{
		m_keep_only_valid_classes = FALSE;
		ar << static_cast<long>(m_keep_only_valid_classes);
		ar << static_cast<long>(m_n_classes);
		for (auto i = 0; i < m_n_classes; i++)
		{
			SpikeClassDescriptor item = m_spike_class_descriptor_array.GetAt(i);
			if (item.n_items > 0 && !(m_keep_only_valid_classes && item.id_number < 0))
				item.Serialize(ar);
		}
	}
	else
	{
		m_extrema_valid = FALSE;

		m_spike_class_descriptor_array.SetSize(1);
		m_spike_class_descriptor_array.SetAt(0, SpikeClassDescriptor(0, 0));
		long dummy;
		ar >> dummy; m_keep_only_valid_classes = dummy;
		ar >> dummy; m_n_classes = dummy;
		
		if (m_n_classes >= 0)
		{
			for (auto i = 0; i < m_n_classes; i++)
			{
				SpikeClassDescriptor item;
				item.Serialize(ar);
				m_spike_class_descriptor_array.SetAt(i, item);
			}
		}
	}
}

void SpikeList::serialize_additional_data(CArchive& ar)
{
	if (ar.IsStoring())
	{
		constexpr int n_parameters = 4;
		ar << n_parameters;
		ar << m_icenter1SL;
		ar << m_icenter2SL;
		ar << m_imaxmin1SL;
		ar << m_imaxmin2SL;
	}
	else
	{
		int n_parameters;
		ar >> n_parameters;
		ar >> m_icenter1SL; n_parameters--;
		ar >> m_icenter2SL; n_parameters--;
		ar >> m_imaxmin1SL; n_parameters--;
		ar >> m_imaxmin2SL; n_parameters--;
		ASSERT(n_parameters < 1);
	}
}

void SpikeList::read_file_version5(CArchive& ar)
{
	serialize_data_parameters(ar);

	WORD w1;
	ar >> w1;
	const int n_spikes = w1;
	m_spike_elements.SetSize(n_spikes); 
	for (auto i = 0; i < n_spikes; i++) 
	{
		auto* se = new(Spike);
		ASSERT(se != NULL);
		se->read_version0(ar);
		m_spike_elements[i] = se;
	}

	serialize_spike_data(ar);

	m_extrema_valid = FALSE;
	m_keep_only_valid_classes = FALSE; 
	m_n_classes = 1;
	m_spike_class_descriptor_array.SetSize(1);
	m_spike_class_descriptor_array.SetAt(0, SpikeClassDescriptor(0, 0)); 
	long dummy;
	ar >> dummy;
	m_keep_only_valid_classes = dummy;
	if (m_keep_only_valid_classes) 
	{
		ar >> dummy;
		m_n_classes = dummy;
		if (m_n_classes != 0)
		{
			m_spike_class_descriptor_array.SetSize(m_n_classes);
			for (int i = 0; i < m_n_classes; i++)
			{
				int dummy1, dummy2;
				ar >> dummy1;
				ar >> dummy2;
				m_spike_class_descriptor_array.SetAt(i, SpikeClassDescriptor(dummy1, dummy2));
			}
		}
	}

	serialize_additional_data(ar);
}

void SpikeList::read_file_version_before5(CArchive& ar, int version)
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
		m_data_encoding_mode = pf_c->encoding; 
		m_bin_zero = pf_c->binzero; 
		m_sampling_rate = pf_c->samprate; 
		m_volts_per_bin = pf_c->voltsperbin;
		m_spike_channel_description = pf_c->comment; 
		m_detection_parameters = pf_c->parm; 
	}
	delete pf_c;
	m_acquisition_channel.Serialize(ar);

	// ----------------------------------------------------
	// (3) load SpikeElements 
	WORD w1;
	ar >> w1;
	const int n_spikes = w1;
	m_spike_elements.SetSize(n_spikes); // change size of the list
	for (auto i = 0; i < n_spikes; i++)
	{
		const auto se = new(Spike);
		ASSERT(se != NULL);
		se->read_version0(ar); 
		m_spike_elements[i] = se;
	}

	// ----------------------------------------------------
	// (4) load spike raw data
	ar >> w1; // read spike length
	SetSpikeLength(w1); // reset parms/buffer
	m_bin_zero = GetAcqBinzero();

	// loop through all data buffers
	const auto n_bytes = w1 * sizeof(short);// *n_spikes;
	for (int i = 0; i < n_spikes; i++)
	{
		const auto lp_dest = GetSpike(i)->GetpSpikeData(GetSpikeLength());
		ar.Read(lp_dest, n_bytes); // read data from disk
	}
	
	m_extrema_valid = FALSE;

	// ----------------------------------------------------
	// (5) load class array if valid flag
	// reset elements of the list
	m_keep_only_valid_classes = FALSE; // default: no valid array
	m_n_classes = 1;
	m_spike_class_descriptor_array.SetSize(2); // default size - some functions
	m_spike_class_descriptor_array.SetAt(0, SpikeClassDescriptor(0, 0)); 

	// load flag and load elements only if valid
	long dummy;
	ar >> dummy;
	m_keep_only_valid_classes = dummy;
	if (m_keep_only_valid_classes) // read class list
	{
		ar >> dummy;
		m_n_classes = dummy;
		if (m_n_classes != 0)
		{
			m_spike_class_descriptor_array.SetSize(m_n_classes);
			for (int i = 0; i < m_n_classes; i++)
			{
				int dummy1, dummy2;
				ar >> dummy1;
				ar >> dummy2;
				m_spike_class_descriptor_array.SetAt(i, SpikeClassDescriptor(dummy1, dummy2));
			}
		}
	}
	if (version > 2)
	{
		int n_parameters;
		ar >> n_parameters;
		ar >> m_icenter1SL;
		n_parameters--;
		ar >> m_icenter2SL;
		n_parameters--;
		ar >> m_imaxmin1SL;
		n_parameters--;
		ar >> m_imaxmin2SL;
		n_parameters--;
	}
	if (version < 3)
	{
		m_icenter1SL = 0;
		m_icenter2SL = m_detection_parameters.prethreshold;
		m_imaxmin1SL = m_icenter2SL;
		m_imaxmin2SL = GetSpikeLength() - 1;
	}
}

void SpikeList::delete_arrays()
{
	const int n_spikes = m_spike_elements.GetSize();
	if (n_spikes > 0)
	{
		for (auto i = n_spikes - 1; i >= 0; i--)
			delete m_spike_elements.GetAt(i);
		m_spike_elements.RemoveAll();
	}

	m_spike_class_descriptor_array.RemoveAll();
}

int SpikeList::RemoveSpike(int spike_index)
{
	if (spike_index <= m_spike_elements.GetUpperBound())
	{
		const auto element = m_spike_elements.GetAt(spike_index);
		delete element;
		m_spike_elements.RemoveAt(spike_index);
	}
	return m_spike_elements.GetSize();
}

BOOL SpikeList::IsAnySpikeAround(const long ii_time, const int jitter, int& spike_index, const int channel_index)
{
	// search spike index of first spike which time is > to present one
	spike_index = 0;
	if (m_spike_elements.GetSize() > 0)
	{
		// loop to find position of the new spike
		for (auto j = m_spike_elements.GetUpperBound(); j >= 0; j--)
		{
			// assumed ordered list
			if (ii_time >= m_spike_elements[j]->get_time())
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
		delta_time = m_spike_elements[spike_index - 1]->get_time() - ii_time;
		if (labs(delta_time) <= jitter) // allow a detection jitter of 2 (?)
		{
			spike_index--; 
			return TRUE;
		}
	}

	if (spike_index <= m_spike_elements.GetUpperBound()) 
	{
		delta_time = m_spike_elements[spike_index]->get_time() - ii_time;
		const auto i_channel_2 = m_spike_elements[spike_index]->get_source_channel();
		if (i_channel_2 == channel_index && labs(delta_time) <= jitter) // allow a detection jitter of 2 (?)
			return TRUE; 
	}

	return FALSE;
}

int SpikeList::AddSpike(short* source_data, const int n_channels, const long ii_time, const int source_channel, const int i_class, const BOOL b_check)
{
	// search spike index of first spike which time is > to present one
	int index_added_spike;
	auto jitter = 0;
	if (b_check)
		jitter = m_jitterSL;
	const auto b_found = IsAnySpikeAround(ii_time, jitter, index_added_spike, source_channel);

	if (!b_found)
	{
		// create spike element and add pointer to array
		auto* se = new Spike(ii_time, source_channel, m_maximum_over_all_spikes, m_minimum_over_all_spikes, 0, i_class, 0);
		ASSERT(se != NULL);
		m_spike_elements.InsertAt(index_added_spike, se);

		if (source_data != nullptr)
		{
			TransferDataToSpikeBuffer(se, source_data, n_channels, TRUE); 
		}
	}
	return index_added_spike;
}

BOOL SpikeList::TransferDataToSpikeBuffer(Spike* pSpike, short* source_data, const int n_channels, const BOOL b_adjust)
{
	// compute avg from m_icenter1SL to m_icenter2SL
	auto lp_b = source_data + n_channels * m_icenter1SL;
	long average_value = 0;
	for (auto i = m_icenter1SL; i <= m_icenter2SL; i++, lp_b += n_channels)
	{
		const int val = *lp_b;
		average_value += val; 
	}

	// search max min
	lp_b = source_data + n_channels * m_imaxmin1SL;
	int max = *lp_b; 
	auto min = max; 
	auto min_index = 0;
	auto max_index = 0;

	for (auto i = m_imaxmin1SL + 1; i <= m_imaxmin2SL; i++)
	{
		const int val = *lp_b;
		if (val > max) 
		{
			max = val; 
			max_index = i;
		}
		else if (val < min) 
		{
			min = val;
			min_index = i;
		}
		lp_b += n_channels;
	}
	const auto i_min_to_max = min_index - max_index;

	// get offset over zero at the requested index
	int offset = 0;
	if (b_adjust)
	{
		if (m_icenter1SL - m_icenter2SL != 0)
		{
			offset = average_value / (m_icenter2SL - m_icenter1SL + 1);
			offset -= pSpike->m_bin_zero;
		}
	}

	// save values computed here within spike element structure

	pSpike->SetSpikeMaxMin(max - offset, min - offset, i_min_to_max);
	pSpike->SetSpikeAmplitudeOffset(offset);
	auto lp_dest = pSpike->GetpSpikeData(GetSpikeLength());
	const auto offset_short = static_cast<short>(offset);
	for (auto i = GetSpikeLength(); i > 0; i--) 
	{
		*lp_dest = *source_data - offset_short; 
		lp_dest++;
		source_data += n_channels; 
	}
	return true;
}

void SpikeList::MeasureSpikeMaxMin(const int index, int* max, int* max_index, int* min, int* min_index) 
{
	auto lp_buffer = GetSpike(index)->GetpSpikeData(GetSpikeLength()); 
	int val = *lp_buffer;
	*max = val; 
	*min = val; 
	*min_index = *max_index = 0;
	for (auto i = 1; i < GetSpikeLength(); i++) 
	{
		lp_buffer++;
		val = *lp_buffer;
		if (val > *max) 
		{
			*max = val;
			*max_index = i;
		}
		else if (val < *min) 
		{
			*min = val;
			*min_index = i;
		}
	}
}

void SpikeList::MeasureSpikeMaxThenMin(const int index, int* max, int* max_index, int* min, int* min_index) 
{
	auto lp_buffer = GetSpike(index)->GetpSpikeData(GetSpikeLength());
	auto lp_buffer_max = lp_buffer;
	int val = *lp_buffer;
	*max = val; 
	*max_index = 0;
	
	for (auto i = 1; i < GetSpikeLength(); i++) 
	{
		lp_buffer++;
		val = *lp_buffer;
		if (val > *max)
		{
			*max = val; 
			*max_index = i;
			lp_buffer_max = lp_buffer;
		}
	}

	lp_buffer = lp_buffer_max;
	*min = *max;
	*min_index = *max_index;
	for (auto i = *max_index; i < GetSpikeLength(); i++) 
	{
		lp_buffer++;
		val = *lp_buffer;
		if (val < *min) 
		{
			*min = val; 
			*min_index = i;
		}
	}
}

void SpikeList::MeasureSpikeMaxMinEx(const int index, int* max, int* max_index, int* min, int* min_index, const int i_first, const int i_last) 
{
	auto lp_buffer = GetSpike(index)->GetpSpikeData(GetSpikeLength());
	lp_buffer += i_first;
	int val = *lp_buffer;
	*max = val; 
	*min = *max; 
	*min_index = *max_index = i_first;
	for (auto i = i_first + 1; i <= i_last; i++) 
	{
		lp_buffer++;
		val = *lp_buffer;
		if (val > *max) 
		{
			*max = val;
			*max_index = i;
		}
		else if (val < *min) 
		{
			*min = val; 
			*min_index = i;
		}
	}
}

void SpikeList::MeasureSpikeMaxThenMinEx(const int index, int* max, int* max_index, int* min, int* min_index, const int i_first, const int i_last) 
{
	auto lp_buffer = GetSpike(index)->GetpSpikeData(GetSpikeLength());
	lp_buffer += i_first; // get pointer to buffer
	auto lp_buffer_max = lp_buffer;
	int val = *lp_buffer;
	*max = val; // assume offset between points = 1 (short)
	*max_index = i_first;

	// first search for max
	for (auto i = i_first + 1; i <= i_last; i++) // loop to scan data
	{
		lp_buffer++;
		val = *lp_buffer;
		if (val > *max) // search max
		{
			*max = val; // change max and max_index
			*max_index = i;
			lp_buffer_max = lp_buffer;
		}
	}

	// search for min
	lp_buffer = lp_buffer_max;
	*min = *max;
	*min_index = *max_index;
	for (int i = *min_index + 1; i <= i_last; i++) // loop to scan data
	{
		lp_buffer++;
		val = *lp_buffer;
		if (val < *min) // search max
		{
			*min = val; // change max and max_index
			*min_index = i;
		}
	}
}

void SpikeList::GetTotalMaxMin(const BOOL b_recalculate, int* max, int* min)
{
	if (b_recalculate || !m_extrema_valid)
	{
		int min1 = 0;
		int max1 = 65535;
		const int n_spikes = m_spike_elements.GetCount();

		for (auto index = 0; index < n_spikes; index++)
		{
			if (GetSpike(index)->get_class() >= 0)
			{
				GetSpikeExtrema(0, &max1, &min1);
				break;
			}
		}
		m_minimum_over_all_spikes = min1;
		m_maximum_over_all_spikes = max1; 
		for (auto index = 0; index < n_spikes; index++)
		{
			if (GetSpike(index)->get_class() >= 0)
			{
				GetSpikeExtrema(index, &max1, &min1);
				if (max1 > m_maximum_over_all_spikes) m_maximum_over_all_spikes = max1;
				if (min1 < m_minimum_over_all_spikes) m_minimum_over_all_spikes = min1;
			}
		}
		m_extrema_valid = TRUE;
	}
	*max = m_maximum_over_all_spikes;
	*min = m_minimum_over_all_spikes;
}

BOOL SpikeList::InitSpikeList(AcqDataDoc* p_data_file, SPKDETECTPARM* pFC)
{
	// remove data from spike list
	EraseData();
	RemoveAllSpikeFlags();

	// copy data from CObArray
	if (pFC != nullptr)
		m_detection_parameters = *pFC;

	auto flag = false;
	if (p_data_file != nullptr)
	{
		const auto wave_format = p_data_file->GetpWaveFormat();
		m_data_encoding_mode = wave_format->mode_encoding;
		m_bin_zero = wave_format->binzero;
		m_sampling_rate = wave_format->sampling_rate_per_channel;
		flag = p_data_file->GetWBVoltsperBin(m_detection_parameters.detectChan, &m_volts_per_bin);
	}

	if (!flag)
	{
		m_extrema_valid = TRUE;
		m_minimum_over_all_spikes = 2048;
		m_maximum_over_all_spikes = 2100;
	}

	// reset buffers, list, spk params
	SetSpikeLength(m_detection_parameters.extractNpoints);

	// reset classes
	m_keep_only_valid_classes = FALSE; // default: no valid array
	m_n_classes = 0;
	m_spike_class_descriptor_array.SetSize(2); // default size - some functions
	m_spike_class_descriptor_array.SetAt(0, SpikeClassDescriptor(0,0)); 
	return TRUE;
}

void SpikeList::EraseData()
{
	delete_arrays();
	m_selected_spike = -1;
}

int SpikeList::SetSpikeFlag(int spike_index, BOOL set_spike_flag)
{
	// set spike flag: add spike to the array
	if (TRUE == set_spike_flag)
	{
		// first look if spike_index is already flagged
		if (!GetSpikeFlag(spike_index))
			m_spikes_flagged.Add(spike_index);
	}

	// remove flag
	else
	{
		// find spike_index within the array and remove it
		auto index = -1;
		for (auto i = m_spikes_flagged.GetCount() - 1; i >= 0; i--)
		{
			if (m_spikes_flagged.GetAt(i) == spike_index)
			{
				index = i;
				break;
			}
		}
		if (index >= 0)
			m_spikes_flagged.RemoveAt(index);
	}
	// return the number of elements within the array
	return GetSpikeFlagArrayCount();
}

int SpikeList::ToggleSpikeFlag(int spike_index)
{
	// find spike within array
	auto index = -1;
	for (auto i = m_spikes_flagged.GetCount() - 1; i >= 0; i--)
	{
		if (m_spikes_flagged.GetAt(i) == spike_index)
		{
			index = i;
			break;
		}
	}

	// if found: remove it
	if (index >= 0)
		m_spikes_flagged.RemoveAt(index);

	// if not found, add it
	else
		m_spikes_flagged.Add(spike_index);

	return GetSpikeFlagArrayCount();
}

void SpikeList::SetSingleSpikeFlag(int spike_index)
{
	if (m_spikes_flagged.GetCount() != 1)
		m_spikes_flagged.SetSize(1);
	m_spikes_flagged.SetAt(0, spike_index);
}

BOOL SpikeList::GetSpikeFlag(int spike_index)
{
	BOOL bFlag = FALSE;
	// search if spike_index is in the list
	for (int i = m_spikes_flagged.GetCount() - 1; i >= 0; i--)
	{
		if (m_spikes_flagged.GetAt(i) == spike_index)
		{
			bFlag = TRUE;
			break;
		}
	}
	return bFlag;
}

void SpikeList::FlagRangeOfSpikes(long l_first, long l_last, BOOL bSet)
{
	// first clear flags of spikes within the flagged array which fall within limits
	long l_time;
	for (auto i = m_spikes_flagged.GetCount() - 1; i >= 0; i--)
	{
		const int spike_index = m_spikes_flagged.GetAt(i);
		l_time = GetSpikeTime(spike_index);
		if (l_time < l_first || l_time > l_last)
			continue;
		m_spikes_flagged.RemoveAt(i);
	}
	// if bSet was set to FALSE, the job is done
	if (bSet == FALSE)
		return;

	// then if bSet is ON, search spike file for spikes falling within this range and flag them
	for (auto i = 0; i < GetTotalSpikes(); i++)
	{
		l_time = GetSpikeTime(i);
		if (l_time < l_first || l_time > l_last)
			continue;
		m_spikes_flagged.Add(i);
	}
}

void SpikeList::SelectSpikesWithinBounds(const int v_min, const int v_max, const long l_first, const long l_last, const BOOL b_add)
{
	if (!b_add)
		RemoveAllSpikeFlags();
	for (auto i = 0; i < GetTotalSpikes(); i++)
	{
		const auto l_time = GetSpikeTime(i);
		if (l_time < l_first || l_time > l_last)
			continue;

		int max, min;
		GetSpikeExtrema(i, &max, &min);
		if (max > v_max) continue;
		if (min < v_min) continue;
		// found within boundaries= remove spike from array
		m_spikes_flagged.Add(i);
	}
}

void SpikeList::GetRangeOfSpikeFlagged(long& l_first, long& l_last)
{
	// no spikes flagged, return dummy values
	if (m_spikes_flagged.GetCount() < 1)
	{
		l_first = -1;
		l_last = -1;
		return;
	}

	// spikes flagged: init max and min to the first spike time
	l_first = GetSpikeTime(m_spikes_flagged.GetAt(0));
	l_last = l_first;

	// search if spike no is in the list
	for (auto i = m_spikes_flagged.GetCount() - 1; i >= 0; i--)
	{
		const auto l_time = GetSpikeTime(m_spikes_flagged.GetAt(i));
		if (l_time < l_first)
			l_first = l_time;
		if (l_time > l_last)
			l_last = l_time;
	}
}

void SpikeList::OffsetSpikeAmplitude(const int index, const int val_first, const int val_last, int center)
{
	auto lp_buffer = GetSpike(index)->GetpSpikeData(GetSpikeLength());
	const auto diff = val_last - val_first; // difference
	int max = *lp_buffer; // compute max/min on the fly
	auto min = max; // provisional max and minimum
	auto max_index = 0;
	auto min_index = val_last - val_first;
	for (auto i = 0; i < GetSpikeLength(); i++) // loop to scan data
	{
		// offset point i
		*lp_buffer += static_cast<short>(val_first + MulDiv(diff, i, GetSpikeLength()));
		if (*lp_buffer > max) // new max?
		{
			max = *lp_buffer;
			max_index = i;
		}
		else if (*lp_buffer < min) // new min?
		{
			min = *lp_buffer;
			min_index = i;
		}
		lp_buffer++; // update pointer
	}
	const auto d_max_to_min = min_index - max_index; // assume that min comes after max in a "normal" spike

	auto offset = m_spike_elements[index]->get_amplitude_offset();
	offset -= val_first; // change spike offset
	m_spike_elements[index]->SetSpikeAmplitudeOffset(offset);
	m_extrema_valid = FALSE;
	m_spike_elements[index]->SetSpikeMaxMin(max, min, d_max_to_min);
}

void SpikeList::CenterSpikeAmplitude(const int spike_index, const int i_first, const int i_last, const WORD method)
{
	auto lp_buffer = GetSpike(spike_index)->GetpSpikeData(GetSpikeLength());
	lp_buffer += i_first;
	int val_first; // contains offset
	auto d_max_to_min = 0;
	int max, min;
	const auto p_se = GetSpike(spike_index);
	p_se->GetSpikeMaxMin(&max, &min, &d_max_to_min);

	switch (method)
	{
	case 0: // ........................ center (max + min )/2
		{
			auto min_index = 0;
			auto max_index = 0;
			max = *lp_buffer; // compute max/min on the fly
			min = max; // provisional max and minimum
			for (auto i = i_first; i < i_last; i++, lp_buffer++)
			{
				// offset point i
				if (*lp_buffer > max) // new max?
				{
					max = *lp_buffer;
					max_index = i;
				}
				else if (*lp_buffer < min) // new min?
				{
					min = *lp_buffer;
					min_index = i;
				}
			}
			val_first = (max + min) / 2 - GetAcqBinzero();
			d_max_to_min = min_index - max_index;
		}
		break;
	case 1: // ........................ center average
		{
			long sum_value = 0;
			const auto offset_value = GetAcqBinzero();
			for (auto i = i_first; i < i_last; i++, lp_buffer++)
				sum_value += (*lp_buffer - offset_value);
			val_first = static_cast<int>(sum_value / static_cast<long>(i_last - i_first + 1));
		}
		break;
	default:
		val_first = 0;
		break;
	}

	// change spike offset
	p_se->SetSpikeAmplitudeOffset(p_se->get_amplitude_offset() + val_first);
	// then offset data (max & min ibidem)
	p_se->SetSpikeMaxMin(max - val_first, min - val_first, d_max_to_min);
	lp_buffer = GetSpike(spike_index)->GetpSpikeData(GetSpikeLength());
	for (auto i = 0; i < GetSpikeLength(); i++, lp_buffer++)
		*lp_buffer -= static_cast<short>(val_first);
	m_extrema_valid = FALSE;
}

long SpikeList::UpdateClassList()
{
	const auto n_spikes = GetTotalSpikes();
	m_n_classes = 1; // erase nb of classes
	m_spike_class_descriptor_array.SetSize(0); // erase array
	m_keep_only_valid_classes = TRUE; // class list OK when exit
	if (n_spikes == 0)
	{
		// no spikes? dummy parameters
		m_spike_class_descriptor_array.Add(SpikeClassDescriptor(0, 0)); 
		return 0L; // done
	}

	m_spike_class_descriptor_array.Add(SpikeClassDescriptor(GetSpike(0)->get_class(), 1));
	m_n_classes = 1; // total nb of classes

	// loop over all spikes of the list
	for (auto i = 1; i < n_spikes; i++)
	{
		const auto spike_class = GetSpike(i)->get_class(); // class of spike i
		auto array_class = 0;

		// loop over all existing classes to find if there is one
		for (auto j = 0; j < m_n_classes; j++)
		{
			array_class = GetclassID(j); // class ID
			if (spike_class == array_class) // class OK?
			{
				m_spike_class_descriptor_array.GetAt(j).n_items++; // increment spike count
				break; // exit spk class loop
			}
			if (spike_class < array_class) // insert new class?
			{
				m_spike_class_descriptor_array.InsertAt(j , SpikeClassDescriptor(spike_class, 1)); 
				m_n_classes++; // exit spk class loop
				break;
			}
		}

		// test if cla not found within array
		if (spike_class > array_class)
		{
			m_spike_class_descriptor_array.Add(SpikeClassDescriptor(spike_class, 1)); 
			m_n_classes++; // update nb classes
		}
	}
	return m_n_classes;
}

void SpikeList::ChangeSpikeClassID(const int old_class_ID, const int new_class_ID)
{
	// first find valid max and min
	for (auto index = 0; index < m_spike_elements.GetSize(); index++)
	{
		if (GetSpike(index)->get_class() == old_class_ID)
			SetSpikeClass(index, new_class_ID);
	}
}

void SpikeList::Measure_case0_AmplitudeMinToMax(const int t1, const int t2)
{
	const auto n_spikes = GetTotalSpikes();
	for (auto spike_index = 0; spike_index < n_spikes; spike_index++)
	{
		const auto spike_element = GetSpike(spike_index);
		auto lp_buffer = GetSpike(spike_index)->GetpSpikeData(GetSpikeLength());
		lp_buffer += t1;
		int val = *lp_buffer;
		auto max = val;
		auto min = val;
		auto min_index = t1;
		auto max_index = t1;
		for (auto i = t1 + 1; i <= t2; i++)
		{
			lp_buffer++;
			val = *lp_buffer;
			if (val > max)
			{
				max = val;
				max_index = i;
			}
			else if (val < min)
			{
				min = val;
				min_index = i;
			}
		}

		spike_element->SetSpikeMaxMin(max, min, min_index - max_index);
		spike_element->set_y1(max - min);
	}
}

void SpikeList::Measure_case1_AmplitudeAtT(const int t)
{
	const auto n_spikes = GetTotalSpikes();

	for (auto spike_index = 0; spike_index < n_spikes; spike_index++)
	{
		const auto spike_element = GetSpike(spike_index);
		auto lp_buffer = GetSpike(spike_index)->GetpSpikeData(GetSpikeLength());
		lp_buffer += t;
		const int val = *lp_buffer;
		spike_element->set_y1(val);
	}
}

void SpikeList::Measure_case2_AmplitudeAtT2MinusAtT1(const int t1, const int t2)
{
	const auto n_spikes = GetTotalSpikes();

	for (auto spike_index = 0; spike_index < n_spikes; spike_index++)
	{
		const auto spike_element = GetSpike(spike_index);
		const auto lp_buffer = GetSpike(spike_index)->GetpSpikeData(GetSpikeLength());
		const int val1 = *(lp_buffer + t1);
		const int val2 = *(lp_buffer + t2);
		spike_element->set_y1(val2 - val1);
	}
}

CSize SpikeList::Measure_Y1_MaxMin()
{
	const auto n_spikes = GetTotalSpikes();
	int max = GetSpike(0)->get_y1();
	int min = max;
	for (auto spike_index = 0; spike_index < n_spikes; spike_index++)
	{
		const auto val = GetSpike(spike_index)->get_y1();
		if (val > max) max = val;
		if (val < min) min = val;
	}

	return {max, min};
}

BOOL SpikeList::SortSpikeWithY1(const CSize from_class_ID_to_class_ID, const CSize time_bounds, const CSize limits)
{
	const auto n_spikes = GetTotalSpikes();

	const auto from_class = from_class_ID_to_class_ID.cx;
	const auto to_class = from_class_ID_to_class_ID.cy;
	const auto first = time_bounds.cx;
	const auto last = time_bounds.cy;
	const int upper = limits.cy;
	const int lower = limits.cx;
	BOOL b_changed = false;

	for (auto spike_index = 0; spike_index < n_spikes; spike_index++)
	{
		const auto spike_element = GetSpike(spike_index);
		if (spike_element->get_class() != from_class)
			continue;
		const auto ii_time = spike_element->get_time();
		if (ii_time < first || ii_time > last)
			continue;
		const auto value = spike_element->get_y1();
		if (value >= lower && value <= upper)
		{
			spike_element->set_class(to_class);
			b_changed = true;
		}
	}

	return b_changed;
}

BOOL SpikeList::SortSpikeWithY1AndY2(const CSize from_class_ID_to_class_ID, const CSize time_bounds, const CSize limits1,
                                      const CSize limits2)
{
	const auto n_spikes = GetTotalSpikes();

	const auto from_class = from_class_ID_to_class_ID.cx;
	const auto to_class = from_class_ID_to_class_ID.cy;
	const auto first = time_bounds.cx;
	const auto last = time_bounds.cy;
	const int upper1 = limits1.cy;
	const int lower1 = limits1.cx;
	const int upper2 = limits2.cy;
	const int lower2 = limits2.cx;
	BOOL b_changed = false;

	for (auto spike_index = 0; spike_index < n_spikes; spike_index++)
	{
		const auto spike_element = GetSpike(spike_index);
		if (spike_element->get_class() != from_class)
			continue;
		const auto ii_time = spike_element->get_time();
		if (ii_time < first || ii_time > last)
			continue;
		const auto value1 = spike_element->get_y1();
		const auto value2 = spike_element->get_y2();
		if ((value1 >= lower1 && value1 <= upper1) && (value2 >= lower2 && value2 <= upper2))
		{
			spike_element->set_class(to_class);
			b_changed = true;
		}
	}

	return b_changed;
}

int SpikeList::GetValidSpikeNumber(int spike_index) const
{
	if (spike_index < 0)
		spike_index = -1;
	if (spike_index >= GetTotalSpikes())
		spike_index = GetTotalSpikes() - 1;
	return spike_index;
}

int SpikeList::GetNextSpike(int spike_index, int delta, BOOL b_keep_same_class) 
{
	const int spike_index_old = spike_index;
	int class_ID_old = 0;
	if (spike_index_old >= 0 && spike_index_old < GetTotalSpikes() - 1)
		class_ID_old = GetSpike(spike_index)->get_class();
	if (delta >= 0)
		delta = 1;
	else
		delta = -1;
	if (b_keep_same_class)
	{
		do
		{
			spike_index += delta;
			if (spike_index < 0 || spike_index >= GetTotalSpikes())
			{
				spike_index = spike_index_old;
				break;
			}
		}
		while (spike_index < GetTotalSpikes() && GetSpike(spike_index)->get_class() != class_ID_old);
	}
	else
		spike_index += delta;

	if (spike_index >= GetTotalSpikes())
		spike_index = spike_index_old;
	return GetValidSpikeNumber(spike_index);
}
