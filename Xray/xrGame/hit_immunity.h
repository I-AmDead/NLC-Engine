// hit_immunity.h: ����� ��� ��� ��������, ������� ������������
//				   ������������ ���������� ��� ������ ����� �����
//////////////////////////////////////////////////////////////////////

#pragma once

#include "alife_space.h"
#include "script_export_space.h"
#include "hit_immunity_space.h"

class CHitImmunity
{
	//������������ �� ������� ����������� ���
	//��� ��������������� ���� �����������
	//(��� �������� �������� � ����������� ��������)
protected:
	HitImmunity::HitTypeSVec m_HitImmunityKoefs;
public:
	CHitImmunity();
	virtual ~CHitImmunity();

	virtual void LoadImmunities (LPCSTR section,CInifile* ini);
	void		AddImmunities	(LPCSTR section, CInifile * ini);
	float		GetHitImmunity	(ALife::EHitType hit_type) const	{return m_HitImmunityKoefs[hit_type];}
	virtual float AffectHit		(float power, ALife::EHitType hit_type);

	HitImmunity::HitTypeSVec &immunities() { return m_HitImmunityKoefs; }
	static void	script_register (lua_State *L);

	virtual	CHitImmunity*	cast_hit_immunities() { return this; }
};