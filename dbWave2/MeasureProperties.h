#pragma once
#include "dbWaveDoc.h"
#include "chartdata.h"
#include "MeasureHZtagsPage.h"
#include "MeasureOptionsPage.h"
#include "MeasureResultsPage.h"
#include "MeasureVTtagsPage.h"


class CMeasureProperties : public CPropertySheet
{
	DECLARE_DYNAMIC(CMeasureProperties)

	// Construction
public:
	CMeasureProperties(CWnd* pParentWnd = nullptr, int iselect = 0);

	// Attributes
public:
	CMeasureHZtagsPage	m_measure_hz_tags_page;
	CMeasureOptionsPage m_measure_options_page;
	CMeasureVTtagsPage	m_measure_vt_tags_page;
	CMeasureResultsPage m_measure_results_page;

	// CMeasureResults
	CChartDataWnd*	m_pChartDataWnd{};
	CdbWaveDoc*		m_pdbDoc{};
	CAcqDataDoc*	m_pdatDoc{};

	OPTIONS_VIEWDATAMEASURE* m_pMO{};
	int				m_currentchan{};
	float			m_samplingrate{};

	// Overrides
public:
	virtual BOOL OnInitDialog();

	// Implementation
public:
	virtual ~CMeasureProperties();

	DECLARE_MESSAGE_MAP()
};
