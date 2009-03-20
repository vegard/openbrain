extern "C" {
#include <math.h>
#include <time.h>
}

extern "C" {
#include <chipmunk.h>
}

#include "brain.hh"
#include "brain_configuration.hh"
#include "simulation.hh"

/* Returns in percent how much closer the creature was able to get to the
 * target. Negative numbers are better. More negative numbers are better. */
static unsigned int
test_brain(brain* b)
{
	simulation* sim = new simulation(b);

#if 0
	/* Distance at the beginning of the simulation */
	double d1 = cpvlength(
		cpvsub(sim->_creature_body->p, sim->_food_body->p));
#endif

	for (unsigned int i = 0; i < 2000; ++i)
		sim->step();

#if 0
	/* Distance at the end of the simulation */
	double d2 = cpvlength(
		cpvsub(sim->_creature_body->p, sim->_food_body->p));
#endif

	delete sim;

//	return (d2 - d1) / d1;
	return sim->_nr_hit_food;
}

static unsigned int
test_brain_configuration(const brain_configuration* bc, unsigned int n)
{
	unsigned int r = 0;

	for (unsigned int i = 0; i < n; ++i) {
		brain* b = new brain(bc);
		r += test_brain(b);
		delete b;
	}

	return r;
}

static brain_configuration*
evolve_linear(unsigned int n)
{
	unsigned int best_result = 0;
	brain_configuration* best = NULL;

	for (unsigned int i = 0; i < n; ++i) {
		brain_configuration* bc = new brain_configuration(32);
		bc->randomize();

		unsigned int r = test_brain_configuration(bc, 100);
		if (r > best_result) {
			if (best)
				delete best;
			best = bc;
			best_result = r;

			printf("evolve_linear: new best: %d\n", r);
		} else
			delete bc;
	}

	return best;
}

static brain_configuration*
evolve_binary_tree(unsigned int i)
{
	if (i == 0) {
		brain_configuration* bc = new brain_configuration(32);
		bc->randomize();
		return bc;
	}

	brain_configuration* bc_a = evolve_binary_tree(i - 1);
	brain_configuration* bc_b = evolve_binary_tree(i - 1);

	unsigned int r_a = test_brain_configuration(bc_a, 100);
	unsigned int r_b = test_brain_configuration(bc_b, 100);

	printf("%d vs. %d\n", r_a, r_b);

	if (r_a < r_b) {
		delete bc_a;
		return bc_b;
	} else {
		delete bc_b;
		return bc_a;
	}
}

static brain_configuration*
evolve_tree_mutations(brain_configuration* bc_parent)
{
	unsigned int best_result = 0;
	brain_configuration* best = NULL;

	for (unsigned int i = 0; i < 10; ++i) {
		brain_configuration* bc_child = new brain_configuration(32);
		*bc_child = *bc_parent;
		if (i != 0)
			bc_child->mutate();

		unsigned int r = test_brain_configuration(bc_child, 100);
		if (r > best_result) {
			if (best)
				delete best;
			best = bc_child;
			best_result = r;

			printf("evolve_tree_mutations: new best: %d\n", r);
		} else
			delete bc_child;
	}

	delete bc_parent;

	return best;
}

int
main(int argc, char* argv[])
{
	cpInitChipmunk();

	srand(time(NULL));

	brain_configuration* bc;

#if 0
	bc = evolve_linear(100);
#else
	bc = new brain_configuration(32);
	bc->restore("best-brain");
#endif
	for (unsigned int i = 0; i < 10; ++i)
		bc = evolve_tree_mutations(bc);

	bc->save("best-brain");
	delete bc;
	return 0;
}
