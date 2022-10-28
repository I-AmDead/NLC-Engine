// File:        UITabButton.cpp
// Description: 
// Created:     19.11.2004
// Last Change: 27.11.2004
// Author:      Serhiy Vynnychenko
// Mail:        narrator@gsc-game.kiev.ua
//
// Copyright: 2004 GSC Game World

#include "StdAfx.h"
#include "UITabButton.h"
#include "../HUDManager.h"

CUITabButton::CUITabButton(){
//.	this->m_pAssociatedWindow = NULL;
}

CUITabButton::~CUITabButton(){

}

bool CUITabButton::OnMouse(float x, float y, EUIMessages mouse_action){
	return CUIWindow::OnMouse(x, y, mouse_action);
}

bool CUITabButton::OnMouseDown(int mouse_btn){
	if (mouse_btn==MOUSE_1)
	{
		GetMessageTarget()->SendMessage(this, TAB_CHANGED);		
		return true;
	}else
		return false;
}

void CUITabButton::Update(){
	CUI3tButton::Update();
}

void CUITabButton::SendMessage(CUIWindow* pWnd, s16 msg, void* pData){
	if (!IsEnabled())
		return;

	switch (msg)
	{
	case TAB_CHANGED:
		if (this == pWnd)
		{
            m_eButtonState = BUTTON_PUSHED;			
//.			ShowAssociatedWindow(true);
			OnClick();
		}
		else		
		{
			m_eButtonState = BUTTON_NORMAL;
//.			ShowAssociatedWindow(false);
		}
		break;
	default:
		;
	}
}