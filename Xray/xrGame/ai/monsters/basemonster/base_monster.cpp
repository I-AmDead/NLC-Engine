#include "stdafx.h"
#include "base_monster.h"
#include "../../../PhysicsShell.h"
#include "../../../hit.h"
#include "../../../PHDestroyable.h"
#include "../../../CharacterPhysicsSupport.h"
#include "../../../game_level_cross_table.h"
#include "../../../game_graph.h"
#include "../../../phmovementcontrol.h"
#include "../ai_monster_squad_manager.h"
#include "../../../xrserver_objects_alife_monsters.h"
#include "../corpse_cover.h"
#include "../../../cover_evaluators.h"
#include "../../../seniority_hierarchy_holder.h"
#include "../../../team_hierarchy_holder.h"
#include "../../../squad_hierarchy_holder.h"
#include "../../../group_hierarchy_holder.h"
#include "../../../phdestroyable.h"
#include "../../../../xr_3da/SkeletonAnimated.h"
#include "../../../detail_path_manager.h"
#include "../../../memory_manager.h"
#include "../../../visual_memory_manager.h"
#include "../monster_velocity_space.h"
#include "../../../entitycondition.h"
#include "../../../sound_player.h"
#include "../../../level.h"
#include "../state_manager.h"
#include "../controlled_entity.h"
#include "../control_animation_base.h"
#include "../control_direction_base.h"
#include "../control_movement_base.h"
#include "../control_path_builder_base.h"
#include "../anomaly_detector.h"
#include "../monster_cover_manager.h"
#include "../monster_home.h"
#include "../../../inventory.h"
#include "../../../xrserver.h"
#include "../ai_monster_squad.h"
#include "../../../actor.h"
#include "../../../ai_object_location.h"
#include "../../../ai_space.h"
#include "../../../script_engine.h"
#include "../../../../xrCore/_vector3d_ext.h"
#include "../anti_aim_ability.h"

// Lain: added 
#include "../../../level_debug.h"
#include "../../../../xr_3da/xrLevel.h"
#include "../../../level_graph.h"
#ifdef DEBUG
#include "debug_text_tree.h"
#endif

#pragma warning (disable:4355)
#pragma warning (push)

CBaseMonster::CBaseMonster() :	m_psy_aura(this, "psy"), 
								m_fire_aura(this, "fire"), 
								m_radiation_aura(this, "radiation"), 
								m_base_aura(this, "base")
{
	m_pPhysics_support=xr_new<CCharacterPhysicsSupport>(CCharacterPhysicsSupport::etBitting,this);
	
	m_pPhysics_support				->in_Init();

	// Components external init 
	
	m_control_manager				= xr_new<CControl_Manager>(this);

	EnemyMemory.init_external		(this, 20000);
	SoundMemory.init_external		(this, 20000);
	CorpseMemory.init_external		(this, 20000);
	HitMemory.init_external			(this, 50000);

	EnemyMan.init_external			(this);
	CorpseMan.init_external			(this);

	// ������������� ���������� ��������	

	StateMan						= 0;

	MeleeChecker.init_external		(this);
	Morale.init_external			(this);

	m_controlled					= 0;
	
	control().add					(&m_com_manager,  ControlCom::eControlCustom);
	
	m_com_manager.add_ability		(ControlCom::eControlSequencer);
	m_com_manager.add_ability		(ControlCom::eControlTripleAnimation);


	m_anomaly_detector				= xr_new<CAnomalyDetector>(this);
	CoverMan						= xr_new<CMonsterCoverManager>(this);

	Home							= xr_new<CMonsterHome>(this);

	com_man().add_ability				(ControlCom::eComCriticalWound);

	EatedCorpse								=	NULL;

	//m_steer_manager							=	NULL;
	m_grouping_behaviour					=	NULL;

	m_feel_enemy_who_just_hit_max_distance	=	0;
	m_feel_enemy_max_distance				=	0;

	m_anti_aim								=	NULL;
	m_head_bone_name						=	"bip01_head";

	m_first_tick_enemy_inaccessible			=	0;
	m_last_tick_enemy_inaccessible			=	0;
	m_first_tick_object_not_at_home			=	0;
}

#pragma warning (pop)

