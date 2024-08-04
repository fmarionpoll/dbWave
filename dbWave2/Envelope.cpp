// envelope.cpp : implementation file
//
// CEnvelope methods

#include "StdAfx.h"
#include "Envelope.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_SERIAL(CHighLight, CObject, 0 /* schema number*/)

CHighLight::CHighLight()
= default;

void CHighLight::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		const WORD version = 1;
		ar << version; // 1
		const auto nitems = 3;
		ar << nitems;
		ar << channel;
		ar << color;
		ar << pensize;
	}
	else
	{
		WORD version;
		ar >> version;
		int nitems;
		ar >> nitems;
		ar >> channel;
		ar >> color;
		ar >> pensize;
	}
	l_first.Serialize(ar);
	l_last.Serialize(ar);
}

CHighLight& CHighLight::operator=(const CHighLight& arg)
{
	if (this != &arg)
	{
		channel = arg.channel;
		color = arg.color;
		pensize = arg.pensize;
		l_first.Copy(arg.l_first);
		l_last.Copy(arg.l_last);
	}
	return *this;
}

IMPLEMENT_SERIAL(CEnvelope, CObject, 0 /* schema number*/)

CEnvelope::CEnvelope()
{
	m_sourceMode = 0; // operation on raw data (nop, deriv, ...)
	m_sourceChan = 0; // source channel
	m_dataperpixel = 2; // Envelope w. max/min
	m_npixels = 0;
	m_span = 0;
}

CEnvelope::CEnvelope(WORD npixels)
{
	m_dataperpixel = 2; // Envelope w. max/min
	m_npixels = npixels;
	m_Envelope.SetSize(m_npixels, m_dataperpixel);
	m_sourceMode = 0; // operation on raw data (nop, deriv, ...)
	m_sourceChan = 0; // source channel
	m_span = 0;
}

CEnvelope::CEnvelope(WORD npixels, int dataperpixel, int ns, int mode, int span)
{
	m_dataperpixel = dataperpixel; // Envelope w. max/min
	m_npixels = npixels;
	m_Envelope.SetSize(m_npixels * m_dataperpixel);
	m_sourceMode = mode;
	m_sourceChan = ns;
	m_span = span;
}

void CEnvelope::fill_envelope_with_abscissa(int npixels, int npoints)
{
	// fill Envelope with data series, step 1
	const auto n = m_Envelope.GetSize();
	if (m_dataperpixel == 1)
		for (auto i = 0; i < n; i++)
			m_Envelope[i] = i;
	// OR fill Envelope with data series, step 2
	else
	{
		if (npixels > npoints)
		{
			auto j = 0;
			for (auto i = 0; i < n; i += 2, j++)
			{
				const auto ix = MulDiv(npixels, j, npoints);
				m_Envelope[i] = ix;
				m_Envelope[i + 1] = ix;
			}
		}
		else
		{
			auto j = 0;
			for (auto i = 0; i < n; i += 2, j++)
			{
				m_Envelope[i] = j;
				m_Envelope[i + 1] = j;
			}
		}
	}
}

// init values to series (x abscissa)
//	lSize	= nb pts to display
//  xfirst	= first abscissa
//	xlast	= last abscissa
// return nDataperPixel

void CEnvelope::fill_envelope_with_abscissa_ex(int pixfirst, int pixlast, int ndatapoints)
{
	const auto n = m_Envelope.GetSize();

	// fill Envelope with data series, step 1
	if (m_dataperpixel == 1)
	{
		for (auto i = 0; i < n; i++)
		{
			const auto ifirst = MulDiv(ndatapoints, i, n) + pixfirst;
			m_Envelope[i] = ifirst;
		}
	}
	// OR fill Envelope with data series, step 2
	else
	{
		for (auto i = 0; i < n; i += 2)
		{
			const auto ifirst = i / 2 + pixfirst;
			m_Envelope[i] = ifirst;
			m_Envelope[i + 1] = ifirst;
		}
	}
}

void CEnvelope::ExportToAbscissa(CArray<CPoint, CPoint>& dest)
{
	for (auto i = 0; i < m_Envelope.GetSize(); i++)
		dest.GetAt(i).x = m_Envelope[i];
}

void CEnvelope::ExportToOrdinates(CArray<CPoint, CPoint>& dest)
{
	for (auto i = 0; i < m_Envelope.GetSize(); i++)
		dest.GetAt(i).y = m_Envelope[i];
}

void CEnvelope::Serialize(CArchive& ar)
{
	m_Envelope.Serialize(ar); // Envelope array

	if (ar.IsStoring())
	{
		ar << m_sourceMode; // operation on raw data (nop, deriv, ...)
		ar << m_sourceChan; // source channel
		ar << m_span; // additionnal data pts necessary to compute transform
		ar << m_dataperpixel; // 2=simple envelope, 1=raw data
		ar << m_npixels; // nb of valid elements in the array
	}
	else
	{
		ar >> m_sourceMode; // operation on raw data (nop, deriv, ...)
		ar >> m_sourceChan; // source channel
		ar >> m_span; // additionnal data pts necessary to compute transform
		ar >> m_dataperpixel; // 2=simple envelope, 1=raw data
		ar >> m_npixels; // nb of valid elements in the array
	}
}

// GetMeantoPolypoints()
// copy average of 2 consecutive pts (ie (max + min)/2)
// used by lineview to indicate where curves are dragged by the mouse

