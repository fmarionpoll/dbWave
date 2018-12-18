// TemplateWnd.cpp Implementation File
//
// Sorting algorithms designed according to :
// FOSTER Clemens, HANDWERKER Hermann O. (1990)
// Automatic classification and analysis of microneurographic spike data 
// using a PC/AT.
// Journal of Neuroscience Methods 31 : 109-118.

#include "StdAfx.h"

#include "resource.h"
#include "scopescr.h"
#include "TemplateWnd.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_SERIAL(CTemplateWnd, CScopeScreen, 0 /* schema number*/ )

//////////////////////////////////////////////////////////////////////////////

void CTemplateWnd::operator = (const CTemplateWnd& arg)
{
	m_nitems = arg.m_nitems;
	m_classID = arg.m_classID;
	m_csID = arg.m_csID;
	m_ktolerance = arg.m_ktolerance;
	m_globalstd = arg.m_globalstd;
	m_power = arg.m_power;
	SetTemplateLength(arg.m_tpllen);
	memcpy(m_pSumArray, arg.m_pSumArray, m_tpllen*2*sizeof(mytype));

	m_bValid=FALSE;	
}

void CTemplateWnd::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{	
		WORD wversion = 1;
		ar << wversion;

		int nstrings = 1;
		ar << nstrings;
		ar << m_csID;		// 1

		int nints = 3;
		ar << nints;
		ar << m_tpllen;		// 1
		ar << m_nitems;		// 2
		ar << m_classID;	// 3

		int nfloats = 1;
		ar << nfloats;
		ar << m_ktolerance;	// 1

		int ndoubles = 2;
		ar << ndoubles;
		ar << m_globalstd;	// 1
		ar << m_power;		// 2
			
		mytype*	pS = m_pSumArray;
		for (int i=0; i< m_tpllen * 2; i++, pS++)
			ar << *pS;
	} 
	else
	{
		WORD version;  ar >> version;

		int nstrings;
		ar >> nstrings;
		ar >> m_csID;		// 1

		int nints;
		ar >> nints;
		int len; ar >> len;	
		SetTemplateLength(len);
		ar >> m_nitems;		// 2
		ar >> m_classID;	// 3

		int nfloats;
		ar >> nfloats;
		ar >> m_ktolerance;	// 1

		int ndoubles;
		ar >> ndoubles;
		ar >> m_globalstd;	// 1
		ar >> m_power;		// 2
	
		mytype*	pS = m_pSumArray;
		for (int i=0; i< m_tpllen * 2; i++, pS++)
			ar >> *pS;
	}
}

//////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CTemplateWnd, CScopeScreen)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////////

CTemplateWnd::CTemplateWnd(): m_power(0), m_pMax0(nullptr), m_pMin0(nullptr)
{
	m_pSumArray = nullptr;
	m_pSUM0 = nullptr; // array with sum (X)
	m_pSUM20 = nullptr; // array with sum (X * X)
	m_nitems = 0; // n elements
	m_tpllen = 0; // length of template array
	m_pAvg = nullptr;
	m_bValid = FALSE;
	m_ktolerance = 1.96f;
	m_globalstd = 0.f;
	m_bUseDIB = TRUE;
	m_classID = 0;
}

// ---------------------------------------------------------------------------------

CTemplateWnd::~CTemplateWnd()
{	
	DeleteArrays();
}

void CTemplateWnd::DeleteArrays()
{
	if (m_pSumArray != nullptr)
		delete [] m_pSumArray;
	if (m_pAvg != nullptr)
		delete [] m_pAvg;

	m_pSumArray = nullptr;
	m_pSUM0 = nullptr;
	m_pSUM20 = nullptr;
	m_pAvg = nullptr;
}

////////////////////////////////////////////////////////////////////////////////////
// DISPLAY SPIKES
//
// Display(CDC*)
////////////////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------------

