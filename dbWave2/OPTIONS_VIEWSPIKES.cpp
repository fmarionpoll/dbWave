#include "StdAfx.h"
#include "OPTIONS_VIEWSPIKES.h"


IMPLEMENT_SERIAL(OPTIONS_VIEWSPIKES, CObject, 0 /* schema number*/)

OPTIONS_VIEWSPIKES::OPTIONS_VIEWSPIKES()
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

OPTIONS_VIEWSPIKES::~OPTIONS_VIEWSPIKES()
= default;

OPTIONS_VIEWSPIKES& OPTIONS_VIEWSPIKES::operator =(const OPTIONS_VIEWSPIKES& arg)
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

void OPTIONS_VIEWSPIKES::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << wversion;

		ar << timestart << timeend << timebin;
		ar << static_cast<WORD>(nbins);
		ar << static_cast<WORD>(classnb); // classnb2 is added to the series of int parameters
		WORD wE, wM;

		wM = 1;
		wE = bacqcomments * wM; //1
		wM += wM;
		wE += bacqdate * wM; //2
		wM += wM;
		wE += bacqcomments * wM; //4
		wM += wM;
		wE += bacqdate * wM; //8
		wM += wM;
		wE += bacqchsettings * wM; //16
		wM += wM;
		wE += bspkcomments * wM; //32
		wM += wM;
		wE += bincrflagonsave * wM; //64
		wM += wM;
		wE += bexportzero * wM; //128
		wM += wM;
		wE += bexportPivot * wM; //256
		wM += wM;
		wE += bexporttoExcel * wM; //512	empty slot
		wM += wM;
		wE += bartefacts * wM; //1024
		wM += wM;
		wE += bcolumnheader * wM; //2048
		wM += wM;
		wE += btotalspikes * wM; //4096
		wM += wM;
		wE += babsolutetime * wM; //8192
		wM += wM;
		wE += ballfiles * wM; //16384
		wM += wM;
		wE += bdotunderline * wM; //32768
		ar << wE;

		// int parameters
		WORD wIntParms = 11;
		ar << wIntParms; // set to nb of following lines
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
		wM = 1;
		wE = bdisplayBars * wM;
		wnb++; // 1
		wM += wM;
		wE += bdisplayShapes * wM;
		wnb++; // 2: 2
		wM += wM;
		wE += bsplitClasses * wM;
		wnb++; // 3: 4
		wM += wM;
		wE += bYmaxAuto * wM;
		wnb++; // 4: 8
		wM += wM;
		wE += bCycleHist * wM;
		wnb++; // 5: 16
		wM += wM;
		wE += ballChannels * wM;
		wnb++; // 6: 32
		ar << wnb; // set to nb of bool parms
		ar << wE; // set compressed BOOL data

		//float parameters or 32 bits values
		WORD wnparms = 29;
		ar << wnparms;
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
		WORD wE;
		WORD wM = 1;
		ar >> wE;
		wM = 1;
		bacqcomments = ((wE & wM) != 0); //1
		wM += wM;
		bacqdate = ((wE & wM) != 0); //2
		wM += wM;
		bacqcomments = ((wE & wM) != 0); //4
		wM += wM;
		bacqdate = ((wE & wM) != 0); //8
		wM += wM;
		bacqchsettings = ((wE & wM) != 0); //16
		wM += wM;
		bspkcomments = ((wE & wM) != 0); //32
		wM += wM;
		bincrflagonsave = ((wE & wM) != 0); //64
		// this parm does not belong to print options but to export options
		wM += wM;
		bexportzero = ((wE & wM) > 0); //128
		wM += wM;
		bexportPivot = ((wE & wM) > 0); //256
		wM += wM;
		bexporttoExcel = ((wE & wM) != 0); //512

		wM += wM;
		bartefacts = ((wE & wM) != 0); //1024
		wM += wM;
		bcolumnheader = ((wE & wM) != 0); //2048
		wM += wM;
		btotalspikes = ((wE & wM) != 0); //4096
		wM += wM;
		babsolutetime = ((wE & wM) != 0); //8192
		wM += wM;
		ballfiles = ((wE & wM) != 0); //16384
		wM += wM;
		bdotunderline = ((wE & wM) != 0); //32768

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
		ar >> wE; // data -> decompress
		// version 1 (20-4-96): 3 parameters
		wM = 1;
		bdisplayBars = ((wE & wM) > 0);
		nb--; // 1
		wM += wM;
		bdisplayShapes = ((wE & wM) > 0);
		nb--; // 2:2
		wM += wM;
		bsplitClasses = ((wE & wM) > 0);
		nb--; // 3:4
		if (nb > 0)
		{
			wM += wM;
			bYmaxAuto = ((wE & wM) > 0);
			nb--;
		} // 4:8
		if (nb > 0)
		{
			wM += wM;
			bCycleHist = ((wE & wM) > 0);
			nb--;
		} // 5:16
		if (nb > 0)
		{
			wM += wM;
			ballChannels = ((wE & wM) > 0);
			nb--;
		} // 6:32
		while (nb > 0)
		{
			ar >> wE;
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
			COLORREF crdummy;
			for (int i = 0; i < 18; i++)
				//if (nb > 0) {ar >> crScale[i]; nb--;}
				if (nb > 0)
				{
					ar >> crdummy;
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
