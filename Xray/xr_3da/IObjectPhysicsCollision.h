#pragma once

__interface IPhysicsShell;
__interface IPhysicsElement;
__interface IObjectPhysicsCollision
{
public:
	virtual	const IPhysicsShell		*physics_shell()const		= 0;
	virtual const IPhysicsElement	*physics_character()const	= 0;//depricated
};