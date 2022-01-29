#pragma once
#include <olmem.h>
#include "dtacq32.h"
#include "OPTIONS_OUTPUTDATA.h"
#include "OUTPUTPARMS.h"


class DataTranslation_DA :
	public CDTAcq32
{
public:
	BOOL OpenSubSystem(CString cardName);
	BOOL ClearAllOutputs();
	void SetChannelList();
	BOOL InitSubSystem(double ADC_channel_samplingrate, int ADC_trigger_mode);
	void DeleteBuffers();
	void DeclareAndFillBuffers(float sweepduration, float chrate, int nbuffers);
	void FillBufferWith_SINUSOID(short* pDTbuf, int chan, OUTPUTPARMS* outputparms_array);
	void FillBufferWith_SQUARE(short* pDTbuf, int chan, OUTPUTPARMS* outputparms_array);
	void FillBufferWith_TRIANGLE(short* pDTbuf, int chan, OUTPUTPARMS* outputparms_array);
	void FillBufferWith_RAMP(short* pDTbuf, int chan, OUTPUTPARMS* outputparms_array);
	void FillBufferWith_CONSTANT(short* pDTbuf, int chan, OUTPUTPARMS* outputparms_array);
	void FillBufferWith_ONOFFSeq(short* pDTbuf, int chan, OUTPUTPARMS* outputparms_array);
	void MSequence(BOOL start, OUTPUTPARMS* outputparms_array);
	void FillBufferWith_MSEQ(short* pDTbuf, int chan, OUTPUTPARMS* outputparms_array);
	void ConvertbufferFrom2ComplementsToOffsetBinary(short* pDTbuf, int chan);

	void Dig_FillBufferWith_SQUARE(short* pDTbuf, int chan, OUTPUTPARMS* outputparms_array);
	void Dig_FillBufferWith_ONOFFSeq(short* pDTbuf, int chan, OUTPUTPARMS* outputparms_array);
	void Dig_FillBufferWith_MSEQ(short* pDTbuf, int chan, OUTPUTPARMS* outputparms_array);

	void FillBuffer(short* pDTbuf);
	void ConfigAndStart();
	void StopAndLiberateBuffers();

protected:
	OPTIONS_OUTPUTDATA* m_pOptions = nullptr; // pointer to data output options
	int m_digitalchannel = 0;
	BOOL m_digitalfirst = false;
	int m_listsize = 0;
	long m_msbit = 0;
	long m_lRes = 0;

	BOOL m_inprogress = false; // D/A in progress
	HBUF m_bufhandle = nullptr;
	long m_buflen = 0; // nb of acq sample per DT buffer
	long m_chbuflen = 0;
	BOOL m_bsimultaneousStart_DA = false;
	long m_nBuffersFilledSinceStart = 0;
	double m_frequency = 1.;

	void DispatchException(COleDispatchException* e);

public:
	void OnBufferDone();
	int GetDigitalChannel() const { return m_digitalchannel; }
	bool IsInProgress() const { return m_inprogress; }
	void SetInProgress() { m_inprogress = true; }
};
