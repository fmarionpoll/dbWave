// StimLevelArray.cpp : implementation file

#include "stdafx.h"
#include "StimLevelArray.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// ------------------------------------------------------

IMPLEMENT_SERIAL(CIntervalsArray, CObject, 0 )

CIntervalsArray::CIntervalsArray()
{
	iID=1;					// ID number of the array
	csDescriptor=_T("stimulus intervals");	// descriptor of the array
   	nitems=0;				// number of on & off events
	iistimulus.SetSize(0);	// time on, time off
	npercycle = 1;
	version = 1;
	ichan = 0;				// otherwise: 0, 1...7
}

CIntervalsArray::~CIntervalsArray()
{
}

void CIntervalsArray::operator = (const CIntervalsArray& arg)
{
	iID			= arg.iID;						// ID number of the array
	csDescriptor= arg.csDescriptor;				// descriptor of the array
   	nitems		= arg.nitems;					// number of on/off events
	iistimulus.SetSize(arg.iistimulus.GetSize());

	for (int i=0; i < arg.iistimulus.GetSize(); i++)
		iistimulus.SetAt(i, arg.iistimulus.GetAt(i)); 	// time on, time off
	npercycle	= arg.npercycle;
}

void CIntervalsArray::Serialize(CArchive& ar)
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

		ar >> n;
		ar >> csDescriptor;	n--;
		ar >> n;
		iistimulus.Serialize(ar); n--;
	}
}

// -------------------------------------------------------

IMPLEMENT_SERIAL(CIntervalPoint, CObject, 0 )

CIntervalPoint::CIntervalPoint()
{
	ii = 0;
	w = 0;
}

CIntervalPoint::CIntervalPoint(const CIntervalPoint& pt)
{
	ii = pt.ii;
	w = pt.w;
}

CIntervalPoint::~CIntervalPoint()
{
}

void CIntervalPoint::Serialize(CArchive & ar)
{
	if (ar.IsStoring())
	{
		int n = 1;
		ar << n;
		ar << ii;
		ar << w;
	}
	else
	{
		int n;
		ar >> n;
		ar >> ii;
		ar >> w;
	}
}

void CIntervalPoint::operator = (const CIntervalPoint & arg)
{
	ii = arg.ii;
	w = arg.w;
}

// --------------------------------------------------------

IMPLEMENT_SERIAL(CIntervalsAndWordsSeries, CObject, 0 )

CIntervalsAndWordsSeries::CIntervalsAndWordsSeries()
{
	version = 1;
	iistep.SetSize(0);
}

CIntervalsAndWordsSeries::CIntervalsAndWordsSeries(const CIntervalsAndWordsSeries& arg)
{
	int nitems = arg.iistep.GetSize();
	iistep.SetSize(nitems);
	for (int i = 0; i < nitems; i++)
		iistep[i] = arg.iistep.GetAt(i);
}

CIntervalsAndWordsSeries::~CIntervalsAndWordsSeries()
{
}

void CIntervalsAndWordsSeries::Serialize(CArchive & ar)
{
	if (ar.IsStoring())
	{
		int n = 1;
		ar << n;
		ar << version;

		n = 1;
		ar << n;
		iistep.Serialize(ar);
	}
	else
	{
		int n;
		ar >> n;
		version = 1; if (n > 0) { ar >> version; n--; }

		ar >> n;
		iistep.Serialize(ar);
	}
}

void CIntervalsAndWordsSeries::operator = (const CIntervalsAndWordsSeries & arg)
{
	int nitems = arg.iistep.GetSize();
	iistep.SetSize(nitems);
	for (int i = 0; i < nitems; i++)
		iistep[i] = arg.iistep.GetAt(i);
}

void CIntervalsAndWordsSeries::EraseAllData()
{
	iistep.RemoveAll();
}

