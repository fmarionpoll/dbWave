// dbEditRecordDlg.cpp : implementation file
//

#include "stdafx.h"
#include <afxconv.h>           // For LPTSTR -> LPSTR macros

#include "dbWave.h"
#include "dbMainTable.h"
#include "dbWaveDoc.h"
#include "findfilesdlg.h"
#include "dbEditFieldDlg.h"
#include "afxdialogex.h"

#include "dbeditrecorddlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CdbEditRecordDlg dialog

IMPLEMENT_DYNAMIC(CdbEditRecordDlg, CDialog)
CdbEditRecordDlg::CdbEditRecordDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CdbEditRecordDlg::IDD, pParent)
	, m_cs_more(_T(""))
	, m_csnameDat(_T(""))
	, m_csnameSpk(_T(""))
{
	//m_pDaoView = NULL;
	m_bshowIDC_NEXT = TRUE;
	m_bshowIDC_PREVIOUS = TRUE;
}

CdbEditRecordDlg::~CdbEditRecordDlg()
{
	m_pdbDoc = NULL;
}

void CdbEditRecordDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_EXPT2,			m_ctlexpt);
	DDX_Control(pDX, IDC_COMBO_INSECTID,		m_ctlinsectID);
	DDX_Control(pDX, IDC_COMBO_SENSILLUMID,		m_ctlSensillumID);
	DDX_Control(pDX, IDC_COMBO_REPEATT,			m_ctlrepeat);
	DDX_Control(pDX, IDC_COMBO_REPEATT2,		m_ctlrepeat2);
	DDX_Control(pDX, IDC_COMBO_STIMULUS,		m_ctlstim);
	DDX_Control(pDX, IDC_COMBO_CONCENTRATION,	m_ctlconc);
	DDX_Control(pDX, IDC_COMBO_STIMULUS2,		m_ctlstim2);
	DDX_Control(pDX, IDC_COMBO_CONCENTRATION2,	m_ctlconc2);
	DDX_Control(pDX, IDC_COMBO_INSECTNAME,		m_ctlinsect);
	DDX_Control(pDX, IDC_COMBO_SENSILLUMNAME,	m_ctlsensillum);
	DDX_Control(pDX, IDC_COMBO_LOCATION,		m_ctllocation);
	DDX_Control(pDX, IDC_COMBO_OPERATOR,		m_ctlOperator);
	DDX_Control(pDX, IDC_COMBO_STRAIN,			m_ctlstrain);
	DDX_Control(pDX, IDC_COMBO_SEX,				m_ctlsex);
	DDX_Control(pDX, IDC_COMBO_PATHDAT,			m_ctlpathdat);
	DDX_Control(pDX, IDC_COMBO_PATHSPK,			m_ctlpathspk);
	DDX_Control(pDX, IDC_COMBO_FLAG,			m_ctlflag);

	DDX_Text(pDX, IDC_EDIT_COMMENT,				m_cs_more);
	DDX_Text(pDX, IDC_EDIT_NAMEDAT,				m_csnameDat);
	DDX_Text(pDX, IDC_EDIT_NAMESPK,				m_csnameSpk);	
}

