// awavepar.cpp    implementation file
//
// this file defines structures used to store infos related to
// user choices ie for spike detection, sorting, measuring,
// and printing. These structures are read from awave.par by
// awave32.cpp during first initialisation, & stored at exit.
// The name of this parameter file and its location can be changed
// from within a dialog box (options)
//
// SCOPESTRUCT
// OPTIONS_ACQDATA
// OPTIONS_VIEWSPIKES
// OPTIONS_VIEWDATAMEASURE
// OPTIONS_IMPORT
//
//////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "dataheader_Atlab.h"
#include <Olxdadefs.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//------------------ class SCOPESTRUCT ---------------------------------

IMPLEMENT_SERIAL(SCOPESTRUCT, CObject, 0 /* schema number*/ )

SCOPESTRUCT::SCOPESTRUCT()
{
	wversion = 2;
	iID=0;
	iXCells=0;
	iYCells=0;
	iXTicks=0;
	iYTicks=0;
	iXTickLine=0;
	iYTickLine=0;
	crScopeFill=RGB(255,255,255);
	crScopeGrid=RGB(255,255,255);
	bDrawframe=TRUE;
	xScaleUnitValue = 0.0f;
	yScaleUnitValue = 0.0f;
	yScaleSpan_v = 0.0f;
	xScaleSpan_s = 0.0f;
}

SCOPESTRUCT::~SCOPESTRUCT()
{
}

void SCOPESTRUCT::operator = (const SCOPESTRUCT& arg)
{
	iID=arg.iID;
	iXCells=arg.iXCells;
	iYCells=arg.iYCells;
	iXTicks=arg.iXTicks;
	iYTicks=arg.iYTicks;
	iXTickLine=arg.iXTickLine;
	iYTickLine=arg.iYTickLine;
	crScopeFill=arg.crScopeFill;
	crScopeGrid=arg.crScopeGrid;
	bDrawframe=arg.bDrawframe;
	xScaleUnitValue = arg.xScaleUnitValue;
	yScaleUnitValue = arg.yScaleUnitValue;
}

void SCOPESTRUCT::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		wversion = 2;
		ar << wversion;
		int nints = 8;
		ar << nints;
		ar << iID;
		ar << iXCells;
		ar << iYCells;
		ar << iXTicks;
		ar << iYTicks;
		ar << iXTickLine;
		ar << iYTickLine;
		ar << bDrawframe;
		int ncolor_items = 2;
		ar << ncolor_items;
		ar << crScopeFill;
		ar << crScopeGrid;
		// save scale values
		int nfloats = 2;
		ar << nfloats;
		ar << xScaleUnitValue;
		ar << yScaleUnitValue;
	} 
	else
	{
		WORD wversion;  ar >> wversion;
		int nints; ar >> nints;
		ar >> iID; nints--;
		ar >> iXCells; nints--;
		ar >> iYCells; nints--;
		ar >> iXTicks; nints--;
		ar >> iYTicks; nints--;
		ar >> iXTickLine; nints--;
		ar >> iYTickLine; nints--;
		ar >> bDrawframe; nints--;
		ASSERT(nints==0);

		int ncolor_items; ar >> ncolor_items;
		ar >> crScopeFill; ncolor_items--;
		ar >> crScopeGrid; ncolor_items--;
		ASSERT(ncolor_items==0);

		if (wversion > 1)
		{
			int nfloats; ar >> nfloats;
			ar >> xScaleUnitValue; nfloats--;
			ar >> yScaleUnitValue; nfloats--;
			while (nfloats > 0)
			{
				float x;
				ar >> x; nfloats--;
			}
		}
	}
}

									   
//------------------ class OPTIONS_VIEWDATA ---------------------------------

IMPLEMENT_SERIAL(OPTIONS_VIEWDATA, CObject, 0 /* schema number*/ )

OPTIONS_VIEWDATA::OPTIONS_VIEWDATA()
{
	bChanged = FALSE;
	m_wversion = 5;

	// print options
	bAcqComment=TRUE;		// print data acquisition comments
	bAcqDateTime=TRUE;		// data acquisition date and time
	bChansComment=TRUE;		// individual data acq channel description
	bChanSettings=TRUE;		// amplifier's settings (CyberAmp conventions)
	bDocName=TRUE;			// file name
	bFrameRect=FALSE;		// draw a rectangle frame around the data
	bClipRect=TRUE;			// clip data to the rectangle
	bTimeScaleBar=TRUE;		// display time scale bar
	bVoltageScaleBar=TRUE;	// display voltage scale bar
	bPrintSelection=TRUE;
	bPrintSpkBars=TRUE;
	bFilterDataSource=TRUE;

	// print parameters	
	HeightDoc=300;			// height of one document
	WidthDoc=1500;			// width of one document
	leftPageMargin=100;		// page left margin (pixels)
	bottomPageMargin=40;	// bottom
	rightPageMargin=40;		// right
	topPageMargin=100;		// top
	heightSeparator=40;		// vertical space between consecutive displays
	textseparator=40;		// space between left margin of display and text
	fontsize=40;			// font size (pixels)

	// how data are displayed from file to file	 [ stored in a word wBrowseflags]
	bAllChannels=TRUE;		// display all channels
	bEntireRecord=TRUE;		// display entire record
	bMultirowDisplay=FALSE;	// display one record on consecutive lines
	bSplitCurves=TRUE;		// arrange curves so that they are split vertically
	bMaximizeGain=TRUE;		// maximize gain
	bCenterCurves=TRUE;		// center curves
	bKeepForEachFile=TRUE;	// keep parameters for each file
	bDispOldDetectP=FALSE;	// display detection parms stored within file
	bFilterDat=FALSE;		// display data filtered or not
	bDetectWhileBrowse=TRUE;// detect spikes if browse and no spike file
	bGotoRecordID=TRUE;		// goto Record ID instead of Record Absolute Pos
	bDisplayFileName=FALSE;	// display file name in individual data widows of dbwaveview

	// export textual infos from data files [stored in a word wInfoflags]
	bacqcomments=TRUE;		// global comment
	bacqdate=TRUE;			// acquisition date
	bacqtime=TRUE;			// acquisition time
	bfilesize=FALSE;		// file size
	bacqchcomment=FALSE;	// acq channel indiv comment
	bacqchsetting=FALSE;	// acq chan indiv settings (gain, filter, etc)
	bunits=TRUE;			// TRUE=mV, FALSE=binary
	bcontours=FALSE;		// TRUE=contours, FALSE=center
	bgraphics=TRUE;			// TRUE=graphics (enh metafile), FALSE=ASCII
	bdatabasecols = TRUE;	// TRUE = export database infos
	btoExcel=FALSE;

	// nb items
	hzResolution=600;
	vtResolution=400;
	unitMode= 0;
	spkheight=0;
	spkwidth=0;
	icolwidth.SetSize(NCOLS);
	int i= 0;
	icolwidth[i] = 10;
	i++; icolwidth[i]=410;
	i++; icolwidth[i]=16;
	i++; icolwidth[i]=30;
	i++; icolwidth[i]=30;
	i++; icolwidth[i]=30;
	i++; icolwidth[i]=30;
	i++; icolwidth[i]=50;
	i++; icolwidth[i]=40;
	i++; icolwidth[i]=40;
	i++; icolwidth[i]=10;

	irowheight = 50;

	// splitters parameters
	col0width = 200;
	row0height = 50;
	bcol0visible = TRUE;
	brow0visible = TRUE;

	// database view parameters
	tFirst=0.f;
	tLast=0.f;
	mVspan=0.f;
	displaymode=1;
	bsetTimeSpan=FALSE;
	bsetmVSpan=FALSE;
	bDisplayAllClasses=TRUE;
	spikeclass=0;
}

OPTIONS_VIEWDATA::~OPTIONS_VIEWDATA()
{
}

