#include "StdAfx.h"
#include "Ruler.h"


CRuler::CRuler()
= default;

CRuler::~CRuler()
= default;

void CRuler::SetRange(double dfirst, double dlast)
{
	m_lowest_value = dfirst;
	m_highest_value = dlast;
	if (m_lowest_value > m_highest_value)
	{
		const auto x = m_lowest_value;
		m_lowest_value = m_highest_value;
		m_highest_value = x;
	}
	AdjustScale();
}


void CRuler::UpdateRange(double dfirst, double dlast)
{
	if (dfirst != m_lowest_value || dlast != m_highest_value)
		SetRange(dfirst, dlast);
}

BOOL CRuler::AdjustScale()
{
	// cf Bramley M. (2000) Data-Based Axis Determination. C/C++ Users Journal 18(7) 20-24
	// http://drdobbs.com/184401258

	const auto last = m_highest_value;
	const auto first = m_lowest_value;
	const auto range = last - first;

	// deal with repeated values
	if (range == 0.)
	{
		m_lowest_value = ceil(last) - 1;
		m_highest_value = m_lowest_value + 1;
		m_length_major_scale = 1;
		return TRUE;
	}

	// compute increment
	const auto exponent = ceil(log10(range / 10));
	m_length_major_scale = pow(10, exponent);

	// set min scale
	m_first_major_scale = static_cast<long>(first / m_length_major_scale) * m_length_major_scale;
	if (m_first_major_scale > first)
		m_first_major_scale -= m_length_major_scale;
	// set max scale
	m_last_major_scale = m_first_major_scale;
	int i = 0;
	do
	{
		++i;
		m_last_major_scale += m_length_major_scale;
	}
	while (m_last_major_scale < last);

	// adjust for too few tickmarks
	constexpr auto i_toofew = 5;
	if (i < i_toofew)
	{
		m_length_major_scale /= 2;
		if ((m_first_major_scale + m_length_major_scale) <= first)
			m_first_major_scale += m_length_major_scale;
		if ((m_last_major_scale - m_length_major_scale) >= last)
			m_last_major_scale -= m_length_major_scale;
	}
	return TRUE;
}
