/////////////////////////////////////////////////////////////////////////////
// CScale methods

#include "StdAfx.h"
#include "Cscale.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_SERIAL(CScale, CObject, 1 /* schema number*/ )

// --------------------------------------------------------------------------
// create CScale with zero points
// --------------------------------------------------------------------------
CScale::CScale()
{
	m_nintervals = 0;
	m_npixels = 1;
}


// --------------------------------------------------------------------------
// create CScale with npoints
// --------------------------------------------------------------------------
CScale::CScale(int npixels)
{
	m_npixels = 1;
	m_intervals.SetSize(npixels);
	m_position.SetSize(npixels+1);
	m_nintervals = 0;
}

//---------------------------------------------------------------------------
//
// FUNCTION fscale (originally developped in Fortran)
// (this function should belong to m_desc)
//
//	compute a series of indexes (lFirst to lLast; i*4) to display data
//	in a smaller scale (1 to m_nintervals)
//	store scale intervals into m_intervals CWordarray
//
//	ASSUME     lLast-lFirst >= m_nintervals; lLast>lFirst
//
//	RETURN ERROR (FALSE) if lLast, lFirst <0, mem alloc,
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

int CScale::SetScale(int npixels, long lxSize)
{
	ASSERT (lxSize != 0);
	m_npixels = npixels;

	// how many invervals in the scale?
	m_lNdatapoints = lxSize;
	if (lxSize >= (long) npixels) 
		m_nintervals = npixels;
	else
		m_nintervals = (int) lxSize;

	// adjust size of the array	
	m_intervals.SetSize(m_nintervals);
	m_position.SetSize(m_nintervals+1);

	int i;
	long lpos=-1; // store last intervals

//     ---------------- UNITY INTERVALS -------------------------------------
	if (lxSize < (long) npixels)		// dest scale < source
	{
		for (i=0; i<m_nintervals; i++)
		{
			m_intervals[i] = 1;
			lpos ++;
			m_position[i]= lpos;
		}
		ASSERT(lpos == lxSize-1);
	}

//     ---------------- MORE THAN 1 DATA PT PER "PIXEL" ---------------------
	else
	{
//		type of scale?
		long lXDelta = lxSize; 		// horizontal move (source interval)
		int iYDelta = npixels;     	// vertical move (destination)	
		int iWholeStep;
		long lTemp = lXDelta / (long) iYDelta;  // minimum of steps in an Y interval
		iWholeStep = (int) lTemp;

//     ---------------- EQUAL INTERVALS -------------------------------------	

		if (lXDelta*lTemp == iYDelta)
		{
			for (i=0; i< iYDelta; i++)	// loop (YDelta+1?)
			{
				m_intervals[i] = iWholeStep;// fills array with intervals
				lpos += iWholeStep;
				m_position[i] = lpos;		// absolute					
			}
			ASSERT(lpos == lxSize-1);
		}

//     ---------------- UNEQUAL INTERVALS -----------------------------------
		else
		{
			ldiv_t AdjUp = ldiv (lXDelta, iYDelta);
			int iAdjUp = (int) AdjUp.rem * 2;
			int iAdjDown = iYDelta * 2;
			int	iErrorTerm = 0;
			int	iRunLength;        
			for (i=0; i< iYDelta; i++)
			{
				iRunLength = iWholeStep;// run at least this long
				iErrorTerm += iAdjUp;	// update error term
				if (iErrorTerm > 0)		// add extra step if crosses bound
				{
					iRunLength++;		// increase step
					iErrorTerm -= iAdjDown;// reset error term
				}
				m_intervals[i] = iRunLength;// store step
				lpos += iRunLength;
				m_position[i]= lpos;
			}
			ASSERT(lpos == lxSize-1);
		}
	}

	ASSERT (m_nintervals > 0);	
	if (m_position[m_nintervals-1] > (DWORD) lxSize)
	{
		int i = m_nintervals-1;
		m_intervals[i] = m_intervals[i] - (int) (m_position[i] - (DWORD) lxSize);
		m_position[i] = (DWORD) lxSize;
	}

	m_position[m_nintervals]= m_position[m_nintervals-1];
	return TRUE;
}