BEGIN_MESSAGE_MAP(CdbEditRecordDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTONINSECTID,		&CdbEditRecordDlg::OnBnClickedButtoninsectid)
	ON_BN_CLICKED(IDC_BUTTONSENSILLUMID,	&CdbEditRecordDlg::OnBnClickedButtonsensillumid)
	ON_BN_CLICKED(IDC_BUTTONSTIMULUS,		&CdbEditRecordDlg::OnBnClickedButtonstimulus)
	ON_BN_CLICKED(IDC_BUTTONCONCENTRATION,	&CdbEditRecordDlg::OnBnClickedButtonconcentration)
	ON_BN_CLICKED(IDC_BUTTONSTIMULUS2,		&CdbEditRecordDlg::OnBnClickedButtonstimulus2)
	ON_BN_CLICKED(IDC_BUTTONCONCENTRATION2,	&CdbEditRecordDlg::OnBnClickedButtonconcentration2)
	ON_BN_CLICKED(IDC_BUTTONINSECTNAME,		&CdbEditRecordDlg::OnBnClickedButtoninsectname)
	ON_BN_CLICKED(IDC_BUTTONSTRAIN,			&CdbEditRecordDlg::OnBnClickedButtonstrain)
	ON_BN_CLICKED(IDC_BUTTONSEX,			&CdbEditRecordDlg::OnBnClickedButtonsex)
	ON_BN_CLICKED(IDC_BUTTONSENSILLUM,		&CdbEditRecordDlg::OnBnClickedButtonsensillum)
	ON_BN_CLICKED(IDC_BUTTONLOCATION,		&CdbEditRecordDlg::OnBnClickedButtonlocation)
	ON_BN_CLICKED(IDC_BUTTONOPERATOR,		&CdbEditRecordDlg::OnBnClickedButtonoperator)
	ON_BN_CLICKED(IDC_BUTTON5,				&CdbEditRecordDlg::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON1,				&CdbEditRecordDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_SYNCHROSINGLE,		&CdbEditRecordDlg::OnBnClickedSynchrosingle)
	ON_BN_CLICKED(IDC_SYNCHROALL,			&CdbEditRecordDlg::OnBnClickedSynchroall)
	ON_BN_CLICKED(IDC_PREVIOUS,				&CdbEditRecordDlg::OnBnClickedPrevious)
	ON_BN_CLICKED(IDC_NEXT,					&CdbEditRecordDlg::OnBnClickedNext)
	ON_BN_CLICKED(IDC_BUTTONREPEAT,			&CdbEditRecordDlg::OnBnClickedButtonrepeat)
	ON_BN_CLICKED(IDC_BUTTONREPEAT2,		&CdbEditRecordDlg::OnBnClickedButtonrepeat2)
	ON_BN_CLICKED(IDC_BUTTONFLAG,			&CdbEditRecordDlg::OnBnClickedButtonflag)
	ON_BN_CLICKED(IDC_BUTTONEXPT2,			&CdbEditRecordDlg::OnBnClickedButtonexpt2)
END_MESSAGE_MAP()

// CdbEditRecordDlg message handlers

BOOL CdbEditRecordDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_pSet = &m_pdbDoc->m_pDB->m_tableSet;
	PopulateControls();
	// TODO? disable controls corresponding to a filtered field
	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CdbEditRecordDlg::PopulateControls()
{
	CdbWdatabase* pDB = m_pdbDoc->m_pDB;
	// fill combo boxes associated with a secondary table
	PopulateCombo_WithText(pDB->m_stimSet,		m_ctlstim,		m_pSet->m_stim_ID);
	PopulateCombo_WithText(pDB->m_concSet,		m_ctlconc,		m_pSet->m_conc_ID);
	PopulateCombo_WithText(pDB->m_stimSet,		m_ctlstim2,		m_pSet->m_stim2_ID);
	PopulateCombo_WithText(pDB->m_concSet,		m_ctlconc2,		m_pSet->m_conc2_ID);
	PopulateCombo_WithText(pDB->m_insectSet,	m_ctlinsect,	m_pSet->m_insect_ID);
	PopulateCombo_WithText(pDB->m_strainSet,	m_ctlstrain,	m_pSet->m_strain_ID);
	PopulateCombo_WithText(pDB->m_sexSet,		m_ctlsex,		m_pSet->m_sex_ID);
	PopulateCombo_WithText(pDB->m_sensillumSet, m_ctlsensillum, m_pSet->m_sensillum_ID);
	PopulateCombo_WithText(pDB->m_locationSet,	m_ctllocation,	m_pSet->m_location_ID);
	PopulateCombo_WithText(pDB->m_operatorSet,	m_ctlOperator,	m_pSet->m_operator_ID);
	PopulateCombo_WithText(pDB->m_pathSet,		m_ctlpathdat,	m_pSet->m_path_ID);
	PopulateCombo_WithText(pDB->m_pathSet,		m_ctlpathspk,	m_pSet->m_path2_ID);
	PopulateCombo_WithText(pDB->m_exptSet,		m_ctlexpt,		m_pSet->m_expt_ID);
	// ID combos
	PopulateCombo_WithNumbers(m_ctlinsectID,	&m_pSet->m_desc[CH_IDINSECT].liArray,	m_pSet->m_IDinsect);
	PopulateCombo_WithNumbers(m_ctlSensillumID,	&m_pSet->m_desc[CH_IDSENSILLUM].liArray,m_pSet->m_IDsensillum);
	PopulateCombo_WithNumbers(m_ctlrepeat,		&m_pSet-> m_desc[CH_REPEAT].liArray,	m_pSet->m_repeat);
	PopulateCombo_WithNumbers(m_ctlrepeat2,		&m_pSet->m_desc[CH_REPEAT2].liArray,	m_pSet->m_repeat2);
	PopulateCombo_WithNumbers(m_ctlflag,		&m_pSet->m_desc[CH_FLAG].liArray,		m_pSet->m_flag);

	// fixed parameters
	m_cs_more = m_pSet->m_more;
	m_csnameDat = m_pSet->m_Filedat;
	m_csnameSpk = m_pSet->m_Filespk;

	// update IDC_NEXT IDC_PREVIOUS
	if (m_bshowIDC_NEXT != GetDlgItem(IDC_NEXT)->IsWindowEnabled())
		GetDlgItem(IDC_NEXT)->EnableWindow(m_bshowIDC_NEXT);
	if (m_bshowIDC_PREVIOUS != GetDlgItem(IDC_PREVIOUS)->IsWindowEnabled())
		GetDlgItem(IDC_PREVIOUS)->EnableWindow(m_bshowIDC_PREVIOUS);
}

