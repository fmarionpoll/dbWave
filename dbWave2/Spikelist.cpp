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
		write_file_version6(ar);
	}
	else
	{
		delete_arrays();
		CString cs_id;
		ar >> cs_id;
		if (cs_id != ID_string)
			read_file_version1(ar);
		else
		{
			WORD version;
			ar >> version;
			if (version == 6)
				read_file_version6(ar);
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
	}
}

void SpikeList::read_file_version1(CArchive& ar)
{
	m_icenter1SL = 0;
	m_icenter2SL = m_detection_parameters.prethreshold;
	m_imaxmin1SL = m_icenter2SL;
	m_imaxmin2SL = m_spike_buffer.GetSpikeLength() - 1;
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
			m_spike_buffer.DeleteSpike(i);
			n_spikes_ok--;
		}
	}
	n_spikes = m_spike_elements.GetSize();
	ASSERT(n_spikes_ok == n_spikes);
	UpdateClassList();
}

void SpikeList::write_file_version6(CArchive& ar)
{
	ar << ID_string;
	ar << m_version; 

	serialize_data_parameters(ar);
	serialize_spike_elements(ar);
	serialize_spike_data(ar);
	serialize_spike_class_descriptors(ar);
	serialize_additional_data(ar);

	ar.Flush(); 
}

void SpikeList::read_file_version6(CArchive& ar)
{
	if (!serialize_data_parameters(ar)) return;
	if (serialize_spike_elements(ar)) return;
	if (serialize_spike_data(ar)) return;
	if (serialize_spike_class_descriptors(ar)) return;
	serialize_additional_data(ar);
}

bool SpikeList::serialize_data_parameters(CArchive& ar)
{
	try
	{
		if (ar.IsStoring())
		{
			ar << m_data_encoding_mode;
			ar << m_bin_zero;
			ar << m_sampling_rate;
			ar << m_volts_per_bin;
			ar << m_spike_channel_description;

			m_detection_parameters.Serialize(ar);
			m_acquisition_channel.Serialize(ar);
		}
		else
		{
			ar >> m_data_encoding_mode;
			ar >> m_bin_zero;
			ar >> m_sampling_rate;
			ar >> m_volts_per_bin;
			ar >> m_spike_channel_description;

			m_detection_parameters.Serialize(ar);
			m_acquisition_channel.Serialize(ar);
		}
	}
	catch (CArchiveException& e)
	{
		if (e.m_cause == CArchiveException::endOfFile)
		{
			TRACE("EOF data parameters\n");
			e.Delete();
		}
		else
		{
			THROW_LAST();
		}
		return false;
	}
	return true;
}

bool SpikeList::serialize_spike_elements(CArchive& ar)
{
	try
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
			WORD w1;
			ar >> w1;
			const int n_spikes = w1;
			m_spike_elements.SetSize(n_spikes);
			for (auto i = 0; i < n_spikes; i++)
			{
				auto* se = new(SpikeElement);
				ASSERT(se != NULL);
				se->Serialize(ar);
				m_spike_elements[i] = se;
			}
		}
	}
	catch (CArchiveException& e)
	{
		if (e.m_cause == CArchiveException::endOfFile)
		{
			TRACE("EOF spike elements\n");
			e.Delete();
		}
		else
		{
			THROW_LAST();
		}
		return false;
	}
	return true;
}

bool SpikeList::serialize_spike_data(CArchive& ar) 
{
	try 
	{
		const auto n_spikes = m_spike_elements.GetSize();
		if (ar.IsStoring())
		{
			const auto spike_length = m_spike_buffer.GetSpikeLength();
			ar << spike_length;

			const auto n_bytes = spike_length * sizeof(short);
			if (n_bytes > 0)
			{
				for (auto i = 0; i < n_spikes; i++)
					ar.Write(m_spike_buffer.GetSpike(i), n_bytes);
			}
		}
		else
		{
			WORD spike_length = 1;
			ar >> spike_length;

			m_spike_buffer.DeleteAllSpikes();
			m_spike_buffer.SetSpikeLength(spike_length);
			m_spike_buffer.m_bin_zero = GetAcqBinzero();

			const auto n_bytes = spike_length * sizeof(short) * n_spikes;
			const auto lp_dest = m_spike_buffer.AllocateSpaceForSeveralSpikes(n_spikes);
			ar.Read(lp_dest, n_bytes);
		}
	}
	catch (CArchiveException& e)
	{
		if (e.m_cause == CArchiveException::endOfFile)
		{
			TRACE("EOF spike data\n");
			e.Delete();
		}
		else
		{
			THROW_LAST();
		}
		return false;
	}
	return true;
}

