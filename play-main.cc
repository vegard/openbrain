extern "C" {
#include <math.h>
#include <time.h>
}

extern "C" {
#include <SDL.h>
#include <SDL_opengl.h>
#include <chipmunk.h>
}

#undef CONFIG_CAPTURE

#include "brain.hh"
#include "brain_configuration.hh"
#include "capture.hh"
#include "simulation.hh"

static brain_configuration* main_bc;

#define NR_CREATURES 120

static brain_configuration* bc[NR_CREATURES];
static brain* b[NR_CREATURES];
static creature* c[NR_CREATURES];

static simulation* sim;

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

#if 0
		unsigned int n = c->_brain->_nr_neurones;
		double* synapses = c->_brain->_synapses;
		double* neurones = c->_brain->_neurones[rd];
#endif

		glPushMatrix();
		glTranslatef(c->_creature_body->p.x, c->_creature_body->p.y, 0);
		glRotatef(cpvtoangle(c->_creature_body->rot) * 180 / M_PI, 0, 0, 1);
		glScalef(50, 50, 0);

		glColor3f(1, 0, 0);
		draw_square();

		glColor3f(0, 0, 0);
		draw_box();

#if 0
		glColor4f(0, 0, 0, 1);
		glBegin(GL_LINES);
		glVertex2f(0, 0);
		glVertex2f(5 * neurones[creature::NEURONE_OUTPUT_MOVE_LEFT], 0);

		glVertex2f(0, 0);
		glVertex2f(-5 * neurones[creature::NEURONE_OUTPUT_MOVE_RIGHT], 0);

		glVertex2f(0, 0);
		glVertex2f(0, -5 * neurones[creature::NEURONE_OUTPUT_MOVE_UP]);

		glVertex2f(0, 0);
		glVertex2f(0, 5 * neurones[creature::NEURONE_OUTPUT_MOVE_DOWN]);
		glEnd();
#endif

		glPopMatrix();
	}

#if 0
	glColor4f(0, 0, 0, 0.5);
	glBegin(GL_LINES);
	glVertex2f(sim->_creature_body->p.x, sim->_creature_body->p.y);
	glVertex2f(sim->_food_body->p.x, sim->_food_body->p.y);
	glEnd();
#endif

	SDL_GL_SwapBuffers();
	capture();

	rd ^= 1;
	wr ^= 1;

	sim->step();
}

static void
init()
{
	main_bc = new brain_configuration(32);
	main_bc->restore("best-brain");
	sim = new simulation();

	for (unsigned int i = 0; i < NR_CREATURES; ++i) {
		bc[i] = new brain_configuration(32);
		*bc[i] = *main_bc;

		b[i] = new brain(bc[i]);
		c[i] = new creature(b[i], sim->_food_body);
		sim->_creatures.insert(c[i]);
		c[i]->add_to_space(sim->_space);
	}
}

static void
destroy()
{
	for (unsigned int i = 0; i < NR_CREATURES; ++i) {
		c[i]->remove_from_space(sim->_space);
		delete c[i];
		delete b[i];
		delete bc[i];
	}

	delete main_bc;
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

	SDL_Surface *surface = SDL_SetVideoMode(640, 480, 0, SDL_OPENGL);
	if (!surface)
		exit(1);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(1, 1, 1, 0);

	glEnable(GL_TEXTURE_2D);

	glEnable(GL_LINE_SMOOTH);
	glLineWidth(1.8);

	resize(640, 480);

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