CBaseMonster::~CBaseMonster()
{
	xr_delete(m_anti_aim);
	//xr_delete(m_steer_manager);
	xr_delete(m_pPhysics_support);
	xr_delete(m_corpse_cover_evaluator);
	xr_delete(m_enemy_cover_evaluator);
	xr_delete(m_cover_evaluator_close_point);
	
	xr_delete(m_control_manager);

	xr_delete(m_anim_base);
	xr_delete(m_move_base);
	xr_delete(m_path_base);
	xr_delete(m_dir_base);

	xr_delete(m_anomaly_detector);
	xr_delete(CoverMan);
	xr_delete(Home);
}

bool   accessible_epsilon (CBaseMonster * const object, Fvector const pos, float epsilon)
{
	Fvector const offsets[]			=	{	Fvector().set( 0.f,			0.f,	0.f),
											Fvector().set(- epsilon, 	0.f,  	0.f),
											Fvector().set(+ epsilon, 	0.f,  	0.f),
											Fvector().set( 0.f,			0.f, 	- epsilon),
											Fvector().set( 0.f,			0.f, 	+ epsilon)	};
	
	for ( u32 i=0; i<sizeof(offsets)/sizeof(offsets[0]); ++i )
	{
		if ( object->movement().restrictions().accessible(pos + offsets[i]) )
			return						true;
	}

	return								false;
}

static
bool enemy_inaccessible (CBaseMonster * const object)
{
	CEntityAlive const * enemy		=	object->EnemyMan.get_enemy();
	if ( !enemy )
		return							false;

	Fvector const enemy_pos			=	enemy->Position();
	Fvector const enemy_vert_pos	=	ai().level_graph().vertex_position(enemy->ai_location().level_vertex_id());
	
	float const xz_dist_to_vertex	=	enemy_vert_pos.distance_to_xz(enemy_pos);
	float const y_dist_to_vertex	=	_abs(enemy_vert_pos.y - enemy_pos.y);

	if ( xz_dist_to_vertex > 0.5f && y_dist_to_vertex > 3.f )
		return							true;

	if ( xz_dist_to_vertex > 1.2f )
		return							true;

	if ( !object->Home->at_home(enemy_pos) )
		return							true;

	if ( !accessible_epsilon(object, enemy_pos, 1.5f) )
		return							true;

	if ( !ai().level_graph().valid_vertex_position(enemy_pos) )
		return							true;
	
	if ( !ai().level_graph().valid_vertex_id(enemy->ai_location().level_vertex_id()) )
		return							true;
	
	return								false;
}

bool CBaseMonster::enemy_accessible ()
{
	if ( !m_first_tick_enemy_inaccessible )
		return							true;

	if ( EnemyMan.get_enemy() )
	{
		u32 const enemy_vertex		=	EnemyMan.get_enemy()->ai_location().level_vertex_id();
		if ( ai_location().level_vertex_id() == enemy_vertex )
			return						false;
	}

	if ( Device.dwTimeGlobal < m_first_tick_enemy_inaccessible + 3000 )
		return							true;

	return								false;
}

bool CBaseMonster::at_home ()
{
	return										!m_first_tick_object_not_at_home ||
												(Device.dwTimeGlobal < m_first_tick_object_not_at_home + 4000);
}

void CBaseMonster::update_enemy_accessible_and_at_home_info	()
{
	if ( !Home->at_home() )
	{
		if ( !m_first_tick_object_not_at_home )
			m_first_tick_object_not_at_home	=	Device.dwTimeGlobal;
	}
	else
		m_first_tick_object_not_at_home		=	0;

	if ( !EnemyMan.get_enemy() )
	{
		m_first_tick_enemy_inaccessible		=	0;
		m_last_tick_enemy_inaccessible		=	0;
		return;
	}

	if ( ::enemy_inaccessible(this) )
	{
		if ( !m_first_tick_enemy_inaccessible )
			m_first_tick_enemy_inaccessible	=	Device.dwTimeGlobal;

		m_last_tick_enemy_inaccessible		=	Device.dwTimeGlobal;
	}
	else
	{
		if ( m_last_tick_enemy_inaccessible && Device.dwTimeGlobal - m_last_tick_enemy_inaccessible > 3000 )
		{
			m_first_tick_enemy_inaccessible	=	0;
			m_last_tick_enemy_inaccessible	=	0;
		}
	}
}

