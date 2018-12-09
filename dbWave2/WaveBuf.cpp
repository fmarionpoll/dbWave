// WaveBuf.cpp : implementation file
//

#include "StdAfx.h"
#include <math.h> 

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
	//m_hWTransf = NULL;
	//m_hWData = NULL;
	m_wversion=0;
	m_bTransf = FALSE;
	m_iNumElements = 0;
	m_parraySorted= nullptr;
	m_parrayCircular= nullptr;
	m_parray_size=0;
	m_dwBufferSize=0;
}


int	CWaveBuf::WBDatachanSetnum(int i)
{
	// no action if the number of data channels is right
	if (m_waveFormat.scan_count == i 
		&& m_chanArray.channel_get_number() == i)
		return i;
	int iNumElements = m_waveFormat.buffersize / m_waveFormat.scan_count;	
	m_waveFormat.scan_count = i;
	m_chanArray.channel_set_number(i);
	ASSERT(m_chanArray.channel_get_number() == m_waveFormat.scan_count);
	CreateWBuffer(iNumElements, i);
	return m_waveFormat.scan_count;
}

// -----------------------------------------------------------
short* CWaveBuf::GetWBAdrRawDataBuf() 
{
	ASSERT(m_pWData != NULL);
	return m_pWData;
}
// -----------------------------------------------------------
short* CWaveBuf::GetWBAdrRawDataElmt(int chan, int index) 
{
	ASSERT(chan < m_waveFormat.scan_count);
	ASSERT(m_pWData != NULL);
	return m_pWData + index*m_waveFormat.scan_count + chan;
}
// -----------------------------------------------------------
short* CWaveBuf::GetWBAdrTransfData() 
	{return m_pWTransf;}
// -----------------------------------------------------------
short* CWaveBuf::GetWBAdrTransfDataElmt(int index) 
	{return m_pWTransf + index ;}
// -----------------------------------------------------------
int	CWaveBuf::GetWBNumElements() 
	{return m_iNumElements; } //m_waveFormat.sample_count;}
// -----------------------------------------------------------
int	CWaveBuf::GetWBNumChannels() 
	{return m_waveFormat.scan_count;}
// -----------------------------------------------------------
void CWaveBuf::SetWBSampleRate(float fSampRate)
	{m_waveFormat.chrate = fSampRate;}
// -----------------------------------------------------------
float CWaveBuf::GetWBSampleRate() 
	{return m_waveFormat.chrate;}
// -----------------------------------------------------------

CWaveBuf::~CWaveBuf()
{
	DeleteBuffers();
	SAFE_DELETE_ARRAY(m_parraySorted);
	SAFE_DELETE_ARRAY(m_parrayCircular);
}

// -----------------------------------------------------------
void CWaveBuf::DeleteBuffers()
{
	if (m_pWData != nullptr)
	{
		//HeapFree(m_hWData, 0, m_pWData);
		//m_hWData = NULL;
		free (m_pWData);
		m_pWData = nullptr;
	}
	if (m_pWTransf != nullptr) 
	{
		//HeapFree(m_hWTransf, 0, m_pWTransf);
		//m_hWTransf = NULL;
		free (m_pWTransf);
		m_pWTransf = nullptr;
	}
	m_iNumElements=0;
}

// -----------------------------------------------------------
BOOL CWaveBuf::CreateWBuffer(int iNumElements, int nchannels)
{
	ASSERT(this);
	ASSERT(iNumElements >= 1); // must have at least one
	m_waveFormat.scan_count = nchannels;

	DWORD dwSafeFactor = 256;		// safety factor to fit data in buffer
	size_t dwBufferSize = iNumElements * nchannels*sizeof(short) + dwSafeFactor;
	if (m_dwBufferSize != dwBufferSize)
	{
		// free memory
		DeleteBuffers();
		// Allocate data on data heap
		//DWORD flOptions = HEAP_GENERATE_EXCEPTIONS;
		//SIZE_T dwInitialize = dwBufferSize;
		//m_hWData = HeapCreate(flOptions, dwInitialize, 0);
		//if (m_hWData  == NULL)
		//{
		//	_tprintf(TEXT("Failed to create a new heap with LastError %d.\n"),
		//		 GetLastError());
		//	ASSERT(m_hWData != NULL);
		//	return FALSE;
		//}
		//m_pWData = (short*) HeapAlloc(m_hWData, flOptions, dwBufferSize);
		
		m_pWData = (short*) malloc( dwBufferSize);
		ASSERT(m_pWData != NULL);
		if (m_pWData == nullptr)
			return FALSE;

		// allocate transform heap if selected
		if (m_bTransf)
		{
			size_t dwBufferSize2 = iNumElements * sizeof(short) + dwSafeFactor * 2 * 60;
			//SIZE_T dwInitialize = dwBufferSize;
			//DWORD flOptions = HEAP_GENERATE_EXCEPTIONS;
			//m_hWTransf = HeapCreate(flOptions, dwInitialize, 0);
			//ASSERT(m_hWTransf != NULL);
			//if (m_hWTransf  == NULL)
			//	return FALSE;
			//m_pWTransf = (short*) HeapAlloc(m_hWTransf, flOptions, dwBufferSize);
			m_pWTransf = (short*) malloc( dwBufferSize2);
			ASSERT(m_pWTransf != NULL);
			if (m_pWTransf == nullptr)
				return FALSE;
		}
		m_iNumElements = iNumElements;
	}
	m_waveFormat.buffersize = iNumElements * nchannels;	
	return TRUE;
}

