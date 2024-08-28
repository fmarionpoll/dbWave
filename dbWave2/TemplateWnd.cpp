// TemplateWnd.cpp Implementation File
//
// Sorting algorithms designed according to :
// FOSTER Clemens, HANDWERKER Hermann O. (1990)
// Automatic classification and analysis of micro-neurographic spike data
// using a PC/AT.
// Journal of Neuroscience Methods 31 : 109-118.

#include "StdAfx.h"

//#include "resource.h"
#include "ChartWnd.h"
#include "TemplateWnd.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_SERIAL(CTemplateWnd, ChartWnd, 0 /* schema number*/)

CTemplateWnd& CTemplateWnd::operator =(const CTemplateWnd& arg)
{
	if (this != &arg)
	{
		m_n_items_ = arg.m_n_items_;
		m_class_id = arg.m_class_id;
		m_cs_id = arg.m_cs_id;
		m_k_tolerance = arg.m_k_tolerance;
		m_global_std = arg.m_global_std;
		m_power = arg.m_power;
		set_template_length(arg.m_tpl_len_);
		memcpy(m_p_sum_array_, arg.m_p_sum_array_, m_tpl_len_ * 2 * sizeof(mytype));

		m_b_valid_ = FALSE;
	}
	return *this;
}

void CTemplateWnd::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		constexpr auto w_version = static_cast<WORD>(1);
		ar << w_version;

		constexpr auto n_string = 1;
		ar << n_string;
		ar << m_cs_id; // 1

		constexpr auto n_int = 3;
		ar << n_int;
		ar << m_tpl_len_; // 1
		ar << m_n_items_; // 2
		ar << m_class_id; // 3

		constexpr auto n_float = 1;
		ar << n_float;
		ar << m_k_tolerance; // 1

		constexpr auto n_double = 2;
		ar << n_double;
		ar << m_global_std; // 1
		ar << m_power; // 2

		mytype* p_spike_element = m_p_sum_array_;
		for (auto i = 0; i < m_tpl_len_ * 2; i++, p_spike_element++)
			ar << *p_spike_element;
	}
	else
	{
		WORD version;
		ar >> version;

		int n_string;
		ar >> n_string;
		ar >> m_cs_id; // 1

		int n_int;
		ar >> n_int;
		int len;
		ar >> len;
		set_template_length(len);
		ar >> m_n_items_; // 2
		ar >> m_class_id; // 3

		int n_float;
		ar >> n_float;
		ar >> m_k_tolerance; // 1

		int n_double;
		ar >> n_double;
		ar >> m_global_std; // 1
		ar >> m_power; // 2

		mytype* p_spike_element = m_p_sum_array_;
		for (auto i = 0; i < m_tpl_len_ * 2; i++, p_spike_element++)
			ar >> *p_spike_element;
	}
}

BEGIN_MESSAGE_MAP(CTemplateWnd, ChartWnd)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()

CTemplateWnd::CTemplateWnd()
{
	b_use_dib_ = TRUE;
}

CTemplateWnd::~CTemplateWnd()
{
	delete_arrays();
}

void CTemplateWnd::delete_arrays()
{
	delete[] m_p_sum_array_;
	delete[] m_p_avg_;

	m_p_sum_array_ = nullptr;
	m_p_sum0_ = nullptr;
	m_p_sum20_ = nullptr;
	m_p_avg_ = nullptr;
}

void CTemplateWnd::plot_data_to_dc(CDC* p_dc)
{
	if (b_erase_background_)
		erase_background(p_dc);

	if (m_tpl_len_ == 0)
		return;
	if (!m_b_valid_)
		t_set_display_data();

	// load resources and prepare context
	const auto n_saved_dc = p_dc->SaveDC();
	p_dc->SetViewportOrg(display_rect_.left, display_rect_.Height() / 2);
	p_dc->SetViewportExt(display_rect_.Width(), -display_rect_.Height());

	get_extents();
	p_dc->SetMapMode(MM_ANISOTROPIC);
	prepare_dc(p_dc);

	if (m_pts_area_.GetSize() != m_tpl_len_ * 2)
		init_polypoint_abscissa();

	// transfer to polypoint, adjust amplitude & plot
	fill_ordinates_at_scale(FALSE);

	// plot area between max and min in grey
	const auto old_pen = p_dc->SelectObject(&pen_table_[SILVER_COLOR]);
	const auto p_old_b = static_cast<CBrush*>(p_dc->SelectStockObject(LTGRAY_BRUSH));
	p_dc->Polygon(&m_pts_area_[0], m_tpl_len_ * 2 + 1);
	p_dc->SelectObject(p_old_b);

	// plot central curve
	p_dc->SelectStockObject(BLACK_PEN);
	p_dc->Polyline(&m_pts_avg_[0], m_tpl_len_);

	// restore resources
	p_dc->SelectObject(old_pen);
	p_dc->RestoreDC(n_saved_dc);
}

