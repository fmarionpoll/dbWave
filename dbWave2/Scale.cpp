#include "StdAfx.h"
#include "Scale.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_SERIAL(Scale, CObject, 1 /* schema number*/)

// create CScale with zero points
Scale::Scale()
{
}

// create CScale with n points
Scale::Scale(const int n_pixels) : m_n_pixels_(n_pixels)
{
	m_intervals_.SetSize(n_pixels);
	m_position_.SetSize(n_pixels + 1);
}

//---------------------------------------------------------------------------
//
// FUNCTION fscale (originally developped in Fortran)
// (this function should belong to m_desc)
//
//	compute a series of indexes (l_first to l_last; i*4) to display data
//	in a smaller scale (1 to m_nintervals)
//	store scale intervals into m_intervals CWordarray
//
//	ASSUME     l_last-l_first >= m_nintervals; l_last>l_first
//
//	RETURN ERROR (FALSE) if l_last, l_first <0, mem alloc,
//  RETURN nb data pt/interval
//
//	use modified run-length slice line drawing algorithm as described by:
//	Michael ABRASH (1992) The Good, the Bad and the Run-sliced
//	Dr Dobb's Journal 194: 171-191
// 7 dec 96:
// this algorithm is a bit faster than direct computation with floats, especially
// when dealing with large number of pixels - using QueryPerformanceCounter(), I have
// found the following:
//	time_run-length_loop	time_floating_loop	npixels	lxSize
//	64	 (ticks)			35	(ticks)			10		1024
//	752						1097				525		26680
//	1572					2314				1160	26680
//
//	set scale: CViewData variables
// 	m_lxSize		-- intput scale span
// 	m_nintervals	-- output scale span
//	m_intervals		-- CWordArray scale
//	m_position		-- CDWord array with end limits of each interval
//  (assume start at zero)
// --------------------------------------------------------------------------

int Scale::set_scale(const int n_pixels, const long n_data_points)
{
	ASSERT(n_data_points != 0);
	m_n_pixels_ = n_pixels;

	// how many intervals in the scale?
	m_l_n_data_points_ = n_data_points;
	if (n_data_points >= static_cast<long>(n_pixels))
		m_n_intervals_ = n_pixels;
	else
		m_n_intervals_ = static_cast<int>(n_data_points);

	// adjust size of the array
	m_intervals_.SetSize(m_n_intervals_);
	m_position_.SetSize(m_n_intervals_ + 1);

	int i;
	long l_position = -1; // store last intervals

	//     ---------------- UNITY INTERVALS -------------------------------------
	if (n_data_points < static_cast<long>(n_pixels)) // dest scale < source
	{
		for (i = 0; i < m_n_intervals_; i++)
		{
			m_intervals_[i] = 1;
			l_position++;
			m_position_[i] = l_position;
		}
		ASSERT(l_position == n_data_points - 1);
	}

	//     ---------------- MORE THAN 1 DATA PT PER "PIXEL" ---------------------
	else
	{
		//		type of scale?
		const auto l_x_delta = n_data_points; // horizontal move (source interval)
		const auto i_y_delta = n_pixels; // vertical move (destination)
		const auto l_temp = l_x_delta / static_cast<long>(i_y_delta); // minimum of steps in an Y interval
		const auto i_whole_step = static_cast<int>(l_temp);

		//     ---------------- EQUAL INTERVALS -------------------------------------

		if (l_x_delta * l_temp == i_y_delta)
		{
			for (i = 0; i < i_y_delta; i++) // loop (YDelta+1?)
			{
				m_intervals_[i] = i_whole_step; // fills array with intervals
				l_position += i_whole_step;
				m_position_[i] = l_position; // absolute
			}
			ASSERT(l_position == n_data_points - 1);
		}

		//     ---------------- UNEQUAL INTERVALS -----------------------------------
		else
		{
			const auto adj_up = ldiv(l_x_delta, i_y_delta);
			const auto i_adj_up = static_cast<int>(adj_up.rem) * 2;
			const auto i_adj_down = i_y_delta * 2;
			auto i_error_term = 0;
			for (i = 0; i < i_y_delta; i++)
			{
				int i_run_length = i_whole_step; // run at least this long
				i_error_term += i_adj_up; // update error term
				if (i_error_term > 0) // add extra step if crosses bound
				{
					i_run_length++; // increase step
					i_error_term -= i_adj_down; // reset error term
				}
				m_intervals_[i] = i_run_length; // store step
				l_position += i_run_length;
				m_position_[i] = l_position;
			}
			ASSERT(l_position == n_data_points - 1);
		}
	}

	ASSERT(m_n_intervals_ > 0);
	if (m_position_[m_n_intervals_ - 1] > static_cast<DWORD>(n_data_points))
	{
		const auto interval = m_n_intervals_ - 1;
		m_intervals_[interval] = m_intervals_[interval] - static_cast<int>(m_position_[interval] - static_cast<DWORD>(
			n_data_points));
		m_position_[interval] = static_cast<DWORD>(n_data_points);
	}

	m_position_[m_n_intervals_] = m_position_[m_n_intervals_ - 1];
	return TRUE;
}

