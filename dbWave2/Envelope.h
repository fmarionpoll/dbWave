#pragma once


class CHighLight : public CObject
{
public:
	CHighLight();	
	DECLARE_SERIAL(CHighLight)
	void Serialize(CArchive& ar) override;
	CHighLight& operator = (const CHighLight& arg);	

public:
	int					channel = 0;
	COLORREF			color = RGB(255, 0, 0); // red
	int					pensize = 1;
	CArray <long, long> l_first;
	CArray <long, long> l_last;
};

class CEnvelope : public CObject
{
public:
				CEnvelope();				// protected constructor used by dynamic creation
				DECLARE_SERIAL(CEnvelope)

public:
				CEnvelope(WORD npixels);	// create Envelope with npoints
				CEnvelope(WORD npixels, int dataperpixel, int ns, int mode, int span);
	void		Serialize(CArchive& ar) override;

	// Attributes
protected:
	int			m_sourceMode;		// operation on raw data (nop, deriv, ...)
	int			m_sourceChan;		// source channel
	int			m_span;				// additionnal data pts necessary to compute transform
	int			m_dataperpixel;		// 2=simple envelope, 1=raw data
	int			m_npixels;			// nb of valid elements in the array
	CArray <short, short> m_Envelope;	// Envelope array
	short*		getMaxMin(int nelements, short* lpData, int nchans, short& i_min, short& i_max, long& y);

// Operations
public:
	void		FillEnvelopeWithAbcissa(int npixels, int npoints);
	void		FillEnvelopeWithAbcissaEx(int pixfirst, int pixlast, int ndatapoints);
	void		GetMeanToAbcissa(CArray<CPoint, CPoint>& dest);
	void		GetMeanToOrdinates(CArray<CPoint, CPoint>& dest);
	void		ExportToAbcissa(CArray<CPoint, CPoint>& dest);
	void		ExportToOrdinates(CArray<CPoint, CPoint>& dest);

	// Helper functions
public:
	//----------description
	inline int	GetDocbufferSpan() const { return m_span; }
	inline void SetDocbufferSpan(int span) { m_span = span; }

	//----------Envelope array
	inline short* GetElmtAdr(int i) { return &m_Envelope[i]; }
	inline int	GetSourceMode()const { return m_sourceMode; }
	inline int	GetSourceChan() const { return m_sourceChan; }
	inline int	GetEnvelopeSize() const { return m_Envelope.GetSize(); }
	inline int	GetnElements() const { return m_npixels; }
	inline int	GetAt(int j) const { return m_Envelope[j]; }
	inline void SetSourceMode(int n, int span) { m_sourceMode = n; m_span = span; }
	inline void SetSourceChan(int n) { m_sourceChan = n; }
	inline void SetPointAt(int j, int val) { m_Envelope[j] = val; }
	inline void SetSourceData(const int chan, const int transform) { m_sourceChan = chan; m_sourceMode = transform; }
	inline short GetPointAt(const int i) { return short(m_Envelope[i]); }

	void		SetEnvelopeSize(int npixels, int ndataperpixel);
	void		FillEnvelopeWithMxMi(int ifirst, short* lpData, int nchans, int nelmts, BOOL bNew);
	void		FillEnvelopeWithSmoothMxMi(int ifirst, short* lpData, int nchans, int nelmts, BOOL bNew, int ioption);
	void		GetEnvelopeMaxMin(int* max, int* min);
	void		GetEnvelopeMaxMinBetweenPoints(int ifirstpixel, int ilastpixel, int* max, int* min);
};