void OPTIONS_VIEWDATA::operator = (const OPTIONS_VIEWDATA& arg)
{
	horzRes				= arg.horzRes;
	vertRes				= arg.vertRes;
	bAcqComment			= arg.bAcqComment;
	bAcqDateTime		= arg.bAcqDateTime;
	bChansComment		= arg.bChansComment;
	bChanSettings		= arg.bChanSettings;
	bDocName			= arg.bDocName;
	bFrameRect			= arg.bFrameRect;
	bClipRect			= arg.bClipRect;
	bTimeScaleBar		= arg.bTimeScaleBar;
	bVoltageScaleBar	= arg.bVoltageScaleBar;
	HeightDoc			= arg.HeightDoc;
	WidthDoc			= arg.WidthDoc;
	leftPageMargin		= arg.leftPageMargin;
	bottomPageMargin	= arg.bottomPageMargin;
	rightPageMargin		= arg.rightPageMargin;
	topPageMargin		= arg.topPageMargin;
	heightSeparator		= arg.heightSeparator;
	textseparator		= arg.textseparator;
	fontsize			= arg.fontsize;
	bAllChannels		= arg.bAllChannels;
	bEntireRecord		= arg.bEntireRecord;
	bMultirowDisplay	= arg.bMultirowDisplay;
	bSplitCurves		= arg.bSplitCurves;
	bMaximizeGain		= arg.bMaximizeGain;
	bCenterCurves		= arg.bCenterCurves;
	bKeepForEachFile	= arg.bKeepForEachFile;
	bDispOldDetectP		= arg.bDispOldDetectP;
	bFilterDataSource	= arg.bFilterDataSource;
	bacqcomments		= arg.bacqcomments;
	bacqdate			= arg.bacqdate;
	bacqtime			= arg.bacqtime;
	bfilesize			= arg.bfilesize;
	bacqchcomment		= arg.bacqchcomment;
	bacqchsetting		= arg.bacqchsetting;
	bPrintSelection		= arg.bPrintSelection;
	bPrintSpkBars		= arg.bPrintSpkBars;
	bunits				= arg.bunits;
	bcontours			= arg.bcontours;
	bgraphics			= arg.bgraphics;
	bdatabasecols		= arg.bdatabasecols;
	btoExcel			= arg.btoExcel;
	hzResolution		= arg.hzResolution;
	vtResolution		= arg.vtResolution;
	unitMode			= arg.unitMode;
	spkheight			= arg.spkheight;
	spkwidth			= arg.spkwidth;
	viewdata			= arg.viewdata;
	viewspkdetectfiltered = arg.viewspkdetectfiltered;
	viewspkdetectdata	= arg.viewspkdetectdata;
	viewspkdetectspk	= arg.viewspkdetectspk;
	viewspkdetectbars	= arg.viewspkdetectbars;
	viewadcontvi		= arg.viewadcontvi;
	spkviewdata			= arg.spkviewdata;
	spkviewspk			= arg.spkviewspk;
	spkviewbars			= arg.spkviewbars;
	spksort1spk			= arg.spksort1spk;
	spksort1parms		= arg.spksort1parms;
	spksort1bars		= arg.spksort1bars;
	spksort1hist		= arg.spksort1hist;
	dbviewdata			= arg.dbviewdata;
	bFilterDat			= arg.bFilterDat;
	dbviewbars			= arg.dbviewbars;
	dbviewshape			= arg.dbviewshape;
	nfilterindex		= arg.nfilterindex;
	bDetectWhileBrowse	= arg.bDetectWhileBrowse;
	bGotoRecordID		= arg.bGotoRecordID;
	col0width			= arg.col0width;
	row0height			= arg.row0height;
	bcol0visible		= arg.bcol0visible;
	brow0visible		= arg.brow0visible;
	irowheight			= arg.irowheight;
	tFirst				= arg.tFirst;
	tLast				= arg.tLast;
	mVspan				= arg.mVspan;
	displaymode			= arg.displaymode;
	bDisplayFileName	= arg.bDisplayFileName;
	bsetTimeSpan		= arg.bsetTimeSpan;
	bsetmVSpan			= arg.bsetmVSpan;
	bDisplayAllClasses	= arg.bDisplayAllClasses;
	spikeclass			= arg.spikeclass;
}