void CBaseMonster::UpdateCL()
{
#ifdef DEBUG
	if ( Level().CurrentEntity() == this )
	{
		DBG().get_text_tree().clear();
		add_debug_info(DBG().get_text_tree());
	}
	if ( is_paused () )
	{
		return;
	}
#endif

	if ( EatedCorpse && !CorpseMemory.is_valid_corpse(EatedCorpse) )
	{
		EatedCorpse = NULL;
	}

	inherited::UpdateCL();
	
	if ( g_Alive() ) 
	{
		update_enemy_accessible_and_at_home_info();
		CStepManager::update(false);
	}

	control().update_frame();

	m_pPhysics_support->in_UpdateCL();
}

void CBaseMonster::shedule_Update(u32 dt)
{
#ifdef DEBUG
	if ( is_paused () )
	{
		dbg_update_cl	= Device.dwFrame;
		return;
	}
#endif

	inherited::shedule_Update	(dt);

	update_eyes_visibility		();

	if ( m_anti_aim )
	{
		m_anti_aim->update_schedule();
	}

	m_psy_aura.update_schedule();
	m_fire_aura.update_schedule();
	m_base_aura.update_schedule();
	m_radiation_aura.update_schedule();

	control().update_schedule	();

	Morale.update_schedule		(dt);

	m_anomaly_detector->update_schedule();
	
	m_pPhysics_support->in_shedule_Update(dt);

#ifdef DEBUG	
	show_debug_info();
#endif
}


//////////////////////////////////////////////////////////////////////
// Other functions
//////////////////////////////////////////////////////////////////////


void CBaseMonster::Die(CObject* who)
{
	if (StateMan) StateMan->critical_finalize();

	m_psy_aura.on_monster_death();
	m_radiation_aura.on_monster_death();
	m_fire_aura.on_monster_death();
	m_base_aura.on_monster_death();

	if ( m_anti_aim )
	{
		m_anti_aim->on_monster_death ();
	}

	inherited::Die(who);

	if (is_special_killer(who))
		sound().play			(MonsterSound::eMonsterSoundDieInAnomaly);
	else
		sound().play			(MonsterSound::eMonsterSoundDie);

	monster_squad().remove_member	((u8)g_Team(),(u8)g_Squad(),(u8)g_Group(),this);

	if ( m_grouping_behaviour )
	{
		m_grouping_behaviour->set_squad(NULL);
	}
	
	
	if (m_controlled)			m_controlled->on_die();
}


void CBaseMonster::Hit(SHit* pHDS)
{
	if(ignore_collision_hit && (pHDS->hit_type == ALife::eHitTypeStrike)) 
		return;
	
	if(invulnerable())
		return;

	if(g_Alive())
		if(!critically_wounded()) 
			update_critical_wounded(pHDS->boneID,pHDS->power);
	
	if(pHDS->hit_type == ALife::eHitTypeFireWound)
	{
		float &hit_power = pHDS->power;
		float ap = pHDS->armor_piercing;
		// ���� ������� �����
		if(!fis_zero(m_fSkinArmor, EPS) && ap > m_fSkinArmor)
		{
			float d_hit_power = (ap - m_fSkinArmor) / ap;
			if(d_hit_power < m_fHitFracMonster)
				d_hit_power = m_fHitFracMonster;

			hit_power *= d_hit_power;
			VERIFY(hit_power>=0.0f);
		}
		// ���� �� ������� �����
		else
		{
			hit_power *= m_fHitFracMonster;
			//pHDS->add_wound = false; 	//���� ���
		}
	}
	inherited::Hit(pHDS);
}

void CBaseMonster::PHHit(SHit &H)
{
	m_pPhysics_support->in_Hit( H );
}

CPHDestroyable*	CBaseMonster::ph_destroyable()
{
	return smart_cast<CPHDestroyable*>(character_physics_support());
}

