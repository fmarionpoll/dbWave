#include "StdAfx.h"
#include "Ruler.h"



CRuler::CRuler()
{
	m_bHorizontal = TRUE;
	m_dfirst = 0.;
	m_dlast = 0.;
	m_dscaleinc = 0.;
	m_dscalefirst = 0.;
	m_dscalelast = 0.;
}

CRuler::~CRuler()
{
}

void CRuler::SetRange(float* dfirst, float* dlast)
{
	m_dfirst = *dfirst;
	m_dlast = *dlast;
	if (m_dfirst > m_dlast)
	{
		const auto x = m_dfirst;
		m_dfirst = m_dlast;
		m_dlast = x;
	}
	AdjustScale();
}

void CRuler::UpdateRange(float* dfirst, float* dlast)
{
	if (*dfirst != m_dfirst || *dlast != m_dlast)
		SetRange(dfirst, dlast);
}

BOOL CRuler::AdjustScale()
{
	// cf Bramley M. (2000) Data-Based Axis Determination. C/C++ Users Journal 18(7) 20-24
	// http://drdobbs.com/184401258

	const auto range = m_dlast - m_dfirst;

	// deal with repeated values
	if (range == 0)
	{
		m_dfirst = ceil(m_dlast) - 1;
		m_dlast = m_dfirst + 1;
		m_dscaleinc = 1;
		return TRUE;
	}

	// compute increment
	const auto exponent = ceil(log10(range / 10));
	m_dscaleinc = pow(10, exponent);

	// set min scale
	m_dscalefirst = static_cast<long>(m_dfirst / m_dscaleinc) * m_dscaleinc;
	if (m_dscalefirst > m_dfirst)
		m_dscalefirst -= m_dscaleinc;
	// set max scale
	m_dscalelast = m_dscalefirst;
	int i = 0;
	do {
		++i;
		m_dscalelast += m_dscaleinc;
	} while (m_dscalelast < m_dlast);

	// adjust for too few tickmarks
	const auto i_toofew = 5;
	if (i < i_toofew)
	{
		m_dscaleinc /= 2;
		if ((m_dscalefirst + m_dscaleinc) <= m_dfirst)
			m_dscalefirst += m_dscaleinc;
		if ((m_dscalelast - m_dscaleinc) >= m_dlast)
			m_dscalelast -= m_dscaleinc;
	}
	return TRUE;
}
