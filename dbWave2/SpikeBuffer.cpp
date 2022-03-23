#include "StdAfx.h"
#include "SpikeBuffer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CSpikeBuffer

IMPLEMENT_SERIAL(CSpikeBuffer, CObject, 0 /* schema number*/)

CSpikeBuffer::CSpikeBuffer()
{
	SetSpklen(1); // init with spike len = 1
	m_spike_data_positions.SetSize(0, 128);
}

CSpikeBuffer::CSpikeBuffer(int lenspk)
{
	SetSpklen(lenspk);
	m_spike_data_positions.SetSize(0, 128);
}

CSpikeBuffer::~CSpikeBuffer()
{
	DeleteAllSpikes();
}

void CSpikeBuffer::Serialize(CArchive& ar)
{
	// store elements
	if (ar.IsStoring())
	{
	}
	// load data
	else
	{
	}
}

void CSpikeBuffer::SetSpklen(int lenspik)
{
	m_spike_length = lenspik;
	DeleteAllSpikes();
	if (m_spike_length > 0)
	{
		// allocate memory by 64 Kb chunks
		m_spk_buffer_increment = static_cast<WORD>(32767);
		m_spk_buffer_increment = (m_spk_buffer_increment / m_spike_length) * m_spike_length;
		m_spk_buffer_length = m_spk_buffer_increment;
		m_spike_data_buffer = static_cast<short*>(malloc(sizeof(short) * m_spk_buffer_length));
		ASSERT(m_spike_data_buffer != NULL);
		m_next_index = 0;
		m_last_index = m_spk_buffer_length / m_spike_length - 1;
	}
}

short* CSpikeBuffer::AllocateSpaceForSpikeAt(int spkindex)
{
	// get pointer to next available buffer area for this spike
	// CAUTION: spkindex != m_next_index
	if (m_next_index > m_last_index)
	{
		m_spk_buffer_length += m_spk_buffer_increment;
		auto* pspkbuffer = static_cast<short*>(realloc(m_spike_data_buffer, sizeof(short) * m_spk_buffer_length));
		if (pspkbuffer != nullptr)
			m_spike_data_buffer = pspkbuffer;
		m_last_index = m_spk_buffer_length / m_spike_length - 1;
	}

	// compute destination address
	const auto offset = m_next_index * m_spike_length;
	const auto lp_dest = m_spike_data_buffer + offset;
	m_spike_data_positions.InsertAt(spkindex, offset);
	m_next_index++;
	return lp_dest;
}

short* CSpikeBuffer::AllocateSpaceForSeveralSpikes(int nspikes)
{
	// get pointer to next available buffer area for these spikes
	const auto currentindex = m_next_index;
	m_next_index += nspikes;
	while (m_next_index > m_last_index)
	{
		m_spk_buffer_length += m_spk_buffer_increment;
		const auto pspkbuffer = static_cast<short*>(realloc(m_spike_data_buffer, sizeof(short) * m_spk_buffer_length));
		if (pspkbuffer != nullptr)
			m_spike_data_buffer = pspkbuffer;
		m_last_index = m_spk_buffer_length / m_spike_length - 1;
	}

	// compute destination address
	const auto lp_dest = m_spike_data_buffer + (currentindex * m_spike_length);
	auto ioffset = currentindex * m_spike_length;
	for (auto i = currentindex; i < m_next_index; i++)
	{
		m_spike_data_positions.InsertAt(i, ioffset);
		ioffset += m_spike_length;
	}
	return lp_dest;
}

void CSpikeBuffer::DeleteAllSpikes()
{
	// delete handle array and liberate corresponding memory
	if (m_spike_data_buffer != nullptr)
		free(m_spike_data_buffer);
	m_spike_data_buffer = nullptr;

	// delete array of pointers
	m_spike_data_positions.RemoveAll();
	m_next_index = 0;
}

BOOL CSpikeBuffer::DeleteSpike(int spkindex)
{
	if (spkindex > m_spike_data_positions.GetUpperBound() || spkindex < 0)
		return FALSE;
	m_spike_data_positions.RemoveAt(spkindex);
	return TRUE;
}

BOOL CSpikeBuffer::ExchangeSpikes(int spk1, int spk2)
{
	if (spk1 > m_spike_data_positions.GetUpperBound() || spk1 < 0
		|| spk2 > m_spike_data_positions.GetUpperBound() || spk2 < 0)
		return FALSE;
	const DWORD dummy = m_spike_data_positions[spk1];
	m_spike_data_positions[spk1] = m_spike_data_positions[spk2];
	m_spike_data_positions[spk2] = dummy;
	return TRUE;
}