bool CBaseMonster::useful(const CItemManager *manager, const CGameObject *object) const
{
	const Fvector& object_pos = object->Position();
	if (!movement().restrictions().accessible(object_pos))
	{
		return false;
	}

	// Lain: added (temp?) guard due to bug http://tiger/bugz/view.php?id=15983
	// sometimes accessible(object->Position())) returns true
	// but accessible(ai_location().level_vertex_id()) crashes 
	// because level_vertex_id is not valid, so this code syncs vertex_id with position
	if ( !ai().level_graph().valid_vertex_id(object->ai_location().level_vertex_id()) )
	{
		u32 vertex_id = ai().level_graph().vertex_id(object_pos);
		if ( !ai().level_graph().valid_vertex_id(vertex_id) )
		{
			return false;
		}
		object->ai_location().level_vertex(vertex_id);
	}

	if ( !movement().restrictions().accessible(object->ai_location().level_vertex_id()) )
	{
		return false;
	}

	const CEntityAlive *pCorpse = smart_cast<const CEntityAlive *>(object); 
	if ( !pCorpse ) 
	{
		return false;
	}
	
	if ( !pCorpse->g_Alive() )
	{
		return true;
	}

	return false;
}

float CBaseMonster::evaluate(const CItemManager *manager, const CGameObject *object) const
{
	return (0.f);
}

//////////////////////////////////////////////////////////////////////////

void CBaseMonster::ChangeTeam(int team, int squad, int group)
{
	if ((team == g_Team()) && (squad == g_Squad()) && (group == g_Group())) return;

#ifdef DEBUG
	if (!g_Alive()) {
		ai().script_engine().print_stack	();
		VERIFY2								(g_Alive(),"you are trying to change team of a dead entity");
	}
#endif // DEBUG

	// remove from current team
	monster_squad().remove_member	((u8)g_Team(),(u8)g_Squad(),(u8)g_Group(),this);
	inherited::ChangeTeam			(team,squad,group);
	monster_squad().register_member	((u8)g_Team(),(u8)g_Squad(),(u8)g_Group(), this);

	if ( m_grouping_behaviour )
	{
		m_grouping_behaviour->set_squad( monster_squad().get_squad(this) );
	}	
}

void CBaseMonster::SetTurnAnimation(bool turn_left)
{
	(turn_left) ? anim().SetCurAnim(eAnimStandTurnLeft) : anim().SetCurAnim(eAnimStandTurnRight);
}

void CBaseMonster::set_state_sound(u32 type, bool once)
{
	if (once) {
	
		sound().play(type);
	
	} else {

		// handle situation, when monster want to play attack sound for the first time
		if ((type == MonsterSound::eMonsterSoundAggressive) && 
			(m_prev_sound_type != MonsterSound::eMonsterSoundAggressive)) {
			
			sound().play(MonsterSound::eMonsterSoundAttackHit);

		} else {
			// get count of monsters in squad
			u8 objects_count = monster_squad().get_squad(this)->get_count(this, 20.f);

			// include myself
			objects_count++;
			VERIFY(objects_count > 0);

			u32 delay = 0;
			switch (type) {
			case MonsterSound::eMonsterSoundIdle : 
				// check distance to actor

				if (Actor()->Position().distance_to(Position()) > db().m_fDistantIdleSndRange) {
					delay = u32(float(db().m_dwDistantIdleSndDelay) * _sqrt(float(objects_count)));
					type  = MonsterSound::eMonsterSoundIdleDistant;
				} else {
					delay = u32(float(db().m_dwIdleSndDelay) * _sqrt(float(objects_count)));
				}
				
				break;
			case MonsterSound::eMonsterSoundEat:
				delay = u32(float(db().m_dwEatSndDelay) * _sqrt(float(objects_count)));
				break;
			case MonsterSound::eMonsterSoundAggressive:
			case MonsterSound::eMonsterSoundPanic:
				delay = u32(float(db().m_dwAttackSndDelay) * _sqrt(float(objects_count)));
				break;
			}

			sound().play(type, 0, 0, delay);
		} 
	}

	m_prev_sound_type	= type;
}

BOOL CBaseMonster::feel_touch_on_contact	(CObject *O)
{
	return		(inherited::feel_touch_on_contact(O));
}

BOOL CBaseMonster::feel_touch_contact(CObject *O)
{
	m_anomaly_detector->on_contact(O);
	return inherited::feel_touch_contact(O);
}