void OPTIONS_VIEWDATA::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << m_wversion;

		// print options
		WORD wPrintFlags, wMult;
		wMult =1;		wPrintFlags =  bAcqComment		* wMult;	//0
		wMult += wMult; wPrintFlags += bAcqDateTime		* wMult;	//2
		wMult += wMult; wPrintFlags += bChansComment	* wMult;	//4
		wMult += wMult; wPrintFlags += bChanSettings	* wMult;	//8
		wMult += wMult; wPrintFlags += bDocName			* wMult;	//16
		wMult += wMult; wPrintFlags += bFrameRect		* wMult;	//32
		wMult += wMult; wPrintFlags += bClipRect		* wMult;	//64
		wMult += wMult; wPrintFlags += bTimeScaleBar	* wMult;	//128
		wMult += wMult; wPrintFlags += bVoltageScaleBar	* wMult;	//256
		wMult += wMult; wPrintFlags += bPrintSelection  * wMult;	//512
		wMult += wMult; wPrintFlags += bPrintSpkBars	* wMult;	//1024
		wMult += wMult; wPrintFlags += bFilterDataSource* wMult;	//2048
		ar << wPrintFlags;

		// print parameters	
		ar << (WORD) horzRes;
		ar << (WORD) vertRes;		
		ar << (WORD) HeightDoc;			// height of one document
		ar << (WORD) WidthDoc;			// width of one document
		ar << (WORD) leftPageMargin;	// page left margin (pixels)
		ar << (WORD) bottomPageMargin;	// bottom
		ar << (WORD) rightPageMargin;	// right
		ar << (WORD) topPageMargin;		// top
		ar << (WORD) heightSeparator;	// vertical space between consecutive displays
		ar << (WORD) textseparator;		// space between left margin of display and text
		ar << (WORD) fontsize;			// font size (pixels)

		// how data are displayed from file to file
		WORD wBrowseFlags;
		wMult =1;		wBrowseFlags  = bAllChannels	* wMult;	//0
		wMult += wMult; wBrowseFlags += bEntireRecord	* wMult;	//2
		wMult += wMult; wBrowseFlags += bMultirowDisplay* wMult;	//4
		wMult += wMult; wBrowseFlags += bSplitCurves	* wMult;	//8
		wMult += wMult; wBrowseFlags += bMaximizeGain	* wMult;	//16
		wMult += wMult; wBrowseFlags += bCenterCurves	* wMult;	//32
		wMult += wMult; wBrowseFlags += bKeepForEachFile* wMult;	//64
		wMult += wMult; wBrowseFlags += bDispOldDetectP * wMult;	//128
		wMult += wMult; wBrowseFlags += bFilterDat		* wMult;	//256
		wMult += wMult; wBrowseFlags += bDetectWhileBrowse* wMult;	//512
		wMult += wMult; wBrowseFlags += bGotoRecordID	* wMult;	//1024
		wMult += wMult; wBrowseFlags += bDisplayFileName* wMult;	//2048
		wMult += wMult; wBrowseFlags += bsetTimeSpan	* wMult;	//4096
		wMult += wMult; wBrowseFlags += bsetmVSpan		* wMult;	//8192
		wMult += wMult; wBrowseFlags += bDisplayAllClasses * wMult;	//16384
		
		ar << wBrowseFlags;

		// export infos from data files
		WORD wInfoFlags;
		wMult =1;		wInfoFlags =  bacqcomments	* wMult;		// 0
		wMult += wMult; wInfoFlags += bacqdate		* wMult;		// 2
		wMult += wMult; wInfoFlags += bacqtime		* wMult;		// 4
		wMult += wMult; wInfoFlags += bfilesize		* wMult;		// 8
		wMult += wMult; wInfoFlags += bacqchcomment	* wMult;		// 16
		wMult += wMult; wInfoFlags += bacqchsetting	* wMult;		// 32
		wMult += wMult; wInfoFlags += bunits		* wMult;		// 64
		wMult += wMult; wInfoFlags += bcontours		* wMult;		// 128
		wMult += wMult; wInfoFlags += bgraphics		* wMult;		// 256
		wMult += wMult; wInfoFlags += btoExcel      * wMult;		// 512
		wMult += wMult; wInfoFlags += bdatabasecols	* wMult;		// 1024
		ar << wInfoFlags;

		// export int(s) WORD = nb of items
		ar << (WORD) (13);
		ar << hzResolution;	//1
		ar << vtResolution;	//2
		ar << unitMode;		//3
		ar << spkheight;	//4
		ar << spkwidth;		//5
		ar << nfilterindex;	//6
		ar << col0width;	//7
		ar << row0height;	//8
		ar << bcol0visible;	//9
		ar << brow0visible;	//10
		ar << irowheight;	//11
		ar << displaymode;	//12
		ar << spikeclass;	//13

		ar << icolwidth.GetSize(); 
		for (int ii=0; ii< icolwidth.GetSize(); ii++)
			ar << icolwidth[ii];

		ar << (int) 3;		// nb floats
		ar << tFirst;
		ar << tLast;
		ar << mVspan;

		nscopeitems = 16; ar << nscopeitems;
		viewdata.Serialize(ar);				//1
		viewspkdetectdata.Serialize(ar);	//2
		viewspkdetectspk.Serialize(ar);		//3
		viewspkdetectbars.Serialize(ar);	//4
		viewadcontvi.Serialize(ar);			//5
		spkviewdata.Serialize(ar);			//6
		spkviewspk.Serialize(ar);			//7
		spkviewbars.Serialize(ar);			//8
		spksort1spk.Serialize(ar);			//9
		spksort1parms.Serialize(ar);		//10
		spksort1hist.Serialize(ar);			//11
		dbviewdata.Serialize(ar);			//12
		dbviewbars.Serialize(ar);			//13
		dbviewshape.Serialize(ar);			//14
		spksort1bars.Serialize(ar);			//15
		viewspkdetectfiltered.Serialize(ar);//16
	} 
	else
	{
		WORD version;  ar >> version;

		// print options
		WORD wPrintFlags, wMult;
		ar >> wPrintFlags;		
		wMult = 1; 		bAcqComment       = ((wPrintFlags & wMult)>0);
		wMult += wMult;	bAcqDateTime      = ((wPrintFlags & wMult)>0);
		wMult += wMult;	bChansComment     = ((wPrintFlags & wMult)>0);
		wMult += wMult;	bChanSettings     = ((wPrintFlags & wMult)>0);
		wMult += wMult;	bDocName          = ((wPrintFlags & wMult)>0);
		wMult += wMult;	bFrameRect        = ((wPrintFlags & wMult)>0);
		wMult += wMult;	bClipRect         = ((wPrintFlags & wMult)>0);
		wMult += wMult;	bTimeScaleBar     = ((wPrintFlags & wMult)>0);
		wMult += wMult;	bVoltageScaleBar  = ((wPrintFlags & wMult)>0);
		wMult += wMult; bPrintSelection   = ((wPrintFlags & wMult)>0);
		wMult += wMult; bPrintSpkBars	  = ((wPrintFlags & wMult)>0);
		wMult += wMult; bFilterDataSource = ((wPrintFlags & wMult)>0);

		// print parameters	
		WORD w1;
		ar >> w1; horzRes=w1;
		ar >> w1; vertRes=w1;
		ar >> w1; HeightDoc=w1;
		ar >> w1; WidthDoc=w1;
		ar >> w1; leftPageMargin=w1;
		ar >> w1; bottomPageMargin=w1;
		ar >> w1; rightPageMargin=w1;
		ar >> w1; topPageMargin=w1;
		ar >> w1; heightSeparator=w1;
		ar >> w1; textseparator=w1;
		ar >> w1; fontsize=w1;

		// how data are displayed from file to file
		WORD wBrowseFlags;
		ar >> wBrowseFlags;
		wMult = 1;		bAllChannels    = ((wBrowseFlags & wMult)>0);
		wMult += wMult;	bEntireRecord   = ((wBrowseFlags & wMult)>0);
		wMult += wMult;	bMultirowDisplay= ((wBrowseFlags & wMult)>0);
		wMult += wMult;	bSplitCurves    = ((wBrowseFlags & wMult)>0);
		wMult += wMult;	bMaximizeGain   = ((wBrowseFlags & wMult)>0);
		wMult += wMult;	bCenterCurves   = ((wBrowseFlags & wMult)>0);
		wMult += wMult;	bKeepForEachFile= ((wBrowseFlags & wMult)>0);
		wMult += wMult; bDispOldDetectP = ((wBrowseFlags & wMult)>0);
		wMult += wMult; bFilterDat		= ((wBrowseFlags & wMult)>0);
		wMult += wMult; bDetectWhileBrowse = ((wBrowseFlags & wMult)>0);
		wMult += wMult; bGotoRecordID	= ((wBrowseFlags & wMult)>0);
		wMult += wMult; bDisplayFileName= ((wBrowseFlags & wMult)>0);
		wMult += wMult; bsetTimeSpan	= ((wBrowseFlags & wMult)>0);
		wMult += wMult; bsetmVSpan		= ((wBrowseFlags & wMult)>0);
		wMult += wMult; bDisplayAllClasses = ((wBrowseFlags & wMult)>0);

		// export textual infos from data files
		WORD wInfoFlags;
		ar >> wInfoFlags;
		wMult=1; 		bacqcomments	= ((wInfoFlags & wMult)>0);
		wMult += wMult;	bacqdate		= ((wInfoFlags & wMult)>0);
		wMult += wMult;	bacqtime		= ((wInfoFlags & wMult)>0);
		wMult += wMult;	bfilesize		= ((wInfoFlags & wMult)>0);
		wMult += wMult;	bacqchcomment	= ((wInfoFlags & wMult)>0);
		wMult += wMult;	bacqchsetting	= ((wInfoFlags & wMult)>0);
		wMult += wMult; bunits			= ((wInfoFlags & wMult)>0);
		wMult += wMult; bcontours		= ((wInfoFlags & wMult)>0);
		wMult += wMult; bgraphics		= ((wInfoFlags & wMult)>0);
		wMult += wMult; btoExcel		= ((wInfoFlags & wMult)>0);
		wMult += wMult; bdatabasecols	= ((wInfoFlags & wMult)>0);

		if (version >4)
		{
			WORD w1; ar >> w1; 
			int nbINTitems = w1;
			ar >> hzResolution;	nbINTitems--; //1
			ar >> vtResolution;	nbINTitems--; //2
			ar >> unitMode;		nbINTitems--; //3
			ar >> spkheight;	nbINTitems--; //4
			ar >> spkwidth;		nbINTitems--; //5
			ar >> nfilterindex;	nbINTitems--; //6
			ar >> col0width;	nbINTitems--; //7
			ar >> row0height;	nbINTitems--; //8
			ar >> bcol0visible;	nbINTitems--; //9
			ar >> brow0visible;	nbINTitems--; //10
			ar >> irowheight;	nbINTitems--; //11
			ar >> displaymode;	nbINTitems--; //12
			if (nbINTitems > 0) ar >> spikeclass; nbINTitems--;

			ar >> nbINTitems; 
			icolwidth.SetSize(nbINTitems);
			for (int ii=0; ii< nbINTitems; ii++)
				ar >> icolwidth[ii];
			int nfloats;
			ar >> nfloats;
			ar >> tFirst; nfloats--;
			ar >> tLast; nfloats--;
			ar >> mVspan; nfloats--;

			ar >> nscopeitems;
			if (nscopeitems > 0)
			{
				viewdata.Serialize(ar); nscopeitems--;
				viewspkdetectdata.Serialize(ar); nscopeitems--;
				viewspkdetectspk.Serialize(ar); nscopeitems--;
				viewspkdetectbars.Serialize(ar); nscopeitems--;
				viewadcontvi.Serialize(ar); nscopeitems--;
				spkviewdata.Serialize(ar); nscopeitems--;
				spkviewspk.Serialize(ar); nscopeitems--;
				spkviewbars.Serialize(ar); nscopeitems--;
				spksort1spk.Serialize(ar); nscopeitems--;
				spksort1parms.Serialize(ar); nscopeitems--;
				spksort1hist.Serialize(ar); nscopeitems--;				
				if(nscopeitems > 0) {dbviewdata.Serialize(ar); nscopeitems--;}
				if(nscopeitems > 0) {dbviewbars.Serialize(ar); nscopeitems--;}
				if(nscopeitems > 0) {dbviewshape.Serialize(ar); nscopeitems--;}
				if(nscopeitems > 0) {spksort1bars.Serialize(ar); nscopeitems--;}
				if(nscopeitems > 0) {viewspkdetectfiltered.Serialize(ar); nscopeitems--;}
				ASSERT(nscopeitems == 0);
			}
		}
		else if (version >= 2)
		{
			WORD w1; ar >> w1; int nbINTitems = w1;
			if (nbINTitems > 0)	ar >> hzResolution; nbINTitems--;
			if (nbINTitems > 0) {
				ar >> vtResolution;  nbINTitems--;
				ar >> unitMode;  nbINTitems--;
			}
			if (nbINTitems > 0)	{
				ar >> spkheight;   nbINTitems--;
				ar >> spkwidth;  nbINTitems--;
			}
			if (nbINTitems > 0)	
				ar >> nfilterindex; 
			nbINTitems--;
			if (nbINTitems > 0)
			{
				for (int ii=0; ii< 8; ii++)
				{
					ar >> icolwidth[ii];
					nbINTitems --;
				}
			}
			if (nbINTitems > 0)
			{
				ar >> col0width; nbINTitems --;
				ar >> row0height; nbINTitems --;
				ar >> bcol0visible; nbINTitems --;
				ar >> brow0visible; nbINTitems --;
			}

			ar >> nscopeitems;
			if (nscopeitems > 0)
			{
				viewdata.Serialize(ar); nscopeitems--;
				viewspkdetectdata.Serialize(ar); nscopeitems--;
				viewspkdetectspk.Serialize(ar); nscopeitems--;
				viewspkdetectbars.Serialize(ar); nscopeitems--;
				viewadcontvi.Serialize(ar); nscopeitems--;
				spkviewdata.Serialize(ar); nscopeitems--;
				spkviewspk.Serialize(ar); nscopeitems--;
				spkviewbars.Serialize(ar); nscopeitems--;
				spksort1spk.Serialize(ar); nscopeitems--;
				spksort1parms.Serialize(ar); nscopeitems--;
				spksort1hist.Serialize(ar); nscopeitems--;				
				if(nscopeitems > 0) {dbviewdata.Serialize(ar); nscopeitems--;}
				if(nscopeitems > 0) {dbviewbars.Serialize(ar); nscopeitems--;}
				if(nscopeitems > 0) {dbviewshape.Serialize(ar); nscopeitems--;}
				if(nscopeitems > 0) {spksort1bars.Serialize(ar); nscopeitems--;}
				if (nscopeitems > 0) {viewspkdetectfiltered.Serialize(ar); nscopeitems--;}
				ASSERT(nscopeitems == 0);
			}
		}
	}
}

