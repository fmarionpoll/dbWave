#include "StdAfx.h"
#include "dbWaveDoc.h"
#include "AcqDataDoc.h"
#include "Editctrl.h"
#include "ChartWnd.h"
#include "ChartData.h"
#include "ChartSpikeShape.h"
#include "DlgSpikeEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



DlgSpikeEdit::DlgSpikeEdit(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
}

void DlgSpikeEdit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_SPIKECLASS, m_spikeclass);
	DDX_Text(pDX, IDC_SPIKENO, m_spike_index);
	DDX_Check(pDX, IDC_ARTEFACT, m_bartefact);
	DDX_Text(pDX, IDC_DISPLAYRATIO, m_displayratio);
	DDX_Text(pDX, IDC_YEXTENT, m_yvextent);
}

BEGIN_MESSAGE_MAP(DlgSpikeEdit, CDialog)
	ON_EN_CHANGE(IDC_SPIKENO, OnEnChangespike_index)
	ON_EN_CHANGE(IDC_SPIKECLASS, OnEnChangeSpikeclass)
	ON_BN_CLICKED(IDC_ARTEFACT, OnArtefact)
	ON_EN_CHANGE(IDC_DISPLAYRATIO, OnEnChangeDisplayratio)
	ON_EN_CHANGE(IDC_YEXTENT, OnEnChangeYextent)
	ON_WM_DESTROY()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
END_MESSAGE_MAP()


void DlgSpikeEdit::LoadSpikeParms()
{
	const Spike* spike = m_pSpkList->get_spike(m_spike_index); 
	m_spikeclass = spike->get_class_id(); 
	m_bartefact = (m_spikeclass < 0);
	m_iitime = spike->get_time();

	m_SpkChartWnd.SelectSpikeShape(m_spike_index);

	LoadSourceViewData();
	UpdateData(FALSE); 
}

