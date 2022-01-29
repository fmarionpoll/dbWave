#pragma once
#include "ChartData.h"

class ChartDataAD :
	public ChartData
{
public:
	// display given buffer operation
	void ADdisplayStart(int chsamplesw);
	void ADdisplayBuffer(short* samples_buffer, long nchsamples);
	void ADdisplayStop() { m_bADbuffers = FALSE; }

};
