#include "StdAfx.h"
#include "SpikeClass.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


SpikeClass::SpikeClass()
= default;

SpikeClass::SpikeClass(int SpikeSize)
{
	m_spike_size = SpikeSize;
}

SpikeClass::~SpikeClass()
{
	EraseData();
}

void SpikeClass::EraseData()
{
	// delete buffer and array
	if (m_class_buffer != nullptr)
		free(m_class_buffer);
	m_class_buffer = nullptr;
	m_buffer_size = 0;

	if (m_elements_array != nullptr)
		free(m_elements_array);
	m_EArraySize = NULL;
}

IMPLEMENT_SERIAL(SpikeClass, CObject, 0 /* schema number*/)

void SpikeClass::Serialize(CArchive& ar)
{
	WORD w1, w2;
	
	if (ar.IsStoring())
	{
		// store attributes
		ar << static_cast<WORD>(m_n_classes);
		ar << static_cast<WORD>(m_spike_size);
		ar << m_buffer_size;
		// store array
		for (auto i = 0; i < m_n_classes; i++)
			ar << static_cast<WORD>(*(m_elements_array + i));

		// store buffer
		const int buffer_total_size = m_n_classes * m_spike_size;
		for (auto i = 0; i < buffer_total_size; i++)
			ar << static_cast<WORD>(*(m_class_buffer + i));
	}
	else
	{
		// load attributes
		ar >> w1; m_n_classes = w1;
		ar >> w2; m_spike_size = w2;
		ar >> m_buffer_size;
		SizeNclasses(w1, w2);

		// load array
		for (auto i = 0; i < m_n_classes; i++)
		{
			ar >> w1; *(m_elements_array + i) = w1;
		}

		const int buffer_total_size = m_n_classes * m_spike_size;
		for (auto i = 0; i < buffer_total_size; i++)
		{
			ar >> w1; *(m_class_buffer + i) = w1;
		}
	}
}

BOOL SpikeClass::SizeNclasses(int nclasses, int spikesize)
{
	if (nclasses * spikesize == 0)
		return FALSE;

	auto b_ret = FALSE;
	const auto wsize = nclasses * spikesize * sizeof(short);
	const auto isize = nclasses * sizeof(int);
	short* p_rw_buffer;
	int* p_e_array;

	if (m_class_buffer == nullptr)
	{
		p_rw_buffer = static_cast<short*>(malloc(wsize));
		p_e_array = static_cast<int*>(malloc(isize));
	}
	else
	{
		p_rw_buffer = static_cast<short*>(realloc(m_class_buffer, wsize));
		p_e_array = static_cast<int*>(realloc(m_elements_array, isize));
	}

	if (p_rw_buffer != nullptr && p_e_array != nullptr)
	{
		m_class_buffer = p_rw_buffer;
		m_elements_array = p_e_array;
		m_n_classes = nclasses;
		m_spike_size = spikesize;
		b_ret = TRUE;
	}

	return b_ret;
}
