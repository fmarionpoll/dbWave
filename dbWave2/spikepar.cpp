// spikepar.cpp    implementation file
//
// this file defines structures used to store infos related to
// user choices ie for spike detection, sorting, measuring,
// and printing. These structures are read from awave.par by
// awave32.cpp during first initialisation, & stored at exit.
// The name of this parameter file and its location can be changed
// from within a dialog box (options)
//
// STIMDETECT				version 1
// SPKDETECTPARM			version 1
// CSpkDetectionArray		version 4
// SPKDETECTARRAY			version 4
// SPKCLASSIF				version 1
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "scopescr.h"
#include "TemplateWnd.h"
#include "TemplateListWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//------------------ class STIMDETECT ---------------------------------

IMPLEMENT_SERIAL(STIMDETECT, CObject, 0)

STIMDETECT::STIMDETECT()
{
	wversion = 2;
	nItems=0;			// number of items/line (nb of chans/detection) ?
	SourceChan=0;		// source channel
	TransformMethod=0;	// detect from data transformed - i = transform method cf CAcqDataDoc
	DetectMethod=0;		// detection method see CDetectionView for methods
	Threshold1=0;		// value of threshold 1	
	bMode = 0;			// ON/OFF
}

STIMDETECT::~STIMDETECT()
{
}

void STIMDETECT::operator = (const STIMDETECT& arg)
{
	//comment=arg.comment;	// CArchive
	nItems=arg.nItems;			// number of items/line (nb of chans/detection) ?
	SourceChan=arg.SourceChan;
	TransformMethod=arg.TransformMethod;
	DetectMethod=arg.DetectMethod;
	Threshold1=arg.Threshold1;	
	bMode = arg.bMode;
}

void STIMDETECT::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << wversion;
		ar << nItems;
		ar << SourceChan;
		ar << TransformMethod;
		ar << DetectMethod;
		ar << Threshold1;
		ar << bMode;
	} 
	else
	{		
		WORD version;  ar >> version;		
		// read data (version 1)
		if (version == 1)
		{
			WORD w1;
			ar >> w1; nItems=w1;
			ar >> w1; SourceChan=w1;
			ar >> w1; TransformMethod=w1;
			ar >> w1; DetectMethod=w1;
			ar >> w1; Threshold1=w1;
		}
		else if (version > 1)
		{
			ar >> nItems;
			ar >> SourceChan;
			ar >> TransformMethod;
			ar >> DetectMethod;
			ar >> Threshold1;
			ar >> bMode;
		}
	}
}


// ------------------------ class CSpkDetectArray ---------------------

IMPLEMENT_SERIAL(CSpkDetectArray, CObject, 0)

CSpkDetectArray::CSpkDetectArray()
{
	bChanged = FALSE;
	wversion = 4;
	SPKDETECTPARM* pparm = new SPKDETECTPARM;
	ASSERT(pparm != NULL);
	parmItems.SetSize(0);
	parmItems.Add(pparm);
}

CSpkDetectArray::~CSpkDetectArray()
{
	DeleteArray();
}

void CSpkDetectArray::DeleteArray()
{
	int isize= parmItems.GetSize();
	for (int i=0; i<isize; i++)
	{
		SPKDETECTPARM* pparm = (SPKDETECTPARM*) parmItems[i];
		delete pparm;
	}
	parmItems.RemoveAll();
}

void CSpkDetectArray::SetSize(int nitems)
{
	int isize = parmItems.GetSize();
	// delete items
	if (isize > nitems)
	{
		for (int i=isize-1; i>= nitems; i--)
		{
			SPKDETECTPARM* pparm = (SPKDETECTPARM*) parmItems[i];
			delete pparm;		
		}
		parmItems.SetSize(nitems);
	}
	// add dummy items
	else if (isize < nitems)
	{
		for (int i=isize; i<nitems; i++)
			AddItem();
	}	
}

// insert one parameter array item
int CSpkDetectArray::AddItem()
{
	SPKDETECTPARM* pparm = new SPKDETECTPARM;
	ASSERT(pparm != NULL);
	parmItems.Add(pparm);
	return parmItems.GetSize();
}

// delete one parameter array item
// return isize left
int	CSpkDetectArray::RemoveItem(int ichan)
{
	int isize = parmItems.GetSize() - 1;
	if (ichan > isize)
		return -1;

	SPKDETECTPARM* pparm = (SPKDETECTPARM*) parmItems[ichan];
	delete pparm;
	parmItems.RemoveAt(ichan);
	return isize;
}

