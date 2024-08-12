#include "StdAfx.h"
#include "SpikeClass.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


SpikeClass::SpikeClass()
= default;

SpikeClass::SpikeClass(const int spike_size)
{
	m_spike_size_ = spike_size;
}

SpikeClass::~SpikeClass()
{
	erase_data();
}

void SpikeClass::erase_data()
{
	// delete buffer and array
	if (m_class_buffer_ != nullptr)
		free(m_class_buffer_);
	m_class_buffer_ = nullptr;
	m_buffer_size_ = 0;

	if (m_elements_array_ != nullptr)
		free(m_elements_array_);
	m_e_array_size_ = NULL;
}

IMPLEMENT_SERIAL(SpikeClass, CObject, 0 /* schema number*/)

void SpikeClass::Serialize(CArchive& ar)
{
	WORD w1, w2;
	
	if (ar.IsStoring())
	{
		// store attributes
		w1 = static_cast<WORD>(m_n_classes_);  ar << w1;
		w2 = static_cast<WORD>(m_spike_size_); ar << w2;
		ar << m_buffer_size_;

		// store array
		if (m_elements_array_ != nullptr) {
			for (auto i = 0; i < m_n_classes_; i++)
				ar << static_cast<WORD>(*(m_elements_array_ + i));
		}

		// store buffer
		const int buffer_total_size = m_n_classes_ * m_spike_size_;
		if (m_class_buffer_ != nullptr) {
			for (auto i = 0; i < buffer_total_size; i++)
			{
				ar << static_cast<WORD>(*(m_class_buffer_ + i));
			}
		}
	}
	else
	{
		// load attributes
		ar >> w1; m_n_classes_ = w1;
		ar >> w2; m_spike_size_ = w2;
		ar >> m_buffer_size_;
		size_n_classes(w1, w2);

		// load array
		if (m_elements_array_ != nullptr) {
			for (auto i = 0; i < m_n_classes_; i++)
			{
				ar >> w1;
				*(m_elements_array_ + i) = w1;
			}
		}

		const int buffer_total_size = m_n_classes_ * m_spike_size_;
		if (m_class_buffer_ != nullptr) {
			for (auto i = 0; i < buffer_total_size; i++)
			{
				ar >> w1;
				*(m_class_buffer_ + i) = static_cast<short>(w1);
			}
		}
	}
}

BOOL SpikeClass::size_n_classes(const int n_classes, const int spike_size)
{
	if (n_classes * spike_size == 0)
		return FALSE;

	auto b_ret = FALSE;
	const auto w_size = n_classes * spike_size * sizeof(short);
	const auto i_size = n_classes * sizeof(int);
	short* p_rw_buffer;
	int* p_e_array;

	if (m_class_buffer_ == nullptr)
	{
		p_rw_buffer = static_cast<short*>(malloc(w_size));
		p_e_array = static_cast<int*>(malloc(i_size));
	}
	else
	{
		p_rw_buffer = static_cast<short*>(realloc(m_class_buffer_, w_size));
		p_e_array = static_cast<int*>(realloc(m_elements_array_, i_size));
	}

	if (p_rw_buffer != nullptr && p_e_array != nullptr)
	{
		m_class_buffer_ = p_rw_buffer;
		m_elements_array_ = p_e_array;
		m_n_classes_ = n_classes;
		m_spike_size_ = spike_size;
		b_ret = TRUE;
	}

	return b_ret;
}
