#include "stdafx.h"
#include "SPKCLASSIF.h"

#include "TemplateListWnd.h"


IMPLEMENT_SERIAL(SPKCLASSIF, CObject, 0 /* schema number*/)

SPKCLASSIF::SPKCLASSIF() : bChanged(0), nintparms(0), nfloatparms(0)
{
	wversion = 2;
	dataTransform = 0; // data transform method (0=raw data)
	iparameter = 0; // type of parameter measured
	ileft = 10; // position of first cursor
	iright = 40; // position of second cursor
	ilower = 0; // second threshold
	iupper = 10; // first threshold
	ixyleft = 10;
	ixyright = 40;
	sourceclass = 0; // source class
	destclass = 0; // destination class
	hitrate = 50;
	hitratesort = 75;
	ktolerance = 1.96f;
	kleft = 10;
	kright = 40;
	rowheight = 100; // height of the spike row within spikeview
	coltext = -1;
	colspikes = 100; // width of the spikes within one row
	colseparator = 5;
	ptpl = nullptr;
	mvmin = 0.f;
	mvmax = 2.f;
	vdestclass = 1;
	vsourceclass = 0;
	fjitter_ms = 1.f;
	bresetzoom = TRUE;
}

SPKCLASSIF::~SPKCLASSIF()
{
	if (ptpl)
		delete static_cast<CTemplateListWnd*>(ptpl);
}

SPKCLASSIF& SPKCLASSIF::operator =(const SPKCLASSIF& arg)
{
	if (this != &arg)
	{
		dataTransform = arg.dataTransform; // transform mode
		iparameter = arg.iparameter; // type of parameter measured
		ileft = arg.ileft; // position of first cursor
		iright = arg.iright; // position of second cursor
		ilower = arg.ilower; // second threshold
		iupper = arg.iupper; // first threshold
		ixyright = arg.ixyright;
		ixyleft = arg.ixyleft;
		sourceclass = arg.sourceclass; // source class
		destclass = arg.destclass; // destination class

		hitrate = arg.hitrate;
		hitratesort = arg.hitratesort;
		ktolerance = arg.ktolerance;
		kleft = arg.kleft;
		kright = arg.kright;
		rowheight = arg.rowheight;
		coltext = arg.coltext;
		colspikes = arg.colspikes;
		colseparator = arg.colseparator;
		vsourceclass = arg.vsourceclass; // source class
		vdestclass = arg.vdestclass; // destination class
		bresetzoom = arg.bresetzoom;
		fjitter_ms = arg.fjitter_ms;

		mvmin = arg.mvmin;
		mvmax = arg.mvmax;

		if (arg.ptpl != nullptr)
		{
			ptpl = new (CTemplateListWnd);
			*static_cast<CTemplateListWnd*>(ptpl) = *static_cast<CTemplateListWnd*>(arg.ptpl);
		}
	}
	return *this;
}

void SPKCLASSIF::Serialize(CArchive& ar)
{
	BOOL btplIspresent = FALSE;
	if (ar.IsStoring())
	{
		wversion = 2;
		ar << wversion;
		ar << static_cast<WORD>(dataTransform);
		ar << static_cast<WORD>(iparameter);
		ar << static_cast<WORD>(ileft);
		ar << static_cast<WORD>(iright);
		ar << static_cast<WORD>(ilower);
		ar << static_cast<WORD>(iupper);
		const auto dummy = static_cast<WORD>(0);
		ar << dummy;
		ar << dummy;

		nfloatparms = 4;
		ar << nfloatparms;
		ar << ktolerance; // 1
		ar << mvmin;
		ar << mvmax;
		ar << fjitter_ms;

		nintparms = 16;
		ar << nintparms;
		ar << kleft; // 1
		ar << kright; // 2
		ar << rowheight; // 3
		ar << hitrate; // 4
		ar << hitratesort; // 5
		btplIspresent = ptpl != nullptr; // test if templatelist is present
		ar << btplIspresent; // 6
		ar << coltext; // 7
		ar << colspikes; // 8
		ar << colseparator; // 9
		ar << sourceclass; // 10
		ar << destclass; // 11
		ar << vsourceclass; // 12
		ar << vdestclass; // 13
		ar << bresetzoom; // 14
		ar << ixyright; // 15
		ar << ixyleft; // 16
	}
	else
	{
		WORD version;
		ar >> version;

		// version 1
		WORD w1;
		ar >> w1;
		dataTransform = w1;
		ar >> w1;
		iparameter = w1;
		ar >> w1;
		ileft = w1;
		ar >> w1;
		iright = w1;
		ar >> w1;
		ilower = w1;
		ar >> w1;
		iupper = w1;
		ar >> w1;
		sourceclass = w1; // dummy in v4
		ar >> w1;
		destclass = w1; // dummy in v4

		// version 2
		if (version > 1)
		{
			// float parameters
			int nfparms;
			ar >> nfparms;
			if (nfparms > 0)
			{
				ar >> ktolerance;
				nfparms--;
			}
			if (nfparms > 0)
			{
				ar >> mvmin;
				nfparms--;
			}
			if (nfparms > 0)
			{
				ar >> mvmax;
				nfparms--;
			}
			if (nfparms > 0)
			{
				ar >> fjitter_ms;
				nfparms--;
			}
			if (nfparms > 0)
			{
				float dummy;
				do
				{
					ar >> dummy;
					nfparms--;
				} while (nfparms > 0);
			}
			ASSERT(nfparms == 0);

			// int parameters
			ar >> nfparms;
			if (nfparms > 0)
			{
				ar >> kleft;
				nfparms--;
			}
			if (nfparms > 0)
			{
				ar >> kright;
				nfparms--;
			}
			if (nfparms > 0)
			{
				ar >> rowheight;
				nfparms--;
			}
			if (nfparms > 0)
			{
				ar >> hitrate;
				nfparms--;
			}
			if (nfparms > 0)
			{
				ar >> hitratesort;
				nfparms--;
			}
			if (nfparms > 0)
			{
				ar >> btplIspresent;
				nfparms--;
			}
			if (nfparms > 0)
			{
				ar >> coltext;
				nfparms--;
			}
			if (nfparms > 0)
			{
				ar >> colspikes;
				nfparms--;
			}
			if (nfparms > 0)
			{
				ar >> colseparator;
				nfparms--;
			}
			if (nfparms > 0)
			{
				ar >> sourceclass;
				nfparms--;
			}
			if (nfparms > 0)
			{
				ar >> destclass;
				nfparms--;
			}
			if (nfparms > 0)
			{
				ar >> vsourceclass;
				nfparms--;
			}
			if (nfparms > 0)
			{
				ar >> vdestclass;
				nfparms--;
			}
			if (nfparms > 0)
			{
				ar >> bresetzoom;
				nfparms--;
			}
			if (nfparms > 0)
			{
				ar >> ixyright;
				nfparms--;
			}
			if (nfparms > 0)
			{
				ar >> ixyleft;
				nfparms--;
			}

			if (nfparms > 0)
			{
				int dummy;
				do
				{
					ar >> dummy;
					nfparms--;
				} while (nfparms > 0);
			}
			ASSERT(nfparms == 0);

			if (!btplIspresent && ptpl != nullptr)
				delete static_cast<CTemplateListWnd*>(ptpl);
		}
	}

	// serialize templates
	if (btplIspresent)
	{
		if (ptpl == nullptr)
			ptpl = new (CTemplateListWnd);
		static_cast<CTemplateListWnd*>(ptpl)->Serialize(ar);
	}
}


void SPKCLASSIF::CreateTPL()
{
	ptpl = new (CTemplateListWnd);
}
