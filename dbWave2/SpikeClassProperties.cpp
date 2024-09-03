#include <StdAfx.h>
#include "SpikeClassProperties.h"

CString SpikeClassProperties::class_descriptor[] =
{
	_T("sugar cell"),
	_T("bitter cell"),
	_T("salt cell"),
	_T("water cell"),
	_T("mechano-receptor"),
	_T("background activity"),
	_T("funny spikes")
};


IMPLEMENT_SERIAL(SpikeClassProperties, CObject, VERSIONABLE_SCHEMA | 2)

SpikeClassProperties::SpikeClassProperties()
=default;

SpikeClassProperties::SpikeClassProperties(const int number, const int items, const CString& descriptor)
{
	class_id_ = number;
	n_items_ = items;
	class_text_ = descriptor;
}

SpikeClassProperties::SpikeClassProperties(const SpikeClassProperties & other)
{
	class_id_ = other.class_id_;
	n_items_ = other.n_items_;
}

SpikeClassProperties::~SpikeClassProperties()
= default;

void SpikeClassProperties::Serialize(CArchive& ar)
{
	CObject::Serialize(ar);

	if (ar.IsStoring())
	{
		ar << class_id_ << n_items_  << class_text_;
	}
	else
	{
		const auto i = ar.GetObjectSchema();
		switch (i)
		{
		case 1:	
			ar >> class_id_ >> n_items_;
			class_text_.Format(_T("class %i"), class_id_);
			break;
		case 2:
		default:
			ar >> class_id_ >> n_items_ >> class_text_;
			if (class_text_.IsEmpty())
				class_text_ = " ";
			break;
		}
	}
}

SpikeClassProperties& SpikeClassProperties::operator=(const SpikeClassProperties& arg)
{
	if (&arg != this)
	{
		class_id_ = arg.class_id_;
		n_items_ = arg.n_items_;
		class_text_= arg.class_text_;
	}
	return *this;
}