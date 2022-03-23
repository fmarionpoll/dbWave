#include "StdAfx.h"
#include "SpikeElement.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


SpikeElement::SpikeElement()
= default;

SpikeElement::SpikeElement(const long time, const int channel)
{
	m_iitime = time;
	m_chanparm = channel;
}

SpikeElement::SpikeElement(const long time, const int channel, const int max, const int min, const int offset, const int class_i, int d_maxmin)
{
	m_iitime = time;
	m_chanparm = channel;
	m_min = min;
	m_max = max;
	m_offset = offset;
	m_class = class_i;
	m_dmaxmin = d_maxmin;
}

SpikeElement::~SpikeElement()
= default;

IMPLEMENT_SERIAL(SpikeElement, CObject, 0 /* schema number*/)

void SpikeElement::Serialize(CArchive& ar)
{
	WORD w1;
	WORD wVersion = 2;

	if (ar.IsStoring())
	{
		ar << wVersion;
		ar << m_iitime;
		ar << static_cast<WORD>(m_class);
		ar << static_cast<WORD>(m_chanparm);
		ar << static_cast<WORD>(m_max);
		ar << static_cast<WORD>(m_min);
		ar << static_cast<WORD>(m_offset);
		ar << static_cast<WORD>(m_dmaxmin);
		ar << static_cast<WORD>(2);
		ar << y1_;
		ar << dt_;
	}
	else
	{
		ar >> wVersion;
		ar >> m_iitime;
		ar >> w1; m_class = static_cast<int>(w1);
		ar >> w1; m_chanparm = static_cast<int>(w1);
		ar >> w1; m_max = static_cast<short>(w1);
		ar >> w1; m_min = static_cast<short>(w1);
		ar >> w1; m_offset = static_cast<short>(w1);
		ar >> w1; m_dmaxmin = static_cast<short>(w1);
		if (wVersion > 1)
		{
			WORD n_items = 0;
			ar >> n_items;
			ar >> y1_; n_items--;
			ar >> dt_; n_items--;
			ASSERT(n_items == 0);
		}
	}
}

void SpikeElement::Read0(CArchive& ar)
{
	WORD w1;

	ASSERT(ar.IsStoring() == FALSE);

	ar >> m_iitime;
	ar >> w1;
	m_class = static_cast<int>(w1);
	ar >> w1;
	m_chanparm = static_cast<int>(w1);
	ar >> w1;
	m_max = static_cast<short>(w1);
	ar >> w1;
	m_min = static_cast<short>(w1);
	ar >> w1;
	m_offset = static_cast<short>(w1);
	m_dmaxmin = 0;
}
