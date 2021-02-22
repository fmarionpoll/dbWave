#pragma once

#include "StdAfx.h"
#include "dbWaveDoc.h"
#include "Spikedoc.h"
#include "chart.h"
#include "Envelope.h"
#include "chanlistitem.h"
#include "ChartData.h"
#include "Spikedoc.h"
#include "ChartSpikeBar.h"

// CDataListCtrlRowObject command target

class CDataListCtrl_Row : public CObject
{
	DECLARE_SERIAL(CDataListCtrl_Row)
public:
	CDataListCtrl_Row();
	CDataListCtrl_Row(int i);
	virtual ~CDataListCtrl_Row();

	BOOL			bChanged = false;
	WORD			wversion = 0;

	BOOL			bInit = false;
	UINT			index;
	long			insectID;

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

	CChartDataWnd*		pDataChartWnd = nullptr;
	CChartSpikeBarWnd*	pSpikeChartWnd = nullptr;
	CdbWaveDoc*		pdbWaveDoc = nullptr;
	CAcqDataDoc*	pdataDoc = nullptr;
	CSpikeDoc*		pspikeDoc = nullptr;

	CDataListCtrl_Row& operator = (const CDataListCtrl_Row& arg);
	void Serialize(CArchive& ar) override;
};
