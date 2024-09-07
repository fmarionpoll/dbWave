#include "stdafx.h"
#include "SpikeClassGridToolBar.h"

#include "ViewSpikeSort.h"

IMPLEMENT_DYNAMIC(SpikeClassGridToolBar, CMFCToolBar)

BEGIN_MESSAGE_MAP(SpikeClassGridToolBar, CMFCToolBar)

	ON_COMMAND(ID_EXPAND_ALL, on_expand_all_properties)
	ON_UPDATE_COMMAND_UI(ID_EXPAND_ALL, on_update_expand_all_properties)
	ON_COMMAND(ID_SORT_PROPERTIES, on_sort_properties)
	ON_UPDATE_COMMAND_UI(ID_SORT_PROPERTIES, on_update_sort_properties)
	ON_BN_CLICKED(IDC_EDIT_INFOS, on_bn_clicked_edit_infos)
	ON_UPDATE_COMMAND_UI(IDC_EDIT_INFOS, on_update_bn_edit_infos)
	ON_BN_CLICKED(IDC_UPDATE_INFOS, on_bn_clicked_update_infos)
	ON_UPDATE_COMMAND_UI(IDC_UPDATE_INFOS, on_update_bn_update_infos)

END_MESSAGE_MAP()

void SpikeClassGridToolBar::on_expand_all_properties()
{
	static_cast<ViewSpikeSort*>(GetParent())->property_grid.ExpandAll();
}

void SpikeClassGridToolBar::on_update_expand_all_properties(CCmdUI* /* pCmdUI */)
{
	TRACE("on_update_expand_all_properties() \n");
}

void SpikeClassGridToolBar::on_sort_properties()
{
	const boolean flag = !static_cast<ViewSpikeSort*>(GetParent())->property_grid.IsAlphabeticMode();
	static_cast<ViewSpikeSort*>(GetParent())->property_grid.SetAlphabeticMode(flag);
}

void SpikeClassGridToolBar::on_update_sort_properties(CCmdUI* p_cmd_ui)
{
	p_cmd_ui->SetCheck(static_cast<ViewSpikeSort*>(GetParent())->property_grid.IsAlphabeticMode());
}

void SpikeClassGridToolBar::on_update_bn_edit_infos(CCmdUI* p_cmd_ui)
{
	TRACE("on_bn_update_bn_edit_infos() \n");
}

void SpikeClassGridToolBar::on_update_bn_update_infos(CCmdUI* p_cmd_ui)
{
	p_cmd_ui->Enable(static_cast<ViewSpikeSort*>(GetParent())->b_changed_property);
}

void SpikeClassGridToolBar::on_bn_clicked_update_infos()
{
	auto* p_parent = static_cast<ViewSpikeSort*>(GetParent());
	p_parent->property_grid.update(p_parent->p_spk_list);
}

void SpikeClassGridToolBar::on_bn_clicked_edit_infos()
{
	TRACE("on_bn_clicked_edit_infos() \n");
}
