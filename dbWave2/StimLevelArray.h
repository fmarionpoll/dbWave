#pragma once


class CIntervalsAndLevels : public CObject
{
	DECLARE_SERIAL(CIntervalsAndLevels)

public:
	CIntervalsAndLevels();
	virtual		~CIntervalsAndLevels();
	void Serialize(CArchive& ar) override;

	CIntervalsAndLevels& operator = (const CIntervalsAndLevels& arg);

	inline long	GetiiTime(int i) { return intervalsArray.GetAt(i); };
	inline void	SetiiTime(int i, long iitime) { intervalsArray.SetAt(i, iitime); };
	inline long	GetSize() { return intervalsArray.GetSize(); }
	inline int  GetChan() { return ichan; }
	inline void SetChan(int chan) { ichan = chan; }
	inline void AddInterval(long ii) { intervalsArray.Add(ii); }

public:
	CArray <long, long> intervalsArray;	// time on, time off (n clock intervals)
	int		iID;						// ID number of the array
	int		ichan;
	CString	csDescriptor;				// descriptor of the array
	int		nitems;						// number of on/off events
	int		npercycle;					// repeat sequence parameter
	float	chrate;						// n clock cycles per second
protected:
	int		version;
};

struct CIntervalPoint {
	long	ii;
	WORD	w;
};

class CIntervalsAndWordsSeries : public CObject
{
	DECLARE_SERIAL(CIntervalsAndWordsSeries)

	CIntervalsAndWordsSeries();
	CIntervalsAndWordsSeries(const CIntervalsAndWordsSeries& arg);
	~CIntervalsAndWordsSeries();
	void			 operator = (const CIntervalsAndWordsSeries& arg);
	void			Serialize(CArchive& ar) override;

	void			EraseAllData();
	inline long		GetSize() { return intervalpoint_array.GetSize(); }
	CIntervalPoint  GetIntervalPointAt(int i);

	void			ImportIntervalsSeries(CIntervalsAndLevels* pIntervals, WORD valUP = 1, BOOL bcopyRate = TRUE);
	void			ImportAndMergeIntervalsArrays(CPtrArray* pIntervals);
	void			ExportIntervalsSeries(int chan, CIntervalsAndLevels* pOut);

public:
	CArray <CIntervalPoint, CIntervalPoint>  intervalpoint_array;
	float			chrate;
	int				version;
};