void CTemplateWnd::get_extents()
{
	if (y_we_ == 1) // && m_yWO == 0)
	{
		auto p_sup = m_p_max0_;
		auto p_inf = m_p_min0_;
		auto max_val = *p_sup;
		auto min_val = *p_inf;
		for (auto i = 0; i < m_tpl_len_; i++, p_sup++, p_inf++)
		{
			if (*p_sup > max_val) max_val = *p_sup;
			if (*p_inf < min_val) min_val = *p_inf;
		}

		y_we_ = max_val - min_val + 2;
		y_wo_ = (max_val + min_val) / 2;
	}

	if (x_we_ == 1) // && m_xWO == 0)
	{
		x_we_ = m_tpl_len_;
		x_wo_ = 0;
	}
}

void CTemplateWnd::init_polypoint_abscissa()
{
	m_pts_area_.SetSize(m_tpl_len_ * 2 + 1);
	m_pts_avg_.SetSize(m_tpl_len_);
	auto i1 = 0;
	auto i2 = m_pts_area_.GetUpperBound() - 1;

	for (auto index = 0; index < m_tpl_len_; index++, i1++, i2--)
	{
		m_pts_avg_[i1].x = index;
		m_pts_area_[i1].x = index;
		m_pts_area_[i2].x = index;
	}
	m_pts_area_[m_pts_area_.GetUpperBound()].x = m_pts_area_[0].x;
}


void CTemplateWnd::fill_ordinates_at_scale(BOOL bScale)
{
	// fill with average data
	auto p_avg = m_p_avg_;
	auto p_max = m_p_max0_;
	auto p_min = m_p_min0_;

	auto i1 = 0;
	auto i2 = m_pts_area_.GetUpperBound() - 1;

	if (!bScale)
	{
		for (auto i = 0; i < m_tpl_len_; i++, i1++, i2--)
		{
			m_pts_avg_[i].y = *p_avg;
			m_pts_area_[i1].y = *p_max;
			m_pts_area_[i2].y = *p_min;
			p_avg++;
			p_max++;
			p_min++;
		}
	}
	else
	{
		for (int i = 0; i < m_tpl_len_; i++, i1++, i2--)
		{
			m_pts_avg_[i].y = MulDiv(*p_avg - y_wo_, y_ve_, y_we_) + y_vo_;
			m_pts_area_[i1].y = MulDiv(*p_max - y_wo_, y_ve_, y_we_) + y_vo_;
			m_pts_area_[i2].y = MulDiv(*p_min - y_wo_, y_ve_, y_we_) + y_vo_;
			p_avg++;
			p_max++;
			p_min++;
		}
	}
	m_pts_area_[m_pts_area_.GetUpperBound()].y = m_pts_area_[0].y;
}

void CTemplateWnd::set_template_length(int len, int extent, int org)
{
	if (len == 0)
		len = m_tpl_len_;
	if (len != m_tpl_len_)
	{
		delete_arrays();

		m_tpl_len_ = len;
		m_n_items_ = 0;

		m_p_sum_array_ = new mytype[len * 2];
		m_p_sum0_ = m_p_sum_array_;
		m_p_sum20_ = m_p_sum_array_ + len;

		m_p_avg_ = new int[len * 3];
		m_p_max0_ = m_p_avg_ + m_tpl_len_;
		m_p_min0_ = m_p_avg_ + 2 * m_tpl_len_;
		t_init();
	}

	if (org < 0)
		org = 0;
	if (extent < 0)
		extent = m_tpl_len_ - org;
	x_we_ = extent;
	x_wo_ = org;
}

