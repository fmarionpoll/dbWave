#include "stdafx.h"
#include "STIMDETECT.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_SERIAL(STIMDETECT, CObject, 0)

STIMDETECT::STIMDETECT()
= default;

STIMDETECT::~STIMDETECT()
= default;

STIMDETECT& STIMDETECT::operator =(const STIMDETECT& arg)
{
	if (this != &arg)
	{
		//comment=arg.comment;	// CArchive
		nItems = arg.nItems;
		SourceChan = arg.SourceChan;
		TransformMethod = arg.TransformMethod;
		DetectMethod = arg.DetectMethod;
		Threshold1 = arg.Threshold1;
		bMode = arg.bMode;
	}
	return *this;
}

void STIMDETECT::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << wversion;
		ar << nItems;
		ar << SourceChan;
		ar << TransformMethod;
		ar << DetectMethod;
		ar << Threshold1;
		ar << bMode;
	}
	else
	{
		WORD version;
		ar >> version;
		// read data (version 1)
		if (version == 1)
		{
			WORD w1;
			ar >> w1;
			nItems = w1;
			ar >> w1;
			SourceChan = w1;
			ar >> w1;
			TransformMethod = w1;
			ar >> w1;
			DetectMethod = w1;
			ar >> w1;
			Threshold1 = w1;
		}
		else if (version > 1)
		{
			ar >> nItems;
			ar >> SourceChan;
			ar >> TransformMethod;
			ar >> DetectMethod;
			ar >> Threshold1;
			ar >> bMode;
		}
	}
}