void CBaseMonster::TranslateActionToPathParams()
{
	bool bEnablePath = true;
	u32 vel_mask = 0;
	u32 des_mask = 0;

	EAction action	=	anim().m_tAction;
	switch (action) 
	{
	case ACT_STAND_IDLE: 
	case ACT_SIT_IDLE:	 
	case ACT_LIE_IDLE:
	case ACT_EAT:
	case ACT_SLEEP:
	case ACT_REST:
		//jump
	//case ACT_JUMP:
	case ACT_LOOK_AROUND:
		bEnablePath = false;
		break;
	case ACT_ATTACK:
		if ( !m_attack_on_move_params.enabled )
		{
			bEnablePath = false;
		}
		else
		{
			if (m_bDamaged) {
				vel_mask = MonsterMovement::eVelocityParamsRunDamaged;
				des_mask = MonsterMovement::eVelocityParameterRunDamaged;
			} else {
				vel_mask = MonsterMovement::eVelocityParamsRun;
				des_mask = MonsterMovement::eVelocityParameterRunNormal;
			}
		}
		break;

	case ACT_HOME_WALK_GROWL:
		vel_mask = MonsterMovement::eVelocityParamsWalkGrowl;
		des_mask = MonsterMovement::eVelocityParameterWalkGrowl;
		break;

	case ACT_HOME_WALK_SMELLING:
		vel_mask = MonsterMovement::eVelocityParamsWalkSmelling;
		des_mask = MonsterMovement::eVelocityParameterWalkSmelling;
		break;
	case ACT_WALK_FWD:
		if (m_bDamaged) {
			vel_mask = MonsterMovement::eVelocityParamsWalkDamaged;
			des_mask = MonsterMovement::eVelocityParameterWalkDamaged;
		} else {
			vel_mask = MonsterMovement::eVelocityParamsWalk;
			des_mask = MonsterMovement::eVelocityParameterWalkNormal;
		}
		break;
	case ACT_WALK_BKWD:
		break;
	case ACT_RUN:
		if (m_bDamaged) {
			vel_mask = MonsterMovement::eVelocityParamsRunDamaged;
			des_mask = MonsterMovement::eVelocityParameterRunDamaged;
		} else {
			vel_mask = MonsterMovement::eVelocityParamsRun;
			des_mask = MonsterMovement::eVelocityParameterRunNormal;
		}
		break;
	case ACT_DRAG:
		vel_mask = MonsterMovement::eVelocityParamsDrag;
		des_mask = MonsterMovement::eVelocityParameterDrag;

		anim().SetSpecParams(ASP_MOVE_BKWD);

		break;
	case ACT_STEAL:
		vel_mask = MonsterMovement::eVelocityParamsSteal;
		des_mask = MonsterMovement::eVelocityParameterSteal;
		break;
	}

	if (state_invisible) {
		vel_mask = MonsterMovement::eVelocityParamsInvisible;
		des_mask = MonsterMovement::eVelocityParameterInvisible;
	}

	if (m_force_real_speed) vel_mask = des_mask;

	if (bEnablePath) {
		path().set_velocity_mask	(vel_mask);
		path().set_desirable_mask	(des_mask);
		path().enable_path			();
	} else {
		path().disable_path			();
	}
}

u32 CBaseMonster::get_attack_rebuild_time()
{
	float dist = EnemyMan.get_enemy()->Position().distance_to(Position());
	return (100 + u32(20.f * dist));
}

void CBaseMonster::on_kill_enemy(const CEntity *obj)
{
	const CEntityAlive *entity	= smart_cast<const CEntityAlive *>(obj);
	
	// �������� � ������ ������	
	CorpseMemory.add_corpse		(entity);
	
	// ������� ��� ���������� � �����
	HitMemory.remove_hit_info	(entity);

	// ������� ��� ���������� � ������
	SoundMemory.clear			();
}

CMovementManager *CBaseMonster::create_movement_manager	()
{
	m_movement_manager = xr_new<CControlPathBuilder>(this);

	control().add					(m_movement_manager, ControlCom::eControlPath);
	control().install_path_manager	(m_movement_manager);
	control().set_base_controller	(m_path_base, ControlCom::eControlPath);

	return			(m_movement_manager);
}