void CdbEditRecordDlg::PopulateCombo_WithNumbers(CComboBox& combo, CArray<long, long>* pIDarray, long& lvar)
{
	combo.ResetContent();
	CString cs;

	int arraySize = pIDarray->GetSize();
	int isel = 0;
	int i=0;

	for (i=0; i< arraySize; i++)
	{
		unsigned int iID = pIDarray->GetAt(i);
		cs.Format(_T("%i"), iID);
		combo.AddString(cs);
		combo.SetItemData(i, iID);
		if (iID == lvar)
			isel = i;
	}
	combo.SetCurSel(isel);
}

void CdbEditRecordDlg::PopulateCombo_WithText(CDaoRecordset& linkedtableSet, CComboBox& combo, int iID)
{
	combo.ResetContent();
	// fill combo box
	if (linkedtableSet.IsOpen() && !linkedtableSet.IsBOF()) 
	{
		COleVariant varValue0, varValue1;
		linkedtableSet.MoveFirst();
		CString cs;
		while(!linkedtableSet.IsEOF()) 
		{
			linkedtableSet.GetFieldValue(0, varValue0);
			linkedtableSet.GetFieldValue(1, varValue1);
			long ID = varValue1.lVal;
			cs =varValue0.bstrVal;
			if (!cs.IsEmpty())
			{
				int i = combo.AddString(cs);
				combo.SetItemData(i, ID);
			}
			linkedtableSet.MoveNext();
		}
	}

	// search item which has value iID
	int isel = -1;
	for (int i=0; i < combo.GetCount(); i++)
	{
		if (iID == combo.GetItemData(i))
		{
			isel = i;
			break;
		}
	}
	combo.SetCurSel(isel);
}