/**************************************************************************
 function:	HowManyIntervalsFit(int firstPixel, long lFirst, long lLast)
 purpose:	count how many pixels fit in a RW buffer / given scale
			m_intervals[] = nb pts / interval
			m_position[]  = scale series; start from zero

 parameters:	firstPixel	index first pixel
				lLast		index last point that might be displayed
							(assuming that start of the data is zero)

 returns:		number of pixels fitting in interval
				lLast 		index last pt of the intervals
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
	The cost of this approach is memory (one more array of DWORDs). 
	The benefit is simpler code and faster computation. It also allows a reduction
	of the number of parameters passed (lFirst). However, the calling procedure
	should care to pass lLast relative to zero (and not the real one). See 
	implementation and use of this in CLineViewWnd::GetDataForDisplay().
 **************************************************************************/

int CScale::HowManyIntervalsFit(int firstPixel, long* lLast)
{
	int npixels = 0;	
	
	// assume that lFirst equal m_position[firstPixel-1]
	DWORD lastpos = *lLast;						// end within m_position
	int lastPixel = (int) (lastpos / m_intervals[firstPixel]); // guess
	if (lastPixel >= m_nintervals)				// clip this guess
		lastPixel = m_nintervals-1;				// to the max size of Scale array

	int k1 = firstPixel-1;
	if (k1 < 0) k1 = 0; 
	
	// 2 cases: CASE 1 = go backwards (estimation was too much)
	// stop when lastPixel = 0
	// or the first time that m_position[lastPixel-1] less than lLast
	if (m_position[lastPixel] > lastpos)
	{
		while (lastPixel > 0 && (m_position[lastPixel] > lastpos))
			{ lastPixel--;} 
	}
	// 2 cases: CASE 2 = go forwards (estimation was too small)
	else if (lastpos > m_position[lastPixel+1])
	{
		while (lastPixel <= m_nintervals-1 && (lastpos > m_position[lastPixel+1]))
			{ lastPixel++; }
	}
	
	*lLast = m_position[lastPixel];
	return (lastPixel-firstPixel+1);
}


// --------------------------------------------------------------------------------------
int CScale::GetWhichInterval(long lindex)
{
	DWORD llindex = lindex;
	if (lindex < 0 || llindex > m_position[m_position.GetUpperBound()])
		return -1;

	int interval=-1;	// init value for error (interval not found)
	int iguess = (int) (lindex/m_intervals[0]); // first guess
	// lindex less than interval guessed    
	if (llindex < m_position[iguess])			// index is less
	{
		int i=iguess;
		for (i=iguess; i>= 0; i--)			// scan position backwards
		{
			if (llindex > m_position[i])		// exit loop if index is greater
				break;							// than position(i)    			
		}
		interval = i+1;							// set position to i
	}
	// lindex greater or equal that interval guessed
	else
	{										// index is higher
		int i=iguess;
		for (i=iguess; i<= m_position.GetUpperBound(); i++)
		{									// scan forward, exit if posit(i+1) is higher
			if (llindex <= m_position[i])		// (but posit(i) is lower)
				break;    			
		}
		interval = i;							// set final position to i
	}
	return interval;
}

void CScale::Serialize(CArchive& ar)
{
	m_intervals.Serialize(ar);	// scale array: npts within each interval
	m_position.Serialize(ar);	// interval array: consecutive file index (long)
	int idummy=0;

	if (ar.IsStoring())
	{
		ar << m_npixels;		// scale built for n pixels	
		ar << m_nintervals;		// nb of elements within Scale
		ar << idummy;
		ar << m_lNdatapoints;	// Ndatapoints are mapped to m_nintervals
	}
	else
	{
		ar >> m_npixels;		// scale built for n pixels	
		ar >> m_nintervals;		// nb of elements within Scale
		ar >> idummy;
		ar >> m_lNdatapoints;	// Ndatapoints are mapped to m_nintervals
	}
}

	
