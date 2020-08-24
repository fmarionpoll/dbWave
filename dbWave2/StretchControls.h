/////////////////////////////////////////////////////////////////////////////
// classe controlprop

#if !defined (_CONTROLP_H_)
#define _CONTROLP_H_

#define SZEQ_XLEQ		1	// equal size -- constant distance with x left
#define SZEQ_XREQ		2	// equal size -- constant dist with x right
#define XLEQ_XREQ		3	// equal dist from both sides -- size accordingly
#define SZEQ_XLPR		4	// equal size -- proportional distance from left border
#define SZEQ_XRPR		5	// equal size -- proportional distance from right border
#define XLPR_XRPR		6	// proportional distance from both borders -- size accordingly
#define SZPR_XLEQ		7
#define SZPR_XREQ		8
#define RIGHT_BAR		9

#define SZEQ_YTEQ		1	// equal size -- constant distance with y top
#define SZEQ_YBEQ		2	// equal size -- constant dist with y bottom
#define YTEQ_YBEQ		3	// equal dist from both sides -- size accordingly
#define SZEQ_YTPR		4	// equal size -- proportional distance from top border
#define SZEQ_YBPR		5	// equal size -- proportional distance from bottom border
#define YTPR_YBPR		6	// proportional distance from both borders -- size accordingly
#define SZPR_YTEQ		7
#define SZPR_YBEQ		8
#define BOTTOM_BAR		9
/*
// Définitions supplémentaires
// afin de raccorder les objets de AppStudio entre eux.
// pour la coordonnée y

#define SZEQ_YDS		10	    // equal size -- constant distance with IDC_DISPLAYSOURCE
#define SZPR_YDS		11		// proportional size -- constant distance with IDC_DISPLAYSOURCE
#define SZEQ_YDM		12		// equal size -- constant distance with IDC_DISPLAYMODIF
#define SZPR_YDM		13		// proportional size -- constant distance with IDC_DISPLAYMODIF
#define SZEQ_YDH		14		// equal size -- constant distance with IDC_DISPLAYHISTOG
#define SZPR_YDH		15		// proportional size -- constant distance with IDC_DISPLAYHISTOG
*/
/////////////////////////////////////////////////////////////////////////////
// CCtrlProp class
class CCtrlProp :public CObject
{
protected:
public:
	CCtrlProp();
	CCtrlProp(int iID, HWND hC, int xSizeHow, int ySizeHow, CRect* rect);
	DECLARE_DYNCREATE(CCtrlProp)

	// Attributes
public:
	int		m_ctrlID;		// id of the control
	int		m_xSizeHow;		// flag: how window x position is affected by wm_size
	int		m_ySizeHow;		// flag: how window y position is affected by wm_size
	HWND 	m_hWnd;			// handle of window
	BOOL	m_bVisible;		// state of the window
	int		m_slaveorder;	// 0: no master, 1: 1 master; 2: 2 cascaded masters, etc.
	int		m_master;		// ID master from which it is slave; else -1
	CRect	m_rect;			// distances from initial dlg borders
	CRect	m_rmaster;		// master rect dialog client coords
	CRect	m_rect0;
	CRect	m_rmaster0;

	// Operations
public:

	// Helper functions
public:
	int		GetID() const { return m_ctrlID; }
	HWND	GetHWnd() const { return m_hWnd; }
	int		GetxSizeHow() const { return m_xSizeHow; }
	int		GetySizeHow() const { return m_ySizeHow; }
	BOOL	IsVisible() const { return m_bVisible; }
	int		GetMaster() const { return m_master; }
	CRect	GetMasterRect() const { return m_rmaster; }

	void	SetID(int ID) { m_ctrlID = ID; }
	void	SetHWnd(HWND hC) { m_hWnd = hC; }
	void	SetxSizeHow(int xSizeHow) { m_xSizeHow = xSizeHow; }
	void	SetySizeHow(int ySizeHow) { m_ySizeHow = ySizeHow; }
	void	SetRectLimits(CRect* rect) { m_rect = *rect; }
	void	SetVisible(BOOL bVisible) { m_bVisible = bVisible; }
	void	SetMaster(int index) { m_master = index; }
	void	SetMasterRect(CRect rmaster) { m_rmaster = rmaster; }
};

/////////////////////////////////////////////////////////////////////////////
// CStretchControl class
class CStretchControl :public CObject
{
public:
	CStretchControl();
	DECLARE_DYNCREATE(CStretchControl)
	virtual ~CStretchControl();

	// Attributes
protected:
	CWnd* m_parent;			// handle to window parent
	CArray < CCtrlProp*, CCtrlProp*> ctrlprop_ptr_array; 				// CCtrlProps props relative to resizable controls
	SIZE	m_DialogSize;			// current dialog size
	SIZE	m_DlgMinSize;			// minimal dialog size
	int 	m_BOTTOMBARHeight;		// height of system scrollbars
	int 	m_RIGHTBARWidth; 		// width of system scrollbars
	BOOL	m_bHBAR;				// true: horizontal bar and associated controls
	BOOL	m_bVBAR;				// true: vertical bar and associated controls
	int		m_slavemax;

public:
	void AttachParent(CFormView* pF);
	void AttachDialogParent(CDialog* p_wnd);
	BOOL newProp(int iID, int xsizeMode, int ysizeMode);
	BOOL newProp(CWnd* p_wnd, int iID, int xsizeMode, int ysizeMode);

	BOOL newSlaveProp(int iID, int xsizeSlave, int ysizeSlave, int iMASTER);
	BOOL GetbVBarControls() const { return m_bVBAR; }
	BOOL GetbHBarControls() const { return m_bHBAR; }
	BOOL DisplayVBarControls(BOOL bVisible);
	BOOL DisplayHBarControls(BOOL bVisible);
	void ChangeBarSizes(int RIGHTwidth, int BOTTOMheight);
	void UpdateClientControls();
	void ResizeControls(UINT nType, int cx, int cy);

	// Attributes
public:
	CSize GetDlgMinSize() const { return m_DlgMinSize; }
	CSize GetDialogSize() const { return m_DialogSize; }
	int   GetBOTTOMBARHeight() const { return m_BOTTOMBARHeight; }
	int   GetRIGHTBARWidth() const { return m_RIGHTBARWidth; }
	// Implementation
protected:
	CRect AlignControl(CCtrlProp* pa, int cx, int cy);
	void AreaLeftbyControls(int* cx, int* cy);
};

#endif // _CONTROLP_H_
