#pragma once

#include "interactive_motion.h"
#include "../xr_3da/SkeletonAnimated.h"
class imotion_position;
class imotion_position:
	public interactive_motion
{
private:
struct tracks_update: public IUpdateTracksCallback
	{
		tracks_update( ): motion( 0 ), update( false ) {}
		virtual	bool	operator () ( float dt, CKinematicsAnimated& k );
		imotion_position *motion;
		bool update;
	} update_callback;
	float			time_to_end;
	UpdateCallback	saved_visual_callback;
	CBlend			*blend;

public:
	imotion_position(): interactive_motion(), time_to_end(0.f), saved_visual_callback( 0 ), blend(0)
	{};
private:
	typedef			interactive_motion inherited;
	virtual	void	move_update	(  );
			float	motion_collide		( float dt, CKinematicsAnimated& k );
			void	collide_not_move	( CKinematicsAnimated& KA );
			void	force_calculate_bones( CKinematicsAnimated& KA );
			float	advance_animation	( float dt, CKinematicsAnimated& k );
			float	collide_animation	( float dt, CKinematicsAnimated& k );
			float	move				( float dt, CKinematicsAnimated& k );
			void	disable_update		(bool v);
	virtual	void	collide		(  ){};
	virtual	void	state_end	(  );
	virtual	void	state_start (  );

static	void	rootbone_callback	( CBoneInstance *BI );
		void	init_bones			();
		void	deinit_bones		();
		void	set_root_callback	();
		void	remove_root_callback();

			void	interactive_motion_diagnostic( LPCSTR message );
};