bool SpikeList::serialize_spike_class_descriptors(CArchive& ar)
{
	try 
	{
		if (ar.IsStoring())
		{
			m_only_valid_classes = FALSE;
			ar << static_cast<long>(m_only_valid_classes);
			ar << static_cast<long>(m_n_classes);
			for (auto i = 0; i < m_n_classes; i++)
			{
				SpikeClassDescriptor item = m_spike_class_descriptor_array.GetAt(i);
				if (item.n_items > 0 && !(m_only_valid_classes && item.id_number < 0))
					item.Serialize(ar);
			}
		}
		else
		{
			m_extrema_valid = FALSE;
			m_only_valid_classes = FALSE;
			m_n_classes = 1;
			m_spike_class_descriptor_array.SetSize(1);
			m_spike_class_descriptor_array.SetAt(0, SpikeClassDescriptor(0, 0));
			long dummy;
			ar >> dummy;
			m_only_valid_classes = dummy;
			ar >> dummy;
			m_n_classes = dummy;
			
			if (m_n_classes >= 0)
			{
				for (auto i = 0; i < m_n_classes; i++)
				{
					SpikeClassDescriptor item;
					item.Serialize(ar);
					m_spike_class_descriptor_array.Add(item);
				}
			}
		}
	}
	catch (CArchiveException& e)
	{
		if (e.m_cause == CArchiveException::endOfFile)
		{
			TRACE("EOF class descriptors\n");
			e.Delete();
		}
		else
		{
			THROW_LAST();
		}
		return false;
	}
	return true;
}

