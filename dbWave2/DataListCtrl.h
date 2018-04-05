#pragma once

// DataListCtrl.h : header file
//

#include "scopescr.h"
#include "Envelope.h" 
#include "chanlistitem.h"
#include "lineview.h"
#include "DataListCtrlRowObject.h"

#define COL_INDEX		1
#define COL_CURVE		COL_INDEX+1
#define COL_INSECT		COL_INDEX+2
#define COL_SENSI		COL_INDEX+3
#define COL_STIM1		COL_INDEX+4
#define COL_CONC1		COL_INDEX+5
#define COL_STIM2		COL_INDEX+6
#define COL_CONC2		COL_INDEX+7
#define COL_NBSPK		COL_INDEX+8
#define COL_FLAG		COL_INDEX+9

/////////////////////////////////////////////////////////////////////////////
// CDataListCtrl window
class CAcqDataDoc;

class CDataListCtrl : public CListCtrl
{
// Construction
public:
	CDataListCtrl();

// Implementation
public:
	virtual ~CDataListCtrl();
	void	InitColumns(CUIntArray* picolwidth = NULL);
	void	SetCurSel(int recposition);
	void	UpdateCache (int ifirst, int ilast);
	void	RefreshDisplay();
	void	ResizeSignalColumn(int npixels);
	void	FitColumnsToSize(int npixels);

	inline void		SetTransformMode(int imode) {m_dattransform = imode;}
	inline void		SetDisplayMode(int imode) {m_displaymode = imode;}
	inline void		SetTimeIntervals(float tfirst, float tlast) {m_tFirst=tfirst; m_tLast=tlast;}
	inline void		SetAmplitudeSpan(float vspan) {m_mVspan = vspan;}
	inline void		SetDisplayFileName(BOOL flag) {m_bDisplayFileName = flag;}
	inline void		SetAmplitudeAdjustMode(BOOL flag) {m_bsetmVSpan = flag;}
	inline void		SetTimespanAdjustMode(BOOL flag) { m_bsetTimeSpan = flag;}
	inline void		SetSpikePlotMode(int imode, int iclass) {m_spikeplotmode = imode; m_selclass=iclass;}
	
	inline int		GetDisplayMode() const {return m_displaymode;}
	inline float	GetTimeFirst() const {return m_tFirst;}
	inline float	GetTimeLast() const {return m_tLast;}
	inline float	GetAmplitudeSpan() const {return m_mVspan;}
	inline int		GetSpikePlotMode() const {return m_spikeplotmode;}
	inline int		GetSpikeClass() const {return m_selclass;}

	// Generated message map functions
protected:
	CPtrArray	m_ptrArray;		// array of cached data
	CImageList	m_imagelist;	// list of bitmap images
	static int	m_icolwidth[NCOLS];
	CUIntArray*	m_picolwidth;

	int			m_cx;			// image height
	int			m_cy;			// image width
	int			m_dattransform;
	int			m_displaymode;
	int			m_spikeplotmode;
	int			m_selclass;
	float		m_tFirst;
	float		m_tLast;
	float		m_mVspan;
	BOOL		m_bsetTimeSpan;
	BOOL		m_bsetmVSpan;

	void		DeletePtrArray();
	void		ResizePtrArray(int nitems);
	void		SetEmptyBitmap(BOOL bForcedUpdate = FALSE);
	CBitmap*	m_pEmptyBitmap;
	void		DisplaySpikeWnd (CDataListCtrlRowObject* ptr, int iImage);
	void		DisplayDataWnd  (CDataListCtrlRowObject* ptr, int iImage);
	void		DisplayEmptyWnd (CDataListCtrlRowObject* ptr, int iImage);

	BOOL		m_bDisplayFileName;
	
	// Generated message map functions
protected:
	afx_msg void OnGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDestroy();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	DECLARE_MESSAGE_MAP()

};