void CSpkDetectArray::operator = (const CSpkDetectArray& arg)
{
	//comment=arg.comment;
	int nItems=arg.parmItems.GetSize();	// number of items
	SetSize(nItems);			// erase old data	
	for (int i=0; i<nItems; i++)		// loop over nItems
		*((SPKDETECTPARM*) parmItems[i])=*((SPKDETECTPARM*) arg.parmItems[i]);
}

void CSpkDetectArray::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{   
		ar << wversion;
		WORD nItems = parmItems.GetSize();
		ar << nItems;
		for (int i=0; i<nItems; i++)
			((SPKDETECTPARM*) parmItems[i])->Serialize(ar);	
	} 
	else
	{
		WORD version;  ar >> version;
		// version 1 (11-2-96 FMP)
		Serialize_Load(ar, version);
	}    	
}

void CSpkDetectArray::Serialize_Load(CArchive& ar, WORD wversion)
{
	ASSERT(ar.IsLoading());
	WORD nItems; ar >> nItems;
	SetSize(nItems);		
	for (int i=0; i<nItems; i++)
		((SPKDETECTPARM*)parmItems[i])->Serialize(ar);
	if (wversion > 1 && wversion < 3)
	{
		int dummy;
		ar >> dummy; ar >> dummy; ar >> dummy; ar >> dummy;
	}
}

//------------------ class SPKDETECTARRAY ---------------------------------

IMPLEMENT_SERIAL(SPKDETECTARRAY, CObject, 0)

SPKDETECTARRAY::SPKDETECTARRAY()
{
	bChanged = FALSE;
	wversion = 4;
}

SPKDETECTARRAY::~SPKDETECTARRAY()
{
	DeleteAll();
}

// erase all arrays of parmItems (and all parmItems within them)
void SPKDETECTARRAY::DeleteAll()
{
	POSITION pos = chanArrayMap.GetStartPosition();
	void* ptr = NULL;
	WORD wKey;
	while (pos) 
	{
		chanArrayMap.GetNextAssoc(pos, wKey, ptr);
		CSpkDetectArray* pspk = (CSpkDetectArray*) ptr;
		ASSERT_VALID(pspk);
		delete pspk;
		pspk = NULL;
	}
	chanArrayMap.RemoveAll();
}

// check if array is ok, if not, increase size of array
// create empty CPtrArray if necessary
CSpkDetectArray* SPKDETECTARRAY::GetChanArray(int acqchan)
{
	void* ptr = NULL;
	BOOL flag = chanArrayMap.Lookup(acqchan, ptr);
	if (!flag)
	{
		CSpkDetectArray* pspk = new CSpkDetectArray;
		ptr = pspk;
		chanArrayMap.SetAt(acqchan, ptr);
	}
	return (CSpkDetectArray*) ptr;
}

void SPKDETECTARRAY::SetChanArray(int acqchan, CSpkDetectArray* pspk)
{
	void* ptr = pspk;
	chanArrayMap.SetAt(acqchan, ptr);
}

void SPKDETECTARRAY::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{   
		wversion = 4;
		ar << wversion;		// wversion = 4
		int narrays = chanArrayMap.GetSize();
		ar << narrays;
		POSITION pos = chanArrayMap.GetStartPosition();
		void* ptr = NULL;
		WORD wKey;
		while (pos) 
		{
			chanArrayMap.GetNextAssoc(pos, wKey, ptr);
			ar << wKey;
			CSpkDetectArray* pspk = (CSpkDetectArray*) ptr;
			ASSERT_VALID(pspk);
			pspk->Serialize(ar);
		}
	} 
	else
	{
		DeleteAll();
		WORD version;  ar >> version;
		// version 1 (11-2-96 FMP)
		if (version < 4)
		{
			int narrays = 1;
			CSpkDetectArray* parmItems = new CSpkDetectArray;
			parmItems->Serialize_Load(ar, version);
			WORD wKey = 1;
			void* ptr = parmItems;
			chanArrayMap.SetAt(wKey, ptr);
		}
		else if (version >= 4)
		{
			int narrays;
			ar >> narrays;
			for (int j=0; j < narrays; j++)
			{
				WORD wKey;
				ar >> wKey;
				CSpkDetectArray* pspk = new CSpkDetectArray;
				pspk->Serialize(ar);
				void* ptr = pspk;
				chanArrayMap.SetAt(wKey, ptr);
			}
		}
	}    	
}