//------------------ class OPTIONS_VIEWSPIKES ---------------------------------

IMPLEMENT_SERIAL(OPTIONS_VIEWSPIKES, CObject, 0 /* schema number*/ )

OPTIONS_VIEWSPIKES::OPTIONS_VIEWSPIKES()
{
	wversion = 3;

	timestart=0.0f;			// interval definition
	timeend=2.0f;			//
	timebin=0.1f;			// bin size (sec)
	histampl_vmax=1.f;
	histampl_vmin=0.f;
	histampl_nbins=20;
	nbins=20;				// number of bins
	classnb=0;				// class nb
	classnb2 = 0;
	bacqcomments=TRUE;		// source data comments
	bacqdate=TRUE;			// source data time and date
	bacqchsettings=FALSE;	// source data acq chan settings
	bspkcomments=FALSE;		// spike file comments
	exportdatatype=0;
	spikeclassoption=FALSE;	// discriminate spike class
	bartefacts=TRUE;		// eliminate artefacts (class < 0)
	bcolumnheader=TRUE;		// column headers ON/OFF
	btotalspikes=TRUE;		// total nb of spikes
	babsolutetime=TRUE;		// TRUE= absolute spike time
	bexportzero=FALSE;

	heightLine=130;			// height of one line
	heightSeparator=20;		// height between lines
	bdisplayBars=TRUE;		// default(TRUE)
	bdisplayShapes=TRUE;	// default(TRUE)
	bsplitClasses=TRUE;		// display classes on different lines
	ballfiles=FALSE;
	dotheight=3;
	dotlineheight=5;
	bdotunderline=FALSE;
	nbinsISI=100;
	binISI=2.f;
	bYmaxAuto=TRUE;
	Ymax=1.0f;

	crHistFill= RGB(0x80, 0x80, 0x80);		// COLORREF
	crHistBorder= RGB(0x80, 0x80, 0x80);
	crStimFill=RGB(117, 192, 239);
	crStimBorder= RGB(117, 192, 239);
	crChartArea=RGB(255,255,255);
	int i = 18;
	i--; crScale [i] = RGB(35,31,28); //RGB(255,255,255);
	i--; crScale [i] = RGB(213,45,31);
	i--; crScale [i] = RGB(222,100,19);
	i--; crScale [i] = RGB(232,142,33);
	i--; crScale [i] = RGB(243,196,0);
	i--; crScale [i] = RGB(226,237,0);
	i--; crScale [i] = RGB(142,201,33);
	i--; crScale [i] = RGB(64,174,53);
	i--; crScale [i] = RGB(0,152,62);
	i--; crScale [i] = RGB(0,142,85);
	i--; crScale [i] = RGB(0,142,123);
	i--; crScale [i] = RGB(0,142,163);
	i--; crScale [i] = RGB(0,139,206);
	i--; crScale [i] = RGB(0,117,190);
	i--; crScale [i] = RGB(0,90,158);
	i--; crScale [i] = RGB(29,62,133);
	i--; crScale [i] = RGB(49,20,105);
	i--; crScale [i] = RGB(255,255,255); //RGB(35,31,28);
	ASSERT(i>=0);
	fScalemax = 100.f;
	istimulusindex = 0;
	bhistType = 0;
	bCycleHist=FALSE;
	nstipercycle=1;
	ballChannels = TRUE;
}

OPTIONS_VIEWSPIKES::~OPTIONS_VIEWSPIKES()
{
}

void OPTIONS_VIEWSPIKES::operator = (const OPTIONS_VIEWSPIKES& arg)
{
	timestart=arg.timestart;
	timeend=arg.timeend;
	timebin=arg.timebin;
	histampl_vmax=arg.histampl_vmax;
	histampl_vmin=arg.histampl_vmin;
	histampl_nbins=arg.histampl_nbins;
	
	nbins=arg.nbins;
	classnb=arg.classnb;
	classnb2= arg.classnb2;
	bacqcomments=arg.bacqcomments;
	bacqdate=arg.bacqdate;
	bacqchsettings=arg.bacqchsettings;
	bspkcomments=arg.bspkcomments;
	exportdatatype=arg.exportdatatype;
	spikeclassoption=arg.spikeclassoption;
	bartefacts=arg.bartefacts;
	bcolumnheader=arg.bcolumnheader;
	btotalspikes=arg.btotalspikes;
	babsolutetime=arg.babsolutetime;
	bexportzero = arg.bexportzero;
	ballChannels = arg.ballChannels;

	heightLine=arg.heightLine;
	bdisplayBars=arg.bdisplayBars;
	bdisplayShapes=arg.bdisplayShapes;
	heightSeparator=arg.heightSeparator;
	bsplitClasses=arg.bsplitClasses;
	ballfiles=arg.ballfiles;
	dotheight=arg.dotheight;
	dotlineheight=arg.dotlineheight;
	bdotunderline=arg.bdotunderline;
	nbinsISI=arg.nbinsISI;
	binISI=arg.binISI;
	bYmaxAuto=arg.bYmaxAuto;
	Ymax=arg.Ymax;

	crHistFill= arg.crHistFill;
	crHistBorder= arg.crHistBorder;
	crStimFill=arg.crStimFill;
	crStimBorder= arg.crStimBorder;
	crChartArea=arg.crChartArea;
	for (int i=0; i< 18; i++)
		crScale[i] = arg.crScale[i];
	fScalemax = arg.fScalemax ;
	istimulusindex = arg.istimulusindex;
	bhistType= arg.bhistType;
	bCycleHist = arg.bCycleHist;
	nstipercycle = arg.nstipercycle;
}