bool SpikeList::serialize_additional_data(CArchive& ar)
{
	try 
	{
		if (ar.IsStoring())
		{
			constexpr auto n_parameters = 4;
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
	catch (CArchiveException& e)
	{
		if (e.m_cause == CArchiveException::endOfFile)
		{
			TRACE("EOF additional data\n");
			e.Delete();
		}
		else
		{
			THROW_LAST();
		}
		return false;
	}
	return true;
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
		auto* se = new(SpikeElement);
		ASSERT(se != NULL);
		se->Read0(ar); // read spike element before version 6
		m_spike_elements[i] = se;
	}

	serialize_spike_data(ar);

	m_extrema_valid = FALSE;
	m_only_valid_classes = FALSE; 
	m_n_classes = 1;
	m_spike_class_descriptor_array.SetSize(1);
	m_spike_class_descriptor_array.SetAt(0, SpikeClassDescriptor(0, 0)); 
	long dummy;
	ar >> dummy;
	m_only_valid_classes = dummy;
	if (m_only_valid_classes) 
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
		auto se = new(SpikeElement);
		ASSERT(se != NULL);
		se->Read0(ar); // read spike element before version 1
		m_spike_elements[i] = se;
	}

	// ----------------------------------------------------
	// (4) load spike raw data
	m_spike_buffer.DeleteAllSpikes(); // delete data buffers
	ar >> w1; // read spike length
	m_spike_buffer.SetSpikeLength(w1); // reset parms/buffer
	m_spike_buffer.m_bin_zero = GetAcqBinzero();

	// loop through all data buffers
	const auto nbytes = w1 * sizeof(short) * n_spikes;
	auto lp_dest = m_spike_buffer.AllocateSpaceForSeveralSpikes(n_spikes);
	ar.Read(lp_dest, nbytes); // read data from disk
	m_extrema_valid = FALSE;

	// ----------------------------------------------------
	// (5) load class array if valid flag
	// reset elements of the list
	m_only_valid_classes = FALSE; // default: no valid array
	m_n_classes = 1;
	m_spike_class_descriptor_array.SetSize(2); // default size - some functions
	m_spike_class_descriptor_array.SetAt(0, SpikeClassDescriptor(0, 0)); 

	// load flag and load elements only if valid
	long dummy;
	ar >> dummy;
	m_only_valid_classes = dummy;
	if (m_only_valid_classes) // read class list
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
		int nparms;
		ar >> nparms;
		ar >> m_icenter1SL;
		nparms--;
		ar >> m_icenter2SL;
		nparms--;
		ar >> m_imaxmin1SL;
		nparms--;
		ar >> m_imaxmin2SL;
		nparms--;
	}
	if (version < 3)
	{
		m_icenter1SL = 0;
		m_icenter2SL = m_detection_parameters.prethreshold;
		m_imaxmin1SL = m_icenter2SL;
		m_imaxmin2SL = m_spike_buffer.GetSpikeLength() - 1;
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

int SpikeList::RemoveSpike(int spikeindex)
{
	if (spikeindex <= m_spike_elements.GetUpperBound())
	{
		const auto pspk_element = m_spike_elements.GetAt(spikeindex);
		delete pspk_element;
		m_spike_elements.RemoveAt(spikeindex);
	}
	return m_spike_elements.GetSize();
}

BOOL SpikeList::IsAnySpikeAround(long ii_time, int jitter, int& jspk, int ichan)
{
	// search spike index of first spike which time is > to present one
	jspk = 0;
	if (m_spike_elements.GetSize() > 0)
	{
		// loop to find position of the new spike
		for (auto j = m_spike_elements.GetUpperBound(); j >= 0; j--)
		{
			// assumed ordered list
			if (ii_time >= m_spike_elements[j]->get_time())
			{
				jspk = j + 1; 
				break; 
			}
		}
	}

	// check if no duplicate otherwise exit immediately
	long deltatime; 
	if (jspk > 0) 
	{
		deltatime = m_spike_elements[jspk - 1]->get_time() - ii_time;
		if (labs(deltatime) <= jitter) // allow a detection jitter of 2 (?)
		{
			jspk--; 
			return TRUE;
		}
	}

	if (jspk <= m_spike_elements.GetUpperBound()) // deal only with case of jspk within the list
	{
		deltatime = m_spike_elements[jspk]->get_time() - ii_time;
		const auto ichan2 = m_spike_elements[jspk]->get_source_channel();
		if (ichan2 == ichan && labs(deltatime) <= jitter) // allow a detection jitter of 2 (?)
			return TRUE; 
	}

	return FALSE;
}

int SpikeList::AddSpike(short* lp_source, const int nchans, const long ii_time, const int sourcechan, const int iclass, const BOOL b_check)
{
	// search spike index of first spike which time is > to present one
	int jspk;
	auto jitter = 0;
	if (b_check)
		jitter = m_jitterSL;
	const auto b_found = IsAnySpikeAround(ii_time, jitter, jspk, sourcechan);

	if (!b_found)
	{
		// create spike element and add pointer to array
		auto* se = new SpikeElement(ii_time, sourcechan, m_maximum_over_all_spikes, m_minimum_over_all_spikes, 0, iclass, 0);
		ASSERT(se != NULL);
		m_spike_elements.InsertAt(jspk, se);

		if (lp_source != nullptr)
		{
			m_spike_buffer.AllocateSpaceForSpikeAt(jspk); 
			TransferDataToSpikeBuffer(jspk, lp_source, nchans, TRUE); 
		}
	}
	return jspk;
}

BOOL SpikeList::TransferDataToSpikeBuffer(const int no, short* lp_source, const int nchans, const BOOL b_adjust)
{
	if (no < 0 || no >= m_spike_elements.GetSize())
		return FALSE;

	// compute avg from m_icenter1SL to m_icenter2SL
	auto lp_b = lp_source + nchans * m_icenter1SL;
	long lavg = 0; // average
	for (auto i = m_icenter1SL; i <= m_icenter2SL; i++, lp_b += nchans)
	{
		const int val = *lp_b;
		lavg += val; 
	}

	// search max min
	lp_b = lp_source + nchans * m_imaxmin1SL;
	int max = *lp_b; 
	auto min = max; 
	auto imin = 0;
	auto imax = 0;

	for (auto i = m_imaxmin1SL + 1; i <= m_imaxmin2SL; i++)
	{
		const int val = *lp_b;
		if (val > max) 
		{
			max = val; 
			imax = i;
		}
		else if (val < min) 
		{
			min = val;
			imin = i;
		}
		lp_b += nchans;
	}
	const auto i_min_to_max = imin - imax;

	// get offset over zero at the requested index
	int offset = 0;
	if (b_adjust)
	{
		if (m_icenter1SL - m_icenter2SL != 0)
		{
			offset = lavg / (m_icenter2SL - m_icenter1SL + 1);
			offset -= m_spike_buffer.m_bin_zero;
		}
	}

	// save values computed here within spike element structure
	auto* se = m_spike_elements.GetAt(no);
	se->SetSpikeMaxMin(max - offset, min - offset, i_min_to_max);
	se->SetSpikeAmplitudeOffset(offset);
	auto lp_dest = m_spike_buffer.GetSpike(no); 
	for (auto i = m_spike_buffer.GetSpikeLength(); i > 0; i--) 
	{
		*lp_dest = *lp_source - static_cast<short>(offset); 
		lp_dest++;
		lp_source += nchans; 
	}
	return true;
}

void SpikeList::MeasureSpikeMaxMin(const int index, int* max, int* imax, int* min, int* imin)
{
	auto lp_buffer = m_spike_buffer.GetSpike(index); 
	int val = *lp_buffer;
	*max = val; 
	*min = val; 
	*imin = *imax = 0;
	for (auto i = 1; i < m_spike_buffer.GetSpikeLength(); i++) 
	{
		lp_buffer++;
		val = *lp_buffer;
		if (val > *max) 
		{
			*max = val;
			*imax = i;
		}
		else if (val < *min) 
		{
			*min = val;
			*imin = i;
		}
	}
}

void SpikeList::MeasureSpikeMaxThenMin(const int index, int* max, int* imax, int* min, int* imin)
{
	auto lp_buffer = m_spike_buffer.GetSpike(index); 
	auto lp_bmax = lp_buffer;
	int val = *lp_buffer;
	*max = val; 
	*imax = 0;
	
	for (auto i = 1; i < m_spike_buffer.GetSpikeLength(); i++) 
	{
		lp_buffer++;
		val = *lp_buffer;
		if (val > *max)
		{
			*max = val; 
			*imax = i;
			lp_bmax = lp_buffer;
		}
	}

	lp_buffer = lp_bmax;
	*min = *max;
	*imin = *imax;
	for (auto i = *imax; i < m_spike_buffer.GetSpikeLength(); i++) 
	{
		lp_buffer++;
		val = *lp_buffer;
		if (val < *min) 
		{
			*min = val; 
			*imin = i;
		}
	}
}

void SpikeList::MeasureSpikeMaxMinEx(const int index, int* max, int* imax, int* min, int* imin, const int ifirst, const int ilast)
{
	auto lp_buffer = m_spike_buffer.GetSpike(index);
	lp_buffer += ifirst;
	int val = *lp_buffer;
	*max = val; 
	*min = *max; 
	*imin = *imax = ifirst;
	for (auto i = ifirst + 1; i <= ilast; i++) 
	{
		lp_buffer++;
		val = *lp_buffer;
		if (val > *max) 
		{
			*max = val;
			*imax = i;
		}
		else if (val < *min) 
		{
			*min = val; 
			*imin = i;
		}
	}
}

void SpikeList::MeasureSpikeMaxThenMinEx(const int index, int* max, int* imax, int* min, int* imin, const int ifirst, const int ilast)
{
	auto lp_buffer = m_spike_buffer.GetSpike(index);
	lp_buffer += ifirst; // get pointer to buffer
	auto lp_bmax = lp_buffer;
	int val = *lp_buffer;
	*max = val; // assume offset between points = 1 (short)
	*imax = ifirst;

	// first search for max
	for (auto i = ifirst + 1; i <= ilast; i++) // loop to scan data
	{
		lp_buffer++;
		val = *lp_buffer;
		if (val > *max) // search max
		{
			*max = val; // change max and imax
			*imax = i;
			lp_bmax = lp_buffer;
		}
	}

	// search for min
	lp_buffer = lp_bmax;
	*min = *max;
	*imin = *imax;
	for (int i = *imin + 1; i <= ilast; i++) // loop to scan data
	{
		lp_buffer++;
		val = *lp_buffer;
		if (val < *min) // search max
		{
			*min = val; // change max and imax
			*imin = i;
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
			if (GetSpikeClass(index) >= 0)
			{
				GetSpikeExtrema(0, &max1, &min1);
				break;
			}
		}
		m_minimum_over_all_spikes = min1;
		m_maximum_over_all_spikes = max1; 
		for (auto index = 0; index < n_spikes; index++)
		{
			if (GetSpikeClass(index) >= 0)
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
	m_spike_buffer.SetSpikeLength(m_detection_parameters.extractNpoints);
	m_spike_buffer.m_bin_zero = m_bin_zero;

	// reset classes
	m_only_valid_classes = FALSE; // default: no valid array
	m_n_classes = 0;
	m_spike_class_descriptor_array.SetSize(2); // default size - some functions
	m_spike_class_descriptor_array.SetAt(0, SpikeClassDescriptor(0,0)); 
	return TRUE;
}

void SpikeList::EraseData()
{
	delete_arrays();
	m_spike_buffer.DeleteAllSpikes();
	m_selected_spike = -1;
}

int SpikeList::SetSpikeFlag(int spikeno, BOOL bFlag)
{
	// set spike flag: add spike to the array
	if (TRUE == bFlag)
	{
		// first look if spikeno is already flagged
		if (!GetSpikeFlag(spikeno))
			m_spikes_flagged_.Add(spikeno);
	}

	// remove flag
	else
	{
		// find spikeno within the array and remove it
		auto index = -1;
		for (auto i = m_spikes_flagged_.GetCount() - 1; i >= 0; i--)
		{
			if (m_spikes_flagged_.GetAt(i) == spikeno)
			{
				index = i;
				break;
			}
		}
		if (index >= 0)
			m_spikes_flagged_.RemoveAt(index);
	}
	// return the number of elements within the array
	return GetSpikeFlagArrayCount();
}

int SpikeList::ToggleSpikeFlag(int spikeno)
{
	// find spike within array
	auto index = -1;
	for (auto i = m_spikes_flagged_.GetCount() - 1; i >= 0; i--)
	{
		if (m_spikes_flagged_.GetAt(i) == spikeno)
		{
			index = i;
			break;
		}
	}

	// if found: remove it
	if (index >= 0)
		m_spikes_flagged_.RemoveAt(index);

		// if not found, add it
	else
		m_spikes_flagged_.Add(spikeno);

	return GetSpikeFlagArrayCount();
}

void SpikeList::SetSingleSpikeFlag(int spikeno)
{
	if (m_spikes_flagged_.GetCount() != 1)
		m_spikes_flagged_.SetSize(1);
	m_spikes_flagged_.SetAt(0, spikeno);
}

BOOL SpikeList::GetSpikeFlag(int spikeno)
{
	BOOL bFlag = FALSE;
	// search if spikeno is in the list
	for (int i = m_spikes_flagged_.GetCount() - 1; i >= 0; i--)
	{
		if (m_spikes_flagged_.GetAt(i) == spikeno)
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
	for (auto i = m_spikes_flagged_.GetCount() - 1; i >= 0; i--)
	{
		const int ispik = m_spikes_flagged_.GetAt(i);
		l_time = GetSpikeTime(ispik);
		if (l_time < l_first || l_time > l_last)
			continue;
		m_spikes_flagged_.RemoveAt(i);
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
		m_spikes_flagged_.Add(i);
	}
}

void SpikeList::SelectSpikeswithinRect(int vmin, int vmax, long l_first, long l_last, BOOL bAdd)
{
	if (!bAdd)
		RemoveAllSpikeFlags();
	for (auto i = 0; i < GetTotalSpikes(); i++)
	{
		const auto lTime = GetSpikeTime(i);
		if (lTime < l_first || lTime > l_last)
			continue;

		int max, min;
		GetSpikeExtrema(i, &max, &min);
		if (max > vmax) continue;
		if (min < vmin) continue;
		// found within boundaries= remove spike from array
		m_spikes_flagged_.Add(i);
	}
}

void SpikeList::GetRangeOfSpikeFlagged(long& l_first, long& l_last)
{
	// no spikes flagged, return dummy values
	if (m_spikes_flagged_.GetCount() < 1)
	{
		l_first = -1;
		l_last = -1;
		return;
	}

	// spikes flagged: init max and min to the first spike time
	l_first = GetSpikeTime(m_spikes_flagged_.GetAt(0));
	l_last = l_first;

	// search if spikeno is in the list
	for (auto i = m_spikes_flagged_.GetCount() - 1; i >= 0; i--)
	{
		const auto lTime = GetSpikeTime(m_spikes_flagged_.GetAt(i));
		if (lTime < l_first)
			l_first = lTime;
		if (lTime > l_last)
			l_last = lTime;
	}
}

void SpikeList::OffsetSpikeAmplitude(int index, int valfirst, int vallast, int center)
{
	auto lp_buffer = m_spike_buffer.GetSpike(index); // get pointer to buffer
	const auto diff = vallast - valfirst; // difference
	int max = *lp_buffer; // compute max/min on the fly
	auto min = max; // provisional max and minimum
	auto imax = 0;
	auto imin = vallast - valfirst;
	for (auto i = 0; i < m_spike_buffer.GetSpikeLength(); i++) // loop to scan data
	{
		// offset point i
		*lp_buffer += valfirst + MulDiv(diff, i, m_spike_buffer.GetSpikeLength());
		if (*lp_buffer > max) // new max?
		{
			max = *lp_buffer;
			imax = i;
		}
		else if (*lp_buffer < min) // new min?
		{
			min = *lp_buffer;
			imin = i;
		}
		lp_buffer++; // update pointer
	}
	const auto dmaxmin = imin - imax; // assume that min comes after max in a "normal" spike

	auto offset = m_spike_elements[index]->get_amplitude_offset();
	offset -= valfirst; // change spike offset
	m_spike_elements[index]->SetSpikeAmplitudeOffset(offset);
	m_extrema_valid = FALSE;
	m_spike_elements[index]->SetSpikeMaxMin(max, min, dmaxmin);
}

void SpikeList::CenterSpikeAmplitude(int spkindex, int ifirst, int ilast, WORD method)
{
	auto lp_buffer = m_spike_buffer.GetSpike(spkindex); // get pointer to buffer
	lp_buffer += ifirst;
	int valfirst; // contains offset
	auto imax = 0;
	auto imin = 0;
	auto dmaxmin = 0;
	int max, min;
	auto p_se = GetSpikeElemt(spkindex);
	p_se->GetSpikeMaxMin(&max, &min, &dmaxmin);

	switch (method)
	{
	case 0: // ........................ center (max + min )/2
		{
			max = *lp_buffer; // compute max/min on the fly
			min = max; // provisional max and minimum
			for (auto i = ifirst; i < ilast; i++, lp_buffer++)
			{
				// offset point i
				if (*lp_buffer > max) // new max?
				{
					max = *lp_buffer;
					imax = i;
				}
				else if (*lp_buffer < min) // new min?
				{
					min = *lp_buffer;
					imin = i;
				}
			}
			valfirst = (max + min) / 2 - GetAcqBinzero();
			dmaxmin = imin - imax;
		}
		break;
	case 1: // ........................ center average
		{
			long lsum = 0;
			const auto ioffset = GetAcqBinzero();
			for (auto i = ifirst; i < ilast; i++, lp_buffer++)
				lsum += (*lp_buffer - ioffset);
			valfirst = static_cast<int>(lsum / static_cast<long>(ilast - ifirst + 1));
		}
		break;
	default:
		valfirst = 0;
		break;
	}

	// change spike offset
	p_se->SetSpikeAmplitudeOffset(p_se->get_amplitude_offset() + valfirst);
	// then offset data (max & min ibidem)
	p_se->SetSpikeMaxMin(max - valfirst, min - valfirst, dmaxmin);
	lp_buffer = m_spike_buffer.GetSpike(spkindex); // get pointer to buffer
	for (auto i = 0; i < m_spike_buffer.GetSpikeLength(); i++, lp_buffer++)
		*lp_buffer -= valfirst;
	m_extrema_valid = FALSE;
}

long SpikeList::UpdateClassList()
{
	const auto n_spikes = GetTotalSpikes();
	m_n_classes = 1; // erase nb of classes
	m_spike_class_descriptor_array.SetSize(0); // erase array
	m_only_valid_classes = TRUE; // class list OK when exit
	if (n_spikes == 0)
	{
		// no spikes? dummy parameters
		m_spike_class_descriptor_array.Add(SpikeClassDescriptor(0, 0)); 
		return 0L; // done
	}

	m_spike_class_descriptor_array.Add(SpikeClassDescriptor(GetSpikeClass(0), 1)); 
	m_n_classes = 1; // total nb of classes

	// loop over all spikes of the list
	for (auto i = 1; i < n_spikes; i++)
	{
		const auto cla = GetSpikeClass(i); // class of spike i
		auto arrayclass = 0;

		// loop over all existing classes to find if there is one
		for (auto j = 0; j < m_n_classes; j++)
		{
			arrayclass = GetclassID(j); // class ID
			if (cla == arrayclass) // class OK?
			{
				m_spike_class_descriptor_array.GetAt(j).n_items++; // increment spike count
				break; // exit spk class loop
			}
			if (cla < arrayclass) // insert new class?
			{
				m_spike_class_descriptor_array.InsertAt(j , SpikeClassDescriptor(cla, 1)); 
				m_n_classes++; // exit spk class loop
				break;
			}
		}

		// test if cla not found within array
		if (cla > arrayclass)
		{
			m_spike_class_descriptor_array.Add(SpikeClassDescriptor(cla, 1)); 
			m_n_classes++; // update nb classes
		}
	}
	return m_n_classes;
}

void SpikeList::ChangeSpikeClassID(int oldclaID, int newclaID)
{
	// first find valid max and min
	for (auto index = 0; index < m_spike_elements.GetSize(); index++)
	{
		if (GetSpikeClass(index) == oldclaID)
			SetSpikeClass(index, newclaID);
	}
}

void SpikeList::Measure_case0_AmplitudeMinToMax(const int t1, const int t2)
{
	const auto n_spikes = GetTotalSpikes();
	for (auto ispike = 0; ispike < n_spikes; ispike++)
	{
		const auto spike_element = GetSpikeElemt(ispike);
		auto lp_buffer = m_spike_buffer.GetSpike(ispike);
		lp_buffer += t1;
		int val = *lp_buffer;
		auto max = val;
		auto min = val;
		auto imin = t1;
		auto imax = t1;
		for (auto i = t1 + 1; i <= t2; i++)
		{
			lp_buffer++;
			val = *lp_buffer;
			if (val > max)
			{
				max = val;
				imax = i;
			}
			else if (val < min)
			{
				min = val;
				imin = i;
			}
		}

		spike_element->SetSpikeMaxMin(max, min, imin - imax);
		spike_element->set_y1(max - min);
	}
}

void SpikeList::Measure_case1_AmplitudeAtT(const int t)
{
	const auto n_spikes = GetTotalSpikes();

	for (auto ispike = 0; ispike < n_spikes; ispike++)
	{
		const auto spike_element = GetSpikeElemt(ispike);
		auto lp_buffer = m_spike_buffer.GetSpike(ispike);
		lp_buffer += t;
		const int val = *lp_buffer;
		spike_element->set_y1(val);
	}
}

void SpikeList::Measure_case2_AmplitudeAtT2MinusAtT1(const int t1, const int t2)
{
	const auto n_spikes = GetTotalSpikes();

	for (auto ispike = 0; ispike < n_spikes; ispike++)
	{
		const auto spike_element = GetSpikeElemt(ispike);
		const auto lp_buffer = m_spike_buffer.GetSpike(ispike);
		const int val1 = *(lp_buffer + t1);
		const int val2 = *(lp_buffer + t2);
		spike_element->set_y1(val2 - val1);
	}
}

CSize SpikeList::Measure_Y1_MaxMin()
{
	const auto n_spikes = GetTotalSpikes();
	int max = GetSpikeElemt(0)->get_y1();
	int min = max;
	for (auto ispike = 0; ispike < n_spikes; ispike++)
	{
		const auto val = GetSpikeElemt(ispike)->get_y1();
		if (val > max) max = val;
		if (val < min) min = val;
	}

	return CSize(max, min);
}

BOOL SpikeList::SortSpikeWithY1(const CSize fromclass_toclass, const CSize timewindow, const CSize limits)
{
	const auto n_spikes = GetTotalSpikes();

	const auto from_class = fromclass_toclass.cx;
	const auto to_class = fromclass_toclass.cy;
	const auto first = timewindow.cx;
	const auto last = timewindow.cy;
	const int upper = limits.cy;
	const int lower = limits.cx;
	BOOL bchanged = false;

	for (auto ispike = 0; ispike < n_spikes; ispike++)
	{
		const auto spike_element = GetSpikeElemt(ispike);
		if (spike_element->get_class() != from_class)
			continue;
		const auto ii_time = spike_element->get_time();
		if (ii_time < first || ii_time > last)
			continue;
		const auto value = spike_element->get_y1();
		if (value >= lower && value <= upper)
		{
			spike_element->set_class(to_class);
			bchanged = true;
		}
	}

	return bchanged;
}

BOOL SpikeList::SortSpikeWithY1AndY2(const CSize fromclass_toclass, const CSize timewindow, const CSize limits1,
                                      const CSize limits2)
{
	const auto n_spikes = GetTotalSpikes();

	const auto from_class = fromclass_toclass.cx;
	const auto to_class = fromclass_toclass.cy;
	const auto first = timewindow.cx;
	const auto last = timewindow.cy;
	const int upper1 = limits1.cy;
	const int lower1 = limits1.cx;
	const int upper2 = limits2.cy;
	const int lower2 = limits2.cx;
	BOOL bchanged = false;

	for (auto ispike = 0; ispike < n_spikes; ispike++)
	{
		const auto spike_element = GetSpikeElemt(ispike);
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
			bchanged = true;
		}
	}

	return bchanged;
}

int SpikeList::GetValidSpikeNumber(int ispike)
{
	if (ispike < 0)
		ispike = -1;
	if (ispike >= GetTotalSpikes())
		ispike = GetTotalSpikes() - 1;
	return ispike;
}

int SpikeList::GetNextSpike(int spikeno, int delta, BOOL bKeepSameClass)
{
	int spikeold = spikeno;
	int classold = 0;
	if (spikeold >= 0 && spikeold < GetTotalSpikes() - 1)
		classold = GetSpikeClass(spikeno);
	if (delta >= 0)
		delta = 1;
	else
		delta = -1;
	if (bKeepSameClass)
	{
		do
		{
			spikeno += delta;
			if (spikeno < 0 || spikeno >= GetTotalSpikes())
			{
				spikeno = spikeold;
				break;
			}
		}
		while (spikeno < GetTotalSpikes() && GetSpikeClass(spikeno) != classold);
	}
	else
		spikeno += delta;

	if (spikeno >= GetTotalSpikes())
		spikeno = spikeold;
	return GetValidSpikeNumber(spikeno);
}
