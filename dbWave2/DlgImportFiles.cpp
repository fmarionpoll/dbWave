#include "StdAfx.h"
#include "dbWave.h"
#include "DlgImportFiles.h"
#include "resource.h"

#include "dataheader_Atlab.H"
#include "dbWaveDoc.h"
#include "DlgADExperiment.h"
#include "DlgADInputParms.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNAMIC(DlgImportFiles, CDialog)

DlgImportFiles::DlgImportFiles(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
}

DlgImportFiles::~DlgImportFiles()
= default;

void DlgImportFiles::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_ncurrent);
	DDX_Text(pDX, IDC_EDIT12, m_nfiles);
	DDX_Text(pDX, IDC_EDIT3, m_filefrom);
	DDX_Text(pDX, IDC_EDIT4, m_fileto);
	DDX_Check(pDX, IDC_CHECK1, m_bReadHeader);
}

BEGIN_MESSAGE_MAP(DlgImportFiles, CDialog)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDCANCEL, &DlgImportFiles::OnBnClickedCancel)
	ON_BN_CLICKED(ID_STARTSTOP, &DlgImportFiles::ADC_OnBnClickedStartstop)
END_MESSAGE_MAP()

// CImportFilesDlg message handlers
BOOL DlgImportFiles::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_nfiles = m_pfilenameArray->GetSize(); // get nb of files
	m_ncurrent = 1;
	// outut file extension
	switch (m_option)
	{
	case GERT: // option non implemented
		m_ext = ".spk";
		break;
	case ATFFILE:
	case ASCIISYNTECH: // option non implemented
	default:
		m_ext = ".dat";
		break;
	}

	UpdateDlgItems();
	return TRUE; // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void DlgImportFiles::UpdateDlgItems()
{
	int j = m_ncurrent - 1;
	m_filefrom = m_pfilenameArray->GetAt(j);
	m_fileto = m_filefrom + m_ext;
	UpdateData(FALSE);
}

void DlgImportFiles::OnDestroy()
{
	CDialog::OnDestroy();

	int ilast = m_ncurrent + 1; // index last data file to open
	// adjust size of the file array
	if (m_pfilenameArray->GetSize() > ilast)
		m_pfilenameArray->SetSize(ilast);
}

void DlgImportFiles::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	CDialog::OnCancel();
}

void DlgImportFiles::ADC_OnBnClickedStartstop()
{
	m_pconvertedFiles->RemoveAll();

	// loop over the array of files
	for (int i = 0; i < m_nfiles; i++)
	{
		m_filefrom = m_pfilenameArray->GetAt(i);
		m_fileto = m_filefrom + m_ext;
		BOOL flag = TRUE;
		UpdateData(FALSE);

		switch (m_option)
		{
		case ATFFILE:
			flag = ImportATFFile();
			break;

		case GERT: // option non implemented
		case ASCIISYNTECH: // option non implemented
		default:
			flag = FALSE;
			break;
		}
		// if the conversion went well, add file to database and ask for further info from user
		if (flag)
			m_pconvertedFiles->Add(m_fileto);
		m_ncurrent++;
	}
	CDialog::OnOK();
}

