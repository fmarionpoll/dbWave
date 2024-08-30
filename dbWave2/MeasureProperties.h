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
	CMeasureProperties(CWnd* pParentWnd = nullptr, int i_select = 0);

	// Attributes
public:
	CMeasureHZtagsPage m_measure_hz_tags_page;
	CMeasureOptionsPage m_measure_options_page;
	CMeasureVTtagsPage m_measure_vt_tags_page;
	CMeasureResultsPage m_measure_results_page;

	// CMeasureResults
	ChartData* m_pChartDataWnd{};
	CdbWaveDoc* m_pdbDoc{};
	AcqDataDoc* m_pdatDoc{};

	options_measure* m_pMO{};
	int m_currentchan{};
	float m_samplingrate{};

	// Overrides
public:
	BOOL OnInitDialog() override;

	// Implementation
public:
	~CMeasureProperties() override;

	DECLARE_MESSAGE_MAP()
};