void OPTIONS_VIEWSPIKES::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << wversion;

		ar << timestart << timeend << timebin;
		ar << (WORD) nbins;
		ar << (WORD) classnb;	// classnb2 is added to the series of int parameters
		WORD wE, wM;

		wM =1;	  wE =  bacqcomments	* wM;		//1:  1
		wM += wM; wE += bacqdate 		* wM;		//2:  2
		wM += wM; wE += bacqcomments 	* wM;		//3:  4
		wM += wM; wE += bacqdate 		* wM;		//4:  8
		wM += wM; wE += bacqchsettings	* wM;		//5:  16
		wM += wM; wE += bspkcomments 	* wM;		//6:  32
		wM += wM; wE += bincrflagonsave	* wM;		//7:  64
		wM += wM; wE += bexportzero 	* wM;		//8:  128
		wM += wM; wE += bexportPivot	* wM;		//9:  256
		wM += wM; wE += bexporttoExcel 	* wM;		//10: 512	empty slot
		wM += wM; wE += bartefacts 		* wM;		//11: 1024
		wM += wM; wE += bcolumnheader 	* wM;		//12: 2048
		wM += wM; wE += btotalspikes 	* wM;		//13: 4096
		wM += wM; wE += babsolutetime 	* wM;		//14: 8192
		wM += wM; wE += ballfiles 		* wM;		//15: 16384
		wM += wM; wE += bdotunderline	* wM;		//16: 32768
		ar << wE;

		// int parameters
		wIntParms = 11; ar << wIntParms;			// set to nb of following lines
		ar << (WORD) heightLine;					// 1
		ar << (WORD) heightSeparator;				// 2
		ar << (WORD) dotheight;						// 3
		ar << (WORD) dotlineheight;					// 4
		ar << (WORD) nbinsISI;						// 5
		ar << (WORD) istimulusindex;				// 6
		ar << (WORD) bhistType;						// 7
		ar << nstipercycle;							// 8
		ar << exportdatatype;						// 9
		ar << spikeclassoption;						// 10
		ar << classnb2;								// 11

		WORD wnb=0;
		wM =1;	  wE=  bdisplayBars * wM; wnb++;	// 1
		wM += wM; wE+= bdisplayShapes* wM; wnb++;	// 2: 2
		wM += wM; wE+= bsplitClasses* wM; wnb++;	// 3: 4
		wM += wM; wE+= bYmaxAuto* wM; wnb++;		// 4: 8
		wM += wM; wE+= bCycleHist* wM; wnb++;		// 5: 16
		wM += wM; wE+= ballChannels* wM; wnb++;		// 6: 32
		ar << wnb;		// set to nb of bool parms
		ar << wE;		// set compressed BOOL data

		//float parameters or 32 bits values
		WORD wnparms = 29; ar << wnparms;
		ar << histampl_vmax;						// 1
		ar << histampl_vmin;						// 2
		ar << histampl_nbins;						// 3
		ar << binISI;								// 4
		ar << Ymax;									// 5
		ar << crHistFill;							// 6
		ar << crHistBorder;							// 7
		ar << crStimFill;							// 8
		ar << crStimBorder;							// 9
		ar << crChartArea;							// 10
		for (int i=0; i< 18; i++)
			ar << crScale[i];
		ar << fScalemax ;							// 29

	} 
	else
	{
		WORD wversion;  ar >> wversion;

		WORD w1;
		ar >> timestart >> timeend >> timebin;
		ar >> w1; nbins=w1;
		ar >> w1; classnb=w1;
		
		// print options
		WORD wE;
		WORD wM=1;		
		ar >> wE;
		wM =1;	  bacqcomments 		= ((wE & wM)!=0);	//1
		wM += wM; bacqdate 			= ((wE & wM)!=0);	//2
		wM += wM; bacqcomments 		= ((wE & wM)!=0);	//4
		wM += wM; bacqdate 			= ((wE & wM)!=0);	//8
		wM += wM; bacqchsettings 	= ((wE & wM)!=0);	//16
		wM += wM; bspkcomments 		= ((wE & wM)!=0);	//32
		wM += wM; bincrflagonsave	= ((wE & wM)!=0);	//64
		// this parm does not belong to print options but to export options
		wM += wM; bexportzero		= ((wE & wM)>0);	//128
		wM += wM; bexportPivot		= ((wE & wM)>0);	//256		
		wM += wM; bexporttoExcel 	= ((wE & wM)!=0);	//512

		wM += wM; bartefacts 		= ((wE & wM)!=0);	//1024
		wM += wM; bcolumnheader		= ((wE & wM)!=0);	//2048
		wM += wM; btotalspikes 		= ((wE & wM)!=0);	//4096
		wM += wM; babsolutetime		= ((wE & wM)!=0);	//8192
		wM += wM; ballfiles 		= ((wE & wM)!=0);	//16384
		wM += wM; bdotunderline		= ((wE & wM)!=0);	//32768

		// int parameters
		int nb; ar >> w1; nb=w1;	// number of int parameters
		ar >> w1; heightLine = w1; nb--;					// 1
		ar >> w1; heightSeparator = w1; nb--;				// 2
		if (nb > 0) {ar >> w1; dotheight=w1; nb--;}			// 3
		if (nb > 0) {ar >> w1; dotlineheight=w1; nb--;}		// 4
		if (nb > 0) {ar >> w1; nbinsISI=w1; nb--;}			// 5
		if (nb > 0) {ar >> w1; istimulusindex=w1; nb--;}	// 6
		if (nb > 0) {ar >> w1; bhistType=w1; nb--;}			// 7
		if (nb > 0) {ar >> nstipercycle; nb--;}				// 8
		if (nb > 0) {ar >> exportdatatype; nb--;}			// 9
		if (nb > 0) {ar >> spikeclassoption; nb--;}			// 10
		if (nb > 0) {ar >> classnb2; nb--;}	else {classnb2 = classnb;} // 11
		while (nb > 0) {ar >> w1; nb--;}

		ar >> w1; nb=w1;	// number of bool parameters
		ar >> wE;			// data -> decompress
		// version 1 (20-4-96): 3 parameters
		wM =1;	  bdisplayBars 		= ((wE& wM)>0); nb--;			// 1
		wM += wM; bdisplayShapes	= ((wE & wM)>0);nb--;			// 2:2
		wM += wM; bsplitClasses		= ((wE & wM)>0);nb--;			// 3:4
		if (nb >0)	{wM += wM; bYmaxAuto = ((wE & wM)>0); nb--;}	// 4:8
		if (nb >0)	{wM += wM; bCycleHist= ((wE & wM)>0); nb--;}	// 5:16
		if (nb > 0) {wM += wM; ballChannels= ((wE & wM)>0);nb--;}	// 6:32
		while (nb > 0) {ar >> wE; nb--;}

		if (wversion > 2)
		{
			ar >> w1; nb = w1;
			ar >> histampl_vmax; nb--;
			ar >> histampl_vmin; nb--;
			ar >> histampl_nbins; nb--;
			if (nb > 0){ ar >> binISI; nb--;}
			if (nb > 0) {ar >> Ymax; nb--;}			
			if (nb > 0) {ar >> crHistFill; nb--;}
			if (nb > 0) {ar >> crHistBorder; nb--;}
			if (nb > 0) {ar >> crStimFill; nb--;}
			if (nb > 0) {ar >> crStimBorder; nb--;}
			if (nb > 0) {ar >> crChartArea; nb--;}
			COLORREF crdummy;
			for (int i=0; i< 18; i++)
				//if (nb > 0) {ar >> crScale[i]; nb--;}
				if (nb > 0) {ar >> crdummy; nb--;}
			if (nb > 0) {ar >> fScalemax; nb--;} 
		}
	}
}

// --------------------------------------------------------------
// view data measure options

//------------------ class OPTIONS_VIEWDATA ---------------------------------

IMPLEMENT_SERIAL(OPTIONS_VIEWDATAMEASURE, CObject, 0 /* schema number*/ )

OPTIONS_VIEWDATAMEASURE::OPTIONS_VIEWDATAMEASURE()
{	
	bChanged=FALSE;
	wversion=2;

	// parameters to measure
	bExtrema=TRUE;			// measure max and min (val, time)
	bDiffExtrema=TRUE;		// difference between extrema (val, time)
	bHalfrisetime=FALSE;	// time necessary to reach half of the amplitude
	bHalfrecovery=FALSE;	// time necessary to regain half of the initial amplitude
	bDatalimits=TRUE;		// data value at limits
	bDiffDatalimits=TRUE;	// difference between data limits
	btime=FALSE;
	bKeepTags=TRUE;			// keep flags after measurement
	bSaveTags=FALSE;		// save tags
	bSetTagsforCompleteFile=FALSE; // dlg box option

	// source data
	bAllChannels=FALSE;		// measure on all chans (TRUE) or on single chan (FALSE)
	wSourceChan=0;			// if bAllChans=FALSE, source data chan
	bAllFiles=FALSE;		// measure on all files (default=FALSE)

	// limits
	wOption=2;				// 0=vertic tags, 1=horiz lines; 2=rectangle; 3=relative to stimulus
	wStimuluschan=0;		// if wOption=2: stimulus channel
	wStimulusthresh=2048;	// if wOption=2: stimulus threshold
	fStimulusoffset=0.0f;	// if wOption=2: offset interval (default=0)

	// edit VT tags options (version 2)
	duration=0.8f;			// on/OFF duration (sec)
	period=3.0f;			// period (sec)
	nperiods=10;			// nb of duplicates
	timeshift=0.0f;			// shift tags

}

OPTIONS_VIEWDATAMEASURE::~OPTIONS_VIEWDATAMEASURE()
{
}

void OPTIONS_VIEWDATAMEASURE::operator = (const OPTIONS_VIEWDATAMEASURE& arg)
{
	bChanged=arg.bChanged;	

	// parameters to measure
	bExtrema=arg.bExtrema;			// measure max and min (val, time)	
	bDiffExtrema=arg.bDiffExtrema;	// difference between extrema (val, time)
	bDiffDatalimits=arg.bDiffDatalimits; // diff between tags/cursors
	bHalfrisetime=arg.bHalfrisetime;// time necessary to reach half of the amplitude
	bHalfrecovery=arg.bHalfrecovery;// time necessary to regain half of the initial amplitude
	bDatalimits=arg.bDatalimits;	// data value at limits
	btime=arg.btime;
	bKeepTags=arg.bKeepTags;		// keep flags after measurement
	bSaveTags=arg.bSaveTags;
	bSetTagsforCompleteFile=arg.bSetTagsforCompleteFile;

	// source data
	bAllChannels=arg.bAllChannels;	// measure on all chans (TRUE) or on single chan (FALSE)
	wSourceChan=arg.wSourceChan;	// if bAllChans=FALSE, source data chan
	bAllFiles=arg.bAllFiles;		// measure on all files (default=FALSE)

	// limits
	wOption=arg.wOption;			// 0=between vertic tags, 1=between horiz lines, 2=relative to stimulus
	wStimuluschan=arg.wStimuluschan;// if wOption=2: stimulus channel
	wStimulusthresh=arg.wStimulusthresh;// if wOption=2: stimulus threshold
	fStimulusoffset=arg.fStimulusoffset;// if wOption=2: offset interval (default=0)
	wLimitSup=arg.wLimitSup;		// rectangle limits
	wLimitInf=arg.wLimitInf;
	lLimitLeft=arg.lLimitLeft;
	lLimitRight=arg.lLimitRight;

	// dlg edit VT
	duration = arg.duration;		// on/OFF duration (sec)
	period=arg.period;				// period (sec)
	nperiods= arg.nperiods;			// nb of duplicates
	timeshift= arg.timeshift;		// shift tags

}

