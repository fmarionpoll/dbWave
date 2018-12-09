// envelope.cpp : implementation file
//
// CEnvelope methods

#include "StdAfx.h"
#include "Envelope.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CEnvelope
// --------------------------------------------------------------------------
IMPLEMENT_SERIAL(CEnvelope, CObject, 0 /* schema number*/ )

// --------------------------------------------------------------------------
CEnvelope::CEnvelope()
{
	m_sourceMode=0;			// operation on raw data (nop, deriv, ...)
	m_sourceChan=0;			// source channel
	m_dataperpixel = 2;		// Envelope w. max/min
	m_npixels = 0;
	m_span=0;
}


// --------------------------------------------------------------------------
// create Envelope with npoints
// --------------------------------------------------------------------------
CEnvelope::CEnvelope(WORD npixels)
{
	m_dataperpixel = 2;		// Envelope w. max/min
	m_npixels = npixels;
	m_Envelope.SetSize(m_npixels, m_dataperpixel);
	m_sourceMode=0;			// operation on raw data (nop, deriv, ...)
	m_sourceChan=0;			// source channel
	m_span=0;
}


// --------------------------------------------------------------------------
// create Envelope with npoints
// --------------------------------------------------------------------------
CEnvelope::CEnvelope(WORD npixels, int dataperpixel, int ns, int mode, int span)
{
	m_dataperpixel = dataperpixel;		// Envelope w. max/min
	m_npixels = npixels;
	m_Envelope.SetSize(m_npixels*m_dataperpixel);
	m_sourceMode = mode;
	m_sourceChan = ns;
	m_span=span;
}


// --------------------------------------------------------------------------
// FillEnvelopeWithAbcissa
//
// init values to series (x abcissa)
//	npixels		= number of pixelsindex last pixel
//	lSize		= n pts to display
// return nDataperPixel
// --------------------------------------------------------------------------

void CEnvelope::FillEnvelopeWithAbcissa (int npixels, int npoints)
{	
	// fill Envelope with data series, step 1	
	int n = m_Envelope.GetSize();
	if (m_dataperpixel == 1)
		for (int i = 0; i<n; i++)
			m_Envelope[i]=i;
	// OR fill Envelope with data series, step 2
	else
	{
		if (npixels > npoints)
		{
			int ix = 0;
			int j= 0;
			for (int i = 0; i < n; i+= 2, j++)
			{
				ix = MulDiv(npixels, j, npoints);
				m_Envelope[i]= ix;
				m_Envelope[i+1]=ix;
			}
		}
		else
		{
			int j=0;
			for (int i = 0; i<n; i+= 2, j++)
			{
				m_Envelope[i]=j;
				m_Envelope[i+1]=j;
			}
		}
	}
}


// --------------------------------------------------------------------------
// FillEnvelopeWithAbcissaEx
//
// init values to series (x abcissa)
//	lSize	= nb pts to display
//  xfirst	= first abcissa
//	xlast	= last abcissa
// return nDataperPixel
// --------------------------------------------------------------------------

