// cscale.h

#pragma once

class CScale :public CObject
{
public:
	CScale();				// protected constructor used by dynamic creation
	CScale(int npixels);	// create Envelope with npoints
	DECLARE_SERIAL(CScale)
	void Serialize(CArchive& ar) override;

protected:
	int m_npixels;			// scale built for n pixels
	int	m_nintervals;		// nb of elements within Scale
	long m_lNdatapoints{};	// Ndatapoints are mapped to m_nintervals
	CWordArray m_intervals;	// scale array: npts within each interval
	CDWordArray m_position;	// interval array: consecutive file index (long)

public:
	int			SetScale(int n_pixels, long n_points);
	int			HowManyIntervalsFit(int fromIndex, long* l_last);
	int			GetWhichInterval(long lindex);
	inline int	GetnPixels() const { return m_npixels; }
	inline int	GetnIntervals() const { return m_nintervals; }
	inline long GetNdatapoints() const { return m_lNdatapoints; }
	inline LPWORD ElementAt(int i) { return &(m_intervals[i]); }
	inline long GetPosition(int i) { return (long)m_position[i]; }
	inline int  GetIntervalSize(int i) { return (int)m_intervals[i]; }
};
