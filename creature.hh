#ifndef CREATURE_HH
#define CREATURE_HH

extern "C" {
#include <chipmunk.h>
}

#include "collision_type.hh"
#include "creature_listener.hh"

class creature {
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

		/* Current energy level */
		NEURONE_INPUT_ENERGY = 32,
	};

public:
	creature(brain_configuration* bc, brain* b, cpBody* food_body, double red, double green, double blue, creature_listener* died_listener, creature_listener* ate_listener);
	~creature();

public:
	void add_to_space(cpSpace*);
	void remove_from_space(cpSpace*);

	void step();

public:
	brain_configuration* _brain_configuration;
	brain* _brain;

	cpBody* _food_body;

	double _red;
	double _green;
	double _blue;

	creature_listener* _died_listener;
	creature_listener* _ate_listener;

	cpBody* _creature_body;
	cpShape* _creature_shape;

	bool _creature_hit_food;
	unsigned int _nr_hit_food;

	double _energy;
	unsigned long _lifetime;
};

creature::creature(brain_configuration* bc, brain* b, cpBody* food_body, double red, double green, double blue, creature_listener* died_listener, creature_listener* ate_listener):
	_brain_configuration(bc),
	_brain(b),
	_food_body(food_body),
	_red(red),
	_green(green),
	_blue(blue),
	_died_listener(died_listener),
	_ate_listener(ate_listener),
	_energy(2000),
	_lifetime(0)
{
	_creature_body = cpBodyNew(1.0, 1.0);
	_creature_body->p.x = 1000. * (2.0 * rand() / RAND_MAX - 1);
	_creature_body->p.y = 1000. * (2.0 * rand() / RAND_MAX - 1);

	_creature_shape = cpCircleShapeNew(_creature_body, 60.0, cpvzero);
	_creature_shape->e = 0;
	_creature_shape->u = 0.5;
	_creature_shape->collision_type = COLLISION_TYPE_CREATURE;
	_creature_shape->data = (void*) this;

	_creature_hit_food = false;
	_nr_hit_food = 0;
}

creature::~creature()
{
}

void
creature::add_to_space(cpSpace* s)
{
	cpSpaceAddBody(s, _creature_body);
	cpSpaceAddShape(s, _creature_shape);
}

void
creature::remove_from_space(cpSpace* s)
{
	cpSpaceRemoveShape(s, _creature_shape);
	cpSpaceRemoveBody(s, _creature_body);
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
creature::step()
{
	/* Um, yes, it's better to use timestamps... */
	++_lifetime;


	/* Update inputs */
	_brain->_neurones[rd][NEURONE_INPUT_RELATIVE_POS_LEFT]
		= clamp((_creature_body->p.x - _food_body->p.x) / 1000);
	_brain->_neurones[rd][NEURONE_INPUT_RELATIVE_POS_RIGHT]
		= clamp((_food_body->p.x - _creature_body->p.x) / 1000);

	_brain->_neurones[rd][NEURONE_INPUT_RELATIVE_POS_UP]
		= clamp((_creature_body->p.y - _food_body->p.y) / 1000);
	_brain->_neurones[rd][NEURONE_INPUT_RELATIVE_POS_DOWN]
		= clamp((_food_body->p.y - _creature_body->p.y) / 1000);

	_brain->_neurones[rd][NEURONE_INPUT_RELATIVE_VEL_LEFT]
		= clamp((_creature_body->v.x - _food_body->v.x) / 100);
	_brain->_neurones[rd][NEURONE_INPUT_RELATIVE_VEL_RIGHT]
		= clamp((_food_body->v.x - _creature_body->v.x) / 100);

	_brain->_neurones[rd][NEURONE_INPUT_RELATIVE_VEL_UP]
		= clamp((_creature_body->v.y - _food_body->v.y) / 100);
	_brain->_neurones[rd][NEURONE_INPUT_RELATIVE_VEL_DOWN]
		= clamp((_food_body->v.y - _creature_body->v.y) / 100);

	_brain->_neurones[rd][NEURONE_INPUT_ENERGY]
		= clamp(_energy / 10000);

	_brain->step();

	/* Read (and apply) outputs */
	double jx = _brain->_neurones[rd][NEURONE_OUTPUT_MOVE_RIGHT]
		- _brain->_neurones[rd][NEURONE_OUTPUT_MOVE_LEFT];
	double jy = _brain->_neurones[rd][NEURONE_OUTPUT_MOVE_UP]
		- _brain->_neurones[rd][NEURONE_OUTPUT_MOVE_DOWN];

	cpBodyApplyImpulse(_creature_body, cpv(jx, jy), cpvzero);

	_energy -= 0.3 + cpvlength(cpv(jx, jy));
	if (_energy < 0)
		_died_listener->handle(this);
}

#endif
