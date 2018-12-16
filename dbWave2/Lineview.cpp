
#include "StdAfx.h"
#include "dbWave_constants.h"
#include <stdlib.h>
#include <Strsafe.h>
#include "resource.h"
#include "Lineview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

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

	m_csEmpty = _T("no data");
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
	for (auto i=envelope_ptr_array.GetUpperBound(); i>= 0; i--)
		delete envelope_ptr_array[i];	
	envelope_ptr_array.RemoveAll();

	// suppress array and objects pointed by m_pChanlistItemArray
	for (auto i=chanlistitem_ptr_array.GetUpperBound(); i>= 0; i--)
		delete chanlistitem_ptr_array[i];
	chanlistitem_ptr_array.RemoveAll();	
}

int CLineViewWnd::AddChanlistItem(int ns, int mode)
{ 
	// first time??	create Envelope(0) with abcissa series
	if (chanlistitem_ptr_array.GetSize() == 0)
	{
		m_PolyPoints.SetSize(m_npixels*4);		// set size of polypoint array
		m_scale.SetScale(m_npixels, m_lxSize);	// compute scale (this is first time)
		m_dataperpixel = 2;
		auto* p_envelope = new CEnvelope(m_npixels*m_dataperpixel, m_dataperpixel, 0, -1, 0);
		ASSERT(p_envelope != NULL);
		envelope_ptr_array.Add(p_envelope);					// add item Ptr to array	
		p_envelope->FillEnvelopeWithAbcissa(m_npixels*m_dataperpixel, m_lxSize);
	}

	// create new Envelope and store pointer into Envelopeslist
	auto span=0;
	if(m_pDataFile != nullptr)
		span = m_pDataFile->GetTransfDataSpan(mode);
	auto* p_envelope_y = new CEnvelope(m_npixels, m_dataperpixel, ns, mode, span);
	ASSERT(p_envelope_y != NULL);
	const auto j = envelope_ptr_array.Add(p_envelope_y);
	
	// create new chanlistitem with x=Envelope(0) and y=the new Envelope
	auto* p_chan_list_item = new CChanlistItem(envelope_ptr_array.GetAt(0), 0, p_envelope_y, j);
	ASSERT(p_chan_list_item != NULL);
	const auto index_newchan = chanlistitem_ptr_array.Add(p_chan_list_item);

	// init display parameters
	p_chan_list_item->InitDisplayParms(1, RGB(0,0,0), 2048, 4096);

	if (m_pDataFile != nullptr)
	{
		float voltsperb;
		m_pDataFile->GetWBVoltsperBin(ns, &voltsperb, mode);
		const auto pchan_array = m_pDataFile->GetpWavechanArray();
		const auto pwave_format = m_pDataFile->GetpWaveFormat();
		p_chan_list_item->SetDataBinFormat(pwave_format->binzero, pwave_format->binspan);
		p_chan_list_item->SetDataVoltsFormat(voltsperb, pwave_format->fullscale_Volts);
		if (ns >= pchan_array->channel_get_number())
			ns = 0;
		const auto pchan = pchan_array->get_p_channel(ns);
		p_chan_list_item->dl_comment = pchan->am_csComment;	// get comment however
		UpdateChanlistMaxSpan();				// update span max
		if (mode > 0)							// modif comment if transform buffer
			p_chan_list_item->dl_comment = (m_pDataFile->GetTransfDataName(mode)).Left(8) + ": " + p_chan_list_item->dl_comment;
	}
	return index_newchan;
}

int CLineViewWnd::RemoveChanlistItem(WORD i)
{
	const auto chanlist_size = chanlistitem_ptr_array.GetSize();	// get size of chan array
	if (chanlist_size >0)	// delete Envelopes ordinates but make sure that it is not used
	{
		const auto pa = chanlistitem_ptr_array[i]->pEnvelopeOrdinates;
		// step 1: check that this envelope is not used by another channel
		auto b_used_only_once = TRUE;
		for (auto lj= chanlist_size; lj>=0; lj--)
		{
			const auto p_envelope_y = chanlistitem_ptr_array[i]->pEnvelopeOrdinates;
			if (pa == p_envelope_y && lj != i)
			{
				b_used_only_once = FALSE;	// the envelope is used by another channel
				break;					// stop search and exit loop
			}
		}
		// step 2: delete corresponding envelope only if envelope used only once.
		if (b_used_only_once)
		{
			for (auto k=envelope_ptr_array.GetUpperBound(); k>= 0; k--)
			{
				const auto pb = envelope_ptr_array[k];
				if (pa == pb)	// search where this Envelope was stored
				{
					delete pa;						// delete  object
					envelope_ptr_array.RemoveAt(k);	// remove pointer from array
					break;		// object is found, stop loop and delete chanlist item
				}
			}
		}
		// step 3: delete channel
		delete chanlistitem_ptr_array[i];
		chanlistitem_ptr_array.RemoveAt(i);		
	}
	UpdateChanlistMaxSpan();
	return chanlist_size-1;
}

void CLineViewWnd::UpdateChanlistMaxSpan()
{
	if (envelope_ptr_array.GetSize() <= 1)
		return;
	// get spanmax stored in Envelope(0)
	auto imax = 0;
	for (auto i = envelope_ptr_array.GetUpperBound(); i>0; i--)
	{
		const auto j = envelope_ptr_array[i]->GetDocbufferSpan();
		if (j > imax) 
			imax = j;
	}
	 // store imax
	envelope_ptr_array[0]->SetDocbufferSpan(imax);	//store max
}

void CLineViewWnd::UpdateChanlistFromDoc()
{
	for (auto i = chanlistitem_ptr_array.GetUpperBound(); i>= 0; i--)
	{
		auto p_chanlist_item =  chanlistitem_ptr_array[i];
		auto p_ord = p_chanlist_item->pEnvelopeOrdinates;
		const auto ns = p_ord->GetSourceChan();
		const auto mode = p_ord->GetSourceMode();
		p_ord->SetDocbufferSpan(m_pDataFile->GetTransfDataSpan(mode));
		const auto pchan_array = m_pDataFile->GetpWavechanArray();
		const auto pchan = pchan_array->get_p_channel(ns);
		p_chanlist_item->dl_comment= pchan->am_csComment;
		if (mode > 0)
			p_chanlist_item->dl_comment = (m_pDataFile->GetTransfDataName(mode)).Left(6) + ": " + p_chanlist_item->dl_comment;
		UpdateGainSettings(i);	// keep physical value of yextent and zero constant
	}
	UpdateChanlistMaxSpan();	// update max span
}

void CLineViewWnd::UpdateGainSettings(int i)
{
	auto p_chanlist_item =  chanlistitem_ptr_array[i];
	const auto p_ord = p_chanlist_item->pEnvelopeOrdinates;
	const auto ns = p_ord->GetSourceChan();
	const auto mode = p_ord->GetSourceMode();
	float doc_volts_per_bin;
	m_pDataFile->GetWBVoltsperBin(ns, &doc_volts_per_bin, mode);
	const auto volts_per_data_bin = p_chanlist_item->GetVoltsperDataBin();
	const auto pwave_format = m_pDataFile->GetpWaveFormat();
	if (doc_volts_per_bin != volts_per_data_bin)
	{
		p_chanlist_item->SetDataBinFormat(pwave_format->binzero, pwave_format->binspan);
		p_chanlist_item->SetDataVoltsFormat(doc_volts_per_bin, pwave_format->fullscale_Volts);
		auto iextent = p_chanlist_item->GetYextent();
		iextent = int(float(iextent) / doc_volts_per_bin * volts_per_data_bin);
		p_chanlist_item->SetYextent(iextent);
	}
}