void CdbEditRecordDlg::UpdateDatabaseFromDialog()
{
	UpdateData(TRUE); // transfer data from dlg to variables
	
	// update combo boxes associated with a secondary table
	m_pSet->Edit();

	CdbWdatabase* pDB = m_pdbDoc->m_pDB; 
	UpdateSetFromCombo(pDB->m_stimSet,		m_ctlstim,		m_pSet->m_stim_ID);
	UpdateSetFromCombo(pDB->m_concSet,		m_ctlconc,		m_pSet->m_conc_ID);
	UpdateSetFromCombo(pDB->m_stimSet,		m_ctlstim2,		m_pSet->m_stim2_ID);
	UpdateSetFromCombo(pDB->m_concSet,		m_ctlconc2,		m_pSet->m_conc2_ID);
	UpdateSetFromCombo(pDB->m_insectSet,	m_ctlinsect,	m_pSet->m_insect_ID);
	UpdateSetFromCombo(pDB->m_strainSet,	m_ctlstrain,	m_pSet->m_strain_ID);
	UpdateSetFromCombo(pDB->m_sexSet,		m_ctlsex,		m_pSet->m_sex_ID);
	UpdateSetFromCombo(pDB->m_sensillumSet, m_ctlsensillum, m_pSet->m_sensillum_ID);
	UpdateSetFromCombo(pDB->m_locationSet,	m_ctllocation,	m_pSet->m_location_ID);
	UpdateSetFromCombo(pDB->m_operatorSet,	m_ctlOperator,	m_pSet->m_operator_ID);
	UpdateSetFromCombo(pDB->m_exptSet,		m_ctlexpt,		m_pSet->m_expt_ID);

	//// save fixed parameters

	CString cs;
	m_ctlinsectID.GetWindowText(cs);
	m_pSet->m_IDinsect = _ttoi(cs);
	m_ctlSensillumID.GetWindowText(cs);
	m_pSet->m_IDsensillum = _ttoi(cs);
	m_ctlrepeat.GetWindowText(cs);
	m_pSet->m_repeat	= _ttoi(cs);
	m_ctlrepeat2.GetWindowText(cs);
	m_pSet->m_repeat2	= _ttoi(cs);
	m_ctlflag.GetWindowText(cs);
	m_pSet->m_flag		= _ttoi(cs);
	m_pSet->m_more		= m_cs_more;	
	m_pSet->m_Filedat	= m_csnameDat;
	m_pSet->m_Filespk	= m_csnameSpk;
	m_pSet->Update();
}

void CdbEditRecordDlg::OnOK()
{
	UpdateDatabaseFromDialog();
	CDialog::OnOK();
}

void CdbEditRecordDlg::UpdateSetFromCombo(CDaoRecordset& linkedtableSet, CComboBox& combo, long& iIDset)
{
	// search if content of edit window is listed in the combo
	CString csCombo;
	combo.GetWindowText(csCombo);
	int nIndex = combo.FindStringExact(0, csCombo);
	if (nIndex == CB_ERR )
	{
		// if new value, add a record in the linked table
		if (!csCombo.IsEmpty())
		{
			linkedtableSet.AddNew();
			linkedtableSet.SetFieldValue(0, COleVariant(csCombo, VT_BSTRT));
			try { linkedtableSet.Update();}
			catch(CDaoException* e) {DisplayDaoException(e, 24); e->Delete();}

			// get value and set the ID number in the main table
			linkedtableSet.MoveLast();
			COleVariant varValue0, varValue1;
			linkedtableSet.GetFieldValue(0, varValue0);
			linkedtableSet.GetFieldValue(1, varValue1);
			CString cs = varValue0.bstrVal;
			ASSERT(csCombo == cs);
			iIDset = varValue1.lVal;				
		}
		// if empty string, set field to null in the main table
		else
			m_pSet->SetFieldNull(&iIDset, TRUE);
	}
	else
	{
		int iID = combo.GetItemData(nIndex);
		iIDset = iID;
	}
}

// edit and update combo boxes
// if new item, update the corresponding array in document...
// update

void CdbEditRecordDlg::OnBnClickedButtoninsectid()
{
	EditChangeItem_MainField(IDC_COMBO_INSECTID);
}

void CdbEditRecordDlg::OnBnClickedButtonsensillumid()
{
	EditChangeItem_MainField(IDC_COMBO_SENSILLUMID);
}

void CdbEditRecordDlg::OnBnClickedButtonflag()
{
	EditChangeItem_MainField(IDC_COMBO_FLAG);
}

void CdbEditRecordDlg::OnBnClickedButtonrepeat()
{
	EditChangeItem_MainField(IDC_COMBO_REPEATT);
}

void CdbEditRecordDlg::OnBnClickedButtonrepeat2()
{
	EditChangeItem_MainField(IDC_COMBO_REPEATT2);
}

// Edit item / linked lists
// IDC = ID descriptor for combo box

void CdbEditRecordDlg::EditChangeItem_IndirectField(int IDC)
{
	DB_ITEMDESC* pdesc = GetItemDescriptors(IDC);
	if (pdesc->pComboBox == NULL)
		return;

	CdbEditFieldDlg dlg;
	dlg.m_pMainTable	= m_pSet;				// address main table
	dlg.m_csColName		= pdesc->csColName;		// name of the column
	dlg.m_pIndexTable	= pdesc->plinkedSet;	// address secondary table
	dlg.m_pliIDArray	= NULL;					// not a primary field
	dlg.m_pdbDoc		= m_pdbDoc;
	if (dlg.DoModal() == IDOK)
		PopulateCombo_WithText(*pdesc->plinkedSet, *pdesc->pComboBox, *pdesc->pdataItem);
}

