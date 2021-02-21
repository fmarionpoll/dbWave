// TemplateWnd.cpp Implementation File
//
// Sorting algorithms designed according to :
// FOSTER Clemens, HANDWERKER Hermann O. (1990)
// Automatic classification and analysis of microneurographic spike data
// using a PC/AT.
// Journal of Neuroscience Methods 31 : 109-118.

#include "StdAfx.h"

//#include "resource.h"
#include "chart.h"
#include "TemplateWnd.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_SERIAL(CTemplateWnd, CChartWnd, 0 /* schema number*/)

CTemplateWnd& CTemplateWnd::operator = (const CTemplateWnd& arg)
{
	if (this != &arg) {
		m_nitems = arg.m_nitems;
		m_classID = arg.m_classID;
		m_csID = arg.m_csID;
		m_ktolerance = arg.m_ktolerance;
		m_globalstd = arg.m_globalstd;
		m_power = arg.m_power;
		SetTemplateLength(arg.m_tpllen);
		memcpy(m_pSumArray, arg.m_pSumArray, m_tpllen * 2 * sizeof(mytype));

		m_bValid = FALSE;
	}
	return *this;
}

void CTemplateWnd::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		const auto wversion = static_cast<WORD>(1);
		ar << wversion;

		const auto nstrings = static_cast<int>(1);
		ar << nstrings;
		ar << m_csID;		// 1

		const auto nints = static_cast<int>(3);
		ar << nints;
		ar << m_tpllen;		// 1
		ar << m_nitems;		// 2
		ar << m_classID;	// 3

		const auto nfloats = static_cast<int>(1);
		ar << nfloats;
		ar << m_ktolerance;	// 1

		const auto ndoubles = static_cast<int>(2);
		ar << ndoubles;
		ar << m_globalstd;	// 1
		ar << m_power;		// 2

		mytype* p_spike_element = m_pSumArray;
		for (auto i = 0; i < m_tpllen * 2; i++, p_spike_element++)
			ar << *p_spike_element;
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

		mytype* p_spike_element = m_pSumArray;
		for (auto i = 0; i < m_tpllen * 2; i++, p_spike_element++)
			ar >> *p_spike_element;
	}
}

BEGIN_MESSAGE_MAP(CTemplateWnd, CChartWnd)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()

CTemplateWnd::CTemplateWnd() : m_power(0), m_pMax0(nullptr), m_pMin0(nullptr)
{
	m_pSumArray = nullptr;
	m_pSUM0 = nullptr;
	m_pSUM20 = nullptr;
	m_nitems = 0;
	m_tpllen = 0;
	m_pAvg = nullptr;
	m_bValid = FALSE;
	m_ktolerance = 1.96f;
	m_globalstd = 0.f;
	m_bUseDIB = TRUE;
	m_classID = 0;
}

CTemplateWnd::~CTemplateWnd()
{
	DeleteArrays();
}

void CTemplateWnd::DeleteArrays()
{
	delete[] m_pSumArray;
	delete[] m_pAvg;

	m_pSumArray = nullptr;
	m_pSUM0 = nullptr;
	m_pSUM20 = nullptr;
	m_pAvg = nullptr;
}

void CTemplateWnd::PlotDatatoDC(CDC* p_dc)
{
	if (m_erasebkgnd)
		EraseBkgnd(p_dc);

	if (m_tpllen == 0)
		return;
	if (!m_bValid)
		tSetdisplayData();

	// load resources and prepare context
	const auto n_saved_dc = p_dc->SaveDC();
	p_dc->SetViewportOrg(m_displayRect.left, m_displayRect.Height() / 2);
	p_dc->SetViewportExt(m_displayRect.Width(), -m_displayRect.Height());

	GetExtents();
	PrepareDC(p_dc);

	if (m_ptsArea.GetSize() != m_tpllen * 2)
		InitPolypointAbcissa();

	// transfer to polypoint, adjust amplitude & plot
	FillOrdinatesAtscale(FALSE);

	// plot area between max and min in grey
	const auto old_pen = p_dc->SelectObject(&m_penTable[SILVER_COLOR]);
	const auto pold_b = (CBrush*)p_dc->SelectStockObject(LTGRAY_BRUSH);
	p_dc->Polygon(&m_ptsArea[0], m_tpllen * 2 + 1);
	p_dc->SelectObject(pold_b);

	// plot central curve
	p_dc->SelectStockObject(BLACK_PEN);
	p_dc->Polyline(&m_ptsAvg[0], m_tpllen);

	// restore resources
	p_dc->SelectObject(old_pen);
	p_dc->RestoreDC(n_saved_dc);
}

