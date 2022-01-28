#pragma once


class OPTIONS_IMPORT : public CObject
{
	DECLARE_SERIAL(OPTIONS_IMPORT);
	OPTIONS_IMPORT();
	virtual ~OPTIONS_IMPORT();
	OPTIONS_IMPORT& operator = (const OPTIONS_IMPORT& arg);
	void Serialize(CArchive& ar) override;

public:
	BOOL  bChanged;					// flag set TRUE if contents has changed
	WORD  wVersion;
	// Varian files
	float fGainFID;					// xgain in the FID channel
	float fGainEAD;					// xgain in the EAD channel
	CString pathWTOASCII;

	// generic files
	BOOL	bSingleRun;
	BOOL	bPreview;
	BOOL	bSapid3_5;
	BOOL	bDummy;
	BOOL	bDiscardDuplicateFiles = false;
	BOOL	bReadColumns = false;
	BOOL	bHeader = false;
	short	nbRuns;
	short	nbChannels;
	float	samplingRate;
	short	encodingMode;
	short	bitsPrecision;
	float	voltageMax;
	float	voltageMin;
	int		skipNbytes;
	CString	title;
	CWaveChanArray* pwaveChanArray;

	// last selected filter - index
	int		nSelectedFilter;
	// export options
	WORD	exportType;				// type of export (0=sapid 3.5, 1=txt, 2=excel, ...)
	BOOL	bAllchannels;			// export all channels or one
	BOOL	bSeparateComments;		// comments in a separate file
	BOOL	bentireFile;			// complete file / interval
	BOOL	bincludeTime;			// include time steps (excel, txt)
	int		selectedChannel;		// one channel
	float	fTimefirst;				// interval first
	float	fTimelast;				// interval last
	int		iundersample;			// undersample factor when exporting data

	CString path;					// path to files to import
};

