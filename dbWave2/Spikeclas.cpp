//////////////////////////////////////////////////////////////////////
// CSpikeClass : implementation file 

#include "stdafx.h" 
#include "spikeclas.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/**************************************************************************
 function:  CSpikeClass()
 purpose:	constructor
 parameters:
 returns:	
 comments:
 **************************************************************************/

CSpikeClass::CSpikeClass()
{
	m_NClass=-1;
	m_SpikeSize=0;
	m_BufferSize=0; 
	m_pRWBuffer = nullptr;
	m_EArraySize=0;
	m_pEArray = nullptr;
}

/**************************************************************************
 function:  CSpikeClass()
 purpose:	constructor
 parameters:int SpikeSize Size of the Spike
 returns:	
 comments:
 **************************************************************************/
CSpikeClass::CSpikeClass(int SpikeSize)
{
	m_NClass=-1;
	m_SpikeSize=SpikeSize;
	m_BufferSize=0;
	m_pRWBuffer = nullptr;
	m_EArraySize=0;
	m_pEArray = nullptr;
}

/**************************************************************************
 function:  ~CSpikeClass()
 purpose:	destructor
 parameters:
 returns:	
 comments:
 **************************************************************************/
CSpikeClass::~CSpikeClass()
{
	EraseData();
}

void CSpikeClass::EraseData()
{
	// delete buffer and array
	if (m_pRWBuffer != nullptr)
		free(m_pRWBuffer);
	m_pRWBuffer = nullptr;
	m_BufferSize=0;

	if (m_pEArray != nullptr)
		free(m_pEArray);
	m_EArraySize=NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CSpikeClass serialization

IMPLEMENT_SERIAL(CSpikeClass, CObject, 0 /* schema number*/ )

void CSpikeClass::Serialize(CArchive& ar)
{
	WORD w1,w2;
	if (ar.IsStoring())
	{   
		// store attributes
		ar << (WORD)m_NClass << (WORD)m_SpikeSize << m_BufferSize; 
		// store array
		for (int i=0;i<m_NClass;i++)
			ar << (WORD)*(m_pEArray+i);
		// store buffer
		for (int i=0;i<(m_NClass*m_SpikeSize);i++)
			ar << (WORD)*(m_pRWBuffer+i);
	}
	else
	{   
		// load attributes
		ar >> w1 >> w2 >> m_BufferSize;
		SizeNclasses(w1, w2);
		// load array
		for (int i=0;i<m_NClass;i++)
		{
			ar >> w1;
			*(m_pEArray+i) = w1;
		}

		for (int i=0;i<(m_NClass*m_SpikeSize);i++)
		{
			ar >> w1;
			*(m_pRWBuffer+i) = w1;
		}
	}
}

BOOL CSpikeClass::SizeNclasses(int nclasses, int spikesize)
{
	if (nclasses * spikesize == 0)
		return FALSE;

	BOOL bRet = FALSE;
	size_t wsize = nclasses*spikesize * sizeof(short);
	size_t isize = nclasses * sizeof(int);
	short*	pRWBuffer = nullptr;
	int*	pEArray = nullptr;

	if (m_pRWBuffer == nullptr)
	{
		pRWBuffer = (short*)	malloc (wsize);
		pEArray	= (int*)	malloc (isize);
	}
	else
	{
		pRWBuffer = (short*)	realloc (m_pRWBuffer, wsize);
		pEArray	= (int*)	realloc (m_pEArray, isize);
	}

	if (pRWBuffer != nullptr && pEArray != nullptr)
	{
		m_pRWBuffer = pRWBuffer;
		m_pEArray = pEArray;
		m_NClass=nclasses;
		m_SpikeSize=spikesize;	
		bRet = TRUE;
	}
	
	return bRet;
}
