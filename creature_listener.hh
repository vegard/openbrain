#ifndef CREATURE_LISTENER_HH
#define CREATURE_LISTENER_HH

class creature;

class creature_listener {
public:
	creature_listener();
	virtual ~creature_listener();

public:
	virtual void handle(creature* e) = 0;
};

creature_listener::creature_listener()
{
}

creature_listener::~creature_listener()
{
}

#endif
