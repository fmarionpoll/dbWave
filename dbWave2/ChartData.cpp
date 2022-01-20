#include "StdAfx.h"
#include <stdlib.h>
#include <Strsafe.h>
#include "resource.h"
#include "ChartData.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_SERIAL(CChartDataWnd, CChartWnd, 1)

CChartDataWnd::CChartDataWnd()
{
	m_bVTtagsLONG = TRUE;
	AddChanlistItem(0, 0);
	ResizeChannels(m_npixels, 1024);
	m_csEmpty = _T("no data (lineview)");
}

CChartDataWnd::~CChartDataWnd()
{
	RemoveAllChanlistItems();
}

BEGIN_MESSAGE_MAP(CChartDataWnd, CChartWnd)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_SIZE()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

void CChartDataWnd::RemoveAllChanlistItems()
{
	for (auto i = envelope_ptr_array.GetUpperBound(); i >= 0; i--)
		delete envelope_ptr_array[i];
	envelope_ptr_array.RemoveAll();

	for (auto i = chanlistitem_ptr_array.GetUpperBound(); i >= 0; i--)
		delete chanlistitem_ptr_array[i];
	chanlistitem_ptr_array.RemoveAll();
}

int CChartDataWnd::AddChanlistItem(int ns, int mode)
{
	// first time??	create Envelope(0) with abcissa series
	if (chanlistitem_ptr_array.GetSize() == 0)
	{
		m_PolyPoints.SetSize(m_npixels * 4);
		m_scale.SetScale(m_npixels, m_lxSize);
		m_dataperpixel = 2;
		auto* p_envelope = new CEnvelope(m_npixels * m_dataperpixel, m_dataperpixel, 0, -1, 0);
		ASSERT(p_envelope != NULL);
		envelope_ptr_array.Add(p_envelope);
		p_envelope->FillEnvelopeWithAbcissa(m_npixels * m_dataperpixel, m_lxSize);
	}

	// create new Envelope and store pointer into Envelopeslist
	auto span = 0;
	if (m_pDataFile != nullptr)
		span = m_pDataFile->GetTransfDataSpan(mode);
	auto* p_envelope_y = new CEnvelope(m_npixels, m_dataperpixel, ns, mode, span);
	ASSERT(p_envelope_y != NULL);
	const auto j = envelope_ptr_array.Add(p_envelope_y);

	// create new chanlistitem with x=Envelope(0) and y=the new Envelope
	auto* p_chan_list_item = new CChanlistItem(envelope_ptr_array.GetAt(0), 0, p_envelope_y, j);
	ASSERT(p_chan_list_item != NULL);
	const auto index_newchan = chanlistitem_ptr_array.Add(p_chan_list_item);

	// init display parameters
	p_chan_list_item->InitDisplayParms(1, RGB(0, 0, 0), 2048, 4096);

	if (m_pDataFile != nullptr)
	{
		float voltsperb;
		m_pDataFile->GetWBVoltsperBin(ns, &voltsperb, mode);
		const auto pchan_array = m_pDataFile->GetpWavechanArray();
		const auto pwave_format = m_pDataFile->GetpWaveFormat();
		p_chan_list_item->SetDataBinFormat(pwave_format->binzero, pwave_format->binspan);
		p_chan_list_item->SetDataVoltsFormat(voltsperb, pwave_format->fullscale_volts);
		if (ns >= pchan_array->ChanArray_getSize())
			ns = 0;
		const auto pchan = pchan_array->Get_p_channel(ns);
		p_chan_list_item->dl_comment = pchan->am_csComment;	// get comment however
		UpdateChanlistMaxSpan();				// update span max
		if (mode > 0)							// modif comment if transform buffer
			p_chan_list_item->dl_comment = (m_pDataFile->GetTransfDataName(mode)).Left(8) + ": " + p_chan_list_item->dl_comment;
	}
	return index_newchan;
}

int CChartDataWnd::RemoveChanlistItem(WORD i)
{
	const auto chanlist_size = chanlistitem_ptr_array.GetSize();	// get size of chan array
	if (chanlist_size > 0)	// delete Envelopes ordinates but make sure that it is not used
	{
		const auto pa = chanlistitem_ptr_array[i]->pEnvelopeOrdinates;
		// step 1: check that this envelope is not used by another channel
		auto b_used_only_once = TRUE;
		for (auto lj = chanlist_size; lj >= 0; lj--)
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
			for (auto k = envelope_ptr_array.GetUpperBound(); k >= 0; k--)
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
	return chanlist_size - 1;
}

void CChartDataWnd::UpdateChanlistMaxSpan()
{
	if (envelope_ptr_array.GetSize() <= 1)
		return;
	// get spanmax stored in Envelope(0)
	auto imax = 0;
	for (auto i = envelope_ptr_array.GetUpperBound(); i > 0; i--)
	{
		const auto j = envelope_ptr_array[i]->GetDocbufferSpan();
		if (j > imax)
			imax = j;
	}
	// store imax
	envelope_ptr_array[0]->SetDocbufferSpan(imax);	//store max
}

void CChartDataWnd::UpdateChanlistFromDoc()
{
	for (auto i = chanlistitem_ptr_array.GetUpperBound(); i >= 0; i--)
	{
		auto p_chanlist_item = chanlistitem_ptr_array[i];
		auto p_ord = p_chanlist_item->pEnvelopeOrdinates;
		const auto ns = p_ord->GetSourceChan();
		const auto mode = p_ord->GetSourceMode();
		p_ord->SetDocbufferSpan(m_pDataFile->GetTransfDataSpan(mode));
		const auto pchan_array = m_pDataFile->GetpWavechanArray();
		const auto pchan = pchan_array->Get_p_channel(ns);
		p_chanlist_item->dl_comment = pchan->am_csComment;
		if (mode > 0)
			p_chanlist_item->dl_comment = (m_pDataFile->GetTransfDataName(mode)).Left(6) + ": " + p_chanlist_item->dl_comment;
		UpdateGainSettings(i);	// keep physical value of yextent and zero constant
	}
	UpdateChanlistMaxSpan();	// update max span
}

void CChartDataWnd::UpdateGainSettings(int ichanlist)
{
	CChanlistItem* pchan = chanlistitem_ptr_array[ichanlist];
	const auto p_ord = pchan->pEnvelopeOrdinates;
	const auto ns = p_ord->GetSourceChan();
	const auto mode = p_ord->GetSourceMode();
	float doc_volts_per_bin;
	m_pDataFile->GetWBVoltsperBin(ns, &doc_volts_per_bin, mode);
	const auto volts_per_data_bin = pchan->GetVoltsperDataBin();
	const auto pwave_format = m_pDataFile->GetpWaveFormat();
	if (doc_volts_per_bin != volts_per_data_bin)
	{
		pchan->SetDataBinFormat(pwave_format->binzero, pwave_format->binspan);
		pchan->SetDataVoltsFormat(doc_volts_per_bin, pwave_format->fullscale_volts);
		auto iextent = pchan->GetYextent();
		iextent = int(float(iextent) / doc_volts_per_bin * volts_per_data_bin);
		pchan->SetYextent(iextent);
	}
}

int CChartDataWnd::SetChanlistSourceChan(WORD ichanlist, int acqchan)
{
	// check if channel is allowed
	const auto pwave_format = m_pDataFile->GetpWaveFormat();
	if (pwave_format->scan_count <= acqchan || acqchan < 0)
		return -1;

	// make sure we have enough data channels...
	if (GetChanlistSize() <= ichanlist)
		for (auto j = GetChanlistSize(); j <= ichanlist; j++)
			AddChanlistItem(j, 0);

	// change channel
	auto p_chanlist_item = chanlistitem_ptr_array[ichanlist];
	auto p_ord = p_chanlist_item->pEnvelopeOrdinates;
	p_ord->SetSourceChan(acqchan);
	const auto mode = p_ord->GetSourceMode();
	// modify comment
	const auto pchan_array = m_pDataFile->GetpWavechanArray();
	const auto pchan = pchan_array->Get_p_channel(acqchan);
	p_chanlist_item->dl_comment = pchan->am_csComment;
	if (mode > 0)
		p_chanlist_item->dl_comment = (m_pDataFile->GetTransfDataName(mode)).Left(6) + _T(": ") + p_chanlist_item->dl_comment;
	UpdateGainSettings(ichanlist);
	return acqchan;
}

void CChartDataWnd::SetChanlistOrdinates(WORD ichanlist, int acqchan, int transform)
{
	// change channel
	auto chanlist_item = chanlistitem_ptr_array[ichanlist];
	chanlist_item->SetOrdinatesSourceData(acqchan, transform);
	// modify comment
	const auto pchanArray = m_pDataFile->GetpWavechanArray();
	if (acqchan >= pchanArray->ChanArray_getSize())
		acqchan = 0;
	const auto pchan = pchanArray->Get_p_channel(acqchan);
	chanlist_item->dl_comment = pchan->am_csComment;
	if (transform > 0)
		chanlist_item->dl_comment = (m_pDataFile->GetTransfDataName(transform)).Left(6) + _T(": ") + chanlist_item->dl_comment;
}

void CChartDataWnd::SetChanlistVoltsExtent(const int ichanlist, const float* pvalue)
{
	auto ichanfirst = ichanlist;
	auto ichanlast = ichanlist;
	if (ichanlist < 0)
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
			voltsextent = yvoltsperbin * p_chanlist_item->GetYextent();

		const auto yextent = int(voltsextent / yvoltsperbin);
		p_chanlist_item->SetYextent(yextent);
	}
}

