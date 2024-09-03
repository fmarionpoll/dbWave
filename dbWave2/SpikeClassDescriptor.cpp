#include <StdAfx.h>
#include "SpikeClassDescriptor.h"

CString SpikeClassDescriptor::class_descriptor[] =
{
	_T("sugar cell"),
	_T("bitter cell"),
	_T("salt cell"),
	_T("water cell"),
	_T("mechano-receptor"),
	_T("background activity"),
	_T("funny spikes")
};


IMPLEMENT_SERIAL(SpikeClassDescriptor, CObject, VERSIONABLE_SCHEMA | 2)

SpikeClassDescriptor::SpikeClassDescriptor()
=default;

SpikeClassDescriptor::SpikeClassDescriptor(const int number, const int items, const CString& descriptor)
{
	class_id_ = number;
	n_items_ = items;
	descriptor_ = descriptor;
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
	CObject::Serialize(ar);

	if (ar.IsStoring())
	{
		ar << class_id_ << n_items_  << descriptor_;
	}
	else
	{
		const auto i = ar.GetObjectSchema();
		switch (i)
		{
		case 1:	
			ar >> class_id_ >> n_items_;
			descriptor_.Format(_T("class %i"), class_id_);
			break;
		case 2:
		default:
			ar >> class_id_ >> n_items_ >> descriptor_;
			if (descriptor_.IsEmpty())
				descriptor_ = " ";
			break;
		}
	}
}

SpikeClassDescriptor& SpikeClassDescriptor::operator=(const SpikeClassDescriptor& arg)
{
	if (&arg != this)
	{
		class_id_ = arg.class_id_;
		n_items_ = arg.n_items_;
		descriptor_= arg.descriptor_;
	}
	return *this;
}