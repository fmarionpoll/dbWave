
#include "StdAfx.h"
#include "dbWave_constants.h"
#include <stdlib.h>
#include <Strsafe.h>

#include "resource.h"
#include "Lineview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define new DEBUG_NEW

IMPLEMENT_SERIAL (CLineViewWnd, CScopeScreen, 1)

CLineViewWnd::CLineViewWnd()
{
	m_lxFirst = 0;
	m_lxLast  = 1024;
	m_lxVeryLast = 1024;
	m_lxSize = 1024;		// data size NULL
	m_cursorType =0;		// standard cursor until de-selected
	m_npixels = 100;		// width = 10 pixel
	m_dataperpixel=1;
	m_pDWintervals = nullptr;
	m_bVTtagsLONG=TRUE;		// VT tags defined as long
	m_btrackCurve = FALSE;
	m_btrackspike = FALSE;	// when mouse down, track spike / channel
	m_tracklen = 60;
	m_trackoffset = 20;
	m_trackchannel = 0;

	// init arrays for cool display of empty data
	m_pDataFile = nullptr;
	m_bADbuffers = FALSE;
	AddChanlistItem(0, 0);
	ResizeChannels(m_npixels, 1024);

	m_csEmpty = "no data";
}

CLineViewWnd::~CLineViewWnd()
{
	// remove list of objects (channels & Envelopes)
	RemoveAllChanlistItems();
}

BEGIN_MESSAGE_MAP(CLineViewWnd, CScopeScreen)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_SIZE()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// lineview operations on chanlist items
// changes to list or list contents
/////////////////////////////////////////////////////////////////////////////

void CLineViewWnd::RemoveAllChanlistItems()
{        
	// suppress array and objects pointed by m_pEnvelopesArray
	for (int i=m_pEnvelopesArray.GetUpperBound(); i>= 0; i--)
		delete m_pEnvelopesArray[i];	
	m_pEnvelopesArray.RemoveAll();

	// suppress array and objects pointed by m_pChanlistItemArray
	for (int i=m_pChanlistItemArray.GetUpperBound(); i>= 0; i--)
		delete m_pChanlistItemArray[i];
	m_pChanlistItemArray.RemoveAll();	
}

int CLineViewWnd::AddChanlistItem(int ns, int mode)
{ 
	// first time??	create Envelope(0) with abcissa series
	if (m_pChanlistItemArray.GetSize() == 0)
	{
		m_PolyPoints.SetSize(m_npixels*4);		// set size of polypoint array
		m_scale.SetScale(m_npixels, m_lxSize);	// compute scale (this is first time)
		m_dataperpixel = 2;
		CEnvelope* pC = new CEnvelope(m_npixels*m_dataperpixel, m_dataperpixel, 0, -1, 0);
		ASSERT(pC != NULL);
		m_pEnvelopesArray.Add(pC);					// add item Ptr to array	
		pC->FillEnvelopeWithAbcissa(m_npixels*m_dataperpixel, m_lxSize);
	}

	// create new Envelope and store pointer into Envelopeslist
	int span=0;
	if(m_pDataFile != nullptr)
		span = m_pDataFile->GetTransfDataSpan(mode);
	CEnvelope* pY = new CEnvelope(m_npixels, m_dataperpixel, ns, mode, span);
	ASSERT(pY != NULL);
	int j = m_pEnvelopesArray.Add(pY);
	
	// create new chanlistitem with x=Envelope(0) and y=the new Envelope
	CChanlistItem* pD = new CChanlistItem(m_pEnvelopesArray.GetAt(0), 0, pY, j);
	ASSERT(pD != NULL);
	int index_newchan = m_pChanlistItemArray.Add(pD);

	// init display parameters
	pD->InitDisplayParms(1, RGB(0,0,0), 2048, 4096);

	if (m_pDataFile != nullptr)
	{
		float voltsperb;
		m_pDataFile->GetWBVoltsperBin(ns, &voltsperb, mode);
		CWaveChanArray* pchanArray = m_pDataFile->GetpWavechanArray();
		CWaveFormat*    pwaveFormat = m_pDataFile->GetpWaveFormat();
		pD->SetDataBinFormat(pwaveFormat->binzero, pwaveFormat->binspan);
		pD->SetDataVoltsFormat(voltsperb, pwaveFormat->fullscale_Volts);
		if (ns >= pchanArray->channel_get_number())
			ns = 0;
		CWaveChan* pchan = pchanArray->get_p_channel(ns);
		pD->dl_comment = pchan->am_csComment;	// get comment however
		UpdateChanlistMaxSpan();				// update span max
		if (mode > 0)							// modif comment if transform buffer
			pD->dl_comment = (m_pDataFile->GetTransfDataName(mode)).Left(8) + ": " + pD->dl_comment;
	}
	return index_newchan;
}

int CLineViewWnd::RemoveChanlistItem(WORD i)
{   
	int j = m_pChanlistItemArray.GetSize();	// get size of chan array
	if (j >0)	// delete Envelopes ordinates but make sure that it is not used
	{
		CEnvelope* pa = m_pChanlistItemArray[i]->pEnvelopeOrdinates;
		// step 1: check that this envelope is not used by another channel
		BOOL bUsedOnlyOnce = TRUE;
		for (int lj= j; lj>=0; lj--)
		{
			CEnvelope* pb = m_pChanlistItemArray[i]->pEnvelopeOrdinates;
			if (pa == pb && lj != i)
			{
				bUsedOnlyOnce = FALSE;	// the envelope is used by another channel
				break;					// stop search and exit loop
			}
		}
		// step 2: delete corresponding envelope only if envelope used only once.
		if (bUsedOnlyOnce)
		{
			for (int k=m_pEnvelopesArray.GetUpperBound(); k>= 0; k--)
			{
				CEnvelope* pb = m_pEnvelopesArray[k];
				if (pa == pb)	// search where this Envelope was stored
				{
					delete pa;						// delete  object
					m_pEnvelopesArray.RemoveAt(k);	// remove pointer from array
					break;		// object is found, stop loop and delete chanlist item
				}
			}
		}
		// step 3: delete channel
		delete m_pChanlistItemArray[i];
		m_pChanlistItemArray.RemoveAt(i);		
	}
	UpdateChanlistMaxSpan();
	return j-1;
}

void CLineViewWnd::UpdateChanlistMaxSpan()
{
	if (m_pEnvelopesArray.GetSize() <= 1)
		return;
	// get spanmax stored in Envelope(0)
	int imax = 0;
	for (int i = m_pEnvelopesArray.GetUpperBound(); i>0; i--)
	{
		int j = m_pEnvelopesArray[i]->GetDocbufferSpan();
		if (j > imax) 
			imax = j;
	}
	 // store imax
	m_pEnvelopesArray[0]->SetDocbufferSpan(imax);	//store max
}

void CLineViewWnd::UpdateChanlistFromDoc()
{
	for (int i = m_pChanlistItemArray.GetUpperBound(); i>= 0; i--)
	{
		CChanlistItem* pD =  m_pChanlistItemArray[i];
		CEnvelope* pOrd = pD->pEnvelopeOrdinates;
		int ns = pOrd->GetSourceChan();
		int mode = pOrd->GetSourceMode();
		pOrd->SetDocbufferSpan(m_pDataFile->GetTransfDataSpan(mode));
		CWaveChanArray* pchanArray = m_pDataFile->GetpWavechanArray();
		CWaveChan* pchan = pchanArray->get_p_channel(ns);
		pD->dl_comment= pchan->am_csComment;
		if (mode > 0)
			pD->dl_comment = (m_pDataFile->GetTransfDataName(mode)).Left(6) + ": " + pD->dl_comment;
		UpdateGainSettings(i);	// keep physical value of yextent and zero constant
	}
	UpdateChanlistMaxSpan();	// update max span
}

void CLineViewWnd::UpdateGainSettings(int i)
{
	CChanlistItem* pD =  m_pChanlistItemArray[i];
	CEnvelope* pOrd = pD->pEnvelopeOrdinates;
	int ns = pOrd->GetSourceChan();
	int mode = pOrd->GetSourceMode();
	float docVoltsperb;
	m_pDataFile->GetWBVoltsperBin(ns,	&docVoltsperb, mode);	
	float pDVoltsperBin = pD->GetVoltsperDataBin();
	CWaveFormat* pwaveFormat = m_pDataFile->GetpWaveFormat();
	if (docVoltsperb != pDVoltsperBin)
	{
		pD->SetDataBinFormat(pwaveFormat->binzero, pwaveFormat->binspan);
		pD->SetDataVoltsFormat(docVoltsperb, pwaveFormat->fullscale_Volts);
		int iextent = pD->GetYextent();
		iextent = (int) (((float) iextent)/docVoltsperb*pDVoltsperBin);
		pD->SetYextent(iextent);
	}
}

int CLineViewWnd::SetChanlistSourceChan(WORD i, int ns)
{
	// check if channel is allowed
	CWaveFormat* pwaveFormat = m_pDataFile->GetpWaveFormat();
	if (pwaveFormat->scan_count <= ns || ns < 0)
	{
		return -1;
	}

	// make sure we have enough data channels...
	if (GetChanlistSize() <= i)
	{
		int nchans = GetChanlistSize();
		for (int j=nchans; j<= i; j++)
			AddChanlistItem(j, 0);
	}
	// change channel
	CChanlistItem* pD =  m_pChanlistItemArray[i];
	CEnvelope* pOrd = pD->pEnvelopeOrdinates;
	pOrd->SetSourceChan(ns);			// change data channel
	int mode = pOrd->GetSourceMode();	// get transform mode
	// modify comment
	CWaveChanArray* pchanArray = m_pDataFile->GetpWavechanArray();
	CWaveChan* pchan = pchanArray->get_p_channel(ns);
	pD->dl_comment= pchan->am_csComment;		
	if (mode > 0)
		pD->dl_comment = (m_pDataFile->GetTransfDataName(mode)).Left(6) 
							+ _T(": ") + pD->dl_comment;
	UpdateGainSettings(i);				// keep physical value of yextent and zero constant
	return ns;
}

