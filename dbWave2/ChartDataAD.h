#pragma once
#include "ChartData.h"

class ChartDataAD :
	public ChartData
{
public:
	void start_display(int points_per_channel);
	void display_buffer(short* samples_buffer, long points_per_channel);
	void stop_display() { m_b_ad_buffers_ = FALSE; }

};
