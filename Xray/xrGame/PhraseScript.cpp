#include "pch_script.h"
#include "PhraseScript.h"
#include "script_engine.h"
#include "ai_space.h"
#include "gameobject.h"
#include "script_game_object.h"
#include "infoportion.h"
#include "inventoryowner.h"
#include "ai_debug.h"
#include "ui/xrUIXmlParser.h"
#include "actor.h"
#include "../xr_3da/lua_tools.h"

#pragma optimize("gyts", off)

CPhraseScript::CPhraseScript	()
{
}
CPhraseScript::~CPhraseScript	()
{
}

//загрузка из XML файла
void CPhraseScript::Load		(CUIXml* uiXml, XML_NODE* phrase_node)
{
//	m_sScriptTextFunc = uiXml.Read(phrase_node, "script_text", 0, NULL);

	LoadSequence(uiXml,phrase_node, "precondition",		m_Preconditions);
	LoadSequence(uiXml,phrase_node, "action",			m_ScriptActions);
	
	LoadSequence(uiXml,phrase_node, "has_info",			m_HasInfo);
	LoadSequence(uiXml,phrase_node, "dont_has_info",	m_DontHasInfo);

	LoadSequence(uiXml,phrase_node, "give_info",		m_GiveInfo);
	LoadSequence(uiXml,phrase_node, "disable_info",		m_DisableInfo);
}

template<class T> 
void  CPhraseScript::LoadSequence (CUIXml* uiXml, XML_NODE* phrase_node, 
								  LPCSTR tag, T&  str_vector)
{
	int tag_num = uiXml->GetNodesNum(phrase_node, tag);
	str_vector.clear();
	for(int i=0; i<tag_num; i++)
	{
		LPCSTR tag_text = uiXml->Read(phrase_node, tag, i, NULL);
		str_vector.push_back(tag_text);
	}
}

bool  CPhraseScript::CheckInfo		(const CInventoryOwner* pOwner) const
{
	THROW(pOwner);

	for(u32 i=0; i<m_HasInfo.size(); i++) {
#pragma todo("Andy->Andy how to check infoportion existence in XML ?")
/*		INFO_INDEX	result = CInfoPortion::IdToIndex(m_HasInfo[i],NO_INFO_INDEX,true);
		if (result == NO_INFO_INDEX) {
			ai().script_engine().script_log(eLuaMessageTypeError,"XML item not found : \"%s\"",*m_HasInfo[i]);
			break;
		}
*/
//.		if (!pOwner->HasInfo(m_HasInfo[i])) {
		if (!Actor()->HasInfo(m_HasInfo[i])) {
#ifdef DEBUG
			if(psAI_Flags.test(aiDialogs) )
				Msg("----rejected: [%s] has info %s", pOwner->Name(), *m_HasInfo[i]);
#endif
			return false;
		}
	}

	for(i=0; i<m_DontHasInfo.size(); i++) {
/*		INFO_INDEX	result = CInfoPortion::IdToIndex(m_DontHasInfo[i],NO_INFO_INDEX,true);
		if (result == NO_INFO_INDEX) {
			ai().script_engine().script_log(eLuaMessageTypeError,"XML item not found : \"%s\"",*m_DontHasInfo[i]);
			break;
		}
*/
//.		if (pOwner->HasInfo(m_DontHasInfo[i])) {
		if (Actor()->HasInfo(m_DontHasInfo[i])) {
#ifdef DEBUG
			if(psAI_Flags.test(aiDialogs) )
				Msg("----rejected: [%s] dont has info %s", pOwner->Name(), *m_DontHasInfo[i]);
#endif
			return false;
		}
	}
	return true;
}


void  CPhraseScript::TransferInfo	(const CInventoryOwner* pOwner) const
{
	THROW(pOwner);

	for(u32 i=0; i<m_GiveInfo.size(); i++)
//.		pOwner->TransferInfo(m_GiveInfo[i], true);
		Actor()->TransferInfo(m_GiveInfo[i], true);

	for(i=0; i<m_DisableInfo.size(); i++)
//.		pOwner->TransferInfo(m_DisableInfo[i],false);
		Actor()->TransferInfo(m_DisableInfo[i], false);
}



