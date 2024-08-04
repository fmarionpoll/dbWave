#pragma once


class CHighLight : public CObject
{
public:
	CHighLight();
	DECLARE_SERIAL(CHighLight)
	void Serialize(CArchive& ar) override;
	CHighLight& operator =(const CHighLight& arg);

public:
	int channel = 0;
	COLORREF color = RGB(255, 0, 0); // red
	int pensize = 1;
	CArray<long, long> l_first;
	CArray<long, long> l_last;
};

class CEnvelope : public CObject
{
public:
	CEnvelope(); 
	DECLARE_SERIAL(CEnvelope)

public:
	CEnvelope(WORD npixels); // create Envelope with npoints
	CEnvelope(WORD npixels, int dataperpixel, int ns, int mode, int span);
	void Serialize(CArchive& ar) override;

	// Attributes
protected:
	int m_sourceMode; // operation on raw data (nop, deriv, ...)
	int m_sourceChan; // source channel
	int m_span; // additionnal data pts necessary to compute transform
	int m_dataperpixel; // 2=simple envelope, 1=raw data
	int m_npixels; // nb of valid elements in the array
	CArray<short, short> m_Envelope; // Envelope array
	short* getMaxMin(int nelements, short* lpData, int nchans, short& i_min, short& i_max, long& y);

	// Operations
public:
	void fill_envelope_with_abscissa(int npixels, int npoints);
	void fill_envelope_with_abscissa_ex(int pixfirst, int pixlast, int ndatapoints);
	void get_mean_to_abscissa(CArray<CPoint, CPoint>& dest);
	void GetMeanToOrdinates(CArray<CPoint, CPoint>& dest);
	void ExportToAbscissa(CArray<CPoint, CPoint>& dest);
	void ExportToOrdinates(CArray<CPoint, CPoint>& dest);

	//----------description
	int GetDocbufferSpan() const { return m_span; }
	void SetDocbufferSpan(int span) { m_span = span; }

	//----------Envelope array
	short* GetElmtAdr(int i) { return &m_Envelope[i]; }
	int GetSourceMode() const { return m_sourceMode; }
	int GetSourceChan() const { return m_sourceChan; }
	int GetEnvelopeSize() const { return m_Envelope.GetSize(); }
	int GetnElements() const { return m_npixels; }
	int GetAt(int j) const { return m_Envelope[j]; }

	void SetSourceMode(int n, int span)
	{
		m_sourceMode = n;
		m_span = span;
	}

	void SetSourceChan(int n) { m_sourceChan = n; }
	void SetPointAt(int j, int val) { m_Envelope[j] = val; }

	void set_envelope_source_data(const int chan, const int transform)
	{
		m_sourceChan = chan;
		m_sourceMode = transform;
	}

	short GetPointAt(const int i) { return static_cast<short>(m_Envelope[i]); }

	void SetEnvelopeSize(int npixels, int ndataperpixel);
	void FillEnvelopeWithMxMi(int ifirst, short* lpData, int nchans, int nelmts, BOOL bNew);
	void FillEnvelopeWithSmoothMxMi(int ifirst, short* lpData, int nchans, int nelmts, BOOL bNew, int ioption);
	void GetEnvelopeMaxMin(int* max, int* min);
	void GetEnvelopeMaxMinBetweenPoints(int ifirstpixel, int ilastpixel, int* max, int* min);
};