void CChartDataWnd::SetChanlistVoltsZero(const int ichanlist, const float* pvalue)
{
	auto ichanfirst = ichanlist;
	auto ichanlast = ichanlist;
	if (ichanlist < 0)
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

int CChartDataWnd::SetChanlistTransformMode(WORD ichanlist, int imode)
{
	// check if transform is allowed
	if (!m_pDataFile->IsWBTransformAllowed(imode) ||	// ? is transform allowed
		!m_pDataFile->InitWBTransformBuffer()) 		// ? is init OK
	{
		AfxMessageBox(IDS_LNVERR02, MB_OK);
		return -1;
	}

	// change transform mode
	auto p_chanlist_item = chanlistitem_ptr_array[ichanlist];
	auto p_ord = p_chanlist_item->pEnvelopeOrdinates;
	const auto ns = p_ord->GetSourceChan();
	// change transform
	p_ord->SetSourceMode(imode, m_pDataFile->GetTransfDataSpan(imode));

	// modify comment
	const auto pchan_array = m_pDataFile->GetpWavechanArray();
	const auto pchan = pchan_array->Get_p_channel(ns);
	p_chanlist_item->dl_comment = pchan->am_csComment;
	if (imode > 0)
		p_chanlist_item->dl_comment = (m_pDataFile->GetTransfDataName(imode)).Left(8)
		+ _T(": ") + p_chanlist_item->dl_comment;
	UpdateGainSettings(ichanlist);
	UpdateChanlistMaxSpan();
	return imode;
}

SCOPESTRUCT* CChartDataWnd::GetScopeParameters()
{
	auto nchannels = chanlistitem_ptr_array.GetSize();
	m_scopestruct.channels.SetSize(nchannels);
	for (auto i = 0; i < nchannels; i++)
	{
		auto p_chanlist_item = chanlistitem_ptr_array[i];
		m_scopestruct.channels[i].izero = p_chanlist_item->GetYzero();
		m_scopestruct.channels[i].iextent = p_chanlist_item->GetYextent();
	}

	return &m_scopestruct;
}

void CChartDataWnd::SetScopeParameters(SCOPESTRUCT* pStruct)
{
	auto nchannels_struct = pStruct->channels.GetSize();
	auto nchannels_chanlist = chanlistitem_ptr_array.GetSize();
	for (auto i = 0; i < nchannels_struct; i++)
	{
		if (i >= nchannels_chanlist)
			break;
		auto p_chanlist_item = chanlistitem_ptr_array[i];
		p_chanlist_item->SetYzero(pStruct->channels[i].izero);
		p_chanlist_item->SetYextent(pStruct->channels[i].iextent);
	}
}

void CChartDataWnd::AutoZoomChan(int j)
{
	auto i1 = j;
	auto i2 = j;
	if (j < 0)
	{
		i1 = 0;
		i2 = GetChanlistSize() - 1;
	}

	int max, min;
	for (auto i = i1; i <= i2; i++)
	{
		CChanlistItem* chan = GetChanlistItem(i);
		chan->GetMaxMin(&max, &min);
		chan->SetYzero((max + min) / 2);
		chan->SetYextent(MulDiv(max - min + 1, 10, 8));
	}
}

void CChartDataWnd::SplitChans()
{
	int max, min;
	const auto nchans = GetChanlistSize();
	auto icur = nchans - 1;
	for (auto i = 0; i < nchans; i++, icur -= 2)
	{
		CChanlistItem* chan = GetChanlistItem(i);
		chan->GetMaxMin(&max, &min);
		const auto amplitudespan = MulDiv((max - min), 12 * nchans, 10);
		chan->SetYextent(amplitudespan);
		const auto ioffset = (max + min) / 2 + MulDiv(amplitudespan, icur, nchans * 2);
		chan->SetYzero(ioffset);
	}
}

void CChartDataWnd::CenterChan(int j)
{
	auto i1 = j;
	auto i2 = j;
	if (j < 0)
	{
		i1 = 0;
		i2 = GetChanlistSize() - 1;
	}

	int max, min;
	for (auto i = i1; i <= i2; i++)
	{
		CChanlistItem* chan = GetChanlistItem(i);
		chan->GetMaxMin(&max, &min);
		const auto yzero = (max + min) / 2;
		chan->SetYzero(yzero);
	}
}

void CChartDataWnd::MaxgainChan(int j)
{
	auto i1 = j;
	auto i2 = j;
	if (j < 0)
	{
		i1 = 0;
		i2 = GetChanlistSize() - 1;
	}

	int max, min;
	for (auto i = i1; i <= i2; i++)
	{
		CChanlistItem* pchan = GetChanlistItem(i);
		pchan->GetMaxMin(&max, &min);
		const auto yextent = MulDiv(max - min + 1, 10, 8);
		pchan->SetYextent(yextent);
	}
}

int CChartDataWnd::ResizeChannels(const int npixels, const long l_size)
{
	const auto old_npixels = m_npixels;

	// trap dummy values and return if no operation is necessary
	if (npixels != 0)				// horizontal resolution
		m_npixels = npixels;
	if (l_size != 0)					// size of data to display
		m_lxSize = l_size;

	// change horizontal resolution	: m_Polypoints receives abcissa and ordinates
	// make provision for max and min points
	if (m_PolyPoints.GetSize() != m_npixels * 4)
		m_PolyPoints.SetSize(m_npixels * 4);

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
		for (auto i_envelope = 0; i_envelope < n_envelopes; i_envelope++)
		{
			p_envelope = envelope_ptr_array.GetAt(i_envelope);
			p_envelope->SetEnvelopeSize(npts, m_dataperpixel);
		}
		p_envelope = envelope_ptr_array.GetAt(0);
		p_envelope->FillEnvelopeWithAbcissa(m_npixels, m_lxSize);// store data series
	}

	// read data and update page and line sizes / file browse
	UpdatePageLineSize();
	m_lxLast = m_lxFirst + m_lxSize - 1;
	return old_npixels;
}

