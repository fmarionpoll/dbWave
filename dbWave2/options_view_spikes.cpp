#include "StdAfx.h"
#include "options_view_spikes.h"

#include "ColorNames.h"


IMPLEMENT_SERIAL(options_view_spikes, CObject, 0 /* schema number*/)

options_view_spikes::options_view_spikes()
{
	int i = 18;
	i--;
	crScale[i] = RGB(35, 31, 28); //RGB(255,255,255);
	i--;
	crScale[i] = RGB(213, 45, 31);
	i--;
	crScale[i] = RGB(222, 100, 19);
	i--;
	crScale[i] = RGB(232, 142, 33);
	i--;
	crScale[i] = RGB(243, 196, 0);
	i--;
	crScale[i] = RGB(226, 237, 0);
	i--;
	crScale[i] = RGB(142, 201, 33);
	i--;
	crScale[i] = RGB(64, 174, 53);
	i--;
	crScale[i] = RGB(0, 152, 62);
	i--;
	crScale[i] = RGB(0, 142, 85);
	i--;
	crScale[i] = RGB(0, 142, 123);
	i--;
	crScale[i] = RGB(0, 142, 163);
	i--;
	crScale[i] = RGB(0, 139, 206);
	i--;
	crScale[i] = RGB(0, 117, 190);
	i--;
	crScale[i] = RGB(0, 90, 158);
	i--;
	crScale[i] = RGB(29, 62, 133);
	i--;
	crScale[i] = RGB(49, 20, 105);
	i--;
	crScale[i] = RGB(255, 255, 255); //RGB(35,31,28);
	ASSERT(i >= 0);
}

options_view_spikes::~options_view_spikes()
= default;

options_view_spikes& options_view_spikes::operator =(const options_view_spikes& arg)
{
	if (this != &arg)
	{
		timestart = arg.timestart;
		timeend = arg.timeend;
		timebin = arg.timebin;
		histampl_vmax = arg.histampl_vmax;
		histampl_vmin = arg.histampl_vmin;
		histampl_nbins = arg.histampl_nbins;

		nbins = arg.nbins;
		classnb = arg.classnb;
		classnb2 = arg.classnb2;
		bacqcomments = arg.bacqcomments;
		bacqdate = arg.bacqdate;
		bacqchsettings = arg.bacqchsettings;
		bspkcomments = arg.bspkcomments;
		exportdatatype = arg.exportdatatype;
		spikeclassoption = arg.spikeclassoption;
		bartefacts = arg.bartefacts;
		bcolumnheader = arg.bcolumnheader;
		btotalspikes = arg.btotalspikes;
		babsolutetime = arg.babsolutetime;
		bexportzero = arg.bexportzero;
		ballChannels = arg.ballChannels;

		heightLine = arg.heightLine;
		bdisplayBars = arg.bdisplayBars;
		bdisplayShapes = arg.bdisplayShapes;
		heightSeparator = arg.heightSeparator;
		bsplitClasses = arg.bsplitClasses;
		ballfiles = arg.ballfiles;
		dotheight = arg.dotheight;
		dotlineheight = arg.dotlineheight;
		bdotunderline = arg.bdotunderline;
		nbinsISI = arg.nbinsISI;
		binISI = arg.binISI;
		bYmaxAuto = arg.bYmaxAuto;
		Ymax = arg.Ymax;

		crHistFill = arg.crHistFill;
		crHistBorder = arg.crHistBorder;
		crStimFill = arg.crStimFill;
		crStimBorder = arg.crStimBorder;
		crChartArea = arg.crChartArea;
		for (int i = 0; i < 18; i++)
			crScale[i] = arg.crScale[i];
		fScalemax = arg.fScalemax;
		istimulusindex = arg.istimulusindex;
		bhistType = arg.bhistType;
		bCycleHist = arg.bCycleHist;
		nstipercycle = arg.nstipercycle;
	}
	return *this;
}

