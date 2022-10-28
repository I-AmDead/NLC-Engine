#pragma once


class CKinematics;
class CInifile;
class CGameObject;
class moving_bones_snd_player
{
			u16				bone_id;
			float			min_factor;
			float			max_factor;
			float			base_velocity;
			float			smothed_velocity;
			ref_sound		sound;
			Fmatrix			previous_position;
			CKinematics		*kinematics;
public:
			moving_bones_snd_player( CKinematics *K, CInifile* ini, LPCSTR section, const Fmatrix &object  );
			~moving_bones_snd_player();
	void	update( float time_delta, CGameObject &object );
	void	play( CGameObject &O );
	void	stop( );
IC	bool	is_active(){ return true;/*!!sound._feedback();*/ }
private:
	void	load( CKinematics &K, CInifile& ini, LPCSTR section, const Fmatrix &object  );
	Fmatrix	&bone_matrix( );
};

extern moving_bones_snd_player* create_moving_bones_snd_player( CGameObject &O );

IC bool is_active( moving_bones_snd_player* player )
{
	return player && player->is_active();
}