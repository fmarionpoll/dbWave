#pragma once


#define EXPORT_PSTH			0
#define EXPORT_ISI			1
#define EXPORT_AUTOCORR		2
#define EXPORT_LATENCY		3
#define EXPORT_AVERAGE		4
#define EXPORT_INTERV		5
#define EXPORT_EXTREMA		6
#define EXPORT_AMPLIT		7
#define EXPORT_HISTAMPL		8
#define EXPORT_SPIKEPOINTS	9

class options_view_spikes final : public CObject
{
	DECLARE_SERIAL(options_view_spikes)
	options_view_spikes();
	~options_view_spikes() override;
	options_view_spikes& operator =(const options_view_spikes& arg);
	void Serialize(CArchive& ar) override;

public:
	BOOL b_changed{false}; // flag set TRUE if contents has changed
	WORD wversion{3}; // version number
	int ichan{0}; // data channel from which we have detected spikes

	float timestart{0.f}; // interval definition
	float timeend{2.f}; //
	float timebin{.1f}; // bin size (sec)
	float histampl_vmax{1.f};
	float histampl_vmin{0.f};

	int histampl_nbins{20};
	int nbins{20}; // number of bins
	int classnb{0}; // class nb
	int classnb2{1};
	int istimulusindex{0};
	int exportdatatype{0}; // export 0=psth, 1=isi, 2=autocorr, 3=intervals, 4=extrema, 5=max-min, 6=hist ampl
	int spikeclassoption{0}; // spike class: -1(one:selected){}; 0(all){}; 1(all:splitted)

	BOOL bacqcomments{true}; // source data comments
	BOOL bacqdate{true}; // source data time and date
	BOOL bacqchsettings{false}; // source data acq chan settings
	BOOL bspkcomments{false}; // spike file comments
	BOOL ballfiles{false}; // export number of spikes / interval
	BOOL bexportzero{false}; // when exporting histogram, export zero (true) or empty cell (false)
	BOOL bexportPivot{false}; // when exporting to excel, export pivot or not
	BOOL bexporttoExcel{false}; // export to Excel (1/0)
	BOOL bartefacts{true}; // eliminate artefacts (class < 0)
	BOOL bcolumnheader{true}; // column headers ON/OFF
	BOOL btotalspikes{true}; // total nb of spikes
	BOOL babsolutetime{true}; // TRUE= absolute spk time, FALSE=relative to first stim
	BOOL bincrflagonsave{false}; // increment database flag when spike file is saved
	BOOL bSpikeDetectThreshold{true}; // spike detection threshold

	int ncommentcolumns{1}; // number of comment columns in the table exported to excel

	// print parameters
	int heightLine{130}; // height of one line
	int heightSeparator{20}; // separator height betw classes
	BOOL bdisplayBars{true}; // default(TRUE)
	BOOL bdisplayShapes{true}; // default(TRUE)
	BOOL bsplitClasses{true}; // separate classes
	BOOL ballChannels{true}; // all spike channels (FALSE: only current)

	// histogram and dot display
	int dotheight{3};
	int dotlineheight{5};
	BOOL bdotunderline{false};
	int nbinsISI{100};
	float binISI{2.f};
	BOOL bYmaxAuto{true};
	float Ymax{1.f};

	COLORREF crHistFill{RGB(0x80, 0x80, 0x80)};
	COLORREF crHistBorder{RGB(0x80, 0x80, 0x80)};
	COLORREF crStimFill{RGB(117, 192, 239)};
	COLORREF crStimBorder{RGB(117, 192, 239)};
	COLORREF crChartArea{RGB(255, 255, 255)};
	COLORREF crScale[18]{};

	float fScalemax{100.f};
	int bhistType{0}; // type of histogram requested (PSTH, ISI, ...)
	BOOL bCycleHist{false}; // cycle histogram (TRUE/FALSE), default=FALSE
	int nstipercycle{1}; // n stimuli per cycle
};