//--------------- class SPKDETECTPARM ---------------------------------

IMPLEMENT_SERIAL(SPKDETECTPARM, CObject, 0 /* schema number*/ )

SPKDETECTPARM::SPKDETECTPARM()
{
	wversion = 7;				// version 6 (Aug 19 2005 FMP)
	detectChan=0;				// source channel
	detectTransform=13;			// detect from data transformed - i = transform method cf CAcqDataDoc
	detectFrom=0;				// detection method 0=data, 1=tags
	extractChan=0;
	extractTransform=13;
	compensateBaseline=FALSE;
	detectThreshold=0;			// value of threshold 1	
	extractNpoints=60;			// spike length (n data pts)
	prethreshold=20;			// offset spike npts before threshold
	refractory=20;				// re-start detection n pts after threshold
	detectThresholdmV = 0.5f;	// detection threshold in mV
	detectWhat=0;				// detect spikes, 1=detect stimulus
	detectMode=0;				// if sti, = ON/OFF
}

SPKDETECTPARM::~SPKDETECTPARM()
{
}

void SPKDETECTPARM::operator = (const SPKDETECTPARM& arg)
{
	comment				=arg.comment;	
	detectFrom	 		=arg.detectFrom;
	detectChan 			=arg.detectChan;	
	detectTransform 	=arg.detectTransform;
	detectThreshold		=arg.detectThreshold;
	compensateBaseline	=arg.compensateBaseline;
	extractChan			=arg.extractChan;
	extractTransform	=arg.extractTransform;
	extractNpoints 		=arg.extractNpoints;
	prethreshold 		=arg.prethreshold;
	refractory			=arg.refractory;
	detectThresholdmV   =arg.detectThresholdmV;
	detectWhat			=arg.detectWhat;
	detectMode			=arg.detectMode;
}

void SPKDETECTPARM::ReadVersionlessthan6(CArchive& ar, int version)
{
	long lw;
	WORD wi;
	ar >> comment;							// CString
	ar >> lw; detectChan = lw;				// long
	ar >> lw; detectTransform = lw;			// long
	ar >> wi; detectFrom = wi;				// WORD
	ar >> wi; compensateBaseline = wi;		// WORD
	ar >> lw; detectThreshold = lw;			// long
	if (version < 3)			// dummy reading (threshold2: removed version 3)
		ar >> lw;
	ar >> lw; extractNpoints = lw;			// long
	ar >> lw; prethreshold = lw;			// long
	ar >> lw; refractory = lw;				// long
	if (version < 3)						// dummy reading (threshold adjust method)
	{										// removed at version 3, moved to SPKdetectARRAY
		ar >> lw; // (int)
		float fw; ar >> fw;
		ar >> lw; ar >> lw;
	}
	if (version > 1)		// added at version 2
	{
		ar >> lw; extractChan = lw;			// long
		ar >> lw; extractTransform = lw;	// long
	}
}

void SPKDETECTPARM::ReadVersion6(CArchive& ar)
{
	int nitems;
	ar >> nitems;
	ar >> comment; nitems--; ASSERT(nitems == 0);

	ar >> nitems;
	ar >> detectFrom; nitems--; 
	ar >> compensateBaseline; nitems--; ASSERT(nitems == 0);

	ar >> nitems;
	ar >> detectChan; nitems--; 
	ar >> detectTransform; nitems--; 
	ar >> detectThreshold; nitems--; 
	ar >> extractNpoints; nitems--; 
	ar >> prethreshold; nitems--; 
	ar >> refractory; nitems--; 
	ar >> extractChan; nitems--; 
	ar >> extractTransform; nitems--; 
	if (nitems > 0) ar >> detectWhat; nitems --;
	if (nitems > 0) ar >> detectMode; nitems--;

	ar >> nitems;
	ar >> detectThresholdmV; nitems--; ASSERT(nitems == 0);
}

