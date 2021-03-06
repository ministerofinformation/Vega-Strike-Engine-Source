/**
 * collision.cpp
 *
 * Copyright (C) 2020 Roy Falk, Stephen G. Tuggy and other Vega Strike
 * contributors
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */


#include "collision.h"

#include "unit_generic.h"
#include "universe.h"
#include "universe_util.h"
#include "configuration/configuration.h"
#include "missile.h"
#include "enhancement.h"
#include <typeinfo>
#include <boost/log/trivial.hpp>

// TODO: convert all float to double and all Vector to QVector.

// TODO: all of these comments were moved around from the old code.
// They are unclear to me. Someone (probably JS) needs to do something with it.

//Collision force caps primarily for AI-AI collisions. Once the AIs get a real collision avoidance system, we can
// turn damage for AI-AI collisions back on, and then we can remove these caps.
//value, in seconds of desired maximum recovery time
//value, in seconds of desired maximum recovery time





Collision::Collision(Unit* unit, const QVector& location, const Vector& normal):
    unit(unit), location(location), normal(normal)
{
    cockpit = _Universe->isPlayerStarship( unit ); // smcp/thcp
    unit_type = unit->isUnit();
    is_player_ship = _Universe->isPlayerStarship(unit);
    mass = std::max(unit->getMass(), configuration.physics.minimum_mass);
    position = unit->Position();
    velocity = unit->GetVelocity();
}




// This function handles the initial reaction of the unit to hitting the other unit
// Note: I'm changing the expected behavior here
// Return value indicates whether to continue processing
/*_UnitType::unit,
_UnitType::planet,
_UnitType::building,
_UnitType::nebula,
_UnitType::asteroid,
_UnitType::enhancement,
_UnitType::missile*/
void Collision::shouldApplyForceAndDealDamage(Unit* other_unit)
{
    // Collision with a nebula does nothing
    if(other_unit->isUnit() == _UnitType::nebula)
    {
        return;
    }

    // Collision with a enhancement improves your shield apparently
    if(other_unit->isUnit() == _UnitType::enhancement)
    {
        apply_force = true;
        return;
    }

    // Collision with a jump point does nothing
    if(other_unit->isJumppoint())
    {
        other_unit->jumpReactToCollision(unit);
        return;
    }

    if(unit->isJumppoint())
    {
        return;
    }

    switch(unit_type)
    {
    // Missiles and asteroids always explode on impact with anything except Nebula and Enhancement.
    case _UnitType::missile:
        // Missile should explode when killed
        // If not, uncomment this
        //((Missile*)unit)->Discharge();
        unit->Kill();
        return;

    case _UnitType::asteroid:
        apply_force = true;
        deal_damage = true;
        return;

    // Planets and Nebulas can't be killed right now
    case _UnitType::planet:
    case _UnitType::nebula:
        return;

    // Buildings should not calculate actual damage
    case _UnitType::building:
        return;

    // Units (ships) should calculate actual damage
    case _UnitType::unit:
        apply_force = true;
        deal_damage = true;
        return;

    // Not sure what an enhancement is, but it looks like it's something that can increase the shields of the unit it collided with.
    // TODO: refactor this.
    case _UnitType::enhancement:
        if (other_unit->isUnit() == _UnitType::asteroid)
        {
            apply_force = true;
            return;
        }

        double percent;
        char tempdata[sizeof(Shield)];
        memcpy( tempdata, &unit->shield, sizeof(Shield));
        unit->shield.number = 0;     //don't want them getting our boosted shields!
        unit->shield.shield2fb.front = unit->shield.shield2fb.back =
                unit->shield.shield2fb.frontmax = unit->shield.shield2fb.backmax = 0;
        other_unit->Upgrade( unit, 0, 0, true, true, percent );
        memcpy( &unit->shield, tempdata, sizeof (Shield) );
        string fn( unit->filename );
        string fac( FactionUtil::GetFaction( unit->faction ) );
        unit->Kill();

        _Universe->AccessCockpit()->savegame->AddUnitToSave( fn.c_str(), _UnitType::enhancement, fac.c_str(), reinterpret_cast<long>(unit));
        apply_force = true;
        return;
    }
}