// -----------------------------------------------------------

void CWaveBuf::Serialize( CArchive& ar)
{
	// call base class function first
	// base class is CObject in this case
	CObject::Serialize( ar);
	m_waveFormat.Serialize(ar);		// serialize data description
	m_chanArray.Serialize(ar);		// serialize channels description

	// now do the stuff for our specific class
	if( ar.IsStoring() )
	{
		ar << m_wversion;
		// save data buffer here?
		// according to flag?
		// and / or save transform data; interleaved?
	}
	else
	{
		WORD wversion;
		ar >> wversion;
		// load data buffer here?
		// according to flag?
		// and / or load transform data; interleaved?
	}
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// add description of new method here

char* CWaveBuf::m_pTransformsAllowed[] = {
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
	if (i < 0)					i = 0;
	else if (i > m_maxtransform)  i = m_maxtransform;
	return CString(m_pTransformsAllowed[i]);
}

// -----------------------------------------------------------

BOOL CWaveBuf::IsWBTransformAllowed(int i)
{
	BOOL flag = TRUE;
	if (i<0 || i> m_maxtransform)  flag = FALSE;
	return flag;
}

// -----------------------------------------------------------

BOOL CWaveBuf::InitWBTransformMode(int itransf)
{
	size_t dwBufferSize = m_waveFormat.sample_count*sizeof(short)+ 100;
	BOOL flag = TRUE;

	// allocate / reallocate transform heap
	if (m_pWTransf != nullptr) 
	{
		free (m_pWTransf);
		m_pWTransf = nullptr;
	}

	m_pWTransf = (short*) malloc (dwBufferSize);
	if (m_pWTransf == nullptr)
		return FALSE;
	flag = (m_pWTransf != nullptr);
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

int CWaveBuf::IsWBSpanChangeAllowed(int i)
{
	return m_bvariableSpan[i];
}

// -----------------------------------------------------------

int CWaveBuf::SetWBTransformSpan(int i, int span)
{
	if (m_bvariableSpan[i] > 0)
		m_TransformBufferSpan[i] = span;
	return m_TransformBufferSpan[i];
}

// -----------------------------------------------------------

int CWaveBuf::GetWBcorrectionFact(int i, float *correct)
{
	if (i<0 || i>m_maxtransform)
		i = -1;
	else						
		*correct = m_correctionFact[i];
	return i;
}

/**************************************************************************
 function:	BDeriv
 purpose:	compute first derivative of one channel from lpSource 
			and store the result in transform buffer lpDest
 parameters:	lpSource pointer to 1rst point to transform from RW buffer x(k=0)
				lpDest	 pointer to destination buffer
				nitems	n points to examine
 returns:	fills lpDest buffer with first derivative of signal/lpSource
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
	expression only once, but stores it in y(k+4) and substract it
	from y(k), using registers ax, bx, and dx to store x(k+1), x(k+2)
	and x(k+3).

  !!! this kind of filter produces an array with 6 missing points
	   (3 points before beginning and 3 points after end are
	   undoubtly unknown). 
 **************************************************************************/
void CWaveBuf::BDeriv(short* lpSource, short* lpDest, int cx)
{
	int ax, bx, dx;			// registers
	const int ispan = 4;		// nb of dummy pts on each side of dest buffer
	short* lpDest0 = lpDest;	// store initial address (clear at the end)
	int nchans = m_waveFormat.scan_count;
	lpSource -= ispan*nchans;	// init source address: k-3
	cx += ispan;				// modify nb of pts
	bx = *lpSource; 			// xk+1 load first data pts
	lpSource +=nchans;			// update offset
	dx = *lpSource; 			// xk+2
	lpSource +=nchans; 			// update offset

	// compute the derivative
	for (cx; cx>0; cx--, lpSource +=nchans, lpDest++)
	{
		ax = bx;				// ax=xk+1 -- register to reg
		bx = dx;				// bx=xk+2 -- register to reg
		dx = *lpSource;  		// dx=xk+3 -- load from memory
		ax += bx + dx;			// =SUM(k+4) -- build intermediate result
		*(lpDest+4)=-ax;		// y(k+4) = 2048 - SUM(k+4) -- store
		*lpDest += ax; 			// y(k) = 2048 - SUM(k-4) + SUM(k) -- store final
	}

	// set undefined pts to zero
	for (cx = ispan; cx>0; cx--, lpDest++, lpDest0++)
	{
		*lpDest = 0;
		*lpDest0 = 0;
	}	
}

/**************************************************************************
 function:	BCopy
 purpose:	copy single channel from RWbuffer (n interleaved channels) 
			into transform channel (single)
 parameters:	lpSource pointer to 1rst point to transform from RW buffer x(k=0)
				lpDest	 pointer to destination buffer
				nitems	n points to copy
 returns:	fills lpDest buffer with copy
 comments:	none    
 **************************************************************************/

void CWaveBuf::BCopy(short* lpSource, short* lpDest, int cx)
{	
	int nchans = m_waveFormat.scan_count;
	for (cx; cx>0; cx--, lpSource +=nchans, lpDest++)
		*lpDest = *lpSource;		
}           

/******************************************************************************
function:	BDiffer3
purpose:	compute the difference between a point and
			the point n-6.
parameters:	lpSource pointer to 1rst point to transform from RW buffer x(k=0)
			lpDest	 pointer to destination buffer
			nitems	n points to examine 
return: 	fills lpDest buffer with difference 
comment:	the difference is computed as
			 y (k) = x(k+3)-x(k-3) 
			 nt = result channel  [y()]
			 ns = source channel  [x()]
			 k  = element number
*******************************************************************************/
					
void CWaveBuf::BDiffer3(short* lpSource, short* lpDest, int cx)
{
	int nchans = m_waveFormat.scan_count;
	// compute the difference
	int offset3 = 3*nchans;
	for (cx; cx>0; cx--, lpSource +=nchans, lpDest++)
		*lpDest = (short) (*(lpSource+offset3) - *(lpSource-offset3));
}

//**************************************************************************
// 	"BDIFFER2 - Difference X(n+2)-X(n-2)",
//**************************************************************************
void CWaveBuf::BDiffer2(short* lpSource, short* lpDest, int cx)
{
	int nchans = m_waveFormat.scan_count;
	int offset2 = 2*nchans;
	for (cx; cx>0; cx--, lpSource +=nchans, lpDest++)
		*lpDest = (short) (*(lpSource+offset2) - *(lpSource-offset2));
}

//**************************************************************************
// 	"BDIFFER1 - Difference X(n+1)-X(n-1)",
//**************************************************************************
void CWaveBuf::BDiffer1(short* lpSource, short* lpDest, int cx)
{
	int nchans = m_waveFormat.scan_count;
	int offset = nchans;
	for (cx; cx>0; cx--, lpSource +=nchans, lpDest++)
		*lpDest = (short) (*(lpSource+offset) - *(lpSource-offset));
}

//**************************************************************************
// 	"BDIFFER10 - Difference X(n+10)-X(n-10)",
//**************************************************************************
void CWaveBuf::BDiffer10(short* lpSource, short* lpDest, int cx)
{
	int nchans = m_waveFormat.scan_count;
	int offset2 = 10*nchans;
	for (cx; cx>0; cx--, lpSource +=nchans, lpDest++)
		*lpDest = (short) (*(lpSource+offset2) - *(lpSource-offset2));
}


//**************************************************************************
//	"BLANCZO2 - Lanczos filter 2l (1/5T)  Ci=1,2",
//**************************************************************************
void CWaveBuf::BLanczo2(short* lpSource, short* lpDest, int cx)
{
	int nchans = m_waveFormat.scan_count;
	int offset1 = nchans;
	int offset2 = 2*offset1;
	for (cx; cx>0; cx--, lpSource +=nchans, lpDest++)
	{
		*lpDest = (short) ((*(lpSource+offset1)- *(lpSource-offset1))
				+ 2*(*(lpSource+offset2)- *(lpSource-offset2)));
	}	
}
//**************************************************************************
//	"BLANCZO3 - Lanczos filter 3l (1/14T) Ci=1,2,3",
//**************************************************************************
void CWaveBuf::BLanczo3(short* lpSource, short* lpDest, int cx)
{
	int nchans = m_waveFormat.scan_count;
	int offset1 = nchans;
	int offset2 = offset1*2;
	int offset3 = offset2 + offset1;
	for (cx; cx>0; cx--, lpSource +=nchans, lpDest++)
	{
		*lpDest = (*(lpSource+offset1)- *(lpSource-offset1))
				+ (2* (*(lpSource+offset2) - *(lpSource-offset2)))
				+ (3* (*(lpSource+offset3) - *(lpSource-offset3)));
	}
}

//**************************************************************************
//	"BDERI1F3 - Derivative (1/3T)  1f3 Ci=1,1",
//**************************************************************************
void CWaveBuf::BDeri1f3(short* lpSource, short* lpDest, int cx)
{
	int nchans = m_waveFormat.scan_count;
	int offset1 = nchans;
	int offset2 = 2*nchans;
	for (cx; cx>0; cx--, lpSource +=nchans, lpDest++)
	{
		*lpDest = (*(lpSource+offset1) - *(lpSource-offset1))
				+ (*(lpSource+offset2) - *(lpSource-offset2));
	}
}

//**************************************************************************
//	"BDERI2F3 - Derivative (1/6T)  2f3 Ci=1,1,1",
//**************************************************************************
void CWaveBuf::BDeri2f3(short* lpSource, short* lpDest, int cx)
{
	int nchans = m_waveFormat.scan_count;
	int offset1 = nchans;
	int offset2 = nchans*2;
	int offset3 = nchans*3;
	for (cx; cx>0; cx--, lpSource +=nchans, lpDest++)
	{
		*lpDest = (*(lpSource+offset1) - *(lpSource-offset1))
				+ (*(lpSource+offset2) - *(lpSource-offset2))
				+ (*(lpSource+offset3) - *(lpSource-offset3));
	}
}

//**************************************************************************
//	"BDERI2F5 - Derivative (1/10T) 2f5 Ci=1,1,1,1",
//**************************************************************************
void CWaveBuf::BDeri2f5(short* lpSource, short* lpDest, int cx)
{
	int nchans = m_waveFormat.scan_count;
	int offset1 = nchans;
	int offset2 = nchans*2;
	int offset3 = nchans*3;
	int offset4 = nchans*4;
	for (cx; cx>0; cx--, lpSource +=nchans, lpDest++)
	{
		*lpDest = (*(lpSource+offset1) - *(lpSource-offset1))
				+ (*(lpSource+offset2) - *(lpSource-offset2))
				+ (*(lpSource+offset3) - *(lpSource-offset3))
				+ (*(lpSource+offset4) - *(lpSource-offset4));
	}
}
//**************************************************************************
//	"BDERI3F3 - Derivative (1/9T)  3f3 Ci=0,1,1,1",
//**************************************************************************
void CWaveBuf::BDeri3f3(short* lpSource, short* lpDest, int cx)
{
	int nchans = m_waveFormat.scan_count;
	int offset2 = nchans*2;
	int offset3 = nchans*3;
	int offset4 = 2* offset2;
	for (cx; cx>0; cx--, lpSource +=nchans, lpDest++)
	{
		*lpDest = (*(lpSource+offset2) - *(lpSource-offset2))
				+ (*(lpSource+offset3) - *(lpSource-offset3))
				+ (*(lpSource+offset4) - *(lpSource-offset4));
	}
}

/**************************************************************************
 function:  GetVoltsperDataBin(short chIndex, float* VoltsperBin)
 purpose:		get nb volts corresp to one bin /data channel chIndex
				voltperBin= 10 (Volts) / gain / 4096 (12 bits)
 parameters:	chIndex = channel index (0->n)
 returns:		BOOL: channel OK/FALSE, float value: n V/data bin (12 bits)
 comments: 
 **************************************************************************/

BOOL CWaveBuf::GetWBVoltsperBin(int chIndex, float* VoltsperBin, int mode)
{
	if ((chIndex >= m_waveFormat.scan_count)|| (chIndex <0))
		return FALSE;
	float correction = 1.0f;
	GetWBcorrectionFact(mode, &correction);

	if (m_chanArray.get_p_channel(chIndex)->am_resolutionV == 0)
	{
		if (m_chanArray.get_p_channel(chIndex)->am_gaintotal == 0.)
		{
			m_chanArray.get_p_channel(chIndex)->am_gaintotal = 
				m_chanArray.get_p_channel(chIndex)->am_gainamplifier 
				* (float) m_chanArray.get_p_channel(chIndex)->am_gainpre 
				* (float) m_chanArray.get_p_channel(chIndex)->am_gainpost 
				* (float)m_chanArray.get_p_channel(chIndex)->am_gainheadstage 
				* (float) m_chanArray.get_p_channel(chIndex)->am_gainAD;
		}
		
		long binspan = m_waveFormat.binspan;
		float fullscale_Volts = m_waveFormat.fullscale_Volts;
		m_chanArray.get_p_channel(chIndex)->am_resolutionV = 
					m_waveFormat.fullscale_Volts 
					/(float)m_chanArray.get_p_channel(chIndex)->am_gaintotal
					/(float)m_waveFormat.binspan;
		double am_resolutionV = m_chanArray.get_p_channel(chIndex)->am_resolutionV;
	}
	*VoltsperBin = (float) (m_chanArray.get_p_channel(chIndex)->am_resolutionV /correction);
	return TRUE;
}

/**************************************************************************
 function:  BMovAvg30(short* lpSource, short* lpDest, int cx)
 purpose:		compute moving average
 parameters:	input array + nb of points to compute
 returns:		output array with average
 comments: 
 **************************************************************************/

void CWaveBuf::BMovAvg30(short* lpSource, short* lpDest, int cx)
{	
	int nchans = m_waveFormat.scan_count;
	int offsetnextpoint = nchans;
	int nbspan = m_TransformBufferSpan[MOVAVG30];
	int offsetspan = offsetnextpoint * nbspan;

	// init sum
	int sum = 0;
	int N=0;
	short* lp = lpSource;
	lp -= offsetspan;
	nbspan += nbspan;
	for (int i=0; i<nbspan; i++)
	{
		sum += *lp;
		lp += offsetnextpoint;
		N++;
	}
	
	// moving average
	for (cx; cx>0; cx--, lpSource += offsetnextpoint, lpDest++)
	{			
		*lpDest = (short) (sum/N);
		sum += (-*(lpSource-offsetspan) + *(lpSource+offsetspan));
	}
}

/**************************************************************************
 function:  BMedian30(short* lpSource, short* lpDest, int cx)
 purpose:		compute moving median
 parameters:	input array + nb of points to compute, span=30 points
 returns:		output array with median value
 comments:
 source:	L. Fiore, G. Corsini, L. Geppetti (1997) Application of non-linear
			filters based on the median filter to experimental and simulated
			multiunit neural recordings. J Neurosci Meth 70: 177-184.
 **************************************************************************/
void CWaveBuf::BMedian35(short* lpSource, short* lpDest, int cx)
{
	BMedian(lpSource, lpDest, cx, 35);
}

void CWaveBuf::BMedian30(short* lpSource, short* lpDest, int cx)
{
	BMedian(lpSource, lpDest, cx, 30);
}

// Compute median of a curve
// lpSource = pointer to source data buffer (interleaved channels) [iitime = lFirst]
// lpDest = pointer to destination data buffer (only one channel)
// nbspan = number of points to take into account on each side of each data point
// assume: m_waveFormat set
// use a temporary array to store data (dimension = nbspan *2 +1)
// assume: temporary array in common of wavebuf (to avoid creation each time this routine is called)

void CWaveBuf::BMedian(short* lpSource, short* lpDest, int cx, int nbspan)
{	
	// assume source buffer is with interleaved channels; if not: scan_count=1
	int nchans = m_waveFormat.scan_count;
	int lpSoffset_nextpoint = nchans;		// number of chans within source buffer
	int lpSoffset_span      = lpSoffset_nextpoint * nbspan;	// offset between center of window & end

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
	short* min_lpSource = m_pWData;
	short* max_lpSource = m_pWData + GetWBNumChannels() * GetWBNumElements();
	short* min_lpDest = m_pWTransf;
	short* max_lpDest = m_pWTransf + GetWBNumElements();
	short* max_parray = &m_parraySorted[m_parray_size-1];
	short* min_parray = m_parraySorted;

	// load parray = consecutive points (image of the data points)	
	int i=0;										// index variable
	short* lp = lpSource;							// pointer to origin of source buffer
	lp -= lpSoffset_span;							// first point of the moving window
	for (i = 0; i < m_parray_size; lp += lpSoffset_nextpoint, i++)
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

	int l = nbspan+ 1;				// temp index
	int ir = m_parray_size - 1;		// temp index
	int j;							// temp index
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
		j = i + i;					// here set up to sift down element to its 
									// proper level
		while (j-1 <= ir)
		{			
			if (j-1 < ir)				
			{
				if (*(m_parraySorted+j-1) < *(m_parraySorted+j))
					j++;			// compare to the better underlining
			}
			if (val < *(m_parraySorted+j-1))// demote val
			{
				*(m_parraySorted+i-1) = *(m_parraySorted+j-1);
				i = j;
				j = j + j;
			}						// this is val's level. Set j to terminate the
			else					// sift-down
				j = ir + 2;
		}
		*(m_parraySorted+i-1) = val;		// put val into its slot
	}
	// end of initial sort

	short newvalue;					// temp variable used in the loop
	short oldvalue;					// temp variable used in the loop
	lp= lpSource;					// first data point
	short* lpNext = lpSource + lpSoffset_span; // last point
	int i_parrayCircular = m_parray_size - 1; // point on the last item so that first operation is blank

	for (cx; cx>0; cx--, lp += lpSoffset_nextpoint, lpNext += lpSoffset_nextpoint, lpDest++)
	{
		oldvalue = *(m_parrayCircular+i_parrayCircular);	// old value
		newvalue = *lpNext;									// new value to insert into array
		*(m_parrayCircular + i_parrayCircular)= newvalue;	// save new value into circular array
		
		// update circular array pointer
		i_parrayCircular++;
		if (i_parrayCircular >= m_parray_size)
			i_parrayCircular = 0;

		// locate position of old value to discard
		// use bisection - cf Numerical Recipes pp 90
		// on exit, j= index of oldvalue

		// binary search
		// Herbert Schildt: C the complete reference McGraw Hill, 1987, pp 488
		int jhigh = m_parray_size-1;	// upper index
		int jlow = 0;					// mid point index
		while (jlow <= jhigh)
		{
			j = (jlow+jhigh)/2;			
			if (oldvalue > *(m_parraySorted+j))
				jlow = j +1;
			else if (oldvalue < *(m_parraySorted+j))
				jhigh = j-1;
			else
				break;
		}

		// insert new value in the correct position

		// case 1: search (and replace) towards higher values
		if (newvalue > *(m_parraySorted + j))
		{	
			for (j; newvalue > *(m_parraySorted + j); j++)
			{								
				if (j == m_parray_size)
					break;
				*(m_parraySorted+ j) = *(m_parraySorted + j + 1);
			} 
			*(m_parraySorted + j-1) = newvalue;
		}

		// case 2: search (and replace) towards lower values
		else if (newvalue < *(m_parraySorted + j))
		{
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
			*(m_parraySorted + j) = newvalue;

		// save median value in the output array
		*lpDest = *lp - *(m_parraySorted+nbspan);
		ASSERT(lp >= min_lpSource);
		ASSERT(lp <= max_lpSource);
	}
}

/**************************************************************************

*/

void CWaveBuf::BRMS(short* lpSource, short* lpDest, int cx)
{
	int nchans = m_waveFormat.scan_count;
	int offsetnextpoint = nchans;
	int nbspan = m_TransformBufferSpan[14] / 2;
	int offsetspan = offsetnextpoint * nbspan;

	// init sum
	double sum2 = 0;
	int N=0;
	short* lp = lpSource;
	lp -= offsetspan;
	nbspan += nbspan;
	for (int i=0; i < nbspan; i++)
	{
		long x = *lp;
		sum2 += x*x;
		lp += offsetnextpoint;
		N++;
	}

	// moving average
	for (cx; cx>0; cx--, lpSource += offsetnextpoint, lpDest++)
	{			
		*lpDest = *lpSource - (short) (sqrt(sum2/N));
		long x = *(lpSource-offsetspan);
		sum2 -= x*x;
		x = *(lpSource+offsetspan);
		sum2 += x*x;
	}
}