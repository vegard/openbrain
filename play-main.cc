#include <vector>

extern "C" {
#include <math.h>
#include <time.h>
}

extern "C" {
#include <SDL.h>
#include <SDL_opengl.h>
#include <chipmunk.h>
}

//#define CONFIG_CAPTURE
#undef CONFIG_CAPTURE
#define WIDTH 640
#define HEIGHT 480

#include "brain.hh"
#include "brain_configuration.hh"
#include "capture.hh"
#include "creature_listener.hh"
#include "simulation.hh"

class died_creature_listener: public creature_listener {
public:
	void handle(creature* c);
};

class ate_creature_listener: public creature_listener {
public:
	void handle(creature* c);
};

static simulation* sim;
static died_creature_listener _died_creature_listener;
static ate_creature_listener _ate_creature_listener;

typedef std::vector<creature*> creature_vector;
static creature_vector dead_creatures;
static creature_vector full_creatures;

static brain_configuration* best_bc = NULL;

void
died_creature_listener::handle(creature* c)
{
	dead_creatures.push_back(c);
}

void
ate_creature_listener::handle(creature* c)
{
	full_creatures.push_back(c);
}

static Uint32
displayTimer(Uint32 interval, void *unused)
{
	SDL_Event ev;

	ev.type = SDL_USEREVENT;
	SDL_PushEvent(&ev);

	return interval;
}

static double aspect;

static void
resize(int width, int height)
{
	if(width == 0 || height == 0)
		return;

	aspect = 1.0 * height / width;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glViewport(0, 0, width, height);
}

static void
draw_square(void)
{
	glBegin(GL_QUADS);
	glVertex2f(-1, -1);
	glVertex2f( 1, -1);
	glVertex2f( 1,  1);
	glVertex2f(-1,  1);
	glEnd();
}

static void
draw_box(void)
{
	glBegin(GL_LINES);
	glVertex2f(-1, -1);
	glVertex2f( 1, -1);

	glVertex2f( 1, -1);
	glVertex2f( 1,  1);

	glVertex2f( 1,  1);
	glVertex2f(-1,  1);

	glVertex2f(-1,  1);
	glVertex2f(-1, -1);
	glEnd();
}

static void
display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluOrtho2D(-3000 / aspect, 3000 / aspect, -3000, 3000);

#if 0
	glPushMatrix();
	glScalef(50, 50, 0);
	glTranslatef(-1. * n / 2, -1. * n / 2, 0);

	glBegin(GL_QUADS);
	for (unsigned int i = 0; i < n; ++i) {
		for (unsigned int j = 0; j < n; ++j) {
			glColor4f(0, 0, 0.5 + 0.5 * synapses[i + j * n], 1);

			glVertex2f(i + 0, 1 + j + 0);
			glVertex2f(i + 1, 1 + j + 0);
			glVertex2f(i + 1, 1 + j + 1);
			glVertex2f(i + 0, 1 + j + 1);
		}
	}

	for (unsigned int i = 0; i < n; ++i) {
		glColor4f(1, 0, neurones[i], 1);

		glVertex2f(i + 0, 0);
		glVertex2f(i + 1, 0);
		glVertex2f(i + 1, 1);
		glVertex2f(i + 0, 1);
	}
	glEnd();

	glPopMatrix();