void CTemplateWnd::PlotDatatoDC(CDC* pDC)
{
	if (m_erasebkgnd)		// erase background
		EraseBkgnd(pDC);

	if (m_tpllen == 0)
		return;
	if (!m_bValid)
		tSetdisplayData();

	// load resources and prepare context	
	int nSavedDC = pDC->SaveDC();
	pDC->SetViewportOrg (m_displayRect.left, m_displayRect.Height()/2);
	pDC->SetViewportExt (m_displayRect.Width(), -m_displayRect.Height());
	
	GetExtents();
	PrepareDC(pDC);

	if (m_polypts.GetSize() != m_tpllen*6)
		InitPolypointAbcissa();

	// transfer to polypoint, adjust amplitude & plot
	FillOrdinatesAtscale(FALSE);

	// plot area between max and min in grey
	CPen* oldPen =pDC->SelectObject(&m_penTable[SILVER_COLOR]);
	CBrush* poldB = (CBrush*) pDC->SelectStockObject(LTGRAY_BRUSH);
	pDC->Polygon((CPoint*) &m_polypts[2*m_tpllen], m_tpllen*2+1);	
	pDC->SelectObject(poldB);

	// plot central curve
	pDC->SelectStockObject(BLACK_PEN);	
	pDC->Polyline((CPoint*) &m_polypts[0], m_tpllen);

	// restore resources		
	pDC->SelectObject(oldPen);
	pDC->RestoreDC(nSavedDC);
}

// ---------------------------------------------------------------------------------

void CTemplateWnd::GetExtents()
{

	if (m_yWE == 1) // && m_yWO == 0)
	{		
		int* pSup = m_pMax0;
		int* pInf = m_pMin0;
		int maxval = *pSup;
		int minval = *pInf;
		for (int i=0; i< m_tpllen; i++, pSup++, pInf++)
		{
			if (*pSup > maxval) maxval = *pSup;
			if (*pInf < minval) minval = *pInf;
		}
		
		m_yWE = maxval - minval+2;  
		m_yWO = (maxval + minval)/2;
	}

	if (m_xWE == 1) // && m_xWO == 0)
	{
		m_xWE = m_tpllen;
		m_xWO = 0;
	}
}

// ------------------------------------------------------------------------------------
// set abcissa:
// first part of m_polypoints (m_tpllen points) = abcissa for the central line (average)
// second part (m_tpllen*2 + 1 points) = closed polygon with upper and lower limits

void CTemplateWnd::InitPolypointAbcissa()
{
	m_polypts.SetSize(m_tpllen*6+2);
	long* pDest = (long*) &m_polypts[0];
	long* pDest1 = (long*) &m_polypts[m_tpllen*2];
	long* pDest2 = (long*) &m_polypts[m_polypts.GetUpperBound() -3];

	for (int i = 1; i<=m_tpllen; i++)
	{
		*pDest = i;	// copy data
		pDest += 2;
		*pDest1 = i;
		pDest1 += 2;
		*pDest2 = i;
		pDest2 -= 2;		
	}
	m_polypts[m_polypts.GetUpperBound() -1] = m_polypts[m_tpllen*2];
}

// ------------------------------------------------------------------------------------
// set ordinates
// first part of m_polypoints (m_tpllen points) = ordinates for the central line (average)
// second part (m_tpllen*2 + 1 points) = closed polygon with upper and lower limits

void CTemplateWnd::FillOrdinatesAtscale(BOOL bScale)
{
		// fill with average data
	int* pAvg = m_pAvg;
	int* pMax = m_pMax0;
	int* pMin = m_pMin0;

	long* pDAvg = (long*) &m_polypts[1];
	long* pDMx = (long*) &m_polypts[m_tpllen*2 + 1];
	long* pDMi = (long*) &m_polypts[m_polypts.GetUpperBound()-2];

	if (!bScale)
	{
		for (int i = 0; i< m_tpllen; i++)
		{
			*pDAvg = *pAvg;
			pAvg++;
			pDAvg+= 2;

			*pDMx = *pMax;
			pMax++;
			pDMx+= 2;

			*pDMi = *pMin;
			pMin++;
			pDMi-= 2;
		}
	}
	else
	{
		for (int i = 0; i< m_tpllen; i++)
		{
			*pDAvg = MulDiv(*pAvg -m_yWO, m_yVE, m_yWE) + m_yVO;
			pAvg++;
			pDAvg+= 2;

			*pDMx = MulDiv(*pMax -m_yWO, m_yVE, m_yWE) + m_yVO;
			pMax++;
			pDMx+= 2;

			*pDMi = MulDiv(*pMin -m_yWO, m_yVE, m_yWE) + m_yVO;
			pMin++;
			pDMi-= 2;
		}
	}
	m_polypts[m_polypts.GetUpperBound()] = m_polypts[m_tpllen*2+1];
}


// ------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------

