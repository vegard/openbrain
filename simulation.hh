#ifndef SIMULATION_HH
#define SIMULATION_HH

#include <set>

extern "C" {
#include <chipmunk.h>
}

#include "brain.hh"
#include "collision_type.hh"
#include "creature.hh"

class simulation {
public:
	typedef std::set<creature*> creature_set;

public:
	simulation();
	~simulation();

public:
	void step();

public:
	cpSpace* _space;

	creature_set _creatures;

	cpBody* _food_body;
	cpShape* _food_shape;
};

static int
creature_food_collision(cpShape* a, cpShape* b,
	cpContact* contacts, int n_contacts, cpFloat normal_coef, void* data)
{
	simulation* s = (simulation*) data;
	creature* c = (creature*) a->data;

	/* When the creature reaches the food, simply toggle this flag. It
	 * will be checked (and cleared) in step(), which also creates a
	 * new food object in a different location. */
	c->_creature_hit_food = true;
	return 1;
}

simulation::simulation()
{
	_space = cpSpaceNew();

	_food_body = cpBodyNew(1.0, 1.0);
	_food_body->p.x = 1000. * (2.0 * rand() / RAND_MAX - 1);
	_food_body->p.y = 1000. * (2.0 * rand() / RAND_MAX - 1);

	_food_shape = cpCircleShapeNew(_food_body, 50.0, cpvzero);
	_food_shape->collision_type = COLLISION_TYPE_FOOD;

	cpSpaceAddBody(_space, _food_body);
	cpSpaceAddShape(_space, _food_shape);

	cpSpaceAddCollisionPairFunc(_space,
		COLLISION_TYPE_CREATURE, COLLISION_TYPE_FOOD,
		&creature_food_collision, (void*) this);
}

simulation::~simulation()
{
	cpSpaceRemoveShape(_space, _food_shape);
	cpSpaceRemoveBody(_space, _food_body);

	cpSpaceFree(_space);
}

void
simulation::step()
{
	for (creature_set::iterator i = _creatures.begin(),
		end = _creatures.end(); i != end; ++i)
	{
		creature* c = *i;
		c->step();
	}

	cpSpaceStep(_space, 1);
}

#endif
