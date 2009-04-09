#ifndef BRAIN_CONFIGURATION_HH
#define BRAIN_CONFIGURATION_HH

extern "C" {
#include <stdio.h>
#include <stdlib.h>
}

class brain_configuration {
public:
	explicit brain_configuration(unsigned int nr_neurones);
	~brain_configuration();

public:
	brain_configuration(const brain_configuration&);
	brain_configuration& operator=(const brain_configuration&);

public:
	void dump();
	void save(const char* filename);
	void restore(const char* filename);

	void randomize();
	void mutate();
	void mutate2();

public:
	unsigned int _nr_neurones;
	double* _synapses;
};

brain_configuration::brain_configuration(unsigned int nr_neurones):
	_nr_neurones(nr_neurones)
{
	_synapses = new double[_nr_neurones * _nr_neurones];
}

brain_configuration::~brain_configuration()
{
	delete[] _synapses;
}

brain_configuration::brain_configuration(const brain_configuration& bc)
{
	for (unsigned int i = 0; i < _nr_neurones; ++i) {
		for (unsigned int j = 0; j < _nr_neurones; ++j) {
			_synapses[i + j * _nr_neurones]
				= bc._synapses[i + j * bc._nr_neurones];
		}
	}
}

brain_configuration&
brain_configuration::operator=(const brain_configuration& bc)
{
	for (unsigned int i = 0; i < _nr_neurones; ++i) {
		for (unsigned int j = 0; j < _nr_neurones; ++j) {
			_synapses[i + j * _nr_neurones]
				= bc._synapses[i + j * bc._nr_neurones];
		}
	}

	return *this;
}

void
brain_configuration::dump()
{
	for (unsigned int i = 0; i < _nr_neurones; ++i) {
		for (unsigned int j = 0; j < _nr_neurones; ++j) {
			printf("%s%.1f ",
				_synapses[i * _nr_neurones + j] < 0 ? "" : " ",
				_synapses[i * _nr_neurones + j]);
		}

		printf("\n");
	}

	printf("\n");
}

void
brain_configuration::save(const char* filename)
{
	FILE* fp = fopen(filename, "w");
	fwrite(_synapses, sizeof(*_synapses), _nr_neurones * _nr_neurones, fp);
	fclose(fp);
}

void
brain_configuration::restore(const char* filename)
{
	FILE* fp = fopen(filename, "r");
	fread(_synapses, sizeof(*_synapses), _nr_neurones * _nr_neurones, fp);
	fclose(fp);
}

void
brain_configuration::randomize()
{
	for (unsigned int i = 0; i < _nr_neurones; ++i) {
		for (unsigned int j = 0; j < _nr_neurones; ++j) {
			_synapses[i * _nr_neurones + j]
				= 2.0 * rand() / RAND_MAX - 1.0;
		}
	}
}

void
brain_configuration::mutate()
{
	/* Number of synapses to change; no more than half of them! */
	unsigned int n = rand() % (_nr_neurones / 2);

	for (unsigned int i = 0; i < n; ++i) {
		unsigned int x = rand() % _nr_neurones;
		unsigned int y = rand() % _nr_neurones;

		/* Just assign a new random value */
		_synapses[x * _nr_neurones + y]
			= 2.0 * rand() / RAND_MAX - 1.0;
	}
}

void
brain_configuration::mutate2()
{
	for (unsigned int i = 0; i < _nr_neurones; ++i) {
		for (unsigned int j = 0; j < _nr_neurones; ++j) {
			double v = _synapses[i * _nr_neurones + j];

			v += 1e-2 * (2.0 * rand() / RAND_MAX - 1.0);

			if (v < -1)
				v = -1;
			else if (v > 1)
				v = 1;

			_synapses[i * _nr_neurones + j] = v;
		}
	}
}

#endif
