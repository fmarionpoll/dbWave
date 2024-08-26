#include "StdAfx.h"
#include <cmath>
#include "WaveBuf.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_SERIAL(CWaveBuf, CObject, 0)

CWaveBuf::CWaveBuf()
= default;

CWaveBuf::~CWaveBuf()
{
	delete_buffers();
	SAFE_DELETE_ARRAY(p_array_sorted_)
	SAFE_DELETE_ARRAY(p_array_circular_)
}

int CWaveBuf::create_buffer_with_n_channels(const int i)
{
	// no action if the number of data channels is right
	if (wave_format_.scan_count == i
		&& channels_array_.chan_array_get_size() == i)
		return i;
	const auto i_num_elements = wave_format_.buffer_size / wave_format_.scan_count;
	wave_format_.scan_count = static_cast<short>(i);
	channels_array_.chan_array_set_size(i);
	ASSERT(channels_array_.chan_array_get_size() == wave_format_.scan_count);
	create_w_buffer(i_num_elements, i);
	return wave_format_.scan_count;
}

short* CWaveBuf::get_pointer_to_raw_data_buffer() const
{
	ASSERT(p_w_data_ != NULL);
	return p_w_data_;
}

short* CWaveBuf::get_pointer_to_raw_data_element(const int chan, const int index) const
{
	ASSERT(chan < wave_format_.scan_count);
	ASSERT(p_w_data_ != NULL);
	return p_w_data_ + index * wave_format_.scan_count + chan;
}

short* CWaveBuf::get_pointer_to_transformed_data_buffer() const
{
	return p_w_data_transformed_;
}

short* CWaveBuf::get_pointer_to_transformed_data_element(const int index) const
{
	return p_w_data_transformed_ + index;
}

int CWaveBuf::get_wb_n_elements() const
{
	return num_elements_;
}

int CWaveBuf::get_wb_n_channels() const
{
	return wave_format_.scan_count;
}

void CWaveBuf::set_wb_sample_rate(const float sampling_rate)
{
	wave_format_.sampling_rate_per_channel = sampling_rate;
}

float CWaveBuf::get_wb_sample_rate() const
{
	return wave_format_.sampling_rate_per_channel;
}

void CWaveBuf::delete_buffers()
{
	if (p_w_data_ != nullptr)
	{
		free(p_w_data_);
		p_w_data_ = nullptr;
	}

	if (p_w_data_transformed_ != nullptr)
	{
		free(p_w_data_transformed_);
		p_w_data_transformed_ = nullptr;
	}
	num_elements_ = 0;
}

// -----------------------------------------------------------
BOOL CWaveBuf::create_w_buffer(const int i_num_elements, const int n_channels)
{
	ASSERT(this);
	ASSERT(i_num_elements >= 1); // must have at least one
	wave_format_.scan_count = static_cast<short>(n_channels);

	constexpr DWORD dw_safe_factor = 256; // safety factor to fit data in buffer
	const size_t dw_buffer_size = i_num_elements * n_channels * sizeof(short) + dw_safe_factor;
	if (dw_buffer_size_ != dw_buffer_size)
	{
		delete_buffers();
		p_w_data_ = static_cast<short*>(malloc(dw_buffer_size));
		ASSERT(p_w_data_ != NULL);
		if (p_w_data_ == nullptr)
			return FALSE;

		// allocate transform heap if selected
		if (is_transformed_)
		{
			const size_t dw_buffer_size2 = i_num_elements * sizeof(short) + dw_safe_factor * 2 * 60;
			p_w_data_transformed_ = static_cast<short*>(malloc(dw_buffer_size2));
			ASSERT(p_w_data_transformed_ != NULL);
			if (p_w_data_transformed_ == nullptr)
				return FALSE;
		}
		num_elements_ = i_num_elements;
	}
	wave_format_.buffer_size = static_cast<WORD>(i_num_elements * n_channels);
	return TRUE;
}

// -----------------------------------------------------------

