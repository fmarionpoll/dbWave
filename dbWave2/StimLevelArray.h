#pragma once

// StimLevelArray.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CStimLevelSeries object


class CStimLevelSeries : public CObject
{
	DECLARE_SERIAL(CStimLevelSeries)

public :
	CStimLevelSeries();			// protected constructor used by dynamic creation
		
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
	virtual		~CStimLevelSeries();
	virtual void	Serialize(CArchive& ar);			// overridden for document i/o
	void	 operator = (const CStimLevelSeries& arg);	// operator redefinition
	long	GetiiTime(int i);
	void	SetiiTime(int i, long iitime);
};
