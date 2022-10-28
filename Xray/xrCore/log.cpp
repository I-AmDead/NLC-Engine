#include "stdafx.h"
#pragma hdrstop

#include <time.h>
#include <windows.h>
#include <mmsystem.h>
#include "resource.h"
#include "log.h"
#include "../../build_config_defines.h"
#ifdef _EDITOR
	#include "malloc.h"
#endif

extern BOOL					LogExecCB		= TRUE;
static string_path			logFName		= "engine.log";
static string_path			log_file_name	= "engine.log";
static BOOL 				no_log			= TRUE;
#ifdef PROFILE_CRITICAL_SECTIONS
	static xrCriticalSection	logCS(MUTEX_PROFILE_ID(log));
#else // PROFILE_CRITICAL_SECTIONS
	static xrCriticalSection	logCS;
#endif // PROFILE_CRITICAL_SECTIONS
xr_vector<shared_str>*		LogFile			= NULL;
static LogCallback			LogCB			= 0;

void FlushLog			()
{
	if (!no_log){
		logCS.Enter			();
		IWriter *f			= FS.w_open(logFName);
        if (f) {
            for (u32 it=0; it<LogFile->size(); it++)	{
				LPCSTR		s	= *((*LogFile)[it]);
				f->w_string	(s?s:"");
			}
            FS.w_close		(f);
        }
		logCS.Leave			();
    }
}

void AddOne				(const char *split) 
{
	if(!LogFile)		
						return;

	logCS.Enter			();

#ifdef DEBUG
	OutputDebugString	(split);
	OutputDebugString	("\n");
#endif

//	DUMP_PHASE;
	{
		shared_str			temp = shared_str(split);
//		DUMP_PHASE;
		LogFile->push_back	(temp);
	}

	//exec CallBack
	if (LogExecCB&&LogCB)LogCB(split);

	logCS.Leave				();
}

void Log				(const char *s) 
{
	int		i,j;

	u32			length = xr_strlen( s );
#ifndef _EDITOR    
	PSTR split  = (PSTR)_alloca( (length + 1) * sizeof(char) );
#else
	PSTR split  = (PSTR)alloca( (length + 1) * sizeof(char) );
#endif
	for (i=0,j=0; s[i]!=0; i++) {
		if (s[i]=='\n') {
			split[j]=0;	// end of line
			if (split[0]==0) { split[0]=' '; split[1]=0; }
			AddOne(split);
			j=0;
		} else {
			split[j++]=s[i];
		}
	}
	split[j]=0;
	AddOne(split);
}

void __cdecl Msg		( const char *format, ...)
{
	va_list		mark;
	string2048	buf;
	va_start	(mark, format );
	int sz		= _vsnprintf(buf, sizeof(buf)-1, format, mark ); buf[sizeof(buf)-1]=0;
    va_end		(mark);
	if (sz)		Log(buf);
}

void Log				(const char *msg, const char *dop) {
	if (!dop) {
		Log		(msg);
		return;
	}

	u32			buffer_size = (xr_strlen(msg) + 1 + xr_strlen(dop) + 1) * sizeof(char);
	PSTR buf	= (PSTR)_alloca( buffer_size );
	strconcat	(buffer_size, buf, msg, " ", dop);
	Log			(buf);
}

void Log				(const char *msg, u32 dop) {
	u32			buffer_size = (xr_strlen(msg) + 1 + 10 + 1) * sizeof(char);
	PSTR buf	= (PSTR)_alloca( buffer_size );

	sprintf_s	(buf, buffer_size, "%s %d", msg, dop);
	Log			(buf);
}

void Log				(const char *msg, int dop) {
	u32			buffer_size = (xr_strlen(msg) + 1 + 11 + 1) * sizeof(char);
	PSTR buf	= (PSTR)_alloca( buffer_size );

	sprintf_s	(buf, buffer_size, "%s %i", msg, dop);
	Log			(buf);
}

void Log				(const char *msg, float dop) {
	// actually, float string representation should be no more, than 40 characters,
	// but we will count with slight overhead
	u32			buffer_size = (xr_strlen(msg) + 1 + 64 + 1) * sizeof(char);
	PSTR buf	= (PSTR)_alloca( buffer_size );

	sprintf_s	(buf, buffer_size, "%s %f", msg, dop);
	Log			(buf);
}

