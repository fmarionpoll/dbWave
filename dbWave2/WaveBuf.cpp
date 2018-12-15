// WaveBuf.cpp : implementation file
//

#include "StdAfx.h"
#include <math.h> 
#include "WaveBuf.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////
// CWaveBuf

IMPLEMENT_SERIAL(CWaveBuf, CObject, 0)

CWaveBuf::CWaveBuf()
{
	m_pWData = nullptr;
	m_pWTransf = nullptr;
	m_wversion=0;
	m_bTransf = FALSE;
	m_iNumElements = 0;
	m_parraySorted= nullptr;
	m_parrayCircular= nullptr;
	m_parray_size=0;
	m_dwBufferSize=0;
}

CWaveBuf::~CWaveBuf()
{
	DeleteBuffers();
	SAFE_DELETE_ARRAY(m_parraySorted);
	SAFE_DELETE_ARRAY(m_parrayCircular);
}

int	CWaveBuf::WBDatachanSetnum(const int i)
{
	// no action if the number of data channels is right
	if (m_waveFormat.scan_count == i 
		&& m_chanArray.channel_get_number() == i)
		return i;
	const auto iNumElements = m_waveFormat.buffersize / m_waveFormat.scan_count;	
	m_waveFormat.scan_count = i;
	m_chanArray.channel_set_number(i);
	ASSERT(m_chanArray.channel_get_number() == m_waveFormat.scan_count);
	CreateWBuffer(iNumElements, i);
	return m_waveFormat.scan_count;
}

short* CWaveBuf::GetWBAdrRawDataBuf() const
{
	ASSERT(m_pWData != NULL);
	return m_pWData;
}

short* CWaveBuf::GetWBAdrRawDataElmt(const int chan, const int index) const
{
	ASSERT(chan < m_waveFormat.scan_count);
	ASSERT(m_pWData != NULL);
	return m_pWData + index*m_waveFormat.scan_count + chan;
}

short* CWaveBuf::GetWBAdrTransfData() const
{return m_pWTransf;}

short* CWaveBuf::GetWBAdrTransfDataElmt(const int index) const
{return m_pWTransf + index ;}

int	CWaveBuf::GetWBNumElements() const
{return m_iNumElements; } 

int	CWaveBuf::GetWBNumChannels() const
{return m_waveFormat.scan_count;}

void CWaveBuf::SetWBSampleRate(const float fSampRate)
	{m_waveFormat.chrate = fSampRate;}

float CWaveBuf::GetWBSampleRate() const
{return m_waveFormat.chrate;}


void CWaveBuf::DeleteBuffers()
{
	if (m_pWData != nullptr)
	{
		free (m_pWData);
		m_pWData = nullptr;
	}
	if (m_pWTransf != nullptr) 
	{
		free (m_pWTransf);
		m_pWTransf = nullptr;
	}
	m_iNumElements=0;
}

// -----------------------------------------------------------
BOOL CWaveBuf::CreateWBuffer(const int i_num_elements, const int nchannels)
{
	ASSERT(this);
	ASSERT(i_num_elements >= 1); // must have at least one
	m_waveFormat.scan_count = nchannels;

	const DWORD dwSafeFactor = 256;		// safety factor to fit data in buffer
	const size_t dwBufferSize = i_num_elements * nchannels*sizeof(short) + dwSafeFactor;
	if (m_dwBufferSize != dwBufferSize)
	{
		DeleteBuffers();
		m_pWData = static_cast<short*>(malloc(dwBufferSize));
		ASSERT(m_pWData != NULL);
		if (m_pWData == nullptr)
			return FALSE;

		// allocate transform heap if selected
		if (m_bTransf)
		{
			const size_t dwBufferSize2 = i_num_elements * sizeof(short) + dwSafeFactor * 2 * 60;
			m_pWTransf = static_cast<short*>(malloc(dwBufferSize2));
			ASSERT(m_pWTransf != NULL);
			if (m_pWTransf == nullptr)
				return FALSE;
		}
		m_iNumElements = i_num_elements;
	}
	m_waveFormat.buffersize = i_num_elements * nchannels;	
	return TRUE;
}

