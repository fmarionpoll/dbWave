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
constexpr auto CURSOR_ARROW = 0; // m_cursorType;
constexpr auto CURSOR_ZOOM = 1;
constexpr auto CURSOR_CROSS = 2;
constexpr auto CURSOR_VERTICAL = 3;
constexpr auto CURSOR_RESIZE_HZ = 4;
constexpr auto CURSOR_RESIZE_VERT = 5;

// tracking
constexpr auto TRACK_OFF = 0; // m_trackmode
constexpr auto TRACK_HZ_TAG = 1;
constexpr auto TRACK_VT_TAG = 2;
constexpr auto TRACK_RECT = 3;
constexpr auto TRACK_CURVE = 4;
constexpr auto TRACK_SPIKE = 5;
constexpr auto TRACK_BASELINE = 6;

// hint used with MY_MESSAGE
constexpr auto HINT_HIT_AREA = 0;			// hit button area, nothing detected lowp = null;
constexpr auto HINT_SET_MOUSE_CURSOR = 1;	// change mouse cursor shape	lowp = cursor index;
constexpr auto HINT_HIT_CHANNEL = 2;		// chan hit 					lowp = channel;
constexpr auto HINT_CHANGE_HZ_LIMITS = 3;	// horizontal limits have changed lowp = unused;
constexpr auto HINT_HIT_SPIKE = 4;			// spike item hit,				lowp = spike nb;
constexpr auto HINT_SELECT_SPIKES = 5;
constexpr auto HINT_DBL_CLK_SEL = 6;		// doubleclick selection		lowp = item selected
constexpr auto HINT_MOVE_CURSOR_LEFT = 7;	// move hz cursor(left side)	lowp = LP value
constexpr auto HINT_MOVE_CURSOR_RIGHT = 8;	// move hz cursor(right side)	lowp = LP value
constexpr auto HINT_OFFSET_CURSOR = 9;		// move hz cursor(both sides)   lowp = diff / previous value
constexpr auto HINT_CHANGE_ZOOM = 10;		// change zoom factor

constexpr auto HINT_HIT_VERT_TAG = 11;		// vertical tag hit				lowp = tag index
constexpr auto HINT_MOVE_VERT_TAG = 12;		// vertical tag has moved 		lowp = new pixel / selected tag
constexpr auto HINT_CHANGE_VERT_TAG = 13;	// vertical tag has changed 	lowp = tag nb

constexpr auto HINT_HIT_HZ_TAG = 14;		// horiz tag hit				lowp = cursor index
constexpr auto HINT_MOVE_HZ_TAG = 15;		// horiz tag has moved 			lowp = new cursor value (pixel)
constexpr auto HINT_CHANGE_HZ_TAG = 16;		// horizontal tag has changed 	lowp = tag nb
constexpr auto HINT_DEFINED_RECT = 17;		// rect defined					lowp = unused; limits: CFButton::GetDefinedRect()
constexpr auto HINT_DROPPED = 18;			// mouse left button is up over (drop object that was dragged)
constexpr auto HINT_R_MOUSE_BUTTON_DOWN = 19;
constexpr auto HINT_R_MOUSE_BUTTON_UP = 20;
constexpr auto HINT_WINDOW_PROPS_CHANGED = 21;

constexpr auto HINT_DEF_SPIKE_TIME = 22;
constexpr auto HINT_CHG_SPIKE_TIME = 23;
constexpr auto HINT_ADD_SPIKE = 24;
constexpr auto HINT_ADD_SPIKE_FORCED = 25;

constexpr auto HINT_HIT_SPIKE_CTRL = 26;	// hit spike with Ctrl key down		lowp = spike nb
constexpr auto HINT_HIT_SPIKE_SHIFT = 27;	// hit spike with SHIFT key down	lowp = spike nb

constexpr auto HINT_SHARED_MEM_FILLED = 29; // shared memory file was created and waits for being displayed
constexpr auto HINT_L_MOUSE_BUTTON_DOWN_CTRL = 30;
// CScopeCtrl when mode is "arrow" and CTRL is down when the mouse left button is up

constexpr auto HINT_VIEW_TAB_CHANGE = 50;	// bottom view tab will change
constexpr auto HINT_VIEW_TAB_HAS_CHANGED = 51; // bottom view tab will change

constexpr auto HINT_DOC_IS_ABOUT_TO_CHANGE = 100; // doc will change
constexpr auto HINT_DOC_HAS_CHANGED = 101;	// file has changed
constexpr auto HINT_VIEW_SIZE_CHANGED = 102; // change zoom
constexpr auto HINT_CLOSE_FILE_MODIFIED = 103; // modified file will be closed
constexpr auto HINT_CHANGE_VIEW = 104;
constexpr auto HINT_DOC_MOVE_RECORD = 105;	// MOVE TO a different record (data+spk file)
//constexpr auto HINT_DOC_SELECT_RECORD =	106;	// select a different record (data+spk file)
constexpr auto HINT_CHANGE_RULER_SCROLL = 107;
constexpr auto HINT_TRACK_RULER_SCROLL = 108;
constexpr auto HINT_FILTER_REMOVED = 109;

constexpr auto HINT_RECORD_GOTO = 200;		// select database record
constexpr auto HINT_REQUERY = 201;			// requery database - refresh list
constexpr auto HINT_TOGGLE_LEFT_PANE = 202;
constexpr auto HINT_REPLACE_VIEW = 203;		// 0=dat; 1 = spk
constexpr auto HINT_GET_SELECTED_RECORDS = 204; // get currently selected records (from a CListCtrl)
constexpr auto HINT_SET_SELECTED_RECORDS = 205; // select records (in a CListCtrl)
constexpr auto HINT_MDI_ACTIVATE = 206;		// MDI activate (to send to PropertiesListWnd);
constexpr auto HINT_RECORD_MODE = 207;		// tells if we enter into record mode (then hide properties);
constexpr auto HINT_ACTIVATE_VIEW = 208;	// message sent when view is activated
constexpr auto HINT_SAVE_SPIKEFILE = 209;	// save spike list along with modified spike classes

// control bars
constexpr auto BAR_BIAS = 1;
constexpr auto BAR_GAIN = 4;

// used with UpdateLegends()
constexpr auto UPD_ABSCISSA = 1; // update abscissa (1st, last pt)
constexpr auto CHG_X_SCALE = 2; // change scale bar channel
constexpr auto UPD_X_SCALE = 4; // update x scale
constexpr auto CHG_X_BAR = 8; // change bar length

constexpr auto UPD_ORDINATES = 16; // update ordinates (not implemented yet);
constexpr auto CHG_Y_SCALE = 32; // change scale bar channel;
constexpr auto UPD_Y_SCALE = 64; // update y scale;
constexpr auto CHG_Y_BAR = 128; // change bar height;

// update bias scroll
constexpr auto Y_ZERO_MIN = -16350;
constexpr auto Y_ZERO_MAX = 16350;
constexpr auto Y_ZERO_LINE = 32;
constexpr auto Y_ZERO_PAGE = 320;
constexpr auto Y_ZERO_SPAN = 32700;
constexpr auto Y_EXTENT_MIN = 1;
constexpr auto Y_EXTENT_MAX = 65535;

// splitted cursor
constexpr auto SPLIT_SIZE = 1;
constexpr auto TRACK_SIZE = 3;

constexpr auto MOVE_RIGHT = 1;
constexpr auto MOVE_BOTTOM = 1;
constexpr auto MOVE_LEFT = 2;
constexpr auto MOVE_TOP = 2;
constexpr auto MOVE_BOTH = 3;
constexpr auto RESIZE = 4;
