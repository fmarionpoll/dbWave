#include "StdAfx.h"
#include "DataListCtrl_Row.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_SERIAL(CDataListCtrl_Row, CObject, 0)

CDataListCtrl_Row::CDataListCtrl_Row()
{
}

CDataListCtrl_Row::CDataListCtrl_Row(int i)
{
	index = i;
}

CDataListCtrl_Row::~CDataListCtrl_Row()
{
	if (pDataChartWnd != nullptr)
	{
		delete pDataChartWnd;
	}
	if (pSpikeChartWnd != nullptr)
	{
		delete pSpikeChartWnd;
	}
	SAFE_DELETE(pdataDoc);
	SAFE_DELETE(pspikeDoc);
	cs_comment.Empty();
	csDatafileName.Empty();
	csSpikefileName.Empty();
	csSensillumname.Empty();
	csStim1.Empty();
	csConc1.Empty();
	csStim2.Empty();
	csConc2.Empty();
	csNspk.Empty();
	csFlag.Empty();
	csDate.Empty();
}

CDataListCtrl_Row& CDataListCtrl_Row::operator =(const CDataListCtrl_Row& arg)
{
	if (this != &arg)
	{
		bInit = arg.bInit;
		index = arg.index;
		insectID = arg.insectID;
		cs_comment = arg.cs_comment;
		csDatafileName = arg.csDatafileName;
		csSpikefileName = arg.csSpikefileName;
		csSensillumname = arg.csSensillumname;
		csStim1 = arg.csStim1;
		csConc1 = arg.csConc1;
		csStim2 = arg.csStim2;
		csConc2 = arg.csConc2;
		csNspk = arg.csNspk;
		csFlag = arg.csFlag;
		csDate = arg.csDate;
		pDataChartWnd = arg.pDataChartWnd;
		pSpikeChartWnd = arg.pSpikeChartWnd;
		pdataDoc = arg.pdataDoc;
		pspikeDoc = arg.pspikeDoc;
	}
	return *this;
}

void CDataListCtrl_Row::Serialize(CArchive& ar)
{
	// not serialized:
	// AcqDataDoc*	pdataDoc;
	// CSpikeDoc*	pspikeDoc;

	if (ar.IsStoring())
	{
		wversion = 2;
		ar << wversion;
		ar << index;

		const auto nstr = 8;
		ar << nstr;
		ar << cs_comment;
		ar << csDatafileName;
		ar << csSensillumname;
		ar << csStim1;
		ar << csConc1;
		ar << csStim2;
		ar << csConc2;
		ar << csNspk;
		ar << csFlag;
		ar << csDate;

		const auto nobj = 3;
		ar << nobj;
		pDataChartWnd->Serialize(ar);
		pSpikeChartWnd->Serialize(ar);
		ar << insectID;
	}
	else
	{
		ar >> wversion;
		ar >> index;

		int nstr;
		ar >> nstr;
		ar >> cs_comment;
		nstr--;
		ar >> csDatafileName;
		nstr--;
		ar >> csSensillumname;
		nstr--;
		ar >> csStim1;
		nstr--;
		ar >> csConc1;
		nstr--;
		if (wversion > 1)
		{
			ar >> csStim2;
			nstr--;
			ar >> csConc2;
			nstr--;
		}
		ar >> csNspk;
		nstr--;
		ar >> csFlag;
		nstr--;
		ar >> csDate;
		nstr--;

		int nobj;
		ar >> nobj;
		ASSERT(nobj >= 2);
		pDataChartWnd->Serialize(ar);
		pSpikeChartWnd->Serialize(ar);
		nobj -= 2;
		nobj--;
		if (nobj > 0) ar >> insectID;
	}
}
