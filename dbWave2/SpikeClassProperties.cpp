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
	class_text_ = other.class_text_;
}

SpikeClassProperties::~SpikeClassProperties()
= default;

void SpikeClassProperties::Serialize(CArchive& ar)
{
	CObject::Serialize(ar);
	WORD trap = 100;
	WORD count = 3;
	if (ar.IsStoring())
	{
		ar << trap << count << class_id_ << n_items_  << class_text_;
	}
	else
	{
		ar >> trap;
		if (trap < 100)
		{
			class_id_ = trap;
			ar >> n_items_;
		}
		else
		{
			ar >> count >> class_id_ >> n_items_ >> class_text_;
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