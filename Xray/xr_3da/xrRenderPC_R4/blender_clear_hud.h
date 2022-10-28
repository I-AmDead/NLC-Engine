#pragma once

class CBlender_ClearHUD : public IBlender  
{
public:
	virtual		LPCSTR		getComment()	{ return "INTERNAL: ClearHUD";	}
	virtual		BOOL		canBeDetailed()	{ return FALSE;	}
	virtual		BOOL		canBeLMAPped()	{ return FALSE;	}

	virtual		void		Compile			(CBlender_Compile& C);

	CBlender_ClearHUD();
	virtual ~CBlender_ClearHUD();
};