int CLineViewWnd::SetChanlistSourceChan(WORD i, int ns)
{
	// check if channel is allowed
	const auto pwave_format = m_pDataFile->GetpWaveFormat();
	if (pwave_format->scan_count <= ns || ns < 0)
		return -1;

	// make sure we have enough data channels...
	if (GetChanlistSize() <= i)
		for (auto j= GetChanlistSize(); j<= i; j++)
			AddChanlistItem(j, 0);
	
	// change channel
	auto p_chanlist_item =  chanlistitem_ptr_array[i];
	auto p_ord = p_chanlist_item->pEnvelopeOrdinates;
	p_ord->SetSourceChan(ns);			// change data channel
	const auto mode = p_ord->GetSourceMode();	// get transform mode
	// modify comment
	const auto pchan_array = m_pDataFile->GetpWavechanArray();
	const auto pchan = pchan_array->get_p_channel(ns);
	p_chanlist_item->dl_comment= pchan->am_csComment;		
	if (mode > 0)
		p_chanlist_item->dl_comment = (m_pDataFile->GetTransfDataName(mode)).Left(6) + _T(": ") + p_chanlist_item->dl_comment;
	UpdateGainSettings(i);				// keep physical value of yextent and zero constant
	return ns;
}

void CLineViewWnd::SetChanlistOrdinates(WORD i, int chan, int transform)
{
	// change channel
	auto chanlist_item =  chanlistitem_ptr_array[i];	
	chanlist_item->SetOrdinatesSourceData(chan, transform);
	// modify comment
	const auto pchanArray = m_pDataFile->GetpWavechanArray();
	if (chan >= pchanArray->channel_get_number())
			chan = 0;
	const auto pchan = pchanArray->get_p_channel(chan);
	chanlist_item->dl_comment= pchan->am_csComment;		
	if (transform > 0)
		chanlist_item->dl_comment = (m_pDataFile->GetTransfDataName(transform)).Left(6) + _T(": ") + chanlist_item->dl_comment;
}

void CLineViewWnd::SetChanlistVoltsExtent(const int chan, const float* pvalue)
{
	auto ichanfirst = chan;
	auto ichanlast = chan;
	if (chan < 0) 
	{
		ichanfirst = 0;
		ichanlast = chanlistitem_ptr_array.GetUpperBound();
	}
	auto voltsextent = 0.f;
	if (pvalue != nullptr)
		voltsextent = *pvalue;
	for (auto i= ichanfirst; i<= ichanlast; i++)
	{
		auto p_chanlist_item =  chanlistitem_ptr_array[i];
		const auto yvoltsperbin = p_chanlist_item->GetVoltsperDataBin();
		if (pvalue == nullptr) 
			voltsextent = yvoltsperbin * p_chanlist_item->GetYextent();

		const auto yextent = int(voltsextent / yvoltsperbin);
		p_chanlist_item->SetYextent(yextent);
	}
}

void CLineViewWnd::SetChanlistVoltsZero(const int chan, const float* pvalue)
{
	auto ichanfirst = chan;
	auto ichanlast = chan;
	if (chan < 0)
	{
		ichanfirst = 0;
		ichanlast = chanlistitem_ptr_array.GetUpperBound();
	}

	auto voltsextent = 0.f;
	if (pvalue != nullptr)
		voltsextent = *pvalue;
	for (auto i = ichanfirst; i <= ichanlast; i++)
	{
		auto p_chanlist_item = chanlistitem_ptr_array[i];
		const auto yvoltsperbin = p_chanlist_item->GetVoltsperDataBin();
		if (pvalue == nullptr)
			voltsextent = yvoltsperbin * p_chanlist_item->GetDataBinZero();

		const auto iyzero = int(voltsextent / yvoltsperbin);
		p_chanlist_item->SetYzero(iyzero);
	}
}

int CLineViewWnd::SetChanlistTransformMode(WORD i, int imode)
{
	// check if transform is allowed
	if (!m_pDataFile->IsWBTransformAllowed(imode) ||	// ? is transform allowed
		!m_pDataFile->InitWBTransformBuffer()) 		// ? is init OK
	{
		AfxMessageBox(IDS_LNVERR02, MB_OK);
		return -1;
	}

	// change transform mode
	auto p_chanlist_item =  chanlistitem_ptr_array[i];
	auto p_ord = p_chanlist_item->pEnvelopeOrdinates;
	const auto ns = p_ord->GetSourceChan();
	// change transform
	p_ord->SetSourceMode(imode, m_pDataFile->GetTransfDataSpan(imode));

	// modify comment
	const auto pchan_array = m_pDataFile->GetpWavechanArray();
	const auto pchan = pchan_array->get_p_channel(ns);
	p_chanlist_item->dl_comment= pchan->am_csComment;		
	if (imode > 0)
		p_chanlist_item->dl_comment = (m_pDataFile->GetTransfDataName(imode)).Left(8) 
							+ _T(": ") + p_chanlist_item->dl_comment;
	UpdateGainSettings(i);	// keep physical value of yextent and zero constant
	UpdateChanlistMaxSpan();
	return imode;
}

void CLineViewWnd::AutoZoomChan(int j)
{
	auto i1 = j;
	auto i2 = j;
	if (j <0)
	{
		i1 = 0;
		i2 = GetChanlistSize()-1;
	}
	
	int max, min;
	for (auto i=i1; i<= i2; i++)
	{
		GetChanlistMaxMin(i, &max, &min);
		SetChanlistYzero(i, (max+min)/2);
		SetChanlistYextent(i, MulDiv(max-min+1, 10, 8));
	}
}

void CLineViewWnd::SplitChans()
{
	int max, min;
	const auto nchans = GetChanlistSize();
	auto icur = nchans-1;
	for (auto i=0; i< nchans; i++, icur -=2)
	{
		GetChanlistMaxMin(i, &max, &min);
		const auto amplitudespan = MulDiv((max-min), 12*nchans, 10);
		SetChanlistYextent(i, amplitudespan);
		const auto ioffset = (max+min)/2 + MulDiv(amplitudespan, icur, nchans*2);
		SetChanlistYzero(i, ioffset);
	}
}

void CLineViewWnd::CenterChan(int j)
{
	auto i1 = j;
	auto i2 = j;
	if (j <0)
	{
		i1 = 0;
		i2 = GetChanlistSize()-1;
	}

	int max, min;	
	for (auto i=i1; i<= i2; i++)
	{
		GetChanlistMaxMin(i, &max, &min);
		const auto yzero = (max+min)/2;
		SetChanlistYzero(i, yzero);
	}
}

