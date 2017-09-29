#pragma once

// WaveBuf.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CWaveBuf window

#define MOVAVG30 13

class CWaveBuf : public CObject
{
	DECLARE_SERIAL(CWaveBuf);
	friend class CAcqDataDoc;

// Attributes
protected:
	short*	GetWBAdrRawDataBuf();
	short*  GetWBAdrRawDataElmt(int chan, int index);
	short*	GetWBAdrTransfData();
	short*	GetWBAdrTransfDataElmt(int index);
public:
	int		GetWBNumElements();
	int		GetWBNumChannels();
	void	SetWBSampleRate(float fSampRate);
	float	GetWBSampleRate();
	BOOL 	GetWBVoltsperBin(int zbchanIndex, float* VoltsperBin, int mode = 0);

//operations
	CWaveBuf();
	virtual ~CWaveBuf();
	virtual void Serialize( CArchive& archive );
	int		WBDatachanSetnum(int i);

	// Transform Data
	//------------	
	CString GetWBTransformsAllowed(int i);
	BOOL 	IsWBTransformAllowed(int i);
	WORD 	GetWBNTypesofTransforms();
	BOOL 	InitWBTransformMode(int i);
	int		GetWBTransformSpan(int i);
	int		IsWBSpanChangeAllowed(int i);
	int		SetWBTransformSpan(int i, int span);
	int		GetWBcorrectionFact(int i, float *correct);

	// Transformations
	//----------------
	void 	BDeriv   (short* lpSource, short* lpDest, int cx);
	void 	BCopy    (short* lpSource, short* lpDest, int cx);
	void 	BLanczo2 (short* lpSource, short* lpDest, int cx);
	void 	BDiffer1 (short* lpSource, short* lpDest, int cx);	
	void 	BDiffer2 (short* lpSource, short* lpDest, int cx);
	void 	BDiffer3 (short* lpSource, short* lpDest, int cx);
	void 	BDiffer10(short* lpSource, short* lpDest, int cx);
	
	void 	BLanczo3 (short* lpSource, short* lpDest, int cx);
	void 	BDeri1f3 (short* lpSource, short* lpDest, int cx);
	void 	BDeri2f3 (short* lpSource, short* lpDest, int cx);
	void 	BDeri2f5 (short* lpSource, short* lpDest, int cx);
	void 	BDeri3f3 (short* lpSource, short* lpDest, int cx);
	void 	BMovAvg30(short* lpSource, short* lpDest, int cx);
	void	BMedian30(short* lpSource, short* lpDest, int cx);
	void	BMedian35(short* lpSource, short* lpDest, int cx);
	void	BMedian  (short* lpSource, short* lpDest, int cx, int nspan);
	void	BRMS     (short* lpSource, short* lpDest, int cx);

private:    
	void DeleteBuffers();

	// data
	//-----

protected:	
	BOOL	CreateWBuffer(int iNumElements, int nchannels=1);
	CWaveChanArray m_chanArray;				// array of structures with the channel description
	CWaveFormat    m_waveFormat;			// structure with data acquisition def & parameters

	static int		m_maxtransform;			// number of transmformation allowed
	static char*	m_pTransformsAllowed[];	// ASCII description of each transformation
	static int		m_TransformBufferSpan[];	// size of sliding window necessary to filter data
	static float	m_correctionFact[];		// correction factor to transform binary data into voltage
	static int		m_bvariableSpan[];		// flag to tell if the sliding window size can be changed

private:
	//HANDLE	m_hWData;
	//HANDLE	m_hWTransf;
	short*	m_pWData;						// Pointer to the origin of the primary data array
	short*	m_pWTransf;						// primary transform buffer

	BOOL	m_bTransf;
	WORD	m_wversion;
	int		m_iNumElements;					// n elements within buffer
	size_t	m_dwBufferSize;
	short*	m_parraySorted;					// array used by BMedian to store a sorted array of data
	short*	m_parrayCircular;				// array used by BMedian to store a sliding window array of data
	int		m_parray_size;
};