void OPTIONS_VIEWDATAMEASURE::Serialize(CArchive& ar)
{
	if (ar.IsStoring())		// save data .....................
	{
		ar << wversion;
		
		// measure options
		WORD wMeasureFlags;
		WORD wMult;
		wMult =1;		wMeasureFlags =  bExtrema		* wMult;
		wMult += wMult; wMeasureFlags += bDiffExtrema	* wMult;
		wMult += wMult; wMeasureFlags += bHalfrisetime	* wMult;
		wMult += wMult; wMeasureFlags += bHalfrecovery	* wMult;
		wMult += wMult; wMeasureFlags += bDatalimits	* wMult;
		wMult += wMult; wMeasureFlags += bDiffDatalimits* wMult;
		wMult += wMult; wMeasureFlags += btime			* wMult;
		wMult += wMult; wMeasureFlags += bKeepTags		* wMult;
		wMult += wMult; wMeasureFlags += bSaveTags		* wMult;
		wMult += wMult; wMeasureFlags += bSetTagsforCompleteFile* wMult;
		ar << wMeasureFlags;		

		// source data	
		ar << (WORD) bAllChannels;	
		ar <<  wSourceChan;
		ar << (WORD) bAllFiles;

		// limits
		ar << wOption;
		ar << wStimuluschan << wStimulusthresh << fStimulusoffset;
		ar << wLimitSup << wLimitInf << lLimitLeft << lLimitRight;

		// dlg edit VT tags
		ar << duration;			// on/OFF duration (sec)
		ar << period;			// period (sec)
		ar << nperiods;			// nb of duplicates
		ar << timeshift;		// shift tags
	} 
	else					// read data ...................
	{
		WORD version; ar >> version;
		// measure options version 1
		WORD wMeasureFlags;
		WORD wMult;
		ar >> wMeasureFlags;
		wMult =1;		bExtrema		= ((wMeasureFlags & wMult)>0);
		wMult += wMult; bDiffExtrema	= ((wMeasureFlags & wMult)>0);
		wMult += wMult; bHalfrisetime	= ((wMeasureFlags & wMult)>0);
		wMult += wMult; bHalfrecovery	= ((wMeasureFlags & wMult)>0);
		wMult += wMult; bDatalimits		= ((wMeasureFlags & wMult)>0);
		wMult += wMult; bDiffDatalimits	= ((wMeasureFlags & wMult)>0);
		wMult += wMult; btime			= ((wMeasureFlags & wMult)>0);
		wMult += wMult; bKeepTags		= ((wMeasureFlags & wMult)>0);
		wMult += wMult; bSaveTags		= ((wMeasureFlags & wMult)>0);
		wMult += wMult; bSetTagsforCompleteFile= ((wMeasureFlags & wMult)>0);

		// source data
		WORD w1;
		ar >> w1; bAllChannels=w1;
		ar >> wSourceChan;
		ar >> w1; bAllFiles=w1;

		// limits
		ar >> wOption;
		ar >> wStimuluschan >> wStimulusthresh >> fStimulusoffset;
		ar >> wLimitSup >> wLimitInf >> lLimitLeft >> lLimitRight;

		// dlg edit VT tags - parameters
		if (version > 1)
		{
			ar >> duration;			// on/OFF duration (sec)
			ar >> period;			// period (sec)
			ar >> nperiods;			// nb of duplicates
			ar >> timeshift;		// shift tags
		}
	}    	
}


//------------------ class OPTIONS_IMPORT ---------------------------------

IMPLEMENT_SERIAL(OPTIONS_IMPORT, CObject, 0)

OPTIONS_IMPORT::OPTIONS_IMPORT()
{
	wVersion = 6;

	// Varian parameters
	fGainFID = 10.f;
	fGainEAD = 10.f;
	// generic import options
	bSingleRun=TRUE;
	bPreview = TRUE;
	bSapid3_5=FALSE;
	nbRuns=1;
	nbChannels=1;
	samplingRate=10.0f;
	encodingMode= OLx_ENC_BINARY;
	bitsPrecision=12;
	voltageMax=10.0f;
	voltageMin=-10.0f;
	skipNbytes=5;
	title="";
	pwaveChanArray = new (CWaveChanArray);
	ASSERT(pwaveChanArray != NULL);
	pwaveChanArray->ChannelAdd();	// add dummy channel
	nSelectedFilter=0;
	// export options
	exportType=0;
	bAllchannels=TRUE;
	bSeparateComments=FALSE;
	bincludeTime=FALSE;
	bentireFile=TRUE;
	selectedChannel=0;
	fTimefirst=0.f;
	fTimelast= 1.f;
	pathWTOASCII.Empty();
	iundersample=1;
	path.Empty();
	bDummy=FALSE;
}

OPTIONS_IMPORT::~OPTIONS_IMPORT()
{
	SAFE_DELETE(pwaveChanArray);
}

void OPTIONS_IMPORT::operator = (const OPTIONS_IMPORT& arg)
{
	wVersion = arg.wVersion;
	fGainFID = arg.fGainFID;
	fGainEAD = arg.fGainEAD;
	pathWTOASCII=arg.pathWTOASCII;

	// generic import options
	bSingleRun=arg.bSingleRun;
	bPreview = arg.bPreview;
	bSapid3_5=arg.bSapid3_5;
	bDummy=arg.bDummy;
	nbRuns=arg.nbRuns;
	nbChannels=arg.nbChannels;
	samplingRate=arg.samplingRate;
	encodingMode=arg.encodingMode;
	bitsPrecision=arg.bitsPrecision;
	voltageMax=arg.voltageMax;
	voltageMin=arg.voltageMin;
	skipNbytes=arg.skipNbytes;
	title=arg.title;
	*pwaveChanArray= *(arg.pwaveChanArray);
	nSelectedFilter = arg.nSelectedFilter;

	// export options
	exportType= arg.exportType;
	bAllchannels= arg.bAllchannels;
	bSeparateComments= arg.bSeparateComments;
	bincludeTime=arg.bincludeTime;
	selectedChannel= arg.selectedChannel;
	fTimefirst= arg.fTimefirst;
	fTimelast= arg.fTimelast;
	bentireFile=arg.bentireFile;
	iundersample=arg.iundersample;

	// others
	path=arg.path;
}

