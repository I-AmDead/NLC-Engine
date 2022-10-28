////////////////////////////////////////////////////////////////////////////
//	Module 		: cover_evaluators.h
//	Created 	: 24.04.2004
//  Modified 	: 24.04.2004
//	Author		: Dmitriy Iassenev
//	Description : Cover evaluators
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "cover_evaluators.h"
#include "cover_point.h"
#include "ai_space.h"
#include "level_graph.h"
#include "game_graph.h"
#include "game_level_cross_table.h"

void CCoverEvaluatorBase::evaluate							(CCoverPoint const *cover_point, float weight)
{
	evaluate_cover			(cover_point, weight);
}

//////////////////////////////////////////////////////////////////////////
// CCoverEvaluatorCloseToEnemy
//////////////////////////////////////////////////////////////////////////

void CCoverEvaluatorCloseToEnemy::evaluate_cover	(const CCoverPoint *cover_point, float weight)
{
	float					enemy_distance	= m_enemy_position.distance_to(cover_point->position());
	//float					my_distance		= m_start_position.distance_to(cover_point->position());

	if ((enemy_distance <= m_min_distance) && (m_current_distance > enemy_distance))
		return;

	if((enemy_distance >= m_max_distance) && (m_current_distance < enemy_distance))
		return;

	if (enemy_distance >= m_current_distance + m_deviation)
		return;

	//Fvector				direction;
	//float					y,p;
	//direction.sub			(m_enemy_position,cover_point->position());
	//direction.getHP		(y,p);
	//y						= angle_normalize(y);
	//float					cover_value = ai().level_graph().cover_in_direction(y,cover_point->level_vertex_id());
	//if (cover_value >= m_best_value)
	//	return;

	if (enemy_distance >= m_best_value)
		return;

	m_selected				= cover_point;
	m_best_value			= enemy_distance;
	//m_best_distance		= my_distance;
}

//////////////////////////////////////////////////////////////////////////
// CCoverEvaluatorFarFromEnemy
//////////////////////////////////////////////////////////////////////////

void CCoverEvaluatorFarFromEnemy::evaluate_cover	(const CCoverPoint *cover_point, float weight)
{
	float					enemy_distance	= m_enemy_position.distance_to(cover_point->position());
//	float					my_distance		= m_start_position.distance_to(cover_point->position());

	if ((enemy_distance <= m_min_distance) && (m_current_distance > enemy_distance))
		return;

	if((enemy_distance >= m_max_distance) && (m_current_distance < enemy_distance))
		return;

	if (enemy_distance <= m_current_distance - m_deviation)
		return;

//	Fvector					direction;
//	float					y,p;
//	direction.sub			(m_enemy_position,cover_point->position());
//	direction.getHP			(y,p);
//	y						= angle_normalize(y);
//	float					cover_value = ai().level_graph().cover_in_direction(y,cover_point->level_vertex_id());
	if (enemy_distance <= -m_best_value)
		return;

	m_selected				= cover_point;
	m_best_value			= -enemy_distance;
}

//////////////////////////////////////////////////////////////////////////
// CCoverEvaluatorBest
//////////////////////////////////////////////////////////////////////////

bool CCoverEvaluatorBest::threat_on_the_way			(Fvector const& cover_position) const
{
	Fvector const start_to_cover			= Fvector().sub(cover_position, m_start_position);
	float const start_to_cover_magnitude	= start_to_cover.magnitude();
	if (start_to_cover_magnitude < EPS_L)
		return								(false);

	Fvector const start_to_threat			= Fvector().sub(m_enemy_position, m_start_position);
	Fvector const start_to_cover_direction	= Fvector(start_to_cover).normalize();
	float const projection					= start_to_cover_direction.dotproduct(start_to_threat);
	float const start_to_threat_magnitude	= start_to_threat.magnitude();
	float const cos_alpha					= projection/start_to_threat_magnitude;
	float const angle						= acosf(cos_alpha);
	if (angle >= PI_DIV_6)
		return								(false);

	if (projection > start_to_cover_magnitude*1.5f)
		return								(false);

	return									(true);
}

