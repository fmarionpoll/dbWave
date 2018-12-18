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
// describing the abcissa and the ordinates
/////////////////////////////////////////////////////////////////////////////


IMPLEMENT_SERIAL(CChanlistItem, CObject, 1)

CChanlistItem::CChanlistItem(): dl_color(0), dl_datavoltspan(0)
{
	pEnvelopeAbcissa = nullptr;
	pEnvelopeOrdinates = nullptr;
	dl_yzero = 2048;
	dl_yextent = 4096;
	dl_datavoltspbin = static_cast<float>(20.) / dl_yextent; // scale factor (1 unit (0-4095) -> y volts)
	dl_databinzero = 2048;
	dl_databinspan = 4096;
	dl_bprint = 1; 
	dl_bHZtagsPrint = FALSE; // do not print HZ tags
	dl_indexabcissa = 0;
	dl_indexordinates = 0;
	dl_bHZtagsPrint = false;
}


//---------------------------------------------------------------------------
// chan list item constructor: store pointer to 2 envelopes abcissa+ordinates
//---------------------------------------------------------------------------

CChanlistItem::CChanlistItem(CEnvelope* p_envelope_X, const int index_x, CEnvelope* p_envelope_Y, const int index_y)
{
	pEnvelopeAbcissa = p_envelope_X;
	pEnvelopeOrdinates = p_envelope_Y;
	dl_indexabcissa = index_x;
	dl_indexordinates = index_y;
}


//---------------------------------------------------------------------------
// init display parameters from chan list item
//---------------------------------------------------------------------------

void CChanlistItem::InitDisplayParms(WORD new_penwidth, WORD new_color, int new_zero, int new_yextent, WORD new_drawmode)
{
	dl_penwidth = new_penwidth;
	dl_color = new_color;
	dl_yzero = new_zero;
	dl_yextent = new_yextent;
	dl_bprint = new_drawmode;	
	dl_datavoltspbin=static_cast<float>(20.)/dl_yextent;
	dl_databinzero=2048;
	dl_databinspan=4096;	
	dl_bHZtagsPrint = FALSE;
	dl_datavoltspan = 0.0f;
}

void CChanlistItem::Serialize (CArchive &ar)
{
	if (ar.IsStoring())
	{
		ar << dl_yzero;			// zero volts
		ar << dl_yextent;		// max to min extent
		ar << dl_penwidth;		// pen size
		ar << dl_color;			// color
		ar << dl_bprint;		// draw mode 
		ar << dl_datavoltspbin;	// scale factor (1 unit (0-4095) -> y volts)
		ar << dl_databinzero;	// value of zero volts
		ar << dl_databinspan;	// nb of bins encoding values within envelope
		ar << dl_bHZtagsPrint;	// print HZ tags flag
		ar << dl_indexabcissa;
		ar << dl_indexordinates;
	}
	else
	{
		ar >> dl_yzero;			// zero volts
		ar >> dl_yextent;		// max to min extent
		ar >> dl_penwidth;		// pen size
		ar >> dl_color;			// color
		ar >> dl_bprint;		// draw mode 
		ar >> dl_datavoltspbin;	// scale factor (1 unit (0-4095) -> y volts)
		ar >> dl_databinzero;		// value of zero volts
		ar >> dl_databinspan;		// nb of bins encoding values within envelope
		ar >> dl_bHZtagsPrint;	// print HZ tags flag
		ar >> dl_indexabcissa;
		ar >> dl_indexordinates;
	}
}

void CChanlistItem::GetEnvelopeArrayIndexes(int& ix, int& iy) const
{
	ix = dl_indexabcissa;
	iy = dl_indexordinates;
}

void CChanlistItem::SetEnvelopeArrays(CEnvelope* px, int ix, CEnvelope* py, int iy)
{
	dl_indexabcissa = ix;
	dl_indexordinates = iy;
	pEnvelopeAbcissa = px;
	pEnvelopeOrdinates = py;
}