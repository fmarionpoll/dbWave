#include <StdAfx.h>
#include "SpikeClassDescriptor.h"

IMPLEMENT_SERIAL(SpikeClassDescriptor, CObject, 0)

SpikeClassDescriptor::SpikeClassDescriptor()
=default;

SpikeClassDescriptor::SpikeClassDescriptor(int number, int items)
{
	class_id_ = number;
	n_items_ = items;
}

SpikeClassDescriptor::SpikeClassDescriptor(const SpikeClassDescriptor & other)
{
	class_id_ = other.class_id_;
	n_items_ = other.n_items_;
}

SpikeClassDescriptor::~SpikeClassDescriptor()
= default;

void SpikeClassDescriptor::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << class_id_;
		ar << n_items_ ;
	}
	else
	{
		ar >> class_id_;
		ar >> n_items_;
	}
}

SpikeClassDescriptor& SpikeClassDescriptor::operator=(const SpikeClassDescriptor& arg)
{
	if (&arg != this)
	{
		class_id_ = arg.class_id_;
		n_items_ = arg.n_items_;
	}
	return *this;
}