void CdbEditRecordDlg::EditChangeItem_MainField(int IDC)
{
	DB_ITEMDESC* pdesc = GetItemDescriptors(IDC);
	if (pdesc->pComboBox == NULL)
		return;

	CdbEditFieldDlg dlg;
	dlg.m_pMainTable	= m_pSet;				// address main table
	dlg.m_csColName		= pdesc->csColName;		// name of the column
	dlg.m_pliIDArray	= &pdesc->liArray;		// address of table of ids
	dlg.m_pIndexTable   = NULL;
	dlg.m_pdbDoc		= m_pdbDoc;
	if (dlg.DoModal() == IDOK)
	{
		// update array
		m_pSet->BuildAndSortIDArrays();
		PopulateCombo_WithNumbers(*pdesc->pComboBox, &pdesc->liArray, *pdesc->pdataItem);

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

// return pointers to parameters useful to change/edit item
// IDC = ID descriptor for check box

DB_ITEMDESC* CdbEditRecordDlg::GetItemDescriptors(int IDC)
{
	DB_ITEMDESC* pdesc = NULL;
	CdbWaveDoc* pdbDoc = m_pdbDoc;
	CdbWdatabase* pDB = pdbDoc->m_pDB;
	int ich= 0;
	switch (IDC)
	{
	case IDC_COMBO_EXPT2:
		ich=CH_EXPT_ID;
		m_pSet->m_desc[ich].pComboBox	= &m_ctlexpt;
		break;
	case IDC_COMBO_INSECTNAME:
		ich=CH_INSECT_ID;
		m_pSet->m_desc[ich].pComboBox	= &m_ctlinsect;
		break;
	case IDC_COMBO_LOCATION:
		ich= CH_LOCATION_ID;
		m_pSet->m_desc[ich].pComboBox	= &m_ctllocation;
		break;
	case IDC_COMBO_SENSILLUMNAME:
		ich = CH_SENSILLUM_ID;
		m_pSet->m_desc[ich].pComboBox	= &m_ctlsensillum;
		break;
	case IDC_COMBO_STIMULUS:
		ich = CH_STIM_ID;
		m_pSet->m_desc[ich].pComboBox	= &m_ctlstim;
		break;
	case IDC_COMBO_CONCENTRATION:
		ich= CH_CONC_ID;
		m_pSet->m_desc[ich].pComboBox	= &m_ctlconc;
		break;
	case IDC_COMBO_OPERATOR:
		ich = CH_OPERATOR_ID;
		m_pSet->m_desc[ich].pComboBox	= &m_ctlOperator;
		break;	
	case IDC_COMBO_STRAIN:
		ich = CH_STRAIN_ID;
		m_pSet->m_desc[ich].pComboBox	= &m_ctlstrain;
		break;
	case IDC_COMBO_SEX:
		ich = CH_SEX_ID;
		m_pSet->m_desc[ich].pComboBox	= &m_ctlsex;
		break;
	case IDC_COMBO_STIMULUS2:
		ich = CH_STIM2_ID;
		m_pSet->m_desc[ich].pComboBox	= &m_ctlstim2;
		break;
	case IDC_COMBO_CONCENTRATION2:
		ich = CH_CONC2_ID;
		m_pSet->m_desc[ich].pComboBox	= &m_ctlconc2;
		break;
	case IDC_COMBO_PATHDAT:
		ich = CH_PATH_ID;
		m_pSet->m_desc[ich].pComboBox	= &m_ctlpathdat;
		break;
	case IDC_COMBO_PATHSPK:
		ich = CH_PATH2_ID;
		m_pSet->m_desc[ich].pComboBox	= &m_ctlpathspk;
		break;

	case IDC_COMBO_INSECTID:
		ich = CH_IDINSECT;
		m_pSet->m_desc[ich].pComboBox	= &m_ctlinsectID;
		break;
	case IDC_COMBO_SENSILLUMID:
		ich = CH_IDSENSILLUM;
		m_pSet->m_desc[ich].pComboBox	= &m_ctlSensillumID;
		break;
	case IDC_COMBO_FLAG:
		ich = CH_FLAG;
		m_pSet->m_desc[ich].pComboBox	= &m_ctlflag;
		break;
	case IDC_COMBO_REPEATT:
		ich = CH_REPEAT;
		m_pSet->m_desc[ich].pComboBox	= &m_ctlrepeat;
		break;
	case IDC_COMBO_REPEATT2:
		ich = CH_REPEAT2;
		m_pSet->m_desc[ich].pComboBox	= &m_ctlrepeat2;
		break;
	
	default:
		ich=-1;
		break;
	}
	if (ich>= 0)
		pdesc = pDB->GetRecordItemDescriptor(ich);

	return pdesc;
}

void CdbEditRecordDlg::OnBnClickedButtonstimulus()
{
	EditChangeItem_IndirectField(IDC_COMBO_STIMULUS);
}

void CdbEditRecordDlg::OnBnClickedButtonconcentration()
{
	EditChangeItem_IndirectField(IDC_COMBO_CONCENTRATION);
}

void CdbEditRecordDlg::OnBnClickedButtonstimulus2()
{
	EditChangeItem_IndirectField(IDC_COMBO_STIMULUS2);
}

void CdbEditRecordDlg::OnBnClickedButtonconcentration2()
{
	EditChangeItem_IndirectField(IDC_COMBO_CONCENTRATION2);
}

void CdbEditRecordDlg::OnBnClickedButtoninsectname()
{
	EditChangeItem_IndirectField(IDC_COMBO_INSECTNAME);
}

void CdbEditRecordDlg::OnBnClickedButtonsensillum()
{
	EditChangeItem_IndirectField(IDC_COMBO_SENSILLUMNAME);
}

void CdbEditRecordDlg::OnBnClickedButtonlocation()
{
	EditChangeItem_IndirectField(IDC_COMBO_LOCATION);
}

void CdbEditRecordDlg::OnBnClickedButtonoperator()
{
	EditChangeItem_IndirectField(IDC_COMBO_OPERATOR);
}

void CdbEditRecordDlg::OnBnClickedButton5()
{
	EditChangeItem_IndirectField(IDC_COMBO_PATHDAT);
}

void CdbEditRecordDlg::OnBnClickedButton1()
{
	if (AfxMessageBox(_T("Are spike files in the same directory as dat files?"), MB_YESNO, -1) != IDYES)
		EditChangeItem_IndirectField(IDC_COMBO_PATHSPK);
	else
	{
		m_pdbDoc->DBTransferDatPathToSpkPath();
	}
}

void CdbEditRecordDlg::OnBnClickedButtonstrain()
{
	EditChangeItem_IndirectField(IDC_COMBO_STRAIN);
}

void CdbEditRecordDlg::OnBnClickedButtonsex()
{
	EditChangeItem_IndirectField(IDC_COMBO_SEX);
}

void CdbEditRecordDlg::OnBnClickedButtonexpt2()
{
	EditChangeItem_IndirectField(IDC_COMBO_EXPT2);
}

void CdbEditRecordDlg::OnBnClickedSynchrosingle()
{
	UpdateDatabaseFromDialog();
	m_pdbDoc->SynchronizeSourceInfos(FALSE);
}

void CdbEditRecordDlg::OnBnClickedSynchroall()
{
	UpdateDatabaseFromDialog();
	m_pdbDoc->SynchronizeSourceInfos(TRUE);
}

void CdbEditRecordDlg::OnBnClickedPrevious()
{
	UpdateDatabaseFromDialog();
	m_pdbDoc->DBMovePrev();

	m_pdbDoc->UpdateAllViews(NULL, 0L, NULL);
	PopulateControls();
	UpdateData(FALSE);
}

void CdbEditRecordDlg::OnBnClickedNext()
{
	UpdateDatabaseFromDialog();
	m_pdbDoc->DBMoveNext();

	m_pdbDoc->UpdateAllViews(NULL, 0L, NULL);

	PopulateControls();
	UpdateData(FALSE);
}



