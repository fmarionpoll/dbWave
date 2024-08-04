#include "StdAfx.h"
#include "OPTIONS_VIEWDATA.h"


IMPLEMENT_SERIAL(OPTIONS_VIEWDATA, CObject, 0 /* schema number*/)

OPTIONS_VIEWDATA::OPTIONS_VIEWDATA()
= default;

OPTIONS_VIEWDATA::~OPTIONS_VIEWDATA()
= default;

OPTIONS_VIEWDATA& OPTIONS_VIEWDATA::operator =(const OPTIONS_VIEWDATA& arg)
{
	if (this != &arg)
	{
		horzRes = arg.horzRes;
		vertRes = arg.vertRes;
		bAcqComment = arg.bAcqComment;
		bAcqDateTime = arg.bAcqDateTime;
		bChansComment = arg.bChansComment;
		bChanSettings = arg.bChanSettings;
		bDocName = arg.bDocName;
		bFrameRect = arg.bFrameRect;
		bClipRect = arg.bClipRect;
		bTimeScaleBar = arg.bTimeScaleBar;
		bVoltageScaleBar = arg.bVoltageScaleBar;
		HeightDoc = arg.HeightDoc;
		WidthDoc = arg.WidthDoc;
		leftPageMargin = arg.leftPageMargin;
		bottomPageMargin = arg.bottomPageMargin;
		rightPageMargin = arg.rightPageMargin;
		topPageMargin = arg.topPageMargin;
		heightSeparator = arg.heightSeparator;
		textseparator = arg.textseparator;
		fontsize = arg.fontsize;
		bAllChannels = arg.bAllChannels;
		b_complete_record = arg.b_complete_record;
		b_multiple_rows = arg.b_multiple_rows;
		bSplitCurves = arg.bSplitCurves;
		bMaximizeGain = arg.bMaximizeGain;
		bCenterCurves = arg.bCenterCurves;
		bKeepForEachFile = arg.bKeepForEachFile;
		bDispOldDetectP = arg.bDispOldDetectP;
		bFilterDataSource = arg.bFilterDataSource;
		bacqcomments = arg.bacqcomments;
		bacqdate = arg.bacqdate;
		bacqtime = arg.bacqtime;
		bfilesize = arg.bfilesize;
		bacqchcomment = arg.bacqchcomment;
		bacqchsetting = arg.bacqchsetting;
		bPrintSelection = arg.bPrintSelection;
		bPrintSpkBars = arg.bPrintSpkBars;
		bunits = arg.bunits;
		bcontours = arg.bcontours;
		bgraphics = arg.bgraphics;
		bdatabasecols = arg.bdatabasecols;
		btoExcel = arg.btoExcel;
		hzResolution = arg.hzResolution;
		vtResolution = arg.vtResolution;
		unitMode = arg.unitMode;
		spkheight = arg.spkheight;
		spkwidth = arg.spkwidth;
		viewdata = arg.viewdata;
		viewspkdetectfiltered = arg.viewspkdetectfiltered;
		viewspkdetectdata = arg.viewspkdetectdata;
		viewspkdetectspk = arg.viewspkdetectspk;
		viewspkdetectbars = arg.viewspkdetectbars;
		viewadcontvi = arg.viewadcontvi;
		spkviewdata = arg.spkviewdata;
		spkviewspk = arg.spkviewspk;
		spkviewbars = arg.spkviewbars;
		spksort1spk = arg.spksort1spk;
		spksort1parms = arg.spksort1parms;
		spksort1bars = arg.spksort1bars;
		spksort1hist = arg.spksort1hist;
		dbviewdata = arg.dbviewdata;
		bFilterDat = arg.bFilterDat;
		dbviewbars = arg.dbviewbars;
		dbviewshape = arg.dbviewshape;
		nfilterindex = arg.nfilterindex;
		bDetectWhileBrowse = arg.bDetectWhileBrowse;
		bGotoRecordID = arg.bGotoRecordID;
		col0width = arg.col0width;
		row0height = arg.row0height;
		bcol0visible = arg.bcol0visible;
		brow0visible = arg.brow0visible;
		irowheight = arg.irowheight;
		tFirst = arg.tFirst;
		tLast = arg.tLast;
		mVspan = arg.mVspan;
		displaymode = arg.displaymode;
		bDisplayFileName = arg.bDisplayFileName;
		bsetTimeSpan = arg.bsetTimeSpan;
		bsetmVSpan = arg.bsetmVSpan;
		bDisplayAllClasses = arg.bDisplayAllClasses;
		spikeclass = arg.spikeclass;
	}
	return *this;
}