DLL_Pure *CBaseMonster::_construct	()
{
	create_base_controls			();

	control().add					(m_anim_base, ControlCom::eControlAnimationBase);
	control().add					(m_move_base, ControlCom::eControlMovementBase);
	control().add					(m_path_base, ControlCom::eControlPathBase);
	control().add					(m_dir_base,  ControlCom::eControlDirBase);

	control().set_base_controller	(m_anim_base, ControlCom::eControlAnimation);
	control().set_base_controller	(m_move_base, ControlCom::eControlMovement);
	control().set_base_controller	(m_dir_base,  ControlCom::eControlDir);
	
	inherited::_construct		();
	CStepManager::_construct	();
	CInventoryOwner::_construct	();
	return						(this);
}

void CBaseMonster::net_Relcase(CObject *O)
{
	inherited::net_Relcase(O);

	StateMan->remove_links			(O);

	com_man().remove_links			(O);

	// TODO: do not clear, remove only object O
	if (g_Alive()) {
		EnemyMemory.remove_links	(O);
		SoundMemory.remove_links	(O);
		HitMemory.remove_hit_info	(O);

		EnemyMan.remove_links		(O);
		CorpseMan.remove_links		(O);

		UpdateMemory				();
		
		monster_squad().remove_links(O);
	}
	CorpseMemory.remove_links		(O);
	m_pPhysics_support->in_NetRelcase(O);
}
	
void CBaseMonster::create_base_controls()
{
	m_anim_base		= xr_new<CControlAnimationBase>		();
	m_move_base		= xr_new<CControlMovementBase>		();
	m_path_base		= xr_new<CControlPathBuilderBase>	();
	m_dir_base		= xr_new<CControlDirectionBase>		();
}

void CBaseMonster::set_action(EAction action)
{
	anim().m_tAction		= action;
}

CParticlesObject* CBaseMonster::PlayParticles(const shared_str& name, const Fvector &position, const Fvector &dir, BOOL auto_remove, BOOL xformed)
{
	CParticlesObject* ps = CParticlesObject::Create(name.c_str(),auto_remove);
	
	// ��������� ������� � �������������� ��������
	Fmatrix	matrix; 

	matrix.identity			();
	matrix.k.set			(dir);
	Fvector::generate_orthonormal_basis_normalized(matrix.k,matrix.j,matrix.i);
	matrix.translate_over	(position);
	
	(xformed) ?				ps->SetXFORM (matrix) : ps->UpdateParent(matrix,zero_vel); 
	ps->Play				();

	return ps;
}

void CBaseMonster::on_restrictions_change()
{
	inherited::on_restrictions_change();

	if (StateMan) StateMan->reinit();
}

void CBaseMonster::load_effector(LPCSTR section, LPCSTR line, SAttackEffector &effector)
{
	LPCSTR ppi_section = pSettings->r_string(section, line);
	effector.ppi.duality.h			= pSettings->r_float(ppi_section,"duality_h");
	effector.ppi.duality.v			= pSettings->r_float(ppi_section,"duality_v");
	effector.ppi.gray				= pSettings->r_float(ppi_section,"gray");
	effector.ppi.blur				= pSettings->r_float(ppi_section,"blur");
	effector.ppi.noise.intensity	= pSettings->r_float(ppi_section,"noise_intensity");
	effector.ppi.noise.grain		= pSettings->r_float(ppi_section,"noise_grain");
	effector.ppi.noise.fps			= pSettings->r_float(ppi_section,"noise_fps");
	VERIFY(!fis_zero(effector.ppi.noise.fps));

	sscanf(pSettings->r_string(ppi_section,"color_base"),	"%f,%f,%f", &effector.ppi.color_base.r,	&effector.ppi.color_base.g,	&effector.ppi.color_base.b);
	sscanf(pSettings->r_string(ppi_section,"color_gray"),	"%f,%f,%f", &effector.ppi.color_gray.r,	&effector.ppi.color_gray.g,	&effector.ppi.color_gray.b);
	sscanf(pSettings->r_string(ppi_section,"color_add"),	"%f,%f,%f", &effector.ppi.color_add.r,	&effector.ppi.color_add.g,	&effector.ppi.color_add.b);

	effector.time				= pSettings->r_float(ppi_section,"time");
	effector.time_attack		= pSettings->r_float(ppi_section,"time_attack");
	effector.time_release		= pSettings->r_float(ppi_section,"time_release");

	effector.ce_time			= pSettings->r_float(ppi_section,"ce_time");
	effector.ce_amplitude		= pSettings->r_float(ppi_section,"ce_amplitude");
	effector.ce_period_number	= pSettings->r_float(ppi_section,"ce_period_number");
	effector.ce_power			= pSettings->r_float(ppi_section,"ce_power");
}

