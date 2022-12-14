////////////////////////////////////////////////////////////////////////////
//	Module 		: script_engine_script.cpp
//	Created 	: 25.12.2002
//  Modified 	: 17.02.2016
//	Author		: Dmitriy Iassenev
//	Description : ALife Simulator script engine export
////////////////////////////////////////////////////////////////////////////

#include "pch_script.h"
#include "script_engine.h"
#include "ai_space.h"
#include "script_debugger.h"
//#include <ostream>
#include "script_additional_libs.h"
#include "xr_level_controller.h"
#include "../x_ray.h"
#include "../lua_tools.h"


ENGINE_API float g_fTimeInteractive;

using namespace luabind;

void LuaLog(LPCSTR caMessage)
{
	ai().script_engine().script_log	(ScriptStorage::eLuaMessageTypeMessage,"%s",caMessage);
#ifdef USE_DEBUGGER
	if( ai().script_engine().debugger() ){
		ai().script_engine().debugger()->Write(caMessage);
	}
#endif
}

void ErrorLog(LPCSTR caMessage)
{
	ai().script_engine().script_log	(ScriptStorage::eLuaMessageTypeError,"%s",caMessage);
#ifdef USE_DEBUGGER
	if( ai().script_engine().debugger() ){
		ai().script_engine().debugger()->Write(caMessage);
	}
#endif
}

void FlushLogs()
{
#ifdef DEBUG
	FlushLog();
	ai().script_engine().flush_log();
#endif // DEBUG
}

void verify_if_thread_is_running()
{
	THROW2	(ai().script_engine().current_thread(),"coroutine.yield() is called outside the LUA thread!");
}

bool editor()
{
#ifdef XRGAME_EXPORTS
	return		(false);
#else
	return		(true);
#endif
}

#ifdef XRGAME_EXPORTS
CRenderDevice *get_device()
{
	return		(&Device);
}
#endif

int bit_and(int i, int j)
{
	return			(i & j);
}

int bit_or(int i, int j)
{
	return			(i | j);
}

int bit_xor(int i, int j)
{
	return			(i ^ j);
}

int bit_not(int i)
{
	return			(~i);
}

LPCSTR user_name()
{
	return			(Core.UserName);
}

void prefetch_module(lua_State *L,  LPCSTR file_name)
{
	if (!strstr(file_name, "\\"))
		ai().script_engine().process_file(L, file_name);
	else
	{
		LPCSTR traceback = GetLuaTraceback(L, 0);
		Msg("!#ERROR: invalid module name %s, called from:\n %s", file_name, (traceback ? traceback : "(null)"));		
	}
}

struct profile_timer_script {
	u64							m_start_cpu_tick_count;
	u64							m_accumulator;
	u64							m_count;
	int							m_recurse_mark;
	
	IC								profile_timer_script	()
	{
		m_start_cpu_tick_count	= 0;
		m_accumulator			= 0;
		m_count					= 0;
		m_recurse_mark			= 0;
	}

	IC								profile_timer_script	(const profile_timer_script &profile_timer)
	{
		*this					= profile_timer;
	}

	IC		profile_timer_script&	operator=				(const profile_timer_script &profile_timer)
	{
		m_start_cpu_tick_count	= profile_timer.m_start_cpu_tick_count;
		m_accumulator			= profile_timer.m_accumulator;
		m_count					= profile_timer.m_count;
		m_recurse_mark			= profile_timer.m_recurse_mark;
		return					(*this);
	}

	IC		bool					operator<				(const profile_timer_script &profile_timer) const
	{
		return					(m_accumulator < profile_timer.m_accumulator);
	}

	IC		void					start					()
	{
		if (m_recurse_mark) {
			++m_recurse_mark;
			return;
		}

		++m_recurse_mark;
		++m_count;
		m_start_cpu_tick_count	= CPU::GetCLK();
	}

	IC		void					stop					()
	{
		THROW					(m_recurse_mark);
		--m_recurse_mark;
		
		if (m_recurse_mark)
			return;
		
		u64						finish = CPU::GetCLK();
		if (finish > m_start_cpu_tick_count)
			m_accumulator		+= finish - m_start_cpu_tick_count;
	}

	IC		float					time					() const
	{
		FPU::m64r				();
		float					result = (float(double(m_accumulator)/double(CPU::clk_per_second))*1000000.f);
		FPU::m24r				();
		return					(result);
	}
};

IC	profile_timer_script	operator+	(const profile_timer_script &portion0, const profile_timer_script &portion1)
{
	profile_timer_script	result;
	result.m_accumulator	= portion0.m_accumulator + portion1.m_accumulator;
	result.m_count			= portion0.m_count + portion1.m_count;
	return					(result);
}

void reset_interacive(float t)
{
	g_fTimeInteractive = t;
}


//IC	std::ostream& operator<<(std::ostream &stream, profile_timer_script &timer)
//{
//	stream					<< timer.time();
//	return					(stream);
//}

void FatalError(LPCSTR msg) 
{
	Device.Pause(TRUE, FALSE, FALSE, "FatalError");
	Device.Destroy();
	Debug.fatal(DEBUG_INFO, "SCRIPT ERROR: %s", msg);		
	TerminateProcess (GetCurrentProcess(), UINT(-5));
}

#ifdef XRGAME_EXPORTS
ICF	u32	script_time_global	()	{ return Device.dwTimeGlobal; }
#else
ICF	u32	script_time_global	()	{ return 0; }
#endif
extern int get_action_dik(EGameActions _action_id);
CApplication *get_application() { return pApp; }

#pragma optimize("s",on)
void CScriptEngine::script_register(lua_State *L)
{
	open_additional_libs(L); //RvP
	module(L)[
		def("log1",	(void(*)(LPCSTR)) &Log),	//RvP		

		class_<profile_timer_script>("profile_timer")
			.def(constructor<>())
			.def(constructor<profile_timer_script&>())
			.def(const_self + profile_timer_script())
			.def(const_self < profile_timer_script())
//			.def(tostring(self))
			.def("start",&profile_timer_script::start)
			.def("stop",&profile_timer_script::stop)
			.def("time",&profile_timer_script::time)
		,
		class_<CApplication>("CApplication")
		.def("set_load_texture",				&CApplication::SetLoadTexture),
		def("get_application",					&get_application)

	];

	function	(L,	"log",							LuaLog);
	function	(L,	"fatal_error",					FatalError);
	function	(L,	"error_log",					ErrorLog);
	function	(L,	"flush",						FlushLogs);
	function	(L,	"prefetch",						prefetch_module, raw(_1));
	function	(L,	"verify_if_thread_is_running",	verify_if_thread_is_running);
	function	(L,	"editor",						editor);
	function	(L,	"bit_and",						bit_and);
	function	(L,	"bit_or",						bit_or);
	function	(L,	"bit_xor",						bit_xor);
	function	(L,	"bit_not",						bit_not);
	function	(L, "user_name",					user_name);
	function	(L, "time_global",					script_time_global);
	function	(L, "bind_to_dik",					get_action_dik);
#ifdef XRGAME_EXPORTS
	function	(L,	"device",						get_device);
	function	(L, "reset_interactive",			reset_interacive);
#endif
}