void OPTIONS_VIEWDATA::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << m_wversion;

		// print options
		WORD wPrintFlags, wMult;
		wMult = 1;
		wPrintFlags = bAcqComment * wMult; //0
		wMult += wMult;
		wPrintFlags += bAcqDateTime * wMult; //2
		wMult += wMult;
		wPrintFlags += bChansComment * wMult; //4
		wMult += wMult;
		wPrintFlags += bChanSettings * wMult; //8
		wMult += wMult;
		wPrintFlags += bDocName * wMult; //16
		wMult += wMult;
		wPrintFlags += bFrameRect * wMult; //32
		wMult += wMult;
		wPrintFlags += bClipRect * wMult; //64
		wMult += wMult;
		wPrintFlags += bTimeScaleBar * wMult; //128
		wMult += wMult;
		wPrintFlags += bVoltageScaleBar * wMult; //256
		wMult += wMult;
		wPrintFlags += bPrintSelection * wMult; //512
		wMult += wMult;
		wPrintFlags += bPrintSpkBars * wMult; //1024
		wMult += wMult;
		wPrintFlags += bFilterDataSource * wMult; //2048
		ar << wPrintFlags;

		// print parameters
		ar << static_cast<WORD>(horzRes);
		ar << static_cast<WORD>(vertRes);
		ar << static_cast<WORD>(HeightDoc); // height of one document
		ar << static_cast<WORD>(WidthDoc); // width of one document
		ar << static_cast<WORD>(leftPageMargin); // page left margin (pixels)
		ar << static_cast<WORD>(bottomPageMargin); // bottom
		ar << static_cast<WORD>(rightPageMargin); // right
		ar << static_cast<WORD>(topPageMargin); // top
		ar << static_cast<WORD>(heightSeparator); // vertical space between consecutive displays
		ar << static_cast<WORD>(textseparator); // space between left margin of display and text
		ar << static_cast<WORD>(fontsize); // font size (pixels)

		// how data are displayed from file to file
		WORD wBrowseFlags;
		wMult = 1;
		wBrowseFlags = bAllChannels * wMult; //0
		wMult += wMult;
		wBrowseFlags += b_complete_record * wMult; //2
		wMult += wMult;
		wBrowseFlags += b_multiple_rows * wMult; //4
		wMult += wMult;
		wBrowseFlags += bSplitCurves * wMult; //8
		wMult += wMult;
		wBrowseFlags += bMaximizeGain * wMult; //16
		wMult += wMult;
		wBrowseFlags += bCenterCurves * wMult; //32
		wMult += wMult;
		wBrowseFlags += bKeepForEachFile * wMult; //64
		wMult += wMult;
		wBrowseFlags += bDispOldDetectP * wMult; //128
		wMult += wMult;
		wBrowseFlags += bFilterDat * wMult; //256
		wMult += wMult;
		wBrowseFlags += bDetectWhileBrowse * wMult; //512
		wMult += wMult;
		wBrowseFlags += bGotoRecordID * wMult; //1024
		wMult += wMult;
		wBrowseFlags += bDisplayFileName * wMult; //2048
		wMult += wMult;
		wBrowseFlags += bsetTimeSpan * wMult; //4096
		wMult += wMult;
		wBrowseFlags += bsetmVSpan * wMult; //8192
		wMult += wMult;
		wBrowseFlags += bDisplayAllClasses * wMult; //16384
		ar << wBrowseFlags;

		// export infos from data files
		WORD wInfoFlags;
		wMult = 1;
		wInfoFlags = bacqcomments * wMult; // 0
		wMult += wMult;
		wInfoFlags += bacqdate * wMult; // 2
		wMult += wMult;
		wInfoFlags += bacqtime * wMult; // 4
		wMult += wMult;
		wInfoFlags += bfilesize * wMult; // 8
		wMult += wMult;
		wInfoFlags += bacqchcomment * wMult; // 16
		wMult += wMult;
		wInfoFlags += bacqchsetting * wMult; // 32
		wMult += wMult;
		wInfoFlags += bunits * wMult; // 64
		wMult += wMult;
		wInfoFlags += bcontours * wMult; // 128
		wMult += wMult;
		wInfoFlags += bgraphics * wMult; // 256
		wMult += wMult;
		wInfoFlags += btoExcel * wMult; // 512
		wMult += wMult;
		wInfoFlags += bdatabasecols * wMult; // 1024
		ar << wInfoFlags;

		// export int(s) WORD = nb of items
		ar << static_cast<WORD>(13);
		ar << hzResolution; //1
		ar << vtResolution; //2
		ar << unitMode; //3
		ar << spkheight; //4
		ar << spkwidth; //5
		ar << nfilterindex; //6
		ar << col0width; //7
		ar << row0height; //8
		ar << bcol0visible; //9
		ar << brow0visible; //10
		ar << irowheight; //11
		ar << displaymode; //12
		ar << spikeclass; //13

		ar << icolwidth.GetSize();
		for (int ii = 0; ii < icolwidth.GetSize(); ii++)
			ar << icolwidth[ii];

		ar << 3; // nb floats
		ar << tFirst;
		ar << tLast;
		ar << mVspan;

		n_scope_items = 16;
		ar << n_scope_items;
		viewdata.Serialize(ar); //1
		viewspkdetectdata.Serialize(ar); //2
		viewspkdetectspk.Serialize(ar); //3
		viewspkdetectbars.Serialize(ar); //4
		viewadcontvi.Serialize(ar); //5
		spkviewdata.Serialize(ar); //6
		spkviewspk.Serialize(ar); //7
		spkviewbars.Serialize(ar); //8
		spksort1spk.Serialize(ar); //9
		spksort1parms.Serialize(ar); //10
		spksort1hist.Serialize(ar); //11
		dbviewdata.Serialize(ar); //12
		dbviewbars.Serialize(ar); //13
		dbviewshape.Serialize(ar); //14
		spksort1bars.Serialize(ar); //15
		viewspkdetectfiltered.Serialize(ar); //16
	}
	else
	{
		WORD version;
		ar >> version;

		// print options
		WORD wPrintFlags, wMult;
		ar >> wPrintFlags;
		wMult = 1;
		bAcqComment = ((wPrintFlags & wMult) > 0);
		wMult += wMult;
		bAcqDateTime = ((wPrintFlags & wMult) > 0);
		wMult += wMult;
		bChansComment = ((wPrintFlags & wMult) > 0);
		wMult += wMult;
		bChanSettings = ((wPrintFlags & wMult) > 0);
		wMult += wMult;
		bDocName = ((wPrintFlags & wMult) > 0);
		wMult += wMult;
		bFrameRect = ((wPrintFlags & wMult) > 0);
		wMult += wMult;
		bClipRect = ((wPrintFlags & wMult) > 0);
		wMult += wMult;
		bTimeScaleBar = ((wPrintFlags & wMult) > 0);
		wMult += wMult;
		bVoltageScaleBar = ((wPrintFlags & wMult) > 0);
		wMult += wMult;
		bPrintSelection = ((wPrintFlags & wMult) > 0);
		wMult += wMult;
		bPrintSpkBars = ((wPrintFlags & wMult) > 0);
		wMult += wMult;
		bFilterDataSource = ((wPrintFlags & wMult) > 0);

		// print parameters
		WORD w1;
		ar >> w1;
		horzRes = w1;
		ar >> w1;
		vertRes = w1;
		ar >> w1;
		HeightDoc = w1;
		ar >> w1;
		WidthDoc = w1;
		ar >> w1;
		leftPageMargin = w1;
		ar >> w1;
		bottomPageMargin = w1;
		ar >> w1;
		rightPageMargin = w1;
		ar >> w1;
		topPageMargin = w1;
		ar >> w1;
		heightSeparator = w1;
		ar >> w1;
		textseparator = w1;
		ar >> w1;
		fontsize = w1;

		// how data are displayed from file to file
		WORD wBrowseFlags;
		ar >> wBrowseFlags;
		wMult = 1;
		bAllChannels = ((wBrowseFlags & wMult) > 0);
		wMult += wMult;
		b_complete_record = ((wBrowseFlags & wMult) > 0);
		wMult += wMult;
		b_multiple_rows = ((wBrowseFlags & wMult) > 0);
		wMult += wMult;
		bSplitCurves = ((wBrowseFlags & wMult) > 0);
		wMult += wMult;
		bMaximizeGain = ((wBrowseFlags & wMult) > 0);
		wMult += wMult;
		bCenterCurves = ((wBrowseFlags & wMult) > 0);
		wMult += wMult;
		bKeepForEachFile = ((wBrowseFlags & wMult) > 0);
		wMult += wMult;
		bDispOldDetectP = ((wBrowseFlags & wMult) > 0);
		wMult += wMult;
		bFilterDat = ((wBrowseFlags & wMult) > 0);
		wMult += wMult;
		bDetectWhileBrowse = ((wBrowseFlags & wMult) > 0);
		wMult += wMult;
		bGotoRecordID = ((wBrowseFlags & wMult) > 0);
		wMult += wMult;
		bDisplayFileName = ((wBrowseFlags & wMult) > 0);
		wMult += wMult;
		bsetTimeSpan = ((wBrowseFlags & wMult) > 0);
		wMult += wMult;
		bsetmVSpan = ((wBrowseFlags & wMult) > 0);
		wMult += wMult;
		bDisplayAllClasses = ((wBrowseFlags & wMult) > 0);

		// export textual infos from data files
		WORD wInfoFlags;
		ar >> wInfoFlags;
		wMult = 1;
		bacqcomments = ((wInfoFlags & wMult) > 0);
		wMult += wMult;
		bacqdate = ((wInfoFlags & wMult) > 0);
		wMult += wMult;
		bacqtime = ((wInfoFlags & wMult) > 0);
		wMult += wMult;
		bfilesize = ((wInfoFlags & wMult) > 0);
		wMult += wMult;
		bacqchcomment = ((wInfoFlags & wMult) > 0);
		wMult += wMult;
		bacqchsetting = ((wInfoFlags & wMult) > 0);
		wMult += wMult;
		bunits = ((wInfoFlags & wMult) > 0);
		wMult += wMult;
		bcontours = ((wInfoFlags & wMult) > 0);
		wMult += wMult;
		bgraphics = ((wInfoFlags & wMult) > 0);
		wMult += wMult;
		btoExcel = ((wInfoFlags & wMult) > 0);
		wMult += wMult;
		bdatabasecols = ((wInfoFlags & wMult) > 0);

		if (version > 4)
		{
			WORD n_int_items;
			ar >> n_int_items;
			int nb_INT_items = n_int_items;
			ar >> hzResolution;
			nb_INT_items--; //1
			ar >> vtResolution;
			nb_INT_items--; //2
			ar >> unitMode;
			nb_INT_items--; //3
			ar >> spkheight;
			nb_INT_items--; //4
			ar >> spkwidth;
			nb_INT_items--; //5
			ar >> nfilterindex;
			nb_INT_items--; //6
			ar >> col0width;
			nb_INT_items--; //7
			ar >> row0height;
			nb_INT_items--; //8
			ar >> bcol0visible;
			nb_INT_items--; //9
			ar >> brow0visible;
			nb_INT_items--; //10
			ar >> irowheight;
			nb_INT_items--; //11
			ar >> displaymode;
			nb_INT_items--; //12
			if (nb_INT_items > 0) ar >> spikeclass;
			nb_INT_items--;

			ar >> nb_INT_items;
			icolwidth.SetSize(nb_INT_items);
			for (auto ii = 0; ii < nb_INT_items; ii++)
				ar >> icolwidth[ii];
			int n_float_items;
			ar >> n_float_items;
			ar >> tFirst;
			n_float_items--;
			ar >> tLast;
			n_float_items--;
			ar >> mVspan;
			n_float_items--;

			ar >> n_scope_items;
			if (n_scope_items > 0)
			{
				viewdata.Serialize(ar);
				n_scope_items--;
				viewspkdetectdata.Serialize(ar);
				n_scope_items--;
				viewspkdetectspk.Serialize(ar);
				n_scope_items--;
				viewspkdetectbars.Serialize(ar);
				n_scope_items--;
				viewadcontvi.Serialize(ar);
				n_scope_items--;
				spkviewdata.Serialize(ar);
				n_scope_items--;
				spkviewspk.Serialize(ar);
				n_scope_items--;
				spkviewbars.Serialize(ar);
				n_scope_items--;
				spksort1spk.Serialize(ar);
				n_scope_items--;
				spksort1parms.Serialize(ar);
				n_scope_items--;
				spksort1hist.Serialize(ar);
				n_scope_items--;
				if (n_scope_items > 0)
				{
					dbviewdata.Serialize(ar);
					n_scope_items--;
				}
				if (n_scope_items > 0)
				{
					dbviewbars.Serialize(ar);
					n_scope_items--;
				}
				if (n_scope_items > 0)
				{
					dbviewshape.Serialize(ar);
					n_scope_items--;
				}
				if (n_scope_items > 0)
				{
					spksort1bars.Serialize(ar);
					n_scope_items--;
				}
				if (n_scope_items > 0)
				{
					viewspkdetectfiltered.Serialize(ar);
					n_scope_items--;
				}
				ASSERT(n_scope_items == 0);
			}
		}
		else if (version >= 2)
		{
			WORD ww1;
			ar >> ww1;
			int nb_int_items = ww1;
			if (nb_int_items > 0) ar >> hzResolution;
			nb_int_items--;
			if (nb_int_items > 0)
			{
				ar >> vtResolution;
				nb_int_items--;
				ar >> unitMode;
				nb_int_items--;
			}
			if (nb_int_items > 0)
			{
				ar >> spkheight;
				nb_int_items--;
				ar >> spkwidth;
				nb_int_items--;
			}
			if (nb_int_items > 0)
				ar >> nfilterindex;
			nb_int_items--;
			if (nb_int_items > 0)
			{
				for (auto ii = 0; ii < 8; ii++)
				{
					ar >> icolwidth[ii];
					nb_int_items--;
				}
			}
			if (nb_int_items > 0)
			{
				ar >> col0width;
				nb_int_items--;
				ar >> row0height;
				nb_int_items--;
				ar >> bcol0visible;
				nb_int_items--;
				ar >> brow0visible;
				nb_int_items--;
			}

			ar >> n_scope_items;
			if (n_scope_items > 0)
			{
				viewdata.Serialize(ar);
				n_scope_items--;
				viewspkdetectdata.Serialize(ar);
				n_scope_items--;
				viewspkdetectspk.Serialize(ar);
				n_scope_items--;
				viewspkdetectbars.Serialize(ar);
				n_scope_items--;
				viewadcontvi.Serialize(ar);
				n_scope_items--;
				spkviewdata.Serialize(ar);
				n_scope_items--;
				spkviewspk.Serialize(ar);
				n_scope_items--;
				spkviewbars.Serialize(ar);
				n_scope_items--;
				spksort1spk.Serialize(ar);
				n_scope_items--;
				spksort1parms.Serialize(ar);
				n_scope_items--;
				spksort1hist.Serialize(ar);
				n_scope_items--;
				if (n_scope_items > 0)
				{
					dbviewdata.Serialize(ar);
					n_scope_items--;
				}
				if (n_scope_items > 0)
				{
					dbviewbars.Serialize(ar);
					n_scope_items--;
				}
				if (n_scope_items > 0)
				{
					dbviewshape.Serialize(ar);
					n_scope_items--;
				}
				if (n_scope_items > 0)
				{
					spksort1bars.Serialize(ar);
					n_scope_items--;
				}
				if (n_scope_items > 0)
				{
					viewspkdetectfiltered.Serialize(ar);
					n_scope_items--;
				}
				ASSERT(n_scope_items == 0);
			}
		}
	}
}