BOOL DlgSpikeEdit::OnInitDialog()
{
	CDialog::OnInitDialog(); 
	m_pAcqDatDoc = m_pdbWaveDoc->m_pDat;
	m_pSpkList = m_pdbWaveDoc->m_pSpk->get_spk_list_current();
	if (m_pSpkList == nullptr || m_pSpkList->get_spikes_count() == 0)
	{
		EndDialog(FALSE); 
		return TRUE;
	}

	// subclass edit controls
	VERIFY(mm_spikeclass.SubclassDlgItem(IDC_SPIKECLASS, this));
	VERIFY(mm_spike_index.SubclassDlgItem(IDC_SPIKENO, this));
	VERIFY(m_HScroll.SubclassDlgItem(IDC_SCROLLBAR1, this));
	VERIFY(m_VScroll.SubclassDlgItem(IDC_SCROLLBAR2, this));

	// add scrollbar to the left of edit controls
	mm_spikeclass.ShowScrollBar(SB_VERT);
	mm_spike_index.ShowScrollBar(SB_VERT);

	// attach spike buffer
	VERIFY(m_SpkChartWnd.SubclassDlgItem(IDC_DISPLAYSPIKE_buttn, this));
	m_SpkChartWnd.set_source_data(m_pSpkList, m_pdbWaveDoc);
	if (m_spike_index < 0) // select at least spike 0
		m_spike_index = 0;

	m_SpkChartWnd.SetRangeMode(RANGE_ALL); // display mode (lines)
	m_SpkChartWnd.set_plot_mode(PLOT_BLACK, 0); // display also artefacts

	if (m_pAcqDatDoc != nullptr)
	{
		VERIFY(m_ChartDataWnd.SubclassDlgItem(IDC_DISPLAREA_buttn, this));
		m_ChartDataWnd.SetbUseDIB(FALSE);
		m_ChartDataWnd.AttachDataFile(m_pAcqDatDoc);
		const auto lvSize = m_ChartDataWnd.GetRectSize();
		m_ChartDataWnd.ResizeChannels(lvSize.cx, 0); // change nb of pixels
		m_ChartDataWnd.RemoveAllChanlistItems();
		m_ChartDataWnd.AddChanlistItem(m_pSpkList->get_detection_parameters()->extract_channel, m_pSpkList->get_detection_parameters()->extract_transform);

		if (m_pSpkList->get_detection_parameters()->compensate_Baseline)
		{
			m_ChartDataWnd.AddChanlistItem(m_pSpkList->get_detection_parameters()->extract_channel, MOVAVG30);
			m_ChartDataWnd.GetChanlistItem(1)->SetColor(6);
			m_ChartDataWnd.GetChanlistItem(1)->SetPenWidth(1);
			static_cast<CButton*>(GetDlgItem(IDC_CHECK1))->SetCheck(1);
		}
		m_intervals_to_highlight_spikes.SetSize(3 + 2); // total size
		m_intervals_to_highlight_spikes.SetAt(0, 0); // source channel
		m_intervals_to_highlight_spikes.SetAt(1, RGB(255, 0, 0)); // red color
		m_intervals_to_highlight_spikes.SetAt(2, 1); // pen size
		m_ChartDataWnd.SetHighlightData(&m_intervals_to_highlight_spikes); // tell sourceview to highlight spk

		// validate associated controls
		VERIFY(mm_yvextent.SubclassDlgItem(IDC_YEXTENT, this));
		VERIFY(mm_displayratio.SubclassDlgItem(IDC_DISPLAYRATIO, this));
		mm_yvextent.ShowScrollBar(SB_VERT);
		mm_displayratio.ShowScrollBar(SB_VERT);
	}
	else
	{
		GetDlgItem(IDC_YEXTENT)->EnableWindow(FALSE);
		GetDlgItem(IDC_DISPLAYRATIO)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON1)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON2)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON3)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON4)->EnableWindow(FALSE);
	}

	m_displayratio = 20; // how much spike versus source data
	m_spkpretrig = m_pSpkList->get_detection_parameters()->detect_pre_threshold; // load parms used many times
	m_spklen = m_pSpkList->get_spike_length(); // pre-trig and spike length
	m_viewdatalen = MulDiv(m_spklen, 100, m_displayratio); // how wide is source window
	if (m_yextent == 0)
	{
		short max, min;
		m_pSpkList->get_total_max_min(TRUE, &max, &min);
		m_yextent = (max - min);
		m_yzero = (max + min) / 2;
	}

	m_yvextent = m_yextent; // ordinates extent

	// display data and init parameters
	LoadSpikeParms(); // load textual parms and displ source
	m_iitimeold = m_iitime;
	m_bchanged = FALSE; // no modif yet to spikes

	// adjust scroll bar (size of button and left/right limits)
#define SCROLLMAX 200
#define SCROLLCENTER 100
	m_HScroll_infos.fMask = SIF_ALL;
	m_HScroll_infos.nMin = 0;
	m_HScroll_infos.nMax = SCROLLMAX;
	m_HScroll_infos.nPos = SCROLLCENTER;
	m_HScroll_infos.nPage = 10;
	m_HScroll.SetScrollInfo(&m_HScroll_infos);

	m_VScroll_infos = m_HScroll_infos;
	m_VScroll.SetScrollInfo(&m_VScroll_infos);

	return TRUE; // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void DlgSpikeEdit::OnEnChangespike_index()
{
	if (mm_spike_index.m_bEntryDone)
	{
		const auto spike_index = m_spike_index;
		mm_spike_index.OnEnChange(this, m_spike_index, 1, -1);

		// check boundaries
		if (m_spike_index < 0)
			m_spike_index = 0;
		if (m_spike_index >= m_pSpkList->get_spikes_count())
			m_spike_index = m_pSpkList->get_spikes_count() - 1;

		if (m_spike_index != spike_index) 
		{
			LoadSpikeParms();
			m_iitimeold = m_iitime;
			UpdateSpikeScroll();
		}
		else
			UpdateData(FALSE);
	}
}

