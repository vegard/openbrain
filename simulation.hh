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

	cpBody* _walls_body;
	cpShape* _walls_shape[4];

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

	c->_ate_listener->handle(c);
	return 1;
}

simulation::simulation()
{
	_space = cpSpaceNew();
	_space->elasticIterations = 10;
	_space->gravity.y = -2e-1;

	{
		_walls_body = cpBodyNew(INFINITY, INFINITY);

		_walls_shape[0] = cpSegmentShapeNew(_walls_body,
			cpv(-3000, -3000), cpv(-3000, 3000), 100);
		_walls_shape[0]->e = 0.8;
		_walls_shape[0]->u = 2.0;
		cpSpaceAddStaticShape(_space, _walls_shape[0]);

		_walls_shape[1] = cpSegmentShapeNew(_walls_body,
			cpv(3000, -3000), cpv(3000, 3000), 100);
		_walls_shape[1]->e = 0.8;
		_walls_shape[1]->u = 2.0;
		cpSpaceAddStaticShape(_space, _walls_shape[1]);

		_walls_shape[2] = cpSegmentShapeNew(_walls_body,
			cpv(-3000, -3000), cpv(3000, -3000), 100);
		_walls_shape[2]->e = 0.8;
		_walls_shape[2]->u = 2.0;
		cpSpaceAddStaticShape(_space, _walls_shape[2]);

		_walls_shape[3] = cpSegmentShapeNew(_walls_body,
			cpv(-3000, 3000), cpv(3000, 3000), 100);
		_walls_shape[3]->e = 0.8;
		_walls_shape[3]->u = 2.0;
		cpSpaceAddStaticShape(_space, _walls_shape[3]);
	}

	_food_body = cpBodyNew(1.0, 1.0);
	_food_body->p.x = 1000. * (2.0 * rand() / RAND_MAX - 1);
	_food_body->p.y = 1000. * (2.0 * rand() / RAND_MAX - 1);
	_food_body->v.x = (2.0 * rand() / RAND_MAX - 1);
	_food_body->v.y = (2.0 * rand() / RAND_MAX - 1);
	_food_body->v = cpvmult(cpvnormalize(_food_body->v), 40);

	_food_shape = cpCircleShapeNew(_food_body, 50.0, cpvzero);
	_food_shape->e = 0.8;
	_food_shape->u = 1.0;
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
