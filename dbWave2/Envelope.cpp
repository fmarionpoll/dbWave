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
		constexpr WORD version = 1;
		ar << version; // 1
		constexpr auto n_items = 3;
		ar << n_items;
		ar << channel;
		ar << color;
		ar << pen_size;
	}
	else
	{
		WORD version;
		ar >> version;
		int n_items;
		ar >> n_items;
		ar >> channel;
		ar >> color;
		ar >> pen_size;
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
		pen_size = arg.pen_size;
		l_first.Copy(arg.l_first);
		l_last.Copy(arg.l_last);
	}
	return *this;
}

IMPLEMENT_SERIAL(CEnvelope, CObject, 0 /* schema number*/)

CEnvelope::CEnvelope()
{
	m_source_mode_ = 0; 
	m_source_chan_ = 0;
	m_data_per_pixel_ = 2; 
	m_n_pixels_ = 0;
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
		for (short i = 0; i < n; i++)
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
				m_envelope_[i] = static_cast<short>(ix);
				m_envelope_[i + 1] = static_cast<short>(ix);
			}
		}
		else
		{
			short j = 0;
			for (auto i = 0; i < n; i += 2, j++)
			{
				m_envelope_[i] = j;
				m_envelope_[i + 1] = j;
			}
		}
	}
}

// init values to series (x abscissa)
//	l_Size	= nb pts to display
//  x_first	= first abscissa
//	x_last	= last abscissa
// return n_Data_per_Pixel

void CEnvelope::fill_envelope_with_abscissa_ex(const int pix_first, int pix_last, const int n_data_points)
{
	const auto n = m_envelope_.GetSize();

	// fill Envelope with data series, step 1
	if (m_data_per_pixel_ == 1)
	{
		for (auto i = 0; i < n; i++)
		{
			const auto i_first = MulDiv(n_data_points, i, n) + pix_first;
			m_envelope_[i] = static_cast<short>(i_first);
		}
	}
	// OR fill Envelope with data series, step 2
	else
	{
		for (short i = 0; i < n; i += 2)
		{
			const short i_first = static_cast<short>(i / 2 + pix_first);
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
	m_envelope_.Serialize(ar); 

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

// get_mean_to_abscissa()
// copy average of 2 consecutive pts (ie (max + min)/2)
// used by line_view to indicate where curves are dragged by the mouse

void CEnvelope::get_mean_to_abscissa(CArray<CPoint, CPoint>& dest)
{
	auto lp_source = &m_envelope_[0]; // source data: a Envelope
	auto j_dest = 0;
	for (auto i = m_envelope_.GetSize() / 2; i > 0; i--)
	{
		dest[j_dest].x = (*lp_source / 2 + *(lp_source + 1) / 2); // copy half value
		j_dest++;
		lp_source += 2;
	}
}

void CEnvelope::get_mean_to_ordinates(CArray<CPoint, CPoint>& dest)
{
	auto lp_source = &m_envelope_[0]; // source data: a Envelope
	auto j_dest = 0;
	for (auto i = m_envelope_.GetSize() / 2; i > 0; i--)
	{
		dest[j_dest].y = (*lp_source / 2 + *(lp_source + 1) / 2); // copy half value
		j_dest++;
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
// 		short 	i_first		- index of first pixel to fill
// 		short* 	lp_Source	- pointer to the first element of the raw data array
// 		short 	n_channels		- number of interleaved channels in the raw data array
// 		short 	n_elements		- number of data to deal with

void CEnvelope::fill_envelope_with_max_min(const int i_first, const short* lp_data, const int n_channels, int n_elements, const BOOL b_new)
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
		const auto i_data = *lp_data; // load value
		if (i_data < i_min)
			i_min = i_data; // change min
		else if (i_data > i_max)
			i_max = i_data; // change max
		lp_data += n_channels; // update data pointer
		n_elements--;
	}

	*lp_envelope = static_cast<short>(i_max); // store max
	lp_envelope++; // update Envelope pointer
	*lp_envelope = static_cast<short>(i_min); // store min
}

short* CEnvelope::get_max_min(const int n_elements, short* lp_data, const int n_channels, short& i_min, short& i_max, long& y1)
{
	y1 = 0;
	int n = n_elements;
	while (n > 0)
	{
		const short i_data = *lp_data;
		y1 += i_data;
		if (i_data < i_min)
			i_min = i_data;
		else if (i_data > i_max)
			i_max = i_data;
		lp_data += n_channels;
		n--;
	}
	if (n_elements > 0)
		y1 = y1 / n_elements;
	return lp_data;
}

void CEnvelope::fill_envelope_with_smooth_mx_mi(const int i_first, short* lp_data, const int n_channels, const int n_elements, const BOOL b_new, const int i_option)
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
	const int n_elements_1 = n_elements / 2;
	long y1 = 0;
	lp_data = get_max_min(n_elements_1, lp_data, n_channels, i_min, i_max, y1);

	// second sub-interval
	const int n_elements_2 = n_elements - n_elements_1;
	long y2 = 0;
	get_max_min(n_elements_2, lp_data, n_channels, i_min, i_max, y2);

	if (i_option == 0)
	{
		if (y1 > y2)
		{
			const auto i_dummy = i_min;
			i_min = i_max;
			i_max = i_dummy;
		}
	}
	else //if (i_option == 2)
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
	if (m_envelope_.GetSize() < 3)
	{
		*max = 8192;
		*min = -8192;
		return;
	}
		
	short max_val = m_envelope_[2];
	short min_val = max_val;
	const int n_pixels = m_envelope_.GetSize();

	// loop over envelope and discard last 2 and first 2 points
	for (auto i = 3; i < n_pixels - 2; i++)
	{
		const short val = m_envelope_[i];
		if (val > max_val)
			max_val = val;
		if (val < min_val)
			min_val = val;
	}
	*min = static_cast<int>(min_val);
	*max = static_cast<int>(max_val);
}

void CEnvelope::get_envelope_max_min_between_points(int i_first_pixel, int i_last_pixel, int* max, int* min)
{
	short max_val = m_envelope_[i_first_pixel];
	short min_val = max_val;
	for (int i = i_first_pixel + 1; i <= i_last_pixel; i++)
	{
		const short val = m_envelope_[i];
		if (val > max_val)
			max_val = val;
		if (val < min_val)
			min_val = val;
	}
	*min = static_cast<int>(min_val);
	*max = static_cast<int>(max_val);
}
