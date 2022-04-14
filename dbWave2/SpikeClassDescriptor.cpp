#include <StdAfx.h>
#include "SpikeClassDescriptor.h"

IMPLEMENT_SERIAL(SpikeClassDescriptor, CObject, 0)

SpikeClassDescriptor::SpikeClassDescriptor()
=default;

SpikeClassDescriptor::SpikeClassDescriptor(int number, int items)
{
	class_id = number;
	n_items = items;
}

SpikeClassDescriptor::SpikeClassDescriptor(const SpikeClassDescriptor & other)
{
	class_id = other.class_id;
	n_items = other.n_items;
}

SpikeClassDescriptor::~SpikeClassDescriptor()
= default;

void SpikeClassDescriptor::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << class_id;
		ar << n_items ;
	}
	else
	{
		ar >> class_id;
		ar >> n_items;
	}
}

SpikeClassDescriptor& SpikeClassDescriptor::operator=(const SpikeClassDescriptor& arg)
{
	if (&arg != this)
	{
		class_id = arg.class_id;
		n_items = arg.n_items;
	}
	return *this;
}