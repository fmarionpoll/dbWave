// StimLevelArray.cpp : implementation file

#include "StdAfx.h"
#include "StimLevelArray.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// ------------------------------------------------------

IMPLEMENT_SERIAL(CIntervalsAndLevels, CObject, 0 )

CIntervalsAndLevels::CIntervalsAndLevels()
{
	iID=1;					// ID number of the array
	csDescriptor=_T("stimulus intervals");	// descriptor of the array
   	nitems=0;				// number of on & off events
	intervalsArray.SetSize(0);	// time on, time off
	npercycle = 1;
	version = 4;
	ichan = 0;				// otherwise: 0, 1...7
	chrate = 10000.;
}

CIntervalsAndLevels::~CIntervalsAndLevels()
{
	intervalsArray.RemoveAll();
}

void CIntervalsAndLevels::operator = (const CIntervalsAndLevels& arg)
{
	iID			= arg.iID;						// ID number of the array
	csDescriptor= arg.csDescriptor;				// descriptor of the array
   	nitems		= arg.nitems;					// number of on/off events
	intervalsArray.SetSize(arg.intervalsArray.GetSize());

	for (int i=0; i < arg.intervalsArray.GetSize(); i++)
		intervalsArray.SetAt(i, arg.intervalsArray.GetAt(i)); 	// time on, time off
	npercycle	= arg.npercycle;
	chrate		= arg.chrate;
}

void CIntervalsAndLevels::Serialize(CArchive& ar)
{
	int iversion = 2;
	if (ar.IsStoring())
	{
		int n=4; 
		ar << n;
		ar << iID;
		ar << nitems;	
		ar << npercycle;
		ar << iversion;

		n=1; ar << n;
		ar << csDescriptor;	

		n = 1; ar << n;
		intervalsArray.Serialize(ar);

		n = 1; ar << n;
		ar << chrate;
	}
	else
	{
		int n; ar >> n;
		ar >> iID; n--;		
		ar >> nitems; n--;	
		npercycle = 1; if (n > 0)  ar >> npercycle; n--;

		if (n > 0) {
			ar >> iversion; n--;
			ASSERT(iversion == 2);

			ar >> n;
			if (n > 0) ar >> csDescriptor; n--;
			ar >> n;
			if (n > 0) intervalsArray.Serialize(ar); n--;
			if (iversion > 1)
				ar >> n;
			if (n > 0) ar >> chrate; n--;
		}
		else // old version
		{
			ar >> n;
			ar >> csDescriptor;		// descriptor of the array
			ar >> n;
			intervalsArray.Serialize(ar);
		}
	}
}

// -------------------------------------------------------
//
//IMPLEMENT_SERIAL(CIntervalPoint, CObject, 0 )
//
//CIntervalPoint::CIntervalPoint()
//{
//	ii = 0;
//	w = 0;
//}
//
//CIntervalPoint::CIntervalPoint(const CIntervalPoint& pt)
//{
//	ii = pt.ii;
//	w = pt.w;
//}
//
//CIntervalPoint::~CIntervalPoint()
//{
//}
//
//void CIntervalPoint::Serialize(CArchive & ar)
//{
//	if (ar.IsStoring())
//	{
//		int n = 2;
//		ar << n;
//		ar << ii;
//		ar << w;
//	}
//	else
//	{
//		int n;
//		ar >> n;
//		if (n>0) ar >> ii;	n--;
//		if (n>0) ar >> w;	n--;
//	}
//}
//
//void CIntervalPoint::operator = (const CIntervalPoint & arg)
//{
//	ii = arg.ii;
//	w = arg.w;
//}

// --------------------------------------------------------

IMPLEMENT_SERIAL(CIntervalsAndWordsSeries, CObject, 0 )

CIntervalsAndWordsSeries::CIntervalsAndWordsSeries()
{
	version = 2;
	iistep.SetSize(0);
	chrate = 10000.;
}

CIntervalsAndWordsSeries::CIntervalsAndWordsSeries(const CIntervalsAndWordsSeries& arg)
{
	int nitems = arg.iistep.GetSize();
	iistep.SetSize(nitems);
	for (int i = 0; i < nitems; i++)
		iistep[i] = arg.iistep.GetAt(i);
	chrate = arg.chrate;
}

CIntervalsAndWordsSeries::~CIntervalsAndWordsSeries()
{
}

void CIntervalsAndWordsSeries::Serialize(CArchive & ar)
{
	int lversion = 2;
	if (ar.IsStoring())
	{
		int n = 1;
		ar << n;
		ar << lversion;
		n = 1;
		ar << n;
		iistep.Serialize(ar);
		n = 1;
		ar << n;
		ar << chrate;
	}
	else
	{
		int n;
		ar >> n;
		if (n > 0) ar >> lversion; n--; 
		ar >> n;
		if (n > 0) iistep.Serialize(ar); n--;
		if (lversion > 1) ar >> n;
		if (n > 0) ar >> chrate; n--;
	}
}

void CIntervalsAndWordsSeries::operator = (const CIntervalsAndWordsSeries & arg)
{
	int nitems = arg.iistep.GetSize();
	iistep.SetSize(nitems);
	for (int i = 0; i < nitems; i++)
		iistep[i] = arg.iistep.GetAt(i);
	chrate = arg.chrate;
}

CIntervalPoint  CIntervalsAndWordsSeries::GetIntervalPointAt(int i)
{ 
	CIntervalPoint pt = iistep[i];
	return pt; 
}

void CIntervalsAndWordsSeries::EraseAllData()
{
	iistep.RemoveAll();
}

void CIntervalsAndWordsSeries::ImportIntervalsSeries(CIntervalsAndLevels * pIntervals, WORD valUP, BOOL bcopyRate)
{	
	float ichrate = pIntervals->chrate;
	if (bcopyRate)
		chrate = pIntervals->chrate;
	double ratio = chrate / ichrate;
	int nitems = pIntervals->GetSize();
	iistep.SetSize(nitems);
	WORD wLOW = 0;
	WORD wUP = valUP;
	WORD wState = wUP;
	CIntervalPoint dummy;
	for (int i = 0; i < nitems; i++)
	{
		dummy.ii = (long) (pIntervals->GetiiTime(i) * ratio);
		dummy.w = wState;
		iistep[i] = dummy;
		if (wState == wLOW)
			wState = wUP;
		else
			wState = wLOW;
	}
}

// combine up to 8 chans stored into CIntervalsAndLevels(s). 
// in the resulting CIntervaAndWordsSeries, each bit is coding for a channel
// the channel number is sotred in the CIntervalsAndLevels (parameter "ichan")
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
		CIntervalsAndLevels * pSource = (CIntervalsAndLevels *) pSourceIntervals->GetAt(i);
		if (pSource->GetSize() == 0)
			continue;

		CIntervalsAndWordsSeries* pTransf = new CIntervalsAndWordsSeries();
		WORD valUP = 2 << pSource->GetChan();
		pTransf->ImportIntervalsSeries(pSource, valUP, FALSE);
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
		if (pTransf == nullptr)
			continue;

		// loop over all intervals stored in this transformed series 
		int k = 0;
		for (int j = 0; j < pTransf->GetSize(); j++)
		{
			CIntervalPoint pt = pTransf->iistep[j];
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
		if (pInterv != nullptr)
			delete pInterv;
	}
	TransformedArrays.RemoveAll();
}


void CIntervalsAndWordsSeries::ExportIntervalsSeries(int chan, CIntervalsAndLevels * pOut)
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


