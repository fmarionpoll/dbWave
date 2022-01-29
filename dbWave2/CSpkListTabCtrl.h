#pragma once

class CSpkListTabCtrl : public CTabCtrl
{
	DECLARE_DYNAMIC(CSpkListTabCtrl)

public:
	CSpkListTabCtrl();
	~CSpkListTabCtrl() override;

	void InitctrlTabFromSpikeDoc(CSpikeDoc* pSpkDoc);


protected:
	DECLARE_MESSAGE_MAP()
};
