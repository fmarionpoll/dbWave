#include "stdafx.h"
#include "SpikeClassGridList.h"


IMPLEMENT_DYNAMIC(SpikeClassGridList, CMFCPropertyGridCtrl)

BEGIN_MESSAGE_MAP(SpikeClassGridList, CMFCPropertyGridCtrl)
	ON_WM_SIZE()
END_MESSAGE_MAP()

void SpikeClassGridList::OnSize(UINT n_type, const int cx, int cy)
{
    EndEditItem();
    AdjustLayout();
}

void SpikeClassGridList::init_header()
{
	EnableHeaderCtrl(TRUE, _T("class"), _T("description"));
	make_fixed_header();
	m_nLeftColumnWidth = 35;
	AdjustLayout();
}

void SpikeClassGridList::delete_all()
{
	const int n_properties = GetPropertyCount();
	for (int i = n_properties - 1; i >= 0; i--)
	{
		auto p_prop = GetProperty(i);
		DeleteProperty(p_prop, FALSE, FALSE);
	}
}

SpikeClassGridProperty* SpikeClassGridList::find_item(const int class_id) const
{
	boolean found = false;
	SpikeClassGridProperty* p_prop = nullptr;
	const int n_properties = GetPropertyCount();
	for (int i = n_properties - 1; i >= 0; i--)
	{
		p_prop = static_cast<SpikeClassGridProperty*>(GetProperty(i));
		if (static_cast<int>(p_prop->GetData()) == class_id)
		{
			found = true;
			break;
		}
	}
	if (!found)
		p_prop = nullptr;
	return p_prop;
}


void SpikeClassGridList::update(SpikeList* spk_list)
{
	const int classes_count = spk_list->get_classes_count();
	for (int i = 0; i < classes_count; i++)
	{
		SpikeClassProperties* p_desc = spk_list->get_class_descriptor_from_index(i);
		const int class_id = p_desc->get_class_id();
		CString cs_desc = p_desc->get_class_text();

		SpikeClassGridProperty* p_prop = find_item(class_id);
		if (p_prop == nullptr)
		{
			p_prop = SpikeClassGridProperty::create(class_id);
			AddProperty(p_prop);
		}

		if (p_prop != nullptr)
		{
			if (cs_desc.IsEmpty())
			{
				CString cs = p_prop->GetValue();
				p_desc->set_class_text(cs);
			}
			else
			{
				p_desc->set_class_text(cs_desc);
			}
		}
	}
}

