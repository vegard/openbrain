extern "C" {
#include <math.h>
#include <time.h>
}

extern "C" {
#include <chipmunk.h>
}

#include "brain.hh"
#include "brain_configuration.hh"

/* Returns in percent how much closer the creature was able to get to the
 * target. Negative numbers are better. More negative numbers are better. */
static double
test_brain(brain* my_brain)
{
	enum {
		/* The relative position between the creature and the food */
		NEURONE_INPUT_RELATIVE_POS_X,
		NEURONE_INPUT_RELATIVE_POS_Y,

		/* The relative velocity between the creature and the food */
		NEURONE_INPUT_RELATIVE_VEL_X,
		NEURONE_INPUT_RELATIVE_VEL_Y,

		/* Muscles */
		NEURONE_OUTPUT_MOVE_LEFT,
		NEURONE_OUTPUT_MOVE_RIGHT,
		NEURONE_OUTPUT_MOVE_UP,
		NEURONE_OUTPUT_MOVE_DOWN,
	};

	cpSpace* space;
	cpBody* creature_body;
	cpBody* food_body;

	space = cpSpaceNew();

	creature_body = cpBodyNew(1.0, 1.0);
	creature_body->p.x = 1000. * (2.0 * rand() / RAND_MAX - 1);
	creature_body->p.y = 1000. * (2.0 * rand() / RAND_MAX - 1);
	cpSpaceAddBody(space, creature_body);

	food_body = cpBodyNew(1.0, 1.0);
	food_body->p.x = 1000. * (2.0 * rand() / RAND_MAX - 1);
	food_body->p.y = 1000. * (2.0 * rand() / RAND_MAX - 1);
	cpSpaceAddBody(space, food_body);

	/* Distance at the beginning of the simulation */
	double d1 = cpvlength(cpvsub(creature_body->p, food_body->p));
	//printf("d1: %f\n", d1);

	for (unsigned int i = 0; i < 500; ++i) {
#if 0
		printf("creature: %.3f %.3f\n",
			creature_body->p.x, creature_body->p.y);
		printf("target:   %.3f %.3f\n",
			food_body->p.x, food_body->p.y);
#endif

		/* Update inputs */
		my_brain->_neurones[wr][NEURONE_INPUT_RELATIVE_POS_X]
			= creature_body->p.x - food_body->p.x;
		my_brain->_neurones[wr][NEURONE_INPUT_RELATIVE_POS_Y]
			= creature_body->p.y - food_body->p.y;

		my_brain->_neurones[wr][NEURONE_INPUT_RELATIVE_VEL_X]
			= creature_body->v.x - food_body->v.x;
		my_brain->_neurones[wr][NEURONE_INPUT_RELATIVE_VEL_Y]
			= creature_body->v.y - food_body->v.y;

		my_brain->step();
		cpSpaceStep(space, 1);

		/* Read (and apply) outputs */
		double jx = my_brain->_neurones[rd][NEURONE_OUTPUT_MOVE_RIGHT]
			- my_brain->_neurones[rd][NEURONE_OUTPUT_MOVE_LEFT];
		double jy = my_brain->_neurones[rd][NEURONE_OUTPUT_MOVE_UP]
			- my_brain->_neurones[rd][NEURONE_OUTPUT_MOVE_DOWN];

		cpBodyApplyImpulse(creature_body, cpv(jx, jy), cpvzero);
	}

	/* Distance at the end of the simulation */
	double d2 = cpvlength(cpvsub(creature_body->p, food_body->p));
	//printf("d2: %f\n", d2);

	cpSpaceRemoveBody(space, creature_body);
	cpSpaceRemoveBody(space, food_body);
	cpSpaceFree(space);

	return (d2 - d1) / d1;
}

static double
test_brain_configuration(const brain_configuration* bc, unsigned int n)
{
	double r = 0;

	for (unsigned int i = 0; i < n; ++i) {
		brain* b = new brain(bc);
		r += test_brain(b);
		delete b;
	}

	return r;
}

int
main(int argc, char* argv[])
{
	cpInitChipmunk();

	srand(time(NULL));

	/* Create one initial brain. Run ~50 random experiments, and use the
	 * average to decide whether the brain is good or not. The one(s) with
	 * the best results are mutated and retried. */

	double best = 0;

	while (1) {
		brain_configuration* bc = new brain_configuration(16);
		bc->randomize();

		double r = test_brain_configuration(bc, 100);
		if (r < best) {
			best = r;

			printf("new best: %f\n", 100 * r);
			//bc->dump();
			bc->save("best-brain-so-far");
		}

		delete bc;
	}

	return 0;
}
