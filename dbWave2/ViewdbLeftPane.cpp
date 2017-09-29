// dbLeftPaneView.cpp : implementation file
//
// left pane of dbwave application
// display data base content as buttons, and linked data as combo boxes
// allow data filtering on a given category

#include "stdafx.h"
#include <afxconv.h>           // For LPTSTR -> LPSTR macros

#include "resource.h"
#include "dbMainTable.h"
#include "DataListCtrl.h"

#include "dbWave.h"

#include "dbMainTable.h"
#include "dbWaveDoc.h"
#include "spikedoc.h"

#include "cscale.h"
#include "scopescr.h"
#include "lineview.h"
#include "editctrl.h"
#include "spikebar.h"
#include "spikeshape.h"
#include "DataListCtrl.h"

#include "ViewdbWave.h"
#include "findfilesdlg.h"
#include "dbEditFieldDlg.h"
#include "MainFrm.h"
#include "childfrm.h"
#include "progdlg.h"
#include ".\ViewdbLeftPane.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


/////////////////////////////////////////////////////////////////////////////
// CdbLeftPaneView

IMPLEMENT_DYNCREATE(CdbLeftPaneView, CDaoRecordView)

CdbLeftPaneView::CdbLeftPaneView()
	: CDaoRecordView(CdbLeftPaneView::IDD)
{
	m_pSet = NULL;
	
	m_bAddMode			=FALSE;
	m_dattransform		=0;
	m_binit				=FALSE;
	m_bUpdateFromDoc	=FALSE;
	m_bEnableActiveAccessibility=FALSE;
}

CdbLeftPaneView::~CdbLeftPaneView()
{
}

void CdbLeftPaneView::DoDataExchange(CDataExchange* pDX)
{
	CDaoRecordView::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_COMBO_INSECTID,		m_ctlinsectID);
	DDX_Control(pDX, IDC_COMBO_SENSILLUMID,		m_ctlSensillumID);
	DDX_Control(pDX, IDC_COMBO_STIMULUS,		m_ctlstim);
	DDX_Control(pDX, IDC_COMBO_CONCENTRATION,	m_ctlconc);
	DDX_Control(pDX, IDC_COMBO_STIMULUS2,		m_ctlstim2);
	DDX_Control(pDX, IDC_COMBO_CONCENTRATION2,	m_ctlconc2);
	DDX_Control(pDX, IDC_COMBO_INSECTNAME,		m_ctlinsectname);	
	DDX_Control(pDX, IDC_COMBO_LOCATION,		m_ctllocation);
	DDX_Control(pDX, IDC_COMBO_SENSILLUM,		m_ctlsensillum);
	DDX_Control(pDX, IDC_COMBO_OPERATOR,		m_ctlOperator);
	DDX_Control(pDX, IDC_COMBO_STRAIN,			m_ctlstrain);
	DDX_Control(pDX, IDC_COMBO_SEX,				m_ctlsex);
	DDX_Control(pDX, IDC_COMBO_REPEAT,			m_ctlrepeat);
	DDX_Control(pDX, IDC_COMBO_REPEAT2,			m_ctlrepeat2);
	DDX_Control(pDX, IDC_COMBO_FLAG,			m_ctlflag);
	DDX_Control(pDX, IDC_COMBO_DATE,			m_ctldate);
	DDX_Control(pDX, IDC_COMBO_EXPT,			m_ctlexpt);

	if (m_pSet != NULL)
	{
		DDX_Check(pDX, IDC_CHECKINSECT,			m_pSet->m_desc[CH_INSECTNAME_ID].bFilter1);
		DDX_Check(pDX, IDC_CHECKSENSILLUM,		m_pSet->m_desc[CH_SENSILLUMNAME_ID].bFilter1); 
		DDX_Check(pDX, IDC_CHECKSTRAIN,			m_pSet->m_desc[CH_STRAIN_ID].bFilter1); 
		DDX_Check(pDX, IDC_CHECKSEX,			m_pSet->m_desc[CH_SEX_ID].bFilter1); 
		DDX_Check(pDX, IDC_CHECKLOCATION,		m_pSet->m_desc[CH_LOCATION_ID].bFilter1); 
		DDX_Check(pDX, IDC_CHECKSTIMULUS,		m_pSet->m_desc[CH_STIM_ID].bFilter1); 
		DDX_Check(pDX, IDC_CHECKCONCENTRATION,	m_pSet->m_desc[CH_CONC_ID].bFilter1); 
		DDX_Check(pDX, IDC_CHECKOPERATOR,		m_pSet->m_desc[CH_OPERATOR_ID].bFilter1); 
		DDX_Check(pDX, IDC_CHECKINSECTID,		m_pSet->m_desc[CH_INSECTID].bFilter1); 
		DDX_Check(pDX, IDC_CHECKSENSILLUMID,	m_pSet->m_desc[CH_SENSILLUMID].bFilter1); 
		DDX_Check(pDX, IDC_CHECKSTIMULUS2,		m_pSet->m_desc[CH_STIM2_ID].bFilter1); 
		DDX_Check(pDX, IDC_CHECKCONCENTRATION2, m_pSet->m_desc[CH_CONC2_ID].bFilter1); 
		DDX_Check(pDX, IDC_CHECKREPEAT,			m_pSet->m_desc[CH_REPEAT].bFilter1); 
		DDX_Check(pDX, IDC_CHECKREPEAT2,		m_pSet->m_desc[CH_REPEAT2].bFilter1);
		DDX_Check(pDX, IDC_CHECKFLAG,			m_pSet->m_desc[CH_FLAG].bFilter1); 
		DDX_Check(pDX, IDC_CHECKDATE,			m_pSet->m_desc[CH_ACQDATE_DAY].bFilter1); 
		DDX_Check(pDX, IDC_CHECKEXPT,			m_pSet->m_desc[CH_EXPT_ID].bFilter1); 
	}
}

