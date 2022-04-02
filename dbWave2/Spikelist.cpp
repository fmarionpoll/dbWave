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
	m_spike_class_descriptors.SetSize(1);
	m_spike_class_descriptors.Add(SpikeClassDescriptor(0,0));
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
	auto n_spikes = m_spikes.GetSize();
	auto n_spikes_ok = n_spikes;

	for (auto i = n_spikes - 1; i >= 0; i--)
	{
		if (m_spikes.GetAt(i)->get_class() < 0)
		{
			const auto se = m_spikes.GetAt(i);
			delete se;
			m_spikes.RemoveAt(i);
			n_spikes_ok--;
		}
	}
	n_spikes = m_spikes.GetSize();
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
		const auto n_spikes = m_spikes.GetSize();
		ar << static_cast<WORD>(n_spikes);
		for (auto i = 0; i < n_spikes; i++)
			m_spikes.GetAt(i)->Serialize(ar);
	}
	else
	{
		WORD w1; ar >> w1; const int n_spikes = w1;
		m_spikes.SetSize(n_spikes);
		for (auto i = 0; i < n_spikes; i++)
		{
			auto* se = new(Spike);
			ASSERT(se != NULL);
			se->Serialize(ar);
			m_spikes.SetAt(i, se);
		}
	}
}

