#pragma once

// envelope.h

#include "cscale.h"


/////////////////////////////////////////////////////////////////////////////
// CEnvelope class

class CEnvelope : public CObject
{   
public:
	CEnvelope();				// protected constructor used by dynamic creation
	DECLARE_SERIAL(CEnvelope)

public:
	CEnvelope(WORD npixels);	// create Envelope with npoints	
	CEnvelope(WORD npixels, int dataperpixel, int ns, int mode, int span);
	virtual void Serialize(CArchive& ar);

// Attributes
protected:
	int		m_sourceMode;		// operation on raw data (nop, deriv, ...)
	int		m_sourceChan;		// source channel	
	int		m_span;				// additionnal data pts necessary to compute transform
	int		m_dataperpixel;		// 2=simple envelope, 1=raw data 
	int		m_npixels;			// nb of valid elements in the array
	CWordArray m_Envelope;		// Envelope array
	
// Operations
public:	
	void	FillEnvelopeWithAbcissa (int npixels, int npoints);
	void	FillEnvelopeWithAbcissaEx (int pixfirst, int pixlast, int ndatapoints);
	void	GetMeantoPolypoints(long* array);

// Helper functions
public:
	//----------description
	inline int	GetDocbufferSpan() const {return m_span;}
	inline void SetDocbufferSpan(int span) {m_span=span;}

    //----------Envelope array
    inline short* GetElmtAdr(int i) {return (short*) &(m_Envelope[i]);}
    inline int	GetSourceMode()const {return m_sourceMode;}
	inline int	GetSourceChan() const {return m_sourceChan;}
    inline int	GetEnvelopeSize() const {return m_Envelope.GetSize();}
	inline int	GetnElements() const {return m_npixels;}
	inline int	GetAt(int j) const {return m_Envelope[j];}
    inline void SetSourceMode(int n, int span) {m_sourceMode=n; m_span = span;}
	inline void SetSourceChan(int n) {m_sourceChan=n;}
    inline void SetPointAt(int j, int val) {m_Envelope[j]=val;}
	inline void SetSourceData(int chan, int transform) {m_sourceChan=chan; m_sourceMode=transform;}
	inline void ExportToPolyPts(long* lpDest) {for (int i= 0; i< m_Envelope.GetSize(); i++, lpDest+= 2) *lpDest = m_Envelope[i]; }
	inline short GetPointAt(int i) {return (short) m_Envelope[i];}

	void	SetEnvelopeSize(int npixels, int ndataperpixel);
	void	FillEnvelopeWithMxMi(int ifirst, short* lpData, int nchans,  int nelmts, BOOL bNew);
	void	FillEnvelopeWithSmoothMxMi(int ifirst, short* lpData, int nchans, int nelmts, BOOL bNew, int ioption);
	void	GetEnvelopeMaxMin(int* max, int* min);
};