void CTemplateWnd::t_init()
{
	memset(m_p_sum_array_, 0, m_tpl_len_ * 2 * sizeof(mytype));
	memset(m_p_avg_, 0, m_tpl_len_ * 3 * sizeof(int));
	m_n_items_ = 0;
}

void CTemplateWnd::t_add_spike_top_sum(int* p_source)
{
	mytype* p_sum = m_p_sum0_;
	mytype* p_sum2 = m_p_sum20_;
	m_n_items_++;

	for (auto i = 0; i < m_tpl_len_; i++, p_sum++, p_sum2++, p_source++)
	{
		const auto x = static_cast<mytype>(*p_source);
		*p_sum += x;
		*p_sum2 += (x * x);
	}
	// mark pMean data as not valid
	m_b_valid_ = FALSE;
}

void CTemplateWnd::t_set_display_data()
{
	auto p_mean = m_p_avg_;
	auto p_sup = m_p_max0_;
	auto p_inf = m_p_min0_;

	mytype* p_sum = m_p_sum0_;
	mytype* p_sum2 = m_p_sum20_;
	mytype y_sum, y_sum2;
	const auto xn = static_cast<mytype>(m_n_items_);

	if (m_n_items_ < 5)
	{
		y_sum2 = m_global_std * m_k_tolerance;
		for (auto i = 0; i < m_tpl_len_; i++, p_sum++, p_mean++, p_sup++, p_inf++)
		{
			y_sum = *p_sum / xn;
			*p_mean = static_cast<int>(y_sum);
			*p_inf = static_cast<int>(y_sum - y_sum2);
			*p_sup = static_cast<int>(y_sum + y_sum2);
		}
	}
	else
	{
		for (auto i = 0; i < m_tpl_len_; i++, p_sum++, p_sum2++, p_mean++, p_sup++, p_inf++)
		{
			y_sum = *p_sum;
			y_sum2 = sqrt((*p_sum2 - y_sum * y_sum / xn) / (xn - 1.f)) * m_k_tolerance;
			y_sum /= xn;
			*p_mean = static_cast<int>(y_sum);
			*p_inf = static_cast<int>(y_sum - y_sum2);
			*p_sup = static_cast<int>(y_sum + y_sum2);
		}
	}

	t_power_of_p_sum();
	m_b_valid_ = TRUE;
}

double CTemplateWnd::t_power_of_p_sum()
{
	mytype* p_sum = m_p_sum0_ + x_wo_;
	double xi;
	double x = 0.f;
	const auto last = x_we_ + x_wo_;

	for (auto i = x_wo_; i < last; i++, p_sum++)
	{
		xi = *p_sum;
		x += xi * xi;
	}
	xi = m_n_items_;
	m_power = sqrt(x / x_we_) / xi;
	return m_power;
}

// ---------------------------------------------------------------------------------
// tGetNumberOfPointsWithin()
//
//	get number of points within limits of 
//	template and current spike
//	if (n points >= tpl_len*hit_rate/100) i_yes = 1
//	if (n points <  tpl_len*hit_rate/100) i_yes = 0

BOOL CTemplateWnd::t_get_number_of_points_within(int* p_source, const int* hit_rate)
{
	if (!m_b_valid_) // we need valid limits..
		t_set_display_data();

	const int last = x_we_ + x_wo_;
	//int* p_mean = m_p_avg_ + x_wo_;
	int* p_sup = m_p_max0_ + x_wo_;
	int* p_inf = m_p_min0_ + x_wo_;
	int* p_data = p_source + x_wo_;
	int n_within = 0;

	for (int i = x_wo_; i < last; i++, p_data++, p_sup++, p_inf++)
	{
		if (*p_data <= *p_sup && *p_data >= *p_inf)
			n_within++;
	}

	const int i_hit_rate = MulDiv(*hit_rate, x_we_, 100);
	return (n_within >= i_hit_rate);
}

// ---------------------------------------------------------------------------------
// tDist
//
//	compute distance between template and current spike
//	out: xdist = sum(abs(t(i)-x(i+t_pkd-1)), i=1, tpl_len)/t_power(it)

