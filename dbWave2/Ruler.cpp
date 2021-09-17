#include "StdAfx.h"
#include "Ruler.h"



CRuler::CRuler()
{
	m_is_horizontal = TRUE;
	m_dfirst = 0.;
	m_dlast = 0.;
	m_dscaleinc = 0.;
	m_dscalefirst = 0.;
	m_dscalelast = 0.;
}

CRuler::~CRuler()
= default;

void CRuler::SetRange(double dfirst, double dlast)
{
	m_dfirst = dfirst;
	m_dlast = dlast;
	if (m_dfirst > m_dlast)
	{
		const auto x = m_dfirst;
		m_dfirst = m_dlast;
		m_dlast = x;
	}
	AdjustScale();
}

int CRuler::GetScaleUnitPixels(int cx) const
{ return static_cast<int>(m_dscaleinc * cx / (m_dlast - m_dfirst)); }

double CRuler::GetScaleIncrement() const
{ return m_dscaleinc; }

void CRuler::UpdateRange(double dfirst, double dlast)
{
	if (dfirst != m_dfirst || dlast != m_dlast)
		SetRange(dfirst, dlast);
}

BOOL CRuler::AdjustScale()
{
	// cf Bramley M. (2000) Data-Based Axis Determination. C/C++ Users Journal 18(7) 20-24
	// http://drdobbs.com/184401258

	const auto last = static_cast<double>(m_dlast);
	const auto first = static_cast<double>(m_dfirst);
	const auto range = last - first;

	// deal with repeated values
	if (range == 0)
	{
		m_dfirst = ceil(last) - 1;
		m_dlast = m_dfirst + 1;
		m_dscaleinc = 1;
		return TRUE;
	}

	// compute increment
	const auto exponent = ceil(log10(range / 10));
	m_dscaleinc = pow(10, exponent);

	// set min scale
	m_dscalefirst = static_cast<long>(first / m_dscaleinc) * m_dscaleinc;
	if (m_dscalefirst > first)
		m_dscalefirst -= m_dscaleinc;
	// set max scale
	m_dscalelast = m_dscalefirst;
	int i = 0;
	do {
		++i;
		m_dscalelast += m_dscaleinc;
	} while (m_dscalelast < last);

	// adjust for too few tickmarks
	constexpr auto i_toofew = 5;
	if (i < i_toofew)
	{
		m_dscaleinc /= 2;
		if ((m_dscalefirst + m_dscaleinc) <= first)
			m_dscalefirst += m_dscaleinc;
		if ((m_dscalelast - m_dscaleinc) >= last)
			m_dscalelast -= m_dscaleinc;
	}
	return TRUE;
}
