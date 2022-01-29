#include "StdAfx.h"
#include "OPTIONS_IMPORT.h"
#include "include/DataTranslation/Olxdadefs.h"


IMPLEMENT_SERIAL(OPTIONS_IMPORT, CObject, 0)

OPTIONS_IMPORT::OPTIONS_IMPORT()
= default;

OPTIONS_IMPORT::~OPTIONS_IMPORT()
{
	SAFE_DELETE(pwave_chan_array)
}

OPTIONS_IMPORT& OPTIONS_IMPORT::operator =(const OPTIONS_IMPORT& arg)
{
	if (this != &arg)
	{
		w_version = arg.w_version;
		fGainFID = arg.fGainFID;
		fGainEAD = arg.fGainEAD;
		path_wtoascii = arg.path_wtoascii;

		// generic import options
		bSingleRun = arg.bSingleRun;
		bPreview = arg.bPreview;
		bSapid3_5 = arg.bSapid3_5;
		bDummy = arg.bDummy;
		nbRuns = arg.nbRuns;
		nbChannels = arg.nbChannels;
		samplingRate = arg.samplingRate;
		encodingMode = arg.encodingMode;
		bitsPrecision = arg.bitsPrecision;
		voltageMax = arg.voltageMax;
		voltageMin = arg.voltageMin;
		skipNbytes = arg.skipNbytes;
		title = arg.title;
		*pwave_chan_array = *(arg.pwave_chan_array);
		nSelectedFilter = arg.nSelectedFilter;

		// export options
		exportType = arg.exportType;
		bAllchannels = arg.bAllchannels;
		bSeparateComments = arg.bSeparateComments;
		bincludeTime = arg.bincludeTime;
		selectedChannel = arg.selectedChannel;
		fTimefirst = arg.fTimefirst;
		fTimelast = arg.fTimelast;
		bentireFile = arg.bentireFile;
		iundersample = arg.iundersample;

		// others
		path = arg.path;
	}
	return *this;
}

void OPTIONS_IMPORT::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		w_version = 6;
		ar << w_version;
		ar << fGainFID;
		ar << fGainEAD;

		// generic data import options
		int bflag = bSapid3_5; // combine flags: update flag, combine
		bflag <<= 1;
		bflag += bPreview;
		bflag <<= 1;
		bflag += bSingleRun;
		ar << bflag;

		ar << static_cast<WORD>(nbRuns);
		ar << static_cast<WORD>(nbChannels);
		ar << samplingRate;
		ar << static_cast<WORD>(encodingMode);
		ar << static_cast<WORD>(bitsPrecision);
		ar << voltageMax;
		ar << voltageMin;
		ar << skipNbytes;
		ar << title;
		if (pwave_chan_array == nullptr)
			initialize_wave_chan_array();
		pwave_chan_array->Serialize(ar);
		ar << nSelectedFilter;

		// generic data export options
		ar << exportType;
		bflag = bAllchannels;
		bflag <<= 1;
		bflag += bSeparateComments;
		bflag <<= 1;
		bflag += bentireFile;
		bflag <<= 1;
		bflag += bincludeTime;
		ar << bflag;

		ar << selectedChannel;
		ar << fTimefirst;
		ar << fTimelast;

		ar << path_wtoascii;

		// add extensible options
		int ntypes = 2;
		ar << ntypes;

		// int
		ntypes = 2;
		ar << ntypes;
		ar << iundersample;
		bflag = bDummy;
		bflag <<= 1;
		bflag += bDiscardDuplicateFiles;
		ar << bflag;

		// CStrings
		ntypes = 1;
		ar << ntypes;
		ar << path;
	}
	else
	{
		WORD version;
		int bflag;
		ar >> version;
		ar >> fGainFID;
		ar >> fGainEAD;
		if (version >= 2)
		{
			WORD w1;
			ar >> bflag;
			// decompose flags: update flag (/2),  get value, mask
			bSingleRun = bflag;
			bSingleRun &= 0x1;
			bflag >>= 1;
			bPreview = bflag;
			bPreview &= 0x1;
			bflag >>= 1;
			bSapid3_5 = bflag;
			bSapid3_5 &= 0x1;

			ar >> w1;
			nbRuns = static_cast<short>(w1);
			ar >> w1;
			nbChannels = static_cast<short>(w1);
			ar >> samplingRate;
			ar >> w1;
			encodingMode = static_cast<short>(w1);
			ar >> w1;
			bitsPrecision = static_cast<short>(w1);
			ar >> voltageMax;
			ar >> voltageMin;
			ar >> skipNbytes;
			ar >> title;
			if (pwave_chan_array == nullptr)
				initialize_wave_chan_array();
			pwave_chan_array->Serialize(ar);
		}
		if (version >= 3)
			ar >> nSelectedFilter;
		if (version >= 4)
		{
			ar >> exportType;
			ar >> bflag;
			bincludeTime = bflag;
			bincludeTime &= 0x1;
			bflag >>= 1;
			bentireFile = bflag;
			bentireFile &= 0x1;
			bflag >>= 1;
			bSeparateComments = bflag;
			bSeparateComments &= 0x1;
			bflag >>= 1;
			bAllchannels = bflag;
			bAllchannels &= 0x1;
			ar >> selectedChannel;
			ar >> fTimefirst;
			ar >> fTimelast;
		}
		if (version >= 5)
			ar >> path_wtoascii;
		if (version >= 6)
		{
			int ntypes;
			ar >> ntypes;
			if (ntypes > 0)
			{
				int nints;
				ar >> nints;
				ar >> iundersample;
				nints--;
				if (nints > 0)
				{
					ar >> bflag;
					nints--;
					bDiscardDuplicateFiles = bflag;
					bDiscardDuplicateFiles &= 0x1;
					bflag >>= 1;
					bDummy = bflag;
					bDummy &= 0x1;
					bflag >>= 1;
				}
			}
			ntypes--;

			if (ntypes > 0)
			{
				int nstrings;
				ar >> nstrings;
				ar >> path;
				nstrings--;
			}
			ntypes--;
		}
	}
}
