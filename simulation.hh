#ifndef SIMULATION_HH
#define SIMULATION_HH

#include "brain.hh"

class simulation {
public:
	enum {
		/* The relative position between the creature and the food */
		NEURONE_INPUT_RELATIVE_POS_LEFT,
		NEURONE_INPUT_RELATIVE_POS_RIGHT,

		NEURONE_INPUT_RELATIVE_POS_UP,
		NEURONE_INPUT_RELATIVE_POS_DOWN,

		/* The relative velocity between the creature and the food */
		NEURONE_INPUT_RELATIVE_VEL_LEFT,
		NEURONE_INPUT_RELATIVE_VEL_RIGHT,

		NEURONE_INPUT_RELATIVE_VEL_UP,
		NEURONE_INPUT_RELATIVE_VEL_DOWN,

		/* Muscles */
		NEURONE_OUTPUT_MOVE_LEFT,
		NEURONE_OUTPUT_MOVE_RIGHT,
		NEURONE_OUTPUT_MOVE_UP,
		NEURONE_OUTPUT_MOVE_DOWN,
	};

	enum {
		COLLISION_TYPE_CREATURE,
		COLLISION_TYPE_FOOD,
	};

public:
	simulation(brain*);
	~simulation();

public:
	void step();

public:
	brain* _brain;

	cpSpace* _space;

	cpBody* _creature_body;
	cpShape* _creature_shape;

	cpBody* _food_body;
	cpShape* _food_shape;

	bool _creature_hit_food;
	unsigned int _nr_hit_food;
};

static int
creature_food_collision(cpShape* a, cpShape* b,
	cpContact* contacts, int n_contacts, cpFloat normal_coef, void* data)
{
	simulation* s = (simulation*) data;

	/* When the creature reaches the food, simply toggle this flag. It
	 * will be checked (and cleared) in update(), which also creates a
	 * new food object in a different location. */
	s->_creature_hit_food = true;
	return 1;
}

simulation::simulation(brain* b):
	_brain(b)
{
	_space = cpSpaceNew();

	_creature_body = cpBodyNew(1.0, 1.0);
	_creature_body->p.x = 1000. * (2.0 * rand() / RAND_MAX - 1);
	_creature_body->p.y = 1000. * (2.0 * rand() / RAND_MAX - 1);

	_creature_shape = cpCircleShapeNew(_creature_body, 50.0, cpvzero);
	_creature_shape->collision_type = COLLISION_TYPE_CREATURE;

	cpSpaceAddBody(_space, _creature_body);
	cpSpaceAddShape(_space, _creature_shape);

	_food_body = cpBodyNew(1.0, 1.0);
	_food_body->p.x = 1000. * (2.0 * rand() / RAND_MAX - 1);
	_food_body->p.y = 1000. * (2.0 * rand() / RAND_MAX - 1);

	_food_shape = cpCircleShapeNew(_food_body, 50.0, cpvzero);
	_food_shape->collision_type = COLLISION_TYPE_FOOD;

	cpSpaceAddBody(_space, _food_body);
	cpSpaceAddShape(_space, _food_shape);

	_creature_hit_food = false;
	_nr_hit_food = 0;

	cpSpaceAddCollisionPairFunc(_space,
		COLLISION_TYPE_CREATURE, COLLISION_TYPE_FOOD,
		&creature_food_collision, (void*) this);
}

simulation::~simulation()
{
	cpSpaceRemoveBody(_space, _creature_body);
	cpSpaceRemoveBody(_space, _food_body);
	cpSpaceFree(_space);
}

static inline double clamp(double x)
{
	if (x < 0)
		return 0;
	if (x > 1)
		return 1;
	return x;
}

void
simulation::step()
{
	if (_creature_hit_food) {
		++_nr_hit_food;
		_creature_hit_food = false;

		_food_body->p.x = 1000. * (2.0 * rand() / RAND_MAX - 1);
		_food_body->p.y = 1000. * (2.0 * rand() / RAND_MAX - 1);
		_food_body->v = cpvzero;
	}

#if 0
	printf("creature: %.3f %.3f\n",
		_creature_body->p.x, _creature_body->p.y);
	printf("target:   %.3f %.3f\n",
		_food_body->p.x, _food_body->p.y);
#endif


	/* Update inputs */
	_brain->_neurones[wr][NEURONE_INPUT_RELATIVE_POS_LEFT]
		= clamp((_creature_body->p.x - _food_body->p.x) / 1000);
	_brain->_neurones[wr][NEURONE_INPUT_RELATIVE_POS_RIGHT]
		= clamp((_food_body->p.x - _creature_body->p.x) / 1000);

	_brain->_neurones[wr][NEURONE_INPUT_RELATIVE_POS_UP]
		= clamp((_creature_body->p.y - _food_body->p.y) / 1000);
	_brain->_neurones[wr][NEURONE_INPUT_RELATIVE_POS_DOWN]
		= clamp((_food_body->p.y - _creature_body->p.y) / 1000);

	_brain->_neurones[wr][NEURONE_INPUT_RELATIVE_VEL_LEFT]
		= clamp((_creature_body->v.x - _food_body->v.x) / 100);
	_brain->_neurones[wr][NEURONE_INPUT_RELATIVE_VEL_RIGHT]
		= clamp((_food_body->v.x - _creature_body->v.x) / 100);

	_brain->_neurones[wr][NEURONE_INPUT_RELATIVE_VEL_UP]
		= clamp((_creature_body->v.y - _food_body->v.y) / 100);
	_brain->_neurones[wr][NEURONE_INPUT_RELATIVE_VEL_DOWN]
		= clamp((_food_body->v.y - _creature_body->v.y) / 100);

#if 0
	printf("input: %.2f %.2f\n",
		_brain->_neurones[wr][NEURONE_INPUT_RELATIVE_POS_LEFT],
		_brain->_neurones[wr][NEURONE_INPUT_RELATIVE_VEL_LEFT]);
#endif

	_brain->step();
	cpSpaceStep(_space, 1);

	/* Read (and apply) outputs */
	double jx = _brain->_neurones[rd][NEURONE_OUTPUT_MOVE_RIGHT]
		- _brain->_neurones[rd][NEURONE_OUTPUT_MOVE_LEFT];
	double jy = _brain->_neurones[rd][NEURONE_OUTPUT_MOVE_UP]
		- _brain->_neurones[rd][NEURONE_OUTPUT_MOVE_DOWN];

	cpBodyApplyImpulse(_creature_body, cpv(jx, jy), cpvzero);
}

#endif