void CIntervalsAndWordsSeries::ImportIntervalsSeries(CIntervalsArray * pIntervals, WORD valUP)
{
	int nitems = pIntervals->GetSize();
	iistep.SetSize(nitems);
	WORD wLOW = 0;
	WORD wUP = valUP;
	WORD wState = wUP;
	CIntervalPoint dummy;
	for (int i = 0; i < nitems; i++)
	{
		dummy.ii = pIntervals->GetiiTime(i);
		dummy.w = wState;
		iistep[i] = dummy;
		if (wState == wLOW)
			wState = wUP;
		else
			wState = wLOW;
	}
}

// combine up to 8 chans stored into CIntervalsArray(s). 
// in the resulting CIntervaAndWordsSeries, each bit is coding for a channel
// the channel number is sotred in the CIntervalsArray (parameter "ichan")
// 1) create separate CIntervalsAndWordsSeries objects with bits set 
// 2) merge the series

void CIntervalsAndWordsSeries::ImportAndMergeIntervalsArrays(CPtrArray* pSourceIntervals)
{
	iistep.RemoveAll();
	int nseries = pSourceIntervals->GetSize();
	if (nseries > 8)
		nseries = 8;
	int nintervals = 0;
	CPtrArray TransformedArrays;
	TransformedArrays.SetSize(8);

	// (1) transform series into CIntervalsAndWordSeries
	int iseries = 0;
	for (int i = 0; i < nseries; i++) 
	{
		// transform this series if not empty
		CIntervalsArray * pSource = (CIntervalsArray *) pSourceIntervals->GetAt(i);
		if (pSource->GetSize() == 0)
			continue;

		CIntervalsAndWordsSeries* pTransf = new CIntervalsAndWordsSeries();
		WORD valUP = 2 << pSource->GetChan();
		pTransf->ImportIntervalsSeries(pSource, valUP);
		TransformedArrays[iseries] = pTransf;
		nintervals += pTransf->GetSize();
		iseries++;
	}

	// (2) now ptrInter contains 8 channels with a list of ON/OFF states (with the proper bit set)
	// merge the 8 channels

	WORD outputState = 0;
	for (int i = 0; i < iseries; i++)
	{
		CIntervalsAndWordsSeries* pTransf = (CIntervalsAndWordsSeries*) TransformedArrays.GetAt(i);
		if (pTransf == NULL)
			continue;

		// loop over all intervals stored in this transformed series 
		int k = 0;
		for (int j = 0; j < pTransf->GetSize(); j++)
		{
			CIntervalPoint pt = (pTransf->iistep).GetAt(j);
			BOOL bFound = false;

			// loop over all intervals stored into the local array and merge output state
			for (k; k < iistep.GetSize(); k++)
			{
				if (pt.ii < iistep.GetAt(k).ii)
				{
					pt.w = outputState & pt.w;	// merge with previous status
					iistep.InsertAt(k, pt);
					bFound = true;
					break;
				}
				else if (pt.ii == iistep.GetAt(k).ii)
				{
					pt.w = iistep.GetAt(k).w & pt.w;	// merge with current status
					iistep.SetAt(k, pt);
					bFound = true;
					break;
				}
				outputState = iistep.GetAt(k).w;		// update output and continue
			}

			// not found into existing intervals? add new interval
			if (!bFound)
			{
				pt.w = outputState & pt.w;
				iistep.Add(pt);
			}
		}
	}

	// (3) delete pTransf objects

	for (int i = 0; i < 8; i++)
	{
		CIntervalsAndWordsSeries* pInterv = (CIntervalsAndWordsSeries*)TransformedArrays.GetAt(i);
		if (pInterv != NULL)
			delete pInterv;
	}
	TransformedArrays.RemoveAll();
}


void CIntervalsAndWordsSeries::ExportIntervalsSeries(int chan, CIntervalsArray * pOut)
{
	WORD ifilter = 2 << chan;
	WORD istatus = 0;
	for (int i = 0; i < iistep.GetSize(); i++)
	{
		if ((ifilter & iistep[i].w) != istatus)
		{
			istatus = ifilter & iistep[i].w;
			pOut->AddInterval(iistep[i].ii);
		}
	}
}