// This stops the unit from going through the other unit
/*void Collision::applyForce(double elasticity, float& m2, Vector& v2)
{
    //for torque... location -- approximation hack of MR^2 for rotational inertia (moment of inertia currently just M)
    Vector torque = force/(unit->radial_size*unit->radial_size);
    Vector force  = this->force-torque; // Note the variable shadowing

    float  max_force  = max_force_multiplier*(unit->limits.forward + unit->limits.retro + unit->limits.lateral + unit->limits.vertical);
    float  max_torque = max_torque_multiplier*(unit->limits.yaw + unit->limits.pitch+unit->limits.roll);

    //Convert from frames to seconds, so that the specified value is meaningful
    max_force  = max_force / (unit->sim_atom_multiplier * simulation_atom_var);
    max_torque = max_torque / (unit->sim_atom_multiplier * simulation_atom_var);
    float torque_magnitude = torque.Magnitude();
    float force_magnitude = force.Magnitude();
    if (torque_magnitude > max_torque) {
        torque *= (max_torque/torque_magnitude);
    }
    if (force_magnitude > max_force) {
        force *= (max_force/force_magnitude);
    }
    unit->ApplyTorque( torque, location );
    unit->ApplyForce( force-torque );
}*/




void Collision::dealDamage(Collision other_collision, float factor)
{
    if(!deal_damage)
    {
        return;
    }

    // Shorten variables for better visibility
    float& m1 = mass;
    float& m2 = other_collision.mass;
    Vector& v1 = velocity;
    Vector& v2 = other_collision.velocity;

    float relative_velocity = (v1-v2).Magnitude();
    float damage = relative_velocity * relative_velocity *
            (m2)/(m1+m2) * factor;

    unit->ApplyDamage(other_collision.location.Cast(),
                      other_collision.normal,
                      damage, unit, GFXColor( 1,1,1,2 ), other_collision.unit->owner
                      != nullptr ? other_collision.unit->owner : this );
}

void Collision::applyForce(double elasticity, float& m2, Vector& v2)
{
    /*
    The current implementation is of an imperfectly elastic collision.
    An elastic collision is an encounter between two bodies in which the total kinetic energy of the two bodies remains the same. In an ideal, perfectly elastic collision, there is no net conversion of kinetic energy into other forms such as heat, noise, or potential energy.
    https://en.wikipedia.org/wiki/Elastic_collision

    This function adds an elasticity ratio to reduce velocity, due to heat, damage, etc.
    */

    // elasticity must be between 0 and 1
    if(elasticity >1 || elasticity <0)
    {
        BOOST_LOG_TRIVIAL(warning) << "Collision::applyForce Expected an elasticity value between 0 and 1 but got " << elasticity;
        elasticity = std::max(elasticity, 0.0);
        elasticity = std::min(elasticity, 1.0);
    }

    if(!apply_force)
    {
        return;
    }

    // Shorten variables for better visibility
    float& m1 = mass;
    Vector& v1 = velocity;

    // Perfectly elastic result
    Vector new_velocity = v1*(m1-m2)/(m1+m2) + v2*2*m2/(m1+m2);

    // Apply ratio
    new_velocity = new_velocity * elasticity;

    // Apply force
    unit->ApplyForce(new_velocity);
}


// Discussion - the original code ran this once for both units. This required a comprison of the units to find out which is smaller.
void Collision::collide( Unit* unit1,
                                   const QVector &location1,
                                   const Vector &normal1,
                                   Unit* unit2,
                                   const QVector &location2,
                                   const Vector &normal2,
                                   float distance)
{
    Collision collision1 = Collision(unit1, location1, normal1);
    Collision collision2 = Collision(unit2, location2, normal2);

    // TODO: Try "crash" landing both ways
    // See commit a66bdcfa1e00bf039183603913567d48e52c7a8e method crashLand for an example
    // If configured, the game should behave like privateer and land if close enough to a dock

    // TODO: Try "auto jump" both ways
    // See commit a66bdcfa1e00bf039183603913567d48e52c7a8e method Unit->jumpReactToCollision for an example
    // I assume this is for "always open" jump gates that you pass through

    collision1.shouldApplyForceAndDealDamage(unit2);
    collision2.shouldApplyForceAndDealDamage(unit1);

    float elasticity = 0.8f;
    collision1.applyForce(elasticity, collision2.mass, collision2.velocity);
    collision2.applyForce(elasticity, collision1.mass, collision1.velocity);

    collision1.dealDamage(collision2);
    collision2.dealDamage(collision1);

    // TODO: add torque
}