BEGIN_MESSAGE_MAP(CdbLeftPaneView, CDaoRecordView)

	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_CHECKINSECT,			&CdbLeftPaneView::OnFilter_insectName)
	ON_BN_CLICKED(IDC_CHECKSENSILLUM,		&CdbLeftPaneView::OnFilter_sensillum)
	ON_BN_CLICKED(IDC_CHECKLOCATION,		&CdbLeftPaneView::OnFilter_location)
	ON_BN_CLICKED(IDC_CHECKSTIMULUS,		&CdbLeftPaneView::OnFilter_stimulus)
	ON_BN_CLICKED(IDC_CHECKCONCENTRATION,	&CdbLeftPaneView::OnFilter_concentration)
	ON_BN_CLICKED(IDC_CHECKOPERATOR,		&CdbLeftPaneView::OnFilter_operator)
	ON_BN_CLICKED(IDC_CHECKINSECTID,		&CdbLeftPaneView::OnFilter_insectID)
	ON_BN_CLICKED(IDC_CHECKSENSILLUMID,		&CdbLeftPaneView::OnFilter_sensillumID)
	ON_BN_CLICKED(IDC_CHECKSTIMULUS2,		&CdbLeftPaneView::OnFilter_stimulus2)
	ON_BN_CLICKED(IDC_CHECKCONCENTRATION2,	&CdbLeftPaneView::OnFilter_concentration2)
	ON_BN_CLICKED(IDC_CHECKFLAG,			&CdbLeftPaneView::OnFilter_flag)
	ON_BN_CLICKED(IDC_CHECKSEX,				&CdbLeftPaneView::OnFilter_sex)
	ON_BN_CLICKED(IDC_CHECKSTRAIN,			&CdbLeftPaneView::OnFilter_strain)
	ON_BN_CLICKED(IDC_CHECKREPEAT,			&CdbLeftPaneView::OnFilter_repeat)
	ON_BN_CLICKED(IDC_CHECKREPEAT2,			&CdbLeftPaneView::OnFilter_repeat2)
	ON_BN_CLICKED(IDC_CHECKDATE,			&CdbLeftPaneView::OnFilter_date)
	ON_BN_CLICKED(IDC_CHECKEXPT,			&CdbLeftPaneView::OnFilter_expt)

	ON_BN_CLICKED(IDC_BUTTONINSECT,			&CdbLeftPaneView::OnEditChange_insectname)
	ON_BN_CLICKED(IDC_BUTTONSENSILLUM,		&CdbLeftPaneView::OnEditChange_sensillum)
	ON_BN_CLICKED(IDC_BUTTONCONCENTRATION,	&CdbLeftPaneView::OnEditChange_concentration)
	ON_BN_CLICKED(IDC_BUTTONOPERATOR,		&CdbLeftPaneView::OnEditChange_operator)
	ON_BN_CLICKED(IDC_BUTTONSTIMULUS,		&CdbLeftPaneView::OnEditChange_stimulus)
	ON_BN_CLICKED(IDC_BUTTONLOCATION,		&CdbLeftPaneView::OnEditChange_location)
	ON_BN_CLICKED(IDC_BUTTONINSECTID,		&CdbLeftPaneView::OnEditChange_insectID)
	ON_BN_CLICKED(IDC_BUTTONSENSILLUMID,	&CdbLeftPaneView::OnEditChange_sensillumID)
	ON_BN_CLICKED(IDC_BUTTONSTIMULUS2,		&CdbLeftPaneView::OnEditChange_stimulus2)
	ON_BN_CLICKED(IDC_BUTTONCONCENTRATION2, &CdbLeftPaneView::OnEditChange_concentration2)
	ON_BN_CLICKED(IDC_BUTTONSEX,			&CdbLeftPaneView::OnEditChange_sex)
	ON_BN_CLICKED(IDC_BUTTONSTRAIN,			&CdbLeftPaneView::OnEditChange_strain)
	ON_BN_CLICKED(IDC_BUTTONREPEAT,			&CdbLeftPaneView::OnEditChange_repeat)
	ON_BN_CLICKED(IDC_BUTTONREPEAT2,		&CdbLeftPaneView::OnEditChange_repeat2)
	ON_BN_CLICKED(IDC_BUTTONFLAG,			&CdbLeftPaneView::OnEditChange_flag)
	ON_BN_CLICKED(IDC_BUTTONEXPT,			&CdbLeftPaneView::OnEditChange_expt)

	ON_CBN_CLOSEUP(IDC_COMBO_INSECTNAME,	&CdbLeftPaneView::OnEnChange_insectName)
	ON_CBN_CLOSEUP(IDC_COMBO_SENSILLUM,		&CdbLeftPaneView::OnEnChange_sensillum)
	ON_CBN_CLOSEUP(IDC_COMBO_STIMULUS,		&CdbLeftPaneView::OnEnChange_stimulus)
	ON_CBN_CLOSEUP(IDC_COMBO_CONCENTRATION, &CdbLeftPaneView::OnEnChange_concentration)
	ON_CBN_CLOSEUP(IDC_COMBO_OPERATOR,		&CdbLeftPaneView::OnEnChange_operator)
	ON_CBN_CLOSEUP(IDC_COMBO_LOCATION,		&CdbLeftPaneView::OnEnChange_location)
	ON_CBN_CLOSEUP(IDC_COMBO_INSECTID,		&CdbLeftPaneView::OnEnChange_insectID)
	ON_CBN_CLOSEUP(IDC_COMBO_SENSILLUMID,	&CdbLeftPaneView::OnEnChange_sensillumID)
	ON_CBN_CLOSEUP(IDC_COMBO_STIMULUS2,		&CdbLeftPaneView::OnEnChange_stimulus2)
	ON_CBN_CLOSEUP(IDC_COMBO_CONCENTRATION2,&CdbLeftPaneView::OnEnChange_concentration2)
	ON_CBN_CLOSEUP(IDC_COMBO_STRAIN,		&CdbLeftPaneView::OnEnChange_strain)
	ON_CBN_CLOSEUP(IDC_COMBO_SEX,			&CdbLeftPaneView::OnEnChange_sex)
	ON_CBN_CLOSEUP(IDC_COMBO_REPEAT,		&CdbLeftPaneView::OnEnChange_repeat)
	ON_CBN_CLOSEUP(IDC_COMBO_REPEAT2,		&CdbLeftPaneView::OnEnChange_repeat2)
	ON_CBN_CLOSEUP(IDC_COMBO_FLAG,			&CdbLeftPaneView::OnEnChange_flag)
	ON_CBN_CLOSEUP(IDC_COMBO_DATE,			&CdbLeftPaneView::OnEnChange_date)
	ON_CBN_CLOSEUP(IDC_COMBO_EXPT,			&CdbLeftPaneView::OnEnChange_expt)

	ON_CBN_SELCHANGE(IDC_COMBO_INSECTNAME,	&CdbLeftPaneView::OnEnChange_insectName)
	ON_CBN_SELCHANGE(IDC_COMBO_SENSILLUM,	&CdbLeftPaneView::OnEnChange_sensillum)
	ON_CBN_SELCHANGE(IDC_COMBO_LOCATION,	&CdbLeftPaneView::OnEnChange_location)
	ON_CBN_SELCHANGE(IDC_COMBO_STIMULUS,	&CdbLeftPaneView::OnEnChange_stimulus)
	ON_CBN_SELCHANGE(IDC_COMBO_CONCENTRATION, &CdbLeftPaneView::OnEnChange_concentration)
	ON_CBN_SELCHANGE(IDC_COMBO_OPERATOR,	&CdbLeftPaneView::OnEnChange_operator)
	ON_CBN_SELCHANGE(IDC_COMBO_SENSILLUMID, &CdbLeftPaneView::OnEnChange_sensillumID)
	ON_CBN_SELCHANGE(IDC_COMBO_INSECTID,	&CdbLeftPaneView::OnEnChange_insectID)
	ON_CBN_SELCHANGE(IDC_COMBO_STIMULUS2,	&CdbLeftPaneView::OnEnChange_stimulus2)
	ON_CBN_SELCHANGE(IDC_COMBO_CONCENTRATION2, &CdbLeftPaneView::OnEnChange_concentration2)
	ON_CBN_SELCHANGE(IDC_COMBO_STRAIN,		&CdbLeftPaneView::OnEnChange_strain)
	ON_CBN_SELCHANGE(IDC_COMBO_SEX,			&CdbLeftPaneView::OnEnChange_sex)
	ON_CBN_SELCHANGE(IDC_COMBO_REPEAT,		&CdbLeftPaneView::OnEnChange_repeat)
	ON_CBN_SELCHANGE(IDC_COMBO_REPEAT2,		&CdbLeftPaneView::OnEnChange_repeat2)
	ON_CBN_SELCHANGE(IDC_COMBO_FLAG,		&CdbLeftPaneView::OnEnChange_flag)
	ON_CBN_SELCHANGE(IDC_COMBO_DATE,		&CdbLeftPaneView::OnEnChange_date)
	ON_CBN_SELCHANGE(IDC_COMBO_EXPT,		&CdbLeftPaneView::OnEnChange_expt)

	ON_CBN_EDITCHANGE(IDC_COMBO_CONCENTRATION2, &CdbLeftPaneView::OnEditChange_concentration2)
	ON_CBN_EDITCHANGE(IDC_COMBO_STIMULUS2,	&CdbLeftPaneView::OnEditChange_stimulus2)
	ON_CBN_EDITCHANGE(IDC_COMBO_INSECTNAME, &CdbLeftPaneView::OnEditChange_insectname)
	ON_CBN_EDITCHANGE(IDC_COMBO_SENSILLUM,	&CdbLeftPaneView::OnEditChange_sensillum)
	ON_CBN_EDITCHANGE(IDC_COMBO_LOCATION,	&CdbLeftPaneView::OnEditChange_location)
	ON_CBN_EDITCHANGE(IDC_COMBO_STIMULUS,	&CdbLeftPaneView::OnEditChange_stimulus)
	ON_CBN_EDITCHANGE(IDC_COMBO_CONCENTRATION, &CdbLeftPaneView::OnEditChange_concentration)
	ON_CBN_EDITCHANGE(IDC_COMBO_OPERATOR,	&CdbLeftPaneView::OnEditChange_operator)
	ON_CBN_EDITCHANGE(IDC_COMBO_SENSILLUMID, &CdbLeftPaneView::OnEditChange_sensillumID)
	ON_CBN_EDITCHANGE(IDC_COMBO_INSECTID,	&CdbLeftPaneView::OnEditChange_insectID)
	ON_CBN_EDITCHANGE(IDC_COMBO_SEX,		&CdbLeftPaneView::OnEditChange_sex)
	ON_CBN_EDITCHANGE(IDC_COMBO_STRAIN,		&CdbLeftPaneView::OnEditChange_strain)
	ON_CBN_EDITCHANGE(IDC_COMBO_REPEAT,		&CdbLeftPaneView::OnEditChange_repeat)
	ON_CBN_EDITCHANGE(IDC_COMBO_REPEAT2,	&CdbLeftPaneView::OnEditChange_repeat2)
	ON_CBN_EDITCHANGE(IDC_COMBO_FLAG,		&CdbLeftPaneView::OnEditChange_flag)
	ON_CBN_EDITCHANGE(IDC_COMBO_DATE,		&CdbLeftPaneView::OnEditChange_date)
	ON_CBN_EDITCHANGE(IDC_COMBO_EXPT,		&CdbLeftPaneView::OnEditChange_expt)
	
	ON_COMMAND(ID_RECORD_ADD, OnRecordAdd)
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_FILE_PRINT, CDaoRecordView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CDaoRecordView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CDaoRecordView::OnFilePrintPreview)
	ON_BN_CLICKED(IDC_CHECKSORT, OnBnClickedChecksort)
	ON_CBN_SELCHANGE(IDC_SORT, OnCbnSelchangeSort)
	ON_BN_CLICKED(IDC_BUTTONREPEAT3, &CdbLeftPaneView::OnBnClickedButtonrepeat3)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CdbLeftPaneView message handlers