void SpikeList::serialize_spike_data(CArchive& ar) 
{
	const auto n_spikes = m_spikes.GetSize();
	if (ar.IsStoring())
	{
		const WORD spike_length = static_cast<WORD>(GetSpikeLength());
		ar << spike_length;

		const auto n_bytes = spike_length * sizeof(short);
		if (n_bytes > 0)
		{
			for (auto i = 0; i < n_spikes; i++)
			{
				ar.Write(GetSpike(i)->GetpData(spike_length), n_bytes);
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
			ar.Read(GetSpike(i)->GetpData(spike_length), n_bytes);
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
			SpikeClassDescriptor item = m_spike_class_descriptors.GetAt(i);
			if (item.n_items > 0 && !(m_keep_only_valid_classes && item.id_number < 0))
				item.Serialize(ar);
		}
	}
	else
	{
		m_extrema_valid = FALSE;
		m_spike_class_descriptors.SetSize(1);
		m_spike_class_descriptors.SetAt(0, SpikeClassDescriptor(0, 0));
		long dummy;
		ar >> dummy; m_keep_only_valid_classes = dummy;
		ar >> dummy; m_n_classes = dummy;
		
		if (m_n_classes >= 0)
		{
			m_spike_class_descriptors.SetSize(m_n_classes);
			for (auto i = 0; i < m_n_classes; i++)
			{
				SpikeClassDescriptor item = m_spike_class_descriptors.GetAt(i);
				item.Serialize(ar);
				m_spike_class_descriptors.SetAt(i, item);
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
	m_spikes.SetSize(n_spikes); 
	for (auto i = 0; i < n_spikes; i++) 
	{
		auto* se = new(Spike);
		ASSERT(se != NULL);
		se->read_version0(ar);
		m_spikes[i] = se;
	}

	serialize_spike_data(ar);

	m_extrema_valid = FALSE;
	m_keep_only_valid_classes = FALSE; 
	m_n_classes = 1;
	m_spike_class_descriptors.SetSize(1);
	m_spike_class_descriptors.SetAt(0, SpikeClassDescriptor(0, 0)); 
	long dummy;
	ar >> dummy;
	m_keep_only_valid_classes = dummy;
	if (m_keep_only_valid_classes) 
	{
		ar >> dummy;
		m_n_classes = dummy;
		if (m_n_classes != 0)
		{
			m_spike_class_descriptors.SetSize(m_n_classes);
			for (int i = 0; i < m_n_classes; i++)
			{
				int dummy1, dummy2;
				ar >> dummy1;
				ar >> dummy2;
				m_spike_class_descriptors.SetAt(i, SpikeClassDescriptor(dummy1, dummy2));
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
	m_spikes.SetSize(n_spikes); 
	for (auto i = 0; i < n_spikes; i++)
	{
		const auto se = new(Spike);
		ASSERT(se != NULL);
		se->read_version0(ar); 
		m_spikes[i] = se;
	}

	// ----------------------------------------------------
	// (4) load spike raw data
	ar >> w1; 
	SetSpikeLength(w1); 
	m_bin_zero = GetAcqBinzero();

	// loop through all data buffers
	const auto n_bytes = w1 * sizeof(short);// *n_spikes;
	for (int i = 0; i < n_spikes; i++)
	{
		const auto lp_dest = GetSpike(i)->GetpData(GetSpikeLength());
		ar.Read(lp_dest, n_bytes); // read data from disk
	}
	
	m_extrema_valid = FALSE;

	// ----------------------------------------------------
	// (5) load class array if valid flag
	// reset elements of the list
	m_keep_only_valid_classes = FALSE; // default: no valid array
	m_n_classes = 1;
	m_spike_class_descriptors.SetSize(2); // default size - some functions
	m_spike_class_descriptors.SetAt(0, SpikeClassDescriptor(0, 0)); 

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
			m_spike_class_descriptors.SetSize(m_n_classes);
			for (int i = 0; i < m_n_classes; i++)
			{
				int dummy1, dummy2;
				ar >> dummy1;
				ar >> dummy2;
				m_spike_class_descriptors.SetAt(i, SpikeClassDescriptor(dummy1, dummy2));
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
	const int n_spikes = m_spikes.GetSize();
	if (n_spikes > 0)
	{
		for (auto i = n_spikes - 1; i >= 0; i--)
			delete m_spikes.GetAt(i);
		m_spikes.RemoveAll();
	}
	m_spike_class_descriptors.RemoveAll();
}

int SpikeList::RemoveSpike(int spike_index)
{
	if (spike_index <= m_spikes.GetUpperBound())
	{
		const auto element = m_spikes.GetAt(spike_index);
		delete element;
		m_spikes.RemoveAt(spike_index);
	}
	return m_spikes.GetSize();
}

BOOL SpikeList::IsAnySpikeAround(const long ii_time, const int jitter, int& spike_index, const int channel_index)
{
	// search spike index of first spike which time is > to present one
	spike_index = 0;
	if (m_spikes.GetSize() > 0)
	{
		// loop to find position of the new spike
		for (auto j = m_spikes.GetUpperBound(); j >= 0; j--)
		{
			// assumed ordered list
			if (ii_time >= m_spikes[j]->get_time())
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
		delta_time = m_spikes[spike_index - 1]->get_time() - ii_time;
		if (labs(delta_time) <= jitter) 
		{
			spike_index--; 
			return TRUE;
		}
	}

	if (spike_index <= m_spikes.GetUpperBound()) 
	{
		delta_time = m_spikes[spike_index]->get_time() - ii_time;
		const auto i_channel_2 = m_spikes[spike_index]->get_source_channel();
		if (i_channel_2 == channel_index && labs(delta_time) <= jitter) 
			return TRUE; 
	}

	return FALSE;
}

int SpikeList::AddSpike(short* source_data, const int n_channels, const long ii_time, const int source_channel, const int i_class, const BOOL b_check)
{
	// search spike index of first spike for which time is > to present one
	int index_added_spike;
	auto jitter = 0;
	if (b_check)
		jitter = m_jitterSL;
	const auto b_found = IsAnySpikeAround(ii_time, jitter, index_added_spike, source_channel);

	if (!b_found)
	{
		auto* se = new Spike(ii_time, source_channel, m_maximum_over_all_spikes, m_minimum_over_all_spikes, 0, i_class, 0);
		ASSERT(se != NULL);
		m_spikes.InsertAt(index_added_spike, se);

		if (source_data != nullptr)
		{
			se->TransferDataToSpikeBuffer(source_data, n_channels);
			// compute max min between brackets for new spike
			int max, min, i_max, i_min;
			se->MeasureMaxMinEx(&max, &i_max, &min, &i_min, m_imaxmin1SL, m_imaxmin2SL);
			se->SetMaxMinEx(max, min, i_min - i_max);
		}
	}
	return index_added_spike;
}

void SpikeList::GetTotalMaxMin(const BOOL b_recalculate, int* max, int* min)
{
	if (b_recalculate || !m_extrema_valid)
	{
		int min1 = 0;
		int max1 = 65535;
		const int n_spikes = m_spikes.GetCount();

		for (auto index = 0; index < n_spikes; index++)
		{
			if (GetSpike(index)->get_class() >= 0)
			{
				GetSpike(0)->GetMaxMin(&max1, &min1);
				break;
			}
		}
		m_minimum_over_all_spikes = min1;
		m_maximum_over_all_spikes = max1; 
		for (auto index = 0; index < n_spikes; index++)
		{
			if (GetSpike(index)->get_class() >= 0)
			{
				GetSpike(index)->GetMaxMin(&max1, &min1);
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
	m_spike_class_descriptors.SetSize(2); // default size - some functions
	m_spike_class_descriptors.SetAt(0, SpikeClassDescriptor(0,0)); 
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
			m_index_flagged_spikes.Add(spike_index);
	}

	// remove flag
	else
	{
		// find spike_index within the array and remove it
		auto index = -1;
		for (auto i = m_index_flagged_spikes.GetCount() - 1; i >= 0; i--)
		{
			if (m_index_flagged_spikes.GetAt(i) == spike_index)
			{
				index = i;
				break;
			}
		}
		if (index >= 0)
			m_index_flagged_spikes.RemoveAt(index);
	}
	// return the number of elements within the array
	return GetSpikeFlagArrayCount();
}

int SpikeList::ToggleSpikeFlag(int spike_index)
{
	// find spike within array
	auto index = -1;
	for (auto i = m_index_flagged_spikes.GetCount() - 1; i >= 0; i--)
	{
		if (m_index_flagged_spikes.GetAt(i) == spike_index)
		{
			index = i;
			break;
		}
	}

	// if found: remove it
	if (index >= 0)
		m_index_flagged_spikes.RemoveAt(index);

	// if not found, add it
	else
		m_index_flagged_spikes.Add(spike_index);

	return GetSpikeFlagArrayCount();
}

void SpikeList::SetSingleSpikeFlag(int spike_index)
{
	if (m_index_flagged_spikes.GetCount() != 1)
		m_index_flagged_spikes.SetSize(1);
	m_index_flagged_spikes.SetAt(0, spike_index);
}

BOOL SpikeList::GetSpikeFlag(int spike_index)
{
	BOOL bFlag = FALSE;
	// search if spike_index is in the list
	for (int i = m_index_flagged_spikes.GetCount() - 1; i >= 0; i--)
	{
		if (m_index_flagged_spikes.GetAt(i) == spike_index)
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
	for (auto i = m_index_flagged_spikes.GetCount() - 1; i >= 0; i--)
	{
		const int spike_index = m_index_flagged_spikes.GetAt(i);
		l_time = GetSpike(spike_index)->get_time();
		if (l_time < l_first || l_time > l_last)
			continue;
		m_index_flagged_spikes.RemoveAt(i);
	}
	// if bSet was set to FALSE, the job is done
	if (bSet == FALSE)
		return;

	// then if bSet is ON, search spike file for spikes falling within this range and flag them
	for (auto i = 0; i < GetTotalSpikes(); i++)
	{
		l_time = GetSpike(i)->get_time();
		if (l_time < l_first || l_time > l_last)
			continue;
		m_index_flagged_spikes.Add(i);
	}
}

void SpikeList::SelectSpikesWithinBounds(const int v_min, const int v_max, const long l_first, const long l_last, const BOOL b_add)
{
	if (!b_add)
		RemoveAllSpikeFlags();
	for (auto i = 0; i < GetTotalSpikes(); i++)
	{
		const auto l_time = GetSpike(i)->get_time();
		if (l_time < l_first || l_time > l_last)
			continue;

		int max, min;
		GetSpike(i)->GetMaxMin(&max, &min);
		if (max > v_max) continue;
		if (min < v_min) continue;
		// found within boundaries= remove spike from array
		m_index_flagged_spikes.Add(i);
	}
}

void SpikeList::GetRangeOfSpikeFlagged(long& l_first, long& l_last)
{
	// no spikes flagged, return dummy values
	if (m_index_flagged_spikes.GetCount() < 1)
	{
		l_first = -1;
		l_last = -1;
		return;
	}

	// spikes flagged: init max and min to the first spike time
	l_first = GetSpike(m_index_flagged_spikes.GetAt(0))->get_time();
	l_last = l_first;

	// search if spike no is in the list
	for (auto i = m_index_flagged_spikes.GetCount() - 1; i >= 0; i--)
	{
		const auto l_time = GetSpike(m_index_flagged_spikes.GetAt(i))->get_time();
		if (l_time < l_first)
			l_first = l_time;
		if (l_time > l_last)
			l_last = l_time;
	}
}

long SpikeList::UpdateClassList()
{
	const auto n_spikes = GetTotalSpikes();
	m_n_classes = 1; // erase nb of classes
	m_spike_class_descriptors.SetSize(0); // erase array
	m_keep_only_valid_classes = TRUE; // class list OK when exit
	if (n_spikes == 0)
	{
		// no spikes? dummy parameters
		m_spike_class_descriptors.Add(SpikeClassDescriptor(0, 0)); 
		return 0L; // done
	}

	m_spike_class_descriptors.Add(SpikeClassDescriptor(GetSpike(0)->get_class(), 1));
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
				m_spike_class_descriptors.GetAt(j).n_items++; // increment spike count
				break; // exit spk class loop
			}
			if (spike_class < array_class) // insert new class?
			{
				m_spike_class_descriptors.InsertAt(j , SpikeClassDescriptor(spike_class, 1)); 
				m_n_classes++; // exit spk class loop
				break;
			}
		}

		// test if cla not found within array
		if (spike_class > array_class)
		{
			m_spike_class_descriptors.Add(SpikeClassDescriptor(spike_class, 1)); 
			m_n_classes++; // update nb classes
		}
	}
	return m_n_classes;
}

void SpikeList::ChangeSpikeClassID(const int old_class_ID, const int new_class_ID)
{
	// first find valid max and min
	for (auto index = 0; index < m_spikes.GetSize(); index++)
	{
		if (GetSpike(index)->get_class() == old_class_ID)
			GetSpike(index)->set_class(new_class_ID);
	}
}

void SpikeList::Measure_case0_AmplitudeMinToMax(const int t1, const int t2)
{
	const auto n_spikes = GetTotalSpikes();
	for (auto spike_index = 0; spike_index < n_spikes; spike_index++)
	{
		const auto spike_element = GetSpike(spike_index);
		auto lp_buffer = GetSpike(spike_index)->GetpData(GetSpikeLength());
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

		spike_element->SetMaxMinEx(max, min, min_index - max_index);
		spike_element->set_y1(max - min);
	}
}

void SpikeList::Measure_case1_AmplitudeAtT(const int t)
{
	const auto n_spikes = GetTotalSpikes();

	for (auto spike_index = 0; spike_index < n_spikes; spike_index++)
	{
		const auto spike_element = GetSpike(spike_index);
		auto lp_buffer = GetSpike(spike_index)->GetpData(GetSpikeLength());
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
		const auto lp_buffer = GetSpike(spike_index)->GetpData(GetSpikeLength());
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
