#include "StdAfx.h"
#include "SpikeElement.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CSpikeElemt

CSpikeElemt::CSpikeElemt()
{
	m_iitime = 0;
	m_class = 0;
	m_chanparm = 0;
	m_min = 0;
	m_max = 4096;
	m_offset = 2048;
	m_dmaxmin = 0;
}

CSpikeElemt::CSpikeElemt(long time, WORD channel)
{
	m_iitime = time;
	m_chanparm = channel;
	m_class = 0;
	m_min = 0;
	m_max = 4096;
	m_offset = 2048;
	m_dmaxmin = 0;
}

CSpikeElemt::CSpikeElemt(long time, WORD channel, int max, int min, int offset, int iclass, int dmaxmin)
{
	m_iitime = time;
	m_chanparm = channel;
	m_min = min;
	m_max = max;
	m_offset = offset;
	m_class = iclass;
	m_dmaxmin = dmaxmin;
}

CSpikeElemt::~CSpikeElemt()
{
}

IMPLEMENT_SERIAL(CSpikeElemt, CObject, 0 /* schema number*/)

void CSpikeElemt::Serialize(CArchive& ar)
{
	WORD w1;
	WORD wVersion = 2;

	if (ar.IsStoring())
	{
		ar << wVersion;
		ar << m_iitime;
		ar << static_cast<WORD>(m_class);
		ar << static_cast<WORD>(m_chanparm);
		ar << static_cast<WORD> (m_max);
		ar << static_cast<WORD> (m_min);
		ar << static_cast<WORD> (m_offset);
		ar << static_cast<WORD> (m_dmaxmin);
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
			WORD nitems = 0; ar >> nitems;
			ar >> y1_; nitems--;
			ar >> dt_; nitems--;
			ASSERT(nitems == 0);
		}
	}
}

void CSpikeElemt::Read0(CArchive& ar)
{
	WORD w1;

	ASSERT(ar.IsStoring() == FALSE);

	ar >> m_iitime;
	ar >> w1; m_class = static_cast<int>(w1);
	ar >> w1; m_chanparm = static_cast<int>(w1);
	ar >> w1; m_max = static_cast<short>(w1);
	ar >> w1; m_min = static_cast<short>(w1);
	ar >> w1; m_offset = static_cast<short>(w1);
	m_dmaxmin = 0;
}