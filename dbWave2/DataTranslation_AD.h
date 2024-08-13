#pragma once
#include <Olmem.h>
#include "dtacq32.h"
#include "options_input.h"


class DataTranslation_AD final :
	public CDTAcq32
{
public:
	BOOL OpenSubSystem(CString card_name);
	BOOL InitSubSystem(options_input* pADC_options);
	void DeleteBuffers();
	void DeclareBuffers(options_input* pADC_options);
	void StopAndLiberateBuffers();
	void ConfigAndStart();
	short* OnBufferDone();
	long VoltsToValue(float fVolts, double dfGain);
	float ValueToVolts(long lVal, double dfGain);

protected:
	void DTLayerError(COleDispatchException* e);

	options_input* m_pOptions = nullptr;
	BOOL m_inprogress = false;
	HBUF m_bufhandle = nullptr;
	long m_buflen = 0;
	long m_chbuflen = 0;
	double m_freqmax = 50000.;
	int m_numchansMAX = 8;
	BOOL m_bsimultaneousStart_AD = false;

public:
	bool IsInProgress() const { return m_inprogress; }
	void SetInProgress() { m_inprogress = true; }
	double GetMaximumFrequency() const { return m_freqmax; }
	int GetMaximumNumberOfChannels() const { return m_numchansMAX; }
	BOOL IsSimultaneousStart() const { return m_bsimultaneousStart_AD; }
	long Getchbuflen() const { return m_chbuflen; }
	long Getbuflen() const { return m_buflen; }
	void ReleaseLastBufferToQueue() { SetQueue(long(m_bufhandle)); }
};