BOOL CdbLeftPaneView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	return CDaoRecordView::PreCreateWindow(cs);
}

void CdbLeftPaneView::OnInitialUpdate()
{
	m_stretch.AttachParent(this);		// attach formview pointer

	m_stretch.newProp(IDC_COMBO_INSECTID,		XLEQ_XREQ,SZEQ_YTEQ);
	m_stretch.newProp(IDC_COMBO_SENSILLUMID,	XLEQ_XREQ,SZEQ_YTEQ);
	m_stretch.newProp(IDC_COMBO_INSECTNAME,		XLEQ_XREQ,SZEQ_YTEQ);
	m_stretch.newProp(IDC_COMBO_SENSILLUM,		XLEQ_XREQ,SZEQ_YTEQ);
	m_stretch.newProp(IDC_COMBO_LOCATION,		XLEQ_XREQ,SZEQ_YTEQ);
	m_stretch.newProp(IDC_COMBO_STIMULUS,		XLEQ_XREQ,SZEQ_YTEQ);
	m_stretch.newProp(IDC_COMBO_CONCENTRATION,	XLEQ_XREQ,SZEQ_YTEQ);
	m_stretch.newProp(IDC_COMBO_STIMULUS2,		XLEQ_XREQ,SZEQ_YTEQ);
	m_stretch.newProp(IDC_COMBO_CONCENTRATION2,	XLEQ_XREQ,SZEQ_YTEQ);
	m_stretch.newProp(IDC_COMBO_OPERATOR,		XLEQ_XREQ,SZEQ_YTEQ);
	m_stretch.newProp(IDC_COMBO_STRAIN,			XLEQ_XREQ,SZEQ_YTEQ);
	m_stretch.newProp(IDC_COMBO_SEX,			XLEQ_XREQ,SZEQ_YTEQ);
	m_stretch.newProp(IDC_COMBO_REPEAT,			XLEQ_XREQ,SZEQ_YTEQ);
	m_stretch.newProp(IDC_COMBO_REPEAT2,		XLEQ_XREQ,SZEQ_YTEQ);
	m_stretch.newProp(IDC_COMBO_DATE,			XLEQ_XREQ,SZEQ_YTEQ);
	m_stretch.newProp(IDC_COMBO_EXPT,			XLEQ_XREQ,SZEQ_YTEQ);

	m_stretch.newProp(IDC_BUTTONINSECTID,		SZEQ_XREQ,SZEQ_YTEQ);
	m_stretch.newProp(IDC_BUTTONSENSILLUMID,	SZEQ_XREQ,SZEQ_YTEQ);
	m_stretch.newProp(IDC_BUTTONINSECT,			SZEQ_XREQ,SZEQ_YTEQ);
	m_stretch.newProp(IDC_BUTTONSENSILLUM,		SZEQ_XREQ,SZEQ_YTEQ);
	m_stretch.newProp(IDC_BUTTONLOCATION,		SZEQ_XREQ,SZEQ_YTEQ);
	m_stretch.newProp(IDC_BUTTONSTIMULUS,		SZEQ_XREQ,SZEQ_YTEQ);
	m_stretch.newProp(IDC_BUTTONCONCENTRATION,	SZEQ_XREQ,SZEQ_YTEQ);
	m_stretch.newProp(IDC_BUTTONSTIMULUS2,		SZEQ_XREQ,SZEQ_YTEQ);
	m_stretch.newProp(IDC_BUTTONCONCENTRATION2,	SZEQ_XREQ,SZEQ_YTEQ);
	m_stretch.newProp(IDC_BUTTONOPERATOR,		SZEQ_XREQ,SZEQ_YTEQ);
	m_stretch.newProp(IDC_BUTTONSTRAIN,			SZEQ_XREQ,SZEQ_YTEQ);
	m_stretch.newProp(IDC_BUTTONSEX,			SZEQ_XREQ,SZEQ_YTEQ);
	m_stretch.newProp(IDC_BUTTONREPEAT,			SZEQ_XREQ,SZEQ_YTEQ);
	m_stretch.newProp(IDC_BUTTONREPEAT2,		SZEQ_XREQ,SZEQ_YTEQ);
	m_stretch.newProp(IDC_BUTTONEXPT,			SZEQ_XREQ,SZEQ_YTEQ);

	m_binit = TRUE;

	// Get document and data
	CdbWaveDoc* pdbDoc = GetDocument();
	m_pSet = &pdbDoc->m_pDB->m_tableSet;
	if (m_pSet->m_desc[CH_INSECTID].liArray.GetSize() == 0)
		m_pSet->BuildAndSortIDArrays();	

	// fetch data into the controls
	if (m_pSet->m_strSort.IsEmpty())
	{
		m_pSet->m_strSort = _T("acq_date");
		m_pSet->RefreshQuery();
	}
	// init the view
	CDaoRecordView::OnInitialUpdate();
	PopulateControls();
	OnMove(ID_RECORD_LAST);
}

// //////////////////////////////////////////////////////////////////////////////////
// populate control and select
// Input parameters:
// pointer to table
// pointer to combo to fill in
// pointer to maximum number of values

