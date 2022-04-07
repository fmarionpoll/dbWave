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
	DDX_Text(pDX, IDC_SPIKENO, m_spikeno);
	DDX_Check(pDX, IDC_ARTEFACT, m_bartefact);
	DDX_Text(pDX, IDC_DISPLAYRATIO, m_displayratio);
	DDX_Text(pDX, IDC_YEXTENT, m_yvextent);
}

BEGIN_MESSAGE_MAP(DlgSpikeEdit, CDialog)
	ON_EN_CHANGE(IDC_SPIKENO, OnEnChangeSpikeno)
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
	const Spike* spike = m_pSpkList->GetSpike(m_spikeno); // get address of spike parms
	m_spikeclass = spike->get_class(); 
	m_bartefact = (m_spikeclass < 0);
	m_iitime = spike->get_time();

	m_SpkChartWnd.SelectSpikeShape(m_spikeno);

	LoadSourceData();
	UpdateData(FALSE); 
}

BOOL DlgSpikeEdit::OnInitDialog()
{
	CDialog::OnInitDialog(); 
	m_pAcqDatDoc = m_pdbWaveDoc->m_pDat;
	m_pSpkList = m_pdbWaveDoc->m_pSpk->GetSpkList_Current();
	if (m_pSpkList == nullptr || m_pSpkList->GetTotalSpikes() == 0)
	{
		EndDialog(FALSE); 
		return TRUE;
	}

	// subclass edit controls
	VERIFY(mm_spikeclass.SubclassDlgItem(IDC_SPIKECLASS, this));
	VERIFY(mm_spikeno.SubclassDlgItem(IDC_SPIKENO, this));
	VERIFY(m_HScroll.SubclassDlgItem(IDC_SCROLLBAR1, this));
	VERIFY(m_VScroll.SubclassDlgItem(IDC_SCROLLBAR2, this));

	// add scrollbar to the left of edit controls
	mm_spikeclass.ShowScrollBar(SB_VERT);
	mm_spikeno.ShowScrollBar(SB_VERT);

	// attach spike buffer
	VERIFY(m_SpkChartWnd.SubclassDlgItem(IDC_DISPLAYSPIKE_buttn, this));
	m_SpkChartWnd.set_source_data(m_pSpkList, m_pdbWaveDoc);
	if (m_spikeno < 0) // select at least spike 0
		m_spikeno = 0;

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
		m_ChartDataWnd.AddChanlistItem(m_pSpkList->GetDetectParms()->extractChan, m_pSpkList->GetDetectParms()->extractTransform);

		if (m_pSpkList->GetDetectParms()->compensateBaseline)
		{
			m_ChartDataWnd.AddChanlistItem(m_pSpkList->GetDetectParms()->extractChan, MOVAVG30);
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
	m_spkpretrig = m_pSpkList->GetDetectParms()->prethreshold; // load parms used many times
	m_spklen = m_pSpkList->GetSpikeLength(); // pre-trig and spike length
	m_viewdatalen = MulDiv(m_spklen, 100, m_displayratio); // how wide is source window
	if (m_yextent == 0)
	{
		int max, min;
		m_pSpkList->GetTotalMaxMin(TRUE, &max, &min);
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

void DlgSpikeEdit::OnEnChangeSpikeno()
{
	if (mm_spikeno.m_bEntryDone)
	{
		const auto spikeno = m_spikeno;
		switch (mm_spikeno.m_nChar)
		{
		// load data from edit controls
		case VK_RETURN: UpdateData(TRUE);
			break;
		case VK_UP:
		case VK_PRIOR: m_spikeno++;
			break;
		case VK_DOWN:
		case VK_NEXT: m_spikeno--;
			break;
		default: ;
		}

		// check boundaries
		if (m_spikeno < 0)
			m_spikeno = 0;
		if (m_spikeno >= m_pSpkList->GetTotalSpikes())
			m_spikeno = m_pSpkList->GetTotalSpikes() - 1;

		mm_spikeno.m_bEntryDone = FALSE; 
		mm_spikeno.m_nChar = 0; 
		mm_spikeno.SetSel(0, -1); 
		if (m_spikeno != spikeno) 
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
		switch (mm_spikeclass.m_nChar)
		{
		// load data from edit controls
		case VK_RETURN: UpdateData(TRUE);
			break;
		case VK_UP:
		case VK_PRIOR: m_spikeclass++;
			break;
		case VK_DOWN:
		case VK_NEXT: m_spikeclass--;
			break;
		default: ;
		}

		m_pSpkList->GetSpike(m_spikeno)->set_class(m_spikeclass);
		mm_spikeclass.m_bEntryDone = FALSE; // clear flag
		mm_spikeclass.m_nChar = 0; // empty buffer
		mm_spikeclass.SetSel(0, -1); // select all text
		m_bartefact = (m_spikeclass < 0);
		UpdateData(FALSE);
		m_bchanged = TRUE;
	}
}

void DlgSpikeEdit::OnArtefact()
{
	UpdateData(TRUE); // load value from control
	m_spikeclass = (m_bartefact) ? -1 : 0;
	m_pSpkList->GetSpike(m_spikeno)->set_class(m_spikeclass);
	UpdateData(FALSE); // update value
	m_bchanged = TRUE;
}

void DlgSpikeEdit::OnEnChangeDisplayratio()
{
	if (mm_displayratio.m_bEntryDone)
	{
		switch (mm_displayratio.m_nChar)
		{
		// load data from edit controls
		case VK_RETURN: UpdateData(TRUE);
			break;
		case VK_UP:
		case VK_PRIOR: m_displayratio++;
			break;
		case VK_DOWN:
		case VK_NEXT: m_displayratio--;
			break;
		default: ;
		}
		mm_displayratio.m_bEntryDone = FALSE; // clear flag
		mm_displayratio.m_nChar = 0; // empty buffer
		mm_displayratio.SetSel(0, -1); // select all text
		if (m_displayratio < 1)
			m_displayratio = 1;
		UpdateData(FALSE);
		m_viewdatalen = MulDiv(m_spklen, 100, m_displayratio);
		LoadSourceData();
	}
}

void DlgSpikeEdit::OnEnChangeYextent()
{
	if (mm_yvextent.m_bEntryDone)
	{
		switch (mm_yvextent.m_nChar)
		{
		// load data from edit controls
		case VK_RETURN: UpdateData(TRUE);
			break;
		case VK_UP:
		case VK_PRIOR: m_yvextent++;
			break;
		case VK_DOWN:
		case VK_NEXT: m_yvextent--;
			break;
		default: ;
		}
		mm_yvextent.m_bEntryDone = FALSE; // clear flag
		mm_yvextent.m_nChar = 0; // empty buffer
		mm_yvextent.SetSel(0, -1); // select all text
		UpdateData(FALSE);

		ASSERT(m_yvextent != 0);

		if (m_yvextent != m_yextent)
		{
			m_yextent = m_yvextent;
			m_ChartDataWnd.GetChanlistItem(0)->SetYextent(m_yextent);
			if (m_pSpkList->GetDetectParms()->compensateBaseline)
				m_ChartDataWnd.GetChanlistItem(1)->SetYextent(m_yextent);
			m_SpkChartWnd.SetYWExtOrg(m_yextent, m_yzero);
			m_ChartDataWnd.Invalidate();
			m_SpkChartWnd.Invalidate();
		}
	}
}

void DlgSpikeEdit::LoadSourceData()
{
	if (m_pAcqDatDoc == nullptr)
		return;

	const auto spike = m_pSpkList->GetSpike(m_spikeno); 
	const auto l_first = spike->get_time() - m_spkpretrig;
	m_intervals_to_highlight_spikes.SetAt(3, l_first); 
	const auto l_last = l_first + m_spklen; 
	m_intervals_to_highlight_spikes.SetAt(4, l_last);

	// compute limits of m_sourceView
	auto l_v_first = l_first + m_spklen / 2 - m_viewdatalen / 2;
	if (l_v_first < 0) 
		l_v_first = 0; 
	auto l_v_last = l_v_first + m_viewdatalen - 1; 
	if (l_v_last > m_ChartDataWnd.GetDocumentLast()) 
	{
		l_v_last = m_ChartDataWnd.GetDocumentLast();
		l_v_first = l_v_last - m_viewdatalen + 1;
	}
	// get data from doc
	m_spikeChan = spike->get_source_channel();

	m_ChartDataWnd.SetChanlistSourceChan(0, m_spikeChan);
	m_ChartDataWnd.GetDataFromDoc(l_v_first, l_v_last); 

	// adjust offset (center spike) : use initial offset from spike
	CChanlistItem* chan0 = m_ChartDataWnd.GetChanlistItem(0);
	chan0->SetYzero(m_yzero + spike->get_amplitude_offset());
	chan0->SetYextent(m_yextent);

	if (m_pSpkList->GetDetectParms()->compensateBaseline)
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
		auto offset = m_pSpkList->GetSpike(m_spikeno)->get_amplitude_offset();
		m_iitime += shift;
		m_pSpkList->GetSpike(m_spikeno)->set_time(m_iitime);
		UpdateSpikeScroll();
		LoadSourceData();

		const auto l_first = m_iitime - m_spkpretrig;

		// get source data buffer address
		Spike* pSpike = m_pSpkList->GetSpike(m_spikeno);
		const auto method = m_pSpkList->GetDetectParms()->extractTransform;
		if (method == 0)
		{
			int n_channels;
			auto lp_source = m_pAcqDatDoc->LoadRawDataParams(&n_channels);
			lp_source += (l_first - m_pAcqDatDoc->GettBUFfirst()) * n_channels+ m_spikeChan;
			pSpike->TransferDataToSpikeBuffer(lp_source, n_channels);
		}
		else
		{
			m_pAcqDatDoc->LoadTransfData(l_first, l_first + m_spklen, method, m_spikeChan);
			auto p_data = m_pAcqDatDoc->GetpTransfDataBUF();
			p_data += l_first - m_pAcqDatDoc->GettBUFfirst();
			pSpike->TransferDataToSpikeBuffer(p_data, 1);
		}

		// copy data to spike buffer
		offset += pSpike->get_amplitude_offset();
		pSpike->OffsetSpikeDataToAverageEx(offset, offset);

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
		shift = nPos - SCROLLCENTER - (m_iitime - m_iitimeold);
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
	case SB_LINEDOWN: shift = +1;
		break; // scroll one line right
	case SB_PAGEUP: shift = -10;
		break; // scroll one page left
	case SB_PAGEDOWN: shift = +10;
		break; // scroll one page right
	case SB_BOTTOM: shift = +10;
		break; // scroll to end right

	case SB_THUMBPOSITION: // scroll to pos = nPos
	case SB_THUMBTRACK: // drag scroll box -- pos = nPos
		shift = nPos - SCROLLCENTER - 4096;
		break;
	default: // NOP: set position only
		return;
	}

	m_pSpkList->GetSpike(m_spikeno)->OffsetSpikeData(shift);

	LoadSpikeParms();
	m_bchanged = TRUE;
}