void Log				(const char *msg, const Fvector &dop) {
	u32			buffer_size = (xr_strlen(msg) + 2 + 3*(64 + 1) + 1) * sizeof(char);
	PSTR buf	= (PSTR)_alloca( buffer_size );

	sprintf_s	(buf, buffer_size,"%s (%f,%f,%f)",msg, VPUSH(dop) );
	Log			(buf);
}

void Log				(const char *msg, const Fmatrix &dop)	{
	u32			buffer_size = (xr_strlen(msg) + 2 + 4*( 4*(64 + 1) + 1 ) + 1) * sizeof(char);
	PSTR buf	= (PSTR)_alloca( buffer_size );

	sprintf_s	(buf, buffer_size,"%s:\n%f,%f,%f,%f\n%f,%f,%f,%f\n%f,%f,%f,%f\n%f,%f,%f,%f\n",
		msg,
		dop.i.x, dop.i.y, dop.i.z, dop._14_,
		dop.j.x, dop.j.y, dop.j.z, dop._24_,
		dop.k.x, dop.k.y, dop.k.z, dop._34_,
		dop.c.x, dop.c.y, dop.c.z, dop._44_
	);
	Log			(buf);
}

void LogWinErr			(const char *msg, long err_code)	{
	Msg					("%s: %s",msg,Debug.error2string(err_code)	);
}

LogCallback SetLogCB	(LogCallback cb)
{
	LogCallback	result	= LogCB;
	LogCB				= cb;
	return				(result);
}

LPCSTR log_name			()
{
	return				(log_file_name);
}

void InitLog()
{
	R_ASSERT			(LogFile==NULL);
	LogFile				= xr_new< xr_vector<shared_str> >();
	LogFile->reserve	(1000);
}

void CreateLog			(BOOL nl)
{
    no_log				= nl;
	strconcat			(sizeof(log_file_name),log_file_name,Core.ApplicationName,"_",Core.UserName,".log");
	if (FS.path_exist("$logs$"))
		FS.update_path	(logFName,"$logs$",log_file_name);
	if (!no_log){
        IWriter *f		= FS.w_open	(logFName);
        if (f==NULL){
        	MessageBox	(NULL,"Can't create log file.","Error",MB_ICONERROR);
        	abort();
        }
        FS.w_close		(f);
    }
}

void CloseLog(void)
{
	FlushLog		();
 	LogFile->clear	();
	xr_delete		(LogFile);
}


typedef void (WINAPI *OFFSET_UPDATER)(LPCSTR key, u32 ofs);

void	LogXrayOffset(LPCSTR key, LPVOID base, LPVOID pval)
{
#ifdef LUAICP_COMPAT
	u32 ofs = (u32)pval - (u32)base;
	Msg	  ("XRAY_OFFSET: %30s = 0x%x base = 0x%p, pval = 0x%p ", key, ofs, base, pval);
	static OFFSET_UPDATER cbUpdater = NULL;
	HMODULE hDLL = GetModuleHandle("luaicp.dll");
	if (!cbUpdater && hDLL)
		cbUpdater = (OFFSET_UPDATER) GetProcAddress(hDLL, "UpdateXrayOffset");

	if (cbUpdater)
		cbUpdater(key, ofs);
#endif
}

CTimer						dump_timer;
LPCSTR						CONTEXT_TAG			= "$#CONTEXT";
LPCSTR						DUMP_CONTEXT_TAG	= "$#DUMP_CONTEXT";
extern	bool g_bFPUInitialized;

 typedef void	( * TIME_GETTER)	(SYSTEMTIME *dst);

 static TIME_GETTER TimeGet = NULL;

 XRCORE_API TIME_GETTER SetTimeGetter(TIME_GETTER tg)
 {
	 TIME_GETTER old = TimeGet;
	 TimeGet = tg;
	 return old;
 }

 XRCORE_API PSYSTEMTIME XrLocalTime() 
 {
	 static SYSTEMTIME result;
	 if (TimeGet)
		 TimeGet (&result);
	 else
		 GetLocalTime(&result);

	 return &result;
 }

 XRCORE_API __time64_t XrLocalTime64()
 {
	 PSYSTEMTIME pst = XrLocalTime();
	 struct tm _tm;
	 _tm.tm_mday = pst->wDay;
	 _tm.tm_mon  = pst->wMonth - 1;
	 _tm.tm_wday = pst->wDayOfWeek;
	 _tm.tm_year = pst->wYear - 1900;

	 _tm.tm_hour = pst->wHour;
	 _tm.tm_min  = pst->wMinute;
	 _tm.tm_sec  = pst->wSecond;
	 _tm.tm_isdst = -1;
	 
	 return _mktime64(&_tm) * 1000LL + pst->wMilliseconds;
 }