bool CBaseMonster::check_start_conditions(ControlCom::EControlType type)
{
	if ( !StateMan->check_control_start_conditions(type) )
	{
		return					false;
	}

	if ( type == ControlCom::eControlRotationJump )
	{
		EMonsterState state	=	StateMan->get_state_type();
		
		if ( !is_state(state, eStateAttack_Run) && 
			 !is_state(state, eStateAttack_RunAttack) ) 
		{
			return false;
		}
	} 
	if ( type == ControlCom::eControlMeleeJump ) 
	{
		EMonsterState state	=	StateMan->get_state_type();

		if (!is_state(state, eStateAttack_Run) && 
			!is_state(state, eStateAttack_Melee) &&
			!is_state(state, eStateAttack_RunAttack) ) 
		{
			return				false;
		}
	}

	return						true;
}

void CBaseMonster::OnEvent(NET_Packet& P, u16 type)
{
	inherited::OnEvent			(P,type);
	CInventoryOwner::OnEvent	(P,type);

	u16			id;
	switch (type){
	case GE_OWNERSHIP_TAKE:
		{
			P.r_u16		(id);
			CObject		*O	= Level().Objects.net_Find	(id);
			VERIFY		(O);

			CGameObject			*GO = smart_cast<CGameObject*>(O);
			CInventoryItem		*pIItem = smart_cast<CInventoryItem*>(GO);
			VERIFY				(inventory().CanTakeItem(pIItem));
			pIItem->m_eItemPlace = eItemPlaceRuck;

			O->H_SetParent		(this);
			inventory().Take	(GO, true, true);
		break;
		}
	case GE_TRADE_SELL:

	case GE_OWNERSHIP_REJECT:
		{
			P.r_u16		(id);
			CObject* O	= Level().Objects.net_Find	(id);
			VERIFY		(O);

			bool just_before_destroy		= !P.r_eof() && P.r_u8();
			bool dont_create_shell			= (type==GE_TRADE_SELL) || just_before_destroy;

			O->SetTmpPreDestroy				(just_before_destroy);
			if (inventory().DropItem(smart_cast<CGameObject*>(O), dont_create_shell) && !O->getDestroy()) 
			{
				//O->H_SetParent	(0,just_before_destroy); //moved to DropItem
				feel_touch_deny	(O,2000);
			}
		}
		break;

	case GE_KILL_SOMEONE:
		P.r_u16		(id);
		CObject* O	= Level().Objects.net_Find	(id);

		if (O)  {
			CEntity *pEntity = smart_cast<CEntity*>(O);
			if (pEntity) on_kill_enemy(pEntity);
		}
			
		break;
	}
}

// Lain: added
bool   CBaseMonster::check_eated_corpse_draggable()
{
	const CEntity* p_corpse = EatedCorpse;
	if ( !p_corpse || !p_corpse->Visual() )
	{
		return false;
	}
	
	if ( CKinematics* K = p_corpse->Visual()->dcast_PKinematics() )
	{
		if ( CInifile* ini = K->LL_UserData() )
		{
			return ini->section_exist("capture_used_bones") && ini->line_exist("capture_used_bones", "bones");
		}
	}

	return false;	
}

//-------------------------------------------------------------------
// CBaseMonster's  Atack on Move
//-------------------------------------------------------------------

bool   CBaseMonster::can_attack_on_move()
{
	return override_if_debug("aom_enabled", m_attack_on_move_params.enabled);
}

float   CBaseMonster::get_attack_on_move_max_go_close_time()
{
	return override_if_debug("aom_max_go_close_time", m_attack_on_move_params.max_go_close_time);
}