void CEnvelope::get_mean_to_abscissa(CArray<CPoint, CPoint>& dest)
{
	auto lp_source = &m_Envelope[0]; // source data: a Envelope
	auto jdest = 0;
	for (auto i = m_Envelope.GetSize() / 2; i > 0; i--)
	{
		dest[jdest].x = (*lp_source / 2 + *(lp_source + 1) / 2); // copy half value
		jdest++;
		lp_source += 2;
	}
}

void CEnvelope::GetMeanToOrdinates(CArray<CPoint, CPoint>& dest)
{
	auto lp_source = &m_Envelope[0]; // source data: a Envelope
	auto jdest = 0;
	for (auto i = m_Envelope.GetSize() / 2; i > 0; i--)
	{
		dest[jdest].y = (*lp_source / 2 + *(lp_source + 1) / 2); // copy half value
		jdest++;
		lp_source += 2;
	}
}

void CEnvelope::SetEnvelopeSize(int npixels, int ndataperpixel)
{
	m_dataperpixel = ndataperpixel;
	m_npixels = npixels * ndataperpixel;
	m_Envelope.SetSize(m_npixels);
}

// parameters:
// 		short 	ifirst		- index of first pixel to fill
// 		short* 	lpSource	- pointer to the first element of the raw data array
// 		short 	nchans		- number of interleaved channels in the raw data array
// 		short 	nelmts		- number of data to deal with

void CEnvelope::FillEnvelopeWithMxMi(int ifirst, short* lp_data, int nchans, int nelmts, BOOL b_new)
{
	auto lp_envelope = &m_Envelope[ifirst * m_dataperpixel];
	if (m_dataperpixel == 1) // only one data point per pixel
	{
		*lp_envelope = *lp_data; // simply copy data
		return;
	}

	int i_max = *lp_data; // init max
	auto i_min = i_max; // init min
	if (!b_new)
	{
		i_min = *lp_envelope;
		i_max = *(lp_envelope + 1);
	}

	while (nelmts > 0) // scan nb elements
	{
		// designed by scale
		const auto idata = *lp_data; // load value
		if (idata < i_min)
			i_min = idata; // change min
		else if (idata > i_max)
			i_max = idata; // change max
		lp_data += nchans; // update data pointer
		nelmts--;
	}

	*lp_envelope = i_max; // store max
	lp_envelope++; // update Envelope pointer
	*lp_envelope = i_min; // store min
}

short* CEnvelope::getMaxMin(int nelements, short* lpData, int nchans, short& i_min, short& i_max, long& y1)
{
	y1 = 0;
	int n = nelements;
	while (n > 0)
	{
		short idata = *lpData;
		y1 += idata;
		if (idata < i_min)
			i_min = idata;
		else if (idata > i_max)
			i_max = idata;
		lpData += nchans;
		n--;
	}
	if (nelements > 0)
		y1 = y1 / nelements;
	return lpData;
}

void CEnvelope::FillEnvelopeWithSmoothMxMi(int ifirst, short* lpData, int nchans, int nelmts, BOOL bNew, int ioption)
{
	// simple case: save just the raw data
	short* lp_envelope = &m_Envelope[ifirst * m_dataperpixel];
	if (m_dataperpixel == 1) // only one data point per pixel
	{
		*lp_envelope = *lpData; // simply copy data
		return;
	}

	// more than 1 point per interval
	// cut interval in 2 and store either [max, min] or [min, max] depending on the direction of the curve
	// find direction by comparing the average point between the 2 sub-intervals
	short i_max = *lpData; // init max
	short i_min = i_max; // init min
	if (!bNew)
	{
		i_min = *lp_envelope;
		i_max = *(lp_envelope + 1);
	}

	// first sub-interval
	const int nelemts1 = nelmts / 2;
	long y1 = 0;
	lpData = getMaxMin(nelemts1, lpData, nchans, i_min, i_max, y1);

	// second sub-interval
	const int nelemts2 = nelmts - nelemts1;
	long y2 = 0;
	lpData = getMaxMin(nelemts2, lpData, nchans, i_min, i_max, y2);

	if (ioption == 0)
	{
		if (y1 > y2)
		{
			const auto idummy = i_min;
			i_min = i_max;
			i_max = idummy;
		}
	}
	else //if (ioption == 2)
	{
		i_min = static_cast<short>(y1);
		i_max = static_cast<short>(y2);
	}

	*lp_envelope = i_min;
	lp_envelope++;
	*lp_envelope = i_max;
}

// GetEnvelopeMaxMin

void CEnvelope::GetEnvelopeMaxMin(int* max, int* min)
{
	short maxval = m_Envelope[2];
	short minval = maxval;
	const int npixels = m_Envelope.GetSize();

	// loop over envelope and discard last 2 and first 2 points
	for (auto i = 3; i < npixels - 2; i++)
	{
		short val = m_Envelope[i];
		if (val > maxval)
			maxval = val;
		if (val < minval)
			minval = val;
	}
	*min = static_cast<int>(minval);
	*max = static_cast<int>(maxval);
}

void CEnvelope::GetEnvelopeMaxMinBetweenPoints(int ifirstpixel, int ilastpixel, int* max, int* min)
{
	short maxval = m_Envelope[ifirstpixel];
	short minval = maxval;
	for (int i = ifirstpixel + 1; i <= ilastpixel; i++)
	{
		short val = m_Envelope[i];
		if (val > maxval)
			maxval = val;
		if (val < minval)
			minval = val;
	}
	*min = static_cast<int>(minval);
	*max = static_cast<int>(maxval);
}
