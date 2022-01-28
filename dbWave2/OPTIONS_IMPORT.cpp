#include "stdafx.h"
#include "OPTIONS_IMPORT.h"
#include "include/DataTranslation/Olxdadefs.h"


IMPLEMENT_SERIAL(OPTIONS_IMPORT, CObject, 0)

OPTIONS_IMPORT::OPTIONS_IMPORT()
{
	wVersion = 6;

	// Varian parameters
	fGainFID = 10.f;
	fGainEAD = 10.f;
	// generic import options
	bSingleRun = TRUE;
	bPreview = TRUE;
	bSapid3_5 = FALSE;
	nbRuns = 1;
	nbChannels = 1;
	samplingRate = 10.0f;
	encodingMode = OLx_ENC_BINARY;
	bitsPrecision = 12;
	voltageMax = 10.0f;
	voltageMin = -10.0f;
	skipNbytes = 5;
	title = "";
	pwaveChanArray = new (CWaveChanArray);
	ASSERT(pwaveChanArray != NULL);
	pwaveChanArray->ChanArray_add();	// add dummy channel
	nSelectedFilter = 0;
	// export options
	exportType = 0;
	bAllchannels = TRUE;
	bSeparateComments = FALSE;
	bincludeTime = FALSE;
	bentireFile = TRUE;
	selectedChannel = 0;
	fTimefirst = 0.f;
	fTimelast = 1.f;
	pathWTOASCII.Empty();
	iundersample = 1;
	path.Empty();
	bDummy = FALSE;
	bDiscardDuplicateFiles = false;
	bChanged = false;
}

OPTIONS_IMPORT::~OPTIONS_IMPORT()
{
	SAFE_DELETE(pwaveChanArray);
}

OPTIONS_IMPORT& OPTIONS_IMPORT::operator = (const OPTIONS_IMPORT& arg)
{
	if (this != &arg) {
		wVersion = arg.wVersion;
		fGainFID = arg.fGainFID;
		fGainEAD = arg.fGainEAD;
		pathWTOASCII = arg.pathWTOASCII;

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
		*pwaveChanArray = *(arg.pwaveChanArray);
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
		wVersion = 6; ar << wVersion;
		ar << fGainFID;
		ar << fGainEAD;

		// generic data import options
		int bflag = bSapid3_5;	// combine flags: update flag, combine
		bflag <<= 1; bflag += bPreview;
		bflag <<= 1; bflag += bSingleRun;
		ar << bflag;

		ar << (WORD)nbRuns;
		ar << (WORD)nbChannels;
		ar << samplingRate;
		ar << (WORD)encodingMode;
		ar << (WORD)bitsPrecision;
		ar << voltageMax;
		ar << voltageMin;
		ar << skipNbytes;
		ar << title;
		pwaveChanArray->Serialize(ar);
		ar << nSelectedFilter;

		// generic data export options
		ar << exportType;
		bflag = bAllchannels;						// 1rst: allchannels
		bflag <<= 1; bflag += bSeparateComments;	// 2nd: separatecomments
		bflag <<= 1; bflag += bentireFile;			// 3rd: entirefile
		bflag <<= 1; bflag += bincludeTime;			// 4th:	includetime
		ar << bflag;

		ar << selectedChannel;
		ar << fTimefirst; ar << fTimelast;

		ar << pathWTOASCII;

		// add extensible options
		int ntypes = 2;
		ar << ntypes;	// nb types

		// int
		ntypes = 2;
		ar << ntypes;	// nb of ints
		ar << iundersample;
		bflag = bDummy;
		bflag <<= 1; bflag += bDiscardDuplicateFiles;
		ar << bflag;

		// CStrings
		ntypes = 1;
		ar << ntypes;	// nb of strings
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
			bSingleRun = bflag;  bSingleRun &= 0x1; bflag >>= 1;
			bPreview = bflag;  bPreview &= 0x1; bflag >>= 1;
			bSapid3_5 = bflag; bSapid3_5 &= 0x1;

			ar >> w1; nbRuns = w1;
			ar >> w1; nbChannels = w1;
			ar >> samplingRate;
			ar >> w1; encodingMode = w1;
			ar >> w1; bitsPrecision = w1;
			ar >> voltageMax;
			ar >> voltageMin;
			ar >> skipNbytes;
			ar >> title;
			pwaveChanArray->Serialize(ar);
		}
		if (version >= 3)
			ar >> nSelectedFilter;
		if (version >= 4)
		{
			ar >> exportType;
			ar >> bflag;
			bincludeTime = bflag; bincludeTime &= 0x1; bflag >>= 1;	// 4th:	includetime
			bentireFile = bflag; bentireFile &= 0x1; bflag >>= 1;	// 3rd: entirefile
			bSeparateComments = bflag; bSeparateComments &= 0x1; bflag >>= 1;// 2nd: separatecomments
			bAllchannels = bflag; bAllchannels &= 0x1;				// 1rst: allchannels
			ar >> selectedChannel;
			ar >> fTimefirst; ar >> fTimelast;
		}
		if (version >= 5)
			ar >> pathWTOASCII;
		if (version >= 6)
		{
			int ntypes;
			ar >> ntypes;	// nb types
			if (ntypes > 0)
			{
				int nints;
				ar >> nints;		// first type: ints
				ar >> iundersample; nints--;
				if (nints > 0)
				{
					ar >> bflag; nints--;
					bDiscardDuplicateFiles = bflag; bDiscardDuplicateFiles &= 0x1; bflag >>= 1;
					bDummy = bflag; bDummy &= 0x1; bflag >>= 1;
				}
			}
			ntypes--;

			if (ntypes > 0)
			{
				int nstrings;
				ar >> nstrings;		// second type: CString
				ar >> path; nstrings--;
			}
			ntypes--;
		}
	}
}


