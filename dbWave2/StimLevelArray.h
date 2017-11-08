#pragma once

// StimLevelArray.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CIntervalsSeries object


class CIntervalsSeries : public CObject
{
	DECLARE_SERIAL(CIntervalsSeries)

public :
	CIntervalsSeries();			// protected constructor used by dynamic creation
		
// Attributes
public:
	CArray <long, long> iistimulus;	// time on, time off
	int		iID;				// ID number of the array
	CString	csDescriptor;		// descriptor of the array
	int		nitems;				// number of on/off events
	int		npercycle;

protected:
	int		version;			// current: 1

// Implementation
public:
	virtual		~CIntervalsSeries();
	virtual void	Serialize(CArchive& ar);			// overridden for document i/o
	void	 operator = (const CIntervalsSeries& arg);	// operator redefinition
	long	GetiiTime(int i);
	void	SetiiTime(int i, long iitime);
};

typedef struct taglongW
{
	long  ii;
	WORD  w;
} longW, *longW, NEAR *longW, FAR *longW;


class CIntervalsAndWordsSeries : public CObject
{
	DECLARE_SERIAL(CIntervalsSeries)

public:
	CIntervalsAndWordsSeries();			// protected constructor used by dynamic creation

// Attributes
public:
	CArray <longW, longW>  iistep;	// time on, time off
	int		nitems;					// number of on/off events

protected:
	int		version;			// current: 1

// Implementation
public:
	virtual			~CIntervalsAndWordsSeries();
	virtual void	Serialize(CArchive& ar);							// overridden for document i/o
	void			operator = (const CIntervalsAndWordsSeries& arg);	// operator redefinition
	void			ImportIntervalsSeries(CIntervalsSeries* pIntervals);
	CIntervalsSeries* ExportIntervalsSeries();
};