void CdbLeftPaneView::PopulateComboFromLinkedTable(CComboBox* pcombo, CDaoRecordset& linkedtableSet, LPCTSTR csColName)
{
	int i;
	pcombo->ResetContent();
	CString cs;
	CString str2 = csColName;
	BOOL flag2 = str2.IsEmpty();
	if (linkedtableSet.IsOpen() && !linkedtableSet.IsBOF()) 
	{
		COleVariant varValue0, varValue1;
		linkedtableSet.MoveFirst();
		while(!linkedtableSet.IsEOF()) 
		{
			linkedtableSet.GetFieldValue(0, varValue0);
			linkedtableSet.GetFieldValue(1, varValue1);
			long ID = varValue1.lVal;
			// add string only if found into m_pSet...
			cs.Format(_T("%s=%li"), csColName, ID);
			BOOL flag = m_pSet->FindFirst(cs);
			if (!flag2 && (flag != 0))
			{
				// OK, proceed
				i = pcombo->AddString(varValue0.bstrVal);
				pcombo->SetItemData(i, ID);
			}
			linkedtableSet.MoveNext();
		}
	}
	pcombo->SetCurSel(0);
}

void CdbLeftPaneView::PopulateComboFromTable (CComboBox* pcombo, int indexcol)
{
	CdbWaveDoc* pdbDoc = GetDocument();
	
	// fill CComboBox only with available data
	pcombo->ResetContent();
	CString cs;		// to construct insect and sensillum number
	CString str;	// to store FindFirst filter
	CdbMainTable* pSet = &pdbDoc->m_pDB->m_tableSet;
	CArray<long, long>* pIDarray = &pSet->m_desc[indexcol].liArray;
	CString cscolhead = pSet->m_desc[indexcol].csColName;
	int arraySize = pIDarray->GetSize();

	for (int i=0; i< arraySize; i++)
	{
		unsigned int iID = pIDarray->GetAt(i);
		// add string only if found into m_pSet...
		str.Format(_T("%s=%li"), cscolhead, iID);
		BOOL flag = pSet->FindFirst(str);
		if (flag != 0)
		{
			cs.Format(_T("%i"), iID);
			int j= pcombo->AddString(cs);
			pcombo->SetItemData(j, iID);
		}
	}
	pcombo->SetCurSel(0);
}

void CdbLeftPaneView::PopulateComboFromTablewithDate (CComboBox* pcombo, int indexcol)
{
	CdbWaveDoc* pdbDoc = GetDocument();
	CdbMainTable* pSet = &(pdbDoc->m_pDB->m_tableSet);

	// fill CComboBox only with available data
	pcombo->ResetContent();
	CString cs;		// to construct date
	CString cscolhead = pdbDoc->m_pDB->m_tableSet.m_desc[indexcol].csColName;
	CString str;	// to construct filter

	CArray<COleDateTime, COleDateTime>*	ptiArray = &pSet->m_desc[CH_ACQDATE_DAY].tiArray;
	int arraySize = ptiArray->GetSize();
	for (int i=0; i< arraySize; i++)
	{
		COleDateTime oTime = ptiArray->GetAt(i);
		cs= oTime.Format(_T("%m/%d/%y"));
		// add string only if found into m_pSet...
		str.Format(_T("%s=#%s#"), cscolhead, cs);
		BOOL flag = m_pSet->FindFirst(str);
		if (flag != 0)
		{
			cs= oTime.Format(VAR_DATEVALUEONLY);
			int j= pcombo->AddString(cs);
		}
	}
	pcombo->SetCurSel(0);
}	

void CdbLeftPaneView::PopulateControls()
{
	CdbWaveDoc* pdbDoc = GetDocument();
	CdbWdatabase* pDB = pdbDoc->m_pDB;
	if (pDB->m_tableSet.IsBOF() && pDB->m_tableSet.IsEOF())
		return;

	// fill combo boxes associated with tables
	PopulateComboFromLinkedTable (&m_ctlOperator,	pDB->m_operatorSet,		pDB->m_tableSet.m_desc[CH_OPERATOR_ID].csColName);		// operator_ID
	PopulateComboFromLinkedTable (&m_ctlinsectname, pDB->m_insectnameSet,	pDB->m_tableSet.m_desc[CH_INSECTNAME_ID].csColName);		// insectname_ID
	PopulateComboFromLinkedTable (&m_ctlsensillum,	pDB->m_sensillumnameSet,pDB->m_tableSet.m_desc[CH_SENSILLUMNAME_ID].csColName);	// sensillumname_ID
	PopulateComboFromLinkedTable (&m_ctllocation,	pDB->m_locationSet,		pDB->m_tableSet.m_desc[CH_LOCATION_ID].csColName);		// location_ID
	PopulateComboFromLinkedTable (&m_ctlstim,		pDB->m_stimSet,			pDB->m_tableSet.m_desc[CH_STIM_ID].csColName);			// stim_ID
	PopulateComboFromLinkedTable (&m_ctlconc,		pDB->m_concSet,			pDB->m_tableSet.m_desc[CH_CONC_ID].csColName);			// conc_ID
	PopulateComboFromLinkedTable (&m_ctlstim2,		pDB->m_stimSet,			pDB->m_tableSet.m_desc[CH_STIM2_ID].csColName);			// stim2_ID
	PopulateComboFromLinkedTable (&m_ctlconc2,		pDB->m_concSet,			pDB->m_tableSet.m_desc[CH_CONC2_ID].csColName);			// conc2_ID
	PopulateComboFromLinkedTable (&m_ctlstrain,		pDB->m_strainSet,		pDB->m_tableSet.m_desc[CH_STRAIN_ID].csColName);			// strain_ID
	PopulateComboFromLinkedTable (&m_ctlsex,		pDB->m_sexSet,			pDB->m_tableSet.m_desc[CH_SEX_ID].csColName);			// sex_ID
	PopulateComboFromLinkedTable (&m_ctlexpt,		pDB->m_exptSet,			pDB->m_tableSet.m_desc[CH_EXPT_ID].csColName);			// expt_ID

	// fill simple combos
	PopulateComboFromTable (&m_ctlinsectID,		CH_INSECTID);
	PopulateComboFromTable (&m_ctlSensillumID,	CH_SENSILLUMID);
	PopulateComboFromTable (&m_ctlrepeat,		CH_REPEAT);
	PopulateComboFromTable (&m_ctlrepeat2,		CH_REPEAT2);
	PopulateComboFromTable (&m_ctlflag,			CH_FLAG);
	PopulateComboFromTablewithDate (&m_ctldate,	CH_ACQDATE_DAY);
}

/////////////////////////////////////////////////////////////////////////////
// CdbLeftPaneView printing

BOOL CdbLeftPaneView::OnPreparePrinting(CPrintInfo* pInfo)
{
	if (!CView::DoPreparePrinting(pInfo))
		return FALSE;
	if (!COleDocObjectItem::OnPreparePrinting(this, pInfo))
		return FALSE;
	return TRUE;
}

void CdbLeftPaneView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CdbLeftPaneView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CdbLeftPaneView::OnPrint(CDC* pDC, CPrintInfo* pInfo)
{
	// TODO: add customized printing code here
	if(pInfo->m_bDocObject)
		COleDocObjectItem::OnPrint(this, pInfo, TRUE);
	else
		CView::OnPrint(pDC, pInfo);
}

void CdbLeftPaneView::OnDestroy()
{
	CDaoRecordView::OnDestroy();
}

/////////////////////////////////////////////////////////////////////////////
// CdbLeftPaneView diagnostics

#ifdef _DEBUG
void CdbLeftPaneView::AssertValid() const
{
	CDaoRecordView::AssertValid();
}

