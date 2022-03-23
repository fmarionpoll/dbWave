#pragma once

#include <string>
#include "AcqWaveFormat.h"
#include "AcqWaveChanArray.h"
#include "Taglist.h"

constexpr auto MOVAVG30 = 13;

class CWaveBuf : public CObject
{
	DECLARE_SERIAL(CWaveBuf);
	friend class AcqDataDoc;

	// Attributes
protected:
	short* getWBAdrRawDataBuf() const;
	short* getWBAdrRawDataElmt(int chan, int index) const;
	short* getWBAdrTransfData() const;
	short* getWBAdrTransfDataElmt(int index) const;

public:
	int GetWBNumElements() const;
	int GetWBNumChannels() const;
	void SetWBSampleRate(float fSampRate);
	float GetWBSampleRate() const;
	BOOL GetWBVoltsperBin(int ch_index, float* volts_per_bin, int mode = 0) const;
	CTagList* GetpHZtags() { return &m_hz_tags; }
	CTagList* GetpVTtags() { return &m_vt_tags; }
	CWaveChanArray* GetpWavechanArray() { return &m_chanArray; }
	CWaveFormat* GetpWaveFormat() { return &m_waveFormat; }

	//operations
	CWaveBuf();
	~CWaveBuf() override;
	void Serialize(CArchive& ar) override;
	int WBDatachanSetnum(int i);

	// Transform Data
	//------------
	static CString GetWBTransformsAllowed(int i);
	static BOOL IsWBTransformAllowed(int i);
	static WORD GetWBNTypesofTransforms();
	BOOL InitWBTransformBuffer();
	static int GetWBTransformSpan(int i);
	static int IsWBSpanChangeAllowed(int i);
	static int SetWBTransformSpan(int i, int span);
	static int GetWBcorrectionFactor(int i, float* correct);

	// Transformations
	//----------------
	void BDeriv(const short* lp_source, short* lp_dest, int cx) const;
	void BCopy(const short* lp_source, short* lp_dest, int cx) const;
	void BLanczo2(const short* lp_source, short* lp_dest, int cx) const;
	void BDiffer1(const short* lp_source, short* lp_dest, int cx) const;
	void BDiffer2(const short* lp_source, short* lp_dest, int cx) const;
	void BDiffer3(const short* lp_source, short* lp_dest, int cx) const;
	void BDiffer10(const short* lp_source, short* lp_dest, int cx) const;

	void BLanczo3(const short* lp_source, short* lp_dest, int cx) const;
	void BDeri1f3(const short* lp_source, short* lp_dest, int cx) const;
	void BDeri2f3(const short* lp_source, short* lp_dest, int cx) const;
	void BDeri2f5(const short* lp_source, short* lp_dest, int cx) const;
	void BDeri3f3(const short* lp_source, short* lp_dest, int cx) const;
	void BMovAvg30(short* lp_source, short* lp_dest, int cx) const;
	void BMedian30(short* lp_source, short* lp_dest, int cx);
	void BMedian35(short* lp_source, short* lp_dest, int cx);
	void BMedian(short* lp_source, short* lp_dest, int cx, int nspan);
	void BRMS(short* lp_source, short* lp_dest, int cx) const;

private:
	void deleteBuffers();

protected:
	BOOL createWBuffer(int i_num_elements, int n_channels = 1);

	CWaveChanArray m_chanArray{}; // array of structures with the channel description
	CWaveFormat m_waveFormat{}; // structure with data acquisition def & parameters
	CTagList m_hz_tags{}; // list of horizontal cursors
	CTagList m_vt_tags{}; // list of vertical tags


	static int m_maxtransform; // number of transformations allowed
	static std::string m_pTransformsAllowed[]; // ASCII description of each transformation
	static int m_TransformBufferSpan[]; // size of sliding window necessary to filter data
	static float m_correctionFact[]; // correction factor to transform binary data into voltage
	static int m_bvariableSpan[]; // flag to tell if the sliding window size can be changed

private:
	short* m_pWData = nullptr; // Pointer to the origin of the primary data array
	short* m_pWTransf = nullptr; // primary transform buffer
	BOOL m_bTransf = FALSE;
	WORD m_wversion = 0;
	int m_iNumElements = 0; // n elements within buffer
	size_t m_dwBufferSize = 0;
	short* m_parraySorted = nullptr; // array used by BMedian to store a sorted array of data
	short* m_parrayCircular = nullptr; // array used by BMedian to store a sliding window array of data
	int m_parray_size = 0;
};