void CEnvelope::FillEnvelopeWithAbcissaEx (int pixfirst, int pixlast, int ndatapoints)
{
	int npixels = pixlast - pixfirst +1;
	int n = m_Envelope.GetSize();

	// fill Envelope with data series, step 1	
	int i, ifirst;
	if (m_dataperpixel == 1)
	{
		for (i = 0; i < n; i++)
		{
			ifirst = MulDiv(ndatapoints, i, n) + pixfirst;
			m_Envelope[i] = ifirst;
		}
	}
	// OR fill Envelope with data series, step 2
	else
	{
		for (i = 0; i < n; i += 2)
		{
			ifirst			= i/2 + pixfirst;
			m_Envelope[i]	=ifirst;
			m_Envelope[i+1]	=ifirst;
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
// --------------------------------------------------------------------------
// CEnvelope serialization
// --------------------------------------------------------------------------
void CEnvelope::Serialize(CArchive& ar)
{
	m_Envelope.Serialize(ar);	// Envelope array

	if (ar.IsStoring())
	{
		ar << m_sourceMode;		// operation on raw data (nop, deriv, ...)
		ar << m_sourceChan;		// source channel
		ar << m_span;			// additionnal data pts necessary to compute transform
		ar << m_dataperpixel;	// 2=simple envelope, 1=raw data 
		ar << m_npixels;		// nb of valid elements in the array
	}
	else
	{
		ar >> m_sourceMode;		// operation on raw data (nop, deriv, ...)
		ar >> m_sourceChan;		// source channel
		ar >> m_span;			// additionnal data pts necessary to compute transform
		ar >> m_dataperpixel;	// 2=simple envelope, 1=raw data 
		ar >> m_npixels;		// nb of valid elements in the array
	}
}


// --------------------------------------------------------------------------
// GetMeantoPolypoints()
// copy mean of 2 consecutive pts (ie (max + min)/2)
// used by lineview to indicate where curves are dragged by the mouse
// --------------------------------------------------------------------------
void CEnvelope::GetMeantoPolypoints(long* lpDest)
{
	short* lpSource = (short*) &m_Envelope[0];	// source data: a Envelope
	int i = m_Envelope.GetSize()/2;
	for (i; i>0; i--, lpDest+= 2, lpSource += 2)
	{
		*lpDest = (*lpSource/2 + *(lpSource+1)/2);	// copy half val
	}
}


// --------------------------------------------------------------------------

void CEnvelope::SetEnvelopeSize(int npixels, int ndataperpixel)
{
	m_dataperpixel= ndataperpixel;
	m_npixels=npixels * ndataperpixel;
	m_Envelope.SetSize(m_npixels);
}

// --------------------------------------------------------------------------
	// FillEnvelopeWithMxMi
	//
	// parameters:
	// 		short 	ifirst		- index of first pixel to fill
	// 		short* 	lpSource	- pointer to the first element of the raw data array
	// 		short 	nchans		- number of interleaved channels in the raw data array
	// 		short 	nelmts		- number of data to deal with
	// 
	// --------------------------------------------------------------------------

	//
void CEnvelope::FillEnvelopeWithMxMi(int ifirst, short* lpData, int nchans, int nelmts, BOOL bNew)
{	
	short* lpEnvelope = (short*) &m_Envelope[ifirst*m_dataperpixel];
	if (m_dataperpixel == 1)	// only one data point per pixel
	{
		*lpEnvelope = *lpData;	// simply copy data
		return;
	}

	int iMax= *lpData;			// init max
	int iMin= iMax;				// init min
	if (!bNew)
	{
		iMin = *lpEnvelope;
		iMax = *(lpEnvelope+1);
	}

	int idata;
	while (nelmts > 0)			// scan nb elements
	{							// designed by scale
		idata = *lpData;		// load value
		if (idata < iMin)
			iMin = idata;		// change min
		else if (idata > iMax)  
			iMax = idata;		// change max
		lpData += nchans;		// update data pointer 
		nelmts--;
	}

	*lpEnvelope = iMax;			// store max
	lpEnvelope++;				// update Envelope pointer
	*lpEnvelope = iMin;			// store min
	lpEnvelope++;				// update Envelope pointer
}

void CEnvelope::FillEnvelopeWithSmoothMxMi(int ifirst, short* lpData, int nchans, int nelmts, BOOL bNew, int ioption)
{	
	// simple case: save just the raw data
	short* lpEnvelope = (short*) &m_Envelope[ifirst*m_dataperpixel];
	if (m_dataperpixel == 1)	// only one data point per pixel
	{
		*lpEnvelope = *lpData;	// simply copy data
		return;
	}

	// more than 1 point per interval
	// cut interval in 2 and store either [max, min] or [min, max] depending on the direction of the curve
	// find direction by comparing the average point between the 2 sub-intervals
	int iMax= *lpData;		// init max
	int iMin= iMax;			// init min
	long y1, y2;
	y1 = 0;
	y2 = 0;
	if (!bNew)
	{
		iMin = *lpEnvelope;
		iMax = *(lpEnvelope+1);
	}

	int nelemts1 = nelmts/2;
	int nelemts2 = nelmts - nelemts1;
	short idata;

	// first subinterval
	int n= nelemts1;
	while (n > 0)				// scan nb elements
	{							// designed by scale
		idata = *lpData;		// load value
		y1 += idata;			// change max
		if (idata < iMin)
			iMin = idata;		// change min
		else if (idata > iMax)  
			iMax = idata;		// change max
		lpData += nchans;		// update data pointer 
		n--;
	}
	if (nelemts1 > 0)
		y1 = y1/nelemts1;

	// second sub-interval
	n = nelemts2;
	while (n > 0)				// scan nb elements
	{							// designed by scale
		idata = *lpData;		// load value
		y2 += idata;			// change max
		if (idata < iMin)
			iMin = idata;		// change min
		else if (idata > iMax)  
			iMax = idata;		// change max
		lpData += nchans;		// update data pointer 
		n--;
	}
	if (nelemts2 > 0)
		y2 = y2 / nelemts2;

	if (ioption == 0)
	{
		if (y1 > y2)
		{
			int idummy = iMin;
			iMin = iMax;
			iMax = idummy;
		}
	}
	else //if (ioption == 2)
	{
		iMin = y1;
		iMax = y2;
	}

	*lpEnvelope = iMin;			// store max
	lpEnvelope++;				// update Envelope pointer
	*lpEnvelope = iMax;			// store min
	lpEnvelope++;				// update Envelope pointer
}

// --------------------------------------------------------------------------
// GetEnvelopeMaxMin
// --------------------------------------------------------------------------

void CEnvelope::GetEnvelopeMaxMin(int* max, int* min)
{	
	short* pEnvel = (short*)&m_Envelope[2];
	int maxval = *pEnvel;			// get first max and min short
	int minval = maxval;
	int npixels = m_Envelope.GetSize();

	// loop over envelope and discard last 2 and first 2 points
	for (int i = npixels-2; i>1; i--, pEnvel++ )	
	{
		if (*pEnvel > maxval)		// is max max?
			maxval = *pEnvel;		// no
		if (*pEnvel < minval)		// is min min?
			minval = *pEnvel;		// no
	}
	*min = minval;					// return value of min
	*max = maxval;					// return value of max
}
