// StimLevelArray.cpp : implementation file

#include "stdafx.h"
#include "StimLevelArray.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CIntervalsSeries

IMPLEMENT_SERIAL(CIntervalsSeries, CObject, 0 /* schema number*/ )

CIntervalsSeries::CIntervalsSeries()
{
	iID=1;					// ID number of the array
	csDescriptor=_T("stimulus intervals");	// descriptor of the array
   	nitems=0;				// number of on & off events
	iistimulus.SetSize(0);		// time on, time off
	npercycle = 1;
	version = 1;
}

CIntervalsSeries::~CIntervalsSeries()
{
}

void CIntervalsSeries::operator = (const CIntervalsSeries& arg)
{
	iID			= arg.iID;						// ID number of the array
	csDescriptor= arg.csDescriptor;				// descriptor of the array
   	nitems		= arg.nitems;					// number of on/off events
	iistimulus.SetSize(arg.iistimulus.GetSize());

	for (int i=0; i < arg.iistimulus.GetSize(); i++)
		iistimulus.SetAt(i, arg.iistimulus.GetAt(i)); 	// time on, time off
	npercycle	= arg.npercycle;
}

long CIntervalsSeries::GetiiTime(int i)
{
	return iistimulus.GetAt(i);
}

void CIntervalsSeries::SetiiTime(int i, long iitime)
{
	iistimulus.SetAt(i, iitime);
}

void CIntervalsSeries::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		int n=4;
		ar << n;
		ar << iID;
		ar << nitems;	
		ar << npercycle;
		ar << version;

		n=1;
		ar << n;
		ar << csDescriptor;	
		n=1;
		ar << n;
		iistimulus.Serialize(ar);
	}
	else
	{
		int n;
		ar >> n;
		ar >> iID; n--;		
		ar >> nitems; n--;	
		npercycle = 1; if (n > 0) { ar >> npercycle; n--;}
		version = 1; if (n > 0) { ar >> version; n--; }	

		n=1;
		ar >> n;
		ar >> csDescriptor;	
		n=1;
		ar >> n;
		if (version == 1)
			iistimulus.Serialize(ar);
		else
			iistimulus.Serialize(ar);
	}
}
