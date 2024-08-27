// MeasureProperties.cpp : implementation file
//

#include "StdAfx.h"
#include "resource.h"
#include "MeasureProperties.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CMeasureProperties, CPropertySheet)

CMeasureProperties::CMeasureProperties(CWnd* p_wnd_parent, const int select_active_page)
	: CPropertySheet(IDS_PROPSHT_CAPTION, p_wnd_parent)
{
	// One way to make a different property page the
	// active one is to call SetActivePage().

	m_pChartDataWnd = nullptr;
	AddPage(&m_measure_hz_tags_page); // measure options
	AddPage(&m_measure_vt_tags_page); // VT tags edit
	AddPage(&m_measure_options_page); // HZ edit
	AddPage(&m_measure_results_page); // measure results
	SetActivePage(select_active_page);
}

CMeasureProperties::~CMeasureProperties()
{
}

BEGIN_MESSAGE_MAP(CMeasureProperties, CPropertySheet)

END_MESSAGE_MAP()

BOOL CMeasureProperties::OnInitDialog()
{
	// all
	m_measure_hz_tags_page.m_p_chart_data_wnd = m_pChartDataWnd;
	m_measure_vt_tags_page.m_p_chart_data_wnd = m_pChartDataWnd;
	m_measure_options_page.m_p_chart_data_wnd = m_pChartDataWnd;
	m_measure_results_page.m_p_chart_data_wnd = m_pChartDataWnd;

	m_measure_hz_tags_page.m_p_options_measure = m_pMO;
	m_measure_vt_tags_page.m_p_options_measure = m_pMO;
	m_measure_options_page.m_p_options_measure = m_pMO;
	m_measure_results_page.m_p_options_measure = m_pMO;

	m_measure_hz_tags_page.m_p_dat_doc = m_pdatDoc;
	m_measure_vt_tags_page.m_p_dat_doc = m_pdatDoc;
	m_measure_options_page.m_p_dat_doc = m_pdatDoc;
	m_measure_results_page.m_p_dat_doc = m_pdatDoc;

	m_measure_hz_tags_page.m_pdb_doc = m_pdbDoc;
	m_measure_vt_tags_page.m_pdb_doc = m_pdbDoc;
	m_measure_options_page.m_pdb_doc = m_pdbDoc;
	m_measure_results_page.m_pdb_doc = m_pdbDoc;

	// measure options
	// vertical tags

	// HZ tags
	m_measure_vt_tags_page.m_sampling_rate = m_samplingrate;

	// measure
	m_measure_results_page.m_current_chan = m_currentchan;
	return CPropertySheet::OnInitDialog();
}
