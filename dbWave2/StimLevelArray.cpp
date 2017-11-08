// StimLevelArray.cpp : implementation file

#include "stdafx.h"
#include "StimLevelArray.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CIntervalsArray

IMPLEMENT_SERIAL(CIntervalsArray, CObject, 0 /* schema number*/ )

CIntervalsArray::CIntervalsArray()
{
	iID=1;					// ID number of the array
	csDescriptor=_T("stimulus intervals");	// descriptor of the array
   	nitems=0;				// number of on & off events
	iistimulus.SetSize(0);	// time on, time off
	npercycle = 1;
	version = 1;
	ichan = -1;				// -1 = undefined; otherwise: 0, 1...7
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

CIntervalPoint::CIntervalPoint()
{
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

void CIntervalPoint::operator=(const CIntervalPoint & arg)
{
	ii = arg.ii;
	w = arg.w;
}

// --------------------------------------------------------

CIntervalsAndWordsSeries::CIntervalsAndWordsSeries()
{
	version = 1;
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

void CIntervalsAndWordsSeries::operator=(const CIntervalsAndWordsSeries & arg)
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
	WORD wState = wLOW;
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

// combine up to 8 chans
void CIntervalsAndWordsSeries::ImportAndMergeIntervalsArrays(CPtrArray* pIntervals)
{
	iistep.RemoveAll();
	int nseries = pIntervals->GetSize();
	if (nseries > 8)
		nseries = 8;
	int nintervals = 0;
	CPtrArray IntervAndWordsSeriesPtrArray;
	IntervAndWordsSeriesPtrArray.SetSize(8);

	// transform series into CIntervalsAndWordSeries
	int iseries = 0;
	for (int i = 0; i < nseries; i++) 
	{
		// transform this series
		IntervAndWordsSeriesPtrArray[i] = NULL;
		CIntervalsArray * pptr = (CIntervalsArray *) pIntervals->GetAt(i);
		if (pptr->GetSize() == 0)
			continue;

		CIntervalsAndWordsSeries* ptr = new CIntervalsAndWordsSeries();
		WORD valUP = 2 * pptr->GetChan();
		ptr->ImportIntervalsSeries(pptr, valUP);
		IntervAndWordsSeriesPtrArray[iseries] = ptr;
		nintervals += ptr->GetSize();
		iseries++;
	}

	// now ptrInter contains 8 channels with a list of ON/OFF states (with the proper bit set)
	// merge the 8 channels
	WORD outputState = 0;
	for (int i = 0; i < iseries; i++)
	{
		CIntervalsAndWordsSeries* pIntervArray = (CIntervalsAndWordsSeries*) IntervAndWordsSeriesPtrArray->GetAt(i);
		if (pIntervArray == NULL)
			continue;

		// loop over all intervals stored in this transformed series 
		int k = 0;
		for (int j = 0; j < pIntervArray->GetSize(); j++)
		{
			CIntervalPoint pt = pIntervArray->iistep;
			BOOL bFound = false;

			// loop over all intervals stored into the local array and merge output state
			for (k; k < iistep.GetSize(); k++)
			{
				if (pt.ii < iistep.ii)
				{
					pt.w = outputState & pt.w;	// merge with previous status
					iistep.InsertAt(k, pt);
					bFound = true;
					break;
				}
				else if (pt.ii == iistep.ii)
				{
					pt.w = iistep.w & pt.w;		// merge with current status
					iistep.SetAt(k, pt);
					bFound = true;
					break;
				}
				outputState = iistep.w;			// update output and continue
			}

			// not found into existing intervals? add new interval
			if (!bFound)
			{
				pt.w = outputState & pt.w;
				iistep.Add(pt);
			}
		}
	}

	// delete ptr array
	for (int i = 0; i < 8; i++)
	{
		CIntervalsAndWordsSeries* pInterv = (CIntervalsAndWordsSeries*) ptrInterv.GetAt(i);
		if (pInterv != NULL)
			delete pInterv;
	}
	IntervAndWordsSeriesPtrArray.RemoveAll();
}

CIntervalsArray * CIntervalsAndWordsSeries::ExportIntervalsSeries(int chan)
{
	CIntervalsArray * ptr = new CIntervalsArray();
	WORD ifilter = 2 * * chan;
	WORD istatus = 0;
	for (int i = 0; i < iistep.GetSize(); i++)
	{
		if (ifilter & iistep[i].w != istatus)
		{
			istatus = ifilter & iistep[i].w;
			ptr->AddInterval(iistem[i].ii);
		}
	}
	return ptr;
}


