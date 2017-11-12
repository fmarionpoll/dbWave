#pragma once

// ----------------------------------------------

class CIntervalsArray : public CObject
{
	DECLARE_SERIAL(CIntervalsArray)

public :
	CIntervalsArray();	
	virtual		~CIntervalsArray();
	virtual void Serialize(CArchive& ar);	
	void operator = (const CIntervalsArray& arg);

	inline long	GetiiTime(int i) { return iistimulus.GetAt(i); };
	inline void	SetiiTime(int i, long iitime) { iistimulus.SetAt(i, iitime);};
	inline long	GetSize() { return iistimulus.GetSize(); }
	inline int  GetChan() { return ichan; }
	inline void SetChan(int chan) { ichan = chan; }
	inline void AddInterval(long ii) { iistimulus.Add(ii); }

public:
	CArray <long, long> iistimulus;	// time on, time off (n clock intervals)
	int		iID;				// ID number of the array
	int		ichan;
	CString	csDescriptor;		// descriptor of the array
	int		nitems;				// number of on/off events
	int		npercycle;			// repeat sequence parameter
	float	chrate;				// n clock cycles per second
protected:
	int		version;
};

// ----------------------------------------------

class CIntervalPoint : public CObject
{
	DECLARE_SERIAL(CIntervalPoint)
	CIntervalPoint();
	CIntervalPoint(const CIntervalPoint& pt);
	//~CIntervalPoint();
	virtual void Serialize(CArchive& ar);
	void operator = (const CIntervalPoint& arg);

public:
	long ii;					// n clock intervals
	WORD w;						// word associated to this time interval
};

// ----------------------------------------------

class CIntervalsAndWordsSeries : public CObject
{
	DECLARE_SERIAL(CIntervalsAndWordsSeries)

	CIntervalsAndWordsSeries();
	CIntervalsAndWordsSeries(const CIntervalsAndWordsSeries& arg);
	//~CIntervalsAndWordsSeries();
	void operator = (const CIntervalsAndWordsSeries& arg);
	virtual void Serialize(CArchive& ar);

	void			EraseAllData();
	inline long		GetSize() { return iistep.GetSize(); }
	CIntervalPoint  GetIntervalPointAt(int i);

	void			ImportIntervalsSeries(CIntervalsArray* pIntervals, WORD valUP=1, BOOL bcopyRate = TRUE);
	void			ImportAndMergeIntervalsArrays(CPtrArray* pIntervals);
	void			ExportIntervalsSeries(int chan, CIntervalsArray* pOut);

public:
	CArray <CIntervalPoint, CIntervalPoint>  iistep;
	float									chrate;
	int										version;
};

