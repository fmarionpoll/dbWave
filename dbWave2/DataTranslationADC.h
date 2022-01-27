#pragma once
#include <olmem.h>

#include "dtacq32.h"
class DataTranslationADC :
    public CDTAcq32
{
public:
	BOOL	ADC_OpenSubSystem(CString cardName);
	BOOL	ADC_InitSubSystem();
	void	ADC_DeleteBuffers();
	void	ADC_DeclareBuffers();
	void	ADC_Transfer(short* pDTbuf);
	void	ADC_StopAndLiberateBuffers();

protected:
	void DTLayerError(COleDispatchException* e);


	OPTIONS_ACQDATA* m_pADC_options = nullptr;
	BOOL	m_ADC_inprogress = false;
	HBUF	m_ADC_bufhandle = nullptr;
	long	m_ADC_buflen = 0;
	long	m_ADC_chbuflen = 0;
};