BOOL DlgImportFiles::ImportATFFile()
{
	// load parameters
	BOOL flag = TRUE;

	// get infos from the source file
	CFileStatus status;
	BOOL b_flag_exists = CFile::GetStatus(m_filefrom, status);
	if (!b_flag_exists || (status.m_attribute & CFile::readOnly))
	{
		UpdateDlgItems();
		return FALSE;
	}

	// open source file; exit if a problem arises
	auto pFrom = new CStdioFile;
	ASSERT(pFrom != NULL);
	CFileException fe; // exception if somethg get wrong
	if (!pFrom->Open(m_filefrom, CFile::modeRead | CFile::shareDenyNone | CFile::typeText, &fe))
	{
		pFrom->Abort(); // abort process
		delete pFrom; // delete object and exit
		UpdateDlgItems();
		return FALSE;
	}

	// read infos; exit if file does not have the requested buzzword
	CString csdummy;
	CString csdummy2;
	CString csLine;

	// read first line
	pFrom->ReadString(csLine);
	int i = 0; // substring index to extract
	// parsing string can also be done with strtok (which allows multiple separators)
	AfxExtractSubString(csdummy, csLine, i, '\t');
	i++;
	AfxExtractSubString(csdummy2, csLine, i, '\t');
	i++;
	// check it is the correct data file version
	if (csdummy.Compare(_T("ATF")) != 0 || csdummy2.Compare(_T("1.0")))
	{
		pFrom->Abort(); // abort process
		delete pFrom; // delete object and exit
		UpdateDlgItems();
		return FALSE;
	}

	// create data file object
	// save data
	b_flag_exists = CFile::GetStatus(m_fileto, status);
	if (b_flag_exists || (status.m_attribute & CFile::readOnly))
	{
		UpdateDlgItems();
		delete pFrom;
		return FALSE;
	}

	auto pTo = new AcqDataDoc;
	ASSERT(pTo != NULL);
	if (!pTo->CreateAcqFile(m_fileto))
	{
		UpdateDlgItems();
		delete pTo;
		delete pFrom;
		return FALSE;
	}

	// read first line i.e. length of the data header
	pFrom->ReadString(csLine); // 1
	i = 0;
	AfxExtractSubString(csdummy, csLine, i, '\t');
	i++;
	AfxExtractSubString(csdummy2, csLine, i, '\t');
	i++;
	int nlines_in_header = _ttoi(csdummy);
	int ncolumns = _ttoi(csdummy2);

	m_scan_count = ncolumns - 1;
	ASSERT(m_scan_count > 0);
	ASSERT(m_scan_count <= 16);

	// describe data
	m_xinstgain = 100.; // tentative value (top to min=2000 mV)
	m_xrate = 10000.0f; // tentative value
	CWaveFormat* pwF = pTo->GetpWaveFormat();
	pwF->fullscale_volts = 20.0f; // 20 V full scale
	pwF->binspan = 65536; // 16 bits resolution
	pwF->binzero = 0; // ?

	pwF->mode_encoding = OLx_ENC_2SCOMP;
	pwF->mode_clock = INTERNAL_CLOCK;
	pwF->mode_trigger = INTERNAL_TRIGGER;
	pwF->scan_count = m_scan_count; // number of channels in scan list
	pwF->sampling_rate_per_channel = static_cast<float>(m_xrate); // channel sampling rate (Hz)
	pwF->csADcardName = "Digidata Axon";

	for (int i = 0; i < m_scan_count; i++)
	{
		int ichan = (pTo->GetpWavechanArray())->ChanArray_add();
		CWaveChan* pChannel = (pTo->GetpWavechanArray())->Get_p_channel(ichan);
		pChannel->am_gaintotal = static_cast<float>(m_xinstgain);
		m_dspan[i] = 20000. / m_xinstgain; // span= 20 V max to min
		m_dbinval[i] = m_dspan[i] / 65536.; // divide voltage span into 2exp16 bins
		pChannel->am_gainamplifier = pChannel->am_gaintotal;
		pChannel->am_gainAD = 1;
		pChannel->am_gainpre = 1;
		pChannel->am_gainpost = 1;
		pChannel->am_gainheadstage = 1;
		pChannel->am_adchannel = i; // channel A/D
	}

	/*
line 1-	ATF	1.0
line 2-	7	3
line 3-	"AcquisitionMode=Gap Free"
line 4-	"Comment="
line 5-	"YTop=200,10"
line 6-	"YBottom=-200,-10"
line 7-	"SweepStartTimesMS=0.000"
line 8-	"SignalsExported=Scaled V,Odour Cmd"
line 9-	"Signals="	"Scaled V"	"Odour Cmd"
line 10- "Time (s)"	"Trace #1 (mV)"	"Trace #1 (Au)"
line 11-	0	141.144	0.0317383
	*/
	// read dummy lines if user requests it or if number of lines != 7
	if (m_bReadHeader == FALSE || nlines_in_header != 7)
	{
		for (i = 0; i <= nlines_in_header; i++)
			pFrom->ReadString(csLine); // 1
	}
	else
	{
		// 1 acquisition mode "AcquisitionMode=Gap Free"
		pFrom->ReadString(csLine);

		// 2 dummy "comments"
		pFrom->ReadString(csLine);

		// 3 & 4 max & min values "YTop=200,10"
		pFrom->ReadString(csLine);
		csLine.Replace('=', ',');
		int i = 0;
		CString csLine2;
		pFrom->ReadString(csLine2);
		csLine2.Replace('=', ',');

		for (int ichan = 0; ichan < m_scan_count; ichan++)
		{
			i++;
			AfxExtractSubString(csdummy, csLine, i, ',');
			float xmax = static_cast<float>(_ttof(csdummy));
			AfxExtractSubString(csdummy2, csLine2, i, ',');
			float xmin = static_cast<float>(_ttof(csdummy2));
			// take the max absolute value
			xmin = abs(xmin);
			xmax = abs(xmax);
			if (xmin > xmax)
				xmax = xmin;
			float xtotal = static_cast<float>(xmax * 2.);
			CWaveChan* pChannel = (pTo->GetpWavechanArray())->Get_p_channel(ichan);
			pChannel->am_gaintotal = 20000. / xtotal;
		}

		// 5 start time
		pFrom->ReadString(csLine);
		// 6 columns names "SignalsExported=Scaled V,Odour Cmd"
		pFrom->ReadString(csLine);
		csLine.Replace('=', ',');
		i = 0;
		for (int ichan = 0; ichan < m_scan_count; ichan++)
		{
			i++;
			AfxExtractSubString(csdummy2, csLine, i, ',');
			csdummy2.Replace('"', ' ');
			CWaveChan* pChannel = (pTo->GetpWavechanArray())->Get_p_channel(ichan);
			pChannel->am_csComment = csdummy2;
		}

		pFrom->ReadString(csLine);
		// 7 columns scale
		pFrom->ReadString(csLine);
		// (8) title of the line
		pFrom->ReadString(csLine);
	}

	// change acq params & ask for proper descriptors
	if (!GetAcquisitionParameters(pTo)
		|| !GetExperimentParameters(pTo))
	{
		pTo->AcqDoc_DataAppendStop();
		pTo->AcqDeleteFile();
		goto Emergency_exit;
	}

	pwF->binspan = 65536; // 16 bits resolution
	pwF->binzero = 0;
	pwF->mode_encoding = OLx_ENC_BINARY;

	// start conversion
	int bufLen = 32768;
	pTo->AdjustBUF(bufLen);
	ULONGLONG compteur = pTo->GetBUFchanLength();
	int compteur2 = 0;
	pTo->AcqDoc_DataAppendStart();
	ULONGLONG compteurtotal = 0;
	double dtime_start = 0;
	double dtime_end = 0;
	short* pdataBUF0 = pTo->GetpRawDataBUF();

	TRY
		{
			while (compteur > 0)
			{
				double dvalue, dvalue2;
				short* pdataBUF = pdataBUF0;
				CString csLinetab;
				compteur2 = 0;

				for (int ii = 0; ii < bufLen; ii++)
				{
					pFrom->ReadString(csLinetab); // ; Wave data Signal
					// trap end-of-file condition - this is the only way we get out of this while loop
					if (csLinetab.IsEmpty())
					{
						dtime_end = _ttof(csdummy2);
						compteur = 0;
						break;
					}

					// extract time
					int i = 0; // substring index to extract
					AfxExtractSubString(csdummy2, csLinetab, i, '\t');
					if (compteurtotal == 0)
						dtime_start = _ttof(csdummy2);

					compteurtotal++;

					// extract channels
					for (int ichan = 0; ichan < m_scan_count; ichan++)
					{
						i++;
						AfxExtractSubString(csdummy, csLinetab, i, '\t');
						dvalue = _ttof(csdummy);
						dvalue2 = dvalue / m_dbinval[ichan];
						if ((dvalue2 > 32768.) || (dvalue2 < -32767.))
						{
							MessageBox(_T("Overflow error : decrease the amplifier gain"));
							pTo->AcqDoc_DataAppendStop();
							pTo->AcqDeleteFile();
							goto Emergency_exit;
						}
						*pdataBUF = static_cast<short>(dvalue2);
						pdataBUF++;
						compteur2++;
					}
				}

				// save data and update display
				CWnd* p_wnd = GetDlgItem(IDC_STATIC6);
				p_wnd->SetWindowText(csdummy2);
				pTo->AcqDoc_DataAppend(pdataBUF0, compteur2 * sizeof(short));
			}

			// update rate
			CWaveFormat* pwF = pTo->GetpWaveFormat();
			float xxrate = static_cast<float>(compteurtotal / (dtime_end - dtime_start));
			pwF->sampling_rate_per_channel = xxrate;
		}

		// useless but ...
	CATCH(CFileException, fe)
		{
			pTo->AcqDoc_DataAppend(pdataBUF0, compteur2 * sizeof(short));
		}
	END_CATCH

	// TODO: update file length?
	pTo->AcqDoc_DataAppendStop();
	pTo->AcqCloseFile();

	// exit: delete objects
Emergency_exit:
	delete pTo;
	delete pFrom;
	return flag;
}