bool call_script_func(LPCSTR func_name, 
						const CGameObject* pSpeakerGO1, 
						const CGameObject* pSpeakerGO2, 
						LPCSTR dialog_id, 
						LPCSTR phrase_id,
						LPCSTR next_phrase_id,
						bool need_result = false)
{
	bool result = false;


	if (strstr(func_name, "(") && strstr(func_name, ")"))
		{
			string128  code = { 0 };
			if (need_result)
				strcpy_s(code, 128, "return ");
			strcat_s(code, 128, func_name);

			lua_State *L = GameLua();
			int save_top = lua_gettop(L);
			// using namespace luabind::detail;
			lua_pushcfunction(L, CScriptEngine::lua_panic);
			int err_func = lua_gettop(L);

			int err = luaL_loadstring(L, code);
			if (0 == err)
			{

				lua_newtable(L);
				// convert_to_lua<CScriptGameObject*>(L, pSpeakerGO->lua_game_object());
				lua_pushinteger(L, pSpeakerGO1->ID());
				lua_setfield(L, -2, "speaker_id");
				if (pSpeakerGO2)
				{
					lua_pushinteger(L, pSpeakerGO2->ID());
					lua_setfield(L, -2, "speaker2_id");
				}

				lua_pushstring(L, dialog_id);
				lua_setfield(L, -2, "dialog_id");
				lua_pushstring(L, phrase_id);
				lua_setfield(L, -2, "phrase_id");
				if (next_phrase_id)
				{
					lua_pushstring(L, next_phrase_id);
					lua_setfield(L, -2, "next_phrase_id");
				}

				lua_setglobal(L, "dialog_params");
				if (0 == lua_pcall(L, 0, LUA_MULTRET, err_func))
				{
					result = need_result ? lua_isboolean(L, -1) && lua_toboolean(L, -1) : true;
				}

				lua_settop(L, save_top);
				return result;
			}
			else
			{
				log_script_error("invalid function call code [[ %s ]], error: %s", func_name, lua_tostring(L, -1));
				lua_settop(L, save_top);
				return false;
			}
		}
	else
	{
		if (!phrase_id)		 phrase_id	    = "";		
		if (!next_phrase_id) next_phrase_id = "";

		THROW(func_name);

		try {
			if (need_result)
			{
				luabind::functor<bool>	lua_function;
				bool functor_exists = ai().script_engine().functor(func_name, lua_function);
				THROW3(functor_exists, "Cannot find phrase precondition function ", func_name);
				if (!functor_exists) return false;
				if (pSpeakerGO2)					
					result = lua_function(pSpeakerGO1->lua_game_object(), pSpeakerGO2->lua_game_object(), dialog_id, phrase_id, next_phrase_id);
				else								
					result = lua_function(pSpeakerGO1->lua_game_object(), dialog_id, phrase_id, next_phrase_id);
			}
			else
			{
				luabind::functor<void>	lua_function;
				bool functor_exists = ai().script_engine().functor(func_name, lua_function);
				THROW3(functor_exists, "Cannot find phrase function ", func_name);
				if (!functor_exists) return false;
				result = true;

				if (pSpeakerGO2)					
					lua_function(pSpeakerGO1->lua_game_object(), pSpeakerGO2->lua_game_object(), dialog_id, phrase_id, next_phrase_id);
				else									
					lua_function(pSpeakerGO1->lua_game_object(), dialog_id, phrase_id, next_phrase_id);
				
			}
		} catch (...) {
		}

		return result;
	}
}


void CPhraseScript::Action(const CGameObject* pSpeakerGO1, const CGameObject* pSpeakerGO2, LPCSTR dialog_id, LPCSTR phrase_id) const 
{
	TransferInfo(smart_cast<const CInventoryOwner*>(pSpeakerGO1));

	for(u32 i = 0; i<Actions().size(); ++i)
	{
		LPCSTR func_name = *Actions()[i];
		call_script_func(func_name, pSpeakerGO1, pSpeakerGO2, dialog_id, phrase_id, NULL);
	}
}

void CPhraseScript::Action(const CGameObject* pSpeakerGO, LPCSTR dialog_id, LPCSTR phrase_id) const 
{

	for(u32 i = 0; i<Actions().size(); ++i)
	{
		LPCSTR func_name = *Actions()[i];
		call_script_func(func_name, pSpeakerGO, NULL, dialog_id, phrase_id, NULL);
	}
	TransferInfo(smart_cast<const CInventoryOwner*>(pSpeakerGO));
}

bool CPhraseScript::Precondition	(	const CGameObject* pSpeakerGO1, 
										const CGameObject* pSpeakerGO2, 
										LPCSTR dialog_id, 
										LPCSTR phrase_id,
										LPCSTR next_phrase_id) const 
{
	bool predicate_result = true;

	if(!CheckInfo(smart_cast<const CInventoryOwner*>(pSpeakerGO1))) {
		#ifdef DEBUG
		if (psAI_Flags.test(aiDialogs))
			Msg("dialog [%s] phrase[%s] rejected by CheckInfo",dialog_id,phrase_id);
		#endif
		return false;
	}
	for(u32 i = 0; i<Preconditions().size(); ++i)
	{
		LPCSTR func_name = *Preconditions()[i];	
		MsgCB("$#CONTEXT: precondition dialog_id = %s, phrase_id = %s, next_phrase_id = %s ", dialog_id, phrase_id, next_phrase_id);
		predicate_result = call_script_func(func_name, pSpeakerGO1, pSpeakerGO2, dialog_id, phrase_id, next_phrase_id, true);
		if(!predicate_result)
		{
		#ifdef DEBUG
			if (psAI_Flags.test(aiDialogs))
				Msg("dialog [%s] phrase[%s] rejected by script predicate",dialog_id,phrase_id);
		#endif
			break;
		}
	}
	return predicate_result;
}

bool CPhraseScript::Precondition(const CGameObject* pSpeakerGO, LPCSTR dialog_id, LPCSTR phrase_id) const 
{
	bool predicate_result = true;

	if(!CheckInfo(smart_cast<const CInventoryOwner*>(pSpeakerGO)))
	{
		#ifdef DEBUG
			if (psAI_Flags.test(aiDialogs))
				Msg("dialog [%s] phrase[%s] rejected by CheckInfo",dialog_id,phrase_id);
		#endif
		return false;
	}

	for(u32 i = 0; i<Preconditions().size(); ++i)
	{
		LPCSTR func_name = *Preconditions()[i];	
		MsgCB("$#CONTEXT: precondition dialog_id = %s, phrase_id = %s ", dialog_id, phrase_id);
		predicate_result = call_script_func(func_name, pSpeakerGO, NULL, dialog_id, phrase_id, "", true);		
		if(!predicate_result){
		#ifdef DEBUG
			if (psAI_Flags.test(aiDialogs))
				Msg("dialog [%s] phrase[%s] rejected by script predicate", dialog_id, phrase_id);
		#endif
			break;
		} 
	}
	return predicate_result;
}