void CLineViewWnd::SetChanlistOrdinates(WORD i, int chan, int transform)
{
	// change channel
	CChanlistItem* pD =  m_pChanlistItemArray[i];	
	pD->SetOrdinatesSourceData(chan, transform);
	// modify comment
	CWaveChanArray* pchanArray = m_pDataFile->GetpWavechanArray();
	if (chan >= pchanArray->channel_get_number())
			chan = 0;
	CWaveChan* pchan = pchanArray->get_p_channel(chan);
	pD->dl_comment= pchan->am_csComment;		
	if (transform > 0)
		pD->dl_comment = (m_pDataFile->GetTransfDataName(transform)).Left(6) 
						+ _T(": ") + pD->dl_comment;
}

void CLineViewWnd::SetChanlistVoltsExtent(int chan, float* pvalue)
{
	int ichanfirst = chan;
	int ichanlast = chan;
	if (chan < 0) 
	{
		ichanfirst = 0;
		ichanlast = m_pChanlistItemArray.GetUpperBound();
	}
	float voltsextent = 0.f;
	if (pvalue != nullptr)
		voltsextent = *pvalue;
	for (int i= ichanfirst; i<= ichanlast; i++)
	{
		CChanlistItem* pD =  m_pChanlistItemArray[i];
		float yvoltsperbin = pD->GetVoltsperDataBin();
		if (pvalue == nullptr) 
			voltsextent = yvoltsperbin * pD->GetYextent();

		int yextent = (int)(voltsextent / yvoltsperbin);
		pD->SetYextent(yextent);
	}
}

void CLineViewWnd::SetChanlistVoltsZero(int chan, float* pvalue)
{
	int ichanfirst = chan;
	int ichanlast = chan;
	if (chan < 0)
	{
		ichanfirst = 0;
		ichanlast = m_pChanlistItemArray.GetUpperBound();
	}

	float voltsextent = 0.f;
	if (pvalue != nullptr)
		voltsextent = *pvalue;
	for (int i = ichanfirst; i <= ichanlast; i++)
	{
		CChanlistItem* pD = m_pChanlistItemArray[i];
		float yvoltsperbin = pD->GetVoltsperDataBin();
		if (pvalue == nullptr)
			voltsextent = yvoltsperbin * pD->GetDataBinZero();

		int iyzero = (int)(voltsextent / yvoltsperbin);
		pD->SetYzero(iyzero);
	}
}

int CLineViewWnd::SetChanlistTransformMode(WORD i, int imode)
{
	// check if transform is allowed
	if (!m_pDataFile->IsWBTransformAllowed(imode) ||	// ? is transform allowed
		!m_pDataFile->InitWBTransformMode(imode)) 		// ? is init OK
	{
		AfxMessageBox(IDS_LNVERR02, MB_OK);
		return -1;
	}

	// change transform mode
	CChanlistItem* pD =  m_pChanlistItemArray[i];
	CEnvelope* pOrd = pD->pEnvelopeOrdinates;
	int ns = pOrd->GetSourceChan();
	// change transform
	pOrd->SetSourceMode(imode, m_pDataFile->GetTransfDataSpan(imode));

	// modify comment
	CWaveChanArray* pchanArray = m_pDataFile->GetpWavechanArray();
	CWaveChan* pchan = pchanArray->get_p_channel(ns);
	pD->dl_comment= pchan->am_csComment;		
	if (imode > 0)
		pD->dl_comment = (m_pDataFile->GetTransfDataName(imode)).Left(8) 
							+ _T(": ") + pD->dl_comment;
	UpdateGainSettings(i);	// keep physical value of yextent and zero constant
	UpdateChanlistMaxSpan();
	return imode;
}

void CLineViewWnd::AutoZoomChan(int j)
{
	int i1 = j;
	int i2 = j;
	if (j <0)
	{
		i1 = 0;
		i2 = GetChanlistSize()-1;
	}
	
	int max, min;
	for (int i=i1; i<= i2; i++)
	{
		GetChanlistMaxMin(i, &max, &min);
		SetChanlistYzero(i, (max+min)/2);
		SetChanlistYextent(i, MulDiv(max-min+1, 10, 8));
	}
}

void CLineViewWnd::SplitChans()
{
	int max, min;
	int nchans = GetChanlistSize();
	int icur = nchans-1;
	for (int i=0; i< nchans; i++, icur -=2)
	{
		GetChanlistMaxMin(i, &max, &min);
		int amplitudespan = MulDiv((max-min), 12*nchans, 10);
		SetChanlistYextent(i, amplitudespan);
		int ioffset = (max+min)/2 + MulDiv(amplitudespan, icur, nchans*2);
		SetChanlistYzero(i, ioffset);
	}
}

void CLineViewWnd::CenterChan(int j)
{
	int i1 = j;
	int i2 = j;
	if (j <0)
	{
		i1 = 0;
		i2 = GetChanlistSize()-1;
	}

	int max, min;	
	for (int i=i1; i<= i2; i++)
	{
		GetChanlistMaxMin(i, &max, &min);
		int yzero = (max+min)/2;
		SetChanlistYzero(i, yzero);
	}
}