void DlgSpikeEdit::OnEnChangeSpikeclass()
{
	if (mm_spikeclass.m_bEntryDone)
	{
		mm_spikeclass.OnEnChange(this, m_spikeclass, 1, -1);
		m_pSpkList->get_spike(m_spike_index)->set_class_id(m_spikeclass);
		m_bartefact = (m_spikeclass < 0);
		UpdateData(FALSE);
		m_bchanged = TRUE;
	}
}

void DlgSpikeEdit::OnArtefact()
{
	UpdateData(TRUE); // load value from control
	m_spikeclass = (m_bartefact) ? -1 : 0;
	m_pSpkList->get_spike(m_spike_index)->set_class_id(m_spikeclass);
	UpdateData(FALSE); // update value
	m_bchanged = TRUE;
}

void DlgSpikeEdit::OnEnChangeDisplayratio()
{
	if (mm_displayratio.m_bEntryDone)
	{
		mm_displayratio.OnEnChange(this, m_displayratio, 1, -1);

		if (m_displayratio < 1)
			m_displayratio = 1;
		UpdateData(FALSE);
		m_viewdatalen = MulDiv(m_spklen, 100, m_displayratio);
		LoadSourceViewData();
	}
}

void DlgSpikeEdit::OnEnChangeYextent()
{
	if (mm_yvextent.m_bEntryDone)
	{
		mm_yvextent.OnEnChange(this, m_yvextent, 1, -1);
		UpdateData(FALSE);
		ASSERT(m_yvextent != 0);
		if (m_yvextent != m_yextent)
		{
			m_yextent = m_yvextent;
			m_ChartDataWnd.GetChanlistItem(0)->SetYextent(m_yextent);
			if (m_pSpkList->get_detection_parameters()->compensate_Baseline)
				m_ChartDataWnd.GetChanlistItem(1)->SetYextent(m_yextent);
			m_SpkChartWnd.SetYWExtOrg(m_yextent, m_yzero);
			m_ChartDataWnd.Invalidate();
			m_SpkChartWnd.Invalidate();
		}
	}
}

void DlgSpikeEdit::LoadSourceViewData()
{
	if (m_pAcqDatDoc == nullptr)
		return;

	const auto spike = m_pSpkList->get_spike(m_spike_index); 
	const auto spike_first = spike->get_time() - m_spkpretrig;
	m_intervals_to_highlight_spikes.SetAt(3, spike_first); 
	const auto spike_last = spike_first + m_spklen; 
	m_intervals_to_highlight_spikes.SetAt(4, spike_last);

	// compute limits of m_sourceView
	auto source_view_first = spike_first + m_spklen / 2 - m_viewdatalen / 2;
	if (source_view_first < 0) 
		source_view_first = 0; 
	auto source_view_last = source_view_first + m_viewdatalen - 1; 
	if (source_view_last > m_ChartDataWnd.GetDocumentLast()) 
	{
		source_view_last = m_ChartDataWnd.GetDocumentLast();
		source_view_first = source_view_last - m_viewdatalen + 1;
	}
	// get data from doc
	m_spikeChan = spike->get_source_channel();

	m_ChartDataWnd.SetChanlistSourceChan(0, m_spikeChan);
	m_ChartDataWnd.GetDataFromDoc(source_view_first, source_view_last);

	const auto method = m_pSpkList->get_detection_parameters()->extract_transform;
	m_pAcqDatDoc->LoadTransformedData(source_view_first, source_view_last, method, m_spikeChan);

	// adjust offset (center spike) : use initial offset from spike
	CChanlistItem* chan0 = m_ChartDataWnd.GetChanlistItem(0);
	chan0->SetYzero(m_yzero + spike->get_amplitude_offset());
	chan0->SetYextent(m_yextent);

	if (m_pSpkList->get_detection_parameters()->compensate_Baseline)
	{
		CChanlistItem* chan1 = m_ChartDataWnd.GetChanlistItem(1);
		chan1->SetYzero(m_yzero + spike->get_amplitude_offset());
		chan1->SetYextent(m_yextent);
	}
	m_ChartDataWnd.Invalidate();
}

