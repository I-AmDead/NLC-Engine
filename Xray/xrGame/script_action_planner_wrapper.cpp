////////////////////////////////////////////////////////////////////////////
//	Module 		: script_action_planner_wrapper.cpp
//	Created 	: 19.03.2004
//  Modified 	: 26.03.2004
//	Author		: Dmitriy Iassenev
//	Description : Script action planner wrapper
////////////////////////////////////////////////////////////////////////////

#include "pch_script.h"
#include "script_action_planner_wrapper.h"
#include "script_game_object.h"
#include "ai_debug.h"

void CScriptActionPlannerWrapper::setup			(CScriptGameObject *object)
{
#ifdef LOG_ACTION
	set_use_log								(!!psAI_Flags.test(aiGOAPScript));
#endif
	luabind::call_member<void>				(this,"setup",object);
}

void CScriptActionPlannerWrapper::setup_static	(CScriptActionPlanner *planner, CScriptGameObject *object)
{
	planner->CScriptActionPlanner::setup	(object);
}

void CScriptActionPlannerWrapper::update		()
{
#ifdef LOG_ACTION
	if ((psAI_Flags.test(aiGOAPScript) && !m_use_log) || (!psAI_Flags.test(aiGOAPScript) && m_use_log))
		set_use_log							(!!psAI_Flags.test(aiGOAPScript));
#endif
	lua_State *L = this->m_object->lua_state();	
	if (L)
	{
		int top = lua_gettop(L);
		lua_pushinteger(L, top);
		lua_setglobal(L, "g_lua_stack_top");
		luabind::call_member<void>(this, "update");
		lua_settop(L, top);
	}
}

void CScriptActionPlannerWrapper::update_static	(CScriptActionPlanner *planner)
{
	planner->CScriptActionPlanner::update	();
}