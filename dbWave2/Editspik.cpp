// editspik.cpp : implementation file
//

#include "stdafx.h"
#include "dbWave.h"

#include "spikedoc.h"
#include "dbWaveDoc.h"
#include "acqdatad.h"
#include "editctrl.h"
#include "cscale.h"
#include "scopescr.h"
#include "lineview.h"
#include "spikeshape.h"
#include "editspik.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CSpikeEditDlg dialog


CSpikeEditDlg::CSpikeEditDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSpikeEditDlg::IDD, pParent)
{
	m_spikeclass = 0;
	m_spikeno = 0;
	m_bartefact = FALSE;
	m_displayratio = 0;
	m_yvextent = 0;
	m_pSpkList =nullptr;		// spike list
	m_dbDoc = nullptr;			// source data doc cDocument
	m_spikeChan = 0;
}

void CSpikeEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_SPIKECLASS, m_spikeclass);
	DDX_Text(pDX, IDC_SPIKENO, m_spikeno);
	DDX_Check(pDX, IDC_ARTEFACT, m_bartefact);
	DDX_Text(pDX, IDC_DISPLAYRATIO, m_displayratio);
	DDX_Text(pDX, IDC_YEXTENT, m_yvextent);
}

BEGIN_MESSAGE_MAP(CSpikeEditDlg, CDialog)
	ON_EN_CHANGE(IDC_SPIKENO, OnEnChangeSpikeno)
	ON_EN_CHANGE(IDC_SPIKECLASS, OnEnChangeSpikeclass)
	ON_BN_CLICKED(IDC_ARTEFACT, OnArtefact)
	ON_EN_CHANGE(IDC_DISPLAYRATIO, OnEnChangeDisplayratio)
	ON_EN_CHANGE(IDC_YEXTENT, OnEnChangeYextent)
	ON_WM_DESTROY()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// implementation functions

// --------------------------------------------------------------------------
// LoadSpikeParms() load parameters from current spike
// get class, set artefact flag and load data from source file
// --------------------------------------------------------------------------
void CSpikeEditDlg::LoadSpikeParms()
{
	CSpikeElemt* pS = m_pSpkList->GetSpikeElemt(m_spikeno);  // get address of spike parms
	m_spikeclass = pS->GetSpikeClass();		// class number
	m_bartefact = (m_spikeclass <0);
	m_iitime = pS->GetSpikeTime();
		
	m_spkForm.SelectSpikeShape(m_spikeno);

	LoadSourceData();						// load data from source file
	UpdateData(FALSE);						// update screen
}


/////////////////////////////////////////////////////////////////////////////
// CSpikeEditDlg message handlers

// --------------------------------------------------------------------------
// OnInitDialog()
// load data from caller, subclass display buttons and CMyEdit edit fields
// load data from source files
// --------------------------------------------------------------------------

