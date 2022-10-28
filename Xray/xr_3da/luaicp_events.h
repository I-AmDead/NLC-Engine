#pragma once
#include <windows.h>
#include "xr_object.h"



typedef struct _GAME_OBJECT_EVENT {
   DWORD msg;		     // event code
   DWORD id;			 // id of object   
   CObject			*O; // client object pointer
   CSE_Abstract *se_obj; // server object pointer
   DWORD res[16];		 // reserved   

} GAME_OBJECT_EVENT, *PGAME_OBJECT_EVENT;


#define   EVT_OBJECT_ADD              0x0001 // ������ ������
#define   EVT_OBJECT_ACTIVATE         0x0002 // ����������� ����
#define	  EVT_OBJECT_SLEEP            0x0003 // �� ����������� �����
#define   EVT_OBJECT_REMOVE           0x0004 // ����� �� �������
#define   EVT_OBJECT_PARENT           0x0005 // ��������� �������� �������
#define   EVT_OBJECT_REJECT           0x0006 // DROP ��� ���-�� � ����� ����
#define   EVT_OBJECT_SPAWN            0x0007 // �������� ����� ����������� �������
#define   EVT_OBJECT_DESTROY          0x0008 // ��������� ������
#define   EVT_OBJECT_SERVER		      0x1000 // ��������� ������
#define   EVT_OBJECT_CLIENT		      0x2000 // ���������� ������





__declspec(dllimport) void FastObjectEvent(PGAME_OBJECT_EVENT evt);

inline void process_object_event(DWORD msg, DWORD id, CObject *O, CSE_Abstract *se_obj = NULL, DWORD r0 = 0, DWORD r1 = 0) // ��� ����������� �������
{
	GAME_OBJECT_EVENT evt;
	// Memory.mem_fill(&evt, 0, sizeof(evt));
	evt.msg = msg;
	evt.id  = id;
	evt.O	= O;
	evt.se_obj = se_obj;	
	evt.res[0] = r0;
	evt.res[1] = r1;	
	FastObjectEvent(&evt);
}

