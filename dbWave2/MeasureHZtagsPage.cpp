// TODO : measure data and output to notedocview

#include "StdAfx.h"
#include "ChartWnd.h"
#include "ChartData.h"
#include "Editctrl.h"
#include "dbWaveDoc.h"
#include "resource.h"
#include "MeasureHZtagsPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CMeasureHZtagsPage, CPropertyPage)

CMeasureHZtagsPage::CMeasureHZtagsPage() : CPropertyPage(IDD), m_pMO(nullptr), m_pdbDoc(nullptr),
                                           m_pdatDoc(nullptr)
{
	m_datachannel = 0;
	m_index = 0;
	m_mvlevel = 0.0f;
	m_nbcursors = 0;
	m_pChartDataWnd = nullptr;
}

CMeasureHZtagsPage::~CMeasureHZtagsPage()
{
}

void CMeasureHZtagsPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_DATACHANNEL, m_datachannel);
	DDX_Text(pDX, IDC_INDEX, m_index);
	DDX_Text(pDX, IDC_MVLEVEL, m_mvlevel);
	DDX_Text(pDX, IDC_NBCURSORS, m_nbcursors);
}

BEGIN_MESSAGE_MAP(CMeasureHZtagsPage, CPropertyPage)
	ON_BN_CLICKED(IDC_CENTER, OnCenter)
	ON_BN_CLICKED(IDC_ADJUST, OnAdjust)
	ON_BN_CLICKED(IDC_REMOVE, OnRemove)
	ON_EN_CHANGE(IDC_DATACHANNEL, OnEnChangeDatachannel)
	ON_EN_CHANGE(IDC_INDEX, OnEnChangeIndex)
	ON_EN_CHANGE(IDC_MVLEVEL, OnEnChangeMvlevel)
	ON_BN_CLICKED(IDC_BUTTON4, OnDeleteAll)
END_MESSAGE_MAP()

BOOL CMeasureHZtagsPage::GetHZcursorVal(const int index)
{
	const BOOL flag = (m_nbcursors > 0 && index < m_nbcursors && index >= 0);
	GetDlgItem(IDC_ADJUST)->EnableWindow(flag);
	GetDlgItem(IDC_CENTER)->EnableWindow(flag);
	GetDlgItem(IDC_REMOVE)->EnableWindow(flag);

	if (index < 0 || index >= m_nbcursors)
		return FALSE;
	m_index = index;
	m_datachannel = m_pChartDataWnd->m_HZtags.GetChannel(index);
	int k = m_pChartDataWnd->m_HZtags.GetValue(m_index);
	m_mvlevel = m_pChartDataWnd->GetChanlistItem(m_datachannel)->ConvertDataBinsToVolts(k) * 1000.f;

	return TRUE;
}

void CMeasureHZtagsPage::OnCenter()
{
	int max, min;
	CChanlistItem* pchan = m_pChartDataWnd->GetChanlistItem(m_datachannel);
	pchan->GetMaxMin(&max, &min);
	const auto val = (max + min) / 2;
	m_pChartDataWnd->m_HZtags.SetTagVal(m_index, val);
	m_pChartDataWnd->Invalidate();
	m_mvlevel = pchan->ConvertDataBinsToVolts(val) * 1000.f;
	UpdateData(FALSE);
}

void CMeasureHZtagsPage::OnRemove()
{
	if (m_index >= 0 && m_index < m_nbcursors)
	{
		m_pChartDataWnd->m_HZtags.RemoveTag(m_index);
		m_nbcursors--;
	}
	if (m_index > m_nbcursors - 1)
		m_index = m_nbcursors;
	GetHZcursorVal(m_index);
	m_pChartDataWnd->Invalidate();
	UpdateData(FALSE);
}

void CMeasureHZtagsPage::OnEnChangeDatachannel()
{
	if (mm_datachannel.m_bEntryDone)
	{
		mm_datachannel.OnEnChange(this, m_datachannel, 1, -1);

		// update dependent parameters
		if (m_datachannel < 0)
			m_datachannel = 0;
		if (m_datachannel >= m_pChartDataWnd->GetChanlistSize())
			m_datachannel = m_pChartDataWnd->GetChanlistSize() - 1;
		if (m_nbcursors > 0 && m_index >= 0 && m_index < m_nbcursors)
		{
			m_pChartDataWnd->m_HZtags.SetTagChan(m_index, m_datachannel);
			m_pChartDataWnd->Invalidate();
		}
		UpdateData(FALSE);
	}
}

