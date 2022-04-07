#pragma once
#include "ChartWnd.h"
#include "dbWaveDoc.h"
#include "Spikedoc.h"
#include "Spikelist.h"

class ChartSpike : public ChartWnd
{
public:
	ChartSpike();
	~ChartSpike();

protected:
	int m_rangemode = RANGE_TIMEINTERVALS;	// display range
	long m_lFirst = 0;						// time index of first pt displayed
	long m_lLast = 0;						// time index of last pt displayed
	int m_spkfirst{};						// index first spike
	int m_spklast{};						// index last spike
	int m_currentclass = 0;					// current class in case of displaying classes
	int m_selectedspike = -1;				// selected spike (display differently)
	int m_selpen{};
	int m_hitspk = -1;						// no of spike selected
	int m_selclass{};						// index class selected
	BOOL m_btrackCurve = false;				// track curve ?
	BOOL m_ballFiles = false;				// display data from all files in CdbWaveDoc*

	SpikeList* p_spikelist_ = nullptr;
	CSpikeDoc* p_spike_doc_ = nullptr;
	CdbWaveDoc* p_dbwave_doc_ = nullptr;

	int get_color_according_to_plotmode(const Spike* spike, int plot_mode) const;

public:
	void SetRangeMode(int mode) { m_rangemode = mode; }
	int GetRangeMode() const { return m_rangemode; }

	void SetSelClass(int selclass) { m_selclass = selclass; }
	int GetSelClass() const { return m_selclass; }

	void SetPlotMode(int mode, int selclass)
	{
		m_plotmode = mode;
		m_selclass = selclass;
	}
	int GetPlotMode() const { return m_plotmode; }

	void SetCurrentClass(int curcla) { m_currentclass = curcla; }
	int GetCurrentClass() const { return m_currentclass; }

	long GetTimeFirst() const { return m_lFirst; }
	long GetTimeLast() const { return m_lLast; }
	void SetTimeIntervals(long l_first, long l_last)
	{
		m_lFirst = l_first;
		m_lLast = l_last;
	}
};