void CTemplateWnd::SetTemplateLength(int len, int extent, int org)
{
	if (len == 0)
		len = m_tpllen;
	if (len != m_tpllen)
	{
		DeleteArrays();

		m_tpllen=len;		// length of template array
		m_nitems=0;			// n elements

		m_pSumArray = new mytype[len*2];
		m_pSUM0	=	m_pSumArray;
		m_pSUM20=	m_pSumArray + len;

		m_pAvg	=	new int [len*3];
		m_pMax0 = m_pAvg + m_tpllen;
		m_pMin0 = m_pAvg + 2*m_tpllen;
		tInit();
	}

	if (org < 0)
		org = 0;
	if( extent < 0)
		extent = m_tpllen-org;
	m_xWE = extent;
	m_xWO = org;
}

// ---------------------------------------------------------------------------------

// Data operations on template

// ---------------------------------------------------------------------------------
// tInit()
//
// erase all data, set data to zero and restore nb of elements to zero

void CTemplateWnd::tInit()
{
	memset (m_pSumArray, 0, m_tpllen*2*sizeof(mytype));
	memset (m_pAvg, 0, m_tpllen*3*sizeof(int));
	m_nitems=0;			// n elements
}

// ---------------------------------------------------------------------------------
// tAdd
//
// add spike data to template
// set flag to indicate that mean and limits are no longer valid

void CTemplateWnd::tAdd(short* pSource)
{
	mytype* pSUM = m_pSUM0;
	mytype* pSUM2 = m_pSUM20;	
	m_nitems++;

	for (int i=0; i< m_tpllen; i++, pSUM++, pSUM2++, pSource++)
	{
		mytype x = (mytype) *pSource;
		*pSUM += x;
		*pSUM2 += (x*x);
	}
	// mark pMean data as not valid
	m_bValid=FALSE;
}

// ---------------------------------------------------------------------------------
// tSetdisplayData()
//
// compute center curve and upper and lower limits
// set flag when done

void CTemplateWnd::tSetdisplayData()
{
	int* pMean = m_pAvg;
	int* pSup = m_pMax0;
	int* pInf = m_pMin0;

	mytype* pSUM = m_pSUM0;
	mytype* pSUM2 = m_pSUM20;
	mytype ysum, ysum2;
	mytype xn = (mytype) m_nitems;

	if (m_nitems < 5)
	{
		ysum2 = (mytype) (m_globalstd * m_ktolerance);
		for (int i=0; i< m_tpllen; i++, pSUM++, pMean++, pSup++, pInf++)
		{
			ysum = *pSUM / xn;
			*pMean = (int) ysum;
			*pInf = (int) (ysum - ysum2);
			*pSup = (int) (ysum + ysum2);
		}

	}
	else
	{
		for (int i=0; i< m_tpllen; i++, pSUM++, pSUM2++, pMean++, pSup++, pInf++)
		{
			ysum = *pSUM;
			ysum2 = (mytype)( sqrt((*pSUM2 - ysum*ysum/xn)/(xn-1.f)) * m_ktolerance);
			ysum /= xn;			
			*pMean = (int) ysum;
			*pInf = (int) (ysum - ysum2);
			*pSup = (int) (ysum + ysum2);
		}
	}

	// update power
	tPower();
	m_bValid=TRUE;
}

// ---------------------------------------------------------------------------------
// tPower()
//
// compute power of this template

double CTemplateWnd::tPower()
{
	mytype* pSUM = m_pSUM0 + m_xWO;
	double xi;
	double x = 0.f;
	int last = m_xWE + m_xWO;

	for (int i=m_xWO; i< last; i++, pSUM++)
	{
		xi = *pSUM;
		x += xi*xi;
	}
	xi= m_nitems;
	m_power = sqrt ( x/m_xWE) / xi;
	return m_power;
}

// ---------------------------------------------------------------------------------
// tWithin()
//
//	get number of points within limits of template
//	template and current spike
//	if (n points >= tpl_len*hitrate/100) iyes = 1
//	if (n points <  tpl_len*hitrate/100) iyes = 0

BOOL CTemplateWnd::tWithin(short* pSource, int* hitrate)
{
	if (!m_bValid)				// we need valid limits..
		tSetdisplayData();

	int last = m_xWE + m_xWO;
	int* pMean = m_pAvg + m_xWO;
	int* pSup = m_pMax0 + m_xWO;
	int* pInf = m_pMin0 + m_xWO;
	short* p_data = pSource + m_xWO;
	int nwithin = 0;

	for (int i=m_xWO; i< last; i++, p_data++, pSup++, pInf++)
	{
		if (*p_data <= *pSup && *p_data >= *pInf)
			nwithin++;
	}

	int ihitrate = MulDiv(*hitrate, m_xWE, 100);
	return (nwithin >= ihitrate);
}