void CdbLeftPaneView::Dump(CDumpContext& dc) const
{
	CDaoRecordView::Dump(dc);
}

CdbWaveDoc* CdbLeftPaneView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CdbWaveDoc)));
	return (CdbWaveDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CdbLeftPaneView database support

CDaoRecordset* CdbLeftPaneView::OnGetRecordset()
{
	CdbWaveDoc* pdbDoc = (CdbWaveDoc*) GetDocument();	// get pointer to document
	//ASSERT(pdbDoc != NULL);								// debug: check that doc is defined
	m_pSet = &pdbDoc->m_pDB->m_tableSet;
	return m_pSet;
}

/////////////////////////////////////////////////////////////////////////////
// CdbLeftPaneView message handlers
void CdbLeftPaneView::UpdateControls()
{
	//ATLTRACE2(_T("CdbLeftPaneView : UpdateControls\n"));
	// if ID has changed, load corresp data from stim combo
	// select item that has the same long value as the new ID
	// do the same for all combobox(es)
	OnMoveUpdateControls (&m_pSet->m_operator_ID,	m_ctlOperator,		FALSE);
	OnMoveUpdateControls (&m_pSet->m_insectname_ID,	m_ctlinsectname,	FALSE);
	OnMoveUpdateControls (&m_pSet->m_location_ID,	m_ctllocation,		FALSE);
	OnMoveUpdateControls (&m_pSet->m_sensillumname_ID, m_ctlsensillum,	FALSE);
	OnMoveUpdateControls (&m_pSet->m_stim_ID,		m_ctlstim,			FALSE);
	OnMoveUpdateControls (&m_pSet->m_conc_ID,		m_ctlconc,			FALSE);
	OnMoveUpdateControls (&m_pSet->m_stim2_ID,		m_ctlstim2,			FALSE);
	OnMoveUpdateControls (&m_pSet->m_conc2_ID,		m_ctlconc2,			FALSE);
	OnMoveUpdateControls (&m_pSet->m_insectID,		m_ctlinsectID,		FALSE);
	OnMoveUpdateControls (&m_pSet->m_sensillumID,	m_ctlSensillumID,	FALSE);
	OnMoveUpdateControls (&m_pSet->m_sex_ID,		m_ctlsex,			FALSE);
	OnMoveUpdateControls (&m_pSet->m_strain_ID,		m_ctlstrain,		FALSE);
	OnMoveUpdateControls (&m_pSet->m_repeat,		m_ctlrepeat,		FALSE);
	OnMoveUpdateControls (&m_pSet->m_repeat2,		m_ctlrepeat2,		FALSE);
	OnMoveUpdateControls (&m_pSet->m_flag,			m_ctlflag,			FALSE);
	OnMoveUpdateControls (&m_pSet->m_acqdate_day,	m_ctldate,			TRUE);
	OnMoveUpdateControls (&m_pSet->m_expt_ID,		m_ctlexpt,			FALSE);
	
	// check sort filter
	int buttonstate = BST_UNCHECKED;
	if(!m_pSet->m_strSort.IsEmpty())
	{
		buttonstate = BST_CHECKED;
		// if filter from somewhere else, check if the combo already has the corresp filter and select it
		CComboBox* pCombo = (CComboBox*) GetDlgItem(IDC_SORT);
		int ipos = pCombo->FindString(-1, m_pSet->m_strSort);
		// add chain if the sort filter is not found in the combo
		if (ipos == CB_ERR)
			ipos = pCombo->AddString(m_pSet->m_strSort);
		ASSERT(ipos != CB_ERR);
		pCombo->SetCurSel(ipos);
	}
	((CButton*) GetDlgItem(IDC_CHECKSORT))->SetCheck(buttonstate);
	OnBnClickedChecksort();
}

void CdbLeftPaneView::OnMoveUpdateControls (void *piID, CComboBox& cb, BOOL bDate)
{
	//  if iID is empty and the value is not defined? show empty value and return
	if (m_pSet->IsFieldNull(piID))
	{
		cb.SetCurSel(-1);
		return;
	}

	// otherwise, select corresponding item in the combo box
	if (!bDate)
	{
		DWORD iID = *((DWORD*) piID);
		if (iID != cb.GetItemData(cb.GetCurSel()))
		{
			int i=0;
			for (i = 0; i< cb.GetCount(); i++)
			{
				if (iID == cb.GetItemData(i))
					break;
			}
			if (iID == cb.GetItemData(i))
				cb.SetCurSel(i);
		}
	}
	// date
	else
	{
		COleDateTime oTime = *((COleDateTime*) piID);
		CString cs = oTime.Format(VAR_DATEVALUEONLY);
		int i = cb.FindStringExact(0, cs);
		cb.SetCurSel(i);
	}
}

BOOL CdbLeftPaneView::OnMove(UINT nIDMoveCommand)
{    
	//ATLTRACE2(_T("CdbLeftPaneView : OnMove\n"));
	BOOL flag = TRUE;
	CDaoException e;
	TRY 
	{
		GetDocument()->m_pDB->MoveRecord(nIDMoveCommand);
		GetDocument()->UpdateAllViews(NULL, HINT_DOCMOVERECORD, NULL);	
	}
	CATCH( CDaoException, e )
	{
		#ifdef _DEBUG
			afxDump << _T("no record found \n");
		#endif
		flag = FALSE;
	}
	END_CATCH
	return flag;
}

// return pointers to parameters useful to change/edit/filter item
// IDC = ID descriptor for check box

DB_ITEMDESC* CdbLeftPaneView::GetItemDescriptors(int IDC)
{
	DB_ITEMDESC* pdesc = NULL;
	CdbWaveDoc* pdbDoc = GetDocument();
	int ich = -1;
	switch (IDC)
	{
	case IDC_CHECKINSECT:
		ich = CH_INSECTNAME_ID;
		m_pSet->m_desc[ich].pComboBox = &m_ctlinsectname;
		break;
	case IDC_CHECKSENSILLUM:
		ich = CH_SENSILLUMNAME_ID;
		m_pSet->m_desc[ich].pComboBox = &m_ctlsensillum;
		break;
	case IDC_CHECKSTRAIN:
		ich = CH_STRAIN_ID;
		m_pSet->m_desc[ich].pComboBox = &m_ctlstrain;
		break;
	case IDC_CHECKSEX:
		ich = CH_SEX_ID;
		m_pSet->m_desc[ich].pComboBox = &m_ctlsex;
		break;
	case IDC_CHECKLOCATION:
		ich = CH_LOCATION_ID;
		m_pSet->m_desc[ich].pComboBox = &m_ctllocation;
		break;
	case IDC_CHECKSTIMULUS:
		ich = CH_STIM_ID;
		m_pSet->m_desc[ich].pComboBox = &m_ctlstim;
		break;
	case IDC_CHECKCONCENTRATION:
		ich = CH_CONC_ID;
		m_pSet->m_desc[ich].pComboBox = &m_ctlconc;
		break;
	case IDC_CHECKOPERATOR:
		ich = CH_OPERATOR_ID;
		m_pSet->m_desc[ich].pComboBox = &m_ctlOperator;
		break;
	case IDC_CHECKSENSILLUMID:
		ich = CH_SENSILLUMID;
		m_pSet->m_desc[ich].pComboBox = &m_ctlSensillumID;
		break;
	case IDC_CHECKINSECTID:
		ich = CH_INSECTID;
		m_pSet->m_desc[ich].pComboBox = &m_ctlinsectID;
		break;
	case IDC_CHECKSTIMULUS2:
		ich = CH_STIM2_ID;
		m_pSet->m_desc[ich].pComboBox = &m_ctlstim2;
		break;
	case IDC_CHECKCONCENTRATION2:
		ich = CH_CONC2_ID;
		m_pSet->m_desc[ich].pComboBox = &m_ctlconc2;
		break;
	case IDC_CHECKREPEAT:
		ich = CH_REPEAT;
		m_pSet->m_desc[ich].pComboBox = &m_ctlrepeat;
		break;
	case IDC_CHECKREPEAT2:
		ich = CH_REPEAT2;
		m_pSet->m_desc[ich].pComboBox = &m_ctlrepeat2;
		break;
	case IDC_CHECKFLAG:
		ich = CH_FLAG;
		m_pSet->m_desc[ich].pComboBox = &m_ctlflag;
		break;
	case IDC_CHECKEXPT:
		ich = CH_EXPT_ID;
		m_pSet->m_desc[ich].pComboBox = &m_ctlexpt;
		break;
		//case IDC_CHECKDATE:
		//	ich = CH_ACQDATE_DAY;
		//	m_pSet->m_desc[ich].pComboBox	= &m_ctldate;
		//	break;
	default:
		ich = -1;
		break;
	}
	if (ich >= 0)
		pdesc = pdbDoc->m_pDB->GetRecordItemDescriptor(ich);

	return pdesc;
}

// ---------------------------------------------------------------------------------
// change item / update filter for item
// IDC = ID descriptor for check box

void CdbLeftPaneView::OnEnChange_Item(int IDC)
{
	DB_ITEMDESC* pdesc = GetItemDescriptors(IDC);
	if (pdesc->pComboBox == NULL)
		return;
	if (m_bAddMode) 
		return;

	if (m_pSet->m_bFilterON && pdesc->bFilter1)
		OnFilter_Item(IDC);
	else
	{
		int nsel = pdesc->pComboBox->GetCurSel();
		CString csContent;
		CString str;
		pdesc->pComboBox->GetLBText(nsel, csContent);
		if ((FIELD_IND_TEXT == pdesc->typeLocal)  || (FIELD_IND_FILEPATH == pdesc->typeLocal))
		{
			long iID;
			pdesc->plinkedSet->FindString (csContent, iID);
			str.Format(_T("%s = %li"), (LPCTSTR) pdesc->csColName, iID);
		}
		else // if ((FIELD_LONG  == pdesc->type) || (FIELD_TEXT  == pdesc->type) )
			str.Format(_T("%s = %s"), (LPCTSTR) pdesc->csColName, (LPCTSTR) csContent);
		// (FIELD_DATE == pdesc->type): str.Format(_T("%s=#%s#"), cscolhead, cs);
		m_pSet->FindFirst(str);
		int irecpos = m_pSet->GetAbsolutePosition();
		GetDocument()->SetIndexCurrentFile(irecpos);
		GetDocument()->UpdateAllViews(this, HINT_DOCMOVERECORD, NULL);
	}
}

void CdbLeftPaneView::OnEnChange_expt()
{
	OnEnChange_Item(IDC_CHECKEXPT);
}

void CdbLeftPaneView::OnEnChange_insectName() 
{
	OnEnChange_Item(IDC_CHECKINSECT);
}

void CdbLeftPaneView::OnEnChange_sensillum() 
{
	OnEnChange_Item(IDC_CHECKSENSILLUM);
}

void CdbLeftPaneView::OnEnChange_location() 
{
	OnEnChange_Item(IDC_CHECKLOCATION);
}

void CdbLeftPaneView::OnEnChange_stimulus() 
{
	OnEnChange_Item(IDC_CHECKSTIMULUS);
}

void CdbLeftPaneView::OnEnChange_sex() 
{
	OnEnChange_Item(IDC_CHECKSEX);
}

void CdbLeftPaneView::OnEnChange_strain() 
{
	OnEnChange_Item(IDC_CHECKSTRAIN);
}

void CdbLeftPaneView::OnEnChange_concentration() 
{
	OnEnChange_Item(IDC_CHECKCONCENTRATION);
}

void CdbLeftPaneView::OnEnChange_stimulus2() 
{
	OnEnChange_Item(IDC_CHECKSTIMULUS2);
}

void CdbLeftPaneView::OnEnChange_concentration2() 
{
	OnEnChange_Item(IDC_CHECKCONCENTRATION2);
}

void CdbLeftPaneView::OnEnChange_operator() 
{
	OnEnChange_Item(IDC_CHECKOPERATOR);
}

void CdbLeftPaneView::OnEnChange_sensillumID() 
{
	OnEnChange_Item(IDC_CHECKSENSILLUMID);
}

void CdbLeftPaneView::OnEnChange_insectID() 
{
	OnEnChange_Item(IDC_CHECKINSECTID);
}

void CdbLeftPaneView::OnEnChange_repeat()
{
	OnEnChange_Item(IDC_CHECKREPEAT);
}

void CdbLeftPaneView::OnEnChange_repeat2()
{
	OnEnChange_Item(IDC_CHECKREPEAT2);
}

void CdbLeftPaneView::OnEnChange_flag()
{
	OnEnChange_Item(IDC_CHECKFLAG);
}

void CdbLeftPaneView::OnEnChange_date()
{
	if (m_bAddMode) 
		return;

	if (m_pSet->m_bFilterON && m_pSet->m_desc[CH_ACQDATE_DAY].bFilter1)
	{
		OnFilter_date();
		return;
	}

	if (m_ctldate.GetCount() >1) 
	{
		CString csColName = m_pSet->m_desc[CH_ACQDATE_DAY].csColName;	
		int nsel = m_ctldate.GetCurSel();
		CString csDate;
		m_ctldate.GetLBText(nsel, csDate);
		COleDateTime oTime;
		oTime.ParseDateTime(csDate);
		csDate = oTime.Format(_T("%m/%d/%y"));
		CString str;
		str.Format(_T("%s = #%s#"), (LPCTSTR) csColName, (LPCTSTR) csDate);
		BOOL flag = m_pSet->FindFirst(str);
		ASSERT (flag != 0);
	}
	int irecpos = m_pSet->GetAbsolutePosition();
	GetDocument()->SetIndexCurrentFile(irecpos);
	GetDocument()->UpdateAllViews(this, HINT_DOCMOVERECORD, NULL);
}

// ---------------------------------------------------------------------------------
// update filter for item
// IDC = ID descriptor for check box

void CdbLeftPaneView::OnFilter_Item(int IDC)
{
	// get pointers to parameters useful to change/edit/filter item
	DB_ITEMDESC* pdesc = GetItemDescriptors(IDC);
	if (!m_pSet->IsOpen() || pdesc == NULL) 	
		return;	
	int recordID = m_pSet->m_ID;
	CString cscombo;
	if (pdesc->pComboBox != NULL)
	{
		int i = pdesc->pComboBox->GetCurSel();
		pdesc->pComboBox->GetLBText(i, cscombo);
	}
	
	// initial state: filter ON ?
	pdesc->bFilter1 = ((CButton*) GetDlgItem(IDC))->GetCheck();
	if (pdesc->bFilter1)
	{
		if (pdesc->pComboBox != NULL)
			pdesc->lfilterParam1 = pdesc->pComboBox->GetItemData(pdesc->pComboBox->GetCurSel());
	}

	// initial state: filter OFF ? get data from combobox
	else if (pdesc->pComboBox != NULL)
	{
		pdesc->lfilterParam1 = pdesc->pComboBox->GetItemData(0); // only one is selected
	}

	// change filter and query the database
	m_pSet->SetFilterSingleItem(pdesc);
	m_pSet->BuildFilters();
	m_pSet->RefreshQuery();
	m_pSet->BuildAndSortIDArrays();

	GetDocument()->UpdateAllViews(NULL, HINT_REQUERY, NULL);

	// final state: filter ON ?
	if (pdesc->bFilter1)
		OnMove(ID_RECORD_FIRST);
	// final state: filter OFF ?
	else
	{
		GotoRecordID(recordID);
		if (pdesc->pComboBox != NULL)
		{
			int i = pdesc->pComboBox->FindStringExact(0, cscombo);
			if (i != CB_ERR)
				pdesc->pComboBox->SetCurSel(i);
		}
	}
	if (m_pSet->IsEOF())
	{
		m_pSet->SetFieldNull(pdesc->pdataItem, FALSE);
		*pdesc->pdataItem= pdesc->lfilterParam1;
	}
}

void CdbLeftPaneView::OnFilter_expt() 
{
	OnFilter_Item(IDC_CHECKEXPT);
}

void CdbLeftPaneView::OnFilter_insectName() 
{
	OnFilter_Item(IDC_CHECKINSECT);
}

void CdbLeftPaneView::OnFilter_sensillum() 
{
	OnFilter_Item(IDC_CHECKSENSILLUM);
}

void CdbLeftPaneView::OnFilter_location() 
{
	OnFilter_Item(IDC_CHECKLOCATION);
}

void CdbLeftPaneView::OnFilter_stimulus() 
{
	OnFilter_Item(IDC_CHECKSTIMULUS);
}

void CdbLeftPaneView::OnFilter_sex() 
{
	OnFilter_Item(IDC_CHECKSEX);
}

void CdbLeftPaneView::OnFilter_strain() 
{
	OnFilter_Item(IDC_CHECKSTRAIN);
}

void CdbLeftPaneView::OnFilter_stimulus2() 
{
	OnFilter_Item(IDC_CHECKSTIMULUS2);
}

void CdbLeftPaneView::OnFilter_concentration() 
{
	OnFilter_Item(IDC_CHECKCONCENTRATION);
}

void CdbLeftPaneView::OnFilter_concentration2() 
{
	OnFilter_Item(IDC_CHECKCONCENTRATION2);
}

void CdbLeftPaneView::OnFilter_operator() 
{
	OnFilter_Item(IDC_CHECKOPERATOR);
}

void CdbLeftPaneView::OnFilter_sensillumID() 
{
	OnFilter_Item(IDC_CHECKSENSILLUMID);
}

void CdbLeftPaneView::OnFilter_insectID()
{
	OnFilter_Item(IDC_CHECKINSECTID);
}

void CdbLeftPaneView::OnFilter_flag()
{
	OnFilter_Item(IDC_CHECKFLAG);
}

void CdbLeftPaneView::OnFilter_repeat()
{
	OnFilter_Item(IDC_CHECKREPEAT);
}

void CdbLeftPaneView::OnFilter_repeat2()
{
	OnFilter_Item(IDC_CHECKREPEAT2);
}

void CdbLeftPaneView::OnFilter_date()
{
	// set filter condition
	CString		csColName;
	csColName = m_pSet->m_desc[CH_ACQDATE_DAY].csColName;	
	ASSERT(m_pSet->IsOpen()); 	
	int recordID = m_pSet->m_ID;
	
	// filter is ON ?
	BOOL flagON = ((CButton*) GetDlgItem(IDC_CHECKDATE))->GetCheck();
	m_pSet->m_desc[CH_ACQDATE_DAY].bFilter1 = flagON;
	CString csDate;
	int nsel = 0;
	if (m_pSet->m_desc[CH_ACQDATE_DAY].bFilter1)
		nsel = m_ctldate.GetCurSel();
	m_ctldate.GetLBText(nsel, csDate);
	COleDateTime tday;
	ASSERT(tday.ParseDateTime(csDate, VAR_DATEVALUEONLY));
	m_pSet->m_desc[CH_ACQDATE_DAY].otfilterParam1 = tday;

	m_pSet->BuildFilters();
	m_pSet->RefreshQuery();
	m_pSet->BuildAndSortIDArrays();
	PopulateControls();

	if (m_pSet->m_desc[CH_ACQDATE_DAY].bFilter1)
		OnMove(ID_RECORD_FIRST);
	else
	{
		GotoRecordID(recordID);
		CString cs = m_pSet->m_acqdate_day.Format(VAR_DATEVALUEONLY);
		int i = m_ctldate.FindStringExact(0, cs);
		m_ctldate.SetCurSel(i);
	}
}

// ---------------------------------------------------------------------------------
// Edit item / linked lists
// IDC = ID descriptor for check box

void CdbLeftPaneView::OnEditChange_Item_LinkedList(int IDC)
{
	DB_ITEMDESC* pdesc = GetItemDescriptors(IDC);
	if (m_bAddMode) 
		return;
	if (pdesc->bFilter1)
	{
		OnFilter_Item(IDC);
	}
	else if (pdesc->pComboBox != NULL)
	{
		CdbEditFieldDlg dlg;
		dlg.m_pMainTable	= m_pSet;			// address main table
		dlg.m_csColName		= pdesc->csColName;		// name of the column
		dlg.m_pIndexTable	= pdesc->plinkedSet;		// address secondary table
		dlg.m_pliIDArray	= NULL;				// not a primary field
		dlg.m_pdbDoc		= GetDocument();
		if (dlg.DoModal() == IDOK)
			PopulateComboFromLinkedTable(pdesc->pComboBox, *pdesc->plinkedSet, pdesc->csColName);
	}
}

void CdbLeftPaneView::OnEditChange_expt() 
{
	OnEditChange_Item_LinkedList(IDC_CHECKEXPT);
}

void CdbLeftPaneView::OnEditChange_insectname() 
{
	OnEditChange_Item_LinkedList(IDC_CHECKINSECT);
}

void CdbLeftPaneView::OnEditChange_sensillum() 
{
	OnEditChange_Item_LinkedList(IDC_CHECKSENSILLUM);
}

void CdbLeftPaneView::OnEditChange_strain() 
{
	OnEditChange_Item_LinkedList(IDC_CHECKSTRAIN);
}

void CdbLeftPaneView::OnEditChange_sex() 
{
	OnEditChange_Item_LinkedList(IDC_CHECKSEX);
}

void CdbLeftPaneView::OnEditChange_concentration() 
{
	OnEditChange_Item_LinkedList(IDC_CHECKCONCENTRATION);
}

void CdbLeftPaneView::OnEditChange_concentration2() 
{
	OnEditChange_Item_LinkedList(IDC_CHECKCONCENTRATION2);
}

void CdbLeftPaneView::OnEditChange_operator() 
{
	OnEditChange_Item_LinkedList(IDC_CHECKOPERATOR);
}

void CdbLeftPaneView::OnEditChange_stimulus() 
{
	OnEditChange_Item_LinkedList(IDC_CHECKSTIMULUS);
}

void CdbLeftPaneView::OnEditChange_stimulus2() 
{
	OnEditChange_Item_LinkedList(IDC_CHECKSTIMULUS2);
}

void CdbLeftPaneView::OnEditChange_location() 
{
	OnEditChange_Item_LinkedList(IDC_CHECKLOCATION);
}

void CdbLeftPaneView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) 
{
	CMainFrame* pmF = (CMainFrame*) AfxGetMainWnd();
	// activate view
	if (bActivate)
	{
		if (pmF->m_pSecondToolBar != NULL) 
			pmF->ShowPane(pmF->m_pSecondToolBar, FALSE, FALSE, TRUE);
	}
	CDaoRecordView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

void CdbLeftPaneView::GotoRecordID(long recordID)
{
	if (!GetDocument()->m_pDB->MoveToID(recordID))
		MessageBox(_T("record not found"));
	m_varBookmarkCurrent = m_pSet->GetBookmark();

	// update scroll list
	GetDocument()->UpdateAllViews(this, HINT_DOCMOVERECORD, NULL);
}

void CdbLeftPaneView::OnRecordAdd() 
{
	if (m_bAddMode)
	{
		m_pSet->CancelUpdate();
		m_bAddMode = FALSE;
	}
	if (m_pSet->m_bFilterON)
		m_pSet->ClearFilters();

	m_pSet->RefreshQuery();

	CString csmore;
	csmore.Empty();
	long insectID = 0;

	if (m_pSet->IsEOF() == 0)
	{
		csmore = m_pSet->m_more;
		insectID = m_pSet->m_insectID;
	}
	m_pSet->AddNew();
	
	m_pSet->SetFieldNull(&(m_pSet->m_acq_date), FALSE);
	m_pSet->m_acq_date = COleDateTime::GetCurrentTime();
	m_pSet->SetFieldNull(&(m_pSet->m_more), FALSE);
	m_pSet->m_more = csmore;
	m_pSet->SetFieldNull(&(m_pSet->m_insectID), FALSE);
	m_pSet->m_insectID = insectID;

	m_bAddMode = TRUE;
	UpdateData(FALSE);
	GetDocument()->UpdateAllViews(NULL, HINT_REQUERY, NULL);
	GetDocument()->m_pDB->GetFilenamesFromCurrentRecord();
}

/////////////////////////////////////////////////////////////////////////////

void CdbLeftPaneView::OnEditChange_insectID() 
{
	if (m_bAddMode) return;
	if (m_pSet->m_desc[CH_INSECTID].bFilter1) //m_binsectID)
		OnFilter_insectID();
	else
		DlgUpdateCombo(IDC_CHECKINSECTID);
}

void CdbLeftPaneView::OnEditChange_sensillumID()
{
	if (m_bAddMode) return;
	if (m_pSet->m_desc[CH_SENSILLUMID].bFilter1) //m_bsensillumID)
		OnFilter_sensillumID();
	else
		DlgUpdateCombo(IDC_CHECKSENSILLUMID);
}

void CdbLeftPaneView::OnEditChange_repeat()
{
	if (m_bAddMode) return;
	if (m_pSet->m_desc[CH_REPEAT].bFilter1) // m_brepeat)
		OnFilter_repeat();
	else
		DlgUpdateCombo(IDC_CHECKREPEAT);
}

void CdbLeftPaneView::OnEditChange_repeat2()
{
	if (m_bAddMode) return;
	if (m_pSet->m_desc[CH_REPEAT2].bFilter1) //m_brepeat2)
		OnFilter_repeat2();
	else
		DlgUpdateCombo(IDC_CHECKREPEAT2);
}

void CdbLeftPaneView::OnEditChange_flag()
{
	if (m_bAddMode) return;
	if (m_pSet->m_desc[CH_FLAG].bFilter1) //m_bflag)
		OnFilter_flag();
	else
		DlgUpdateCombo(IDC_CHECKFLAG);
}

void CdbLeftPaneView::OnEditChange_date()
{
	if (m_bAddMode) return;
	if (m_pSet->m_desc[CH_FLAG].bFilter1)
		OnFilter_date();
	else
		PopulateComboFromTablewithDate (&m_ctldate,	CH_ACQDATE_DAY);
}
// if new item, update the corresponding array in document...
// update

void CdbLeftPaneView::DlgUpdateCombo(int IDC)
{
	DB_ITEMDESC* pdesc = GetItemDescriptors(IDC);
	if (pdesc->pComboBox == NULL)
		return;

	CdbEditFieldDlg dlg;
	dlg.m_pMainTable	= m_pSet;					// address main table
	dlg.m_csColName		= pdesc->csColName;		// name of the column
	dlg.m_pliIDArray	= &pdesc->liArray;			// address of table of ids
	dlg.m_pIndexTable   = NULL;
	dlg.m_pdbDoc		= GetDocument();
	if (dlg.DoModal() == IDOK)
	{
		// update array
		CdbMainTable* pSet = &(GetDocument()->m_pDB->m_tableSet);
		pSet->BuildAndSortIDArrays();
		PopulateComboFromTable (pdesc->pComboBox, pdesc->icol);

		// find current selection and set combo to this position
		unsigned int iID = *pdesc->pdataItem;
		int icursel = 0;
		for (int i = pdesc->liArray.GetUpperBound(); i>= 0; i--)
		{
			if (iID == pdesc->liArray.GetAt(i))
			{
				icursel = i;
				break;
			}
		}
		pdesc->pComboBox->SetCurSel(icursel);
	}
}

///////////////////////////////////////////////////////////////////////////////
void CdbLeftPaneView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	//ATLTRACE2(_T("CdbLeftPaneView: OnUpdate\n"));
	m_bUpdateFromDoc = TRUE;
	switch (LOWORD(lHint))
	{
	case HINT_REPLACEVIEW:
		break;
	case HINT_FILTERREMOVED:
		m_pSet->ClearFilters();
	case HINT_REQUERY:
	case HINT_DOCHASCHANGED:		// file has changed?
		PopulateControls();
	case HINT_DOCMOVERECORD:
	default:
		UpdateControls();
		break;
	}
	m_bUpdateFromDoc = FALSE;
}