double CTemplateWnd::t_dist(int* p_source) const
{
	// assume power correctly set by calling routine tMinDist
	//	if (!m_bValid)				// we need valid limits
	//		t_Set_display_Data();		// and also correct power

	const int last = x_we_ + x_wo_;
	int* p_mean = m_p_avg_ + x_wo_;
	int* p_data = p_source + x_wo_;
	double ii = 0;

	for (int i = x_wo_; i < last; i++, p_data++, p_mean++)
	{
		ii += abs(*p_mean - *p_data);
	}
	const double xdist = ii / m_power;
	return xdist;
}

// ---------------------------------------------------------------------------------
// tMinDist
//
//	compute minimal distance between template and current spike
//	jitter of -2/+2
//	out: xdist = sum(abs(t(i)-x(i+tp_kd-1)), i=1, tpl_len)/t_power(it)

double CTemplateWnd::t_min_dist(int* p_source, int* i_offset_min, const BOOL b_jitter)
{
	if (!m_b_valid_) // we need valid limits..
		t_set_display_data(); // and also correct power

	const int jitter = (b_jitter ? 2 : 0);
	int* p_data = p_source - jitter;
	double xdist = t_dist(p_data);
	double x_min_dist = xdist;
	*i_offset_min = -jitter;
	p_data++;

	for (int i = -jitter + 1; i <= jitter; i++, p_data++)
	{
		xdist = t_dist(p_data);
		if (xdist < x_min_dist)
		{
			x_min_dist = xdist;
			*i_offset_min = i;
		}
	}

	return x_min_dist;
}

// ---------------------------------------------------------------------------------
// t_Global_stats
//
//	compute global_std, global distance

void CTemplateWnd::t_global_stats(double* g_std, double* g_dist)
{
	double y_sum = 0;
	double y_sum2 = 0;
	const double xn = m_n_items_;

	//... compute global std and global distance
	const int last = x_we_ + x_wo_;
	mytype* p_sum = m_p_sum0_ + x_wo_;
	mytype* p_sum2 = m_p_sum20_ + x_wo_;
	double y_std = 0;

	for (int i = x_wo_; i < last; i++, p_sum++, p_sum2++)
	{
		y_sum += *p_sum;
		y_sum2 += *p_sum2;
		y_std += sqrt((*p_sum2 - *p_sum * *p_sum / xn) / (xn - 1.));
	}

	const double x = xn * static_cast<double>(x_we_);
	*g_std = sqrt((y_sum2 - (y_sum * y_sum / x)) / (x - 1.));

	//.... compute global distance
	if (!m_b_valid_)
		t_power_of_p_sum();
	*g_dist = y_std * m_k_tolerance / m_power;
}

// ---------------------------------------------------------------------------------
// trap messages and send them to parent

void CTemplateWnd::OnLButtonDblClk(const UINT n_flags, CPoint point)
{
	MapWindowPoints(GetParent(), &point, 1);
	GetParent()->PostMessage(WM_LBUTTONDBLCLK, n_flags, MAKELPARAM(point.x, point.y));
}

void CTemplateWnd::OnLButtonDown(const UINT n_flags, CPoint point)
{
	MapWindowPoints(GetParent(), &point, 1);
	GetParent()->PostMessage(WM_LBUTTONDOWN, n_flags, MAKELPARAM(point.x, point.y));
}

void CTemplateWnd::OnLButtonUp(const UINT n_flags, CPoint point)
{
	MapWindowPoints(GetParent(), &point, 1);
	GetParent()->PostMessage(WM_LBUTTONUP, n_flags, MAKELPARAM(point.x, point.y));
}

void CTemplateWnd::OnMouseMove(const UINT n_flags, CPoint point)
{
	MapWindowPoints(GetParent(), &point, 1);
	GetParent()->PostMessage(WM_MOUSEMOVE, n_flags, MAKELPARAM(point.x, point.y));
}

void CTemplateWnd::OnRButtonDown(const UINT n_flags, CPoint point)
{
	MapWindowPoints(GetParent(), &point, 1);
	GetParent()->PostMessage(WM_RBUTTONDOWN, n_flags, MAKELPARAM(point.x, point.y));
}

void CTemplateWnd::OnRButtonUp(const UINT n_flags, CPoint point)
{
	MapWindowPoints(GetParent(), &point, 1);
	GetParent()->PostMessage(WM_RBUTTONUP, n_flags, MAKELPARAM(point.x, point.y));
}

BOOL CTemplateWnd::OnSetCursor(CWnd* p_wnd, UINT n_hit_test, UINT message)
{
	return 0;
}
