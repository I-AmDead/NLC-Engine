///////////////////////////////////////////////////////////////
// ExoOutfit.h
// ExoOutfit - �������� ������ � ���������
///////////////////////////////////////////////////////////////


#pragma once

#include "customoutfit.h"

class CExoOutfit: public CCustomOutfit
{
private:
    typedef	CCustomOutfit inherited;
protected:	
	float							m_fBatteryCharge;	// ����� ������� ��� ������������
	float							m_fBatteryCritical;	// ������� ������������ ������
	bool							m_bPowered;			// �������� �� ���� �������

public:
	CExoOutfit(void);
	virtual ~CExoOutfit(void);


	virtual float					GetAdditionalWeight		(int index = 1) const;
	virtual float					GetBatteryCharge		()		const { return m_fBatteryCharge; }
	virtual void					SetBatteryCharge		(float value) { m_fBatteryCharge = value; }

	virtual float					GetPowerLoss		();
	virtual void					Load				(LPCSTR section);

	virtual void					net_Export			(NET_Packet& P);
	virtual void					net_Import			(NET_Packet& P);

};
