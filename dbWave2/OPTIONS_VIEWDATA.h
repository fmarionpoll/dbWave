#pragma once

#include "SCOPESTRUCT.h"


class OPTIONS_VIEWDATA : public CObject
{
	DECLARE_SERIAL(OPTIONS_VIEWDATA)
	OPTIONS_VIEWDATA();
	~OPTIONS_VIEWDATA() override;
	OPTIONS_VIEWDATA& operator =(const OPTIONS_VIEWDATA& arg);
	void Serialize(CArchive& ar) override;

public:
	BOOL b_changed{false}; // flag set TRUE if contents has changed
	WORD m_wversion{5}; // version number

	// print options
	BOOL bAcqComment{true}; // print data acquisition comments
	BOOL bAcqDateTime{true}; // data acquisition date and time
	BOOL bChansComment{true}; // individual data acq channel description
	BOOL bChanSettings{true}; // amplifier's settings (CyberAmp conventions)
	BOOL bDocName{true}; // file name
	BOOL bFrameRect{false}; // draw a rectangle frame around the data
	BOOL bClipRect{true}; // clip data to the rectangle
	BOOL bTimeScaleBar{true}; // display time scale bar
	BOOL bVoltageScaleBar{true}; // display voltage scale bar
	BOOL bPrintSelection{true}; // print only selection or whole series
	BOOL bPrintSpkBars{true}; // for viewspkd view, print bars or not
	BOOL bFilterDataSource{true}; // display source data filtered

	// print parameters
	int horzRes{1}; // horizontal printer resolution
	int vertRes{1}; // vertical printer resolution
	int HeightDoc{300}; // height of one document
	int WidthDoc{1500}; // width of one document
	int leftPageMargin{100}; // page left margin (pixels)
	int bottomPageMargin{40}; // bottom
	int rightPageMargin{40}; // right
	int topPageMargin{100}; // top
	int heightSeparator{40}; // vertical space between consecutive displays
	int textseparator{40}; // space between left margin of display and text
	int fontsize{40}; // font size (pixels)
	int spkheight{0}; //
	int spkwidth{0}; //

	// how data are displayed from file to file
	BOOL bAllChannels{true}; // display all channels
	BOOL b_complete_record{true}; // display entire record
	BOOL b_multiple_rows{false}; // display one record on consecutive lines
	BOOL bSplitCurves{true}; // arrange curves so that they are distributed vertically
	BOOL bMaximizeGain{true}; // maximize gain
	BOOL bCenterCurves{true}; // center curves
	BOOL bKeepForEachFile{true}; // keep parameters for each file
	BOOL bDispOldDetectP{false}; // spk detection params: display old (true), keep same
	BOOL bFilterDat{false}; // for dbViewdata
	BOOL bDetectWhileBrowse{true}; // for spkdetectionview
	BOOL bGotoRecordID{true}; // for dbWaveView
	BOOL bDisplayFileName{true}; // for dbViewData
	BOOL bsetTimeSpan{false};
	BOOL bsetmVSpan{false};
	BOOL bDisplayAllClasses{true};

	// export infos from data files
	BOOL bacqcomments{true}; // global comment
	BOOL bacqdate{true}; // acquisition date
	BOOL bacqtime{true}; // acquisition time
	BOOL bfilesize{false}; // file size
	BOOL bacqchcomment{false}; // acq channel indiv comment
	BOOL bacqchsetting{false}; // acq chan indiv settings (gain, filter, etc)
	BOOL bunits{true}; // TRUE=mV, FALSE=binary
	BOOL bcontours{false}; // TRUE=contours, FALSE=center
	BOOL bgraphics{true}; // TRUE=graphics (enh metafile), FALSE=ASCII
	BOOL bdatabasecols{true}; // TRUE= export content of database columns
	BOOL btoExcel{false};

	int hzResolution{600}; // int item #1: horizontal size of exported data (points)
	int vtResolution{400}; // vertical size of exported data (points)
	int unitMode{0}; // provisional
	int nfilterindex{0}; // index list item in fileopen dialog m_nfilterindex

	// overview list parameters
#define NCOLS	11
	CUIntArray icolwidth{};
	int irowheight{50};

	// save scopescreen item
	int n_scope_items{0};
	SCOPESTRUCT viewdata{};
	SCOPESTRUCT viewspkdetectfiltered{};
	SCOPESTRUCT viewspkdetectdata{};
	SCOPESTRUCT viewspkdetectspk{};
	SCOPESTRUCT viewspkdetectbars{};
	SCOPESTRUCT viewadcontvi{};
	SCOPESTRUCT spkviewdata{};
	SCOPESTRUCT spkviewspk{};
	SCOPESTRUCT spkviewbars{};
	SCOPESTRUCT spksort1spk{};
	SCOPESTRUCT spksort1parms{};
	SCOPESTRUCT spksort1bars{};
	SCOPESTRUCT spksort1hist{};
	SCOPESTRUCT dbviewdata{};
	SCOPESTRUCT dbviewbars{};
	SCOPESTRUCT dbviewshape{};

	// splitter settings
	int col0width{200};
	int row0height{50};
	BOOL bcol0visible{true};
	BOOL brow0visible{true};

	// int parm - spike class
	int spikeclass{0};

	// database list view settings
	float tFirst{0.f};
	float tLast{0.f};
	float mVspan{0.f};
	int displaymode{1};
};
