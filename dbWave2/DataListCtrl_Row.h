#pragma once

//#include "StdAfx.h"
#include "Spikedoc.h"
#include "ChartData.h"
#include "ChartSpikeBar.h"

// CDataListCtrlRowObject command target

class CDataListCtrl_Row : public CObject
{
	DECLARE_SERIAL(CDataListCtrl_Row)
public:
	CDataListCtrl_Row();
	CDataListCtrl_Row(int i);
	virtual ~CDataListCtrl_Row() override;

	BOOL			bChanged = false;
	WORD			wversion = 0;
	BOOL			bInit = false;
	UINT			index = 0;
	long			insectID = 0;

	CString			cs_comment{};
	CString			csDatafileName{};
	CString			csSpikefileName{};
	CString			csSensillumname{};
	CString			csStim1{};
	CString			csConc1{};
	CString			csStim2{};
	CString			csConc2{};
	CString			csNspk{};
	CString			csFlag{};
	CString			csDate{};

	CAcqDataDoc* pdataDoc = nullptr;
	CChartDataWnd* pDataChartWnd = nullptr;
	CSpikeDoc* pspikeDoc = nullptr;
	CChartSpikeBarWnd* pSpikeChartWnd = nullptr;

	CDataListCtrl_Row& operator = (const CDataListCtrl_Row& arg);
	void Serialize(CArchive& ar) override;
};
