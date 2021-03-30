#pragma once

// dbWave_constants.h : main header file for the DBWAVE application
// Oct 23, 2008 - FMP
// store constants

// toolbar
#define WM_TOOLBARTYPE WM_USER+1

// private message ID
#define WM_MYMESSAGE WM_USER+1000

// document type
constexpr auto DOC_UNID = 0;
constexpr auto DOC_TEXT = 1;
constexpr auto DOC_DATA = 2;
constexpr auto DOC_SPIKE = 4;

// type of mouse cursor
constexpr auto CURSOR_ARROW = 0;				// m_cursorType;
constexpr auto CURSOR_ZOOM = 1;
constexpr auto CURSOR_CROSS = 2;
constexpr auto CURSOR_VERTICAL = 3;
constexpr auto CURSOR_RESIZE_HZ = 4;
constexpr auto CURSOR_RESIZE_VERT = 5;

// tracking
constexpr auto TRACK_OFF = 0;					// m_trackmode
constexpr auto TRACK_HZTAG = 1;
constexpr auto TRACK_VTTAG = 2;
constexpr auto TRACK_RECT = 3;
constexpr auto TRACK_CURVE = 4;
constexpr auto TRACK_SPIKE = 5;
constexpr auto TRACK_BASELINE = 6;

// hint used with MY_MESSAGE
constexpr auto HINT_HITAREA = 0;				// hit button area, nothing detected lowp = null;
constexpr auto HINT_SETMOUSECURSOR = 1;			// change mouse cursor shape	lowp = cursor index;
constexpr auto HINT_HITCHANNEL = 2;				// chan hit 					lowp = channel;
constexpr auto HINT_CHANGEHZLIMITS = 3;			// horizontal limits have changed lowp = unused;
constexpr auto HINT_HITSPIKE = 4;				// spike item hit,				lowp = spike nb;
constexpr auto HINT_SELECTSPIKES = 5;
constexpr auto HINT_DBLCLKSEL = 6;				// doubleclick selection		lowp = item selected
constexpr auto HINT_MOVECURSORLEFT = 7;			// move hz cursor(left side)	lowp = LP value
constexpr auto HINT_MOVECURSORRIGHT = 8;		// move hz cursor(right side)	lowp = LP value
constexpr auto HINT_OFFSETCURSOR = 9;			// move hz cursor(both sides)   lowp = diff / previous value
constexpr auto HINT_CHANGEZOOM = 10;			// change zoom factor

constexpr auto HINT_HITVERTTAG = 11;			// vertical tag hit				lowp = tag index
constexpr auto HINT_MOVEVERTTAG = 12;			// vertical tag has moved 		lowp = new pixel / selected tag
constexpr auto HINT_CHANGEVERTTAG = 13;			// vertical tag has changed 	lowp = tag nb

constexpr auto HINT_HITHZTAG = 14;				// horiz tag hit				lowp = cursor index
constexpr auto HINT_MOVEHZTAG = 15;				// horiz tag has moved 			lowp = new cursor value (pixel)
constexpr auto HINT_CHANGEHZTAG = 16;			// horizontal tag has changed 	lowp = tag nb
constexpr auto HINT_DEFINEDRECT = 17;			// rect defined					lowp = unused; limits: CFButton::GetDefinedRect()
constexpr auto HINT_DROPPED = 18;				// mouse left button is up over (drop object that was dragged)
constexpr auto HINT_RMOUSEBUTTONDOWN = 19;
constexpr auto HINT_RMOUSEBUTTONUP = 20;
constexpr auto HINT_WINDOWPROPSCHANGED = 21;

constexpr auto HINT_DEFSPIKETIME = 22;
constexpr auto HINT_CHGSPIKETIME = 23;
constexpr auto HINT_ADDSPIKE = 24;
constexpr auto HINT_ADDSPIKEFORCED = 25;

