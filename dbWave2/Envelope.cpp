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
	m_source_mode_ = 0; // operation on raw data (nop, deriv, ...)
	m_source_chan_ = 0; // source channel
	m_data_per_pixel_ = 2; // Envelope w. max/min
	m_n_pixels_ = 0;
	m_span_ = 0;
}

CEnvelope::CEnvelope(const WORD n_pixels)
{
	m_data_per_pixel_ = 2; // Envelope w. max/min
	m_n_pixels_ = n_pixels;
	m_envelope_.SetSize(m_n_pixels_, m_data_per_pixel_);
	m_source_mode_ = 0; // operation on raw data (nop, derivative, ...)
	m_source_chan_ = 0;
	m_span_ = 0;
}

CEnvelope::CEnvelope(const WORD n_pixels, const int data_per_pixel, const int ns, const int mode, const int span)
{
	m_data_per_pixel_ = data_per_pixel; // Envelope w. max/min
	m_n_pixels_ = n_pixels;
	m_envelope_.SetSize(m_n_pixels_ * m_data_per_pixel_);
	m_source_mode_ = mode;
	m_source_chan_ = ns;
	m_span_ = span;
}

void CEnvelope::fill_envelope_with_abscissa(int n_pixels, int n_points)
{
	// fill Envelope with data series, step 1
	const auto n = m_envelope_.GetSize();
	if (m_data_per_pixel_ == 1)
		for (auto i = 0; i < n; i++)
			m_envelope_[i] = i;
	// OR fill Envelope with data series, step 2
	else
	{
		if (n_pixels > n_points)
		{
			auto j = 0;
			for (auto i = 0; i < n; i += 2, j++)
			{
				const auto ix = MulDiv(n_pixels, j, n_points);
				m_envelope_[i] = ix;
				m_envelope_[i + 1] = ix;
			}
		}
		else
		{
			auto j = 0;
			for (auto i = 0; i < n; i += 2, j++)
			{
				m_envelope_[i] = j;
				m_envelope_[i + 1] = j;
			}
		}
	}
}

// init values to series (x abscissa)
//	lSize	= nb pts to display
//  xfirst	= first abscissa
//	xlast	= last abscissa
// return nDataperPixel

void CEnvelope::fill_envelope_with_abscissa_ex(int pix_first, int pix_last, int n_data_points)
{
	const auto n = m_envelope_.GetSize();

	// fill Envelope with data series, step 1
	if (m_data_per_pixel_ == 1)
	{
		for (auto i = 0; i < n; i++)
		{
			const auto i_first = MulDiv(n_data_points, i, n) + pix_first;
			m_envelope_[i] = i_first;
		}
	}
	// OR fill Envelope with data series, step 2
	else
	{
		for (auto i = 0; i < n; i += 2)
		{
			const auto i_first = i / 2 + pix_first;
			m_envelope_[i] = i_first;
			m_envelope_[i + 1] = i_first;
		}
	}
}

void CEnvelope::export_to_abscissa(CArray<CPoint, CPoint>& dest)
{
	for (auto i = 0; i < m_envelope_.GetSize(); i++)
		dest.GetAt(i).x = m_envelope_[i];
}

void CEnvelope::export_to_ordinates(CArray<CPoint, CPoint>& dest)
{
	for (auto i = 0; i < m_envelope_.GetSize(); i++)
		dest.GetAt(i).y = m_envelope_[i];
}

void CEnvelope::Serialize(CArchive& ar)
{
	m_envelope_.Serialize(ar); // Envelope array

	if (ar.IsStoring())
	{
		ar << m_source_mode_; 
		ar << m_source_chan_;
		ar << m_span_;
		ar << m_data_per_pixel_;
		ar << m_n_pixels_;
	}
	else
	{
		ar >> m_source_mode_; 
		ar >> m_source_chan_; 
		ar >> m_span_;
		ar >> m_data_per_pixel_; 
		ar >> m_n_pixels_;
	}
}

// GetMeantoPolypoints()
// copy average of 2 consecutive pts (ie (max + min)/2)
// used by lineview to indicate where curves are dragged by the mouse

void CEnvelope::get_mean_to_abscissa(CArray<CPoint, CPoint>& dest)
{
	auto lp_source = &m_envelope_[0]; // source data: a Envelope
	auto jdest = 0;
	for (auto i = m_envelope_.GetSize() / 2; i > 0; i--)
	{
		dest[jdest].x = (*lp_source / 2 + *(lp_source + 1) / 2); // copy half value
		jdest++;
		lp_source += 2;
	}
}

