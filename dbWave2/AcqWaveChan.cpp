#include "StdAfx.h"
#include "AcqWaveChan.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_SERIAL(CWaveChan, CObject, 0 /* schema number*/)

CWaveChan::CWaveChan()
{
	am_csComment.Empty();
	am_gaintotal = 0.0f;
	am_resolutionV = 0.;
	am_adchannel = 0;
	am_gainAD = 1;
	am_csheadstage = _T("none");
	am_gainheadstage = 1;
	am_csamplifier = _T("Unknown");
	am_amplifierchan = 0;
	am_gainpre = 1;
	am_gainpost = 1;
	am_notchfilt = 0;
	am_lowpass = 0;
	am_inputpos = 0;
	am_inputneg = 0;
	am_csInputpos = _T("DC");
	am_csInputneg = _T("GND");
	am_offset = 0.0f;
	am_amplifiergain = 1.0f;
	am_version = 1;
	am_csversion = _T("mcid");
}

CWaveChan::CWaveChan(CWaveChan& arg)
{
	this->Copy(&arg);
	am_inputpos = arg.am_inputpos;
	am_inputneg = arg.am_inputneg;
	am_version = arg.am_version;
}

CWaveChan::~CWaveChan()
= default;

void CWaveChan::Copy(const CWaveChan* arg)
{
	am_csComment = arg->am_csComment;
	am_adchannel = arg->am_adchannel;
	am_gainAD = arg->am_gainAD;

	am_csheadstage = arg->am_csheadstage;
	am_gainheadstage = arg->am_gainheadstage;

	am_csamplifier = arg->am_csamplifier;
	am_amplifierchan = arg->am_amplifierchan;
	am_gainpre = arg->am_gainpre;
	am_gainpost = arg->am_gainpost;
	am_notchfilt = arg->am_notchfilt;
	am_lowpass = arg->am_lowpass;
	am_offset = arg->am_offset;
	am_csInputpos = arg->am_csInputpos;
	am_csInputneg = arg->am_csInputneg;
	am_amplifiergain = arg->am_amplifiergain;
	am_gaintotal = arg->am_gaintotal;
	am_resolutionV = arg->am_resolutionV;
}

void CWaveChan::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		const auto dummy = am_csversion + am_csComment;
		ar << dummy;
		const auto x_gain = 0.0f;
		ar << x_gain; // dummy
		ar << static_cast<WORD>(am_adchannel) << static_cast<WORD>(am_gainAD);
		ar << am_csheadstage;
		ar << static_cast<WORD>(am_gainheadstage);
		ar << am_csamplifier << static_cast<WORD>(am_amplifierchan);
		ar << static_cast<WORD>(am_gainpre) << static_cast<WORD>(am_gainpost);
		ar << static_cast<WORD>(am_notchfilt) << static_cast<WORD>(am_lowpass);
		am_inputpos = 0; // dummy value
		am_inputneg = 0; // dummy value
		ar << am_inputpos << am_inputneg;
		ar << am_offset;
		ar << am_csInputpos;
		ar << am_csInputneg;
		// addition 5 nov 2010
		ar << am_version;
		ar << am_amplifiergain;
		ar << am_gaintotal;
		ar << am_resolutionV;
	}
	else
	{
		WORD w1, w2, w3, w4;
		ar >> am_csComment;
		float gain; ar >> gain; 
		ar >> w1 >> w2;
		am_adchannel = static_cast<short>(w1);
		am_gainAD = static_cast<short>(w2);
		ar >> am_csheadstage;
		ar >> w1; am_gainheadstage = w1;
		ar >> am_csamplifier;
		ar >> w1 >> w2 >> w3;
		am_amplifierchan = static_cast<short>(w1);
		am_gainpre = static_cast<short>(w2);
		am_gainpost = static_cast<short>(w3);
		ar >> w1 >> w2 >> w3 >> w4;
		am_notchfilt = static_cast<short>(w1);
		am_lowpass = static_cast<short>(w2);
		am_inputpos = static_cast<short>(w3);
		am_inputneg = static_cast<short>(w4);
		ar >> am_offset;
		if (am_inputpos == 0 && am_inputneg == 0)
		{
			ar >> am_csInputpos;
			ar >> am_csInputneg;
		}
		else
		{
			am_csInputpos = GetCyberAmpInput(am_inputpos);
			am_csInputneg = GetCyberAmpInput(am_inputneg);
		}

		// patch to read values stored after version 0
		const auto n_chars = am_csversion.GetLength();
		if (am_csComment.GetLength() >= n_chars && am_csComment.Find(am_csversion, 0) >= 0)
		{
			// extract the dummy sequence ("mcid")
			auto dummy = am_csComment;
			am_csComment = dummy.Right(dummy.GetLength() - n_chars);
			// read the rest of the archive
			ar >> w1;
			ar >> am_amplifiergain;
			ar >> am_gaintotal;
			ar >> am_resolutionV;
		}
	}
}

// Write CWaveChan in a binary file

long CWaveChan::Write(CFile* datafile)
{
	const auto p1 = datafile->GetPosition();
	CArchive ar(datafile, CArchive::store);
	Serialize(ar);
	ar.Close();
	const auto p2 = datafile->GetPosition();
	return static_cast<long>(p2 - p1);
}

// Read CWaveChan from a binary file
BOOL CWaveChan::Read(CFile* datafile)
{
	CArchive ar(datafile, CArchive::load);
	auto flag = TRUE;
	try
	{
		Serialize(ar);
	}
	catch (CException* e)
	{
		e->Delete();
		flag = FALSE;
	}
	ar.Close();
	return flag;
}

CString CWaveChan::GetCyberAmpInput(const int value)
{
	CString cs_out;
	// values:GND(-1) DC(0) .1 1 10 30 100 300 Hz (*10)
	switch (value)
	{
	case -10:
		cs_out = _T("GND");
		break;
	case 0:
		cs_out = _T("DC");
		break;
	default:
		cs_out.Format(_T("%i"), value / 10);
		break;
	}
	return cs_out;
}