void options_view_spikes::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << wversion;

		ar << timestart << timeend << timebin;
		ar << static_cast<WORD>(nbins);
		ar << static_cast<WORD>(classnb); // classnb2 is added to the series of int parameters
		WORD w_e, w_m;

		w_m = 1;
		w_e = bacqcomments * w_m; //1
		w_m += w_m;
		w_e += bacqdate * w_m; //2
		w_m += w_m;
		w_e += bacqcomments * w_m; //4
		w_m += w_m;
		w_e += bacqdate * w_m; //8
		w_m += w_m;
		w_e += bacqchsettings * w_m; //16
		w_m += w_m;
		w_e += bspkcomments * w_m; //32
		w_m += w_m;
		w_e += bincrflagonsave * w_m; //64
		w_m += w_m;
		w_e += bexportzero * w_m; //128
		w_m += w_m;
		w_e += bexportPivot * w_m; //256
		w_m += w_m;
		w_e += bexporttoExcel * w_m; //512	empty slot
		w_m += w_m;
		w_e += bartefacts * w_m; //1024
		w_m += w_m;
		w_e += bcolumnheader * w_m; //2048
		w_m += w_m;
		w_e += btotalspikes * w_m; //4096
		w_m += w_m;
		w_e += babsolutetime * w_m; //8192
		w_m += w_m;
		w_e += ballfiles * w_m; //16384
		w_m += w_m;
		w_e += bdotunderline * w_m; //32768
		ar << w_e;

		// int parameters
		WORD w_int = 11;
		ar << w_int; // set to nb of following lines
		ar << static_cast<WORD>(heightLine); // 1
		ar << static_cast<WORD>(heightSeparator); // 2
		ar << static_cast<WORD>(dotheight); // 3
		ar << static_cast<WORD>(dotlineheight); // 4
		ar << static_cast<WORD>(nbinsISI); // 5
		ar << static_cast<WORD>(istimulusindex); // 6
		ar << static_cast<WORD>(bhistType); // 7
		ar << nstipercycle; // 8
		ar << exportdatatype; // 9
		ar << spikeclassoption; // 10
		ar << classnb2; // 11

		WORD wnb = 0;
		w_m = 1;
		w_e = bdisplayBars * w_m;
		wnb++; // 1
		w_m += w_m;
		w_e += bdisplayShapes * w_m;
		wnb++; // 2: 2
		w_m += w_m;
		w_e += bsplitClasses * w_m;
		wnb++; // 3: 4
		w_m += w_m;
		w_e += bYmaxAuto * w_m;
		wnb++; // 4: 8
		w_m += w_m;
		w_e += bCycleHist * w_m;
		wnb++; // 5: 16
		w_m += w_m;
		w_e += ballChannels * w_m;
		wnb++; // 6: 32
		ar << wnb; // set to nb of bool parameters
		ar << w_e; // set compressed BOOL data

		//float parameters or 32 bits values
		WORD w_n_float = 29;
		ar << w_n_float;
		ar << histampl_vmax; // 1
		ar << histampl_vmin; // 2
		ar << histampl_nbins; // 3
		ar << binISI; // 4
		ar << Ymax; // 5
		ar << crHistFill; // 6
		ar << crHistBorder; // 7
		ar << crStimFill; // 8
		ar << crStimBorder; // 9
		ar << crChartArea; // 10
		for (unsigned long i : crScale)
			ar << i;
		ar << fScalemax; // 29
	}
	else
	{
		ar >> wversion;

		WORD w1;
		ar >> timestart >> timeend >> timebin;
		ar >> w1;
		nbins = w1;
		ar >> w1;
		classnb = w1;

		// print options
		WORD w_e;
		WORD w_m = 1;
		ar >> w_e;
		w_m = 1;
		bacqcomments = ((w_e & w_m) != 0); //1
		w_m += w_m;
		bacqdate = ((w_e & w_m) != 0); //2
		w_m += w_m;
		bacqcomments = ((w_e & w_m) != 0); //4
		w_m += w_m;
		bacqdate = ((w_e & w_m) != 0); //8
		w_m += w_m;
		bacqchsettings = ((w_e & w_m) != 0); //16
		w_m += w_m;
		bspkcomments = ((w_e & w_m) != 0); //32
		w_m += w_m;
		bincrflagonsave = ((w_e & w_m) != 0); //64
		// this parameter does not belong to print options but to export options
		w_m += w_m;
		bexportzero = ((w_e & w_m) > 0); //128
		w_m += w_m;
		bexportPivot = ((w_e & w_m) > 0); //256
		w_m += w_m;
		bexporttoExcel = ((w_e & w_m) != 0); //512

		w_m += w_m;
		bartefacts = ((w_e & w_m) != 0); //1024
		w_m += w_m;
		bcolumnheader = ((w_e & w_m) != 0); //2048
		w_m += w_m;
		btotalspikes = ((w_e & w_m) != 0); //4096
		w_m += w_m;
		babsolutetime = ((w_e & w_m) != 0); //8192
		w_m += w_m;
		ballfiles = ((w_e & w_m) != 0); //16384
		w_m += w_m;
		bdotunderline = ((w_e & w_m) != 0); //32768

		// int parameters
		int nb;
		ar >> w1;
		nb = w1; // number of int parameters
		ar >> w1;
		heightLine = w1;
		nb--; // 1
		ar >> w1;
		heightSeparator = w1;
		nb--; // 2
		if (nb > 0)
		{
			ar >> w1;
			dotheight = w1;
			nb--;
		} // 3
		if (nb > 0)
		{
			ar >> w1;
			dotlineheight = w1;
			nb--;
		} // 4
		if (nb > 0)
		{
			ar >> w1;
			nbinsISI = w1;
			nb--;
		} // 5
		if (nb > 0)
		{
			ar >> w1;
			istimulusindex = w1;
			nb--;
		} // 6
		if (nb > 0)
		{
			ar >> w1;
			bhistType = w1;
			nb--;
		} // 7
		if (nb > 0)
		{
			ar >> nstipercycle;
			nb--;
		} // 8
		if (nb > 0)
		{
			ar >> exportdatatype;
			nb--;
		} // 9
		if (nb > 0)
		{
			ar >> spikeclassoption;
			nb--;
		} // 10
		if (nb > 0)
		{
			ar >> classnb2;
			nb--;
		}
		else { classnb2 = classnb; } // 11
		while (nb > 0)
		{
			ar >> w1;
			nb--;
		}

		ar >> w1;
		nb = w1; // number of bool parameters
		ar >> w_e; // data -> decompress
		// version 1 (20-4-96): 3 parameters
		w_m = 1;
		bdisplayBars = ((w_e & w_m) > 0);
		nb--; // 1
		w_m += w_m;
		bdisplayShapes = ((w_e & w_m) > 0);
		nb--; // 2:2
		w_m += w_m;
		bsplitClasses = ((w_e & w_m) > 0);
		nb--; // 3:4
		if (nb > 0)
		{
			w_m += w_m;
			bYmaxAuto = ((w_e & w_m) > 0);
			nb--;
		} // 4:8
		if (nb > 0)
		{
			w_m += w_m;
			bCycleHist = ((w_e & w_m) > 0);
			nb--;
		} // 5:16
		if (nb > 0)
		{
			w_m += w_m;
			ballChannels = ((w_e & w_m) > 0);
			nb--;
		} // 6:32
		while (nb > 0)
		{
			ar >> w_e;
			nb--;
		}

		if (wversion > 2)
		{
			ar >> w1;
			nb = w1;
			ar >> histampl_vmax;
			nb--;
			ar >> histampl_vmin;
			nb--;
			ar >> histampl_nbins;
			nb--;
			if (nb > 0)
			{
				ar >> binISI;
				nb--;
			}
			if (nb > 0)
			{
				ar >> Ymax;
				nb--;
			}
			if (nb > 0)
			{
				ar >> crHistFill;
				nb--;
			}
			if (nb > 0)
			{
				ar >> crHistBorder;
				nb--;
			}
			if (nb > 0)
			{
				ar >> crStimFill;
				nb--;
			}
			if (nb > 0)
			{
				ar >> crStimBorder;
				nb--;
			}
			if (nb > 0)
			{
				ar >> crChartArea;
				nb--;
			}
			COLORREF cr_dummy;
			for (int i = 0; i < 18; i++)
				//if (nb > 0) {ar >> crScale[i]; nb--;}
				if (nb > 0)
				{
					ar >> cr_dummy;
					nb--;
				}
			if (nb > 0)
			{
				ar >> fScalemax;
				nb--;
			}
		}
	}
}
