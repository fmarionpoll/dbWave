#pragma once

class CScale : public CObject
{
public:
	CScale();
	CScale(int npixels); 
	DECLARE_SERIAL(CScale)
	void Serialize(CArchive& ar) override;

protected:
	int m_npixels = 1; 
	int m_nintervals = 0; 
	long m_lNdatapoints = 0; 
	CWordArray m_intervals; 
	CDWordArray m_position; 

public:
	int SetScale(int n_pixels, long n_points);
	int HowManyIntervalsFit(int fromIndex, long* l_last);
	int GetWhichInterval(long lindex);
	int GetnPixels() const { return m_npixels; }
	int GetnIntervals() const { return m_nintervals; }
	long GetNdatapoints() const { return m_lNdatapoints; }
	LPWORD ElementAt(int i) { return &(m_intervals[i]); }
	long GetPosition(int i) { return static_cast<long>(m_position[i]); }
	int GetIntervalSize(int i) { return m_intervals[i]; }
};
