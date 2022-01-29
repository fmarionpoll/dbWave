#pragma once
#include "AcqWaveChanArray.h"
#include <Olxdadefs.h>

class OPTIONS_IMPORT : public CObject
{
	DECLARE_SERIAL(OPTIONS_IMPORT)
	OPTIONS_IMPORT();
	~OPTIONS_IMPORT() override;
	OPTIONS_IMPORT& operator =(const OPTIONS_IMPORT& arg);
	void Serialize(CArchive& ar) override;

public:
	BOOL b_changed{false};
	WORD w_version{6};

	// Varian files
	float fGainFID{10.f};
	float fGainEAD{10.f};
	CString path_wtoascii{};

	// generic files
	BOOL bSingleRun{true};
	BOOL bPreview{true};
	BOOL bSapid3_5{false};
	BOOL bDummy{false};
	BOOL bDiscardDuplicateFiles{false};
	BOOL bReadColumns{false};
	BOOL bHeader{false};
	short nbRuns{1};
	short nbChannels{1};
	float samplingRate{10.f};
	short encodingMode{OLx_ENC_BINARY};
	short bitsPrecision{12};
	float voltageMax{10.f};
	float voltageMin{-10.f};
	int skipNbytes{5};
	CString title{};
	CWaveChanArray* pwave_chan_array{nullptr};

	// last selected filter - index
	int nSelectedFilter{0};
	// export options
	WORD exportType{2}; // type of export (0=sapid 3.5, 1=txt, 2=excel, ...)
	BOOL bAllchannels{true}; // export all channels or one
	BOOL bSeparateComments{false}; // comments in a separate file
	BOOL bentireFile{true}; // complete file / interval
	BOOL bincludeTime{true}; // include time steps (excel, txt)
	int selectedChannel{0}; // one channel
	float fTimefirst{0.f}; // interval first
	float fTimelast{2.f}; // interval last
	int iundersample{1}; // undersample factor when exporting data

	CString path{}; // path to files to import

	void initialize_wave_chan_array()
	{
		pwave_chan_array = new (CWaveChanArray);
		ASSERT(pwave_chan_array != NULL);
		pwave_chan_array->ChanArray_add();
	}
};
