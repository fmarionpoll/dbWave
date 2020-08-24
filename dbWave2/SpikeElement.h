#pragma once

/////////////////////////////////////////////////////////////////////////////
// CSpikeElemt CObject
// this object is a serialized structure containing parameters associated to
// each spike detected from a data document
// stores:  time of occurence, initial data acq chan and a modifiable parameter
// the class
// this basic object is part of a CSpikelist object that stores parameters
// concerning the source document (file, date, acq rate and parms, etc) and
// eventually the source data extracted from the data file
// real data are stored in a separate object managing data buffers

class CSpikeElemt : public CObject
{
	DECLARE_SERIAL(CSpikeElemt)
public:
	CSpikeElemt();
	CSpikeElemt(LONG time, WORD channel);
	CSpikeElemt(LONG time, WORD channel, int max, int min, int offset, int iclass, int dmaxmin);

	// Attributes
private:
	long	m_iitime;		// occurence time - multiply by rate to get time in seconds
	int		m_class;		// spike class - init to zero at first
	int		m_chanparm;		// spike detection array index
	int		m_max;			// spike max	(used to scan rapidly to adjust display)
	int		m_min;			// min val		(used to scan rapidly to adjust display)
	int		m_dmaxmin;
	int		m_offset;		// offset voltage pt 1
	int		y1_ = 0;			// parameter measured and stored
	int		y2_ = 0;
	int		dt_ = 0;

	// Operations set/change elements of spikeele
public:
	long	get_time() const { return m_iitime; }
	int		get_class() const { return m_class; }
	int		get_source_channel() const { return m_chanparm; }
	short	get_maximum() const { return m_max; }
	short	get_minimum() const { return m_min; }
	int		get_amplitude_offset() const { return m_offset; }
	void	GetSpikeMaxMin(int* max, int* min, int* dmaxmin) { *max = m_max; *min = m_min; *dmaxmin = m_dmaxmin; }
	void	GetSpikeExtrema(int* max, int* min) { *max = m_max; *min = m_min; }
	int		get_y1() const { return y1_; }
	int		get_y2() const { return y2_; }
	int		get_dt() const { return dt_; }

	void	set_time(long ii) { m_iitime = ii; }
	void	set_class(int cl) { m_class = cl; }
	void	SetSpikeMaxMin(int max, int min, int dmaxmin) { m_max = max; m_min = min; m_dmaxmin = dmaxmin; }
	void	SetSpikeAmplitudeOffset(int offset) { m_offset = offset; }
	void	set_y1(int y) { y1_ = y; }
	void	set_y2(int y) { y2_ = y; }
	void	set_dt(long x) { dt_ = x; }

	// Implementation
public:
	virtual ~CSpikeElemt();
	void	Read0(CArchive& ar);
	void Serialize(CArchive& ar) override;
};