void CTemplateWnd::GetExtents()
{
	if (m_yWE == 1) // && m_yWO == 0)
	{
		auto p_sup = m_pMax0;
		auto p_inf = m_pMin0;
		auto maxval = *p_sup;
		auto minval = *p_inf;
		for (auto i = 0; i < m_tpllen; i++, p_sup++, p_inf++)
		{
			if (*p_sup > maxval) maxval = *p_sup;
			if (*p_inf < minval) minval = *p_inf;
		}

		m_yWE = maxval - minval + 2;
		m_yWO = (maxval + minval) / 2;
	}

	if (m_xWE == 1) // && m_xWO == 0)
	{
		m_xWE = m_tpllen;
		m_xWO = 0;
	}
}

void CTemplateWnd::InitPolypointAbcissa()
{
	m_ptsArea.SetSize(m_tpllen * 2 + 1);
	m_ptsAvg.SetSize(m_tpllen);
	auto i1 = 0;
	auto i2 = m_ptsArea.GetUpperBound() - 1;

	for (auto index = 0; index < m_tpllen; index++, i1++, i2--)
	{
		m_ptsAvg[i1].x = index;
		m_ptsArea[i1].x = index;
		m_ptsArea[i2].x = index;
	}
	m_ptsArea[m_ptsArea.GetUpperBound()].x = m_ptsArea[0].x;
}


void CTemplateWnd::FillOrdinatesAtscale(BOOL bScale)
{
	// fill with average data
	auto p_avg = m_pAvg;
	auto p_max = m_pMax0;
	auto p_min = m_pMin0;

	auto i1 = 0;
	auto i2 = m_ptsArea.GetUpperBound() -1;

	if (!bScale)
	{
		for (auto i = 0; i < m_tpllen; i++, i1++, i2--)
		{
			m_ptsAvg[i].y = *p_avg;
			m_ptsArea[i1].y = *p_max;
			m_ptsArea[i2].y = *p_min;
			p_avg++;
			p_max++;
			p_min++;
		}
	}
	else
	{
		for (int i = 0; i < m_tpllen; i++, i1++, i2--)
		{
			m_ptsAvg[i].y = MulDiv(*p_avg - m_yWO, m_yVE, m_yWE) + m_yVO;;
			m_ptsArea[i1].y = MulDiv(*p_max - m_yWO, m_yVE, m_yWE) + m_yVO;
			m_ptsArea[i2].y = MulDiv(*p_min - m_yWO, m_yVE, m_yWE) + m_yVO;
			p_avg++;
			p_max++;
			p_min++;

		}
	}
	m_ptsArea[m_ptsArea.GetUpperBound()].y = m_ptsArea[0].y;
}

void CTemplateWnd::SetTemplateLength(int len, int extent, int org)
{
	if (len == 0)
		len = m_tpllen;
	if (len != m_tpllen)
	{
		DeleteArrays();

		m_tpllen = len;	
		m_nitems = 0;

		m_pSumArray = new mytype[len * 2];
		m_pSUM0 = m_pSumArray;
		m_pSUM20 = m_pSumArray + len;

		m_pAvg = new int[len * 3];
		m_pMax0 = m_pAvg + m_tpllen;
		m_pMin0 = m_pAvg + 2 * m_tpllen;
		tInit();
	}

	if (org < 0)
		org = 0;
	if (extent < 0)
		extent = m_tpllen - org;
	m_xWE = extent;
	m_xWO = org;
}

void CTemplateWnd::tInit()
{
	memset(m_pSumArray, 0, m_tpllen * 2 * sizeof(mytype));
	memset(m_pAvg, 0, m_tpllen * 3 * sizeof(int));
	m_nitems = 0;
}

void CTemplateWnd::tAddSpikeTopSum(short* p_source)
{
	mytype* p_sum = m_pSUM0;
	mytype* p_sum2 = m_pSUM20;
	m_nitems++;

	for (auto i = 0; i < m_tpllen; i++, p_sum++, p_sum2++, p_source++)
	{
		const auto x = static_cast<mytype>(*p_source);
		*p_sum += x;
		*p_sum2 += (x * x);
	}
	// mark pMean data as not valid
	m_bValid = FALSE;
}