// ---------------------------------------------------------------------------------
// tDist
//
//	compute distance between template and current spike
//	out: xdist = sum(abs(t(i)-x(i+tpkd-1)), i=1, tpl_len)/tpower(it)

double CTemplateWnd::tDist(short* pSource)
{
	// assume power correctly set by calling routine tMinDist
//	if (!m_bValid)				// we need valid limits..
//		tSetdisplayData();		// and also correct power

	int last = m_xWE + m_xWO;
	int* pMean = m_pAvg + m_xWO;
	short* p_data = pSource + m_xWO;
	double ii=0;

	for (int i=m_xWO; i< last; i++, p_data++, pMean++)	
	{
		ii += abs(*pMean - *p_data);
	}
	double xdist = ii/m_power;
	return xdist;
}

// ---------------------------------------------------------------------------------
// tMinDist
//
//	compute minimal distance between template and current spike
//	jitter of -2/+2
//	out: xdist = sum(abs(t(i)-x(i+tpkd-1)), i=1, tpl_len)/tpower(it) 

double CTemplateWnd::tMinDist(short* pSource, int* ioffsetmin, BOOL bJitter)
{
	if (!m_bValid)				// we need valid limits..
		tSetdisplayData();		// and also correct power

	int jitter= (bJitter ? 2 : 0);
	short* p_data = pSource-jitter;	
	double xdist = tDist(p_data);
	double xmindist = xdist;
	*ioffsetmin = -jitter;
	p_data++;	

	for (int i=-jitter+1; i<= jitter ; i++, p_data++)
	{
		xdist = tDist(p_data);
		if (xdist < xmindist)
		{
			xmindist = xdist;
			*ioffsetmin = i;
		}
	}

	return xmindist;
}

// ---------------------------------------------------------------------------------
// tGlobalstats
//
//	compute globalstd, global distance

void CTemplateWnd::tGlobalstats(double* gstd, double* gdist)
{
	double ysum  =0;
	double ysum2 =0;
	double xn = (double) m_nitems;

	//... compute global std and global distance
	int last = m_xWE + m_xWO;
	mytype* pSUM = m_pSUM0 + m_xWO;
	mytype* pSUM2 = m_pSUM20 + m_xWO;
	double ystd = 0;
	
	for (int i=m_xWO; i< last; i++, pSUM++, pSUM2++)
	{
		ysum += *pSUM;
		ysum2 += *pSUM2;
		ystd += sqrt( (*pSUM2 - *pSUM * *pSUM/xn)/(xn-1.) );
	}

	double x = xn * (double) m_xWE;
	*gstd = sqrt((ysum2 - (ysum*ysum/x))/(x-1.));	

	//.... compute global distance
	if (!m_bValid)
		tPower();
	*gdist = ystd * m_ktolerance /m_power;
}

// ---------------------------------------------------------------------------------
// trap messages and send them to parent
void CTemplateWnd::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	MapWindowPoints(GetParent(), &point, 1);
	GetParent()->PostMessage(WM_LBUTTONDBLCLK, nFlags, MAKELPARAM(point.x, point.y));
}

void CTemplateWnd::OnLButtonDown(UINT nFlags, CPoint point) 
{
	MapWindowPoints(GetParent(), &point, 1);
	GetParent()->PostMessage(WM_LBUTTONDOWN, nFlags, MAKELPARAM(point.x, point.y));
}

void CTemplateWnd::OnLButtonUp(UINT nFlags, CPoint point) 
{
	MapWindowPoints(GetParent(), &point, 1);
	GetParent()->PostMessage(WM_LBUTTONUP, nFlags, MAKELPARAM(point.x, point.y));
}

void CTemplateWnd::OnMouseMove(UINT nFlags, CPoint point) 
{
	MapWindowPoints(GetParent(), &point, 1);
	GetParent()->PostMessage(WM_MOUSEMOVE, nFlags, MAKELPARAM(point.x, point.y));
}

void CTemplateWnd::OnRButtonDown(UINT nFlags, CPoint point) 
{
	MapWindowPoints(GetParent(), &point, 1);
	GetParent()->PostMessage(WM_RBUTTONDOWN, nFlags, MAKELPARAM(point.x, point.y));
}

void CTemplateWnd::OnRButtonUp(UINT nFlags, CPoint point) 
{
	MapWindowPoints(GetParent(), &point, 1);
	GetParent()->PostMessage(WM_RBUTTONUP, nFlags, MAKELPARAM(point.x, point.y));
}

BOOL CTemplateWnd::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	return 0;
}