void SPKDETECTPARM::ReadVersion7(CArchive& ar)
{
	int nitems;
	// string parameters
	ar >> nitems;
	ar >> comment; nitems--; ASSERT(nitems == 0);
	// int parameters
	ar >> nitems;
	ar >> detectFrom; nitems--; 
	ar >> compensateBaseline; nitems--;
	ar >> detectChan; nitems--; 
	ar >> detectTransform; nitems--; 
	ar >> detectThreshold; nitems--; 
	ar >> extractNpoints; nitems--; 
	ar >> prethreshold; nitems--; 
	ar >> refractory; nitems--; 
	ar >> extractChan; nitems--; 
	ar >> extractTransform; nitems--; 
	ar >> detectWhat; nitems--;
	ar >> detectMode; nitems --; 

	ASSERT(nitems == 0);
	// float
	ar >> nitems;
	ar >> detectThresholdmV; nitems--; 
	ASSERT(nitems == 0);
}

void SPKDETECTPARM::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{   
		// version 7 (Aug 19 2005 FMP)
		ar <<  wversion;
		int nitems = 1; ar << nitems;
		ar << comment;

		nitems = 12; ar << nitems;
		ar << detectFrom;
		ar << compensateBaseline;
		ar << detectChan;
		ar << detectTransform;
		ar << detectThreshold;
		ar << extractNpoints;
		ar << prethreshold;
		ar << refractory;
		ar << extractChan;
		ar << extractTransform;
		ar << detectWhat;
		ar << detectMode;
		
		nitems= 1; ar << nitems;
		ar << detectThresholdmV;
	} 
	else
	{
		WORD version;  ar >> version;
		if (version != wversion)
		{
			if (version > 0 && version < 5)
				ReadVersionlessthan6(ar, version);
			else if (version <7) //== 6)
				ReadVersion6(ar);
			else
			{	
				ASSERT(FALSE);
				ATLTRACE2("error: unrecognizable version %i\n", version);
			}
		}
		else 
			ReadVersion7(ar);
	}
}

//------------------ class SPKCLASSIF ---------------------------------

IMPLEMENT_SERIAL(SPKCLASSIF, CObject, 0 /* schema number*/ )

SPKCLASSIF::SPKCLASSIF()
{	
	wversion = 2;
	dataTransform=0;// data transform method (0=raw data)
	iparameter=0;	// type of parameter measured
	ileft=10;		// position of first cursor
	iright=40;		// position of second cursor
	ilower=0;		// second threshold
	iupper=10;		// first threshold
	ixyleft= 10;
	ixyright= 40;
	sourceclass=0;	// source class
	destclass=0;	// destination class	
	hitrate= 50;
	hitratesort = 75;
	ktolerance = 1.96f;
	kleft = 10;
	kright = 40;
	rowheight=100;	// height of the spike row within spikeview
	coltext=-1;
	colspikes=100;		// width of the spikes within one row
	colseparator=5;
	ptpl = NULL;
	mvmin =0.f;
	mvmax= 2.f;
	vdestclass = 1;
	vsourceclass = 0;
	fjitter_ms = 1.f;
	bresetzoom = TRUE;
}

SPKCLASSIF::~SPKCLASSIF()
{
	if (ptpl)
		delete ((CTemplateListWnd*)ptpl);
}

void SPKCLASSIF::operator = (const SPKCLASSIF& arg)
{
	dataTransform = arg.dataTransform;// transform mode
	iparameter = arg.iparameter;	// type of parameter measured
	ileft = arg.ileft;				// position of first cursor
	iright = arg.iright;			// position of second cursor
	ilower = arg.ilower;			// second threshold
	iupper = arg.iupper;			// first threshold
	ixyright= arg.ixyright;
	ixyleft= arg.ixyleft;
	sourceclass = arg.sourceclass;	// source class
	destclass = arg.destclass;		// destination class

	hitrate = arg.hitrate;
	hitratesort = arg.hitratesort;
	ktolerance = arg.ktolerance;
	kleft = arg.kleft;
	kright = arg.kright;
	rowheight = arg.rowheight;
	coltext= arg.coltext;
	colspikes= arg.colspikes;
	colseparator= arg.colseparator;
	vsourceclass = arg.vsourceclass;	// source class
	vdestclass = arg.vdestclass;		// destination class
	bresetzoom = arg.bresetzoom;
	fjitter_ms = arg.fjitter_ms;

	mvmin = arg.mvmin;
	mvmax = arg.mvmax;
	
	if (arg.ptpl != NULL)
	{
		ptpl = new (CTemplateListWnd);
		*((CTemplateListWnd*) ptpl) = *((CTemplateListWnd*) arg.ptpl);
	}

}

