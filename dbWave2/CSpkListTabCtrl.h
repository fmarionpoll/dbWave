#pragma once

// CSpkListTabCtrl

class CSpkListTabCtrl : public CTabCtrl
{
	DECLARE_DYNAMIC(CSpkListTabCtrl)

public:
	CSpkListTabCtrl();
	virtual ~CSpkListTabCtrl();

	void InitctrlTabFromSpikeDoc(CSpikeDoc* pSpkDoc);


protected:
	DECLARE_MESSAGE_MAP()
};