constexpr auto HINT_HITSPIKE_CTRL = 26;			// hit spike with Ctrl key down		lowp = spike nb
constexpr auto HINT_HITSPIKE_SHIFT = 27;		// hit spike with SHIFT key down	lowp = spike nb

constexpr auto HINT_SHAREDMEMFILLED = 29;		// shared memory file was created and waits for being displayed
constexpr auto HINT_LMOUSEBUTTONDOW_CTRL = 30;	// CScopeCtrl when mode is "arrow" and CTRL is down when the mouse left button is up

constexpr auto HINT_VIEWTABCHANGE = 50;			// bottom view tab will change
constexpr auto HINT_VIEWTABHASCHANGED = 51;		// bottom view tab will change

constexpr auto HINT_DOCISABOUTTOCHANGE = 100;	// doc will change
constexpr auto HINT_DOCHASCHANGED = 101;		// file has changed
constexpr auto HINT_VIEWSIZECHANGED = 102;		// change zoom
constexpr auto HINT_CLOSEFILEMODIFIED = 103;	// modified file will be closed
constexpr auto HINT_CHANGEVIEW = 104;
constexpr auto HINT_DOCMOVERECORD = 105;		// MOVE TO a different record (data+spk file)
//constexpr auto HINT_DOCSELECTRECORD =	106;	// select a different record (data+spk file)
constexpr auto HINT_CHANGERULERSCROLL = 107;
constexpr auto HINT_TRACKRULERSCROLL = 108;
constexpr auto HINT_FILTERREMOVED = 109;

constexpr auto HINT_RECORDGOTO = 200;			// select database record
constexpr auto HINT_REQUERY = 201;				// requery database - refresh list
constexpr auto HINT_TOGGLELEFTPANE = 202;
constexpr auto HINT_REPLACEVIEW = 203;			// 0=dat; 1 = spk
constexpr auto HINT_GETSELECTEDRECORDS = 204;	// get currently selected records (from a CListCtrl)
constexpr auto HINT_SETSELECTEDRECORDS = 205;	// select records (in a CListCtrl)
constexpr auto HINT_MDIACTIVATE = 206;			// MDI activate (to send to PropertiesListWnd);
constexpr auto HINT_RECORDMODE = 207;			// tells if we enter into record mode (then hide properties);
constexpr auto HINT_ACTIVATEVIEW = 208;			// message sent when view is activated;

// control bars
constexpr auto BAR_BIAS = 1;
constexpr auto BAR_GAIN = 4;

// used with UpdateLegends()
constexpr auto 	UPD_ABCISSA = 1;				// update abcissa (1st, last pt)
constexpr auto  CHG_XSCALE = 2;					// change scale bar channel
constexpr auto  UPD_XSCALE = 4;					// update x scale
constexpr auto  CHG_XBAR = 8;					// change bar length

constexpr auto UPD_ORDINATES = 16;				// update ordinates (not implemented yet);
constexpr auto CHG_YSCALE = 32;					// change scale bar channel;
constexpr auto UPD_YSCALE = 64;					// update y scale;
constexpr auto CHG_YBAR = 128;					// change bar height;

// update bias scroll
constexpr auto YZERO_MIN = -16350;
constexpr auto YZERO_MAX = 16350;
constexpr auto YZERO_LINE = 32;
constexpr auto YZERO_PAGE = 320;
constexpr auto YZERO_SPAN = 32700;
constexpr auto YEXTENT_MIN = 1;
constexpr auto YEXTENT_MAX = 65535;

// splitted cursor
constexpr auto SPLITSIZE = 1;
constexpr auto TRACKSIZE = 3;

constexpr auto MOVERIGHT = 1;
constexpr auto MOVEBOTTOM = 1;
constexpr auto MOVELEFT = 2;
constexpr auto MOVETOP = 2;
constexpr auto MOVEBOTH = 3;
constexpr auto RESIZE = 4;