void CLineViewWnd::MaxgainChan(int j)
{
	int i1 = j;
	int i2 = j;
	if (j <0)
	{
		i1 = 0;
		i2 = GetChanlistSize()-1;
	}

	int max, min;	
	for (int i=i1; i<= i2; i++)
	{
		GetChanlistMaxMin(i, &max, &min);	
		int yextent = MulDiv(max-min+1, 10, 8);
		SetChanlistYextent(i, yextent);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CLineViewWnd message handlers
/////////////////////////////////////////////////////////////////////////////

int CLineViewWnd::ResizeChannels(int npixels, long lSize)
{
	int oldNpixels = m_npixels;
	int olddataperpixel = m_dataperpixel;

	// trap dummy values and return if no operation is necessary

	if (npixels ==0)				// horizontal resolution
		npixels = m_npixels;
	else
		m_npixels = npixels;
	if (lSize == 0)					// size of data to display
		lSize = m_lxSize;
	
	// change horizontal resolution	: m_Polypoints receives abcissa and ordinates
	// make provision for max and min points

	if (m_PolyPoints.GetSize() != m_npixels*4)	
		m_PolyPoints.SetSize(m_npixels*4);

	// compute new scale and change size of Envelopes

	m_lxSize = lSize;
	m_scale.SetScale(m_npixels, m_lxSize);
	m_dataperpixel = 2;

	// change Envelopes size
	int npts = m_npixels;			// set Envelope size to the nb of pixels
	if (m_npixels > m_lxSize)
		npts = (int) m_lxSize;		// except when there is only one data pt
	
	ASSERT(npts > 0);

	int nEnvelopes = m_pEnvelopesArray.GetSize();// loop through all Envelopes
	if (nEnvelopes > 0)
	{
		CEnvelope* pC;
		for (int iEnvelope=0; iEnvelope< nEnvelopes; iEnvelope++)
		{
			pC = m_pEnvelopesArray.GetAt(iEnvelope);
			pC->SetEnvelopeSize(npts, m_dataperpixel);
		}
		pC = m_pEnvelopesArray.GetAt(0);	
		pC->FillEnvelopeWithAbcissa(m_npixels, m_lxSize);// store data series
	}

	// read data and update page and line sizes / file browse    
	UpdatePageLineSize();
	m_lxLast = m_lxFirst + m_lxSize-1;	
	return oldNpixels;
}

BOOL CLineViewWnd::AttachDataFile(CAcqDataDoc* pDataFile, long lSize)
{
	m_pDataFile = pDataFile;
	m_samplingrate = m_pDataFile->GetpWaveFormat()->chrate;
	m_pDataFile->SetReadingBufferDirty();
	ASSERT(m_pDataFile->GetDOCchanLength() > 0);
	if (lSize <= 0)
	{
		lSize = GetDataSize();
		if (lSize > m_pDataFile->GetDOCchanLength())	// check
			lSize = m_pDataFile->GetDOCchanLength();	
	}

	// init parameters used to display Envelopes
	int lVeryLast = m_lxVeryLast;
	m_lxVeryLast = m_pDataFile->GetDOCchanLength()-1;
	m_lxFirst = 0;
	m_lxLast = lSize -1;
	if (m_lxSize !=lSize || lVeryLast != m_lxVeryLast)
	{
		m_lxSize =lSize;
		m_dataperpixel = m_scale.SetScale(m_npixels, m_lxSize);
		m_dataperpixel = 2;
	}

	//Remove irrelevant Chanlist items;	
	int docchanmax = m_pDataFile->GetpWaveFormat()->scan_count - 1;
	int chanlistmax = m_pChanlistItemArray.GetUpperBound();
	for (int i=chanlistmax; i>=0; i--)
	{
		if (GetChanlistSourceChan(i) > docchanmax)
			RemoveChanlistItem(i);
	}

	// Remove irrelevant Envelopes();
	int nEnvelopes = m_pEnvelopesArray.GetUpperBound();
	CEnvelope* pC;
	for (int i=nEnvelopes; i> 0; i--) // ! Envelope(0)=abcissa
	{
		pC = m_pEnvelopesArray.GetAt(i);
		if (pC->GetSourceChan() > docchanmax)
		{
			delete pC; 
			m_pEnvelopesArray.RemoveAt(i);
		}
	}

	// update chan list
	UpdateChanlistFromDoc();
	return TRUE;
}

BOOL CLineViewWnd::GetDataFromDoc()
{   	
	// get document parameters: exit if empty document	
	if (m_bADbuffers || m_pDataFile == nullptr )
		return FALSE;
	if (m_pDataFile->GetDOCchanLength() <= 0)
		return FALSE;

	// check intervals	(assume m_lxSize OK)
	if (m_lxFirst <0)	
		m_lxFirst = 0;							// avoid negative start
	m_lxLast = m_lxFirst + m_lxSize -1;			// test end
	if (m_lxLast > m_lxVeryLast)				// past end of file?
	{
		if (m_lxSize >= m_lxVeryLast+1)
			m_lxSize = m_lxVeryLast +1;
		m_lxLast = m_lxVeryLast;				// clip to end
		m_lxFirst = m_lxLast -m_lxSize +1;		// change start
	}
	int nchans;									// n raw channels
	short* lpBuf= m_pDataFile->LoadRawDataParams(&nchans);
	short* lpData;								// pointer used later

	// max nb of points spanning around raw data pt stored in array(0)
	if (m_pEnvelopesArray.GetSize() < 1)
		return FALSE;
	CEnvelope* pCont = m_pEnvelopesArray.GetAt(0);
	int nspan = pCont->GetDocbufferSpan();		// additional pts necessary

	// loop through all pixels if data buffer is longer than data displayed 
	// within one pixel...
	long lFirst = m_lxFirst;					// start
	int ipixelmax = m_scale.GetnIntervals();	// max pixel

	for (int ipixel = 0; ipixel < ipixelmax; ipixel++)
	{
		// compute file index of pts within current pixel
		int data_within_1pixel = m_scale.GetIntervalSize(ipixel); // size first pixel
		int lLast = lFirst + data_within_1pixel- 1;
		BOOL bNew = TRUE;						// flag to tell routine that it should start from new data

		while (lFirst <= lLast)
		{
			long lBUFchanFirst= lFirst;			// index very first pt within pixel
			long lBUFchanLast = lLast;			// index very last pixel

			 // ask document to read raw data, document returns index of data loaded within the buffer
			if (!m_pDataFile->LoadRawData(&lBUFchanFirst, &lBUFchanLast, nspan))
				break;							// exit if error reported

			// build Envelopes  .................
			if (lBUFchanLast > lLast)
				lBUFchanLast = lLast;
			int npoints = lBUFchanLast - lFirst +1; 
			if (npoints <= 0)
				break;

			// loop over each envelope
			for (int iEnvelope=m_pEnvelopesArray.GetUpperBound(); iEnvelope>0; iEnvelope--)
			{
				pCont = m_pEnvelopesArray.GetAt(iEnvelope);

				int sourceChan = pCont->GetSourceChan();// get source channel
				int itransf =  pCont->GetSourceMode();	// get transform mode
				if (itransf>0)							// if transformation, compute transf
				{										// and then build envelope
					lpData = m_pDataFile->LoadTransfData(lFirst, lBUFchanLast, itransf, sourceChan);
					pCont->FillEnvelopeWithMxMi(ipixel, lpData, 1, npoints, bNew);
				}
				else									// no transformation: compute max min
				{										// and then build envelope
					lpData = m_pDataFile->GetpRawDataElmt(sourceChan, lFirst);
					pCont->FillEnvelopeWithMxMi(ipixel, lpData, nchans, npoints, bNew);
				}
			}
			bNew = FALSE;
			lFirst = lBUFchanLast + 1;
		}	
	}
	return TRUE;
}

BOOL CLineViewWnd::GetSmoothDataFromDoc(int ioption)
{   	
	// get document parameters: exit if empty document	
	if (m_bADbuffers || m_pDataFile == nullptr )
		return FALSE;
	if (m_pDataFile->GetDOCchanLength() <= 0)
		return FALSE;

	// check intervals	(assume m_lxSize OK)
	if (m_lxFirst <0)	
		m_lxFirst = 0;							// avoid negative start
	m_lxLast = m_lxFirst + m_lxSize -1;			// test end
	if (m_lxLast > m_lxVeryLast)				// past end of file?
	{
		if (m_lxSize >= m_lxVeryLast+1)
			m_lxSize = m_lxVeryLast +1;
		m_lxLast = m_lxVeryLast;				// clip to end
		m_lxFirst = m_lxLast -m_lxSize +1;		// change start
	}
	int nchans;									// n raw channels
	short* lpBuf= m_pDataFile->LoadRawDataParams(&nchans);
	short* lpData;								// pointer used later
	// max nb of points spanning around raw data pt stored in array(0)
	CEnvelope* pCont = m_pEnvelopesArray.GetAt(0);
	int nspan = pCont->GetDocbufferSpan();		// additional pts necessary

	// loop through all pixels if data buffer is longer than data displayed 
	// within one pixel...
	long lFirst = m_lxFirst;					// start
	int ipixelmax = m_scale.GetnIntervals();	// max pixel

	for (int ipixel = 0; ipixel < ipixelmax; ipixel++)
	{
		// compute file index of pts within current pixel
		int data_within_1pixel = m_scale.GetIntervalSize(ipixel); // size first pixel
		int lLast = lFirst + data_within_1pixel- 1;
		BOOL bNew = TRUE;						// flag to tell routine that it should start from new data

		while (lFirst <= lLast)
		{
			long lBUFchanFirst= lFirst;			// index very first pt within pixel
			long lBUFchanLast = lLast;			// index very last pixel

			 // ask document to read raw data, document returns index of data loaded within the buffer
			if (!m_pDataFile->LoadRawData(&lBUFchanFirst, &lBUFchanLast, nspan))
				break;							// exit if error reported

			// build Envelopes  .................
			if (lBUFchanLast > lLast)
				lBUFchanLast = lLast;
			int npoints = lBUFchanLast - lFirst +1; 
			if (npoints <= 0)
				break;

			// loop over each envelope
			for (int iEnvelope=m_pEnvelopesArray.GetUpperBound(); iEnvelope>0; iEnvelope--)
			{
				pCont = m_pEnvelopesArray.GetAt(iEnvelope);

				int sourceChan = pCont->GetSourceChan();// get source channel
				int itransf =  pCont->GetSourceMode();	// get transform mode
				if (itransf>0)							// if transformation, compute transf
				{										// and then build envelope
					lpData = m_pDataFile->LoadTransfData(lFirst, lBUFchanLast, itransf, sourceChan);
					pCont->FillEnvelopeWithSmoothMxMi(ipixel, lpData, 1, npoints, bNew, ioption);
				}
				else									// no transformation: compute max min
				{										// and then build envelope
					lpData = m_pDataFile->GetpRawDataElmt(sourceChan, lFirst);
					pCont->FillEnvelopeWithSmoothMxMi(ipixel, lpData, nchans, npoints, bNew, ioption);
				}
			}
			bNew = FALSE;
			lFirst = lBUFchanLast + 1;
		}	
	}
	return TRUE;
}

BOOL CLineViewWnd::GetDataFromDoc(long lFirst)
{	
	if (lFirst == m_lxFirst)
		return TRUE;
	m_lxFirst = lFirst;
	return GetDataFromDoc();	
}

BOOL CLineViewWnd::GetDataFromDoc(long lFirst, long lLast)
{
	// check if size has changed
	if (lFirst <0) 				// first check limits across document's
		lFirst = 0;				// minimum
	if (lLast > m_lxVeryLast)	// maximum
		lLast = m_lxVeryLast;    
	if ((lFirst > m_lxVeryLast) || (lLast < lFirst))
	{							// shuffled intervals
		lFirst = 0;
		if (m_lxSize > m_lxVeryLast+1)
			m_lxSize =	m_lxVeryLast+1;
		lLast = lFirst + m_lxSize -1;
	}

	// requested size different from current one?
	if (m_lxSize != (lLast - lFirst+1))
	{		
		m_lxFirst = lFirst;
		ResizeChannels(0, (lLast - lFirst+1));		// n pixels = m_npixels
	}
	// load data
	m_lxFirst = lFirst;
	m_lxLast = lLast;
	return GetDataFromDoc();
}

BOOL CLineViewWnd::ScrollDataFromDoc(WORD nSBCode)
{
	long lFirst=m_lxFirst;	
	switch (nSBCode)
		{
		case SB_LEFT:		// Scroll to far left.
			lFirst = 0;	
			break;
		case SB_LINELEFT:	// Scroll left.
			lFirst -= m_lxLine;
			break;
		case SB_LINERIGHT:	// Scroll right.
			lFirst += m_lxLine;
			break;
		case SB_PAGELEFT:	// Scroll one page left.
			lFirst -= m_lxPage;
			break;
		case SB_PAGERIGHT:	// Scroll one page right.
			lFirst += m_lxPage;
			break;
		case SB_RIGHT:		// Scroll to far right.
			lFirst = m_lxVeryLast - m_lxSize+1;
			break;
		default:
			return FALSE;
			break;
		}	
	m_lxFirst = lFirst;
	return GetDataFromDoc();	
}

void CLineViewWnd::UpdatePageLineSize()
{
	if (m_pDataFile != nullptr)
		m_lxPage = m_lxSize;
	else
		m_lxPage = m_lxSize/10;
	if (m_lxPage == 0)
		m_lxPage = 1;
	m_lxLine = m_lxPage/m_npixels;
	if (m_lxLine == 0)
		m_lxLine=1;
}

void CLineViewWnd::ZoomData(CRect* r1, CRect* r2)
{
	r1->NormalizeRect();
	r2->NormalizeRect();	
	
	int i = m_pChanlistItemArray.GetUpperBound();
	// change gain & offset of all channels:
	for (i; i>=0; i--)	// scan all channels
	{
		// display loop: load abcissa
		CChanlistItem* pDL = m_pChanlistItemArray[i];
		int extent = pDL->GetYextent();
		int newext = MulDiv (extent, r2->Height(), r1->Height());
		pDL->SetYextent (newext);
		int zero = pDL->GetYzero();
		int newzero = zero - (r1->Height() - r2->Height()) / 2;
		pDL->SetYzero (newzero);
	}
		
	// change index of first and last pt displayed
	long lSize = m_lxLast-m_lxFirst+1;
	long lFirst = m_lxFirst + (lSize*(r2->left-r1->left))/r1->Width();
	lSize = (lSize *r2->Width()) / r1->Width();
	if (lSize > m_lxVeryLast)
	{
		lSize = m_lxVeryLast;
		lFirst = 0;
	}
	long lLast  = lFirst +lSize -1; 
		
	// call for new data only if indexes are different
	if (lFirst != m_lxFirst || lLast != m_lxLast)
	{
		ResizeChannels(0, lLast-lFirst+1);
		GetDataFromDoc(lFirst, lLast);
	}	
	PostMyMessage(HINT_VIEWSIZECHANGED, NULL);	
}

//---------------------------------------------------------------------------
// Plot functions
//
// PlotDatatoDC();  - draw on DC passed in argument
//
// CLineViewWnd does not use the DIB surface provided by CScopeScreen from
// which it is derived. Thus, it must get its DC from Windows.
// In this case, when the nb of curves and the nb of points is small, this
// direct approach is faster than resorting to the DIBsurface
//---------------------------------------------------------------------------

void CLineViewWnd::UpdateXRuler()
{
	if (m_bNiceGrid)
	{
		if (m_pXRulerBar != nullptr)
		{
			float first = m_lxFirst/ m_samplingrate;
			float last = m_lxLast/ m_samplingrate;
			m_xRuler.UpdateRange(&first, &last);
		}
	}
}

void CLineViewWnd::UpdateYRuler()
{
	if (m_bNiceGrid)
	{
		if (m_pYRulerBar != nullptr)
		{
			int binlow = GetChanlistPixeltoBin(0, 0);
			int binhigh = GetChanlistPixeltoBin(0, m_clientRect.Height());
			float yfirst = ConvertChanlistDataBinsToVolts(0, binlow);
			float ylast = ConvertChanlistDataBinsToVolts(0, binhigh);
			m_yRuler.UpdateRange(&yfirst, &ylast);
		}
	}
}

void CLineViewWnd::PlotDatatoDC(CDC* pDC)
{
	if (m_bADbuffers)
		return;

	// erase background
	if (m_erasebkgnd) 
	{
		UpdateXRuler();
		UpdateYRuler();
		EraseBkgnd(pDC);
	}

	CRect rect = m_displayRect;
	rect.DeflateRect(1,1);
	CFont* pOldFont= pDC->SelectObject(&m_hFont);

	// exit if no data defined
	if (!IsDefined() || m_pDataFile == nullptr) 
	{
		int textlen = m_csEmpty.GetLength();
		pDC->DrawText(m_csEmpty, textlen, rect, DT_LEFT);
		return;
	}

	// plot comment at the bottom
	if (m_bBottomComment)
	{
		int textlen = m_csBottomComment.GetLength();
		pDC->SetTextColor(RGB(0, 0, 255)); // BLUE
		pDC->DrawText(m_csBottomComment, textlen, rect, DT_RIGHT|DT_BOTTOM|DT_SINGLELINE); 
	}

	// save DC
	int nSavedDC = pDC->SaveDC();
	ASSERT(nSavedDC != 0);
	pDC->IntersectClipRect(&m_displayRect);
	
	// prepare DC
	pDC->SetMapMode (MM_ANISOTROPIC);
	pDC->SetViewportOrg (m_displayRect.left, m_displayRect.Height()/2);
	pDC->SetViewportExt (m_displayRect.Width(), -m_displayRect.Height());
	pDC->SetWindowExt (m_displayRect.Width(), m_displayRect.Height()); 
	pDC->SetWindowOrg (0, 0);

	// display all channels
	long* pPolypoints_X = (long*) &m_PolyPoints[0];
	long* pPolypoints_Y = (long*) &m_PolyPoints[1];
	long* pData=pPolypoints_X;
	int nelements=0;
	int ichan = m_pChanlistItemArray.GetUpperBound();
	int worg = -1;		// force origin
	int wext = -1;		// force <= yextent
	int yVE = m_displayRect.Height();
	CEnvelope* pX = nullptr;
	CEnvelope* pY = nullptr;
	int xextent = m_npixels;
	int color = BLACK_COLOR;
	CPen* poldpen=pDC->SelectObject(&m_penTable[color]);

	// display loop:
	for (ichan; ichan>=0; ichan--)	// scan all channels
	{
		// get pointer to display list item "i"
		CChanlistItem* pDL = m_pChanlistItemArray[ichan];

		// display: load new abcissa ?  -------------------------------------------
		if (pX != pDL->pEnvelopeAbcissa)
		{
			pX = pDL->pEnvelopeAbcissa;
			nelements = pX->GetEnvelopeSize();
			xextent = pX->GetnElements();
			// this should not happen but it does ..; so here is the ugly test:
			if (m_PolyPoints.GetSize() != nelements*2)
			{
				m_PolyPoints.SetSize(nelements*2);
				pPolypoints_X = (long*) &m_PolyPoints[0];
				pPolypoints_Y = (long*) &m_PolyPoints[1];
				pData=pPolypoints_X;
			}
			pX->ExportToPolyPts(pPolypoints_X);			
		}
		// change extent, org ? ------------------------------------------------
		if (pDL->GetYextent() != wext)
			wext = pDL->GetYextent();
		if (pDL->GetYzero() != worg)
			worg = pDL->GetYzero();
		// display: load new ordinates ? ------------------------------------------
		pY = pDL->pEnvelopeOrdinates;
		pY->ExportToPolyPts(pPolypoints_Y);	
		// change pen, draw mode ? ----------------- not implemented yet
		//	WORD penwidth;			// pen size
		//	WORD color;				// color
		if (pDL->GetColor() != color)
		{
			color = pDL->GetColor();			
			pDC->SelectObject(&m_penTable[color]);
		}
		if (pDL->GetPenWidth() == 0)
			continue;
		LPPOINT pD = (LPPOINT) pData;			// temporary pointer
		for (int j = 0; j<nelements; j++, pD++)
			pD->y = MulDiv(((short)pD->y) - worg, yVE, wext);
		pDC->MoveTo(*pData,*(pData+1));
		pDC->Polyline((LPPOINT) pData, nelements);

		//display associated cursors ----------------------------------------
		if (GetNHZtags() > 0)
		{
			// select pen and display mode 
			pDC->SelectObject(&m_blackDottedPen);
			int noldROP = pDC->SetROP2(R2_NOTXORPEN);
			// iterate through HZ cursor list			
			for (int j= GetNHZtags()-1; j>= 0; j--)
			{				
				if (GetHZtagChan(j) != ichan)
					continue;				
				int k = GetHZtagVal(j);		// get val
				k = MulDiv(((short)k)-worg, yVE, wext);
				pDC->MoveTo(0,k);			// set initial pt
				pDC->LineTo(nelements, k);	// HZ line				
			}
			pDC->SetROP2(noldROP);			// restore old display mode
			pDC->SelectObject(poldpen);
		}
			
		// highlight data
		if (m_pDWintervals != nullptr)
			HighlightData(pDC, ichan);
	}

	// restore DC
	pDC->SelectObject(poldpen);		// restore old pen
	pDC->RestoreDC(nSavedDC);

	// display vertical cursors ------------------------------------
	if (GetNVTtags() > 0)
	{			
		// select pen and display mode 
		CPen* poldp = pDC->SelectObject(&m_blackDottedPen);
		int noldROP = pDC->SetROP2(R2_NOTXORPEN);
		// iterate through VT cursor list	
		int y0 = 0;
		int y1 = m_displayRect.bottom;
		for (int j=GetNVTtags()-1; j>=0; j--)
		{			
			long lk = GetVTtagLval(j);	// get val
			if (lk <m_lxFirst || lk > m_lxLast)
				continue;
			float llk = (lk-m_lxFirst)*(float)(m_displayRect.Width()) / (m_lxLast - m_lxFirst+1); //muldiv
			int k = (int) llk;
			pDC->MoveTo(k,y0);			// set initial pt
			pDC->LineTo(k,y1);			// VT line
		}
		pDC->SetROP2(noldROP);			// restore old display mode
		pDC->SelectObject(poldp);
	}

	// temp tag
	if (m_hwndReflect != nullptr && m_tempVTtag != nullptr)
	{
		CPen* poldp =pDC->SelectObject(&m_blackDottedPen);
		int noldROP = pDC->SetROP2(R2_NOTXORPEN);
		pDC->MoveTo(m_tempVTtag->m_pixel,2);
		pDC->LineTo(m_tempVTtag->m_pixel,m_displayRect.bottom-2);
		pDC->SetROP2(noldROP);
		pDC->SelectObject(poldp);
	}
}

void CLineViewWnd::OnSize(UINT nType, int cx, int cy)
{
	CScopeScreen::OnSize(nType, cx, cy);
	if (!IsDefined() || m_pDataFile == nullptr)
		return;

	ResizeChannels(cx-1, 0);
	if (!m_bADbuffers)
	{
		GetDataFromDoc();
	}
}

void CLineViewWnd::Print(CDC* pDC, CRect* pRect, BOOL bCenterLine)
{	
	// save DC & old client rect
	int nSavedDC = pDC->SaveDC();
	ASSERT(nSavedDC != 0);
	CRect oldRect = m_clientRect;

	// prepare DC
	int previousmapping = pDC->SetMapMode(MM_TEXT);	// change map mode to text (1 pixel = 1 logical point)
	m_clientRect = *pRect; //CRect(0,0, pRect->Width(), pRect->Height());
	AdjustDisplayRect(pRect); 
	EraseBkgnd(pDC);
	// clip curves
	if (m_parms.bClipRect)	
		pDC->IntersectClipRect(m_displayRect);
	else
		pDC->SelectClipRgn(nullptr);

	// adjust coordinates for anisotropic mode
	int yVE = -m_displayRect.Height();
	int yVO = m_displayRect.top + m_displayRect.Height()/2;
	int xVE = m_displayRect.Width();
	int xVO = m_displayRect.left;
	
	// exit if no data defined
	if (!IsDefined())
	{
		pDC->TextOut(10, 10, _T("No data"));
		return;
	}

	// change horizontal resolution;
	ResizeChannels(m_displayRect.Width(), m_lxSize);
	if (!bCenterLine)
		GetDataFromDoc();
	else
		GetSmoothDataFromDoc(bCenterLine);

	CEnvelope* pC = m_pEnvelopesArray.GetAt(0);
	pC->FillEnvelopeWithAbcissaEx(xVO, xVE, m_lxSize);

	// display all channels
	long* pPolypoints_X = (long*) &m_PolyPoints[0];
	long* pPolypoints_Y = (long*) &m_PolyPoints[1];
	long* pData=pPolypoints_X;
	int nelements=0;
	CEnvelope* pX = nullptr;
	CEnvelope* pY = nullptr;
	int xextent = m_npixels;
	int yextent=0;
	int yzero=0;
	
	BOOL bPolyLine = (pDC->m_hAttribDC == nullptr) || (pDC->GetDeviceCaps(LINECAPS) && LC_POLYLINE);
	int color = BLACK_COLOR;
	CPen* poldpen=pDC->SelectObject(&m_penTable[color]);

	// display loop:
	int ichan = m_pChanlistItemArray.GetUpperBound();	
	for (ichan; ichan>=0; ichan--)	// scan all channels
	{		
		CChanlistItem* pDL = m_pChanlistItemArray[ichan];
		if (pDL->GetflagPrintVisible() == FALSE)
			continue;

		// display: load abcissa   ----------------------------------------------
		if (pX != pDL->pEnvelopeAbcissa)
		{
			pX = pDL->pEnvelopeAbcissa;					// load pointer to abcissa
			pX->ExportToPolyPts(pPolypoints_X);	// copy abcissa to polypts buffer
			nelements = pX->GetEnvelopeSize();	// update nb of elements
			xextent = pX->GetnElements();		// reset x extent
		}
		// display: load ordinates ---------------------------------------------
		pY = pDL->pEnvelopeOrdinates;				// load pointer to ordinates
		pY->ExportToPolyPts(pPolypoints_Y);	// copy ordinates to polypts buffer

		// change extent, org and color ----------------------------------------
		yextent = pDL->GetYextent();
		yzero   = pDL->GetYzero();
		if (pDL->GetColor() != color)	
		{
			color = pDL->GetColor();			
			pDC->SelectObject(&m_penTable[color]);
		}
		// transform ordinates ------------------------------------------------
		LPPOINT pD = (LPPOINT) pData;			// temporary pointer
		for (int j = 0; j<nelements; j++, pD++)
		{
			short y = (short) pD->y;
			pD->y = MulDiv(y - yzero, yVE, yextent) +yVO;
		}
		//  display points ----------------------------------------------------
		pD = (LPPOINT) pData;		
		if (bPolyLine)
			pDC->Polyline(pD, nelements);		// draw curve
		else
		{
			pDC->MoveTo(*pD);					// move pen to first pair of coords
			for (int j = 0; j<nelements; j++, pD++)
				pDC->LineTo(*pD);				// draw lines
		}

		//display associated cursors ------------------------------------------
		if (GetNHZtags() > 0)					// print HZ cursors if any?
		{
			// select pen and display mode 
			CPen ltgreyPen(PS_SOLID, 0, m_colorTable[SILVER_COLOR]);
			CPen* poldPen = pDC->SelectObject(&ltgreyPen);
			// iterate through HZ cursor list
			int x0 = pRect->left;
			int x1 = pRect->right;			
			for (int j= GetNHZtags()-1; j>= 0; j--)
			{
				if (GetHZtagChan(j) != ichan)	// next tag if not associated with
					continue;					// current channel
				int k = GetHZtagVal(j);			// get value
				k = MulDiv(k-yzero, yVE, yextent) + yVO;
				pDC->MoveTo(x0,k);				// set initial pt
				pDC->LineTo(x1,k);				// HZ line				
			}
			pDC->SelectObject(poldPen);
		}
		// highlight data ------------------------------------------------------
		if (m_pDWintervals != nullptr)
			HighlightData(pDC, ichan);
	}

	// display vertical cursors ------------------------------------------------
	if (GetNVTtags() > 0)
	{			
		// select pen and display mode 
		CPen ltgreyPen(PS_SOLID, 0, m_colorTable[SILVER_COLOR]); 
		CPen* poldp =pDC->SelectObject(&ltgreyPen);
		// iterate through VT cursor list	
		int y0		= pRect->top;
		int y1		= pRect->bottom;
		int k0		= pRect->left;
		int ksize	= pRect->right - k0;
		for (int j=GetNVTtags()-1; j>=0; j--)
		{			
			long lk = GetVTtagLval(j);	// get val
			if (lk <m_lxFirst || lk > m_lxLast)
				continue;
			int k= k0 + (lk-m_lxFirst)* ksize / (m_lxLast - m_lxFirst+1);
			pDC->MoveTo(k,y0);			// set initial pt
			pDC->LineTo(k,y1);			// VT line		
		}		
		pDC->SelectObject(poldp);
	}

	// restore DC ----------------------------------------------------------------
	pDC->SelectObject(poldpen);			// restore old pen
	pDC->RestoreDC(nSavedDC);			// restore DC
	pDC->SetMapMode (previousmapping);	// restore map mode
	m_clientRect = oldRect;
	AdjustDisplayRect(&m_clientRect);
}

BOOL CLineViewWnd::CopyAsText(int ioption, int iunit, int nabcissa)
{	
	// Clean clipboard of contents, and copy the text
	BOOL flag = FALSE;	
	if (OpenClipboard())
	{
		BeginWaitCursor();
		int oldcx = ResizeChannels(nabcissa, 0);	// compute new Envelopes
		GetDataFromDoc();
		EmptyClipboard();							// prepare clipboard
		DWORD dwLen = 32768;	// 32 Kb
		size_t  pcchRemaining = dwLen / sizeof(TCHAR);
		HANDLE hCopy = (HANDLE) ::GlobalAlloc (GHND, dwLen);
		CWaveFormat* pwaveFormat = m_pDataFile->GetpWaveFormat();

		if (hCopy != nullptr)
		{
			// memory allocated -- get pointer to it
			LPTSTR lpCopy = (LPTSTR) ::GlobalLock((HGLOBAL) hCopy);

			// data file name, comment, header
			CString date = (pwaveFormat->acqtime).Format(_T("%#d %B %Y %X")); 
			HRESULT code = StringCchPrintfEx (lpCopy, pcchRemaining, &lpCopy, &pcchRemaining, STRSAFE_NULL_ON_FAILURE, _T("%s\t%s\r\n"), (LPCTSTR) m_pDataFile->GetPathName(), (LPCTSTR) date);
			code = StringCchPrintfEx(lpCopy, pcchRemaining, &lpCopy, &pcchRemaining, STRSAFE_NULL_ON_FAILURE,			_T("%s\t\r\n"), (LPCTSTR) pwaveFormat->GetComments(_T("\t"), 0));
			// time interval
			float tt = GetDataFirst()/ pwaveFormat->chrate;		// first interval (sec)
			code = StringCchPrintfEx(lpCopy, pcchRemaining, &lpCopy, &pcchRemaining, STRSAFE_NULL_ON_FAILURE, _T("time start(s):\t%f\r\n"), tt);
			tt = GetDataLast()/ pwaveFormat->chrate;			// last interval (sec)
			code = StringCchPrintfEx(lpCopy, pcchRemaining, &lpCopy, &pcchRemaining, STRSAFE_NULL_ON_FAILURE, _T("time end(s):\t%f"), tt);
			if (iunit == 0)
			{
				// zero volt for document
				code = StringCchPrintfEx(lpCopy, pcchRemaining, &lpCopy, &pcchRemaining, STRSAFE_NULL_ON_FAILURE, _T("\r\nzero volts(bin):\t%i\r\n"), 0 /*pwaveFormat->binzero*/);
				// unit for each channel
				code = StringCchPrintfEx(lpCopy, pcchRemaining, &lpCopy, &pcchRemaining, STRSAFE_NULL_ON_FAILURE, _T("mvolts per bin:\r\n"));
				for (int i=0; i<GetChanlistSize(); i++) 
					code = StringCchPrintfEx(lpCopy, pcchRemaining, &lpCopy, &pcchRemaining, STRSAFE_NULL_ON_FAILURE, _T("%f\t"), double(GetChanlistVoltsperDataBin(i))*1000.f);
				lpCopy--;	// erase last tab
			}

			// comment for each channel
			code = StringCchPrintfEx(lpCopy, pcchRemaining, &lpCopy, &pcchRemaining, STRSAFE_NULL_ON_FAILURE, _T("\r\nchan title:\r\n"));
			for (int i=0; i<GetChanlistSize(); i++)
				code = StringCchPrintfEx(lpCopy, pcchRemaining, &lpCopy, &pcchRemaining, STRSAFE_NULL_ON_FAILURE, _T("%s\t"), (LPCTSTR) GetChanlistComment(i));
			lpCopy--;	// erase last tab
			code = StringCchPrintfEx(lpCopy, pcchRemaining, &lpCopy, &pcchRemaining, STRSAFE_NULL_ON_FAILURE, _T("\r\n"));
	
			// data
			if (m_pChanlistItemArray.GetSize() < 1)
			{		
				code = StringCchPrintfEx(lpCopy, pcchRemaining, &lpCopy, &pcchRemaining, STRSAFE_NULL_ON_FAILURE, _T("No data to display"));
			}
			else
			{
				switch (ioption)
				{
				case 0:
					GetAsciiEnvelope(lpCopy, iunit);
					break;
				case 1:
					GetAsciiLine(lpCopy, iunit);
					break;
				default:
					GetAsciiEnvelope(lpCopy, iunit);
					break;
				}
			}
			// un-allock array
			::GlobalUnlock((HGLOBAL) hCopy);
			flag = TRUE;		
			SetClipboardData (CF_TEXT, hCopy);
		}
		CloseClipboard();							// close connect w.clipboard		
		ResizeChannels(oldcx, 0);					// restore initial conditions
		GetDataFromDoc();
		EndWaitCursor();
	}
	return flag;
}

LPTSTR CLineViewWnd::GetAsciiEnvelope(LPTSTR lpCopy, int iunit)
{
	// time intervals
	int ichans = m_pChanlistItemArray.GetUpperBound();		
	int npoints = m_pEnvelopesArray.GetAt(0)->GetEnvelopeSize();
	// loop through all points	
	for (int j=0; j<npoints; j++)
	{
		// loop through all channels
		for (int i=0; i<=ichans; i++)	// scan all channels
		{
			CChanlistItem* pDL = m_pChanlistItemArray[i];			
			int k = (pDL->pEnvelopeOrdinates)->GetPointAt(j);
			if (iunit == 1)
			{
				lpCopy += wsprintf(lpCopy, _T("%f\t"), double(k) *GetChanlistVoltsperDataBin(i) *1000.f);
			}
			else
				lpCopy += wsprintf(lpCopy, _T("%i\t"), k);
		}
		lpCopy--;
		*lpCopy =0xD; 		//CR
		*lpCopy ++=0xA; 	//LF
	}

	*lpCopy = _T('\0');
	lpCopy++;	
	return lpCopy;
}

LPTSTR CLineViewWnd::GetAsciiLine(LPTSTR lpCopy, int iunit)
{
	// time intervals
	int ichans = m_pChanlistItemArray.GetUpperBound();		
	int npoints = m_pEnvelopesArray.GetAt(0)->GetEnvelopeSize();
	// loop through all points	
	for (int j=0; j<npoints; j+= m_dataperpixel)
	{
		// loop through all channels
		for (int i=0; i<=ichans; i++)	// scan all channels
		{
			CChanlistItem* pDL = m_pChanlistItemArray[i];			
			int k = (pDL->pEnvelopeOrdinates)->GetPointAt(j);
			if (m_dataperpixel > 1)
			{
				k += (pDL->pEnvelopeOrdinates)->GetPointAt(j+1);
				k = k/2;
			}
			if (iunit == 1)
			{
				lpCopy += wsprintf(lpCopy, _T("%f\t"), double(k)*GetChanlistVoltsperDataBin(i)*1000.f);
			}
			else
				lpCopy += wsprintf(lpCopy, _T("%i\t"), k);			
		}
		lpCopy--;		
		*lpCopy =0xD; 		//CR
		*lpCopy ++=0xA; 	//LF
	}

	*lpCopy = _T('\0');
	lpCopy++;	
	return lpCopy;
}

// XORcurve
// in order to move a curve vertically with the cursor, a special envelope is
// stored within the pData array and displayed using XOR mode.
// this curve has 2 times less points (median) to speed up the display
// Although CClientDC is claimed as attached to the client area of the button
// ("this"), moving the curve along the vertical direction will draw outside of
// the client area, suggesting that it is necessary to clip the client area...
// this code was also tested with m_dibSurf: XOR to DIB surface, then redraw the
// client area by fast BitBlt. This latter method was less efficient (slower)
// than XORing directly to the screen.

void CLineViewWnd::XORcurve()
{		
	// ------- client area (direct draw)
	CDC* pDC = CWnd::GetDC();			// select dc
	int nSavedDC = pDC->SaveDC();		// preserve current settings
	CPen tempPen;
	tempPen.CreatePen(PS_SOLID, 0, m_colorTable[SILVER_COLOR]);	
	CPen* poldpen=pDC->SelectObject(&tempPen); // load pen
	pDC->IntersectClipRect(&m_displayRect);

	pDC->SetMapMode (MM_ANISOTROPIC);	// prepare display with appropriate
	pDC->SetViewportOrg (m_displayRect.left, m_yVO);		// scaling
	pDC->SetViewportExt (Width(), m_yVE);
	pDC->SetWindowExt (m_XORxext, m_XORyext);
	pDC->SetWindowOrg (0, 0);
	
	// display envelope store into pData using XOR mode
	int noldROP = pDC->SetROP2(R2_NOTXORPEN);
	short* pData = (short*) &m_PolyPoints[0];
	//pDC->MoveTo(*pData,*(pData+1));			
	pDC->Polyline((LPPOINT) pData, m_XORnelmts);
	pDC->SetROP2(noldROP);

	pDC->SelectObject(poldpen);			// restore resources - pen
	pDC->RestoreDC(nSavedDC);			// restore DC in previous mode
	ReleaseDC(pDC);						// release DC
	tempPen.DeleteObject();
}

void CLineViewWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	// convert chan values stored within HZ tags into pixels
	if (GetNHZtags() > 0)
	{
		for (int icur = 0; icur<GetNHZtags(); icur++)
		{
			// convert device coordinates into val
			int pixval = GetChanlistBintoPixel(GetHZtagChan(icur), GetHZtagVal(icur));			
			SetHZtagPix(icur, pixval);			// set pixval
		}
	}

	if (GetNVTtags() > 0)
	{
		m_liFirst = m_lxFirst;
		m_liLast = m_lxLast;
	}

	// call base class to test for horiz cursor or XORing rectangle
	CScopeScreen::OnLButtonDown(nFlags, point);		// capture cursor
	
	// if cursor mode = 0 and no tag hit detected, mouse mode=track rect
	// test curve hit -- specific to lineview, if hit curve, track curve instead
	if (m_currCursorMode==0 && m_HCtrapped < 0)	// test if cursor hits a curve
	{
		m_trackMode = TRACK_RECT;
		BOOL bHit = FALSE;
		m_hitcurve = DoesCursorHitCurve(point);
		if (m_hitcurve >= 0)
		{
			// cancel track rect mode (cursor captured)
			m_trackMode = TRACK_CURVE;		// flag: track curve

			// modify polypoint and prepare for XORing curve tracked with mouse			
			CChanlistItem* pDL = m_pChanlistItemArray[m_hitcurve];
			CEnvelope* pX =  pDL->pEnvelopeAbcissa;		// display: load abcissa			
			pX->GetMeantoPolypoints((long*) &m_PolyPoints[0]);
			m_XORnelmts = pX->GetEnvelopeSize() /2;	// nb of elements
			m_XORxext = pX->GetnElements() / 2;		// extent
			
			pX = pDL->pEnvelopeOrdinates;				// load ordinates
			pX->GetMeantoPolypoints((long*) &m_PolyPoints[1]);			
			m_XORyext = pDL->GetYextent();		// store extent
			m_zero = pDL->GetYzero();			// store zero
			m_ptFirst = point;					// save first point
			m_curTrack = m_zero;				// use m_curTrack to store zero

			XORcurve();							// xor curve
			PostMyMessage(HINT_HITCHANNEL, m_hitcurve);	// tell parent chan selected
			return;
		}			
	}

	// if horizontal cursor hit -- specific .. deal with variable gain
	if (m_trackMode == TRACK_HZTAG)
	{		
		CChanlistItem* pDL = m_pChanlistItemArray[GetHZtagChan(m_HCtrapped)];
		m_yWE = pDL->GetYextent();				// store extent
		m_yWO = pDL->GetYzero();					// store zero		
	}
}

void CLineViewWnd::OnMouseMove(UINT nFlags, CPoint point)
{	
	
	switch (m_trackMode)
	{
	case TRACK_CURVE:
		XORcurve();					// erase past curve and compute new zero
		m_zero = MulDiv(point.y-m_ptFirst.y, m_XORyext, -m_yVE) + m_curTrack;
		XORcurve();					// plot new curve
		break;

	default:
		CScopeScreen::OnMouseMove(nFlags, point);
		break;
	}
}

void CLineViewWnd::OnLButtonUp(UINT nFlags, CPoint point)
{
	// liberate cursor
	ReleaseCursor();

	// perform action according to track mode
	switch (m_trackMode)
	{
	// curve was tracked
	case TRACK_CURVE:
		{
		XORcurve();	// (clear) necessary since XORcurve can draw outside client area
		CChanlistItem* pDL = m_pChanlistItemArray[m_hitcurve];
		pDL->SetYzero(m_zero);
		m_trackMode = TRACK_OFF;
		PostMyMessage(HINT_HITCHANNEL, m_hitcurve);	// tell parent chan selected
		Invalidate();
		}
		break;
	// horizontal cursor was tracked
	case TRACK_HZTAG:
		{
		int chan = GetHZtagChan(m_HCtrapped);		// get channel
		int val = GetChanlistPixeltoBin(chan, m_ptLast.y);
		SetHZtagVal(m_HCtrapped, val);				// change cursor value
		point.y = GetChanlistBintoPixel(chan, val);		
		XorHZtag(point.y);
		PostMyMessage(HINT_CHANGEHZTAG, m_HCtrapped);	// tell parent that val changed		
		m_trackMode = TRACK_OFF;
		}
		break;
	// test if vertical tag was tracked
	case TRACK_VTTAG:
		{
		// convert pix into data value and back again
		long lval = ((long)(point.x))*(m_liLast-m_liFirst+1)/((long) m_displayRect.right) + m_liFirst;
		SetVTtagLval(m_HCtrapped, lval);		
		point.x = (int) ((lval-m_liFirst)*((long) m_displayRect.right)/(m_liLast-m_liFirst+1));
		XorVTtag(point.x);
		PostMyMessage(HINT_CHANGEVERTTAG, m_HCtrapped);
		m_trackMode = TRACK_OFF;
		}
		break;
	case TRACK_RECT:
		// mouse was captured: no -> exit!
		{
		// skip too small a rectangle (5 pixels?)
		CRect rectOut(m_ptFirst.x, m_ptFirst.y, m_ptLast.x, m_ptLast.y);		
		const int jitter = 3;		
		BOOL bRectOK = ((abs(rectOut.Height())> jitter) || (abs(rectOut.Width())> jitter));

		// perform action according to cursor type
		CRect rectIn = m_displayRect;
		switch (m_cursorType)
		{
		case 0:		// if no cursor, no curve track, then move display
			if (bRectOK)
			{
				InvertTracker(point);
				rectOut = rectIn;
				rectOut.OffsetRect(m_ptFirst.x-m_ptLast.x, m_ptFirst.y-m_ptLast.y);
				ZoomData(&rectIn, &rectOut);
			}
			break;
		case CURSOR_ZOOM: 	 // zoom operation
			if (bRectOK)
			{
				ZoomData(&rectIn, &rectOut);					
				m_ZoomFrom = rectIn;
				m_ZoomTo   = rectOut;					
				m_iUndoZoom = 1;
			}
			else
				ZoomIn();				
			PostMyMessage(HINT_SETMOUSECURSOR, m_oldcursorType);
			break;
		case CURSOR_MEASURE: // rectangle / measure mode
			PostMyMessage(HINT_DEFINEDRECT, NULL);	// tell parent that val changed
			break;
		default:
			break;
		}
		m_trackMode = TRACK_OFF;
		Invalidate();
		}
		break;
	default:
		break;
	}	
}

int CLineViewWnd::DoesCursorHitCurve(CPoint point)
{
	int chanfound=-1;						// output value
	int ichans = m_pChanlistItemArray.GetUpperBound();
	int npoints = m_pEnvelopesArray.GetAt(0)->GetEnvelopeSize();
	CEnvelope* pC = m_pChanlistItemArray[0]->pEnvelopeAbcissa;
	int xextent = pC->GetnElements();		// n elements stored in one Envelope	
	int index1 = point.x -m_cxjitter;		// horizontal jitter backwards
	int index2 = index1 + m_cxjitter;		// horiz jitter forwards
	if (index1 <0) index1 = 0;				// clip to windows limits
	if (index2 > (Width()-1)) index2 = Width()-1;
	// convert index1 into Envelope indexes
	index1 = index1*m_dataperpixel;		// start from
	index2 = (index2+1)*m_dataperpixel;	// stop at
	// special case when less pt than pixels
	if (index1 == index2)
	{
		index1--;
		if (index1 < 0) index1=0;
		index2++;
		if (index2 > xextent) index2 = xextent;
	}	
		// loop through all channels
	for (int chan=0; chan<=ichans; chan++)	// scan all channels
	{
		// convert device coordinates into val
		int ival = GetChanlistPixeltoBin(chan, point.y);
		int ijitter = MulDiv(m_cyjitter, GetChanlistYextent(chan), -m_yVE);
		int valmax = ival+ijitter;			// mouse max
		int valmin = ival-ijitter;			// mouse min
		pC = m_pChanlistItemArray[chan]->pEnvelopeOrdinates;
			
		// loop around horizontal jitter...
		for (int index=index1; index<index2 && chanfound<0; index++)
		{				
			int kmax = pC->GetPointAt(index);	// get chan Envelope data point
			// special case: one point per pixel
			if (m_dataperpixel == 1)
			{	// more than min AND less than max
				if (kmax >= valmin && kmax <= valmax) 
				{
					chanfound=chan;
					break;
				}
			}
			// second case: 2 pts per pixel - Envelope= max, min
			else
			{
				index++;
				int kmin = pC->GetPointAt(index);	// get min
				if (kmin > kmax)					// ensure that k1=max
				{
					int k = kmax;					// exchange data between
					kmax = kmin;					// kmax and kmin
					kmin = k;						// discard k then
				}
				// test if mouse interval crosses data interval!
				if (!(valmin > kmax || valmax < kmin))
				{
					chanfound=chan;
					break;
				}
			}
		}
	}
	return chanfound;
}

void CLineViewWnd::MoveHZtagtoVal(int i, int val)
{	
	int chan = GetHZtagChan(i);
	CChanlistItem* pDL = m_pChanlistItemArray[chan];
	m_XORyext = pDL->GetYextent();			// store extent
	m_zero = pDL->GetYzero();				// store zero	
	m_ptLast.y = MulDiv(GetHZtagVal(i) - m_zero, m_yVE, m_XORyext) + m_yVO;
	CPoint point;
	point.y = MulDiv(val - m_zero, m_yVE, m_XORyext) + m_yVO;
	XorHZtag(point.y);
	SetHZtagVal(i, val);
}

void CLineViewWnd::SetHighlightData(CDWordArray* pDWintervals)
{
	if (pDWintervals!= nullptr && pDWintervals->GetSize() < 5)
		pDWintervals = nullptr;
	m_pDWintervals = pDWintervals;
}

void CLineViewWnd::HighlightData(CDC* pDC, int chan)
{
	// skip if not the correct chan
	if (chan != (short) m_pDWintervals->GetAt(0))
		return;

	// get color and pen size from array m_pDWintervals
	CPen newPen;
	COLORREF  color = (COLORREF) m_pDWintervals->GetAt(1);
	int pensize = (int) m_pDWintervals->GetAt(2);
	newPen.CreatePen(PS_SOLID, pensize, color);
	CPen* poldpen = (CPen*) pDC->SelectObject(&newPen);
	BOOL bPolyLine = (pDC->m_hAttribDC == nullptr)
		|| (pDC->GetDeviceCaps(LINECAPS) && LC_POLYLINE);

	// loop to display data	

	// pointer to descriptor
	CChanlistItem* pDL = m_pChanlistItemArray[chan];
	long* pPolypoints_X = (long*) &m_PolyPoints[0];	// address first abcissa	
	for (int i=3; i<m_pDWintervals->GetSize(); i++, i++)
	{
		// load ith interval values
		long lFirst = (long) m_pDWintervals->GetAt(i);	// first val
		long lLast  = m_pDWintervals->GetAt(i+1);		// last val
		if (lLast < m_lxFirst || lFirst > m_lxLast)
			continue;							// next if out of range

		// clip data if out of range
		if (lFirst < m_lxFirst)				// minimum interval
			lFirst = m_lxFirst;
		if (lLast > m_lxLast)					// maximum interval
			lLast = m_lxLast;

		// compute corresponding interval (assume same m_scale for all chans... (!!)
		int ifirst = m_scale.GetWhichInterval(lFirst-m_lxFirst);		
		if (ifirst < 0)
			continue;
		int ilast = m_scale.GetWhichInterval(lLast-m_lxFirst)+1;		
		if (ilast < 0) 
			continue;

		if (m_dataperpixel != 1)
		{									// envelope plotting
			ifirst =  ifirst * m_dataperpixel;
			ilast  =  ilast  * m_dataperpixel;
		}
		// display data	
		int nelements = (int) (ilast-ifirst);
		int offset = (int) (ifirst*2);		
		if (bPolyLine)			
			pDC->Polyline((LPPOINT) (pPolypoints_X + offset), nelements);
		else
		{
			pDC->MoveTo(*(pPolypoints_X + offset),*(pPolypoints_X + offset+1));
			LPPOINT pD = (LPPOINT) (pPolypoints_X + offset);
			for (int j = 0; j<nelements; j++, pD++)
				pDC->LineTo(*pD);
		}
	}

	// restore previous pen
	pDC->SelectObject(poldpen);	
}

//---------------------------------------------------------------------------
// ADdisplayStart
//
// prepare a client DC with pen, ext/org, cliprect, prepare abcissa
// init position of next display
// companion routine: ADdisplayStop which closes the client DC
// param:
//	nADchannels		nb of data acquisition channels
//---------------------------------------------------------------------------

void CLineViewWnd::ADdisplayStart(int chsamples)
{
	// init parameters related to AD display
	m_bADbuffers = TRUE;							// yes, display ADbuffers
	m_lADbufferdone = 0;							// length of data already displayed
	CEnvelope* pC = m_pEnvelopesArray.GetAt(0);	
	pC->FillEnvelopeWithAbcissaEx(1, m_displayRect.right-1, chsamples);
	pC->ExportToPolyPts((long*) &m_PolyPoints[0]);
	SetbUseDIB(FALSE); //TRUE);

	// clear window before acquiring data
	CClientDC dc(this);
	EraseBkgnd(&dc);

	// print comment
	dc.SelectObject (GetStockObject (DEFAULT_GUI_FONT));
	CRect rect = m_displayRect;
	rect.DeflateRect(1,1);
	CString cs = _T("Waiting for trigger");
	int textlen = cs.GetLength();
	dc.DrawText(cs, textlen, rect, DT_LEFT); //|DT_WORDBREAK);
}

// ADdisplayBuffer()
//		display nsamples of data, all channels
//
// parameters in:
//		short*	lpBuf ......address of first pt of the buffer
//		long	nsamples ...number of data pt to display / channel
// hidden parameters;
//		long	m_lxSize ........max number of samples (along a line) 
//							(init: OnSize, ResizeChannels)
//		short   m_ADx(m_ADy)VO,VE,WO,WE display parameters (gain, offset)
//							(init: ADdisplayStart)
// 		long	m_lADbufferdone ..index of last pt displayed 
//							(init: last ADdisplayBuffer & ADdisplayStart)
//		CScale	m_scale	........scale long index -> pixel index
//		CObjectArray m_pChanlistItemArray ...array of envelopes to display
//---------------------------------------------------------------------------

void CLineViewWnd::ADdisplayBuffer(short* lpBuf, long nsamples)
{		
	// check data wrap	
	// deal with overlapping data (this should never occur if sweep size defined
	// as a multiple of the nb of buffers)
	if (m_lADbufferdone+nsamples > m_lxSize)
	{
		long ndata = m_lxSize - m_lADbufferdone;
		if (ndata > 0)
		{
			ADdisplayBuffer(lpBuf, ndata);			// display end
			nsamples -= ndata;						// reduce nb of data to display
			lpBuf += (ndata* (long) m_pDataFile->GetpWaveFormat()->scan_count);
		}
		m_lADbufferdone = 0;						// clear len of data
	}

	// create device context and prepare bitmap to receive drawing commands
	CClientDC dc(this);						// get window DC
	CDC dcMem;								// prepare device context
	dcMem.CreateCompatibleDC(&dc);			// create compatible DC
	CBitmap bitmapPlot;
	CBitmap* poldbitmap = dcMem.SelectObject(&bitmapPlot);
	int isaveDC = dcMem.SaveDC();			// save DC
	CDC* pDC = &dc;
	if (m_bUseDIB)
		pDC = &dcMem;

	// get first and last pixels of the interval to display
	int	iADpixelfirst = MulDiv(m_lADbufferdone, m_npixels, m_lxSize);
	int	iADpixellast = MulDiv(m_lADbufferdone+nsamples-1, m_npixels, m_lxSize);
	if (iADpixellast > m_displayRect.right-2)
		iADpixellast = m_displayRect.right-2;

	int	npixels = iADpixellast - iADpixelfirst+1;
	int npoints = npixels * m_dataperpixel;

	//////////////////////////// prepare display ///////////////////////////////////

	POINT* pPolypoints_X = (POINT*) &m_PolyPoints[iADpixelfirst * m_dataperpixel*2];	
	//dcMem.SetMapMode (MM_TEXT);
	//dcMem.SetViewportOrg (0, 0);
	//dcMem.SetWindowOrg (0, 0);

	CRect rect(iADpixelfirst, m_displayRect.top, iADpixellast, m_displayRect.bottom);
	if (iADpixelfirst == 0)
		rect.left = 1;
	rect.DeflateRect(0, 1);
	pDC->IntersectClipRect(rect);
	pDC->FillSolidRect(&rect, m_parms.crScopeFill);

	CPen *ppenOld = (CPen*) pDC->SelectStockObject(BLACK_PEN);
	CChanlistItem* pDL = m_pChanlistItemArray[0];
	pDC->SetMapMode (MM_ANISOTROPIC);		// display in anisotropic mode
	pDC->SetViewportExt (m_xVE, m_yVE);
	pDC->SetViewportOrg (m_xVO, m_yVO);
	pDC->SetWindowExt (m_npixels, m_yVE);	//pDL->GetYextent());
	pDC->SetWindowOrg (0, 0);				//pDL->GetYzero());
	int yVE = m_yVE;

	for (int ichan = 0; ichan<m_pChanlistItemArray.GetSize(); ichan++)
	{
		// load channel descriptors
		pDL = m_pChanlistItemArray[ichan];
		CPen tempPen;
		tempPen.CreatePen(PS_SOLID, 0, m_colorTable[pDL->GetColor()]);	
		pDC->SelectObject(&tempPen);
		
		// compute max min and plot array
		POINT* ppt = pPolypoints_X;
		short* pData = lpBuf + ichan;
		int worg=pDL->GetYzero(); 
		int wext=pDL->GetYextent();

		// only one data point per pixel
		if (m_dataperpixel == 1)
		{
			int nDOCchans = (long) m_pDataFile->GetpWaveFormat()->scan_count;
			for (int i=0; i<npixels; i++, ppt++, pData += nDOCchans)
				ppt->y = MulDiv(*pData-worg, yVE, wext);
		}

		// at least 2 data points per pixel		
		 else
		{
			int nelmts0 =  0;
			int nDOCchans = (long) m_pDataFile->GetpWaveFormat()->scan_count;
			for (int i=0; i<npixels; i++)
			{
				short	iMax= *pData;			// init max
				short	iMin= iMax;				// init min
				int nelmts = ((i+1)* nsamples/ npixels)-nelmts0;
				nelmts0 += nelmts;
				while (nelmts > 0)			// scan nb elements
				{							// designed by scale
					short idata = *pData;		// load value
					if (idata < iMin)
						iMin = idata;		// change min
					else if (idata > iMax)  
						iMax = idata;		// change max
					pData += nDOCchans;	// update data pointer 
					nelmts--;
				}

				ppt->y = MulDiv(iMax-worg, yVE, wext);
				ppt++;		// update Envelope pointer
				ppt->y = MulDiv(iMin-worg, yVE, wext);
				ppt++;		// update Envelope pointer
			}
		}
		pDC->MoveTo(pPolypoints_X->x -2, pPolypoints_X->y);
		pDC->PolylineTo(pPolypoints_X, npoints);
		tempPen.DeleteObject();	
	}

	// restore the old pen and exit	
	pDC->SelectObject(ppenOld);	// select initial object
	pDC->RestoreDC(isaveDC);		// restore DC
	pDC->SetMapMode (MM_TEXT);
	pDC->SetViewportOrg (0, 0);
	pDC->SetWindowOrg (0, 0);

	if (m_bUseDIB)
		dc.BitBlt(rect.left, rect.top,
			rect.Width(), rect.Height(),
			&dcMem,
			rect.left, rect.top,
			SRCCOPY);

	dcMem.SelectObject(poldbitmap);	// release bitmap
	m_lADbufferdone +=nsamples;	
}

void CLineViewWnd::Serialize( CArchive& ar )
{
	CScopeScreen::Serialize(ar);
	m_PolyPoints.Serialize(ar);
	m_scale.Serialize(ar);

	if (ar.IsStoring())
	{
		ar << m_dataperpixel;		// nb of data point per pixel
		ar << m_lxVeryLast;			// end of document
		ar << m_lxPage;				// size of page increment / file index
		ar << m_lxLine;				// size of line increment / file index
		ar << m_lxSize;				// nb of data pts represented in a Envelope
		ar << m_lxFirst;			// file index of 1rst pt in the Envelopes
		ar << m_lxLast;				// file index of last pt in the Envelopes
		ar << m_npixels;			// nb pixels displayed horizontally	

		int nenvelopes = m_pEnvelopesArray.GetSize();
		ar << nenvelopes;
		int nchanlistItems = m_pChanlistItemArray.GetSize();
		ar << nchanlistItems;
		
		for (int i=0; i< nenvelopes; i++)
			m_pEnvelopesArray[i]->Serialize(ar);

		for (int i=0; i< nchanlistItems; i++)
			m_pChanlistItemArray[i]->Serialize(ar);
	} 
	else
	{		
		ar >> m_dataperpixel;		// nb of data point per pixel
		ar >> m_lxVeryLast;			// end of document
		ar >> m_lxPage;				// size of page increment / file index
		ar >> m_lxLine;				// size of line increment / file index
		ar >> m_lxSize;				// nb of data pts represented in a Envelope
		ar >> m_lxFirst;			// file index of 1rst pt in the Envelopes
		ar >> m_lxLast;				// file index of last pt in the Envelopes
		ar >> m_npixels;			// nb pixels displayed horizontally	

		int nenvelopes;
		ar >> nenvelopes;
		int nchanlistItems;
		ar >> nchanlistItems;		

		// CEnvelope array
		if (m_pEnvelopesArray.GetSize() > nenvelopes)
		{			
			for (int i=m_pEnvelopesArray.GetUpperBound(); i>= nenvelopes; i--)
				delete m_pEnvelopesArray[i];				
			m_pEnvelopesArray.SetSize(nenvelopes);
		}
		else if (m_pEnvelopesArray.GetSize() < nenvelopes)
		{
			int nenvelope0 = m_pEnvelopesArray.GetSize();
			m_pEnvelopesArray.SetSize(nenvelopes);
			for (int i=nenvelope0; i < nenvelopes; i++)
				m_pEnvelopesArray[i] = new CEnvelope; // (CEnvelope*)
		}
		for (int i=0; i< nenvelopes; i++)
			m_pEnvelopesArray[i]->Serialize(ar);

		// ChanList array
		if (m_pChanlistItemArray.GetSize() > nchanlistItems)
		{
			for (int i=m_pChanlistItemArray.GetUpperBound(); i>= nchanlistItems; i--)
				delete m_pChanlistItemArray[i];
			m_pChanlistItemArray.SetSize(nchanlistItems);
			
		}
		else if (m_pChanlistItemArray.GetSize() < nchanlistItems)
		{
			int nchanlistItems0 = m_pChanlistItemArray.GetSize();
			m_pChanlistItemArray.SetSize(nchanlistItems);
			for (int i=nchanlistItems0; i < nchanlistItems; i++)
				m_pChanlistItemArray[i] = new CChanlistItem; // (CChanlistItem*) 
		}
		int ix = 0;
		int iy = 0;
		for (int i=0; i< nchanlistItems; i++)
		{
			m_pChanlistItemArray[i]->Serialize(ar);
			m_pChanlistItemArray[i]->GetEnvelopeArrayIndexes(ix, iy);
			m_pChanlistItemArray[i]->SetEnvelopeArrays(m_pEnvelopesArray.GetAt(ix), ix, m_pEnvelopesArray.GetAt(iy), iy);
		}
	}	

	/*
	CDWordArray*	m_pDWintervals;	// intervals of data that are highlighted
	CDataFile*		m_pDataFile;
	*/
}

void CLineViewWnd::SetTrackSpike(BOOL btrackspike, int tracklen, int trackoffset, int trackchannel)
{
	m_btrackspike = btrackspike;
	m_tracklen = tracklen;
	m_trackoffset = trackoffset;
	m_trackchannel = trackchannel;
}