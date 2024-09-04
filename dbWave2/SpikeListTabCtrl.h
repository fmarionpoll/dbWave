#pragma once

class SpikeListTabCtrl : public CTabCtrl
{
	DECLARE_DYNAMIC(SpikeListTabCtrl)

public:
	SpikeListTabCtrl();
	~SpikeListTabCtrl() override;

	void init_ctrl_tab_from_spike_doc(CSpikeDoc* p_spk_doc);


protected:
	DECLARE_MESSAGE_MAP()
};
