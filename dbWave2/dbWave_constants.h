#pragma once

// dbWave_constants.h : main header file for the DBWAVE application
// Oct 23, 2008 - FMP
// store all constants on one place

// private message ID
#define WM_MYMESSAGE WM_USER+1000

// constants
// toolbar 
#define WM_TOOLBARTYPE WM_USER+1

// private message ID
#define WM_MYMESSAGE WM_USER+1000

// constants
#define DOC_UNID				0
#define DOC_TEXT				1
#define DOC_DATA				2
#define DOC_SPIKE				4

#define CURSOR_ARROW			0	// m_cursorType
#define CURSOR_ZOOM				1
#define CURSOR_MEASURE			2
#define CURSOR_VERTICAL			3

#define TRACK_OFF				0	// m_trackmode
#define TRACK_HZTAG				1
#define TRACK_VTTAG				2
#define TRACK_RECT				3
#define TRACK_CURVE				4
#define TRACK_SPIKE				5
#define TRACK_BASELINE			6

// hint used with MY_MESSAGE

#define HINT_HITAREA			0	// hit button area, nothing detected lowp = null
#define HINT_SETMOUSECURSOR		1	// change mouse cursor shape	lowp = cursor index
#define HINT_HITCHANNEL			2	// chan hit 					lowp = channel
#define HINT_CHANGEHZLIMITS		3	// horizontal limits have changed lowp = unused
#define HINT_HITSPIKE			4	// spike item hit,				lowp = spike nb
#define HINT_SELECTSPIKES		5
#define HINT_DBLCLKSEL			6	// doubleclick selection		lowp = item selected
#define HINT_MOVECURSORLEFT		7	// move hz cursor(left side)	lowp = LP value
#define HINT_MOVECURSORRIGHT	8	// move hz cursor(right side)	lowp = LP value
#define HINT_OFFSETCURSOR		9	// move hz cursor(both sides)   lowp = diff / previous val
#define HINT_CHANGEZOOM			10	// change zoom factor

#define HINT_HITVERTTAG			11	// vertical tag hit				lowp = tag index	
#define HINT_MOVEVERTTAG		12	// vertical tag has moved 		lowp = new pixel / selected tag
#define HINT_CHANGEVERTTAG		13	// vertical tag has changed 	lowp = tag nb

#define HINT_HITHZTAG			14	// horiz tag hit				lowp = cursor index	
#define HINT_MOVEHZTAG			15	// horiz tag has moved 			lowp = new cursor value (pixel)
#define HINT_CHANGEHZTAG		16	// horizontal tag has changed 	lowp = tag nb
#define HINT_DEFINEDRECT		17	// rect defined					lowp = unused; limits: CFButton::GetDefinedRect()
#define HINT_DROPPED			18	// mouse left button is up over (drop object that was dragged)
#define HINT_RMOUSEBUTTONDOWN	19
#define HINT_RMOUSEBUTTONUP		20
#define HINT_WINDOWPROPSCHANGED	21

#define HINT_DEFSPIKETIME		22
#define HINT_CHGSPIKETIME		23
#define HINT_ADDSPIKE			24
#define HINT_ADDSPIKEFORCED		25

#define HINT_HITSPIKE_CTRL		26	// hit spike with Ctrl key down		lowp = spike nb
#define HINT_HITSPIKE_SHIFT		27	// hit spike with SHIFT key down	lowp = spike nb

#define HINT_SHAREDMEMFILLED	29	// shared memory file was created and waits for being displayed
#define HINT_LMOUSEBUTTONDOW_CTRL	30	// CScopeCtrl when mode is "arrow" and CTRL is down when the mouse left button is up

#define HINT_DOCISABOUTTOCHANGE	100	// doc will change
#define HINT_DOCHASCHANGED		101 // file has changed
#define HINT_VIEWSIZECHANGED	102 // change zoom
#define HINT_CLOSEFILEMODIFIED	103	// modified file will be closed
#define HINT_CHANGEVIEW			104
#define HINT_DOCMOVERECORD		105 // MOVE TO a different record (data+spk file)
//#define HINT_DOCSELECTRECORD	106 // select a different record (data+spk file)
#define HINT_CHANGERULERSCROLL	107
#define HINT_TRACKRULERSCROLL	108

#define HINT_FILTERREMOVED		109

#define HINT_RECORDGOTO			200	// select database record
#define HINT_REQUERY			201	// requery database - refresh list
#define HINT_TOGGLELEFTPANE		202
#define HINT_REPLACEVIEW		203	// 0=dat; 1 = spk
#define HINT_GETSELECTEDRECORDS 204 // get currently selected records (from a CListCtrl)
#define HINT_SETSELECTEDRECORDS 205	// select records (in a CListCtrl)
#define HINT_MDIACTIVATE		206	// MDI activate (to send to PropertiesListWnd)
#define HINT_RECORDMODE			207	// tells if we enter into record mode (then hide properties)
#define HINT_ACTIVATEVIEW		208	// message sent when view is activated

// parameters used to control bars
#define BAR_BIAS		1
#define BAR_GAIN		4

// parameters used with UpdateLegends()
#define	UPD_ABCISSA		1		// update abcissa (1st, last pt)
#define CHG_XSCALE		2		// change scale bar channel
#define UPD_XSCALE		4		// update x scale
#define CHG_XBAR 		8		// change bar length

#define UPD_ORDINATES 	16		// update ordinates (not implemented yet)
#define CHG_YSCALE		32		// change scale bar channel
#define UPD_YSCALE		64		// update y scale
#define CHG_YBAR		128		// change bar height

// number of digits used when printing
#define FLT_DIG     6     /* # of decimal digits of precision */	

// default values used to update bias scroll 
#define YZERO_MIN -16350
#define YZERO_MAX 16350
#define YZERO_LINE 32
#define YZERO_PAGE 320
#define YZERO_SPAN 32700
#define YEXTENT_MIN 1
#define YEXTENT_MAX 65535

