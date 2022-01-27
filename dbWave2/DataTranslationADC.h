#pragma once
#include <olmem.h>

#include "dtacq32.h"
class DataTranslationADC :
    public CDTAcq32
{
public:
	BOOL	ADC_OpenSubSystem(CString cardName);
	BOOL	ADC_InitSubSystem(OPTIONS_ACQDATA* pADC_options);
	void	ADC_DeleteBuffers();
	void	ADC_DeclareBuffers();
	void	ADC_Transfer(short* pDTbuf);
	void	ADC_StopAndLiberateBuffers();
	void	ADC_Start();
	void	ADC_OnBufferDone();

protected:
	void DTLayerError(COleDispatchException* e);


	OPTIONS_ACQDATA* m_pADC_options = nullptr;
	BOOL	m_ADC_inprogress = false;
	HBUF	m_ADC_bufhandle = nullptr;
	long	m_ADC_buflen = 0;
	long	m_ADC_chbuflen = 0;
	double 	m_ADC_freqmax = 50000.;			// maximum sampling frequency (Hz)
	int		m_ADC_numchansMAX = 8;
	BOOL	m_bsimultaneousStartAD = false;


public:
	bool	ADC_IsInProgress() const { return m_ADC_inprogress; }
	void	ADC_SetInProgress() { m_ADC_inprogress = true; }
	double	ADC_GetMaximumFrequency() const { return m_ADC_freqmax; }
	int		ADC_GetMaximumNumberOfChannels() const { return m_ADC_numchansMAX; }
	BOOL	ADC_IsSimultaneousStart() { return m_bsimultaneousStartAD; }
};

