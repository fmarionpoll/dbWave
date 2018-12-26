#pragma once

//////////////////////////////////////////////////////////////////////
//
// CSpikeClass : to  class spikes
// spikeclas.h header file

///////////////////////////////////////////////////////////////////////////////////////////

class CSpikeClass :  public CObject
{
	DECLARE_SERIAL(CSpikeClass);

// attributes
protected :
	int 	m_NClass;			// Number of different classes of spikes
	int 	m_SpikeSize;		// size of spike
	long    m_BufferSize;		// GetRectSize of the buffer 	
	short* 	m_pRWBuffer;		// pointer to the buffer
	int		m_EArraySize;
	int*	m_pEArray;			// array of class elemt
	BOOL	SizeNclasses(int nclasses, int spikesize);

public :
	void	EraseData();
	
// constructor /destructor
public :     
	CSpikeClass();
	CSpikeClass(int SpikeSize);
	virtual ~CSpikeClass();
	virtual void Serialize(CArchive& ar);	// overridden for document i/o
};