BOOL CSpikeEditDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();			// call base class function
	if (m_pSpkList == nullptr || m_pSpkList->GetTotalSpikes() == 0)
	{
		EndDialog(FALSE);			// exit if no spikes to edit
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
	VERIFY(m_spkForm.SubclassDlgItem(IDC_DISPLAYSPIKE_buttn, this));
	m_spkForm.SetSourceData(m_pSpkList);	// source data: spike list
	if (m_spikeno < 0)						// select at least spike 0
		m_spikeno = 0;

	m_spkForm.SetRangeMode(RANGE_ALL);		// display mode (lines)
	m_spkForm.SetPlotMode(PLOT_BLACK, 0);// display also artefacts

	if (m_dbDoc != nullptr)
	{
		VERIFY(m_sourceView.SubclassDlgItem(IDC_DISPLAREA_buttn, this));
		m_sourceView.SetbUseDIB(FALSE);
		m_sourceView.AttachDataFile(m_dbDoc, m_dbDoc->GetDOCchanLength());
		CSize lvSize = m_sourceView.Size();
		m_sourceView.ResizeChannels(lvSize.cx, 0);// change nb of pixels	
		m_sourceView.RemoveAllChanlistItems();
		m_sourceView.AddChanlistItem(m_pSpkList->GetextractChan(), m_pSpkList->GetextractTransform()); 

		if (m_pSpkList->GetcompensateBaseline())
		{
			m_sourceView.AddChanlistItem(m_pSpkList->GetextractChan(), MOVAVG30);
			m_sourceView.SetChanlistColor(1, 6);		
			m_sourceView.SetChanlistPenWidth(1, 1);
			((CButton*) GetDlgItem(IDC_CHECK1))->SetCheck(1);
		}
		m_DWintervals.SetSize(3+2);						// total size
		m_DWintervals.SetAt(0, 0);						// source channel
		m_DWintervals.SetAt(1, (DWORD) RGB(255,   0,   0));	// red color
		m_DWintervals.SetAt(2, 1);						// pen size
		m_sourceView.SetHighlightData(&m_DWintervals);// tell sourceview to highlight spk

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

	m_displayratio = 20;						// how much spike versus source data
	m_spkpretrig = m_pSpkList->GetSpikePretrig();	// load parms used many times
	m_spklen = m_pSpkList->GetSpikeLength();		// pre-trig and spike length
	m_viewdatalen = MulDiv(m_spklen, 100, m_displayratio); // how wide is source window
	if (m_yextent == 0)
	{
		int max, min;
		m_pSpkList->GetTotalMaxMin(TRUE, &max, &min);
		m_yextent = (max-min);
		m_yzero = (max+min)/2;
	}	
	
	m_yvextent = m_yextent;						// ordinates extent

	// display data and init parameters    
	LoadSpikeParms(); 							// load textual parms and displ source
	m_iitimeold = m_iitime;
	m_bchanged = FALSE;							// no modif yet to spikes

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

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


// --------------------------------------------------------------------------
// OnEnChangeSpikeno
// update edit control and update all infos displayed (check boundaries)
// spikeno = spike index displayed
// --------------------------------------------------------------------------

void CSpikeEditDlg::OnEnChangeSpikeno() 
{
	if (!mm_spikeno.m_bEntryDone)
		return;
	int spikeno = m_spikeno;
	switch (mm_spikeno.m_nChar)
	{				// load data from edit controls
		case VK_RETURN:	UpdateData(TRUE);	break;
		case VK_UP:
		case VK_PRIOR:	m_spikeno++;	break;
		case VK_DOWN:
		case VK_NEXT:   m_spikeno--;	break;
	}

	// check boundaries
	if (m_spikeno<0)
		m_spikeno = 0;
	if (m_spikeno >= m_pSpkList->GetTotalSpikes())
		m_spikeno = m_pSpkList->GetTotalSpikes()-1;

	mm_spikeno.m_bEntryDone=FALSE;	// clear flag
	mm_spikeno.m_nChar=0;			// empty buffer
	mm_spikeno.SetSel(0, -1);		// select all text    
	if (m_spikeno != spikeno)	// change display if necessary
	{
		LoadSpikeParms();
		m_iitimeold = m_iitime;
		UpdateSpikeScroll();
	}
	else
		UpdateData(FALSE);

	return;	
}

// --------------------------------------------------------------------------
// OnEnChangeSpikeclass()
// update edit control and modifies spike class
// --------------------------------------------------------------------------

void CSpikeEditDlg::OnEnChangeSpikeclass()
{	
	if (!mm_spikeclass.m_bEntryDone)
		return;
	
	switch (mm_spikeclass.m_nChar)
	{				// load data from edit controls
		case VK_RETURN:	UpdateData(TRUE); break;
		case VK_UP:
		case VK_PRIOR:	m_spikeclass++; break;
		case VK_DOWN:
		case VK_NEXT:   m_spikeclass--; break;
	}

	m_pSpkList->SetSpikeClass(m_spikeno, m_spikeclass);	
	mm_spikeclass.m_bEntryDone=FALSE;	// clear flag
	mm_spikeclass.m_nChar=0;			// empty buffer
	mm_spikeclass.SetSel(0, -1);		// select all text    
	m_bartefact = (m_spikeclass<0);
	UpdateData(FALSE);
	m_bchanged = TRUE;
	return;	
}

// --------------------------------------------------------------------------
// OnArtefact()
// Update button and change spike class
// --------------------------------------------------------------------------

void CSpikeEditDlg::OnArtefact() 
{
	UpdateData(TRUE);		// load value from control	
	m_spikeclass = (m_bartefact)? -1: 0;
	m_pSpkList->SetSpikeClass(m_spikeno, m_spikeclass);	
	UpdateData(FALSE);		// update value
	m_bchanged = TRUE;
}

// --------------------------------------------------------------------------
// OnEnChangeDisplayratio()
// update edit control and change display ratio / source view
// --------------------------------------------------------------------------

void CSpikeEditDlg::OnEnChangeDisplayratio()
{
	if (!mm_displayratio.m_bEntryDone)
		return;

	switch (mm_displayratio.m_nChar)
	{				// load data from edit controls
		case VK_RETURN:	UpdateData(TRUE);	break;
		case VK_UP:
		case VK_PRIOR:	m_displayratio++;	break;
		case VK_DOWN:
		case VK_NEXT:   m_displayratio--;	break;
	}
	mm_displayratio.m_bEntryDone=FALSE;	// clear flag
	mm_displayratio.m_nChar=0;			// empty buffer
	mm_displayratio.SetSel(0, -1);		// select all text
	if (m_displayratio < 1)
		m_displayratio = 1;    
	UpdateData(FALSE);
	m_viewdatalen = MulDiv(m_spklen, 100, m_displayratio);
	LoadSourceData();
}

// --------------------------------------------------------------------------
// OnEnChangeYextent
// update edit control and modifies gain of source view window
// --------------------------------------------------------------------------

void CSpikeEditDlg::OnEnChangeYextent()
{
	if (!mm_yvextent.m_bEntryDone)
		return;

	switch (mm_yvextent.m_nChar)
	{				// load data from edit controls
		case VK_RETURN:	UpdateData(TRUE);	break;
		case VK_UP:
		case VK_PRIOR:	m_yvextent++;	break;
		case VK_DOWN:
		case VK_NEXT:   m_yvextent--;	break;
	}
	mm_yvextent.m_bEntryDone=FALSE;	// clear flag
	mm_yvextent.m_nChar=0;			// empty buffer
	mm_yvextent.SetSel(0, -1);		// select all text
	UpdateData(FALSE);

	ASSERT(m_yvextent != 0);

	if (m_yvextent != m_yextent)
	{
		m_yextent = m_yvextent;
		m_sourceView.SetChanlistYextent(0, m_yextent);
		if (m_pSpkList->GetcompensateBaseline())
			m_sourceView.SetChanlistYextent(1, m_yextent);
		m_spkForm.SetYWExtOrg(m_yextent, m_yzero);	
		m_sourceView.Invalidate();
		m_spkForm.Invalidate();
	}
}

/////////////////////////////////////////////////////////////////////////////
// Buttons actions

// --------------------------------------------------------------------------
// LoadSourceData
// load data displayed within sourceView button
// --------------------------------------------------------------------------

void CSpikeEditDlg::LoadSourceData()
{
	if (m_dbDoc == nullptr)
		return;

	CSpikeElemt* pS = m_pSpkList->GetSpikeElemt(m_spikeno);  // get address of spike parms	
	long lFirst = pS->GetSpikeTime() - m_spkpretrig;	// change selection
	m_DWintervals.SetAt(3, lFirst);						// store interval
	long lLast = lFirst + m_spklen;						// end interval
	m_DWintervals.SetAt(4, lLast);						// store

	// compute limits of m_sourceView
	long lVFirst = lFirst + m_spklen/2 - m_viewdatalen/2;
	if (lVFirst < 0)									// check limits
		lVFirst = 0;									// clip to start of the file
	long lVLast  = lVFirst + m_viewdatalen-1;			// last pt
	if (lVLast > m_sourceView.GetDocumentLast())		// clip to size of data
	{
		lVLast = m_sourceView.GetDocumentLast();
		lVFirst = lVLast - m_viewdatalen+1;
	}
	// get data from doc
	m_spikeChan = pS->GetSpikeChannel();
	m_sourceView.SetChanlistSourceChan(0, m_spikeChan);
	m_sourceView.GetDataFromDoc(lVFirst, lVLast);	// load data from file

	// adjust offset (center spike) : use initial offset from spike	
	m_sourceView.SetChanlistYzero(0, m_yzero+pS->GetSpikeAmplitudeOffset());
	m_sourceView.SetChanlistYextent(0, m_yextent);

	if (m_pSpkList->GetcompensateBaseline())
	{
		m_sourceView.SetChanlistYzero(1,m_yzero+pS->GetSpikeAmplitudeOffset());
		m_sourceView.SetChanlistYextent(1, m_yextent);
	}
	m_sourceView.Invalidate();
}


// ----------------------------------------------------------------------
// shift spike horizontally one interval backwards or forwards

void CSpikeEditDlg::LoadSpikeFromData(int shift)
{
	if (m_dbDoc == nullptr)
		return;

	int offset = m_pSpkList->GetSpikeAmplitudeOffset(m_spikeno);
	m_iitime += shift;
	m_pSpkList->SetSpikeTime(m_spikeno, m_iitime);
	UpdateSpikeScroll();
	LoadSourceData();

	long lFirst = m_iitime - m_spkpretrig;

	// get source data buffer address
	int method = m_pSpkList->GetextractTransform();
	if (method == 0)
	{		
		int nchans;						// get buffer address and structure
		short* lpSource = m_dbDoc->LoadRawDataParams(&nchans);		
		lpSource += ((lFirst - m_dbDoc->GettBUFfirst())*nchans 
					+ m_spikeChan);
		m_pSpkList->SetSpikeData(m_spikeno, lpSource, nchans);
	}
	else
	{
		m_dbDoc->LoadTransfData(lFirst, lFirst+m_spklen, method, m_spikeChan);
		short* pData = m_dbDoc->GetpTransfDataBUF();
		pData += (lFirst - m_dbDoc->GettBUFfirst());
		m_pSpkList->SetSpikeData(m_spikeno, pData, 1);
	}

	// copy data to spike buffer
	offset += m_pSpkList->GetSpikeAmplitudeOffset(m_spikeno);
	m_pSpkList->OffsetSpikeAmplitude(m_spikeno, offset, offset);
	
	m_spkForm.Invalidate();
	m_bchanged = TRUE;
}

// ----------------------------------------------------------------------

void CSpikeEditDlg::OnDestroy() 
{
	CDialog::OnDestroy();
}

// ----------------------------------------------------------------------

void CSpikeEditDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if (pScrollBar == nullptr)
	{
		CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
		return;
	}

	// assume that code comes from SCROLLBAR1
	int shift=0;
	// get corresponding data
	switch (nSBCode)
	{
	case SB_LEFT:		shift = -10; break;	// scroll to the start
	case SB_LINELEFT:	shift =  -1; break;	// scroll one line left
	case SB_LINERIGHT:	shift =  +1; break;	// scroll one line right
	case SB_PAGELEFT:	shift = -10; break;	// scroll one page left
	case SB_PAGERIGHT:	shift = +10; break;	// scroll one page right
	case SB_RIGHT:		shift = +10; break;	// scroll to end right

	case SB_THUMBPOSITION:	// scroll to pos = nPos			
	case SB_THUMBTRACK:		// drag scroll box -- pos = nPos
		shift = nPos - SCROLLCENTER  - (m_iitime-m_iitimeold);
		break;					
	default:				// NOP: set position only
		return;
		break;			
	}
	LoadSpikeFromData(shift);
}

// ----------------------------------------------------------------------

void CSpikeEditDlg::UpdateSpikeScroll()
{
	m_HScroll_infos.fMask = SIF_PAGE | SIF_POS;
	m_HScroll_infos.nPos = m_iitime - m_iitimeold + SCROLLCENTER;
	m_HScroll_infos.nPage = 10;
	m_HScroll.SetScrollInfo(&m_HScroll_infos);
}

void CSpikeEditDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if (pScrollBar == nullptr)
	{
		CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
		return;
	}

	// assume that code comes from SCROLLBAR2
	int shift=0;
	// get corresponding data
	switch (nSBCode)
	{
	case SB_TOP:		shift = -10; break;	// scroll to the start
	case SB_LINEUP:		shift =  -1; break;	// scroll one line left
	case SB_LINEDOWN:	shift =  +1; break;	// scroll one line right
	case SB_PAGEUP:		shift = -10; break;	// scroll one page left
	case SB_PAGEDOWN:	shift = +10; break;	// scroll one page right
	case SB_BOTTOM:		shift = +10; break;	// scroll to end right

	case SB_THUMBPOSITION:	// scroll to pos = nPos			
	case SB_THUMBTRACK:		// drag scroll box -- pos = nPos
		shift = nPos - SCROLLCENTER  - 4096;
		break;					
	default:				// NOP: set position only
		return;
		break;			
	}

	m_pSpkList->OffsetSpikeAmplitude(m_spikeno, shift, shift);
	
	LoadSpikeParms();	
	m_bchanged = TRUE;
}
