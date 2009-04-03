#ifndef BRAIN_HH
#define BRAIN_HH

extern "C" {
#include <stdio.h>
#include <stdlib.h>
}

#include "brain_configuration.hh"

static unsigned int rd = 0;
static unsigned int wr = 1;

class brain {
public:
	brain(const brain_configuration* bc);
	~brain();

public:
	void dump();

	void step();

public:
	unsigned int _nr_neurones;
	double* _neurones[2];
	double* _synapses;
};

brain::brain(const brain_configuration* bc)
{
	_nr_neurones = bc->_nr_neurones;
	_neurones[0] = new double[_nr_neurones];
	_neurones[1] = new double[_nr_neurones];

	for (unsigned int i = 0; i < _nr_neurones; ++i)
		_neurones[wr][i] = 0;

	_synapses = new double[_nr_neurones * _nr_neurones];

	for (unsigned int i = 0; i < _nr_neurones; ++i) {
		for (unsigned int j = 0; j < _nr_neurones; ++j) {
			_synapses[i * _nr_neurones + j]
				= bc->_synapses[i * _nr_neurones + j];
		}
	}
}

brain::~brain()
{
	delete[] _neurones[0];
	delete[] _neurones[1];
	delete[] _synapses;
}

void
brain::step()
{
#if 0
	for (unsigned int i = 0; i < _nr_neurones; ++i)
		printf("%.2f ", _neurones[rd][i]);

	printf("\n");
#endif

	for (unsigned int i = 0; i < _nr_neurones; ++i) {
		double sum = 0;

		for (unsigned int j = 0; j < _nr_neurones; ++j) {
			sum += _synapses[i * _nr_neurones + j]
				* _neurones[rd][j];
		}

		/* A neurone itself can't send inhibitory signals, it's the
		 * synapse that determines the "sign" of the signal. */
		if (sum < 0)
			sum = 0;

		if (sum > 1)
			sum = 1;

		_neurones[wr][i] = sum;
	}

#if 0
	/* Learning factor */
	static const double lf = 1e-8;

	/* Strengthen (or weaken) synapses: The more they are used, the
	 * stronger it gets (can't go above 1.0 or below -1.0, though). */
	for (unsigned int i = 0; i < _nr_neurones; ++i) {
		for (unsigned int j = 0; j < _nr_neurones; ++j) {
			double v = _synapses[i * _nr_neurones + j];
			double u;

			if (_neurones[rd][i] < 0)
				u = -lf;
			else
				u = lf;

			if (v < 0)
				_synapses[i * _nr_neurones + j] = v + u;
			else
				_synapses[i * _nr_neurones + j] = v - u;
		}
	}
#endif
}

#endif