BOOL CChartDataWnd::AttachDataFile(CAcqDataDoc* p_data_file)
{
	m_pDataFile = p_data_file;
	m_samplingrate = m_pDataFile->GetpWaveFormat()->chrate;
	m_pDataFile->SetReadingBufferDirty();
	ASSERT(m_pDataFile->GetDOCchanLength() > 0);

	long l_size = m_pDataFile->GetDOCchanLength();

	// init parameters used to display Envelopes
	const int l_very_last = m_lxVeryLast;
	m_lxVeryLast = m_pDataFile->GetDOCchanLength() - 1;
	m_lxFirst = 0;
	m_lxLast = l_size - 1;
	if (m_lxSize != l_size || l_very_last != m_lxVeryLast)
	{
		m_lxSize = l_size;
		m_dataperpixel = m_scale.SetScale(m_npixels, m_lxSize);
		m_dataperpixel = 2;
	}

	//Remove irrelevant Chanlist items;
	const auto docchanmax = m_pDataFile->GetpWaveFormat()->scan_count - 1;
	const auto chanlistmax = chanlistitem_ptr_array.GetUpperBound();
	for (auto i = chanlistmax; i >= 0; i--)
	{
		if (GetChanlistItem(i)->GetSourceChan() > docchanmax)
			RemoveChanlistItem(i);
	}

	// Remove irrelevant Envelopes();
	const auto n_envelopes = envelope_ptr_array.GetUpperBound();
	for (auto i = n_envelopes; i > 0; i--) // ! Envelope(0)=abcissa
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

BOOL CChartDataWnd::GetDataFromDoc()
{
	// get document parameters: exit if empty document
	if (m_bADbuffers || m_pDataFile == nullptr)
		return FALSE;
	if (m_pDataFile->GetDOCchanLength() <= 0)
		return FALSE;

	// check intervals	(assume m_lxSize OK)
	if (m_lxFirst < 0)
		m_lxFirst = 0;								// avoid negative start
	m_lxLast = m_lxFirst + m_lxSize - 1;			// test end
	if (m_lxLast > m_lxVeryLast)					// past end of file?
	{
		if (m_lxSize >= m_lxVeryLast + 1)
			m_lxSize = m_lxVeryLast + 1;
		m_lxLast = m_lxVeryLast;					// clip to end
		m_lxFirst = m_lxLast - m_lxSize + 1;		// change start
	}

	// max nb of points spanning around raw data pt stored in array(0)
	if (envelope_ptr_array.GetSize() < 1)
		return FALSE;
	auto p_cont = envelope_ptr_array.GetAt(0);
	const auto nspan = p_cont->GetDocbufferSpan();	// additional pts necessary

	// loop through all pixels if data buffer is longer than data displayed
	// within one pixel...
	auto l_first = m_lxFirst;						// start
	const auto ipixelmax = m_scale.GetnIntervals();	// max pixel
	const auto nchans = m_pDataFile->GetScanCount();

	for (auto ipixel = 0; ipixel < ipixelmax; ipixel++)
	{
		// compute file index of pts within current pixel
		const auto data_within_1_pixel = m_scale.GetIntervalSize(ipixel); // size first pixel
		const int l_last = l_first + data_within_1_pixel - 1;
		auto b_new = TRUE;							// flag to tell routine that it should start from new data

		while (l_first <= l_last)
		{
			auto lBUFchanFirst = l_first;			// index very first pt within pixel
			long lBUFchanLast = l_last;				// index very last pixel

			 // ask document to read raw data, document returns index of data loaded within the buffer
			if (!m_pDataFile->LoadRawData(&lBUFchanFirst, &lBUFchanLast, nspan))
				break;								// exit if error reported

			// build Envelopes  .................
			if (lBUFchanLast > l_last)
				lBUFchanLast = l_last;
			const int npoints = lBUFchanLast - l_first + 1;
			if (npoints <= 0)
				break;

			// loop over each envelope
			for (auto i_envelope = envelope_ptr_array.GetUpperBound(); i_envelope > 0; i_envelope--)
			{
				p_cont = envelope_ptr_array.GetAt(i_envelope);

				const auto source_chan = p_cont->GetSourceChan();// get source channel
				const auto itransf = p_cont->GetSourceMode();	// get transform mode
				if (itransf > 0)						// if transformation, compute transf
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

BOOL CChartDataWnd::GetSmoothDataFromDoc(int ioption)
{
	// get document parameters: exit if empty document
	if (m_bADbuffers || m_pDataFile == nullptr)
		return FALSE;
	if (m_pDataFile->GetDOCchanLength() <= 0)
		return FALSE;

	// check intervals	(assume m_lxSize OK)
	if (m_lxFirst < 0)
		m_lxFirst = 0;							// avoid negative start
	m_lxLast = m_lxFirst + m_lxSize - 1;		// test end
	if (m_lxLast > m_lxVeryLast)				// past end of file?
	{
		if (m_lxSize >= m_lxVeryLast + 1)
			m_lxSize = m_lxVeryLast + 1;
		m_lxLast = m_lxVeryLast;				// clip to end
		m_lxFirst = m_lxLast - m_lxSize + 1;		// change start
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

	for (int pixel = 0; pixel < ipixelmax; pixel++)
	{
		// compute file index of pts within current pixel
		const auto data_within_1_pixel = m_scale.GetIntervalSize(pixel); // size first pixel
		const auto l_last = l_first + data_within_1_pixel - 1;
		auto b_new = TRUE;						// flag to tell routine that it should start from new data

		while (l_first <= l_last)
		{
			auto l_buf_chan_first = l_first;			// index very first pt within pixel
			auto l_buf_chan_last = l_last;			// index very last pixel

			 // ask document to read raw data, document returns index of data loaded within the buffer
			if (!m_pDataFile->LoadRawData(&l_buf_chan_first, &l_buf_chan_last, nspan))
				break;							// exit if error reported

			// build Envelopes  .................
			if (l_buf_chan_last > l_last)
				l_buf_chan_last = l_last;
			const int npoints = l_buf_chan_last - l_first + 1;
			if (npoints <= 0)
				break;

			// loop over each envelope
			for (auto i_envelope = envelope_ptr_array.GetUpperBound(); i_envelope > 0; i_envelope--)
			{
				p_cont = envelope_ptr_array.GetAt(i_envelope);

				const auto source_chan = p_cont->GetSourceChan();
				const auto itransf = p_cont->GetSourceMode();
				int intervals = nchans;
				if (itransf > 0)						// if transformation, compute transf
				{										// and then build envelope
					lp_data = m_pDataFile->LoadTransfData(l_first, l_buf_chan_last, itransf, source_chan);
					intervals = 1;
				}
				else									// no transformation: compute max min
					lp_data = m_pDataFile->GetpRawDataElmt(source_chan, l_first);
				p_cont->FillEnvelopeWithSmoothMxMi(pixel, lp_data, intervals, npoints, b_new, ioption);
			}
			b_new = FALSE;
			l_first = l_buf_chan_last + 1;
		}
	}
	return TRUE;
}

BOOL CChartDataWnd::GetDataFromDoc(long l_first)
{
	if (l_first == m_lxFirst)
		return TRUE;
	m_lxFirst = l_first;
	return GetDataFromDoc();
}

BOOL CChartDataWnd::GetDataFromDoc(long l_first, long l_last)
{
	// check if size has changed
	if (l_first < 0) 				// first check limits across document's
		l_first = 0;				// minimum
	if (l_last > m_lxVeryLast)	// maximum
		l_last = m_lxVeryLast;
	if ((l_first > m_lxVeryLast) || (l_last < l_first))
	{							// shuffled intervals
		l_first = 0;
		if (m_lxSize > m_lxVeryLast + 1)
			m_lxSize = m_lxVeryLast + 1;
		l_last = l_first + m_lxSize - 1;
	}

	// requested size different from current one?
	if (m_lxSize != (l_last - l_first + 1))
	{
		m_lxFirst = l_first;
		ResizeChannels(0, (l_last - l_first + 1));		// n pixels = m_npixels
	}
	// load data
	m_lxFirst = l_first;
	m_lxLast = l_last;
	return GetDataFromDoc();
}

BOOL CChartDataWnd::ScrollDataFromDoc(WORD nSBCode)
{
	auto l_first = m_lxFirst;
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
		l_first = m_lxVeryLast - m_lxSize + 1;
		break;
	default:
		return FALSE;
	}
	m_lxFirst = l_first;
	return GetDataFromDoc();
}

void CChartDataWnd::UpdatePageLineSize()
{
	if (m_pDataFile != nullptr)
		m_lxPage = m_lxSize;
	else
		m_lxPage = m_lxSize / 10;
	if (m_lxPage == 0)
		m_lxPage = 1;
	m_lxLine = m_lxPage / m_npixels;
	if (m_lxLine == 0)
		m_lxLine = 1;
}

void CChartDataWnd::ZoomData(CRect* r1, CRect* r2)
{
	r1->NormalizeRect();
	r2->NormalizeRect();

	// change gain & offset of all channels:
	for (auto i = chanlistitem_ptr_array.GetUpperBound(); i >= 0; i--)	// scan all channels
	{
		// display loop: load abcissa
		auto chanlist_item = chanlistitem_ptr_array[i];
		const auto extent = chanlist_item->GetYextent();
		const auto newext = MulDiv(extent, r2->Height(), r1->Height());
		chanlist_item->SetYextent(newext);
		const auto zero = chanlist_item->GetYzero();
		const auto newzero = zero - (r1->Height() - r2->Height()) / 2;
		chanlist_item->SetYzero(newzero);
	}

	// change index of first and last pt displayed
	auto l_size = m_lxLast - m_lxFirst + 1;
	auto l_first = m_lxFirst + (l_size * (r2->left - r1->left)) / r1->Width();
	l_size = (l_size * r2->Width()) / r1->Width();
	if (l_size > m_lxVeryLast)
	{
		l_size = m_lxVeryLast;
		l_first = 0;
	}
	const auto l_last = l_first + l_size - 1;

	// call for new data only if indexes are different
	if (l_first != m_lxFirst || l_last != m_lxLast)
	{
		ResizeChannels(0, l_last - l_first + 1);
		GetDataFromDoc(l_first, l_last);
	}
	postMyMessage(HINT_VIEWSIZECHANGED, NULL);
}

void CChartDataWnd::UpdateXRuler()
{
	if (m_bNiceGrid)
	{
		if (m_pXRulerBar != nullptr)
		{
			auto first = m_lxFirst / m_samplingrate;
			auto last = m_lxLast / m_samplingrate;
			m_xRuler.UpdateRange(first, last);
		}
	}
}

void CChartDataWnd::UpdateYRuler()
{
	if (m_bNiceGrid)
	{
		if (m_pYRulerBar != nullptr)
		{
			const auto binlow = GetChanlistPixeltoBin(0, 0);
			const auto binhigh = GetChanlistPixeltoBin(0, m_clientRect.Height());
			CChanlistItem* pchan = GetChanlistItem(0);
			auto yfirst = pchan->ConvertDataBinsToVolts(binlow);
			auto ylast = pchan->ConvertDataBinsToVolts(binhigh);
			m_yRuler.UpdateRange(yfirst, ylast);
		}
	}
}

void CChartDataWnd::PlotDatatoDC(CDC* p_dc)
{
	if (m_bADbuffers)
		return;
	//ATLTRACE2("start PlotDataToDC \n");
	// erase background
	if (m_erasebkgnd)
	{
		UpdateXRuler();
		UpdateYRuler();
		EraseBkgnd(p_dc);
	}

	auto rect = m_displayRect;
	rect.DeflateRect(1, 1);
	p_dc->SelectObject(&m_hFont);

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
		p_dc->DrawText(m_csBottomComment, textlen, rect, DT_RIGHT | DT_BOTTOM | DT_SINGLELINE);
	}

	// save DC
	const auto n_saved_dc = p_dc->SaveDC();
	ASSERT(n_saved_dc != 0);
	p_dc->IntersectClipRect(&m_displayRect);

	// prepare DC
	p_dc->SetMapMode(MM_ANISOTROPIC);
	p_dc->SetViewportOrg(m_displayRect.left, m_displayRect.Height() / 2);
	p_dc->SetViewportExt(m_displayRect.Width(), -m_displayRect.Height());
	p_dc->SetWindowExt(m_displayRect.Width(), m_displayRect.Height());

	p_dc->SetWindowOrg(0, 0);

	// display all channels
	auto worg = -1;		// force origin
	auto wext = -1;		// force <= yextent
	const auto yVE = m_displayRect.Height();
	CEnvelope* pX = nullptr;
	auto nelements = 0;

	auto color = BLACK_COLOR;
	const auto poldpen = p_dc->SelectObject(&m_penTable[color]);

	// display loop:
	for (auto ichan = chanlistitem_ptr_array.GetUpperBound(); ichan >= 0; ichan--)	// scan all channels
	{
		const auto chanlist_item = chanlistitem_ptr_array[ichan];
		wext = chanlist_item->GetYextent();
		worg = chanlist_item->GetYzero();
		p_dc->SelectObject(&m_penTable[chanlist_item->GetColor()]);
		if (chanlist_item->GetPenWidth() == 0)
			continue;

		if (pX != chanlist_item->pEnvelopeAbcissa)
		{
			pX = chanlist_item->pEnvelopeAbcissa;
			nelements = pX->GetEnvelopeSize();
			if (m_PolyPoints.GetSize() != nelements * 2)
				m_PolyPoints.SetSize(nelements * 2);
			pX->ExportToAbcissa(m_PolyPoints);
		}

		auto pY = chanlist_item->pEnvelopeOrdinates;
		pY->ExportToOrdinates(m_PolyPoints);

		for (auto j = 0; j < nelements; j++) {
			const auto p_point = &m_PolyPoints[j];
			p_point->y = MulDiv(short(p_point->y) - worg, yVE, wext);
		}
		p_dc->MoveTo(m_PolyPoints[0]);
		p_dc->Polyline(&m_PolyPoints[0], nelements);

		if (m_HZtags.GetNTags() > 0)
			displayHZtags_Chan(p_dc, ichan, chanlist_item);

		highlightData(p_dc, ichan);
	}

	// restore DC
	p_dc->SelectObject(poldpen);
	p_dc->RestoreDC(n_saved_dc);

	if (m_VTtags.GetNTags() > 0)
		displayVTtags_LValue(p_dc);

	// temp tag
	if (m_hwndReflect != nullptr && m_tempVTtag != nullptr)
	{
		const auto poldp = p_dc->SelectObject(&m_blackDottedPen);
		const auto nold_rop = p_dc->SetROP2(R2_NOTXORPEN);
		p_dc->MoveTo(m_tempVTtag->m_pixel, 2);
		p_dc->LineTo(m_tempVTtag->m_pixel, m_displayRect.bottom - 2);
		p_dc->SetROP2(nold_rop);
		p_dc->SelectObject(poldp);
	}
	//ATLTRACE2("end PlotDataToDC \n");
}

void CChartDataWnd::displayHZtags_Chan(CDC* p_dc, int ichan, CChanlistItem* pChan)
{
	const auto pold = p_dc->SelectObject(&m_blackDottedPen);
	const auto nold_rop = p_dc->SetROP2(R2_NOTXORPEN);
	auto wext = pChan->GetYextent();
	auto worg = pChan->GetYzero();
	const auto yVE = m_displayRect.Height();
	for (auto i = m_HZtags.GetNTags() - 1; i >= 0; i--)
	{
		if (m_HZtags.GetChannel(i) != ichan)
			continue;
		//ATLTRACE2("display HZtag %i \n", i);
		auto k = m_HZtags.GetValue(i);
		k = MulDiv(short(k) - worg, yVE, wext);
		p_dc->MoveTo(m_displayRect.left, k);
		p_dc->LineTo(m_displayRect.right, k);
	}
	p_dc->SetROP2(nold_rop);
	p_dc->SelectObject(pold);
}

void CChartDataWnd::displayVTtags_LValue(CDC* p_dc)
{
	const auto oldp = p_dc->SelectObject(&m_blackDottedPen);
	const auto nold_rop = p_dc->SetROP2(R2_NOTXORPEN);
	const auto y0 = 0;
	const int y1 = m_displayRect.bottom;

	for (auto j = m_VTtags.GetNTags() - 1; j >= 0; j--)
	{
		const auto lk = m_VTtags.GetTagLVal(j);
		if (lk <m_lxFirst || lk > m_lxLast)
			continue;
		const auto llk = (lk - m_lxFirst) * float(m_displayRect.Width()) / (m_lxLast - m_lxFirst + 1);
		const auto k = int(llk);
		p_dc->MoveTo(k, y0);
		p_dc->LineTo(k, y1);
	}

	p_dc->SelectObject(oldp);
	p_dc->SetROP2(nold_rop);
}

void CChartDataWnd::OnSize(UINT nType, int cx, int cy)
{
	CChartWnd::OnSize(nType, cx, cy);
	if (!IsDefined() || m_pDataFile == nullptr)
		return;

	ResizeChannels(cx - 1, 0);
	if (!m_bADbuffers)
	{
		GetDataFromDoc();
	}
}

void CChartDataWnd::Print(CDC* p_dc, CRect* pRect, BOOL bCenterLine)
{
	// save DC & old client rect
	const auto n_saved_dc = p_dc->SaveDC();
	ASSERT(n_saved_dc != 0);
	const auto old_rect = m_clientRect;

	// prepare DC
	const auto previousmapping = p_dc->SetMapMode(MM_TEXT);	// change map mode to text (1 pixel = 1 logical point)
	m_clientRect = *pRect; //CRect(0,0, pRect->GetRectWidth(), pRect->GetRectHeight());
	AdjustDisplayRect(pRect);
	EraseBkgnd(p_dc);
	// clip curves
	if (m_scopestruct.bClipRect)
		p_dc->IntersectClipRect(m_displayRect);
	else
		p_dc->SelectClipRgn(nullptr);

	// adjust coordinates for anisotropic mode
	const auto yVE = -m_displayRect.Height();
	const int yVO = m_displayRect.top + m_displayRect.Height() / 2;
	const auto xVE = m_displayRect.Width();
	const int xVO = m_displayRect.left;

	// exit if no data defined
	if (!IsDefined())
	{
		p_dc->TextOut(10, 10, _T("No data"));
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
	auto nelements = 0;
	auto pX = chanlistitem_ptr_array[0]->pEnvelopeAbcissa;
	const BOOL b_poly_line = (p_dc->m_hAttribDC == nullptr) || (p_dc->GetDeviceCaps(LINECAPS) & LC_POLYLINE);
	auto color = BLACK_COLOR;
	const auto poldpen = p_dc->SelectObject(&m_penTable[color]);

	// display loop:
	for (auto ichan = chanlistitem_ptr_array.GetUpperBound(); ichan >= 0; ichan--)	// scan all channels
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
		const auto yzero = chanlist_item->GetYzero();
		if (chanlist_item->GetColor() != color)
		{
			color = chanlist_item->GetColor();
			p_dc->SelectObject(&m_penTable[color]);
		}
		// transform ordinates ------------------------------------------------
		for (auto j = 0; j < nelements; j++)
		{
			const auto p_point = &m_PolyPoints[j];
			p_point->y = MulDiv(p_point->y - yzero, yVE, yextent) + yVO;
		}
		//  display points ----------------------------------------------------
		if (b_poly_line)
			p_dc->Polyline(&m_PolyPoints[0], nelements);		// draw curve
		else
		{
			p_dc->MoveTo(m_PolyPoints[0]);					// move pen to first pair of coords
			for (auto j = 0; j < nelements; j++)
				p_dc->LineTo(m_PolyPoints[j]);				// draw lines
		}

		//display associated cursors ------------------------------------------
		if (m_HZtags.GetNTags() > 0)					// print HZ cursors if any?
		{
			// select pen and display mode
			CPen ltgrey_pen(PS_SOLID, 0, m_colorTable[SILVER_COLOR]);
			const auto pold_pen = p_dc->SelectObject(&ltgrey_pen);
			// iterate through HZ cursor list
			const int x0 = pRect->left;
			const int x1 = pRect->right;
			for (auto j = m_HZtags.GetNTags() - 1; j >= 0; j--)
			{
				if (m_HZtags.GetChannel(j) != ichan)	// next tag if not associated with
					continue;					// current channel
				auto k = m_HZtags.GetValue(j);
				k = MulDiv(k - yzero, yVE, yextent) + yVO;
				p_dc->MoveTo(x0, k);				// set initial pt
				p_dc->LineTo(x1, k);				// HZ line
			}
			p_dc->SelectObject(pold_pen);
		}
		// highlight data ------------------------------------------------------
		highlightData(p_dc, ichan);
	}

	// display vertical cursors ------------------------------------------------
	if (m_VTtags.GetNTags() > 0)
	{
		// select pen and display mode
		CPen ltgrey_pen(PS_SOLID, 0, m_colorTable[SILVER_COLOR]);
		const auto poldp = p_dc->SelectObject(&ltgrey_pen);
		// iterate through VT cursor list
		const int y0 = pRect->top;
		const int y1 = pRect->bottom;
		const int k0 = pRect->left;
		const int ksize = pRect->right - k0;
		for (auto j = m_VTtags.GetNTags() - 1; j >= 0; j--)
		{
			const auto lk = m_VTtags.GetTagLVal(j);	// get value
			if (lk <m_lxFirst || lk > m_lxLast)
				continue;
			const int k = k0 + (lk - m_lxFirst) * ksize / (m_lxLast - m_lxFirst + 1);
			p_dc->MoveTo(k, y0);			// set initial pt
			p_dc->LineTo(k, y1);			// VT line
		}
		p_dc->SelectObject(poldp);
	}

	// restore DC ----------------------------------------------------------------
	p_dc->SelectObject(poldpen);			// restore old pen
	p_dc->RestoreDC(n_saved_dc);			// restore DC
	p_dc->SetMapMode(previousmapping);	// restore map mode
	m_clientRect = old_rect;
	AdjustDisplayRect(&m_clientRect);
}

BOOL CChartDataWnd::CopyAsText(int ioption, int iunit, int nabcissa)
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
		const auto h_copy = static_cast<HANDLE>(::GlobalAlloc(GHND, dw_len));
		const auto pwave_format = m_pDataFile->GetpWaveFormat();

		if (h_copy != nullptr)
		{
			// memory allocated -- get pointer to it
			auto lp_copy = static_cast<LPTSTR>(::GlobalLock(h_copy));

			// data file name, comment, header
			const auto date = (pwave_format->acqtime).Format(_T("%#d %B %Y %X"));
			StringCchPrintfEx(lp_copy, pcch_remaining, &lp_copy, &pcch_remaining, STRSAFE_NULL_ON_FAILURE, _T("%s\t%s\r\n"), static_cast<LPCTSTR>(m_pDataFile->GetPathName()), static_cast<LPCTSTR>(date));
			StringCchPrintfEx(lp_copy, pcch_remaining, &lp_copy, &pcch_remaining, STRSAFE_NULL_ON_FAILURE, _T("%s\t\r\n"), static_cast<LPCTSTR>(pwave_format->GetComments(_T("\t"), 0)));
			// time interval
			auto tt = GetDataFirst() / pwave_format->chrate;		// first interval (sec)
			StringCchPrintfEx(lp_copy, pcch_remaining, &lp_copy, &pcch_remaining, STRSAFE_NULL_ON_FAILURE, _T("time start(s):\t%f\r\n"), tt);
			tt = GetDataLast() / pwave_format->chrate;			// last interval (sec)
			StringCchPrintfEx(lp_copy, pcch_remaining, &lp_copy, &pcch_remaining, STRSAFE_NULL_ON_FAILURE, _T("time end(s):\t%f"), tt);
			if (iunit == 0)
			{
				// zero volt for document
				StringCchPrintfEx(lp_copy, pcch_remaining, &lp_copy, &pcch_remaining, STRSAFE_NULL_ON_FAILURE, _T("\r\nzero volts(bin):\t%i\r\n"), 0 /*pwaveFormat->binzero*/);
				// unit for each channel
				StringCchPrintfEx(lp_copy, pcch_remaining, &lp_copy, &pcch_remaining, STRSAFE_NULL_ON_FAILURE, _T("mvolts per bin:\r\n"));
				for (auto i = 0; i < GetChanlistSize(); i++)
					StringCchPrintfEx(lp_copy, pcch_remaining, &lp_copy, &pcch_remaining, STRSAFE_NULL_ON_FAILURE, _T("%f\t"), double(GetChanlistItem(i)->GetVoltsperDataBin()) * 1000.f);
				lp_copy--;	// erase last tab
			}

			// comment for each channel
			StringCchPrintfEx(lp_copy, pcch_remaining, &lp_copy, &pcch_remaining, STRSAFE_NULL_ON_FAILURE, _T("\r\nchan title:\r\n"));
			for (auto i = 0; i < GetChanlistSize(); i++)
				StringCchPrintfEx(lp_copy, pcch_remaining, &lp_copy, &pcch_remaining, STRSAFE_NULL_ON_FAILURE, _T("%s\t"), static_cast<LPCTSTR>(GetChanlistItem(i)->GetComment()));
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
			SetClipboardData(CF_TEXT, h_copy);
		}
		CloseClipboard();							// close connect w.clipboard
		ResizeChannels(oldcx, 0);					// restore initial conditions
		GetDataFromDoc();
		EndWaitCursor();
	}
	return flag;
}

LPTSTR CChartDataWnd::GetAsciiEnvelope(LPTSTR lpCopy, int iunit)
{
	// time intervals
	const auto ichans = chanlistitem_ptr_array.GetUpperBound();
	const auto npoints = envelope_ptr_array.GetAt(0)->GetEnvelopeSize();
	// loop through all points
	for (auto j = 0; j < npoints; j++)
	{
		// loop through all channels
		for (auto i = 0; i <= ichans; i++)	// scan all channels
		{
			const auto chanlist_item = chanlistitem_ptr_array[i];
			const int k = (chanlist_item->pEnvelopeOrdinates)->GetPointAt(j);
			if (iunit == 1)
			{
				lpCopy += wsprintf(lpCopy, _T("%f\t"), double(k) * GetChanlistItem(i)->GetVoltsperDataBin() * 1000.f);
			}
			else
				lpCopy += wsprintf(lpCopy, _T("%i\t"), k);
		}
		lpCopy--;
		*lpCopy = 0xD; 		//CR
		*lpCopy++ = 0xA; 	//LF
	}
	*lpCopy = _T('\0');
	lpCopy++;
	return lpCopy;
}

LPTSTR CChartDataWnd::GetAsciiLine(LPTSTR lpCopy, int iunit)
{
	// time intervals
	const auto ichans = chanlistitem_ptr_array.GetUpperBound();
	const auto npoints = envelope_ptr_array.GetAt(0)->GetEnvelopeSize();
	// loop through all points
	for (auto j = 0; j < npoints; j += m_dataperpixel)
	{
		// loop through all channels
		for (auto i = 0; i <= ichans; i++)	// scan all channels
		{
			const auto chanlist_item = chanlistitem_ptr_array[i];
			int k = (chanlist_item->pEnvelopeOrdinates)->GetPointAt(j);
			if (m_dataperpixel > 1)
			{
				k += (chanlist_item->pEnvelopeOrdinates)->GetPointAt(j + 1);
				k = k / 2;
			}
			if (iunit == 1)
				lpCopy += wsprintf(lpCopy, _T("%f\t"), double(k) * GetChanlistItem(i)->GetVoltsperDataBin() * 1000.f);
			else
				lpCopy += wsprintf(lpCopy, _T("%i\t"), k);
		}
		lpCopy--;
		*lpCopy = 0xD; 		//CR
		*lpCopy++ = 0xA; 	//LF
	}
	*lpCopy = _T('\0');
	lpCopy++;
	return lpCopy;
}

// XORcurve
// in order to move a curve vertically with the cursor, a special envelope is
// stored within the p_data array and displayed using XOR mode.
// this curve has 2 times less points (median) to speed up the display
// Although CClientDC is claimed as attached to the client area of the button
// ("this"), moving the curve along the vertical direction will draw outside of
// the client area, suggesting that it is necessary to clip the client area...
// this code was also tested with m_dibSurf: XOR to DIB surface, then redraw the
// client area by fast BitBlt. This latter method was less efficient (slower)
// than XORing directly to the screen.

void CChartDataWnd::curveXOR()
{
	// ------- client area (direct draw)
	auto p_dc = CWnd::GetDC();			// select dc
	const auto n_saved_dc = p_dc->SaveDC();		// preserve current settings
	CPen temp_pen;
	temp_pen.CreatePen(PS_SOLID, 0, m_colorTable[SILVER_COLOR]);
	const auto poldpen = p_dc->SelectObject(&temp_pen); // load pen
	p_dc->IntersectClipRect(&m_displayRect);

	p_dc->SetMapMode(MM_ANISOTROPIC);	// prepare display with appropriate
	p_dc->SetViewportOrg(m_displayRect.left, m_yVO);		// scaling
	p_dc->SetViewportExt(GetRectWidth(), m_yVE);
	p_dc->SetWindowExt(m_XORxext, m_XORyext);
	p_dc->SetWindowOrg(0, 0);

	// display envelope store into p_data using XOR mode
	const auto nold_rop = p_dc->SetROP2(R2_NOTXORPEN);
	p_dc->MoveTo(m_PolyPoints[0]);
	p_dc->Polyline(&m_PolyPoints[0], m_XORnelmts);
	p_dc->SetROP2(nold_rop);

	p_dc->SelectObject(poldpen);			// restore resources - pen
	p_dc->RestoreDC(n_saved_dc);			// restore DC in previous mode
	ReleaseDC(p_dc);						// release DC
	temp_pen.DeleteObject();
}

void CChartDataWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	// convert chan values stored within HZ tags into pixels
	if (m_HZtags.GetNTags() > 0)
	{
		for (auto icur = 0; icur < m_HZtags.GetNTags(); icur++)
		{
			const auto pixval = GetChanlistBintoPixel(
				m_HZtags.GetChannel(icur),
				m_HZtags.GetValue(icur));
			m_HZtags.SetTagPix(icur, pixval);
		}
	}

	if (m_VTtags.GetNTags() > 0)
	{
		m_liFirst = m_lxFirst;
		m_liLast = m_lxLast;
	}

	// call base class to test for horiz cursor or XORing rectangle
	CChartWnd::OnLButtonDown(nFlags, point);

	// if cursor mode = 0 and no tag hit detected, mouse mode=track rect
	// test curve hit -- specific to lineview, if hit curve, track curve instead
	if (m_currCursorMode == 0 && m_HCtrapped < 0)	// test if cursor hits a curve
	{
		m_trackMode = TRACK_RECT;
		m_hitcurve = doesCursorHitCurve(point);
		if (m_hitcurve >= 0)
		{
			// cancel track rect mode (cursor captured)
			m_trackMode = TRACK_CURVE;						// flag: track curve

			// modify polypoint and prepare for XORing curve tracked with mouse
			const auto chanlist_item = chanlistitem_ptr_array[m_hitcurve];
			auto pX = chanlist_item->pEnvelopeAbcissa;		// display: load abcissa
			pX->GetMeanToAbcissa(m_PolyPoints);
			m_XORnelmts = pX->GetEnvelopeSize() / 2;		// nb of elements
			m_XORxext = pX->GetnElements() / 2;				// extent

			auto pY = chanlist_item->pEnvelopeOrdinates;	// load ordinates
			pY->GetMeanToOrdinates(m_PolyPoints);
			m_XORyext = chanlist_item->GetYextent();		// store extent
			m_zero = chanlist_item->GetYzero();				// store zero
			m_ptFirst = point;								// save first point
			m_curTrack = m_zero;							// use m_curTrack to store zero

			curveXOR();										// xor curve
			postMyMessage(HINT_HITCHANNEL, m_hitcurve);		// tell parent chan selected
			return;
		}
	}

	// if horizontal cursor hit -- specific .. deal with variable gain
	if (m_trackMode == TRACK_HZTAG)
	{
		const auto chanlist_item = chanlistitem_ptr_array[m_HZtags.GetChannel(m_HCtrapped)];
		m_yWE = chanlist_item->GetYextent();				// store extent
		m_yWO = chanlist_item->GetYzero();					// store zero
	}
}

void CChartDataWnd::OnMouseMove(UINT nFlags, CPoint point)
{
	switch (m_trackMode)
	{
	case TRACK_CURVE:
		curveXOR();					// erase past curve and compute new zero
		m_zero = MulDiv(point.y - m_ptFirst.y, m_XORyext, -m_yVE) + m_curTrack;
		curveXOR();					// plot new curve
		break;

	default:
		CChartWnd::OnMouseMove(nFlags, point);
		break;
	}
}

void CChartDataWnd::OnLButtonUp(UINT nFlags, CPoint point)
{
	releaseCursor();
	switch (m_trackMode)
	{
	case TRACK_CURVE:
	{
		curveXOR();	// (clear) necessary since XORcurve can draw outside client area
		auto chanlist_item = chanlistitem_ptr_array[m_hitcurve];
		chanlist_item->SetYzero(m_zero);
		m_trackMode = TRACK_OFF;
		postMyMessage(HINT_HITCHANNEL, m_hitcurve);	// tell parent chan selected
		Invalidate();
	}
	break;

	case TRACK_HZTAG:
		lbuttonUp_HzTag(nFlags, point);
		break;

	case TRACK_VTTAG:
	{
		// convert pix into data value and back again
		const auto lval = long(point.x) * (m_liLast - m_liFirst + 1) / long(m_displayRect.right) + m_liFirst;
		m_VTtags.SetTagLVal(m_HCtrapped, lval);
		point.x = int((lval - m_liFirst) * long(m_displayRect.right) / (m_liLast - m_liFirst + 1));
		XorVTtag(point.x);
		postMyMessage(HINT_CHANGEVERTTAG, m_HCtrapped);
		m_trackMode = TRACK_OFF;
	}
	break;

	case TRACK_RECT:
	{
		// skip too small a rectangle (5 pixels?)
		CRect rect_out(m_ptFirst.x, m_ptFirst.y, m_ptLast.x, m_ptLast.y);
		const auto jitter = 3;
		const BOOL b_rect_ok = ((abs(rect_out.Height()) > jitter) || (abs(rect_out.Width()) > jitter));

		// perform action according to cursor type
		auto rect_in = m_displayRect;
		switch (m_cursorType)
		{
		case 0:		// if no cursor, no curve track, then move display
			if (b_rect_ok)
			{
				invertTracker(point);
				rect_out = rect_in;
				rect_out.OffsetRect(m_ptFirst.x - m_ptLast.x, m_ptFirst.y - m_ptLast.y);
				ZoomData(&rect_in, &rect_out);
			}
			break;
		case CURSOR_ZOOM: 	 // zoom operation
			if (b_rect_ok)
			{
				ZoomData(&rect_in, &rect_out);
				m_ZoomFrom = rect_in;
				m_ZoomTo = rect_out;
				m_iUndoZoom = 1;
			}
			else
				zoomIn();
			postMyMessage(HINT_SETMOUSECURSOR, m_oldcursorType);
			break;
		case CURSOR_CROSS:
			postMyMessage(HINT_DEFINEDRECT, NULL);
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

int CChartDataWnd::doesCursorHitCurve(CPoint point)
{
	auto chanfound = -1;						// output value
	const auto ichans = chanlistitem_ptr_array.GetUpperBound();
	auto chanlist_item = chanlistitem_ptr_array[0]->pEnvelopeAbcissa;
	const auto xextent = chanlist_item->GetnElements();		// n elements stored in one Envelope
	int index1 = point.x - m_cxjitter;		// horizontal jitter backwards
	auto index2 = index1 + m_cxjitter;		// horiz jitter forwards
	if (index1 < 0) index1 = 0;				// clip to windows limits
	if (index2 > (GetRectWidth() - 1)) index2 = GetRectWidth() - 1;
	// convert index1 into Envelope indexes
	index1 = index1 * m_dataperpixel;		// start from
	index2 = (index2 + 1) * m_dataperpixel;	// stop at
	// special case when less pt than pixels
	if (index1 == index2)
	{
		index1--;
		if (index1 < 0) index1 = 0;
		index2++;
		if (index2 > xextent) index2 = xextent;
	}
	// loop through all channels
	for (auto chan = 0; chan <= ichans; chan++)	// scan all channels
	{
		// convert device coordinates into value
		const auto ival = GetChanlistPixeltoBin(chan, point.y);
		const auto ijitter = MulDiv(m_cyjitter, GetChanlistItem(chan)->GetYextent(), -m_yVE);
		const auto valmax = ival + ijitter;			// mouse max
		const auto valmin = ival - ijitter;			// mouse min
		chanlist_item = chanlistitem_ptr_array[chan]->pEnvelopeOrdinates;

		// loop around horizontal jitter...
		for (auto index = index1; index < index2 && chanfound < 0; index++)
		{
			int kmax = chanlist_item->GetPointAt(index);	// get chan Envelope data point
			// special case: one point per pixel
			if (m_dataperpixel == 1)
			{	// more than min AND less than max
				if (kmax >= valmin && kmax <= valmax)
				{
					chanfound = chan;
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
					chanfound = chan;
					break;
				}
			}
		}
	}
	return chanfound;
}

void CChartDataWnd::MoveHZtagtoVal(int i, int val)
{
	const auto chan = m_HZtags.GetChannel(i);
	const auto chanlist_item = chanlistitem_ptr_array[chan];
	m_XORyext = chanlist_item->GetYextent();
	m_zero = chanlist_item->GetYzero();
	m_ptLast.y = MulDiv(m_HZtags.GetValue(i) - m_zero, m_yVE, m_XORyext) + m_yVO;
	CPoint point;
	point.y = MulDiv(val - m_zero, m_yVE, m_XORyext) + m_yVO;
	XorHZtag(point.y);
	m_HZtags.SetTagVal(i, val);
}

void CChartDataWnd::SetHighlightData(CHighLight& source)
{
	m_highlighted = source;
}

void CChartDataWnd::SetHighlightData(CDWordArray* pDWintervals)
{
	m_highlighted.l_first.RemoveAll();
	m_highlighted.l_last.RemoveAll();
	if (pDWintervals == nullptr)
		return;

	m_highlighted.channel = pDWintervals->GetAt(0);
	m_highlighted.color = static_cast<COLORREF>(pDWintervals->GetAt(1));
	m_highlighted.pensize = pDWintervals->GetAt(2);
	const auto size = (pDWintervals->GetSize() - 3) / 2;
	m_highlighted.l_first.SetSize(size);
	m_highlighted.l_last.SetSize(size);

	for (auto i = 3; i < pDWintervals->GetSize(); i += 2)
	{
		m_highlighted.l_first.Add(pDWintervals->GetAt(i));
		m_highlighted.l_last.Add(pDWintervals->GetAt(i + 1));
	}
}

void CChartDataWnd::highlightData(CDC* p_dc, int chan)
{
	// skip if not the correct chan
	if (chan != m_highlighted.channel || m_highlighted.l_first.GetSize() < 2)
		return;

	// get color and pen size from array m_pDWintervals
	CPen new_pen;
	new_pen.CreatePen(PS_SOLID, m_highlighted.pensize, m_highlighted.color);
	const auto poldpen = static_cast<CPen*>(p_dc->SelectObject(&new_pen));
	const BOOL b_poly_line = (p_dc->m_hAttribDC == nullptr)
		|| (p_dc->GetDeviceCaps(LINECAPS) & LC_POLYLINE);

	// loop to display data

	// pointer to descriptor
	for (auto i = 0; i < m_highlighted.l_first.GetSize(); i++)
	{
		// load ith interval values
		auto l_first = m_highlighted.l_first[i];		// first value
		auto l_last = m_highlighted.l_last[i];		// last value

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
			p_dc->Polyline(&m_PolyPoints[ifirst], nelements);
		else
		{
			p_dc->MoveTo(m_PolyPoints[ifirst]);
			for (auto j = 0; j < nelements; j++)
				p_dc->LineTo(m_PolyPoints[ifirst + j]);
		}
	}

	// restore previous pen
	p_dc->SelectObject(poldpen);
}

void CChartDataWnd::ADdisplayStart(int chsamples)
{
	// init parameters related to AD display
	m_bADbuffers = TRUE;							// yes, display ADbuffers
	m_lADbufferdone = 0;							// length of data already displayed
	auto envelope = envelope_ptr_array.GetAt(0);
	envelope->FillEnvelopeWithAbcissaEx(1, m_displayRect.right - 1, chsamples);
	envelope->ExportToAbcissa(m_PolyPoints);
	SetbUseDIB(FALSE);

	// clear window before acquiring data
	CClientDC dc(this);
	EraseBkgnd(&dc);

	// print comment
	dc.SelectObject(GetStockObject(DEFAULT_GUI_FONT));
	auto rect = m_displayRect;
	rect.DeflateRect(1, 1);
	CString cs = _T("Waiting for trigger");
	const auto textlen = cs.GetLength();
	dc.DrawText(cs, textlen, rect, DT_LEFT);
}

void CChartDataWnd::ADdisplayBuffer(short* samples_buffer, long samples_number)
{
	// check data wrap
	if (m_lADbufferdone + samples_number > m_lxSize)
	{
		const long pixels_left_until_end_of_Display = m_lxSize - m_lADbufferdone;
		if (pixels_left_until_end_of_Display > 0)
		{
			ADdisplayBuffer(samples_buffer, pixels_left_until_end_of_Display);
			samples_number -= pixels_left_until_end_of_Display;	
			samples_buffer += (pixels_left_until_end_of_Display * static_cast<long>(m_pDataFile->GetpWaveFormat()->scan_count));
		}
		m_lADbufferdone = 0;
	}

	// create device context and prepare bitmap to receive drawing commands
	CClientDC dc(this);	
	CDC dc_mem;	
	dc_mem.CreateCompatibleDC(&dc);	
	CBitmap bitmap_plot;
	const auto bitmap_old = dc_mem.SelectObject(&bitmap_plot);
	const auto dc_old = dc_mem.SaveDC();
	CDC* p_dc = &dc;
	if (m_bUseDIB)
		p_dc = &dc_mem;

	// get first and last pixels of the interval to display
	const int ad_pixel_first = MulDiv(m_lADbufferdone, m_npixels, m_lxSize);
	int ad_pixel_last = MulDiv(m_lADbufferdone + samples_number - 1, m_npixels, m_lxSize);
	if (ad_pixel_last > m_displayRect.right - 2)
		ad_pixel_last = m_displayRect.right - 2;

	const int display_pixels = ad_pixel_last - ad_pixel_first + 1;
	const int display_data_points = display_pixels * m_dataperpixel;

	//////////////////////////// prepare display ///////////////////////////////////

	const auto points_to_display = &m_PolyPoints[ad_pixel_first * m_dataperpixel * 2];

	CRect rect(ad_pixel_first, m_displayRect.top, ad_pixel_last, m_displayRect.bottom);
	if (ad_pixel_first == 0)
		rect.left = 1;
	rect.DeflateRect(0, 1);
	p_dc->IntersectClipRect(rect);
	p_dc->FillSolidRect(&rect, m_scopestruct.crScopeFill);

	auto* ppen_old = dynamic_cast<CPen*>(p_dc->SelectStockObject(BLACK_PEN));

	p_dc->SetMapMode(MM_ANISOTROPIC);
	p_dc->SetViewportExt(m_xVE, m_yVE);
	p_dc->SetViewportOrg(m_xVO, m_yVO);
	p_dc->SetWindowExt(m_npixels, m_yVE);	//chanlist_item->GetYextent());
	p_dc->SetWindowOrg(0, 0);				//chanlist_item->GetYzero());
	const auto yVE = m_yVE;

	for (int channel_number = 0; channel_number < chanlistitem_ptr_array.GetSize(); channel_number++)
	{
		// load channel descriptors
		const auto channel_item = chanlistitem_ptr_array[channel_number];
		CPen temp_pen;
		temp_pen.CreatePen(PS_SOLID, 0, m_colorTable[channel_item->GetColor()]);
		p_dc->SelectObject(&temp_pen);

		// compute max min and plot array
		auto ppt = points_to_display;
		auto p_data = samples_buffer + channel_number;
		const auto y_zero = channel_item->GetYzero();
		const auto y_extent = channel_item->GetYextent();
		const int n_channels = m_pDataFile->GetpWaveFormat()->scan_count;

		// only one data point per pixel
		if (m_dataperpixel > 1)
		{
			int number_of_elements_displayed = 0;
			for (int i = 0; i < display_pixels; i++)
			{
				auto i_max = *p_data;
				auto i_min = i_max;	
				int number_of_elements = ((i + 1) * samples_number / display_pixels) - number_of_elements_displayed;
				number_of_elements_displayed += number_of_elements;
				while (number_of_elements > 0)
				{
					const auto value_to_display = *p_data;
					if (value_to_display < i_min)
						i_min = value_to_display;
					else if (value_to_display > i_max)
						i_max = value_to_display;	
					p_data += n_channels;
					number_of_elements--;
				}

				ppt->y = MulDiv(i_max - y_zero, yVE, y_extent);
				ppt->x = i + ad_pixel_first;
				ppt++;
				ppt->y = MulDiv(i_min - y_zero, yVE, y_extent);
				ppt->x = i + ad_pixel_first;
				ppt++;
			}
		}

		// 1 point per pixel
		else
		{
			for (auto i = 0; i < display_pixels; i++, ppt++, p_data += n_channels)
			{
				ppt->y = MulDiv(*p_data - y_zero, yVE, y_extent);
				ppt->x = i + ad_pixel_first;
			}
		}
		p_dc->MoveTo(static_cast<int>(points_to_display->x - 2), static_cast<int>(points_to_display->y));
		p_dc->PolylineTo(points_to_display, display_data_points);
		temp_pen.DeleteObject();
	}

	// restore the old pen and exit
	p_dc->SelectObject(ppen_old);
	p_dc->RestoreDC(dc_old);
	p_dc->SetMapMode(MM_TEXT);
	p_dc->SetViewportOrg(0, 0);
	p_dc->SetWindowOrg(0, 0);

	if (m_bUseDIB)
		dc.BitBlt(rect.left, rect.top,
			rect.Width(), rect.Height(),
			&dc_mem,
			rect.left, rect.top,
			SRCCOPY);

	dc_mem.SelectObject(bitmap_old);
	m_lADbufferdone += samples_number;
}

void CChartDataWnd::Serialize(CArchive& ar)
{
	CChartWnd::Serialize(ar);
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

		const auto n_envelopes = envelope_ptr_array.GetSize();
		ar << n_envelopes;
		const auto n_channels = chanlistitem_ptr_array.GetSize();
		ar << n_channels;

		for (auto i = 0; i < n_envelopes; i++)
			envelope_ptr_array[i]->Serialize(ar);

		for (auto i = 0; i < n_channels; i++)
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
			for (auto i = envelope_ptr_array.GetUpperBound(); i >= nenvelopes; i--)
				delete envelope_ptr_array[i];
			envelope_ptr_array.SetSize(nenvelopes);
		}
		else if (envelope_ptr_array.GetSize() < nenvelopes)
		{
			const auto nenvelope0 = envelope_ptr_array.GetSize();
			envelope_ptr_array.SetSize(nenvelopes);
			for (auto i = nenvelope0; i < nenvelopes; i++)
				envelope_ptr_array[i] = new CEnvelope; // (CEnvelope*)
		}
		for (auto i = 0; i < nenvelopes; i++)
			envelope_ptr_array[i]->Serialize(ar);

		// ChanList array
		if (chanlistitem_ptr_array.GetSize() > nchanlistItems)
		{
			for (auto i = chanlistitem_ptr_array.GetUpperBound(); i >= nchanlistItems; i--)
				delete chanlistitem_ptr_array[i];
			chanlistitem_ptr_array.SetSize(nchanlistItems);
		}
		else if (chanlistitem_ptr_array.GetSize() < nchanlistItems)
		{
			const auto nchanlistItems0 = chanlistitem_ptr_array.GetSize();
			chanlistitem_ptr_array.SetSize(nchanlistItems);
			for (auto i = nchanlistItems0; i < nchanlistItems; i++)
				chanlistitem_ptr_array[i] = new CChanlistItem; // (CChanlistItem*)
		}
		auto ix = 0;
		auto iy = 0;
		for (auto i = 0; i < nchanlistItems; i++)
		{
			chanlistitem_ptr_array[i]->Serialize(ar);
			chanlistitem_ptr_array[i]->GetEnvelopeArrayIndexes(ix, iy);
			chanlistitem_ptr_array[i]->SetEnvelopeArrays(envelope_ptr_array.GetAt(ix), ix, envelope_ptr_array.GetAt(iy), iy);
		}
	}
}

void CChartDataWnd::SetTrackSpike(BOOL btrackspike, int tracklen, int trackoffset, int trackchannel)
{
	m_btrackspike = btrackspike;
	m_tracklen = tracklen;
	m_trackoffset = trackoffset;
	m_trackchannel = trackchannel;
}