void DlgSpikeEdit::LoadSpikeFromData(int shift)
{
	if (m_pAcqDatDoc != nullptr)
	{
		Spike* pSpike = m_pSpkList->get_spike(m_spike_index);
		//auto offset = pSpike->get_amplitude_offset();
		m_iitime += shift;
		pSpike->set_time(m_iitime);
		UpdateSpikeScroll();

		LoadSourceViewData();
		const auto spike_first = m_iitime - m_spkpretrig;

		auto lp_source = m_pAcqDatDoc->GetpTransfDataBUF();
		const auto delta = spike_first - m_pAcqDatDoc->GettBUFfirst();
		lp_source += delta;
		pSpike->transfer_data_to_spike_buffer(lp_source, 1, m_spklen);
		short max, min;
		int i_max, i_min;
		pSpike->measure_max_min_ex(&max, &i_max, &min, &i_min, 0, m_pSpkList->get_spike_length() - 1);
		pSpike->set_max_min_ex(max, min, i_min - i_max);

		// copy data to spike buffer
		//offset += pSpike->get_amplitude_offset();
		//pSpike->OffsetSpikeDataToAverageEx(offset, offset);

		m_SpkChartWnd.Invalidate();
		m_bchanged = TRUE;
	}
}

void DlgSpikeEdit::OnDestroy()
{
	CDialog::OnDestroy();
}

void DlgSpikeEdit::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar == nullptr)
	{
		CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
		return;
	}

	// assume that code comes from SCROLLBAR1
	int shift;
	// get corresponding data
	switch (nSBCode)
	{
	case SB_LEFT: shift = -10;
		break; // scroll to the start
	case SB_LINELEFT: shift = -1;
		break; // scroll one line left
	case SB_LINERIGHT: shift = +1;
		break; // scroll one line right
	case SB_PAGELEFT: shift = -10;
		break; // scroll one page left
	case SB_PAGERIGHT: shift = +10;
		break; // scroll one page right
	case SB_RIGHT: shift = +10;
		break; // scroll to end right

	case SB_THUMBPOSITION: // scroll to pos = nPos
	case SB_THUMBTRACK: // drag scroll box -- pos = nPos
		shift = static_cast<int>(nPos) - SCROLLCENTER - (m_iitime - m_iitimeold);
		break;
	default: // NOP: set position only
		return;
	}
	LoadSpikeFromData(shift);
}

void DlgSpikeEdit::UpdateSpikeScroll()
{
	m_HScroll_infos.fMask = SIF_PAGE | SIF_POS;
	m_HScroll_infos.nPos = m_iitime - m_iitimeold + SCROLLCENTER;
	m_HScroll_infos.nPage = 10;
	m_HScroll.SetScrollInfo(&m_HScroll_infos);
}

void DlgSpikeEdit::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar == nullptr)
	{
		CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
		return;
	}

	// assume that code comes from SCROLLBAR2
	int shift;
	// get corresponding data
	switch (nSBCode)
	{
	case SB_TOP: shift = -10;
		break; // scroll to the start
	case SB_LINEUP: shift = -1;
		break; // scroll one line left
	case SB_LINEDOWN: shift = 1;
		break; // scroll one line right
	case SB_PAGEUP: shift = -10;
		break; // scroll one page left
	case SB_PAGEDOWN: shift = 10;
		break; // scroll one page right
	case SB_BOTTOM: shift = 10;
		break; // scroll to end right

	case SB_THUMBPOSITION: // scroll to pos = nPos
	case SB_THUMBTRACK: // drag scroll box -- pos = nPos
		shift = nPos - SCROLLCENTER - 4096;
		break;
	default: // NOP: set position only
		return;
	}

	auto spike = m_pSpkList->get_spike(m_spike_index);
	spike->set_spike_length(m_pSpkList->get_spike_length());
	spike->offset_spike_data(static_cast<short>(shift));

	LoadSpikeParms();
	m_bchanged = TRUE;
}