void CEnvelope::get_mean_to_ordinates(CArray<CPoint, CPoint>& dest)
{
	auto lp_source = &m_envelope_[0]; // source data: a Envelope
	auto jdest = 0;
	for (auto i = m_envelope_.GetSize() / 2; i > 0; i--)
	{
		dest[jdest].y = (*lp_source / 2 + *(lp_source + 1) / 2); // copy half value
		jdest++;
		lp_source += 2;
	}
}

void CEnvelope::set_envelope_size(int n_pixels, int n_data_per_pixel)
{
	m_data_per_pixel_ = n_data_per_pixel;
	m_n_pixels_ = n_pixels * n_data_per_pixel;
	m_envelope_.SetSize(m_n_pixels_);
}

// parameters:
// 		short 	ifirst		- index of first pixel to fill
// 		short* 	lpSource	- pointer to the first element of the raw data array
// 		short 	nchans		- number of interleaved channels in the raw data array
// 		short 	nelmts		- number of data to deal with

void CEnvelope::fill_envelope_with_max_min(int i_first, short* lp_data, int n_channels, int n_elements, BOOL b_new)
{
	auto lp_envelope = &m_envelope_[i_first * m_data_per_pixel_];
	if (m_data_per_pixel_ == 1) // only one data point per pixel
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

	while (n_elements > 0) // scan nb elements
	{
		// designed by scale
		const auto idata = *lp_data; // load value
		if (idata < i_min)
			i_min = idata; // change min
		else if (idata > i_max)
			i_max = idata; // change max
		lp_data += n_channels; // update data pointer
		n_elements--;
	}

	*lp_envelope = i_max; // store max
	lp_envelope++; // update Envelope pointer
	*lp_envelope = i_min; // store min
}

short* CEnvelope::get_max_min(int n_elements, short* lp_data, int n_channels, short& i_min, short& i_max, long& y1)
{
	y1 = 0;
	int n = n_elements;
	while (n > 0)
	{
		short idata = *lp_data;
		y1 += idata;
		if (idata < i_min)
			i_min = idata;
		else if (idata > i_max)
			i_max = idata;
		lp_data += n_channels;
		n--;
	}
	if (n_elements > 0)
		y1 = y1 / n_elements;
	return lp_data;
}

void CEnvelope::fill_envelope_with_smooth_mx_mi(int i_first, short* lp_data, int n_channels, int n_elements, BOOL b_new, int i_option)
{
	// simple case: save just the raw data
	short* lp_envelope = &m_envelope_[i_first * m_data_per_pixel_];
	if (m_data_per_pixel_ == 1) // only one data point per pixel
	{
		*lp_envelope = *lp_data; // simply copy data
		return;
	}

	// more than 1 point per interval
	// cut interval in 2 and store either [max, min] or [min, max] depending on the direction of the curve
	// find direction by comparing the average point between the 2 sub-intervals
	short i_max = *lp_data; // init max
	short i_min = i_max; // init min
	if (!b_new)
	{
		i_min = *lp_envelope;
		i_max = *(lp_envelope + 1);
	}

	// first sub-interval
	const int nelemts1 = n_elements / 2;
	long y1 = 0;
	lp_data = get_max_min(nelemts1, lp_data, n_channels, i_min, i_max, y1);

	// second sub-interval
	const int nelemts2 = n_elements - nelemts1;
	long y2 = 0;
	lp_data = get_max_min(nelemts2, lp_data, n_channels, i_min, i_max, y2);

	if (i_option == 0)
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

void CEnvelope::get_envelope_max_min(int* max, int* min)
{
	short maxval = m_envelope_[2];
	short minval = maxval;
	const int npixels = m_envelope_.GetSize();

	// loop over envelope and discard last 2 and first 2 points
	for (auto i = 3; i < npixels - 2; i++)
	{
		short val = m_envelope_[i];
		if (val > maxval)
			maxval = val;
		if (val < minval)
			minval = val;
	}
	*min = static_cast<int>(minval);
	*max = static_cast<int>(maxval);
}

void CEnvelope::get_envelope_max_min_between_points(int i_first_pixel, int i_last_pixel, int* max, int* min)
{
	short maxval = m_envelope_[i_first_pixel];
	short minval = maxval;
	for (int i = i_first_pixel + 1; i <= i_last_pixel; i++)
	{
		short val = m_envelope_[i];
		if (val > maxval)
			maxval = val;
		if (val < minval)
			minval = val;
	}
	*min = static_cast<int>(minval);
	*max = static_cast<int>(maxval);
}
