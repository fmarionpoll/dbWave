#pragma once
#include <olmem.h>

#include "dtacq32.h"
class DataTranslationDAC :
    public CDTAcq32
{
public:
	BOOL	DAC_OpenSubSystem(CString cardName);
	BOOL	DAC_ClearAllOutputs();
	void	DAC_SetChannelList();
	BOOL	DAC_InitSubSystem();
	void	DAC_DeleteBuffers();
	void	DAC_DeclareAndFillBuffers();
	void	DAC_FillBufferWith_SINUSOID(short* pDTbuf, int chan, OUTPUTPARMS* outputparms_array);
	void	DAC_FillBufferWith_SQUARE(short* pDTbuf, int chan, OUTPUTPARMS* outputparms_array);
	void	DAC_FillBufferWith_TRIANGLE(short* pDTbuf, int chan, OUTPUTPARMS* outputparms_array);
	void	DAC_FillBufferWith_RAMP(short* pDTbuf, int chan, OUTPUTPARMS* outputparms_array);
	void	DAC_FillBufferWith_CONSTANT(short* pDTbuf, int chan, OUTPUTPARMS* outputparms_array);
	void	DAC_FillBufferWith_ONOFFSeq(short* pDTbuf, int chan, OUTPUTPARMS* outputparms_array);
	void	DAC_MSequence(BOOL start, OUTPUTPARMS* outputparms_array);
	void	DAC_FillBufferWith_MSEQ(short* pDTbuf, int chan, OUTPUTPARMS* outputparms_array);
	void	DAC_ConvertbufferFrom2ComplementsToOffsetBinary(short* pDTbuf, int chan);

	void	DAC_Dig_FillBufferWith_SQUARE(short* pDTbuf, int chan, OUTPUTPARMS* outputparms_array);
	void	DAC_Dig_FillBufferWith_ONOFFSeq(short* pDTbuf, int chan, OUTPUTPARMS* outputparms_array);
	void	DAC_Dig_FillBufferWith_MSEQ(short* pDTbuf, int chan, OUTPUTPARMS* outputparms_array);

	void	DAC_FillBuffer(short* pDTbuf);
	void	DAC_StopAndLiberateBuffers();

protected:
	OPTIONS_OUTPUTDATA* m_pDAC_options = nullptr;	// pointer to data output options
	int		m_DACdigitalchannel = 0;
	BOOL	m_DACdigitalfirst = false;
	int		m_DAClistsize = 0;
	long	m_DACmsbit = 0;
	long	m_DAClRes = 0;

	BOOL	m_DAC_inprogress = false;	// D/A in progress
	HBUF	m_DAC_bufhandle = nullptr;
	long	m_DAC_buflen = 0;			// nb of acq sample per DT buffer
	long	m_DAC_chbuflen = 0;
	BOOL	m_bsimultaneousStartDA = false;
	long	m_DAC_nBuffersFilledSinceStart = 0;
	double	m_DAC_frequency = 1.;

	void DTLayerError(COleDispatchException* e);

public:
	void OnBufferDone_DAC();
};