void OPTIONS_IMPORT::Serialize(CArchive& ar)
{
	if(ar.IsStoring())
	{
		wVersion = 6; ar << wVersion;
		ar << fGainFID;
		ar << fGainEAD;

		// generic data import options
		int bflag=bSapid3_5;	// combine flags: update flag, combine
		bflag <<= 1; bflag += bPreview;
		bflag <<= 1; bflag += bSingleRun;
		ar << bflag;

		ar << (WORD) nbRuns;
		ar << (WORD) nbChannels;
		ar << samplingRate;
		ar << (WORD) encodingMode;
		ar << (WORD) bitsPrecision;
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
		bflag <<=1; bflag += bImportDuplicateFiles;
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
			// decompose flags: update flag (/2),  get val, mask			
			bSingleRun = bflag;  bSingleRun &= 0x1; bflag >>= 1;
			bPreview = bflag;  bPreview &= 0x1; bflag >>= 1;
			bSapid3_5 = bflag; bSapid3_5 &= 0x1;
		
			ar >> w1; nbRuns =w1;
			ar >> w1; nbChannels=w1;
			ar >> samplingRate;
			ar >> w1; encodingMode=w1;
			ar >> w1; bitsPrecision=w1;
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
			bAllchannels=bflag; bAllchannels &= 0x1;				// 1rst: allchannels
			ar >> selectedChannel;
			ar >> fTimefirst; ar >> fTimelast;
		}
		if (version >=5)
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
					bImportDuplicateFiles = bflag; bImportDuplicateFiles &= 0x1; bflag >>= 1;
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

//------------------ class OPTIONS_ACQDATA ---------------------------------

IMPLEMENT_SERIAL(OPTIONS_ACQDATA, CObject, 0 /* schema number*/ )

OPTIONS_ACQDATA::OPTIONS_ACQDATA()
{
	bChanged = FALSE;
	m_wversion = 2;
	icsA_stimulus=0;
	icsA_concentration=0;
	icsA_insect=0;
	icsA_location=0;
	icsA_sensillum=0;
	icsA_strain=0;
	icsA_sex=0;
	icsA_operatorname=0;
	icsA_stimulus2=0;
	icsA_concentration2=0;
	iundersample=1;
	baudiblesound=0;
	bChannelType=OLx_CHNT_DIFFERENTIAL;
	izoomCursel=0;
	sweepduration = 1.0f;
}

OPTIONS_ACQDATA::~OPTIONS_ACQDATA()
{
}

void OPTIONS_ACQDATA::operator = (const OPTIONS_ACQDATA& arg)
{
	chanArray=arg.chanArray;
	waveFormat=arg.waveFormat;
	csBasename=arg.csBasename;
	csPathname=arg.csPathname;
	exptnumber=arg.exptnumber;
	csA_stimulus.RemoveAll(); csA_stimulus.Append(arg.csA_stimulus);
	csA_concentration.RemoveAll(); csA_concentration.Append(arg.csA_concentration);
	csA_stimulus2.RemoveAll(); csA_stimulus2.Append(arg.csA_stimulus2);
	csA_concentration2.RemoveAll(); csA_concentration2.Append(arg.csA_concentration2);
	csA_insect.RemoveAll(); csA_insect.Append(arg.csA_insect);
	csA_location.RemoveAll(); csA_location.Append(arg.csA_location);
	csA_sensillum.RemoveAll(); csA_sensillum.Append(arg.csA_sensillum);
	csA_strain.RemoveAll(); csA_strain.Append(arg.csA_strain);
	csA_sex.RemoveAll(); csA_sex.Append(arg.csA_sex);
	csA_operatorname.RemoveAll(); csA_operatorname.Append(arg.csA_operatorname);
	csA_expt.RemoveAll(); csA_expt.Append(arg.csA_expt);

	icsA_stimulus=arg.icsA_stimulus;
	icsA_concentration=arg.icsA_concentration;
	icsA_stimulus2=arg.icsA_stimulus2;
	icsA_concentration2=arg.icsA_concentration2;
	icsA_expt= arg.icsA_expt;
	izoomCursel= arg.izoomCursel;

	icsA_insect=arg.icsA_insect;
	icsA_location=arg.icsA_location;
	icsA_sensillum=arg.icsA_sensillum;
	icsA_strain=arg.icsA_strain;
	icsA_sex = arg.icsA_sex;
	icsA_operatorname=arg.icsA_operatorname;
	iundersample=arg.iundersample;
	baudiblesound=arg.baudiblesound;
	bChannelType = arg.bChannelType;

	sweepduration = arg.sweepduration;
}

void OPTIONS_ACQDATA::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << m_wversion;
		
		ar << (WORD) 2;				// 1 - string parameters
		ar << csBasename;
		ar << csPathname;

		ar << (WORD) 18;			// 2 - int parameters
		ar << exptnumber;			// 1
		ar << icsA_stimulus;		// 2
		ar << icsA_concentration;	// 3
		ar << icsA_insect;			// 4
		ar << icsA_location;		// 5
		ar << icsA_sensillum;		// 6
		ar << icsA_strain;			// 7
		ar << icsA_operatorname;	// 8
		ar << iundersample;			// 9
		ar << baudiblesound;		// 10
		ar << bChannelType;			// 11
		ar << icsA_stimulus2;		// 12
		ar << icsA_concentration2;	// 13
		ar << izoomCursel;			// 14
		ar << icsA_sex;				// 15
		ar << icsA_repeat;			// 16
		ar << icsA_repeat2;			// 17
		ar << icsA_expt;			// 18

		ar << (WORD) 11;			// 3 - CStringArray parameters
		int nsize;
		int i;
		nsize= csA_stimulus.GetSize();		ar << nsize; for(i=0; i<nsize; i++) { ar << csA_stimulus.GetAt(i);};
		nsize= csA_concentration.GetSize(); ar << nsize; for(i=0; i<nsize; i++) { ar << csA_concentration.GetAt(i);};
		nsize= csA_insect.GetSize();		ar << nsize; for(i=0; i<nsize; i++) { ar << csA_insect.GetAt(i);};
		nsize= csA_location.GetSize();		ar << nsize; for(i=0; i<nsize; i++) { ar << csA_location.GetAt(i);};
		nsize= csA_sensillum.GetSize();		ar << nsize; for(i=0; i<nsize; i++) { ar << csA_sensillum.GetAt(i);};
		nsize= csA_strain.GetSize();		ar << nsize; for(i=0; i<nsize; i++) { ar << csA_strain.GetAt(i);};
		nsize= csA_operatorname.GetSize();	ar << nsize; for(i=0; i<nsize; i++) { ar << csA_operatorname.GetAt(i);};
		nsize= csA_stimulus2.GetSize();		ar << nsize; for(i=0; i<nsize; i++) { ar << csA_stimulus2.GetAt(i);};
		nsize= csA_concentration2.GetSize();ar << nsize; for(i=0; i<nsize; i++) { ar << csA_concentration2.GetAt(i);};
		nsize= csA_sex.GetSize();			ar << nsize; for(i=0; i<nsize; i++) { ar << csA_sex.GetAt(i);};
		nsize= csA_expt.GetSize();			ar << nsize; for(i=0; i<nsize; i++) { ar << csA_expt.GetAt(i);};

		ar << (WORD) 3;				// 4 - serialized objects
		chanArray.Serialize(ar);
		waveFormat.Serialize(ar);
		ar << sweepduration;
	} 
	else
	{
		WORD version;  ar >> version;
		int n; 
		WORD wn;

		// string parameters
		// TODO: check if names (CString) are read ok
		ar >> wn; n = wn;
		if (n > 0) ar >> csBasename; n--;
		if (n > 0) ar >> csPathname; n--;
		CString csdummy; while (n > 0) {n--; ar >> csdummy;}

		// int parameters
		ar >> wn; n = wn;
		if (n > 0) ar >> exptnumber; n--;
		if (n > 0) ar >> icsA_stimulus; n--;
		if (n > 0) ar >> icsA_concentration; n--;
		if (n > 0) ar >> icsA_insect; n--;
		if (n > 0) ar >> icsA_location; n--;
		if (n > 0) ar >> icsA_sensillum; n--;
		if (n > 0) ar >> icsA_strain; n--;
		if (n > 0) ar >> icsA_operatorname; n--;
		if (n > 0) ar >> iundersample; n--;
		if (n > 0) ar >> baudiblesound; n--;
		if (n > 0) ar >> bChannelType; n--;
		if (n > 0) ar >> icsA_stimulus2; n--;
		if (n > 0) ar >> icsA_concentration2; n--;
		if (n > 0) ar >> izoomCursel; n--;
		if (n > 0) ar >> icsA_sex; n--;
		if (n > 0) ar >> icsA_repeat; n--;
		if (n > 0) ar >> icsA_repeat2; n--;
		if (n > 0) ar >> icsA_expt; n--;
		int	idummy; while (n > 0) {n--; ar >> idummy;}

		// CStringArray parameters
		ar >> wn; n = wn;
		int nsize;
		int i;
		CString dummy;
		if (n>0) {ar >> nsize; csA_stimulus.SetSize(nsize); for(i=0; i<nsize; i++) { ar >> dummy; csA_stimulus.SetAt(i, dummy);};} n--;
		if (n>0) {ar >> nsize; csA_concentration.SetSize(nsize); for(i=0; i<nsize; i++) { ar >> dummy; csA_concentration.SetAt(i, dummy);};} n--;
		if (n>0) {ar >> nsize; csA_insect.SetSize(nsize); for(i=0; i<nsize; i++) { ar >> dummy; csA_insect.SetAt(i, dummy);};} n--;
		if (n>0) {ar >> nsize; csA_location.SetSize(nsize); for(i=0; i<nsize; i++) { ar >> dummy; csA_location.SetAt(i, dummy);};} n--;
		if (n>0) {ar >> nsize; csA_sensillum.SetSize(nsize); for(i=0; i<nsize; i++) { ar >> dummy; csA_sensillum.SetAt(i, dummy);};} n--;
		if (n>0) {ar >> nsize; csA_strain.SetSize(nsize); for(i=0; i<nsize; i++) { ar >> dummy; csA_strain.SetAt(i, dummy);};} n--;
		if (n>0) {ar >> nsize; csA_operatorname.SetSize(nsize); for(i=0; i<nsize; i++) { ar >> dummy; csA_operatorname.SetAt(i, dummy);};} n--;
		if (n>0) {ar >> nsize; csA_stimulus2.SetSize(nsize); for(i=0; i<nsize; i++) { ar >> dummy; csA_stimulus2.SetAt(i, dummy);};} n--;
		if (n>0) {ar >> nsize; csA_concentration2.SetSize(nsize); for(i=0; i<nsize; i++) { ar >> dummy; csA_concentration2.SetAt(i, dummy);};} n--;
		if (n>0) {ar >> nsize; csA_sex.SetSize(nsize); for(i=0; i<nsize; i++) { ar >> dummy; csA_sex.SetAt(i, dummy);};} n--;
		if (n>0) {ar >> nsize; csA_expt.SetSize(nsize); for(i=0; i<nsize; i++) { ar >> dummy; csA_expt.SetAt(i, dummy);};} n--;
		
		while (n > 0) {
			n--; ar >> nsize; for(i=0; i<nsize; i++) { ar >> dummy;}
		}
		// serialized objects
		ar >> wn; n = wn;
		if (n > 0) chanArray.Serialize(ar); n--; 
		if (n > 0) waveFormat.Serialize(ar); n--;
		if (n > 0) ar >> sweepduration; n--; 
	}
}

//----------------------------------------------
	
//------------------ class OPTIONS_OUTPUTDATA ---------------------------------

