// chanlistitem.cpp
//

#include "StdAfx.h"
#include <stdlib.h>
#include "Envelope.h"
#include "chanlistitem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
//	CChanlistItem
//
// lineview keeps a list of channels to be displayed in a list
// each item of this list group together the parameters necessary to
// display the channel, ie color, etc and a pointer to structures
// describing the abscissa and the ordinates
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_SERIAL(CChanlistItem, CObject, 1)

CChanlistItem::CChanlistItem()
= default;

CChanlistItem::CChanlistItem(CEnvelope* p_envelope_X, const int index_x, CEnvelope* p_envelope_Y, const int index_y)
{
	pEnvelopeAbscissa = p_envelope_X;
	pEnvelopeOrdinates = p_envelope_Y;
	dl_indexabscissa = index_x;
	dl_indexordinates = index_y;
}

void CChanlistItem::InitDisplayParms(WORD new_penwidth, WORD new_color, int new_zero, int new_yextent,
                                     WORD new_drawmode)
{
	dl_penwidth = new_penwidth;
	dl_color = new_color;
	dl_yzero = new_zero;
	dl_yextent = new_yextent;
	dl_bprint = new_drawmode;
	dl_datavoltspbin = static_cast<float>(20.) / dl_yextent;
	dl_databinzero = 2048;
	dl_databinspan = 4096;
	dl_bHZtagsPrint = FALSE;
	dl_datavoltspan = 0.0f;
}

void CChanlistItem::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << dl_yzero; // zero volts
		ar << dl_yextent; // max to min extent
		ar << dl_penwidth; // pen size
		ar << dl_color; // color
		ar << dl_bprint; // draw mode
		ar << dl_datavoltspbin; // scale factor (1 unit (0-4095) -> y volts)
		ar << dl_databinzero; // value of zero volts
		ar << dl_databinspan; // nb of bins encoding values within envelope
		ar << dl_bHZtagsPrint; // print HZ tags flag
		ar << dl_indexabscissa;
		ar << dl_indexordinates;
	}
	else
	{
		ar >> dl_yzero; // zero volts
		ar >> dl_yextent; // max to min extent
		ar >> dl_penwidth; // pen size
		ar >> dl_color; // color
		ar >> dl_bprint; // draw mode
		ar >> dl_datavoltspbin; // scale factor (1 unit (0-4095) -> y volts)
		ar >> dl_databinzero; // value of zero volts
		ar >> dl_databinspan; // nb of bins encoding values within envelope
		ar >> dl_bHZtagsPrint; // print HZ tags flag
		ar >> dl_indexabscissa;
		ar >> dl_indexordinates;
	}
}

void CChanlistItem::GetEnvelopeArrayIndexes(int& ix, int& iy) const
{
	ix = dl_indexabscissa;
	iy = dl_indexordinates;
}

void CChanlistItem::SetEnvelopeArrays(CEnvelope* px, int ix, CEnvelope* py, int iy)
{
	dl_indexabscissa = ix;
	dl_indexordinates = iy;
	pEnvelopeAbscissa = px;
	pEnvelopeOrdinates = py;
}
