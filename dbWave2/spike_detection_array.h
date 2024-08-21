#pragma once
#include "SpikeDetectArray.h"

// array of spike detection parameters
// 2 dimensions

class spike_detection_array : public CObject
{
	DECLARE_SERIAL(spike_detection_array)
public:
	BOOL b_changed; // flag set TRUE if contents has changed
	void set_chan_array(int acq_chan, SpikeDetectArray* p_spk);
	SpikeDetectArray* get_chan_array(int acq_chan);

protected:
	WORD w_version_;
	CMapWordToPtr chan_array_map_; 
	void delete_all(); 

public:
	spike_detection_array();
	~spike_detection_array() override;
	void Serialize(CArchive& ar) override;
};