/**************************************************************************
 function:	HowManyIntervalsFit(int firstPixel, long l_first, long l_last)
 purpose:	count how many pixels fit in a RW buffer / given scale
			m_intervals[] = nb pts / interval
			m_position[]  = scale series; start from zero

 parameters:	firstPixel	index first pixel
				l_last		index last point that might be displayed
							(assuming that start of the data is zero)

 returns:		number of pixels fitting in interval
				l_last 		index last pt of the intervals
 comments:  note that between kd and kf (array index),
			there are kd+kf+1 intervals
 history:
	the initial version used only one array with nb of data points in each
	interval. The algorithm used to find how many intervals fitted in one
	data buffer is left in commentary at the end of the function. But the
	search involved adding each time the routine was called the sum of all
	intervals from start to the end searched.

	the second version of this routine involves an additional array "m_position"
	in which these intervals are stored.
	The cost of this approach is memory (one more array of DWORD).
	The benefit is simpler code and faster computation. It also allows a reduction
	of the number of parameters passed (l_first). However, the calling procedure
	should care to pass l_last relative to zero (and not the real one). See
	implementation and use of this in CLineViewWnd::GetDataForDisplay().
 **************************************************************************/

int Scale::how_many_intervals_fit(const int first_pixel, long* l_last)
{
	// assume that l_first equal m_position[firstPixel-1]
	const DWORD last_pos = *l_last; // end within m_position
	auto last_pixel = static_cast<int>(last_pos / m_intervals_[first_pixel]); // guess
	if (last_pixel >= m_n_intervals_) // clip this guess
		last_pixel = m_n_intervals_ - 1; // to the max size of Scale array

	// 2 cases: CASE 1 = go backwards (estimation was too much)
	// stop when lastPixel = 0
	// or the first time that m_position[lastPixel-1] less than l_last
	if (m_position_[last_pixel] > last_pos)
	{
		while (last_pixel > 0 && (m_position_[last_pixel] > last_pos))
		{
			last_pixel--;
		}
	}
	// 2 cases: CASE 2 = go forwards (estimation was too small)
	else if (last_pos > m_position_[last_pixel + 1])
	{
		while (last_pixel <= m_n_intervals_ - 1 && (last_pos > m_position_[last_pixel + 1]))
		{
			last_pixel++;
		}
	}

	*l_last = static_cast<long>(m_position_[last_pixel]);
	return (last_pixel - first_pixel + 1);
}

int Scale::get_which_interval(const long l_index)
{
	const DWORD ll_index = l_index;
	if (l_index < 0 || ll_index > m_position_[m_position_.GetUpperBound()])
		return -1;

	int interval; // init value for error (interval not found)
	const auto i_guess = static_cast<int>(l_index / m_intervals_[0]); // first guess
	// l_index less than interval guessed
	if (ll_index < m_position_[i_guess]) // index is less
	{
		int i;
		for (i = i_guess; i >= 0; i--) // scan position backwards
		{
			if (ll_index > m_position_[i]) // exit loop if index is greater
				break; // than position(i)
		}
		interval = i + 1; // set position to i
	}
	// l_index greater or equal that interval guessed
	else
	{
		// index is higher
		int i;
		for (i = i_guess; i <= m_position_.GetUpperBound(); i++)
		{
			// scan forward, exit if posit(i+1) is higher
			if (ll_index <= m_position_[i]) // (but posit(i) is lower)
				break;
		}
		interval = i; // set final position to i
	}
	return interval;
}

void Scale::Serialize(CArchive& ar)
{
	m_intervals_.Serialize(ar); // scale array: n_pts within each interval
	m_position_.Serialize(ar); // interval array: consecutive file index (long)
	int i_dummy = 0;

	if (ar.IsStoring())
	{
		ar << m_n_pixels_; // scale built for n pixels
		ar << m_n_intervals_; // nb of elements within Scale
		ar << i_dummy;
		ar << m_l_n_data_points_; // N_data_points are mapped to m_n_intervals
	}
	else
	{
		ar >> m_n_pixels_; // scale built for n pixels
		ar >> m_n_intervals_; // nb of elements within Scale
		ar >> i_dummy;
		ar >> m_l_n_data_points_; // N_data_points are mapped to m_n_intervals
	}
}
