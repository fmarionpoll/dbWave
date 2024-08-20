#pragma once
#include "ColorNames.h"


class CHighLight : public CObject
{
public:
	CHighLight();
	DECLARE_SERIAL(CHighLight)
	void Serialize(CArchive& ar) override;
	CHighLight& operator =(const CHighLight& arg);

	int channel {0};
	COLORREF color {col_red};
	int pen_size = 1;
	CArray<long, long> l_first;
	CArray<long, long> l_last;
};

class CEnvelope : public CObject
{
public:
	CEnvelope(); 
	DECLARE_SERIAL(CEnvelope)
	CEnvelope(WORD n_pixels, int data_per_pixel, int ns, int mode, int span);
	void Serialize(CArchive& ar) override;

	// Attributes
protected:
	int m_source_mode_; // operation on raw data (nop, deriv, ...)
	int m_source_chan_; // source channel
	int m_span_; // additional data pts necessary to compute transform
	int m_data_per_pixel_; // 2=simple envelope, 1=raw data
	int m_n_pixels_; // nb of valid elements in the array
	CArray<short, short> m_envelope_; // Envelope array
	static short* get_max_min(int n_elements, short* lp_data, int n_channels, short& i_min, short& i_max, long& y1);

	// Operations
public:
	void fill_envelope_with_abscissa(int n_pixels, int n_points);
	void fill_envelope_with_abscissa_ex(int pix_first, int pix_last, int n_data_points);
	void get_mean_to_abscissa(CArray<CPoint, CPoint>& dest);
	void get_mean_to_ordinates(CArray<CPoint, CPoint>& dest);
	void export_to_abscissa(CArray<CPoint, CPoint>& dest);
	void export_to_ordinates(CArray<CPoint, CPoint>& dest);

	//----------description
	int get_doc_buffer_span() const { return m_span_; }
	void set_doc_buffer_span(const int span) { m_span_ = span; }

	//----------Envelope array
	short* get_element(const int i) { return &m_envelope_[i]; }
	int get_source_mode() const { return m_source_mode_; }
	int get_source_chan() const { return m_source_chan_; }
	int get_envelope_size() const { return m_envelope_.GetSize(); }
	int get_n_elements() const { return m_n_pixels_; }
	short get_at(const int j) const { return m_envelope_[j]; }

	void set_source_mode(const int n, const int span)
	{
		m_source_mode_ = n;
		m_span_ = span;
	}

	void set_source_chan(const int n) { m_source_chan_ = n; }
	void set_point_at(const int j, const short val) { m_envelope_[j] = val; }

	void set_envelope_source_data(const int chan, const int transform)
	{
		m_source_chan_ = chan;
		m_source_mode_ = transform;
	}

	short get_point_at(const int i) { return static_cast<short>(m_envelope_[i]); }

	void set_envelope_size(int n_pixels, int n_data_per_pixel);
	void fill_envelope_with_max_min(int i_first, const short* lp_data, int n_channels, int n_elements, BOOL b_new);
	void fill_envelope_with_smooth_mx_mi(int i_first, short* lp_data, int n_channels, int n_elements, BOOL b_new, int i_option);
	void get_envelope_max_min(int* max, int* min);
	void get_envelope_max_min_between_points(int i_first_pixel, int i_last_pixel, int* max, int* min);
};
