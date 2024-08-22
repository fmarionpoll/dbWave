#pragma once
#include <Olmem.h>
#include "dtacq32.h"
#include "options_input.h"
#include "options_output.h"
#include "output_parameters.h"


class DataTranslation_DA :
	public CDTAcq32
{
public:
	BOOL OpenSubSystem(const CString& card_name);
	BOOL ClearAllOutputs();
	void SetChannelList();
	BOOL InitSubSystem(const options_input* pADC_options);
	void DeleteBuffers();
	void DeclareAndFillBuffers(const options_input* pADC_options);
	void FillBufferWith_SINUSOID(short* pDTbuf, int chan, output_parameters* outputparms_array);
	void FillBufferWith_SQUARE(short* pDTbuf, int chan, output_parameters* outputparms_array);
	void FillBufferWith_TRIANGLE(short* pDTbuf, int chan, output_parameters* outputparms_array);
	void FillBufferWith_RAMP(short* pDTbuf, int chan, output_parameters* outputparms_array);
	void FillBufferWith_CONSTANT(short* pDTbuf, int chan, output_parameters* outputparms_array);
	void FillBufferWith_ONOFFSeq(short* pDTbuf, int chan, output_parameters* outputparms_array);
	void MSequence(BOOL start, output_parameters* outputparms_array);
	void FillBufferWith_MSEQ(short* pDTbuf, int chan, output_parameters* outputparms_array);
	void ConvertbufferFrom2ComplementsToOffsetBinary(short* pDTbuf, int chan);

	void Dig_FillBufferWith_SQUARE(short* pDTbuf, int chan, output_parameters* outputparms_array);
	void Dig_FillBufferWith_ONOFFSeq(short* pDTbuf, int chan, output_parameters* outputparms_array);
	void Dig_FillBufferWith_MSEQ(short* pDTbuf, int chan, output_parameters* outputparms_array);

	void FillBuffer(short* pDTbuf);
	void ConfigAndStart();
	void StopAndLiberateBuffers();

protected:
	options_output _options_OutputData; // = nullptr; // pointer to data output options
	int m_digitalchannel = 0;
	BOOL m_digitalfirst = false;
	int m_listsize = 0;
	long m_msbit = 0;
	long m_lRes = 0;

	BOOL m_is_DA_in_progress = false; // D/A in progress
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
	bool IsInProgress() const { return m_is_DA_in_progress; }
	void SetInProgress() { m_is_DA_in_progress = true; }
};
