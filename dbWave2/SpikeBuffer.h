#pragma once

// CSpikeBuffer stores data collected from data acquisition files

class CSpikeBuffer : public CObject
{
	DECLARE_SERIAL(CSpikeBuffer)
public:
	CSpikeBuffer();
	CSpikeBuffer(int lenspik);
	virtual ~CSpikeBuffer();

	// Attributes
public:
	int			m_lenspk{};				// length of one spike
	int			m_spkbufferincrement{};	//
	int			m_spkbufferlength{};	// current buffer length
	int			m_nextindex{};			// next available index
	int			m_lastindex{};			// index last free space for spike data
	int			m_binzero = 2048;		// zero (if 12 bits scale = 0-4095)
	int			m_currentfilter{};		// ID of last transform
	short*		m_spikedata_buffer = nullptr;	// buffer address
	CArray <int, int>	m_spikedata_positions;	// offsets to individual spike data - used to exchange, add, remove spikes without modifying buffer

// Operations
public:
	void 		SetSpklen(int lenspik);
	inline int 	GetSpklen() const { return m_lenspk; }

	// Implementation
public:
	void Serialize(CArchive& ar) override;

	void 	DeleteAllSpikes();
	short*	AllocateSpaceForSpikeAt(int spkindex);
	short*	AllocateSpaceForSeveralSpikes(int spkindex);
	BOOL  	DeleteSpike(int spkindex);
	BOOL  	ExchangeSpikes(int spk1, int spk2);
	short*	GetSpike(int index) const { return (m_spikedata_buffer + m_spikedata_positions[index]); }
};