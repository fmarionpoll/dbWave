#pragma once
#include <olmem.h>

#include "dtacq32.h"
class DataTranslation_AD :
    public CDTAcq32
{
public:
	BOOL	OpenSubSystem(CString card_name);
	BOOL	InitSubSystem(OPTIONS_ACQDATA* pADC_options);
	void	DeleteBuffers();
	void	DeclareBuffers();
	void	Transfer(short* pDTbuf);
	void	StopAndLiberateBuffers();
	void	Start();
	short*	OnBufferDone();

protected:
	void	DTLayerError(COleDispatchException* e);


	OPTIONS_ACQDATA* m_pOptions = nullptr;
	BOOL	m_inprogress = false;
	HBUF	m_bufhandle = nullptr;
	long	m_buflen = 0;
	long	m_chbuflen = 0;
	double 	m_freqmax = 50000.;			// maximum sampling frequency (Hz)
	int		m_numchansMAX = 8;
	BOOL	m_bsimultaneousStart_AD = false;


public:
	bool	IsInProgress() const { return m_inprogress; }
	void	SetInProgress() { m_inprogress = true; }
	double	GetMaximumFrequency() const { return m_freqmax; }
	int		GetMaximumNumberOfChannels() const { return m_numchansMAX; }
	BOOL	IsSimultaneousStart() { return m_bsimultaneousStart_AD; }
};