void CdbLeftPaneView::OnSize(UINT nType, int cx, int cy) 
{
	// adapt size of resizeable controls
	if (m_binit)
	{
		switch (nType)
		{
		case SIZE_MAXIMIZED:
		case SIZE_RESTORED:
			if (cx <= 0 || cy <= 0)
				break;
			// change size of windows declared to this m_stretch
			m_stretch.ResizeControls(nType, cx, cy);
			break;
		default:
			break;
		}
	}
	// do other resizing
	CDaoRecordView::OnSize(nType, cx, cy);
}

void CdbLeftPaneView::OnBnClickedChecksort()
{
	BOOL bSort = ((CButton*) GetDlgItem(IDC_CHECKSORT))->GetCheck();
	int show = SW_HIDE;
	if (bSort)
		show = SW_SHOW;
	CString cs;
	GetDlgItem(IDC_SORT)->ShowWindow(show);
	if (bSort)
	{
		GetDlgItem(IDC_SORT)->GetWindowText(cs);
		if (cs.IsEmpty())
		{
			int isel = ((CComboBox*) GetDlgItem(IDC_SORT))->GetCurSel();
			if (isel < 0)
				isel = 0;
			((CComboBox*) GetDlgItem(IDC_SORT))->SetCurSel(isel);
			GetDlgItem(IDC_SORT)->GetWindowText(cs);		
		}
	}

	// update other views if we came here from within lefpaneview
	if (!m_bUpdateFromDoc )
	{
		m_pSet->m_strSort = cs;
		m_pSet->Requery();
		GetDocument()->UpdateAllViews(NULL, HINT_REQUERY, NULL);
	}
}

void CdbLeftPaneView::OnCbnSelchangeSort()
{
	CString cs;
	int isel = ((CComboBox*) GetDlgItem(IDC_SORT))->GetCurSel();
	((CComboBox*) GetDlgItem(IDC_SORT))->SetCurSel(isel);
	GetDlgItem(IDC_SORT)->GetWindowText(cs);
	// update other views if we came here from within lefpaneview
	if (!m_bUpdateFromDoc )
	{
		m_pSet->m_strSort = cs;
		m_pSet->Requery();
		GetDocument()->UpdateAllViews(NULL, HINT_REQUERY, NULL);
	}
}

#include "GridCtrl\GridCtrl.h"
#include "PivotDlg.h"

void CdbLeftPaneView::OnBnClickedButtonrepeat3()
{
	CPivotDlg dlg;
	if (IDOK == dlg.DoModal())
		MessageBox(_T("done"), _T("done"), MB_OK);
}
