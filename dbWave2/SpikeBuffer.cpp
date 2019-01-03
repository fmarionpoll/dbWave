
#include "StdAfx.h"
#include "SpikeBuffer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CSpikeBuffer

IMPLEMENT_SERIAL(CSpikeBuffer, CObject, 0 /* schema number*/)

CSpikeBuffer::CSpikeBuffer()
{
	m_spikedata_buffer = nullptr;
	m_binzero = 2048;
	SetSpklen(1);	// init with spike len = 1	
	m_spikedata_positions.SetSize(0, 128);
}

CSpikeBuffer::CSpikeBuffer(int lenspk)
{
	m_spikedata_buffer = nullptr;
	m_binzero = 2048;
	SetSpklen(lenspk);
	m_spikedata_positions.SetSize(0, 128);
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
	m_lenspk = lenspik;
	DeleteAllSpikes();
	if (m_lenspk > 0) {
		// allocate memory by 64 Kb chunks
		m_spkbufferincrement = static_cast<WORD>(32767);
		m_spkbufferincrement = (m_spkbufferincrement / m_lenspk)*m_lenspk;
		m_spkbufferlength = m_spkbufferincrement;
		m_spikedata_buffer = static_cast<short*>(malloc(sizeof(short) * m_spkbufferlength));
		ASSERT(m_spikedata_buffer != NULL);
		m_nextindex = 0;
		m_lastindex = m_spkbufferlength / m_lenspk - 1;
	}
}

short* CSpikeBuffer::AllocateSpaceForSpikeAt(int spkindex)
{
	// get pointer to next available buffer area for this spike
	// CAUTION: spkindex != m_nextindex
	if (m_nextindex > m_lastindex)
	{
		m_spkbufferlength += m_spkbufferincrement;
		auto* pspkbuffer = static_cast<short*>(realloc(m_spikedata_buffer, sizeof(short) * m_spkbufferlength));
		if (pspkbuffer != nullptr)
			m_spikedata_buffer = pspkbuffer;
		m_lastindex = m_spkbufferlength / m_lenspk - 1;
	}

	// compute destination address
	const auto offset = m_nextindex * m_lenspk;
	const auto lp_dest = m_spikedata_buffer + offset;
	m_spikedata_positions.InsertAt(spkindex, offset);
	m_nextindex++;
	return lp_dest;
}

short*	CSpikeBuffer::AllocateSpaceForSeveralSpikes(int nspikes)
{
	// get pointer to next available buffer area for these spikes
	const auto currentindex = m_nextindex;
	m_nextindex += nspikes;
	while (m_nextindex > m_lastindex)
	{
		m_spkbufferlength += m_spkbufferincrement;
		const auto pspkbuffer = static_cast<short*>(realloc(m_spikedata_buffer, sizeof(short) * m_spkbufferlength));
		if (pspkbuffer != nullptr)
			m_spikedata_buffer = pspkbuffer;
		m_lastindex = m_spkbufferlength / m_lenspk - 1;
	}

	// compute destination address
	const auto lp_dest = m_spikedata_buffer + (currentindex*m_lenspk);
	auto ioffset = currentindex * m_lenspk;
	for (auto i = currentindex; i < m_nextindex; i++)
	{
		m_spikedata_positions.InsertAt(i, ioffset);
		ioffset += m_lenspk;
	}
	return lp_dest;
}

void CSpikeBuffer::DeleteAllSpikes()
{
	// delete handle array and liberate corresponding memory
	if (m_spikedata_buffer != nullptr)
		free(m_spikedata_buffer);
	m_spikedata_buffer = nullptr;

	// delete array of pointers
	m_spikedata_positions.RemoveAll();
	m_nextindex = 0;
}

BOOL CSpikeBuffer::DeleteSpike(int spkindex)
{
	if (spkindex > m_spikedata_positions.GetUpperBound() || spkindex < 0)
		return FALSE;
	m_spikedata_positions.RemoveAt(spkindex);
	return TRUE;
}

BOOL CSpikeBuffer::ExchangeSpikes(int spk1, int spk2)
{
	if (spk1 > m_spikedata_positions.GetUpperBound() || spk1 <0
		|| spk2 > m_spikedata_positions.GetUpperBound() || spk2 < 0)
		return FALSE;
	const DWORD dummy = m_spikedata_positions[spk1];
	m_spikedata_positions[spk1] = m_spikedata_positions[spk2];
	m_spikedata_positions[spk2] = dummy;
	return TRUE;
}