// -----------------------------------------------------------

void CWaveBuf::Serialize( CArchive& ar)
{
	CObject::Serialize( ar);
	m_waveFormat.Serialize(ar);		// serialize data description
	m_chanArray.Serialize(ar);		// serialize channels description

	if( ar.IsStoring() )
	{
		ar << m_wversion;
	}
	else
	{
		WORD wversion;
		ar >> wversion;
	}
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// add description of new method here

std::string CWaveBuf::m_pTransformsAllowed[] = {
	"BMAXMIN  - Raw data",								// 0
	"BDERIVAT - First derivative",						// 1
	"BLANCZO2 - Lanczos filter 2l (1/5T)  Ci=1,2",		// 2
	"BLANCZO3 - Lanczos filter 3l (1/14T) Ci=1,2,3",	// 3
	"BDERI1F3 - Derivative (1/3T)  1f3 Ci=1,1",			// 4
	"BDERI2F3 - Derivative (1/6T)  2f3 Ci=1,1,1",		// 5
	"BDERI2F5 - Derivative (1/10T) 2f5 Ci=1,1,1,1",		// 6
	"BDERI3F3 - Derivative (1/9T)  3f3 Ci=0,1,1,1",		// 7
	"BDIFFER1 - Difference X(n+1)-X(n-1)",				// 8
	"BDIFFER2 - Difference X(n+2)-X(n-2)",				// 9
	"BDIFFER3 - Difference X(n+3)-X(n-3)",				// 10
	"BDIFFER10 - Difference X(n+10)-X(n-10)",			// 11
	"MOVAVG30 - Moving average, 30 points",				// 12
	"MEDIAN30 - Moving median , 30 points",				// 13
	"MEDIAN35 - Moving median , 35 points",				// 14
	"RMS30 - Moving RMS, 30 points",					// 15
	""};

int CWaveBuf::m_maxtransform=16;

// -----------------------------------------------------------

int CWaveBuf::m_TransformBufferSpan[] = {
	0, 		// maxmin
	4,		// bderiv
	2,		// lanczos2
	3,		// lanczos3
	2,		// deriv2
	3,		// deriv3
	4,		// deriv4
	4,		// deriv4 bis
	1,		// diff n+1/n-1
	2,		// diff2 n+2/n-2
	3,		// diff3 n+3/n-3
	10,		// diff n+10/n-10	
	60,		// moving average 60*2
	30,		// median 30
	35,		// median 35
	30,		// RMS
	-1};

// -----------------------------------------------------------

int CWaveBuf::m_bvariableSpan[] = {
	0, 		// maxmin
	0,		// bderiv
	0,		// lanczos2
	0,		// lanczos3
	0,		// deriv2
	0,		// deriv3
	0,		// deriv4
	0,		// deriv4 bis
	0,		// diff n+1/n-1
	0,		// diff2 n+2/n-2
	0,		// diff3 n+3/n-3
	0,		// diff n+10/n-10	
	1,		// moving average
	1,		// median filter 30
	1,		// median filter 35
	1,		// root-mean-square
	-1};

// -----------------------------------------------------------

float CWaveBuf::m_correctionFact[] = {
	1.f,  	// maxmin
	6.f, 	// bderiv
	5.f,	// lanczos2
	14.f,	// lanczos3
	3.f,	// deriv2
	6.f,	// deriv3
	10.f,	// deriv4
	9.f,    // deriv4 bis
	1.f,	// diff1
	1.f,	// diff2 n+2/n-2
	1.f, 	// diff3 n+3/n-3
	1.f,	// diff10
	1.f,	// MOVAVG30
	1.f,	// Median30
	1.f,	// Median35
	1.f,	// Root-mean-square
	1.f};

///////////////////////////////////////////////////////////////////////////////////////

WORD CWaveBuf::GetWBNTypesofTransforms() 
{
	if (m_maxtransform <0)
	{
		m_maxtransform=0;
		while (m_TransformBufferSpan[m_maxtransform] >=0)
			m_maxtransform++;
	}
	return m_maxtransform;
}

// -----------------------------------------------------------

CString CWaveBuf::GetWBTransformsAllowed(int i)
{
	if (i < 0)					
		i = 0;
	else if (i > m_maxtransform)  
		i = m_maxtransform;
	return CString(m_pTransformsAllowed[i].c_str());
}

// -----------------------------------------------------------

BOOL CWaveBuf::IsWBTransformAllowed(int i)
{
	auto flag = TRUE;
	if (i<0 || i> m_maxtransform)  flag = FALSE;
	return flag;
}

// -----------------------------------------------------------

BOOL CWaveBuf::InitWBTransformBuffer()
{
	const size_t dw_buffer_size = m_waveFormat.sample_count*sizeof(short)+ 100;
	// allocate / reallocate transform heap
	if (m_pWTransf != nullptr) 
	{
		free (m_pWTransf);
		m_pWTransf = nullptr;
	}

	m_pWTransf = static_cast<short*>(malloc(dw_buffer_size));
	if (m_pWTransf == nullptr)
		return FALSE;
	const auto flag = (m_pWTransf != nullptr);
	m_bTransf = TRUE;	
	return flag;
}

// -----------------------------------------------------------

int CWaveBuf::GetWBTransformSpan(int i)
{
	if (i<0 || i> m_maxtransform)
		return 0;
	else						
		return m_TransformBufferSpan[i];
}

// -----------------------------------------------------------
// note: span width can be modified for several routines (like running average, running median, ...)
// those routines have a flag in bvariableSpan = 1
// the modification can be done within the dialog box "format series"

int CWaveBuf::IsWBSpanChangeAllowed(const int i)
{
	return m_bvariableSpan[i];
}

// -----------------------------------------------------------

int CWaveBuf::SetWBTransformSpan(const int i, const int span)
{
	if (m_bvariableSpan[i] > 0)
		m_TransformBufferSpan[i] = span;
	return m_TransformBufferSpan[i];
}

// -----------------------------------------------------------

int CWaveBuf::GetWBcorrectionFactor(int i, float *correct)
{
	if (i<0 || i>m_maxtransform)
		i = -1;
	else						
		*correct = m_correctionFact[i];
	return i;
}

/**************************************************************************
 function:	BDeriv
 purpose:	compute first derivative of one channel from lp_source 
			and store the result in transform buffer lp_dest
 parameters:	lp_source pointer to 1rst point to transform from RW buffer x(k=0)
				lp_dest	 pointer to destination buffer
				n items	n points to examine
 returns:	fills lp_dest buffer with first derivative of signal/lp_source
 comments:	; use a formula from 
	Usui S. and Amidror I. (1982)
	Digital low-pass differentiation for biological signal processing.
	IEEE Trans. Biomed. Eng.  20 (10) 686-693

  "New algorithms: 2f3", cf Table I, p 691
  y (k) =  (1/6T)*
				   [   (x(k+1) + x(k+2) + x(k+3) )   (a)
					 - (x(k-1) + x(k-2) + x(k-3) ) ] (b)
  nt = result channel  [y()]
  ns = source channel  [x()]
  k  = element number
  T  = sampling interval

	BDERIV computes only the expression between brackets [..]

	The expression is evaluated in two steps. It should be noted that
	expression (a) equals expression (b) * (-1) 4 steps later. The
	algorithm actually implemented use this fact to evaluate this
	expression only once, but stores it in y(k+4) and subtract it
	from y(k), using registers ax, bx, and dx to store x(k+1), x(k+2)
	and x(k+3).

  !!! this kind of filter produces an array with 6 missing points
	   (3 points before beginning and 3 points after end are
	   undoubtedly unknown). 
 **************************************************************************/
void CWaveBuf::BDeriv(const short* lp_source, short* lp_dest, int cx) const
{
	const auto ispan = 4;		// nb of dummy pts on each side of dest buffer
	auto lp_dest0 = lp_dest;	// store initial address (clear at the end)
	const int nchans = m_waveFormat.scan_count;
	lp_source -= ispan*nchans;	// init source address: k-3
	cx += ispan;				// modify nb of pts
	int bx = *lp_source; 		// xk+1 load first data pts
	lp_source +=nchans;			// update offset
	int dx = *lp_source; 		// xk+2
	lp_source +=nchans; 		// update offset

	// compute the derivative
	for (auto i=cx; i>0; i--, lp_source +=nchans, lp_dest++)
	{
		auto ax = bx;			// ax=xk+1 -- register to reg
		bx = dx;				// bx=xk+2 -- register to reg
		dx = *lp_source;  		// dx=xk+3 -- load from memory
		ax += bx + dx;			// =SUM(k+4) -- build intermediate result
		*(lp_dest+ispan)=-ax;		// y(k+4) = 2048 - SUM(k+4) -- store
		*lp_dest += ax; 		// y(k) = 2048 - SUM(k-4) + SUM(k) -- store final
	}

	// set undefined pts to zero
	for (auto icx = ispan; icx>0; icx--, lp_dest++, lp_dest0++)
	{
		*lp_dest = 0;
		*lp_dest0 = 0;
	}	
}

/**************************************************************************
 function:	BCopy
 purpose:	copy single channel from RWbuffer (n interleaved channels) 
			into transform channel (single)
 parameters:	lp_source pointer to 1rst point to transform from RW buffer x(k=0)
				lp_dest	 pointer to destination buffer
				nitems	n points to copy
 returns:	fills lp_dest buffer with copy
 comments:	none    
 **************************************************************************/

void CWaveBuf::BCopy(const short* lp_source, short* lp_dest, const int cx) const
{
	const int nchans = m_waveFormat.scan_count;
	for (auto icx=cx; icx>0; icx--, lp_source +=nchans, lp_dest++)
		*lp_dest = *lp_source;		
}           

/******************************************************************************
function:	BDiffer3
purpose:	compute the difference between a point and
			the point n-6.
parameters:	lp_source pointer to 1rst point to transform from RW buffer x(k=0)
			lp_dest	 pointer to destination buffer
			n_items	n points to examine 
return: 	fills lp_dest buffer with difference 
comment:	the difference is computed as
			 y (k) = x(k+3)-x(k-3) 
			 nt = result channel  [y()]
			 ns = source channel  [x()]
			 k  = element number
*******************************************************************************/
					
void CWaveBuf::BDiffer3(const short* lp_source, short* lp_dest, const int cx) const
{
	const int nchans = m_waveFormat.scan_count;
	// compute the difference
	const auto offset3 = 3*nchans;
	for (auto icx=cx; icx>0; icx--, lp_source +=nchans, lp_dest++)
		*lp_dest = short(*(lp_source + offset3) - *(lp_source - offset3));
}

//**************************************************************************
// 	"BDIFFER2 - Difference X(n+2)-X(n-2)",
//**************************************************************************
void CWaveBuf::BDiffer2(const short* lp_source, short* lp_dest, const int cx) const
{
	const int nchans = m_waveFormat.scan_count;
	const auto offset2 = 2*nchans;
	for (auto icx = cx; icx > 0; icx--, lp_source +=nchans, lp_dest++)
		*lp_dest = short(*(lp_source + offset2) - *(lp_source - offset2));
}

//**************************************************************************
// 	"BDIFFER1 - Difference X(n+1)-X(n-1)",
//**************************************************************************
void CWaveBuf::BDiffer1(const short* lp_source, short* lp_dest, const int cx) const
{
	const int nchans = m_waveFormat.scan_count;
	const auto offset = nchans;
	for (auto icx = cx; icx > 0; icx--, lp_source +=nchans, lp_dest++)
		*lp_dest = short(*(lp_source + offset) - *(lp_source - offset));
}

//**************************************************************************
// 	"BDIFFER10 - Difference X(n+10)-X(n-10)",
//**************************************************************************
void CWaveBuf::BDiffer10(const short* lp_source, short* lp_dest, const int cx) const
{
	const int nchans = m_waveFormat.scan_count;
	const auto offset2 = 10*nchans;
	for (auto icx = cx; icx > 0; icx--, lp_source +=nchans, lp_dest++)
		*lp_dest = short(*(lp_source + offset2) - *(lp_source - offset2));
}


//**************************************************************************
//	"BLANCZO2 - Lanczos filter 2l (1/5T)  Ci=1,2",
//**************************************************************************
void CWaveBuf::BLanczo2(const short* lp_source, short* lp_dest, const int cx) const
{
	const int nchans = m_waveFormat.scan_count;
	const auto offset1 = nchans;
	const auto offset2 = 2*offset1;
	for (auto icx = cx; icx > 0; icx--, lp_source +=nchans, lp_dest++)
	{
		*lp_dest = short((*(lp_source + offset1) - *(lp_source - offset1))
			+ 2 * (*(lp_source + offset2) - *(lp_source - offset2)));
	}	
}
//**************************************************************************
//	"BLANCZO3 - Lanczos filter 3l (1/14T) Ci=1,2,3",
//**************************************************************************
void CWaveBuf::BLanczo3(const short* lp_source, short* lp_dest, const int cx) const
{
	const int nchans = m_waveFormat.scan_count;
	const auto offset1 = nchans;
	const auto offset2 = offset1*2;
	const auto offset3 = offset2 + offset1;
	for (auto icx = cx; icx > 0; icx--, lp_source +=nchans, lp_dest++)
	{
		*lp_dest = short ((*(lp_source+offset1)- *(lp_source-offset1))
				+ (2* (*(lp_source+offset2) - *(lp_source-offset2)))
				+ (3* (*(lp_source+offset3) - *(lp_source-offset3))));
	}
}

//**************************************************************************
//	"BDERI1F3 - Derivative (1/3T)  1f3 Ci=1,1",
//**************************************************************************
void CWaveBuf::BDeri1f3(const short* lp_source, short* lp_dest, const int cx) const
{
	const int nchans = m_waveFormat.scan_count;
	const auto offset1 = nchans;
	const auto offset2 = 2*nchans;
	for (auto icx = cx; icx > 0; icx--, lp_source +=nchans, lp_dest++)
	{
		*lp_dest = short ((*(lp_source+offset1) - *(lp_source-offset1))
				+ (*(lp_source+offset2) - *(lp_source-offset2)));
	}
}

//**************************************************************************
//	"BDERI2F3 - Derivative (1/6T)  2f3 Ci=1,1,1",
//**************************************************************************
void CWaveBuf::BDeri2f3(const short* lp_source, short* lp_dest, const int cx) const
{
	const int nchans = m_waveFormat.scan_count;
	const auto offset1 = nchans;
	const auto offset2 = nchans*2;
	const auto offset3 = nchans*3;
	for (auto icx = cx; icx > 0; icx--, lp_source +=nchans, lp_dest++)
	{
		*lp_dest = short((*(lp_source+offset1) - *(lp_source-offset1))
				+ (*(lp_source+offset2) - *(lp_source-offset2))
				+ (*(lp_source+offset3) - *(lp_source-offset3)));
	}
}

//**************************************************************************
//	"BDERI2F5 - Derivative (1/10T) 2f5 Ci=1,1,1,1",
//**************************************************************************
void CWaveBuf::BDeri2f5(const short* lp_source, short* lp_dest, const int cx) const
{
	const int nchans = m_waveFormat.scan_count;
	const auto offset1 = nchans;
	const auto offset2 = nchans*2;
	const auto offset3 = nchans*3;
	const auto offset4 = nchans*4;
	for (auto icx = cx; icx > 0; icx--, lp_source +=nchans, lp_dest++)
	{
		*lp_dest = short((*(lp_source+offset1) - *(lp_source-offset1))
				+ (*(lp_source+offset2) - *(lp_source-offset2))
				+ (*(lp_source+offset3) - *(lp_source-offset3))
				+ (*(lp_source+offset4) - *(lp_source-offset4)));
	}
}
//**************************************************************************
//	"BDERI3F3 - Derivative (1/9T)  3f3 Ci=0,1,1,1",
//**************************************************************************
void CWaveBuf::BDeri3f3(const short* lp_source, short* lp_dest, const int cx) const
{
	const int nchans = m_waveFormat.scan_count;
	const auto offset2 = nchans*2;
	const auto offset3 = nchans*3;
	const auto offset4 = 2* offset2;
	for (auto icx = cx; icx > 0; icx--, lp_source +=nchans, lp_dest++)
	{
		*lp_dest = (*(lp_source+offset2) - *(lp_source-offset2))
				+ (*(lp_source+offset3) - *(lp_source-offset3))
				+ (*(lp_source+offset4) - *(lp_source-offset4));
	}
}

/**************************************************************************
 function:  GetVoltsperDataBin(short chIndex, float* VoltsperBin)
 purpose:		get nb volts corresp to one bin /data channel chIndex
				volt_per_Bin= 10 (Volts) / gain / 4096 (12 bits)
 parameters:	chIndex = channel index (0->n)
 returns:		BOOL: channel OK/FALSE, float value: n V/data bin (12 bits)
 comments: 
 **************************************************************************/

BOOL CWaveBuf::GetWBVoltsperBin(const int ch_index, float* volts_per_bin, const int mode) const
{
	if ((ch_index >= m_waveFormat.scan_count)|| (ch_index <0))
		return FALSE;
	auto correction = 1.0f;
	GetWBcorrectionFactor(mode, &correction);

	if (m_chanArray.get_p_channel(ch_index)->am_resolutionV == 0)
	{
		if (m_chanArray.get_p_channel(ch_index)->am_gaintotal == 0.)
		{
			m_chanArray.get_p_channel(ch_index)->am_gaintotal = m_chanArray.get_p_channel(ch_index)->am_gainamplifier 
				* float(m_chanArray.get_p_channel(ch_index)->am_gainpre) 
				* float(m_chanArray.get_p_channel(ch_index)->am_gainpost) 
				* float(m_chanArray.get_p_channel(ch_index)->am_gainheadstage) 
				* float(m_chanArray.get_p_channel(ch_index)->am_gainAD);
		}

		m_chanArray.get_p_channel(ch_index)->am_resolutionV = m_waveFormat.fullscale_Volts 
					/float(m_chanArray.get_p_channel(ch_index)->am_gaintotal)
					/float(m_waveFormat.binspan);
		//auto binspan = m_waveFormat.binspan;
		//auto fullscale_volts = m_waveFormat.fullscale_Volts;
		//auto am_resolution_v = m_chanArray.get_p_channel(ch_index)->am_resolutionV;
	}
	*volts_per_bin = static_cast<float>(m_chanArray.get_p_channel(ch_index)->am_resolutionV / correction);
	return TRUE;
}

/**************************************************************************
 function:  BMovAvg30(short* lp_source, short* lp_dest, int cx)
 purpose:		compute moving average
 parameters:	input array + nb of points to compute
 returns:		output array with average
 comments: 
 **************************************************************************/

void CWaveBuf::BMovAvg30(short* lp_source, short* lp_dest, const int cx) const
{
	const int nchans = m_waveFormat.scan_count;
	const auto offsetnextpoint = nchans;
	auto nbspan = m_TransformBufferSpan[MOVAVG30];
	const auto offsetspan = offsetnextpoint * nbspan;

	// init sum
	auto sum = 0;
	auto n=0;
	auto lp = lp_source;
	lp -= offsetspan;
	nbspan += nbspan;
	for (auto i=0; i<nbspan; i++)
	{
		sum += *lp;
		lp += offsetnextpoint;
		n++;
	}
	
	// moving average
	for (auto icx = cx; icx > 0; icx--, lp_source += offsetnextpoint, lp_dest++)
	{			
		*lp_dest = static_cast<short>(sum / n);
		sum += (-*(lp_source-offsetspan) + *(lp_source+offsetspan));
	}
}

/**************************************************************************
 function:  BMedian30(short* lp_source, short* lp_dest, int cx)
 purpose:		compute moving median
 parameters:	input array + nb of points to compute, span=30 points
 returns:		output array with median value
 comments:
 source:	L. Fiore, G. Corsini, L. Geppetti (1997) Application of non-linear
			filters based on the median filter to experimental and simulated
			multiunit neural recordings. J Neurosci Meth 70: 177-184.
 **************************************************************************/
void CWaveBuf::BMedian35(short* lp_source, short* lp_dest, const int cx)
{
	BMedian(lp_source, lp_dest, cx, 35);
}

void CWaveBuf::BMedian30(short* lp_source, short* lp_dest, const int cx)
{
	BMedian(lp_source, lp_dest, cx, 30);
}

// Compute median of a curve
// lp_source = pointer to source data buffer (interleaved channels) [iitime = lFirst]
// lp_dest = pointer to destination data buffer (only one channel)
// nbspan = number of points to take into account on each side of each data point
// assume: m_waveFormat set
// use a temporary array to store data (dimension = nbspan *2 +1)
// assume: temporary array in common of wavebuf (to avoid creation each time this routine is called)

void CWaveBuf::BMedian(short* lp_source, short* lp_dest, int cx, const int nbspan)
{	
	// assume source buffer is with interleaved channels; if not: scan_count=1
	const int nchans = m_waveFormat.scan_count;
	const auto lp_source_offset_nextpoint = nchans;							// number of chans within source buffer
	const auto lp_source_offset_span      = lp_source_offset_nextpoint * nbspan;	// offset between center of window & end

	if (m_parray_size != nbspan*2 +1)						// adjust size of temporary array or create if not previously set
	{
		SAFE_DELETE_ARRAY(m_parraySorted);
		SAFE_DELETE_ARRAY(m_parrayCircular);
		m_parray_size = nbspan*2 +1;
		m_parraySorted   = new short [m_parray_size];
		m_parrayCircular = new short [m_parray_size];
	}
	
	// DEBUG: bounds check
	ASSERT(cx <= GetWBNumElements());
	const auto min_lp_source = m_pWData;
	const auto max_lp_source = m_pWData + GetWBNumChannels() * GetWBNumElements();
	//auto min_lp_dest = m_pWTransf;
	//auto max_lp_dest = m_pWTransf + GetWBNumElements();
	//auto max_parray = &m_parraySorted[m_parray_size-1];
	//auto min_parray = m_parraySorted;

	// load parray = consecutive points (image of the data points)	
	auto lp = lp_source;							// pointer to origin of source buffer
	lp -= lp_source_offset_span;					// first point of the moving window
	int i;
	for (i = 0; i < m_parray_size; lp += lp_source_offset_nextpoint, i++)
	{
		*(m_parraySorted+i) = *lp;					// load m_parraySorted with source data values
		*(m_parrayCircular+i)=*lp;
	}

	// sort m_parraySorted into ascending order using heapsort algorithm
	// cf Numerical recipes Press et al.1986, pp 231
	// "l"= index which will be decremented from initial value down to 0 during
	// 'hiring' (heap creation) phase. Once it reaches 0, the index "ir" will be 
	// decremented from its initial value down to 0 during the 'retirement-and-
	// promotion' (heap selection) phase.

	auto l = nbspan+ 1;				// temp index
	auto ir = m_parray_size - 1;	// temp index
	short val;						// temp storage	

	for (;;)						// pseudo-loop over m_parraySorted
	{
		// -------------------------
		if (l > 0)					// still in hiring phase
		{
			l--;
			val = *(m_parraySorted + l);
		}
		else						// in retirement-and-promotion phase
		{
			val = *(m_parraySorted+ir);		// clear a space at the end of the array
			*(m_parraySorted+ir) = *(m_parraySorted);	// retire the top of the heap into it
			ir--;					// decrease the size of the corporation
			if (ir == 0)			// done with the last promotion
			{
				*(m_parraySorted) = val;	// the least competent worker of all
				break;				// exit the sorting algorithm
			}
		}
		// -------------------------
		i = l + 1;					// wether we are in the hiring or promotion, we
		auto jj1 = i + i;					// here set up to sift down element to its 
									// proper level
		while (jj1-1 <= ir)
		{			
			if (jj1-1 < ir)				
			{
				if (*(m_parraySorted+jj1-1) < *(m_parraySorted+jj1))
					jj1++;			// compare to the better underlining
			}
			if (val < *(m_parraySorted+jj1-1))// demote val
			{
				*(m_parraySorted+i-1) = *(m_parraySorted+jj1-1);
				i = jj1;
				jj1 = jj1 + jj1;
			}						// this is val's level. Set j to terminate the
			else					// sift-down
				jj1 = ir + 2;
		}
		*(m_parraySorted+i-1) = val;		// put val into its slot
	}
	// end of initial sort

	lp= lp_source;					// first data point
	auto lp_next = lp_source + lp_source_offset_span; // last point
	auto i_parray_circular = m_parray_size - 1; // point on the last item so that first operation is blank

	for (auto icx = cx; icx>0; icx--, lp += lp_source_offset_nextpoint, lp_next += lp_source_offset_nextpoint, lp_dest++)
	{
		const auto oldvalue = *(m_parrayCircular + i_parray_circular);	// old value
		const auto newvalue = *lp_next;									// new value to insert into array
		*(m_parrayCircular + i_parray_circular)= newvalue;	// save new value into circular array
		
		// update circular array pointer
		i_parray_circular++;
		if (i_parray_circular >= m_parray_size)
			i_parray_circular = 0;

		// locate position of old value to discard
		// use bisection - cf Numerical Recipes pp 90
		// on exit, j= index of oldvalue

		// binary search
		// Herbert Schildt: C the complete reference McGraw Hill, 1987, pp 488
		auto jhigh = m_parray_size-1;	// upper index
		auto jlow = 0;					// mid point index
		auto jj2 = (jlow + jhigh) / 2;
		while (jlow <= jhigh)
		{
			jj2 = (jlow+jhigh)/2;			
			if (oldvalue > *(m_parraySorted+jj2))
				jlow = jj2 +1;
			else if (oldvalue < *(m_parraySorted+jj2))
				jhigh = jj2-1;
			else
				break;
		}

		// insert new value in the correct position

		// case 1: search (and replace) towards higher values
		if (newvalue > *(m_parraySorted + jj2))
		{
			auto j=jj2;
			for (j; newvalue > *(m_parraySorted + j); j++)
			{								
				if (j == m_parray_size)
					break;
				*(m_parraySorted+ j) = *(m_parraySorted + j + 1);
			} 
			*(m_parraySorted + j-1) = newvalue;
		}

		// case 2: search (and replace) towards lower values
		else if (newvalue < *(m_parraySorted + jj2))
		{
			auto j=jj2;
			for (j; newvalue < *(m_parraySorted + j); j--)
			{
				if (j == 0)
				{
					if (newvalue < *m_parraySorted)
						j--;
					break;
				}
				*(m_parraySorted + j)  = *(m_parraySorted + j -1);
			}
			*(m_parraySorted + j+1) = newvalue;
		}

		// case 3: already found!
		else
			*(m_parraySorted + jj2) = newvalue;

		// save median value in the output array
		*lp_dest = *lp - *(m_parraySorted+nbspan);
		ASSERT(lp >= min_lp_source);
		ASSERT(lp <= max_lp_source);
	}
}

/**************************************************************************

*/

void CWaveBuf::BRMS(short* lp_source, short* lp_dest, const int cx) const
{
	const int nchans = m_waveFormat.scan_count;
	const auto offsetnextpoint = nchans;
	auto nbspan = m_TransformBufferSpan[14] / 2;
	const auto offsetspan = offsetnextpoint * nbspan;

	// init sum
	double sum2 = 0;
	auto n=0;
	auto lp = lp_source;
	lp -= offsetspan;
	nbspan += nbspan;
	for (auto i=0; i < nbspan; i++)
	{
		const long x = *lp;
		sum2 += double(x)*x;
		lp += offsetnextpoint;
		n++;
	}

	// moving average
	for (auto icx = cx; icx > 0; icx--, lp_source += offsetnextpoint, lp_dest++)
	{			
		*lp_dest = *lp_source - static_cast<short>(sqrt(sum2 / n));
		long x = *(lp_source-offsetspan);
		sum2 -= double(x)*x;
		x = *(lp_source+offsetspan);
		sum2 += double(x)*x;
	}
}
