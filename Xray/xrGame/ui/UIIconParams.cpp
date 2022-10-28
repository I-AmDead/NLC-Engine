#include "StdAfx.h"
#include "UIIconParams.h"
#include "UIInventoryUtilities.h"


void CIconParams::Load(const shared_str item_section)
{
	section		= READ_IF_EXISTS(pSettings, r_string, item_section, "icon_section", item_section);	// ����� ��� �������� ������ ����� �������� � ��������� ����
	icon_group  = READ_IF_EXISTS(pSettings, r_u32, section, "icon_group", 0);	
	name		= READ_IF_EXISTS(pSettings, r_string, section, "icon_name", NULL);	
	grid_width	= pSettings->r_float(section, "inv_grid_width");
	grid_height	= pSettings->r_float(section, "inv_grid_height");
	grid_x		= pSettings->r_float(section, "inv_grid_x");
	grid_y		= pSettings->r_float(section, "inv_grid_y");
	loaded		= true;		
}


ref_shader& CIconParams::get_shader() const
{
	return  InventoryUtilities::GetEquipmentIconsShader(icon_group);
}

Frect& CIconParams::original_rect() const
{
	static Frect rect;
	rect.x1 = INV_GRID_WIDTH  * grid_x;
	rect.y1 = INV_GRID_HEIGHT * grid_y;
	rect.x2 = rect.x1 + INV_GRID_WIDTH  * grid_width;
	rect.y2 = rect.y1 + INV_GRID_HEIGHT * grid_height;
	return rect;
}