void CTemplateWnd::tSetdisplayData()
{
	auto p_mean = m_pAvg;
	auto p_sup = m_pMax0;
	auto p_inf = m_pMin0;

	mytype* p_sum = m_pSUM0;
	mytype* p_sum2 = m_pSUM20;
	mytype ysum, ysum2;
	const auto xn = static_cast<mytype>(m_nitems);

	if (m_nitems < 5)
	{
		ysum2 = static_cast<mytype>(m_globalstd * m_ktolerance);
		for (auto i = 0; i < m_tpllen; i++, p_sum++, p_mean++, p_sup++, p_inf++)
		{
			ysum = *p_sum / xn;
			*p_mean = static_cast<int>(ysum);
			*p_inf = static_cast<int>(ysum - ysum2);
			*p_sup = static_cast<int>(ysum + ysum2);
		}
	}
	else
	{
		for (auto i = 0; i < m_tpllen; i++, p_sum++, p_sum2++, p_mean++, p_sup++, p_inf++)
		{
			ysum = *p_sum;
			ysum2 = static_cast<mytype>(sqrt((*p_sum2 - ysum * ysum / xn) / (xn - 1.f)) * m_ktolerance);
			ysum /= xn;
			*p_mean = static_cast<int>(ysum);
			*p_inf = static_cast<int>(ysum - ysum2);
			*p_sup = static_cast<int>(ysum + ysum2);
		}
	}

	tPowerOfpSum();
	m_bValid = TRUE;
}

double CTemplateWnd::tPowerOfpSum()
{
	mytype* p_sum = m_pSUM0 + m_xWO;
	double xi;
	double x = 0.f;
	const auto last = m_xWE + m_xWO;

	for (auto i = m_xWO; i < last; i++, p_sum++)
	{
		xi = *p_sum;
		x += xi * xi;
	}
	xi = m_nitems;
	m_power = sqrt(x / m_xWE) / xi;
	return m_power;
}

// ---------------------------------------------------------------------------------
// tGetNumberOfPointsWithin()
//
//	get number of points within limits of template
//	template and current spike
//	if (n points >= tpl_len*hitrate/100) iyes = 1
//	if (n points <  tpl_len*hitrate/100) iyes = 0

BOOL CTemplateWnd::tGetNumberOfPointsWithin(short* p_source, int* hitrate)
{
	if (!m_bValid)				// we need valid limits..
		tSetdisplayData();

	int last = m_xWE + m_xWO;
	int* pMean = m_pAvg + m_xWO;
	int* pSup = m_pMax0 + m_xWO;
	int* pInf = m_pMin0 + m_xWO;
	short* p_data = p_source + m_xWO;
	int nwithin = 0;

	for (int i = m_xWO; i < last; i++, p_data++, pSup++, pInf++)
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

double CTemplateWnd::tDist(short* p_source)
{
	// assume power correctly set by calling routine tMinDist
//	if (!m_bValid)				// we need valid limits..
//		tSetdisplayData();		// and also correct power

	int last = m_xWE + m_xWO;
	int* pMean = m_pAvg + m_xWO;
	short* p_data = p_source + m_xWO;
	double ii = 0;

	for (int i = m_xWO; i < last; i++, p_data++, pMean++)
	{
		ii += abs(*pMean - *p_data);
	}
	double xdist = ii / m_power;
	return xdist;
}

// ---------------------------------------------------------------------------------
// tMinDist
//
//	compute minimal distance between template and current spike
//	jitter of -2/+2
//	out: xdist = sum(abs(t(i)-x(i+tpkd-1)), i=1, tpl_len)/tpower(it)

double CTemplateWnd::tMinDist(short* p_source, int* ioffsetmin, BOOL bJitter)
{
	if (!m_bValid)				// we need valid limits..
		tSetdisplayData();		// and also correct power

	int jitter = (bJitter ? 2 : 0);
	short* p_data = p_source - jitter;
	double xdist = tDist(p_data);
	double xmindist = xdist;
	*ioffsetmin = -jitter;
	p_data++;

	for (int i = -jitter + 1; i <= jitter; i++, p_data++)
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
	double ysum = 0;
	double ysum2 = 0;
	double xn = (double)m_nitems;

	//... compute global std and global distance
	int last = m_xWE + m_xWO;
	mytype* pSUM = m_pSUM0 + m_xWO;
	mytype* pSUM2 = m_pSUM20 + m_xWO;
	double ystd = 0;

	for (int i = m_xWO; i < last; i++, pSUM++, pSUM2++)
	{
		ysum += *pSUM;
		ysum2 += *pSUM2;
		ystd += sqrt((*pSUM2 - *pSUM * *pSUM / xn) / (xn - 1.));
	}

	double x = xn * (double)m_xWE;
	*gstd = sqrt((ysum2 - (ysum * ysum / x)) / (x - 1.));

	//.... compute global distance
	if (!m_bValid)
		tPowerOfpSum();
	*gdist = ystd * m_ktolerance / m_power;
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

BOOL CTemplateWnd::OnSetCursor(CWnd* p_wnd, UINT nHitTest, UINT message)
{
	return 0;
}