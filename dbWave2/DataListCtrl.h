#pragma once

#include "chart.h"
#include "Envelope.h"
#include "chanlistitem.h"
#include "ChartData.h"
#include "DataListCtrl_Row.h"

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

class CDataListCtrl : public CListCtrl
{
public:
					CDataListCtrl();
	virtual			~CDataListCtrl() override;

	void			InitColumns(CUIntArray* picolwidth = nullptr);
	void			SetCurSel(int recposition);
	void			UpdateCache(int ifirst, int ilast);
	void			RefreshDisplay();
	void			ResizeSignalColumn(int npixels);
	void			FitColumnsToSize(int npixels);

	void			SetTransformMode(int imode) { m_dattransform = imode; }
	void			SetDisplayMode(int imode) { m_displaymode = imode; }
	void			SetTimeIntervals(float tfirst, float tlast) { m_tFirst = tfirst; m_tLast = tlast; }
	void			SetAmplitudeSpan(float vspan) { m_mVspan = vspan; } // TODO ; get extent, mode from current line
	void			SetDisplayFileName(BOOL flag) { m_bDisplayFileName = flag; }
	void			SetAmplitudeAdjustMode(BOOL flag) { m_bsetmVSpan = flag; }
	void			SetTimespanAdjustMode(BOOL flag) { m_bsetTimeSpan = flag; }
	void			SetSpikePlotMode(int imode, int iclass) { m_spikeplotmode = imode; m_selclass = iclass; }

	int				GetDisplayMode()		const { return m_displaymode; }
	float			GetTimeFirst()			const { return m_tFirst; }
	float			GetTimeLast()			const { return m_tLast; }
	float			GetAmplitudeSpan()		const { return m_mVspan; }
	int				GetSpikePlotMode()		const { return m_spikeplotmode; }
	int				GetSpikeClass()			const { return m_selclass; }

	CChartDataWnd*	GetDataViewCurrentRecord();
	inline int		GetVisibleRowsSize() {return ptrArray.GetSize();}
	inline CAcqDataDoc*	GetVisibleRowsAcqDataDocAt(int index) { return  ptrArray[index]->pdataDoc; }
	inline CSpikeDoc*	GetVisibleRowsSpikeDocAt(int index) { return ptrArray[index]->pspikeDoc; }

protected:
	CArray <CDataListCtrl_Row*, CDataListCtrl_Row*> ptrArray;
	CImageList		m_imagelist;
	static int		m_colwidth[NCOLS];
	static CString	m_colheaders[NCOLS];
	static int		m_colfmt[NCOLS];
	static int		m_colindex[NCOLS];

	CUIntArray*		m_picolwidth;
	CBitmap*		m_pEmptyBitmap;
	
	int				m_cx;			// image height
	int				m_cy;			// image width
	int				m_dattransform;
	int				m_displaymode;
	int				m_spikeplotmode;
	int				m_selclass;
	float			m_tFirst;
	float			m_tLast;
	float			m_mVspan;
	BOOL			m_bsetTimeSpan;
	BOOL			m_bsetmVSpan;
	BOOL			m_bDisplayFileName;

	void			deletePtrArray();
	void			resizePtrArray(int nitems);
	void			setEmptyBitmap(BOOL bForcedUpdate = FALSE);
	void			displaySpikeWnd(CDataListCtrl_Row* ptr, int iImage);
	void			displayDataWnd(CDataListCtrl_Row* ptr, int iImage);
	void			displayEmptyWnd(CDataListCtrl_Row* ptr, int iImage);
	
	// Generated message map functions
	afx_msg void OnGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDestroy();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);

	DECLARE_MESSAGE_MAP()
};