XRCORE_API LPCSTR format_time(LPCSTR format, __time64_t t, bool add_ms = true)
{
	if (!t)
		return "never";

	u32 msec = 0;
	if (t > 1000000000000LL)
	{
		msec = (t % 1000LL);
		t /= 1000LL;
	}


	static string64 buffers [8];
	static int last_buff = 0;
	char *buff = buffers[last_buff ++];
	last_buff &= 7;
	struct tm * lt = localtime(&t);
	strftime(buff, 63, format, lt);

	if (add_ms)
		sprintf_s(buff, 64, "%s.%03d", buff, msec);

	return buff;
}

 typedef struct  _LOG_LINE
 {
	 __time64_t		ts;
	 char		    msg [4096 - sizeof(__time64_t)];
 } LOG_LINE; 

XRCORE_API void __cdecl ExecLogCB (LPCSTR msg) // alpet: вывод сообщений только в колбек (для отладки и передачи данных в перехватчик)
{

#define RING_SIZE 64
	static CTimer ctx_timer;
	static LOG_LINE ctx_ring[RING_SIZE];   // кольцевой буфер для сохранения данных контекста выполнения (выводится при сбое, или по необходимости)
	static u32 ctx_index		= 0;
	static u32 last_dump_index	= 0;
	if (strstr(msg, "$#DEBUG:"))
	{
		if (!IsDebuggerPresent()) return;
	}


	// функция двойного назначения: может использоваться для вотчинга произвольных переменных в местах потенциальных сбоев

	LPCSTR start = strstr(msg, CONTEXT_TAG);
	if (start && xr_strlen(start) >= 16)
	{
		start += 2;

		// SYSTEMTIME lt;
		// GetLocalTime(&lt);
		float elps = 0;
		if (ctx_index > 0 && g_bFPUInitialized)
			elps = ctx_timer.GetElapsed_sec() * 1000.f;
		else
			dump_timer.Start();

		LOG_LINE &dest = ctx_ring[ctx_index & (RING_SIZE - 1)];
		dest.ts = XrLocalTime64();
		if (dest.ts < 1000000000000LL)
		{
			SYSTEMTIME st;
			GetLocalTime(&st);
			dest.ts *= 1000LL;
			dest.ts += (__time64_t)st.wMilliseconds;
		}
		ctx_index++;
		// copy-paste forever	
		//  lt.wHour, lt.wMinute, lt.wSecond, lt.wMilliseconds, 
		//  %02d:%02d:%02d.%03d
		
		sprintf_s(dest.msg, sizeof(dest.msg) - 1, "[%5.1f]. %s", elps, start);						
		ctx_timer.Start();
		return;
	}
	if (strstr(msg, DUMP_CONTEXT_TAG))
	{
		if (last_dump_index == ctx_index) return;

		last_dump_index = ctx_index;
		dump_timer.Start();
		__time64_t now;
		_time64(&now);
		// время в микросекундах, т.е. надо делить на миллион
#define  ONE_SEC64 1000000

		MsgCB("<DEBUG_CONTEXT_DUMP>"); // рекурсия!
		for (u32 i = RING_SIZE - 1; i > 0; i--)
		{
			LOG_LINE &line = ctx_ring[(ctx_index + i) & (RING_SIZE - 1)];
			if (line.ts + 5 * ONE_SEC64 >= now) // 5 seconds limit
			{
				LPCSTR tm = format_time("%H:%M:%S", line.ts, true);
				if (!strstr(line.msg, DUMP_CONTEXT_TAG))
					 MsgCB("# [%s]%s", tm, line.msg);
				line.ts = 0; // чтобы больше не выводить 
			}
			
		}
		MsgCB("<DEBUG_CONTEXT_DUMP/>");
		return;
	}

	if (NULL != LogCB && msg[0]) LogCB(msg);
}

void 	XRCORE_API	__cdecl  MsgCB(LPCSTR format, ...)
{
	// на этапе вызова этой функции, ещё не инициализированн xrMemory объект
	va_list mark;
	va_start(mark, format);
	char	buf[0x4000]; // 64KiB // alpet: размер буфера лучше сделать побольше, чтобы избежать вылетов invalid parameter handler при выводе стеков вызова
	int sz = _vsnprintf(buf, sizeof(buf) - 1, format, mark); buf[sizeof(buf) - 1] = 0;
	buf[sz] = 0;
	va_end(mark);
	ExecLogCB (buf);
}