float   CBaseMonster::get_attack_on_move_far_radius()
{
	float radius	=	override_if_debug("aom_far_radius", m_attack_on_move_params.far_radius);
	clamp				(radius, 0.f, 100.f);
	return				radius;
}

float   CBaseMonster::get_attack_on_move_attack_radius()
{
	return override_if_debug("aom_attack_radius", m_attack_on_move_params.attack_radius);
}

float   CBaseMonster::get_attack_on_move_update_side_period()
{
	return override_if_debug("aom_update_side_period", m_attack_on_move_params.update_side_period);
}

float   CBaseMonster::get_attack_on_move_prediction_factor()
{
	return override_if_debug("aom_prediction_factor", m_attack_on_move_params.prediction_factor);
}

float   CBaseMonster::get_attack_on_move_prepare_radius()
{
	return override_if_debug("aom_prepare_radius", m_attack_on_move_params.prepare_radius);
}

float   CBaseMonster::get_attack_on_move_prepare_time()
{
	return override_if_debug("aom_prepare_time", m_attack_on_move_params.prepare_time);
}

float   CBaseMonster::get_psy_influence ()
{
	return m_psy_aura.calculate();
}

float   CBaseMonster::get_radiation_influence ()
{
	return m_radiation_aura.calculate();
}

float   CBaseMonster::get_fire_influence ()
{
	return m_fire_aura.calculate();
}

void   CBaseMonster::play_detector_sound()
{
	m_psy_aura.play_detector_sound();
	m_radiation_aura.play_detector_sound();
	m_fire_aura.play_detector_sound();
}

bool CBaseMonster::is_jumping()
{
	return m_com_manager.is_jumping();
}

void CBaseMonster::update_eyes_visibility ()
{
	if ( !m_left_eye_bone_name )
	{
		return;
	}

	CKinematics* const skeleton	=	smart_cast<CKinematics*>(Visual());
	if ( !skeleton )
	{
		return;
	}

	u16 const left_eye_bone_id	=	skeleton->LL_BoneID(m_left_eye_bone_name);
	u16 const right_eye_bone_id	=	skeleton->LL_BoneID(m_right_eye_bone_name);

	R_ASSERT						(left_eye_bone_id != u16(-1) && right_eye_bone_id != u16(-1));

	bool eyes_visible			=	!g_Alive() || get_screen_space_coverage_diagonal() > 0.05f;

	bool const was_visible		=	!!skeleton->LL_GetBoneVisible	(left_eye_bone_id);
	skeleton->LL_SetBoneVisible		(left_eye_bone_id, eyes_visible, true);
	skeleton->LL_SetBoneVisible		(right_eye_bone_id, eyes_visible, true);

	if ( !was_visible && eyes_visible )
	{
		skeleton->CalculateBones_Invalidate();
		skeleton->CalculateBones		();
	}
}

float CBaseMonster::get_screen_space_coverage_diagonal()
{
	Fbox		b		= Visual()->getVisData().box;

	Fmatrix				xform;
	xform.mul			(Device.mFullTransform,XFORM());
	Fvector2	mn		={flt_max,flt_max},mx={flt_min,flt_min};

	for (u32 k=0; k<8; ++k)
	{
		Fvector p;
		b.getpoint		(k,p);
		xform.transform	(p);
		mn.x			= _min(mn.x,p.x);
		mn.y			= _min(mn.y,p.y);
		mx.x			= _max(mx.x,p.x);
		mx.y			= _max(mx.y,p.y);
	}

	float const width	=	mx.x - mn.x;
	float const height	=	mx.y - mn.y;

	float const	average_diagonal	=	_sqrt(width * height);
	return				average_diagonal;
}

#ifdef DEBUG

bool   CBaseMonster::is_paused () const
{
	bool monsters_result		=	false;	
	ai_dbg::get_var					("monsters_paused", monsters_result);

	u32 const id				=	ID();
	char id_paused_var_name			[128];
	xr_sprintf						(id_paused_var_name, sizeof(id_paused_var_name), "%d_paused", id);

	bool monster_result			=	false;

	return							ai_dbg::get_var (id_paused_var_name, monster_result) ?
									monster_result : monsters_result;
}

#endif // DEBUG