void CWaveBuf::Serialize(CArchive& ar)
{
	CObject::Serialize(ar);
	wave_format_.Serialize(ar); // serialize data description
	channels_array_.Serialize(ar); // serialize channels description

	if (ar.IsStoring())
	{
		ar << w_version_;
	}
	else
	{
		WORD version;
		ar >> version;
	}
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// add description of new method here

std::string CWaveBuf::m_p_transforms_allowed_[] = {
	"BMAXMIN  - Raw data", // 0
	"BDERIVAT - First derivative", // 1
	"BLANCZO2 - Lanczos filter 2l (1/5T)  Ci=1,2", // 2
	"BLANCZO3 - Lanczos filter 3l (1/14T) Ci=1,2,3", // 3
	"BDERI1F3 - Derivative (1/3T)  1f3 Ci=1,1", // 4
	"BDERI2F3 - Derivative (1/6T)  2f3 Ci=1,1,1", // 5
	"BDERI2F5 - Derivative (1/10T) 2f5 Ci=1,1,1,1", // 6
	"BDERI3F3 - Derivative (1/9T)  3f3 Ci=0,1,1,1", // 7
	"BDIFFER1 - Difference X(n+1)-X(n-1)", // 8
	"BDIFFER2 - Difference X(n+2)-X(n-2)", // 9
	"BDIFFER3 - Difference X(n+3)-X(n-3)", // 10
	"BDIFFER10 - Difference X(n+10)-X(n-10)", // 11
	"MOVAVG30 - Moving average, 30 points", // 12
	"MEDIAN30 - Moving median , 30 points", // 13
	"MEDIAN35 - Moving median , 35 points", // 14
	"RMS30 - Moving RMS, 30 points", // 15
	""
};

int CWaveBuf::transforms_count_ = 16;

// -----------------------------------------------------------

int CWaveBuf::m_transform_buffer_span_[] = {
	0, // maxmin
	4, // bderiv
	2, // lanczos2
	3, // lanczos3
	2, // deriv2
	3, // deriv3
	4, // deriv4
	4, // deriv4 bis
	1, // diff n+1/n-1
	2, // diff2 n+2/n-2
	3, // diff3 n+3/n-3
	10, // diff n+10/n-10
	60, // moving average 60*2
	30, // median 30
	35, // median 35
	30, // RMS
	-1
};

// -----------------------------------------------------------

int CWaveBuf::b_variable_span_[] = {
	0, // max_min
	0, // b_deriv
	0, // lanczos_2
	0, // lanczos_3
	0, // deriv_2
	0, // deriv3
	0, // deriv4
	0, // deriv4 bis
	0, // diff n+1/n-1
	0, // diff2 n+2/n-2
	0, // diff3 n+3/n-3
	0, // diff n+10/n-10
	1, // moving average
	1, // median filter 30
	1, // median filter 35
	1, // root-mean-square
	-1
};

// -----------------------------------------------------------

float CWaveBuf::m_correction_factor_[] = {
	1.f, // maxmin
	6.f, // bderiv
	5.f, // lanczos2
	14.f, // lanczos3
	3.f, // deriv2
	6.f, // deriv3
	10.f, // deriv4
	9.f, // deriv4 bis
	1.f, // diff1
	1.f, // diff2 n+2/n-2
	1.f, // diff3 n+3/n-3
	1.f, // diff10
	1.f, // MOVAVG30
	1.f, // Median30
	1.f, // Median35
	1.f, // Root-mean-square
	1.f
};

///////////////////////////////////////////////////////////////////////////////////////

int CWaveBuf::wb_get_n_transforms()
{
	if (transforms_count_ < 0)
	{
		transforms_count_ = 0;
		while (m_transform_buffer_span_[transforms_count_] >= 0)
			transforms_count_++;
	}
	return transforms_count_;
}

CString CWaveBuf::wb_get_transforms_allowed(int i)
{
	if (i < 0)
		i = 0;
	else if (i > transforms_count_)
		i = transforms_count_;
	return CString(m_p_transforms_allowed_[i].c_str());
}

BOOL CWaveBuf::wb_is_transform_allowed(int i)
{
	auto flag = TRUE;
	if (i < 0 || i > transforms_count_) 
		flag = FALSE;
	return flag;
}

BOOL CWaveBuf::wb_init_transform_buffer()
{
	const size_t dw_buffer_size = wave_format_.sample_count * sizeof(short) + 100;
	// allocate / reallocate transform heap
	if (p_w_data_transformed_ != nullptr)
	{
		free(p_w_data_transformed_);
		p_w_data_transformed_ = nullptr;
	}

	p_w_data_transformed_ = static_cast<short*>(malloc(dw_buffer_size));
	if (p_w_data_transformed_ == nullptr)
		return FALSE;
	const auto flag = (p_w_data_transformed_ != nullptr);
	is_transformed_ = TRUE;
	return flag;
}

int CWaveBuf::wb_get_transform_span(int i)
{
	if (i < 0 || i > transforms_count_)
		return 0;
	return m_transform_buffer_span_[i];
}

// -----------------------------------------------------------
// note: span width can be modified for several routines (like running average, running median, ...)
// those routines have a flag in b_variable_Span = 1
// the modification can be done within the dialog box "format series"

int CWaveBuf::wb_is_span_change_allowed(const int i)
{
	return b_variable_span_[i];
}

int CWaveBuf::wb_set_transform_span(const int i, const int span)
{
	if (b_variable_span_[i] > 0)
		m_transform_buffer_span_[i] = span;
	return m_transform_buffer_span_[i];
}

int CWaveBuf::wb_get_correction_factor(int i, float* correct)
{
	if (i < 0 || i > transforms_count_)
		i = -1;
	else
		*correct = m_correction_factor_[i];
	return i;
}

/**************************************************************************
 function:	B_Deriv
 purpose:	compute first derivative of one channel from lp_source
			and store the result in transform buffer lp_dest
 parameters:	lp_source pointer to 1rst point to transform from RW buffer x(k=0)
				lp_dest	 pointer to destination buffer
				cx = n items	n points to examine
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

	The expression is evaluated in two steps. 
	Expression (a) equals expression (b) * (-1) 4 steps later. The
	algorithm actually implemented use this fact to evaluate this
	expression only once, but stores it in y(k+4) and subtract it
	from y(k), using ax, bx, and dx to store x(k+1), x(k+2)
	and x(k+3).

  !!! this filter produces an array with 2 to 6 missing points
	   at the beginning and at the end of the array
 **************************************************************************/

void CWaveBuf::low_pass_differentiation(const short* lp_source, short* lp_dest, int cx) const
{
	constexpr auto i_span = 4; // nb of dummy pts on each side of dest buffer
	auto lp_dest0 = lp_dest; // store initial address (clear at the end)
	const int n_channels = wave_format_.scan_count;
	lp_source -= i_span * n_channels; // init source address: k-3
	cx += i_span; // modify nb of pts
	int bx = *lp_source; // xk+1 load first data pts
	lp_source += n_channels; // update offset
	int dx = *lp_source; // xk+2
	lp_source += n_channels; // update offset

	// compute the derivative
	for (auto i = cx; i > 0; i--, lp_source += n_channels, lp_dest++)
	{
		auto ax = bx; // ax=xk+1 -- register to reg
		bx = dx; // bx=xk+2 -- register to reg
		dx = *lp_source; // dx=xk+3 -- load from memory
		ax += bx + dx; // =SUM(k+4) -- build intermediate result
		*(lp_dest + i_span) = static_cast<short>(-ax); // y(k+4) = 2048 - SUM(k+4) -- store
		*lp_dest += static_cast<short>(ax); // y(k) = 2048 - SUM(k-4) + SUM(k) -- store final
	}

	// set undefined pts to zero
	for (auto icx = i_span; icx > 0; icx--, lp_dest++, lp_dest0++)
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

void CWaveBuf::copy(const short* lp_source, short* lp_dest, const int cx) const
{
	const int n_channels = wave_format_.scan_count;
	for (auto icx = cx; icx > 0; icx--, lp_source += n_channels, lp_dest++)
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

void CWaveBuf::low_pass_diff_3(const short* lp_source, short* lp_dest, const int cx) const
{
	const int n_channels = wave_format_.scan_count;
	// compute the difference
	const auto offset3 = 3 * n_channels;
	for (auto icx = cx; icx > 0; icx--, lp_source += n_channels, lp_dest++)
		*lp_dest = static_cast<short>(*(lp_source + offset3) - *(lp_source - offset3));
}

//**************************************************************************
// 	"BDIFFER2 - Difference X(n+2)-X(n-2)",
//**************************************************************************
void CWaveBuf::low_pass_diff_2(const short* lp_source, short* lp_dest, const int cx) const
{
	const int n_channels = wave_format_.scan_count;
	const auto offset2 = 2 * n_channels;
	for (auto icx = cx; icx > 0; icx--, lp_source += n_channels, lp_dest++)
		*lp_dest = static_cast<short>(*(lp_source + offset2) - *(lp_source - offset2));
}

//**************************************************************************
// 	"BDIFFER1 - Difference X(n+1)-X(n-1)",
//**************************************************************************
void CWaveBuf::low_pass_diff_1(const short* lp_source, short* lp_dest, const int cx) const
{
	const int n_channels = wave_format_.scan_count;
	const auto offset = n_channels;
	for (auto icx = cx; icx > 0; icx--, lp_source += n_channels, lp_dest++)
		*lp_dest = static_cast<short>(*(lp_source + offset) - *(lp_source - offset));
}

//**************************************************************************
// 	"BDIFFER10 - Difference X(n+10)-X(n-10)",
//**************************************************************************
void CWaveBuf::low_pass_diff_10(const short* lp_source, short* lp_dest, const int cx) const
{
	const int n_channels = wave_format_.scan_count;
	const auto offset2 = 10 * n_channels;
	for (auto icx = cx; icx > 0; icx--, lp_source += n_channels, lp_dest++)
		*lp_dest = static_cast<short>(*(lp_source + offset2) - *(lp_source - offset2));
}

//**************************************************************************
//	"BLANCZO2 - Lanczos filter 2l (1/5T)  Ci=1,2",
//**************************************************************************
void CWaveBuf::low_pass_lanczo_2(const short* lp_source, short* lp_dest, const int cx) const
{
	const int n_channels = wave_format_.scan_count;
	const auto offset1 = n_channels;
	const auto offset2 = 2 * offset1;
	for (auto icx = cx; icx > 0; icx--, lp_source += n_channels, lp_dest++)
	{
		*lp_dest = static_cast<short>((*(lp_source + offset1) - *(lp_source - offset1))
			+ 2 * (*(lp_source + offset2) - *(lp_source - offset2)));
	}
}

//**************************************************************************
//	"BLANCZO3 - Lanczos filter 3l (1/14T) Ci=1,2,3",
//**************************************************************************
void CWaveBuf::low_pass_lanczo_3(const short* lp_source, short* lp_dest, const int cx) const
{
	const int n_channels = wave_format_.scan_count;
	const auto offset1 = n_channels;
	const auto offset2 = offset1 * 2;
	const auto offset3 = offset2 + offset1;
	for (auto icx = cx; icx > 0; icx--, lp_source += n_channels, lp_dest++)
	{
		*lp_dest = static_cast<short>((*(lp_source + offset1) - *(lp_source - offset1))
			+ (2 * (*(lp_source + offset2) - *(lp_source - offset2)))
			+ (3 * (*(lp_source + offset3) - *(lp_source - offset3))));
	}
}

//**************************************************************************
//	"BDERI1F3 - Derivative (1/3T)  1f3 Ci=1,1",
//**************************************************************************
void CWaveBuf::low_pass_derivative_1f3(const short* lp_source, short* lp_dest, const int cx) const
{
	const int n_channels = wave_format_.scan_count;
	const auto offset1 = n_channels;
	const auto offset2 = 2 * n_channels;
	for (auto icx = cx; icx > 0; icx--, lp_source += n_channels, lp_dest++)
	{
		*lp_dest = static_cast<short>((*(lp_source + offset1) - *(lp_source - offset1))
			+ (*(lp_source + offset2) - *(lp_source - offset2)));
	}
}

//**************************************************************************
//	"BDERI2F3 - Derivative (1/6T)  2f3 Ci=1,1,1",
//**************************************************************************
void CWaveBuf::low_pass_derivative_2f3(const short* lp_source, short* lp_dest, const int cx) const
{
	const int n_channels = wave_format_.scan_count;
	const auto offset1 = n_channels;
	const auto offset2 = n_channels * 2;
	const auto offset3 = n_channels * 3;
	for (auto icx = cx; icx > 0; icx--, lp_source += n_channels, lp_dest++)
	{
		*lp_dest = static_cast<short>((*(lp_source + offset1) - *(lp_source - offset1))
			+ (*(lp_source + offset2) - *(lp_source - offset2))
			+ (*(lp_source + offset3) - *(lp_source - offset3)));
	}
}

//**************************************************************************
//	"BDERI2F5 - Derivative (1/10T) 2f5 Ci=1,1,1,1",
//**************************************************************************
void CWaveBuf::low_pass_derivative_2f5(const short* lp_source, short* lp_dest, const int cx) const
{
	const int n_channels = wave_format_.scan_count;
	const auto offset1 = n_channels;
	const auto offset2 = n_channels * 2;
	const auto offset3 = n_channels * 3;
	const auto offset4 = n_channels * 4;
	for (auto icx = cx; icx > 0; icx--, lp_source += n_channels, lp_dest++)
	{
		*lp_dest = static_cast<short>((*(lp_source + offset1) - *(lp_source - offset1))
			+ (*(lp_source + offset2) - *(lp_source - offset2))
			+ (*(lp_source + offset3) - *(lp_source - offset3))
			+ (*(lp_source + offset4) - *(lp_source - offset4)));
	}
}

//**************************************************************************
//	"BDERI3F3 - Derivative (1/9T)  3f3 Ci=0,1,1,1",
//**************************************************************************
void CWaveBuf::low_pass_derivative_3f3(const short* lp_source, short* lp_dest, const int cx) const
{
	const int n_channels = wave_format_.scan_count;
	const auto offset2 = n_channels * 2;
	const auto offset3 = n_channels * 3;
	const auto offset4 = 2 * offset2;
	for (auto icx = cx; icx > 0; icx--, lp_source += n_channels, lp_dest++)
	{
		*lp_dest = (*(lp_source + offset2) - *(lp_source - offset2))
			+ (*(lp_source + offset3) - *(lp_source - offset3))
			+ (*(lp_source + offset4) - *(lp_source - offset4));
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

BOOL CWaveBuf::get_wb_volts_per_bin(const int ch_index, float* volts_per_bin, const int mode) const
{
	if ((ch_index >= wave_format_.scan_count) || (ch_index < 0))
		return false;

	auto correction = 1.0f;
	wb_get_correction_factor(mode, &correction);

	if (channels_array_.get_p_channel(ch_index)->am_resolutionV == 0)
	{
		if (channels_array_.get_p_channel(ch_index)->am_gaintotal == 0.)
		{
			channels_array_.get_p_channel(ch_index)->am_gaintotal = channels_array_.get_p_channel(ch_index)->am_amplifiergain
				* static_cast<float>(channels_array_.get_p_channel(ch_index)->am_gainpre)
				* static_cast<float>(channels_array_.get_p_channel(ch_index)->am_gainpost)
				* static_cast<float>(channels_array_.get_p_channel(ch_index)->am_gainheadstage)
				* static_cast<float>(channels_array_.get_p_channel(ch_index)->am_gainAD);
		}

		channels_array_.get_p_channel(ch_index)->am_resolutionV = wave_format_.full_scale_volts
			/ static_cast<float>(channels_array_.get_p_channel(ch_index)->am_gaintotal)
			/ static_cast<float>(wave_format_.bin_span);
	}
	*volts_per_bin = static_cast<float>(channels_array_.get_p_channel(ch_index)->am_resolutionV / correction);
	return true;
}

/**************************************************************************
 function:  BMovAvg30(short* lp_source, short* lp_dest, int cx)
 purpose:		compute moving average
 parameters:	input array + nb of points to compute
 returns:		output array with average
 comments:
 **************************************************************************/

void CWaveBuf::moving_average_30(short* lp_source, short* lp_dest, const int cx) const
{
	const int n_channels = wave_format_.scan_count;
	const auto offset_next_point = n_channels;
	auto n_span = m_transform_buffer_span_[MOVAVG30];
	const auto offset_span = offset_next_point * n_span;

	// init sum
	auto sum = 0;
	auto n = 0;
	auto lp = lp_source;
	lp -= offset_span;
	n_span += n_span;
	for (auto i = 0; i < n_span; i++)
	{
		sum += *lp;
		lp += offset_next_point;
		n++;
	}

	// moving average
	for (auto icx = cx; icx > 0; icx--, lp_source += offset_next_point, lp_dest++)
	{
		*lp_dest = static_cast<short>(sum / n);
		sum += (-*(lp_source - offset_span) + *(lp_source + offset_span));
	}
}

/**************************************************************************
 function:  BMedian30(short* lp_source, short* lp_dest, int cx)
 purpose:		compute moving median
 parameters:	input array + nb of points to compute, span=30 points
 returns:		output array with median value
 comments:
 source:	L. Fiore, G. Corsini, L. Geppetti (1996) Application of non-linear
			filters based on the median filter to experimental and simulated
			multiunit neural recordings. J Neurosci Meth 70: 177-184.
 **************************************************************************/
void CWaveBuf::moving_median_35(short* lp_source, short* lp_dest, const int cx)
{
	moving_median(lp_source, lp_dest, cx, 35);
}

void CWaveBuf::moving_median_30(short* lp_source, short* lp_dest, const int cx)
{
	moving_median(lp_source, lp_dest, cx, 30);
}

// Compute median of a curve
// lp_source = pointer to source data buffer (interleaved channels) [ii_time = l_first]
// lp_dest = pointer to destination data buffer (only one channel)
// nb_span = number of points to take into account on each side of each data point
// assume: m_waveFormat set
// use a temporary array to store data (dimension = nb_span *2 +1)
// assume: temporary array in common of wave_buf (to avoid creation each time this routine is called)

void CWaveBuf::moving_median(short* lp_source, short* lp_dest, int cx, const int n_span)
{
	// assume source buffer is with interleaved channels; if not: scan_count=1
	const int n_channels = wave_format_.scan_count;
	const auto lp_source_offset_next_point = n_channels; 
	const auto lp_source_offset_span = lp_source_offset_next_point * n_span; 

	if (p_array_size_ != n_span * 2 + 1) // adjust size of temporary array or create if not previously set
	{
		SAFE_DELETE_ARRAY(p_array_sorted_);
		SAFE_DELETE_ARRAY(p_array_circular_);
		p_array_size_ = n_span * 2 + 1;
		p_array_sorted_ = new short[p_array_size_];
		p_array_circular_ = new short[p_array_size_];
	}

	// DEBUG: bounds check
	ASSERT(cx <= get_wb_n_elements());
	const auto min_lp_source = p_w_data_;
	const auto max_lp_source = p_w_data_ + get_wb_n_channels() * get_wb_n_elements();

	auto lp = lp_source; // pointer to origin of source buffer
	lp -= lp_source_offset_span; // first point of the moving window
	int i;
	for (i = 0; i < p_array_size_; lp += lp_source_offset_next_point, i++)
	{
		*(p_array_sorted_ + i) = *lp; // load m_parraySorted with source data values
		*(p_array_circular_ + i) = *lp;
	}

	// sort m_parraySorted into ascending order using heapsort algorithm
	// cf Numerical recipes Press et al.1986, pp 231
	// "l"= index which will be decremented from initial value down to 0 during
	// 'hiring' (heap creation) phase. Once it reaches 0, the index "ir" will be
	// decremented from its initial value down to 0 during the 'retirement-and-
	// promotion' (heap selection) phase.

	auto l = n_span + 1; // temp index
	auto ir = p_array_size_ - 1; // temp index
	short val; // temp storage

	for (;;) // pseudo-loop over m_parraySorted
	{
		// -------------------------
		if (l > 0) // still in hiring phase
		{
			l--;
			val = *(p_array_sorted_ + l);
		}
		else // in retirement-and-promotion phase
		{
			val = *(p_array_sorted_ + ir); // clear a space at the end of the array
			*(p_array_sorted_ + ir) = *(p_array_sorted_); // retire the top of the heap into it
			ir--; // decrease the size of the corporation
			if (ir == 0) // done with the last promotion
			{
				*(p_array_sorted_) = val; // the least competent worker of all
				break; // exit the sorting algorithm
			}
		}
		// -------------------------
		i = l + 1; // wether we are in the hiring or promotion, we
		auto jj1 = i + i; // here set up to sift down element to its
		// proper level
		while (jj1 - 1 <= ir)
		{
			if (jj1 - 1 < ir)
			{
				if (*(p_array_sorted_ + jj1 - 1) < *(p_array_sorted_ + jj1))
					jj1++; // compare to the better underlining
			}
			if (val < *(p_array_sorted_ + jj1 - 1)) // demote value
			{
				*(p_array_sorted_ + i - 1) = *(p_array_sorted_ + jj1 - 1);
				i = jj1;
				jj1 = jj1 + jj1;
			} // this is value's level. Set j to terminate the
			else // sift-down
				jj1 = ir + 2;
		}
		*(p_array_sorted_ + i - 1) = val; // put value into its slot
	}
	// end of initial sort

	lp = lp_source; // first data point
	auto lp_next = lp_source + lp_source_offset_span; // last point
	auto i_p_array_circular = p_array_size_ - 1; // point on the last item so that first operation is blank

	for (auto icx = cx; icx > 0; icx--, lp += lp_source_offset_next_point, lp_next += lp_source_offset_next_point, lp_dest
	     ++)
	{
		const auto old_value = *(p_array_circular_ + i_p_array_circular); // old value
		const auto new_value = *lp_next; // new value to insert into array
		*(p_array_circular_ + i_p_array_circular) = new_value; // save new value into circular array

		// update circular array pointer
		i_p_array_circular++;
		if (i_p_array_circular >= p_array_size_)
			i_p_array_circular = 0;

		// locate position of old value to discard
		// use bisection - cf Numerical Recipes pp 90
		// on exit, j= index of oldvalue

		// binary search
		// Herbert Schildt: C the complete reference McGraw Hill, 1987, pp 488
		auto j_high = p_array_size_ - 1; // upper index
		auto j_low = 0; // mid point index
		auto jj2 = (j_low + j_high) / 2;
		while (j_low <= j_high)
		{
			jj2 = (j_low + j_high) / 2;
			if (old_value > *(p_array_sorted_ + jj2))
				j_low = jj2 + 1;
			else if (old_value < *(p_array_sorted_ + jj2))
				j_high = jj2 - 1;
			else
				break;
		}

		// insert new value in the correct position

		// case 1: search (and replace) towards higher values
		if (new_value > *(p_array_sorted_ + jj2))
		{
			auto j = jj2;
			for (auto k = jj2; new_value > *(p_array_sorted_ + k); k++, j++)
			{
				if (k == p_array_size_)
					break;
				*(p_array_sorted_ + j) = *(p_array_sorted_ + j + 1);
			}
			*(p_array_sorted_ + j - 1) = new_value;
		}

		// case 2: search (and replace) towards lower values
		else if (new_value < *(p_array_sorted_ + jj2))
		{
			auto j = jj2;
			for (auto k = jj2; new_value < *(p_array_sorted_ + k); k--, j--)
			{
				if (j == 0)
				{
					if (new_value < *p_array_sorted_)
						j--;
					break;
				}
				*(p_array_sorted_ + j) = *(p_array_sorted_ + j - 1);
			}
			*(p_array_sorted_ + j + 1) = new_value;
		}

		// case 3: already found!
		else
			*(p_array_sorted_ + jj2) = new_value;

		// save median value in the output array
		*lp_dest = *lp - *(p_array_sorted_ + n_span);
		ASSERT(lp >= min_lp_source);
		ASSERT(lp <= max_lp_source);
	}
}

/**************************************************************************

*/

void CWaveBuf::root_to_mean_square(short* lp_source, short* lp_dest, const int cx) const
{
	const int n_channels = wave_format_.scan_count;
	const auto offset_next_point = n_channels;
	auto nb_span = m_transform_buffer_span_[14] / 2;
	const auto offset_span = offset_next_point * nb_span;

	// init sum
	double sum2 = 0;
	auto n = 0;
	auto lp = lp_source;
	lp -= offset_span;
	nb_span += nb_span;
	for (auto i = 0; i < nb_span; i++)
	{
		const long x = *lp;
		sum2 += static_cast<double>(x) * x;
		lp += offset_next_point;
		n++;
	}

	// moving average
	for (auto icx = cx; icx > 0; icx--, lp_source += offset_next_point, lp_dest++)
	{
		*lp_dest = *lp_source - static_cast<short>(sqrt(sum2 / n));
		long x = *(lp_source - offset_span);
		sum2 -= static_cast<double>(x) * x;
		x = *(lp_source + offset_span);
		sum2 += static_cast<double>(x) * x;
	}
}