IMPLEMENT_SERIAL(OPTIONS_OUTPUTDATA, CObject, 0 /* schema number*/ )

OPTIONS_OUTPUTDATA::OPTIONS_OUTPUTDATA()
{
	bChanged=FALSE;						// flag set TRUE if contents has changed
	wversion=1;							// version number
	bAllowDA				=TRUE;
	bPresetWave				=TRUE;
	iDAbufferlength			=10000;
	iDAnbuffers				=10;
	iDATriggermode			=0;
	dDAFrequency_perchan	=1000.;
	parmsChan.SetSize(1);
}

OPTIONS_OUTPUTDATA::~OPTIONS_OUTPUTDATA()
{
}

void OPTIONS_OUTPUTDATA::operator = (const OPTIONS_OUTPUTDATA& arg)
{
	wversion				=arg.wversion;
	bAllowDA				=arg.bAllowDA;
	csOutputFile			=arg.csOutputFile;;
	bPresetWave				=arg.bPresetWave;
	iDAbufferlength			=arg.iDAbufferlength;
	iDAnbuffers				=arg.iDAnbuffers;
	iDATriggermode			=arg.iDATriggermode;
	dDAFrequency_perchan	=arg.dDAFrequency_perchan;
	int nchannels			=arg.parmsChan.GetSize();
	parmsChan.SetSize(nchannels);

	for (int i = 0; i < nchannels; i++) {
		parmsChan.GetAt(i) = arg.parmsChan.GetAt(i);
	}
}

void OPTIONS_OUTPUTDATA::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << wversion;

		ar << (WORD) 1;			// CString 
		ar << csOutputFile;

		ar << (WORD) 2;			// BOOL
		ar << bAllowDA;
		ar << bPresetWave;

		ar << (WORD) 3;			// int 
		ar << iDAbufferlength;
		ar << iDAnbuffers;
		ar << iDATriggermode;

		ar << (WORD) 1;			// double
		ar << dDAFrequency_perchan;

		int nchannels = parmsChan.GetSize(); // OUTPUT_PARMS
		ar << (WORD) nchannels;	
		for (int i = 0; i < nchannels; i++) {
			parmsChan.GetAt(i).Serialize(ar);
		}
		ar << (WORD) 0;			// no more ...
	} 
	else
	{
		WORD version;  ar >> version;
		int n; 
		WORD wn;

		// cstring parameters
		ar >> wn; n = wn;
		if (n > 0) ar >> csOutputFile;			n--;
		CString csdummy; while (n > 0) {n--; ar >> csdummy;}

		// BOOL parameters
		ar >> wn; n = wn;
		if (n > 0) ar >> bAllowDA;				n--;
		if (n > 0) ar >> bPresetWave;			n--;
		BOOL bdummy; while (n > 0) {n--; ar >> bdummy;}

		// int parameters
		ar >> wn; n = wn;
		if (n > 0) ar >> iDAbufferlength;		n--;
		if (n > 0) ar >> iDAnbuffers;			n--;
		if (n > 0) ar >> iDATriggermode;		n--;
		int idummy; while (n > 0) {n--; ar >> idummy;}

		// double parameters
		ar >> wn; n = wn;
		if (n > 0) ar >> dDAFrequency_perchan;	n--;
		double ddummy; while (n > 0) {n--; ar >> ddummy;}

		// output_parms
		ar >> wn; n = wn;
		parmsChan.SetSize(n);
		for (int i = 0; i < n; i++) {
			parmsChan.GetAt(i).Serialize(ar);
		}

		// other?
		ar >> wn; n = wn;
		ASSERT(n==0);
	}
}


//------------------ class OUTPUTPARMS ---------------------------------

IMPLEMENT_SERIAL(OUTPUTPARMS, CObject, 0 /* schema number*/ )

OUTPUTPARMS::OUTPUTPARMS()
{
	wversion=1;
	bON = FALSE;
	noise_bExternal=FALSE;
	iChan=0;
	iWaveform=0;
	mseq_iRatio=1000;
	mseq_iDelay=0;
	mseq_iSeed=0;	
	noise_iDelay=0;
	dAmplitudeMaxV=1.;
	dAmplitudeMinV=-1.;
	dFrequency=1000.;	
	dummy1=1.;
	dummy2=0.;
	noise_dAmplitV=1.;	
	noise_dFactor=1.;
	noise_dOffsetV=0.;
	value = 0;
	num = 512;
	bit33 = 1;
	count = 1;
	stim8lines.SetSize(8);
	for (int i = 0; i < 8; i++)
		stim8lines[i].SetChan(i);
}

OUTPUTPARMS::~OUTPUTPARMS()
{
}

void OUTPUTPARMS::operator = (const OUTPUTPARMS& arg)
{
	wversion		=arg.wversion;
	csFilename		=arg.csFilename;
	noise_bExternal	=arg.noise_bExternal;
	bON				= arg.bON;
	iChan			=arg.iChan;
	iWaveform		=arg.iWaveform;
	mseq_iRatio		=arg.mseq_iRatio;
	mseq_iDelay		=arg.mseq_iDelay;
	mseq_iSeed		=arg.mseq_iSeed;	
	noise_iDelay	=arg.noise_iDelay;
	dAmplitudeMaxV	=arg.dAmplitudeMaxV;
	dAmplitudeMinV	=arg.dAmplitudeMinV;
	dFrequency		=arg.dFrequency;	
	dummy1			=arg.dummy1;
	dummy2			=arg.dummy2;
	noise_dAmplitV	=arg.noise_dAmplitV;	
	noise_dFactor	=arg.noise_dFactor;
	noise_dOffsetV	=arg.noise_dOffsetV;	
	stimulussequence = arg.stimulussequence;
	value			= arg.value;
	for (int i = 0; i < 8; i++)
		stim8lines[i] = arg.stim8lines[i];
	sti				= arg.sti;
}

void OUTPUTPARMS::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << wversion;
		
		ar << (WORD) 1;			// CString 
		ar << csFilename;
		
		ar << (WORD) 2;			// BOOL
		ar << noise_bExternal;
		ar << bON;

		ar << (WORD) 6;			// int 
		ar << iChan;
		ar << iWaveform;
		ar << mseq_iRatio;
		ar << mseq_iDelay;
		ar << mseq_iSeed;	
		ar << noise_iDelay;

		ar << (WORD) 9;			// double
		ar << dAmplitudeMaxV;
		ar << dAmplitudeMinV;
		ar << dFrequency;	
		ar << dummy1;
		ar << dummy2;
		ar << noise_dAmplitV;	
		ar << noise_dFactor;
		ar << noise_dOffsetV;
		ar << value;

		ar << (WORD) 10;			// 1 more object
		stimulussequence.Serialize(ar);
		for (int i = 0; i < 8; i++)
			stim8lines[i].Serialize(ar);
		sti.Serialize(ar);
	} 
	else
	{
		WORD version; ar >> version;
		int n; 
		WORD wn;

		// cstring parameters
		ar >> wn; n = wn;
		if (n > 0) ar >> csFilename; n--;
		CString csdummy; while (n > 0) {n--; ar >> csdummy;}

		// BOOL parameters
		ar >> wn; n = wn;
		if (n > 0) ar >> noise_bExternal;	n--;
		if (n > 0) ar >> bON; n--;
		BOOL bdummy; while (n > 0) {n--; ar >> bdummy;}

		// int parameters
		ar >> wn; n = wn;
		if (n > 0) ar >> iChan;				n--;
		if (n > 0) ar >> iWaveform;			n--;
		if (n > 0) ar >> mseq_iRatio;		n--;	
		if (n > 0) ar >> mseq_iDelay;		n--;	
		if (n > 0) ar >> mseq_iSeed;		n--;
		if (n > 0) ar >> noise_iDelay;		n--;
		int idummy; while (n > 0) {n--; ar >> idummy;}

		// double parameters
		ar >> wn; n = wn;
		if (n > 0) ar >> dAmplitudeMaxV;	n--;
		if (n > 0) ar >> dAmplitudeMinV;	n--;
		if (n > 0) ar >> dFrequency;		n--;
		if (n > 0) ar >> dummy1;			n--;
		if (n > 0) ar >> dummy2;			n--;
		if (n > 0) ar >> noise_dAmplitV;	n--;
		if (n > 0) ar >> noise_dFactor;		n--;
		if (n > 0) ar >> noise_dOffsetV;	n--;
		if (n > 0) ar >> value;				n--;
		double ddummy; while (n > 0) {n--; ar >> ddummy;}

		// other?
		ar >> wn; n = wn;
		if (n > 0) { stimulussequence.Serialize(ar); n--; }
		for (int i = 0; i < 8; i++) 
		{
			if (n > 0) { 
				stim8lines[i].Serialize(ar); 
				n--; 
			}
		}
		if (n > 0) { sti.Serialize(ar); n--; }
		ASSERT(n==0);
	}
}




