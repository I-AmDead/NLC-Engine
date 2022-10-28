#pragma once

class CBlender_reflections : public IBlender
{
public:
	virtual		LPCSTR		getComment()	{ return "INTERNAL: reflections";	}
	virtual		BOOL		canBeDetailed()	{ return FALSE;	}
	virtual		BOOL		canBeLMAPped()	{ return FALSE;	}

	virtual		void		Compile			(CBlender_Compile& C);

	CBlender_reflections();
	virtual ~CBlender_reflections();
};