BOOL DlgImportFiles::GetExperimentParameters(const AcqDataDoc* pTo) const
{
	DlgADExperiment dlg; 
	dlg.m_bFilename = FALSE; 
	auto p_app = dynamic_cast<CdbWaveApp*>(AfxGetApp());
	OPTIONS_INPUTDATA* pacqD = &(p_app->options_acqdata);
	dlg.m_pADC_options = pacqD;
	dlg.m_pdbDoc = m_pdbDoc;
	const BOOL flag = dlg.DoModal();
	if (IDOK == flag)
	{
		CWaveFormat* pwFTo = pTo->GetpWaveFormat();
		const CWaveFormat* pwFDlg = &pacqD->waveFormat;

		pwFTo->csADcardName = pwFDlg->csADcardName;
		pwFTo->cs_comment = pwFDlg->cs_comment;
		pwFTo->csStimulus = pwFDlg->csStimulus;
		pwFTo->csConcentration = pwFDlg->csConcentration;
		pwFTo->csInsectname = pwFDlg->csInsectname;
		pwFTo->csLocation = pwFDlg->csLocation;
		pwFTo->csSensillum = pwFDlg->csSensillum;
		pwFTo->csStrain = pwFDlg->csStrain;
		pwFTo->csOperator = pwFDlg->csOperator;
		pwFTo->csMoreComment = pwFDlg->csMoreComment;
		pwFTo->csStimulus2 = pwFDlg->csStimulus2;
		pwFTo->csConcentration2 = pwFDlg->csConcentration2;
		pwFTo->csSex = pwFDlg->csSex;
		pwFTo->insectID = pwFDlg->insectID;
		pwFTo->sensillumID = pwFDlg->sensillumID;
		return TRUE;
	}
	return FALSE;
}

BOOL DlgImportFiles::GetAcquisitionParameters(AcqDataDoc* pTo)
{
	DlgADInputs dlg2;
	dlg2.m_pwFormat = pTo->GetpWaveFormat();
	dlg2.m_pchArray = pTo->GetpWavechanArray();

	// invoke dialog box
	const BOOL flag = dlg2.DoModal();
	if (IDOK == flag)
	{
		for (int i = 0; i < m_scan_count; i++)
		{
			CWaveChan* pChannel = (pTo->GetpWavechanArray())->Get_p_channel(i);
			m_xinstgain = pChannel->am_gaintotal;
			m_dspan[i] = 20000. / m_xinstgain; // span= 20 V max to min
			m_dbinval[i] = m_dspan[i] / 65536.; // divide voltage span into 2exp16 bins
		}
		return TRUE;
	}
	return FALSE;
}
