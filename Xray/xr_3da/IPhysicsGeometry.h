#ifndef	__IPHYSICS_GEOMETRY_H__
#define	__IPHYSICS_GEOMETRY_H__

__interface	IPhysicsGeometry
{
	virtual		void		get_Box				(  Fmatrix& form, Fvector&	sz )const	=0;
	virtual		bool		collide_fluids		() const								=0;
};

#endif