void CCoverEvaluatorBest::evaluate_cover			(const CCoverPoint *cover_point, float weight)
{
	if (fis_zero(weight))
		return;

	float					enemy_distance	= m_enemy_position.distance_to(cover_point->position());

	if ((enemy_distance <= m_min_distance) && (m_current_distance > enemy_distance))
		return;

	if((enemy_distance >= m_max_distance) && (m_current_distance < enemy_distance))
		return;

	if (threat_on_the_way(cover_point->position()))
		return;

	Fvector					direction;
	float					y,p;
	direction.sub			(m_enemy_position,cover_point->position());
	direction.getHP			(y,p);
	y						= angle_normalize(y);

	float					cover_value = ai().level_graph().cover_in_direction(y,cover_point->level_vertex_id());
	float					value = cover_value;
	if (ai().level_graph().neighbour_in_direction(direction,cover_point->level_vertex_id()))
		value				+= 10.f;

	value					/= weight;

	if (value > m_best_value)
		return;

	if ((value == m_best_value) && (cover_point > m_selected))
		return;

	m_selected				= cover_point;
	m_best_value			= value;
//	Msg						("Loophole is NULL CCoverEvaluatorBest::evaluate_cover");
}

//////////////////////////////////////////////////////////////////////////
// CCoverEvaluatorAngle
//////////////////////////////////////////////////////////////////////////

void CCoverEvaluatorAngle::initialize				(const Fvector &start_position, bool fake_call)
{
	inherited::initialize		(start_position,fake_call);
	m_best_alpha				= -1.f;
	m_direction.sub				(m_start_position,m_enemy_position);
	m_direction.normalize_safe	();
	float						best_value = -1.f;
	float						m_best_angle = 0.f;
	for (float alpha = 0.f, step = PI_MUL_2/360.f; alpha < PI_MUL_2; alpha += step) {
		float					value = ai().level_graph().compute_square(alpha,PI_DIV_2,m_level_vertex_id);
		if (value > best_value) {
			best_value			= value;
			m_best_angle		= alpha;
		}
	}
	m_best_direction.setHP		(m_best_angle,0.f);
}

void CCoverEvaluatorAngle::evaluate_cover			(const CCoverPoint *cover_point, float weight)
{
	float					enemy_distance	= m_enemy_position.distance_to(cover_point->position());

	if ((enemy_distance <= m_min_distance) && (m_current_distance > enemy_distance))
		return;

	if((enemy_distance >= m_max_distance) && (m_current_distance < enemy_distance))
		return;

	Fvector					direction;
	direction.sub			(cover_point->position(),m_enemy_position);
	direction.normalize_safe();
	float					cos_a = direction.dotproduct(m_best_direction);
	if (cos_a < m_best_alpha)
		return;

	m_selected				= cover_point;
	m_best_alpha			= cos_a;
}

//////////////////////////////////////////////////////////////////////////
// CCoverEvaluatorSafe
//////////////////////////////////////////////////////////////////////////

void CCoverEvaluatorSafe::evaluate_cover			(const CCoverPoint *cover_point, float weight)
{
	if (m_start_position.distance_to(cover_point->position()) <= m_min_distance)
		return;

	float					cover_value = ai().level_graph().vertex_cover(cover_point->level_vertex_id());
	if (cover_value >= m_best_value)
		return;

	m_selected				= cover_point;
	m_best_value			= cover_value;
}

//////////////////////////////////////////////////////////////////////////
// CCoverEvaluatorAmbush
//////////////////////////////////////////////////////////////////////////

void CCoverEvaluatorAmbush::setup					(const Fvector &my_position, const Fvector &enemy_position, float min_enemy_distance)
{
	inherited::setup		();

//	m_actuality				= m_actuality && m_my_position.similar(my_position);
	m_my_position			= my_position;

	m_actuality				= m_actuality && m_enemy_position.similar(enemy_position,5.f);
	m_enemy_position		= enemy_position;

	m_actuality				= m_actuality && fsimilar(m_min_enemy_distance,min_enemy_distance);
	m_min_enemy_distance	= min_enemy_distance;
}

void CCoverEvaluatorAmbush::evaluate_cover			(const CCoverPoint *cover_point, float weight)
{
//	float					enemy_distance = m_enemy_position.distance_to(cover_point->position());
	float					my_distance = m_my_position.distance_to(cover_point->position());

	if (my_distance <= m_min_enemy_distance)
		return;

	Fvector					direction;
	float					y,p;

	direction.sub			(m_enemy_position,cover_point->position());
	direction.getHP			(y,p);
	y						= angle_normalize(y);
	float					cover_from_enemy = ai().level_graph().cover_in_direction(y,cover_point->level_vertex_id());

	direction.sub			(m_my_position,cover_point->position());
	direction.getHP			(y,p);
	y						= angle_normalize(y);
	float					cover_from_myself = ai().level_graph().cover_in_direction(y,cover_point->level_vertex_id());

	float					value = cover_from_enemy/cover_from_myself;
	if (value >= m_best_value)
		return;

	m_selected				= cover_point;
	m_best_value			= value;
}

