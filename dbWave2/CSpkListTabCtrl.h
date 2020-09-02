#pragma once

// CSpkListTabCtrl

class CSpkListTabCtrl : public CTabCtrl
{
	DECLARE_DYNAMIC(CSpkListTabCtrl)

public:
	CSpkListTabCtrl();
	virtual ~CSpkListTabCtrl();

	void InitctrlTabFromSpikeList(CdbWaveDoc* pDoc);

	void SetCurSpkList(CdbWaveDoc* pDoc);

protected:
	DECLARE_MESSAGE_MAP()
};