void CLineViewWnd::MaxgainChan(int j)
{
	auto i1 = j;
	auto i2 = j;
	if (j <0)
	{
		i1 = 0;
		i2 = GetChanlistSize()-1;
	}

	int max, min;	
	for (auto i=i1; i<= i2; i++)
	{
		GetChanlistMaxMin(i, &max, &min);	
		const auto yextent = MulDiv(max-min+1, 10, 8);
		SetChanlistYextent(i, yextent);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CLineViewWnd message handlers
/////////////////////////////////////////////////////////////////////////////

int CLineViewWnd::ResizeChannels(const int npixels, const long l_size)
{
	const auto old_npixels = m_npixels;

	// trap dummy values and return if no operation is necessary
	if (npixels !=0)				// horizontal resolution
		m_npixels = npixels;
	if (l_size != 0)					// size of data to display
		m_lxSize = l_size;
	
	// change horizontal resolution	: m_Polypoints receives abcissa and ordinates
	// make provision for max and min points
	if (m_PolyPoints.GetSize() != m_npixels*4)	
		m_PolyPoints.SetSize(m_npixels*4);

	// compute new scale and change size of Envelopes
	m_scale.SetScale(m_npixels, m_lxSize);
	m_dataperpixel = 2;

	// change Envelopes size
	auto npts = m_npixels;			// set Envelope size to the nb of pixels
	if (m_npixels > m_lxSize)
		npts = int(m_lxSize);		// except when there is only one data pt
	ASSERT(npts > 0);

	const auto n_envelopes = envelope_ptr_array.GetSize();// loop through all Envelopes
	if (n_envelopes > 0)
	{
		CEnvelope* p_envelope;
		for (auto i_envelope=0; i_envelope< n_envelopes; i_envelope++)
		{
			p_envelope = envelope_ptr_array.GetAt(i_envelope);
			p_envelope->SetEnvelopeSize(npts, m_dataperpixel);
		}
		p_envelope = envelope_ptr_array.GetAt(0);	
		p_envelope->FillEnvelopeWithAbcissa(m_npixels, m_lxSize);// store data series
	}

	// read data and update page and line sizes / file browse    
	UpdatePageLineSize();
	m_lxLast = m_lxFirst + m_lxSize-1;	
	return old_npixels;
}

BOOL CLineViewWnd::AttachDataFile(CAcqDataDoc* p_data_file, long l_size)
{
	m_pDataFile = p_data_file;
	m_samplingrate = m_pDataFile->GetpWaveFormat()->chrate;
	m_pDataFile->SetReadingBufferDirty();
	ASSERT(m_pDataFile->GetDOCchanLength() > 0);
	if (l_size <= 0)
	{
		l_size = GetDataSize();
		if (l_size > m_pDataFile->GetDOCchanLength())	// check
			l_size = m_pDataFile->GetDOCchanLength();	
	}

	// init parameters used to display Envelopes
	const int l_very_last = m_lxVeryLast;
	m_lxVeryLast = m_pDataFile->GetDOCchanLength()-1;
	m_lxFirst = 0;
	m_lxLast = l_size -1;
	if (m_lxSize !=l_size || l_very_last != m_lxVeryLast)
	{
		m_lxSize =l_size;
		m_dataperpixel = m_scale.SetScale(m_npixels, m_lxSize);
		m_dataperpixel = 2;
	}

	//Remove irrelevant Chanlist items;	
	const auto docchanmax = m_pDataFile->GetpWaveFormat()->scan_count - 1;
	const auto chanlistmax = chanlistitem_ptr_array.GetUpperBound();
	for (auto i=chanlistmax; i>=0; i--)
	{
		if (GetChanlistSourceChan(i) > docchanmax)
			RemoveChanlistItem(i);
	}

	// Remove irrelevant Envelopes();
	const auto n_envelopes = envelope_ptr_array.GetUpperBound();
	for (auto i=n_envelopes; i> 0; i--) // ! Envelope(0)=abcissa
	{
		const auto p_envelope = envelope_ptr_array.GetAt(i);
		if (p_envelope->GetSourceChan() > docchanmax)
		{
			delete p_envelope; 
			envelope_ptr_array.RemoveAt(i);
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

	// max nb of points spanning around raw data pt stored in array(0)
	if (envelope_ptr_array.GetSize() < 1)
		return FALSE;
	auto p_cont = envelope_ptr_array.GetAt(0);
	const auto nspan = p_cont->GetDocbufferSpan();		// additional pts necessary

	// loop through all pixels if data buffer is longer than data displayed 
	// within one pixel...
	auto l_first = m_lxFirst;					// start
	const auto ipixelmax = m_scale.GetnIntervals();	// max pixel
	const auto nchans = m_pDataFile->GetScanCount();

	for (auto ipixel = 0; ipixel < ipixelmax; ipixel++)
	{
		// compute file index of pts within current pixel
		const auto data_within_1_pixel = m_scale.GetIntervalSize(ipixel); // size first pixel
		const int l_last = l_first + data_within_1_pixel- 1;
		auto b_new = TRUE;						// flag to tell routine that it should start from new data
	
		while (l_first <= l_last)
		{
			auto lBUFchanFirst= l_first;			// index very first pt within pixel
			long lBUFchanLast = l_last;			// index very last pixel

			 // ask document to read raw data, document returns index of data loaded within the buffer
			if (!m_pDataFile->LoadRawData(&lBUFchanFirst, &lBUFchanLast, nspan))
				break;							// exit if error reported

			// build Envelopes  .................
			if (lBUFchanLast > l_last)
				lBUFchanLast = l_last;
			const int npoints = lBUFchanLast - l_first +1; 
			if (npoints <= 0)
				break;

			// loop over each envelope
			for (auto i_envelope=envelope_ptr_array.GetUpperBound(); i_envelope>0; i_envelope--)
			{
				p_cont = envelope_ptr_array.GetAt(i_envelope);

				const auto source_chan = p_cont->GetSourceChan();// get source channel
				const auto itransf =  p_cont->GetSourceMode();	// get transform mode
				if (itransf>0)							// if transformation, compute transf
				{										// and then build envelope
					const auto lp_data = m_pDataFile->LoadTransfData(l_first, lBUFchanLast, itransf, source_chan);
					p_cont->FillEnvelopeWithMxMi(ipixel, lp_data, 1, npoints, b_new);
				}
				else									// no transformation: compute max min
				{										// and then build envelope
					const auto lp_data = m_pDataFile->GetpRawDataElmt(source_chan, l_first);
					p_cont->FillEnvelopeWithMxMi(ipixel, lp_data, nchans, npoints, b_new);
				}
			}
			b_new = FALSE;
			l_first = lBUFchanLast + 1;
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
	const auto nchans = m_pDataFile->GetScanCount();									// n raw channels
	short* lp_data;								// pointer used later
	// max nb of points spanning around raw data pt stored in array(0)
	auto p_cont = envelope_ptr_array.GetAt(0);
	const auto nspan = p_cont->GetDocbufferSpan();		// additional pts necessary

	// loop through all pixels if data buffer is longer than data displayed 
	// within one pixel...
	auto l_first = m_lxFirst;					// start
	const auto ipixelmax = m_scale.GetnIntervals();	// max pixel

	for (auto ipixel = 0; ipixel < ipixelmax; ipixel++)
	{
		// compute file index of pts within current pixel
		const auto data_within_1_pixel = m_scale.GetIntervalSize(ipixel); // size first pixel
		const auto l_last = l_first + data_within_1_pixel- 1;
		auto b_new = TRUE;						// flag to tell routine that it should start from new data

		while (l_first <= l_last)
		{
			auto lBUFchanFirst= l_first;			// index very first pt within pixel
			auto lBUFchanLast = l_last;			// index very last pixel

			 // ask document to read raw data, document returns index of data loaded within the buffer
			if (!m_pDataFile->LoadRawData(&lBUFchanFirst, &lBUFchanLast, nspan))
				break;							// exit if error reported

			// build Envelopes  .................
			if (lBUFchanLast > l_last)
				lBUFchanLast = l_last;
			const int npoints = lBUFchanLast - l_first +1; 
			if (npoints <= 0)
				break;

			// loop over each envelope
			for (auto i_envelope=envelope_ptr_array.GetUpperBound(); i_envelope>0; i_envelope--)
			{
				p_cont = envelope_ptr_array.GetAt(i_envelope);

				const auto source_chan = p_cont->GetSourceChan();// get source channel
				const auto itransf =  p_cont->GetSourceMode();	// get transform mode
				if (itransf>0)							// if transformation, compute transf
				{										// and then build envelope
					lp_data = m_pDataFile->LoadTransfData(l_first, lBUFchanLast, itransf, source_chan);
					p_cont->FillEnvelopeWithSmoothMxMi(ipixel, lp_data, 1, npoints, b_new, ioption);
				}
				else									// no transformation: compute max min
				{										// and then build envelope
					lp_data = m_pDataFile->GetpRawDataElmt(source_chan, l_first);
					p_cont->FillEnvelopeWithSmoothMxMi(ipixel, lp_data, nchans, npoints, b_new, ioption);
				}
			}
			b_new = FALSE;
			l_first = lBUFchanLast + 1;
		}	
	}
	return TRUE;
}

BOOL CLineViewWnd::GetDataFromDoc(long l_first)
{	
	if (l_first == m_lxFirst)
		return TRUE;
	m_lxFirst = l_first;
	return GetDataFromDoc();	
}

BOOL CLineViewWnd::GetDataFromDoc(long l_first, long l_last)
{
	// check if size has changed
	if (l_first <0) 				// first check limits across document's
		l_first = 0;				// minimum
	if (l_last > m_lxVeryLast)	// maximum
		l_last = m_lxVeryLast;    
	if ((l_first > m_lxVeryLast) || (l_last < l_first))
	{							// shuffled intervals
		l_first = 0;
		if (m_lxSize > m_lxVeryLast+1)
			m_lxSize =	m_lxVeryLast+1;
		l_last = l_first + m_lxSize -1;
	}

	// requested size different from current one?
	if (m_lxSize != (l_last - l_first+1))
	{		
		m_lxFirst = l_first;
		ResizeChannels(0, (l_last - l_first+1));		// n pixels = m_npixels
	}
	// load data
	m_lxFirst = l_first;
	m_lxLast = l_last;
	return GetDataFromDoc();
}

BOOL CLineViewWnd::ScrollDataFromDoc(WORD nSBCode)
{
	auto l_first=m_lxFirst;	
	switch (nSBCode)
		{
		case SB_LEFT:		// Scroll to far left.
			l_first = 0;	
			break;
		case SB_LINELEFT:	// Scroll left.
			l_first -= m_lxLine;
			break;
		case SB_LINERIGHT:	// Scroll right.
			l_first += m_lxLine;
			break;
		case SB_PAGELEFT:	// Scroll one page left.
			l_first -= m_lxPage;
			break;
		case SB_PAGERIGHT:	// Scroll one page right.
			l_first += m_lxPage;
			break;
		case SB_RIGHT:		// Scroll to far right.
			l_first = m_lxVeryLast - m_lxSize+1;
			break;
		default:
			return FALSE;
		}	
	m_lxFirst = l_first;
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

	// change gain & offset of all channels:
	for (auto i = chanlistitem_ptr_array.GetUpperBound(); i>=0; i--)	// scan all channels
	{
		// display loop: load abcissa
		auto chanlist_item = chanlistitem_ptr_array[i];
		const auto extent = chanlist_item->GetYextent();
		const auto newext = MulDiv (extent, r2->Height(), r1->Height());
		chanlist_item->SetYextent (newext);
		const auto zero = chanlist_item->GetYzero();
		const auto newzero = zero - (r1->Height() - r2->Height()) / 2;
		chanlist_item->SetYzero (newzero);
	}
		
	// change index of first and last pt displayed
	auto l_size = m_lxLast-m_lxFirst+1;
	auto l_first = m_lxFirst + (l_size*(r2->left-r1->left))/r1->Width();
	l_size = (l_size *r2->Width()) / r1->Width();
	if (l_size > m_lxVeryLast)
	{
		l_size = m_lxVeryLast;
		l_first = 0;
	}
	const auto l_last  = l_first +l_size -1; 
		
	// call for new data only if indexes are different
	if (l_first != m_lxFirst || l_last != m_lxLast)
	{
		ResizeChannels(0, l_last-l_first+1);
		GetDataFromDoc(l_first, l_last);
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
			auto first = m_lxFirst/ m_samplingrate;
			auto last = m_lxLast/ m_samplingrate;
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
			const auto binlow = GetChanlistPixeltoBin(0, 0);
			const auto binhigh = GetChanlistPixeltoBin(0, m_clientRect.Height());
			auto yfirst = ConvertChanlistDataBinsToVolts(0, binlow);
			auto ylast = ConvertChanlistDataBinsToVolts(0, binhigh);
			m_yRuler.UpdateRange(&yfirst, &ylast);
		}
	}
}

void CLineViewWnd::PlotDatatoDC(CDC* p_dc)
{
	if (m_bADbuffers)
		return;

	// erase background
	if (m_erasebkgnd) 
	{
		UpdateXRuler();
		UpdateYRuler();
		EraseBkgnd(p_dc);
	}

	auto rect = m_displayRect;
	rect.DeflateRect(1,1);
	/*auto p_old_font= */p_dc->SelectObject(&m_hFont);

	// exit if no data defined
	if (!IsDefined() || m_pDataFile == nullptr) 
	{
		const auto textlen = m_csEmpty.GetLength();
		p_dc->DrawText(m_csEmpty, textlen, rect, DT_LEFT);
		return;
	}

	// plot comment at the bottom
	if (m_bBottomComment)
	{
		const auto textlen = m_csBottomComment.GetLength();
		p_dc->SetTextColor(RGB(0, 0, 255)); // BLUE
		p_dc->DrawText(m_csBottomComment, textlen, rect, DT_RIGHT|DT_BOTTOM|DT_SINGLELINE); 
	}

	// save DC
	const auto n_saved_dc = p_dc->SaveDC();
	ASSERT(n_saved_dc != 0);
	p_dc->IntersectClipRect(&m_displayRect);
	
	// prepare DC
	p_dc->SetMapMode (MM_ANISOTROPIC);
	p_dc->SetViewportOrg (m_displayRect.left, m_displayRect.Height()/2);
	p_dc->SetViewportExt (m_displayRect.Width(), -m_displayRect.Height());
	p_dc->SetWindowExt (m_displayRect.Width(), m_displayRect.Height()); 
	p_dc->SetWindowOrg (0, 0);

	// display all channels
	auto worg = -1;		// force origin
	auto wext = -1;		// force <= yextent
	const auto yVE = m_displayRect.Height();
	CEnvelope* pX = nullptr;
	auto nelements =0;
	
	auto color = BLACK_COLOR;
	const auto poldpen=p_dc->SelectObject(&m_penTable[color]);

	// display loop:
	for (auto ichan = chanlistitem_ptr_array.GetUpperBound(); ichan>=0; ichan--)	// scan all channels
	{
		const auto chanlist_item = chanlistitem_ptr_array[ichan];
		if (pX != chanlist_item->pEnvelopeAbcissa)
		{
			pX = chanlist_item->pEnvelopeAbcissa;
			nelements = pX->GetEnvelopeSize();
			if (m_PolyPoints.GetSize() != nelements*2)
				m_PolyPoints.SetSize(nelements*2);
			pX->ExportToAbcissa(m_PolyPoints);
		}

		if (chanlist_item->GetYextent() != wext)
			wext = chanlist_item->GetYextent();
		if (chanlist_item->GetYzero() != worg)
			worg = chanlist_item->GetYzero();
	
		auto pY = chanlist_item->pEnvelopeOrdinates;
		pY->ExportToOrdinates(m_PolyPoints);

		if (chanlist_item->GetColor() != color)
		{
			color = chanlist_item->GetColor();			
			p_dc->SelectObject(&m_penTable[color]);
		}
		if (chanlist_item->GetPenWidth() == 0)
			continue;

		for (auto j = 0; j < nelements; j++) {
			const auto p_point = &m_PolyPoints[j];
			p_point->y = MulDiv(short(p_point->y) - worg, yVE, wext);
		}

		p_dc->MoveTo(m_PolyPoints[0]);
		p_dc->Polyline(&m_PolyPoints[0], nelements);

		//display associated cursors ----------------------------------------
		if (GetNHZtags() > 0)
		{
			// select pen and display mode 
			p_dc->SelectObject(&m_blackDottedPen);
			const auto nold_rop = p_dc->SetROP2(R2_NOTXORPEN);
			// iterate through HZ cursor list			
			for (auto j= GetNHZtags()-1; j>= 0; j--)
			{				
				if (GetHZtagChan(j) != ichan)
					continue;
				auto k = GetHZtagVal(j);		// get val
				k = MulDiv(short(k)-worg, yVE, wext);
				p_dc->MoveTo(0,k);			// set initial pt
				p_dc->LineTo(nelements, k);	// HZ line				
			}
			p_dc->SetROP2(nold_rop);			// restore old display mode
			p_dc->SelectObject(poldpen);
		}
			
		// highlight data
		HighlightData(p_dc, ichan);
	}

	// restore DC
	p_dc->SelectObject(poldpen);		// restore old pen
	p_dc->RestoreDC(n_saved_dc);

	// display vertical cursors ------------------------------------
	if (GetNVTtags() > 0)
	{			
		// select pen and display mode 
		const auto poldp = p_dc->SelectObject(&m_blackDottedPen);
		const auto nold_rop = p_dc->SetROP2(R2_NOTXORPEN);
		// iterate through VT cursor list	
		const auto y0 = 0;
		const int y1 = m_displayRect.bottom;
		for (auto j=GetNVTtags()-1; j>=0; j--)
		{
			const auto lk = GetVTtagLval(j);	// get val
			if (lk <m_lxFirst || lk > m_lxLast)
				continue;
			const auto llk = (lk-m_lxFirst)*float(m_displayRect.Width()) / (m_lxLast - m_lxFirst+1); //muldiv
			const auto k = int(llk);
			p_dc->MoveTo(k,y0);			// set initial pt
			p_dc->LineTo(k,y1);			// VT line
		}
		p_dc->SetROP2(nold_rop);			// restore old display mode
		p_dc->SelectObject(poldp);
	}

	// temp tag
	if (m_hwndReflect != nullptr && m_tempVTtag != nullptr)
	{
		const auto poldp =p_dc->SelectObject(&m_blackDottedPen);
		const auto nold_rop = p_dc->SetROP2(R2_NOTXORPEN);
		p_dc->MoveTo(m_tempVTtag->m_pixel,2);
		p_dc->LineTo(m_tempVTtag->m_pixel,m_displayRect.bottom-2);
		p_dc->SetROP2(nold_rop);
		p_dc->SelectObject(poldp);
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
	const auto n_saved_dc = pDC->SaveDC();
	ASSERT(n_saved_dc != 0);
	const auto old_rect = m_clientRect;

	// prepare DC
	const auto previousmapping = pDC->SetMapMode(MM_TEXT);	// change map mode to text (1 pixel = 1 logical point)
	m_clientRect = *pRect; //CRect(0,0, pRect->Width(), pRect->Height());
	AdjustDisplayRect(pRect); 
	EraseBkgnd(pDC);
	// clip curves
	if (m_parms.bClipRect)	
		pDC->IntersectClipRect(m_displayRect);
	else
		pDC->SelectClipRgn(nullptr);

	// adjust coordinates for anisotropic mode
	const auto yVE = -m_displayRect.Height();
	const int yVO = m_displayRect.top + m_displayRect.Height()/2;
	const auto xVE = m_displayRect.Width();
	const int xVO = m_displayRect.left;
	
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

	auto p_envelope = envelope_ptr_array.GetAt(0);
	p_envelope->FillEnvelopeWithAbcissaEx(xVO, xVE, m_lxSize);

	// display all channels
	auto nelements=0;
	auto pX = chanlistitem_ptr_array[0]->pEnvelopeAbcissa;
	const BOOL b_poly_line = (pDC->m_hAttribDC == nullptr) || (pDC->GetDeviceCaps(LINECAPS) & LC_POLYLINE);
	auto color = BLACK_COLOR;
	const auto poldpen=pDC->SelectObject(&m_penTable[color]);

	// display loop:	
	for (auto ichan = chanlistitem_ptr_array.GetUpperBound(); ichan>=0; ichan--)	// scan all channels
	{
		const auto chanlist_item = chanlistitem_ptr_array[ichan];
		if (chanlist_item->GetflagPrintVisible() == FALSE)
			continue;

		// display: load abcissa   ----------------------------------------------
		if (pX != chanlist_item->pEnvelopeAbcissa)
		{
			pX = chanlist_item->pEnvelopeAbcissa;		// load pointer to abcissa
			pX->ExportToAbcissa(m_PolyPoints);	// copy abcissa to polypts buffer
			nelements = pX->GetEnvelopeSize();			// update nb of elements
		}
		// display: load ordinates ---------------------------------------------
		auto pY = chanlist_item->pEnvelopeOrdinates;	// load pointer to ordinates
		pY->ExportToOrdinates(m_PolyPoints);		// copy ordinates to polypts buffer

		// change extent, org and color ----------------------------------------
		const auto yextent = chanlist_item->GetYextent();
		const auto yzero   = chanlist_item->GetYzero();
		if (chanlist_item->GetColor() != color)	
		{
			color = chanlist_item->GetColor();			
			pDC->SelectObject(&m_penTable[color]);
		}
		// transform ordinates ------------------------------------------------
		for (auto j = 0; j<nelements; j++)
		{
			const auto p_point = &m_PolyPoints[j];
			p_point->y = MulDiv(p_point->y - yzero, yVE, yextent) +yVO;
		}
		//  display points ----------------------------------------------------	
		if (b_poly_line)
			pDC->Polyline(&m_PolyPoints[0], nelements);		// draw curve
		else
		{
			pDC->MoveTo(m_PolyPoints[0]);					// move pen to first pair of coords
			for (auto j = 0; j<nelements; j++)
				pDC->LineTo(m_PolyPoints[j]);				// draw lines
		}

		//display associated cursors ------------------------------------------
		if (GetNHZtags() > 0)					// print HZ cursors if any?
		{
			// select pen and display mode 
			CPen ltgrey_pen(PS_SOLID, 0, m_colorTable[SILVER_COLOR]);
			const auto pold_pen = pDC->SelectObject(&ltgrey_pen);
			// iterate through HZ cursor list
			const int x0 = pRect->left;
			const int x1 = pRect->right;			
			for (auto j= GetNHZtags()-1; j>= 0; j--)
			{
				if (GetHZtagChan(j) != ichan)	// next tag if not associated with
					continue;					// current channel
				auto k = GetHZtagVal(j);			// get value
				k = MulDiv(k-yzero, yVE, yextent) + yVO;
				pDC->MoveTo(x0,k);				// set initial pt
				pDC->LineTo(x1,k);				// HZ line				
			}
			pDC->SelectObject(pold_pen);
		}
		// highlight data ------------------------------------------------------
		HighlightData(pDC, ichan);
	}

	// display vertical cursors ------------------------------------------------
	if (GetNVTtags() > 0)
	{			
		// select pen and display mode 
		CPen ltgrey_pen(PS_SOLID, 0, m_colorTable[SILVER_COLOR]);
		const auto poldp =pDC->SelectObject(&ltgrey_pen);
		// iterate through VT cursor list	
		const int y0		= pRect->top;
		const int y1		= pRect->bottom;
		const int k0		= pRect->left;
		const int ksize	= pRect->right - k0;
		for (auto j=GetNVTtags()-1; j>=0; j--)
		{
			const auto lk = GetVTtagLval(j);	// get val
			if (lk <m_lxFirst || lk > m_lxLast)
				continue;
			const int k= k0 + (lk-m_lxFirst)* ksize / (m_lxLast - m_lxFirst+1);
			pDC->MoveTo(k,y0);			// set initial pt
			pDC->LineTo(k,y1);			// VT line		
		}		
		pDC->SelectObject(poldp);
	}

	// restore DC ----------------------------------------------------------------
	pDC->SelectObject(poldpen);			// restore old pen
	pDC->RestoreDC(n_saved_dc);			// restore DC
	pDC->SetMapMode (previousmapping);	// restore map mode
	m_clientRect = old_rect;
	AdjustDisplayRect(&m_clientRect);
}

BOOL CLineViewWnd::CopyAsText(int ioption, int iunit, int nabcissa)
{	
	// Clean clipboard of contents, and copy the text
	auto flag = FALSE;	
	if (OpenClipboard())
	{
		BeginWaitCursor();
		const auto oldcx = ResizeChannels(nabcissa, 0);	// compute new Envelopes
		GetDataFromDoc();
		EmptyClipboard();							// prepare clipboard
		const DWORD dw_len = 32768;	// 32 Kb
		size_t  pcch_remaining = dw_len / sizeof(TCHAR);
		const auto h_copy = (HANDLE) ::GlobalAlloc(GHND, dw_len);
		const auto pwave_format = m_pDataFile->GetpWaveFormat();

		if (h_copy != nullptr)
		{
			// memory allocated -- get pointer to it
			auto lp_copy =(LPTSTR) ::GlobalLock((HGLOBAL) h_copy);

			// data file name, comment, header
			const auto date = (pwave_format->acqtime).Format(_T("%#d %B %Y %X"));
			StringCchPrintfEx (lp_copy, pcch_remaining, &lp_copy, &pcch_remaining, STRSAFE_NULL_ON_FAILURE, _T("%s\t%s\r\n"), static_cast<LPCTSTR>(m_pDataFile->GetPathName()), static_cast<LPCTSTR>(date));
			StringCchPrintfEx(lp_copy, pcch_remaining, &lp_copy, &pcch_remaining, STRSAFE_NULL_ON_FAILURE,			_T("%s\t\r\n"), static_cast<LPCTSTR>(pwave_format->GetComments(_T("\t"), 0)));
			// time interval
			auto tt = GetDataFirst()/ pwave_format->chrate;		// first interval (sec)
			 StringCchPrintfEx(lp_copy, pcch_remaining, &lp_copy, &pcch_remaining, STRSAFE_NULL_ON_FAILURE, _T("time start(s):\t%f\r\n"), tt);
			tt = GetDataLast()/ pwave_format->chrate;			// last interval (sec)
			StringCchPrintfEx(lp_copy, pcch_remaining, &lp_copy, &pcch_remaining, STRSAFE_NULL_ON_FAILURE, _T("time end(s):\t%f"), tt);
			if (iunit == 0)
			{
				// zero volt for document
				StringCchPrintfEx(lp_copy, pcch_remaining, &lp_copy, &pcch_remaining, STRSAFE_NULL_ON_FAILURE, _T("\r\nzero volts(bin):\t%i\r\n"), 0 /*pwaveFormat->binzero*/);
				// unit for each channel
				StringCchPrintfEx(lp_copy, pcch_remaining, &lp_copy, &pcch_remaining, STRSAFE_NULL_ON_FAILURE, _T("mvolts per bin:\r\n"));
				for (auto i=0; i<GetChanlistSize(); i++) 
					StringCchPrintfEx(lp_copy, pcch_remaining, &lp_copy, &pcch_remaining, STRSAFE_NULL_ON_FAILURE, _T("%f\t"), double(GetChanlistVoltsperDataBin(i))*1000.f);
				lp_copy--;	// erase last tab
			}

			// comment for each channel
			StringCchPrintfEx(lp_copy, pcch_remaining, &lp_copy, &pcch_remaining, STRSAFE_NULL_ON_FAILURE, _T("\r\nchan title:\r\n"));
			for (auto i=0; i<GetChanlistSize(); i++)
				StringCchPrintfEx(lp_copy, pcch_remaining, &lp_copy, &pcch_remaining, STRSAFE_NULL_ON_FAILURE, _T("%s\t"), static_cast<LPCTSTR>(GetChanlistComment(i)));
			lp_copy--;	// erase last tab
			StringCchPrintfEx(lp_copy, pcch_remaining, &lp_copy, &pcch_remaining, STRSAFE_NULL_ON_FAILURE, _T("\r\n"));
	
			// data
			if (chanlistitem_ptr_array.GetSize() < 1)
			{		
				StringCchPrintfEx(lp_copy, pcch_remaining, &lp_copy, &pcch_remaining, STRSAFE_NULL_ON_FAILURE, _T("No data to display"));
			}
			else
			{
				switch (ioption)
				{
				case 0:
					GetAsciiEnvelope(lp_copy, iunit);
					break;
				case 1:
					GetAsciiLine(lp_copy, iunit);
					break;
				default:
					GetAsciiEnvelope(lp_copy, iunit);
					break;
				}
			}
			// un-allock array
			::GlobalUnlock(HGLOBAL(h_copy));
			flag = TRUE;		
			SetClipboardData (CF_TEXT, h_copy);
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
	const auto ichans = chanlistitem_ptr_array.GetUpperBound();
	const auto npoints = envelope_ptr_array.GetAt(0)->GetEnvelopeSize();
	// loop through all points	
	for (auto j=0; j<npoints; j++)
	{
		// loop through all channels
		for (auto i=0; i<=ichans; i++)	// scan all channels
		{
			const auto chanlist_item = chanlistitem_ptr_array[i];
			const int k = (chanlist_item->pEnvelopeOrdinates)->GetPointAt(j);
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
	const auto ichans = chanlistitem_ptr_array.GetUpperBound();
	const auto npoints = envelope_ptr_array.GetAt(0)->GetEnvelopeSize();
	// loop through all points	
	for (auto j=0; j<npoints; j+= m_dataperpixel)
	{
		// loop through all channels
		for (auto i=0; i<=ichans; i++)	// scan all channels
		{
			const auto chanlist_item = chanlistitem_ptr_array[i];			
			int k = (chanlist_item->pEnvelopeOrdinates)->GetPointAt(j);
			if (m_dataperpixel > 1)
			{
				k += (chanlist_item->pEnvelopeOrdinates)->GetPointAt(j+1);
				k = k/2;
			}
			if (iunit == 1)
				lpCopy += wsprintf(lpCopy, _T("%f\t"), double(k)*GetChanlistVoltsperDataBin(i)*1000.f);
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
	auto p_dc = CWnd::GetDC();			// select dc
	const auto n_saved_dc = p_dc->SaveDC();		// preserve current settings
	CPen temp_pen;
	temp_pen.CreatePen(PS_SOLID, 0, m_colorTable[SILVER_COLOR]);
	const auto poldpen=p_dc->SelectObject(&temp_pen); // load pen
	p_dc->IntersectClipRect(&m_displayRect);

	p_dc->SetMapMode (MM_ANISOTROPIC);	// prepare display with appropriate
	p_dc->SetViewportOrg (m_displayRect.left, m_yVO);		// scaling
	p_dc->SetViewportExt (Width(), m_yVE);
	p_dc->SetWindowExt (m_XORxext, m_XORyext);
	p_dc->SetWindowOrg (0, 0);
	
	// display envelope store into pData using XOR mode
	const auto nold_rop = p_dc->SetROP2(R2_NOTXORPEN);
	p_dc->MoveTo(m_PolyPoints[0]);
	p_dc->Polyline(&m_PolyPoints[0], m_XORnelmts);
	p_dc->SetROP2(nold_rop);

	p_dc->SelectObject(poldpen);			// restore resources - pen
	p_dc->RestoreDC(n_saved_dc);			// restore DC in previous mode
	ReleaseDC(p_dc);						// release DC
	temp_pen.DeleteObject();
}

void CLineViewWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	// convert chan values stored within HZ tags into pixels
	if (GetNHZtags() > 0)
	{
		for (auto icur = 0; icur<GetNHZtags(); icur++)
		{
			// convert device coordinates into val
			const auto pixval = GetChanlistBintoPixel(GetHZtagChan(icur), GetHZtagVal(icur));			
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
		m_hitcurve = DoesCursorHitCurve(point);
		if (m_hitcurve >= 0)
		{
			// cancel track rect mode (cursor captured)
			m_trackMode = TRACK_CURVE;		// flag: track curve

			// modify polypoint and prepare for XORing curve tracked with mouse			
			const auto chanlist_item = chanlistitem_ptr_array[m_hitcurve];
			auto pX =  chanlist_item->pEnvelopeAbcissa;		// display: load abcissa			
			pX->GetMeanToAbcissa(m_PolyPoints);
			m_XORnelmts = pX->GetEnvelopeSize() /2;	// nb of elements
			m_XORxext = pX->GetnElements() / 2;		// extent
			
			auto pY = chanlist_item->pEnvelopeOrdinates;				// load ordinates
			pY->GetMeanToOrdinates(m_PolyPoints);
			m_XORyext = chanlist_item->GetYextent();		// store extent
			m_zero = chanlist_item->GetYzero();			// store zero
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
		const auto chanlist_item = chanlistitem_ptr_array[GetHZtagChan(m_HCtrapped)];
		m_yWE = chanlist_item->GetYextent();				// store extent
		m_yWO = chanlist_item->GetYzero();					// store zero		
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
			auto chanlist_item = chanlistitem_ptr_array[m_hitcurve];
			chanlist_item->SetYzero(m_zero);
			m_trackMode = TRACK_OFF;
			PostMyMessage(HINT_HITCHANNEL, m_hitcurve);	// tell parent chan selected
			Invalidate();
		}
		break;
	// horizontal cursor was tracked
	case TRACK_HZTAG:
		{
			const auto chan = GetHZtagChan(m_HCtrapped);		// get channel
			const auto val = GetChanlistPixeltoBin(chan, m_ptLast.y);
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
			const auto lval = long(point.x)*(m_liLast-m_liFirst+1)/long(m_displayRect.right) + m_liFirst;
			SetVTtagLval(m_HCtrapped, lval);		
			point.x = int((lval - m_liFirst) * long(m_displayRect.right) / (m_liLast - m_liFirst + 1));
			XorVTtag(point.x);
			PostMyMessage(HINT_CHANGEVERTTAG, m_HCtrapped);
			m_trackMode = TRACK_OFF;
		}
		break;
	case TRACK_RECT:
		// mouse was captured: no -> exit!
		{
			// skip too small a rectangle (5 pixels?)
			CRect rect_out(m_ptFirst.x, m_ptFirst.y, m_ptLast.x, m_ptLast.y);		
			const auto jitter = 3;
			const BOOL b_rect_ok = ((abs(rect_out.Height())> jitter) || (abs(rect_out.Width())> jitter));

			// perform action according to cursor type
			auto rect_in = m_displayRect;
			switch (m_cursorType)
			{
			case 0:		// if no cursor, no curve track, then move display
				if (b_rect_ok)
				{
					InvertTracker(point);
					rect_out = rect_in;
					rect_out.OffsetRect(m_ptFirst.x-m_ptLast.x, m_ptFirst.y-m_ptLast.y);
					ZoomData(&rect_in, &rect_out);
				}
				break;
			case CURSOR_ZOOM: 	 // zoom operation
				if (b_rect_ok)
				{
					ZoomData(&rect_in, &rect_out);					
					m_ZoomFrom = rect_in;
					m_ZoomTo   = rect_out;					
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
	auto chanfound=-1;						// output value
	const auto ichans = chanlistitem_ptr_array.GetUpperBound();
	auto chanlist_item = chanlistitem_ptr_array[0]->pEnvelopeAbcissa;
	const auto xextent = chanlist_item->GetnElements();		// n elements stored in one Envelope	
	int index1 = point.x -m_cxjitter;		// horizontal jitter backwards
	auto index2 = index1 + m_cxjitter;		// horiz jitter forwards
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
	for (auto chan=0; chan<=ichans; chan++)	// scan all channels
	{
		// convert device coordinates into val
		const auto ival = GetChanlistPixeltoBin(chan, point.y);
		const auto ijitter = MulDiv(m_cyjitter, GetChanlistYextent(chan), -m_yVE);
		const auto valmax = ival+ijitter;			// mouse max
		const auto valmin = ival-ijitter;			// mouse min
		chanlist_item = chanlistitem_ptr_array[chan]->pEnvelopeOrdinates;
			
		// loop around horizontal jitter...
		for (auto index=index1; index<index2 && chanfound<0; index++)
		{				
			int kmax = chanlist_item->GetPointAt(index);	// get chan Envelope data point
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
				int kmin = chanlist_item->GetPointAt(index);	// get min
				if (kmin > kmax)					// ensure that k1=max
				{
					const auto k = kmax;					// exchange data between
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
	const auto chan = GetHZtagChan(i);
	const auto chanlist_item = chanlistitem_ptr_array[chan];
	m_XORyext = chanlist_item->GetYextent();			// store extent
	m_zero = chanlist_item->GetYzero();				// store zero	
	m_ptLast.y = MulDiv(GetHZtagVal(i) - m_zero, m_yVE, m_XORyext) + m_yVO;
	CPoint point;
	point.y = MulDiv(val - m_zero, m_yVE, m_XORyext) + m_yVO;
	XorHZtag(point.y);
	SetHZtagVal(i, val);
}

void CLineViewWnd::SetHighlightData(CHighLight& source)
{
	m_highlighted = source;
}

void CLineViewWnd::SetHighlightData(CDWordArray* pDWintervals)
{
	if (pDWintervals == nullptr)
		return;

	m_highlighted.channel = pDWintervals->GetAt(0);
	m_highlighted.color = (COLORREF)pDWintervals->GetAt(1);
	m_highlighted.pensize = pDWintervals->GetAt(2);
	m_highlighted.l_first.RemoveAll();
	m_highlighted.l_last.RemoveAll();
	const auto size = (pDWintervals->GetSize() - 3) / 2;
	m_highlighted.l_first.SetSize(size);
	m_highlighted.l_last.SetSize(size);

	for (auto i = 3; i < pDWintervals->GetSize(); i += 2)
	{
		m_highlighted.l_first.Add(pDWintervals->GetAt(i));
		m_highlighted.l_last.Add(pDWintervals->GetAt(i+1));
	}
}

void CLineViewWnd::HighlightData(CDC* pDC, int chan)
{
	// skip if not the correct chan
	if (chan != m_highlighted.channel || m_highlighted.l_first.GetSize() < 2)
		return;

	// get color and pen size from array m_pDWintervals
	CPen new_pen;
	new_pen.CreatePen(PS_SOLID, m_highlighted.pensize, m_highlighted.color);
	const auto poldpen = (CPen*)pDC->SelectObject(&new_pen);
	const BOOL b_poly_line = (pDC->m_hAttribDC == nullptr)
		|| (pDC->GetDeviceCaps(LINECAPS) & LC_POLYLINE);

	// loop to display data	

	// pointer to descriptor	
	for (auto i = 0; i < m_highlighted.l_first.GetSize(); i ++)
	{
		// load ith interval values
		auto l_first = m_highlighted.l_first[i];		// first val
		auto l_last = m_highlighted.l_last[i];		// last val

		if (l_last < m_lxFirst || l_first > m_lxLast)
			continue;								// next if out of range

		// clip data if out of range
		if (l_first < m_lxFirst)				// minimum interval
			l_first = m_lxFirst;
		if (l_last > m_lxLast)					// maximum interval
			l_last = m_lxLast;

		// compute corresponding interval (assume same m_scale for all chans... (!!)
		auto ifirst = m_scale.GetWhichInterval(l_first - m_lxFirst);
		if (ifirst < 0)
			continue;
		auto ilast = m_scale.GetWhichInterval(l_last - m_lxFirst) + 1;
		if (ilast < 0)
			continue;

		if (m_dataperpixel != 1)
		{									// envelope plotting
			ifirst = ifirst * m_dataperpixel;
			ilast = ilast * m_dataperpixel;
		}
		// display data	
		const auto nelements = ilast - ifirst;
		if (b_poly_line)
			pDC->Polyline(&m_PolyPoints[ifirst], nelements);
		else
		{
			pDC->MoveTo(m_PolyPoints[ifirst]);
			for (auto j = 0; j < nelements; j++)
				pDC->LineTo(m_PolyPoints[ifirst + j]);
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
	auto pC = envelope_ptr_array.GetAt(0);	
	pC->FillEnvelopeWithAbcissaEx(1, m_displayRect.right-1, chsamples);
	pC->ExportToAbcissa(m_PolyPoints);
	SetbUseDIB(FALSE); 

	// clear window before acquiring data
	CClientDC dc(this);
	EraseBkgnd(&dc);

	// print comment
	dc.SelectObject (GetStockObject (DEFAULT_GUI_FONT));
	auto rect = m_displayRect;
	rect.DeflateRect(1,1);
	CString cs = _T("Waiting for trigger");
	const auto textlen = cs.GetLength();
	dc.DrawText(cs, textlen, rect, DT_LEFT); 
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
		const auto ndata = m_lxSize - m_lADbufferdone;
		if (ndata > 0)
		{
			ADdisplayBuffer(lpBuf, ndata);			// display end
			nsamples -= ndata;						// reduce nb of data to display
			lpBuf += (ndata* long(m_pDataFile->GetpWaveFormat()->scan_count));
		}
		m_lADbufferdone = 0;						// clear len of data
	}

	// create device context and prepare bitmap to receive drawing commands
	CClientDC dc(this);						// get window DC
	CDC dc_mem;								// prepare device context
	dc_mem.CreateCompatibleDC(&dc);			// create compatible DC
	CBitmap bitmap_plot;
	const auto poldbitmap = dc_mem.SelectObject(&bitmap_plot);
	const auto isave_dc = dc_mem.SaveDC();			// save DC
	CDC* pDC = &dc;
	if (m_bUseDIB)
		pDC = &dc_mem;

	// get first and last pixels of the interval to display
	const auto ad_pixel_first = MulDiv(m_lADbufferdone, m_npixels, m_lxSize);
	auto ad_pixel_last = MulDiv(m_lADbufferdone+nsamples-1, m_npixels, m_lxSize);
	if (ad_pixel_last > m_displayRect.right-2)
		ad_pixel_last = m_displayRect.right-2;

	const auto npixels = ad_pixel_last - ad_pixel_first+1;
	const auto npoints = npixels * m_dataperpixel;

	//////////////////////////// prepare display ///////////////////////////////////

	const auto p_polypoints = &m_PolyPoints[ad_pixel_first * m_dataperpixel * 2];	
	//dcMem.SetMapMode (MM_TEXT);
	//dcMem.SetViewportOrg (0, 0);
	//dcMem.SetWindowOrg (0, 0);

	CRect rect(ad_pixel_first, m_displayRect.top, ad_pixel_last, m_displayRect.bottom);
	if (ad_pixel_first == 0)
		rect.left = 1;
	rect.DeflateRect(0, 1);
	pDC->IntersectClipRect(rect);
	pDC->FillSolidRect(&rect, m_parms.crScopeFill);

	auto* ppen_old = (CPen*)pDC->SelectStockObject(BLACK_PEN);

	pDC->SetMapMode (MM_ANISOTROPIC);		// display in anisotropic mode
	pDC->SetViewportExt (m_xVE, m_yVE);
	pDC->SetViewportOrg (m_xVO, m_yVO);
	pDC->SetWindowExt (m_npixels, m_yVE);	//chanlist_item->GetYextent());
	pDC->SetWindowOrg (0, 0);				//chanlist_item->GetYzero());
	const auto yVE = m_yVE;

	for (auto ichan = 0; ichan<chanlistitem_ptr_array.GetSize(); ichan++)
	{
		// load channel descriptors
		const auto chanlist_item = chanlistitem_ptr_array[ichan];
		CPen temp_pen;
		temp_pen.CreatePen(PS_SOLID, 0, m_colorTable[chanlist_item->GetColor()]);	
		pDC->SelectObject(&temp_pen);
		
		// compute max min and plot array
		auto ppt = p_polypoints;
		auto p_data = lpBuf + ichan;
		const auto worg=chanlist_item->GetYzero();
		const auto wext=chanlist_item->GetYextent();
		const int n_doc_chans = m_pDataFile->GetpWaveFormat()->scan_count;
		// only one data point per pixel
		if (m_dataperpixel != 1)
		{
			auto nelmts0 =  0;
			for (auto i=0; i<npixels; i++)
			{
				auto i_max= *p_data;			// init max
				auto i_min= i_max;				// init min
				int nelmts = ((i+1)* nsamples/ npixels)-nelmts0;
				nelmts0 += nelmts;
				while (nelmts > 0)			// scan nb elements
				{							// designed by scale
					const auto idata = *p_data;		// load value
					if (idata < i_min)
						i_min = idata;		// change min
					else if (idata > i_max)  
						i_max = idata;		// change max
					p_data += n_doc_chans;	// update data pointer 
					nelmts--;
				}

				ppt->y = MulDiv(i_max-worg, yVE, wext);
				ppt++;		// update Envelope pointer
				ppt->y = MulDiv(i_min-worg, yVE, wext);
				ppt++;		// update Envelope pointer
			}
		}

		// at least 2 data points per pixel		
		else
		{
			for (auto i=0; i<npixels; i++, ppt++, p_data += n_doc_chans)
				ppt->y = MulDiv(*p_data-worg, yVE, wext);
		}
		pDC->MoveTo(int(p_polypoints->x -2), int(p_polypoints->y));
		pDC->PolylineTo(p_polypoints, npoints);
		temp_pen.DeleteObject();	
	}

	// restore the old pen and exit	
	pDC->SelectObject(ppen_old);	// select initial object
	pDC->RestoreDC(isave_dc);		// restore DC
	pDC->SetMapMode (MM_TEXT);
	pDC->SetViewportOrg (0, 0);
	pDC->SetWindowOrg (0, 0);

	if (m_bUseDIB)
		dc.BitBlt(rect.left, rect.top,
			rect.Width(), rect.Height(),
			&dc_mem,
			rect.left, rect.top,
			SRCCOPY);

	dc_mem.SelectObject(poldbitmap);	// release bitmap
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

		const auto nenvelopes = envelope_ptr_array.GetSize();
		ar << nenvelopes;
		const auto nchanlist_items = chanlistitem_ptr_array.GetSize();
		ar << nchanlist_items;
		
		for (auto i=0; i< nenvelopes; i++)
			envelope_ptr_array[i]->Serialize(ar);

		for (auto i=0; i< nchanlist_items; i++)
			chanlistitem_ptr_array[i]->Serialize(ar);
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
		if (envelope_ptr_array.GetSize() > nenvelopes)
		{			
			for (auto i=envelope_ptr_array.GetUpperBound(); i>= nenvelopes; i--)
				delete envelope_ptr_array[i];				
			envelope_ptr_array.SetSize(nenvelopes);
		}
		else if (envelope_ptr_array.GetSize() < nenvelopes)
		{
			const auto nenvelope0 = envelope_ptr_array.GetSize();
			envelope_ptr_array.SetSize(nenvelopes);
			for (auto i=nenvelope0; i < nenvelopes; i++)
				envelope_ptr_array[i] = new CEnvelope; // (CEnvelope*)
		}
		for (auto i=0; i< nenvelopes; i++)
			envelope_ptr_array[i]->Serialize(ar);

		// ChanList array
		if (chanlistitem_ptr_array.GetSize() > nchanlistItems)
		{
			for (auto i=chanlistitem_ptr_array.GetUpperBound(); i>= nchanlistItems; i--)
				delete chanlistitem_ptr_array[i];
			chanlistitem_ptr_array.SetSize(nchanlistItems);
			
		}
		else if (chanlistitem_ptr_array.GetSize() < nchanlistItems)
		{
			const auto nchanlistItems0 = chanlistitem_ptr_array.GetSize();
			chanlistitem_ptr_array.SetSize(nchanlistItems);
			for (auto i=nchanlistItems0; i < nchanlistItems; i++)
				chanlistitem_ptr_array[i] = new CChanlistItem; // (CChanlistItem*) 
		}
		int ix = 0;
		int iy = 0;
		for (int i=0; i< nchanlistItems; i++)
		{
			chanlistitem_ptr_array[i]->Serialize(ar);
			chanlistitem_ptr_array[i]->GetEnvelopeArrayIndexes(ix, iy);
			chanlistitem_ptr_array[i]->SetEnvelopeArrays(envelope_ptr_array.GetAt(ix), ix, envelope_ptr_array.GetAt(iy), iy);
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