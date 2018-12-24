//////////////////////////////////////////////////////////////////////
// CSpikeClass : implementation file 

#include "StdAfx.h"
#include "Spikeclas.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CSpikeClass::CSpikeClass()
{
	m_NClass=-1;
	m_SpikeSize=0;
	m_BufferSize=0; 
	m_pRWBuffer = nullptr;
	m_EArraySize=0;
	m_pEArray = nullptr;
}

CSpikeClass::CSpikeClass(int SpikeSize)
{
	m_NClass=-1;
	m_SpikeSize=SpikeSize;
	m_BufferSize=0;
	m_pRWBuffer = nullptr;
	m_EArraySize=0;
	m_pEArray = nullptr;
}

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

IMPLEMENT_SERIAL(CSpikeClass, CObject, 0 /* schema number*/ )

void CSpikeClass::Serialize(CArchive& ar)
{
	WORD w1,w2;
	if (ar.IsStoring())
	{   
		// store attributes
		ar << static_cast<WORD>(m_NClass);
		ar << static_cast<WORD>(m_SpikeSize);
		ar << m_BufferSize;
		// store array
		for (auto i=0;i<m_NClass;i++)
			ar << static_cast<WORD>(*(m_pEArray + i));
		// store buffer
		for (auto i=0;i<(m_NClass*m_SpikeSize);i++)
			ar << static_cast<WORD>(*(m_pRWBuffer + i));
	}
	else
	{   
		// load attributes
		ar >> w1;
		ar >> w2;
		ar >> m_BufferSize;
		SizeNclasses(w1, w2);
		// load array
		for (auto i=0;i<m_NClass;i++)
		{
			ar >> w1;
			*(m_pEArray+i) = w1;
		}

		for (auto i=0;i<(m_NClass*m_SpikeSize);i++)
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

	auto b_ret = FALSE;
	const auto wsize = nclasses*spikesize * sizeof(short);
	const auto isize = nclasses * sizeof(int);
	short*	p_rw_buffer;
	int*	p_e_array;

	if (m_pRWBuffer == nullptr)
	{
		p_rw_buffer = static_cast<short*>(malloc(wsize));
		p_e_array	= static_cast<int*>(malloc(isize));
	}
	else
	{
		p_rw_buffer = static_cast<short*>(realloc(m_pRWBuffer, wsize));
		p_e_array	= static_cast<int*>(realloc(m_pEArray, isize));
	}

	if (p_rw_buffer != nullptr && p_e_array != nullptr)
	{
		m_pRWBuffer = p_rw_buffer;
		m_pEArray = p_e_array;
		m_NClass=nclasses;
		m_SpikeSize=spikesize;	
		b_ret = TRUE;
	}
	
	return b_ret;
}
