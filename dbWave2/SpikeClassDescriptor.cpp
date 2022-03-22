#include <StdAfx.h>
#include "SpikeClassDescriptor.h"

IMPLEMENT_SERIAL(SpikeClassDescriptor, CObject, 0)

SpikeClassDescriptor::SpikeClassDescriptor()
=default;

SpikeClassDescriptor::SpikeClassDescriptor(int number, int items)
{
	id_number = number;
	n_items = items;
}

SpikeClassDescriptor::SpikeClassDescriptor(const SpikeClassDescriptor & other)
{
	id_number = other.id_number;
	n_items = other.n_items;
}

SpikeClassDescriptor::~SpikeClassDescriptor()
= default;

void SpikeClassDescriptor::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << id_number;
		ar << n_items ;
	}
	else
	{
		ar >> id_number;
		ar >> n_items;
	}
}

SpikeClassDescriptor& SpikeClassDescriptor::operator=(const SpikeClassDescriptor& arg)
{
	if (&arg != this)
	{
		id_number = arg.id_number;
		n_items = arg.n_items;
	}
	return *this;
}