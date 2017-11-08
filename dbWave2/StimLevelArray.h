#pragma once

// StimLevelArray.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CIntervalsArray object


class CIntervalsArray : public CObject
{
	DECLARE_SERIAL(CIntervalsArray)

public :
	CIntervalsArray();			// protected constructor used by dynamic creation
		
// Attributes
public:
	CArray <long, long> iistimulus;	// time on, time off
	int		iID;				// ID number of the array
	int		ichan;
	CString	csDescriptor;		// descriptor of the array
	int		nitems;				// number of on/off events
	int		npercycle;

protected:
	int		version;			// current: 1

// Implementation
public:
	virtual		~CIntervalsArray();
	virtual void Serialize(CArchive& ar);			// overridden for document i/o
	void	operator = (const CIntervalsArray& arg);	// operator redefinition
	inline long	GetiiTime(int i) { return iistimulus.GetAt(i); };
	inline void	SetiiTime(int i, long iitime) { iistimulus.SetAt(i, iitime);};
	inline long	GetSize() { return iistimulus.GetSize(); }
	inline int  GetChan() { return ichan; }
	inline void SetChan(int chan) { ichan = chan; }
	inline void AddInterval(long ii) { iistimulus.Add(ii); }
};


class CIntervalPoint : public CObject
{
	DECLARE_SERIAL(CIntervalPoint)
	CIntervalPoint();
	~CIntervalPoint();
	virtual void Serialize(CArchive& ar);
	void operator = (const CIntervalPoint& arg);

public:
	long ii;
	WORD w;
};

class CIntervalsAndWordsSeries : public CObject
{
	DECLARE_SERIAL(CIntervalsArray)
	CIntervalsAndWordsSeries();		// protected constructor used by dynamic creation
	~CIntervalsAndWordsSeries();
	void			operator = (const CIntervalsAndWordsSeries& arg);	// operator redefinition
	virtual void	Serialize(CArchive& ar);							// overridden for document i/o

public:
	CArray <CIntervalPoint, CIntervalPoint>  iistep;	// time on, time off
protected:
	int		version;				// current: 1

// Implementation
public:
	void			EraseAllData();
	inline long		GetSize() { return iistep.GetSize(); }
	void			ImportIntervalsSeries(CIntervalsArray* pIntervals, WORD valUP=1);
	void			ImportAndMergeIntervalsArrays(CPtrArray* pIntervals);
	CIntervalsArray* ExportIntervalsSeries(int chan);
};