#endif

	glPushMatrix();
	glTranslatef(sim->_food_body->p.x, sim->_food_body->p.y, 0);
	glRotatef(cpvtoangle(sim->_food_body->rot) * 180 / M_PI, 0, 0, 1);
	glScalef(50, 50, 0);

	glColor3f(0, 1, 0);
	draw_square();

	glColor3f(0, 0, 0);
	draw_box();

	glPopMatrix();

	for (simulation::creature_set::iterator i = sim->_creatures.begin(),
		end = sim->_creatures.end(); i != end; ++i)
	{
		creature* c = *i;

		glPushMatrix();
		glTranslatef(c->_creature_body->p.x, c->_creature_body->p.y, 0);
		glRotatef(cpvtoangle(c->_creature_body->rot) * 180 / M_PI, 0, 0, 1);
		glScalef(50, 50, 0);
		glScalef(5e-4 * c->_energy, 5e-4 * c->_energy, 0);
		glColor4f(0, 0, 0, 1. / 3);
		draw_square();

		glPopMatrix();
	}

	for (simulation::creature_set::iterator i = sim->_creatures.begin(),
		end = sim->_creatures.end(); i != end; ++i)
	{
		creature* c = *i;

		glPushMatrix();
		glTranslatef(c->_creature_body->p.x, c->_creature_body->p.y, 0);
		glRotatef(cpvtoangle(c->_creature_body->rot) * 180 / M_PI, 0, 0, 1);
		glScalef(50, 50, 0);

		glColor4f(c->_red, c->_green, c->_blue, 1e-3 * c->_energy);
		draw_square();

		glColor4f(0, 0, 0, 1e-3 * c->_energy);
		draw_box();

		glPopMatrix();
	}

	SDL_GL_SwapBuffers();
	capture();

	rd ^= 1;
	wr ^= 1;

	sim->step();

	for (creature_vector::iterator i = dead_creatures.begin(),
		end = dead_creatures.end(); i != end; ++i)
	{
		creature* c = *i;

		printf("creature with lifetime %lu died\n", c->_lifetime);

		c->remove_from_space(sim->_space);
		sim->_creatures.erase(c);
		delete c->_brain;
		delete c;
	}

	static double best = 0;

	for (creature_vector::iterator i = full_creatures.begin(),
		end = full_creatures.end(); i != end; ++i)
	{
		creature* c = *i;

		++c->_nr_hit_food;

		c->_food_body->p.x = 1800. * (2.0 * rand() / RAND_MAX - 1);
		c->_food_body->p.y = 1800. * (2.0 * rand() / RAND_MAX - 1);
		c->_food_body->v.x = (2.0 * rand() / RAND_MAX - 1);
		c->_food_body->v.y = (2.0 * rand() / RAND_MAX - 1);
		c->_food_body->v = cpvmult(cpvnormalize(c->_food_body->v), 40);

		c->_energy += 2000;

		double x = 1e0 * c->_nr_hit_food / (1 + c->_lifetime)
			+ 1e-5 * c->_lifetime;

		if (x > best) {
			best_bc = c->_brain_configuration;
			best = x;

			best_bc->save("best64");
			printf("best = %f\n", best);
		}
	}

	full_creatures.clear();

	static int recreate = 0;
	if (++recreate == 1000) {
		recreate = 0;

		brain_configuration* bc = new brain_configuration(64);
		bc->randomize();
		*bc = *best_bc;
		bc->mutate2();

		brain* b = new brain(bc);

		creature* c = new creature(bc, b, sim->_food_body,
			1.0 * rand() / RAND_MAX,
			1.0 * rand() / RAND_MAX,
			1.0 * rand() / RAND_MAX,
			&_died_creature_listener,
			&_ate_creature_listener);

		sim->_creatures.insert(c);
		c->add_to_space(sim->_space);
	}

	dead_creatures.clear();
}

static void
init()
{
	best_bc = new brain_configuration(64);
	best_bc->restore("best64");

	sim = new simulation();

	for (unsigned int i = 0; i < 5; ++i) {
		brain_configuration* bc = new brain_configuration(64);
		bc->randomize();
		*bc = *best_bc;
		bc->mutate2();

		brain* b = new brain(bc);

		creature* c = new creature(bc, b, sim->_food_body,
			1.0 * rand() / RAND_MAX,
			1.0 * rand() / RAND_MAX,
			1.0 * rand() / RAND_MAX,
			&_died_creature_listener,
			&_ate_creature_listener);

		sim->_creatures.insert(c);
		c->add_to_space(sim->_space);
	}
}

static void
destroy()
{
	delete sim;
}

static void
keyboard(SDL_KeyboardEvent* key)
{
	switch(key->keysym.sym) {
	case SDLK_RETURN:
		/* Reinitialise the simulation */
		destroy();
		init();
		break;
	case SDLK_ESCAPE:
		{
			SDL_Event ev;
			ev.type = SDL_QUIT;
			SDL_PushEvent(&ev);
		}
		break;
	default:
		;
	}
}

int
main(int argc, char* argv[])
{
	cpInitChipmunk();

	srand(time(NULL));

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) == -1)
		exit(1);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

	SDL_Surface *surface = SDL_SetVideoMode(WIDTH, HEIGHT, 0, SDL_OPENGL);
	if (!surface)
		exit(1);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(1, 1, 1, 0);

	glEnable(GL_TEXTURE_2D);

	glEnable(GL_LINE_SMOOTH);
	glLineWidth(1.8);

	resize(WIDTH, HEIGHT);

	init();

#ifndef CONFIG_CAPTURE
	SDL_TimerID display_timer = SDL_AddTimer(1000 / 100,
		&displayTimer, NULL);
	if (!display_timer)
		exit(1);
#endif

	int running = 1;
	while (running) {
		SDL_Event ev;

#ifdef CONFIG_CAPTURE
		display();
		while (SDL_PollEvent(&ev))
#else
		SDL_WaitEvent(&ev);
#endif
		switch (ev.type) {
		case SDL_KEYDOWN:
			keyboard(&ev.key);
			break;
		case SDL_VIDEORESIZE:
			resize(ev.resize.w, ev.resize.h);
			break;
		case SDL_QUIT:
			running = 0;
			break;
		case SDL_USEREVENT:
			display();
			break;
		}
	}

	destroy();

	SDL_Quit();
	return 0;
}
