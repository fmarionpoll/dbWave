#pragma once

class CSpkListTabCtrl : public CTabCtrl
{
	DECLARE_DYNAMIC(CSpkListTabCtrl)

public:
	CSpkListTabCtrl();
	~CSpkListTabCtrl() override;

	void init_ctrl_tab_from_spike_doc(CSpikeDoc* p_spk_doc);


protected:
	DECLARE_MESSAGE_MAP()
};