void CMeasureHZtagsPage::OnEnChangeIndex()
{
	if (mm_index.m_bEntryDone)
	{
		mm_index.OnEnChange(this, m_index, 1, -1);
		// update dependent parameters
		if (m_index >= m_nbcursors)
			m_index = m_nbcursors - 1;
		if (m_index < 0)
			m_index = 0;
		GetHZcursorVal(m_index);
		UpdateData(FALSE);
	}
}

void CMeasureHZtagsPage::OnEnChangeMvlevel()
{
	if (mm_mvlevel.m_bEntryDone)
	{
		mm_mvlevel.OnEnChange(this, m_mvlevel, 1.f, -1.f);
		UpdateData(FALSE);
		if (m_nbcursors > 0 && m_index >= 0 && m_index < m_nbcursors)
		{
			const auto val = m_pChartDataWnd->GetChanlistItem(m_datachannel)->ConvertVoltsToDataBins(
				m_mvlevel / 1000.0f);
			m_pChartDataWnd->m_HZtags.SetTagVal(m_index, val);
			m_pChartDataWnd->Invalidate();
		}
	}
}

void CMeasureHZtagsPage::OnAdjust()
{
	int max, min;
	m_pChartDataWnd->GetChanlistItem(m_datachannel)->GetMaxMin(&max, &min);
	// get nb cursors / m_datachannel
	auto n_cursors = 0;
	for (auto i = m_nbcursors - 1; i >= 0; i--)
		if (m_pChartDataWnd->m_HZtags.GetChannel(i) == m_datachannel)
			n_cursors++;

	// then split cursors across m_datachannel span
	if (n_cursors == 1)
	{
		OnCenter();
		return;
	}

	const auto dv = (max - min) / (n_cursors - 1);
	auto val = min;
	for (auto i = 0; i < m_nbcursors; i++)
	{
		if (m_pChartDataWnd->m_HZtags.GetChannel(i) == m_datachannel)
		{
			m_pChartDataWnd->m_HZtags.SetTagVal(i, val);
			val += dv;
		}
	}
	m_pChartDataWnd->Invalidate();
	val = m_pChartDataWnd->m_HZtags.GetValue(m_index);
	m_mvlevel = m_pChartDataWnd->GetChanlistItem(m_datachannel)->ConvertDataBinsToVolts(val) * 1000.f;
	UpdateData(FALSE);
}

void CMeasureHZtagsPage::OnOK()
{
	auto p_tags_list = m_pdatDoc->get_hz_tags_list();
	p_tags_list->CopyTagList(&m_pChartDataWnd->m_HZtags);
	m_pMO->b_changed = TRUE;
	if (m_pMO->wOption != 1)
	{
		m_pChartDataWnd->m_HZtags.RemoveAllTags();
		if (m_pMO->wOption == 0)
			m_pChartDataWnd->m_VTtags.CopyTagList(m_pdatDoc->get_vt_tags_list());
	}
	CPropertyPage::OnOK();
}

void CMeasureHZtagsPage::OnCancel()
{
	// restore initial state of HZcursors
	if (m_pMO->wOption != 1)
	{
		m_pChartDataWnd->m_HZtags.RemoveAllTags();
		if (m_pMO->wOption == 0)
			m_pChartDataWnd->m_VTtags.CopyTagList(m_pdatDoc->get_vt_tags_list());
	}
	else
		m_pChartDataWnd->m_HZtags.CopyTagList(m_pdatDoc->get_hz_tags_list());
	m_pChartDataWnd->Invalidate();
	CPropertyPage::OnCancel();
}

BOOL CMeasureHZtagsPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	m_pChartDataWnd->m_HZtags.CopyTagList(m_pdatDoc->get_hz_tags_list());
	m_pChartDataWnd->m_VTtags.RemoveAllTags();
	m_pChartDataWnd->Invalidate();
	m_nbcursors = m_pChartDataWnd->m_HZtags.GetNTags();
	GetHZcursorVal(0);

	// sublassed edits
	VERIFY(mm_index.SubclassDlgItem(IDC_INDEX, this));
	VERIFY(mm_datachannel.SubclassDlgItem(IDC_DATACHANNEL, this));
	VERIFY(mm_mvlevel.SubclassDlgItem(IDC_MVLEVEL, this));
	UpdateData(FALSE);

	return TRUE; // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CMeasureHZtagsPage::OnDeleteAll()
{
	m_pChartDataWnd->m_HZtags.RemoveAllTags();
	m_pChartDataWnd->Invalidate();
	m_nbcursors = 0;
	GetHZcursorVal(0);
	UpdateData(FALSE);
}
