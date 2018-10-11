/*
 * YaoPlayer.h
 *
 */

#ifndef YAO_YAOPLAYER_H_
#define YAO_YAOPLAYER_H_

#include "Networking/Player.h"
#include "Networking/Server.h"

class YaoPlayer
{
	string progname;
	Names N;
	Server* server;

public:
	YaoPlayer(int argc, const char** argv);
	~YaoPlayer();
};

#endif /* YAO_YAOPLAYER_H_ */