void SPKCLASSIF::Serialize(CArchive& ar)
{
	BOOL btplIspresent=FALSE;
	if (ar.IsStoring())
	{
		wversion = 2;
		ar << wversion;
		ar << (WORD) dataTransform;
		ar << (WORD) iparameter;
		ar << (WORD) ileft;
		ar << (WORD) iright;
		ar << (WORD) ilower;
		ar << (WORD) iupper;
		WORD dummy = 0;
		ar << dummy;
		ar << dummy;

		nfloatparms = 4;
		ar << nfloatparms;
		ar << ktolerance;		// 1
		ar << mvmin;
		ar << mvmax;
		ar << fjitter_ms;

		nintparms = 16;
		ar << nintparms;
		ar << kleft;			// 1
		ar << kright;			// 2
		ar << rowheight;		// 3
		ar << hitrate;			// 4
		ar << hitratesort;		// 5
		btplIspresent = (ptpl != NULL);	// test if templatelist is present
		ar << btplIspresent;	// 6
		ar << coltext;			// 7
		ar << colspikes;		// 8
		ar << colseparator;		// 9
		ar << sourceclass;		// 10
		ar << destclass;		// 11
		ar << vsourceclass;		// 12
		ar << vdestclass;		// 13
		ar << bresetzoom;		// 14
		ar << ixyright;			// 15
		ar << ixyleft;			// 16
	} 
	else
	{
		WORD version;  ar >> version;

		// version 1
		WORD w1;
		ar >> w1; dataTransform=w1;
		ar >> w1; iparameter=w1;
		ar >> w1; ileft=w1;
		ar >> w1; iright=w1;
		ar >> w1; ilower=w1;
		ar >> w1; iupper=w1;
		ar >> w1; sourceclass=w1;
		ar >> w1; destclass=w1;

		// version 2
		if (version > 1)
		{
			// float parameters
			int nfparms; ar >> nfparms;
			if (nfparms > 0) { ar >> ktolerance; nfparms--;}
			if (nfparms > 0) { ar >> mvmin; nfparms--;}
			if (nfparms > 0) { ar >> mvmax; nfparms--;}
			if (nfparms > 0) { ar >> fjitter_ms; nfparms--;}
			if (nfparms > 0)
			{
				float dummy;
				do {ar >> dummy; nfparms--;} while (nfparms > 0);
			}
			ASSERT(nfparms == 0);

			// int parameters
			ar >> nfparms;
			if (nfparms > 0) { ar >> kleft; nfparms--;}
			if (nfparms > 0) { ar >> kright; nfparms--;}
			if (nfparms > 0) { ar >> rowheight; nfparms--;}			
			if (nfparms > 0) { ar >> hitrate; nfparms--;}
			if (nfparms > 0) { ar >> hitratesort;  nfparms--;}
			if (nfparms > 0) { ar >> btplIspresent;  nfparms--;}
			if (nfparms > 0) { ar >> coltext;  nfparms--;}
			if (nfparms > 0) { ar >> colspikes;  nfparms--;}
			if (nfparms > 0) { ar >> colseparator;  nfparms--;}
			if (nfparms > 0) { ar >> sourceclass;  nfparms--;}
			if (nfparms > 0) { ar >> destclass;  nfparms--;}
			if (nfparms > 0) { ar >> vsourceclass;  nfparms--;}
			if (nfparms > 0) { ar >> vdestclass;  nfparms--;}
			if (nfparms > 0) { ar >> bresetzoom;  nfparms--;}
			if (nfparms > 0) { ar >> ixyright;  nfparms--;}
			if (nfparms > 0) { ar >> ixyleft;  nfparms--;}

			if (nfparms > 0)
			{
				int dummy;
				do {ar >> dummy; nfparms--;} while (nfparms > 0);
			}
			ASSERT(nfparms == 0);

			if (!btplIspresent && ptpl != NULL)
				delete (CTemplateListWnd*)ptpl;
		}
	}

	// serialize templates
	if (btplIspresent)
	{
		if (ptpl == NULL)
			ptpl = new (CTemplateListWnd);
		((CTemplateListWnd*)ptpl)->Serialize(ar);
	}
}

void SPKCLASSIF::CreateTPL()
{
	ptpl = new (CTemplateListWnd);
}