// StimLevelArray.cpp : implementation file

#include "stdafx.h"
#include "StimLevelArray.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CStimLevelSeries

IMPLEMENT_SERIAL(CStimLevelSeries, CObject, 0 /* schema number*/ )

CStimLevelSeries::CStimLevelSeries()
{
	iID=1;					// ID number of the array
	csDescriptor=_T("stimulus intervals");	// descriptor of the array
   	nitems=0;				// number of on & off events
	iisti.SetSize(0);		// time on, time off
	npercycle = 1;
}

CStimLevelSeries::~CStimLevelSeries()
{
}

void CStimLevelSeries::operator = (const CStimLevelSeries& arg)
{
	iID			= arg.iID;						// ID number of the array
	csDescriptor= arg.csDescriptor;				// descriptor of the array
   	nitems		= arg.nitems;					// number of on/off events
	iisti.SetSize(arg.iisti.GetSize());

	for (int i=0; i < arg.iisti.GetSize(); i++)
		iisti.SetAt(i, arg.iisti.GetAt(i)); 	// time on, time off
	npercycle	= arg.npercycle;
}

void CStimLevelSeries::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		int n=3;
		ar << n;
		ar << iID;				// ID number of the array
		ar << nitems;			// number of on/off events
		ar << npercycle;
		n=1;
		ar << n;
		ar << csDescriptor;		// descriptor of the array
		n=1;
		ar << n;
		iisti.Serialize(ar);
	}
	else
	{
		int n;
		ar >> n;
		ar >> iID; n--;			// ID number of the array
		ar >> nitems; n--;		// number of on/off events
		npercycle = 1; if (n > 0) { ar >> npercycle; n--;}	// number of repeated stimulations

		n=1;
		ar >> n;
		ar >> csDescriptor;		// descriptor of the array
		n=1;
		ar >> n;
		iisti.Serialize(ar);
	}
}
