/*
 * YaoPlayer.cpp
 *
 */

#include "YaoPlayer.h"
#include "YaoGarbler.h"
#include "YaoEvaluator.h"
#include "Tools/ezOptionParser.h"

YaoPlayer::YaoPlayer(int argc, const char** argv)
{
	ez::ezOptionParser opt;
	opt.add(
			"", // Default.
			1, // Required?
			1, // Number of args expected.
			0, // Delimiter if expecting multiple args.
			"This player's number, 0 for garbling, 1 for evaluating.", // Help description.
			"-p", // Flag token.
			"--player" // Flag token.
	);
	opt.add(
			"localhost", // Default.
			0, // Required?
			1, // Number of args expected.
			0, // Delimiter if expecting multiple args.
			"Host where party 0 is running (default: localhost)", // Help description.
			"-h", // Flag token.
			"--hostname" // Flag token.
	);
	opt.add(
			"5000", // Default.
			0, // Required?
			1, // Number of args expected.
			0, // Delimiter if expecting multiple args.
			"Base port number (default: 5000).", // Help description.
			"-pn", // Flag token.
			"--portnum" // Flag token.
	);
	opt.add(
			"", // Default.
			0, // Required?
			0, // Number of args expected.
			0, // Delimiter if expecting multiple args.
			"Evaluate while garbling (default: false).", // Help description.
			"-C", // Flag token.
			"--continuous" // Flag token.
	);
	opt.add(
			"1024", // Default.
			0, // Required?
			1, // Number of args expected.
			0, // Delimiter if expecting multiple args.
			"Minimum number of gates for multithreading (default: 1024).", // Help description.
			"-t", // Flag token.
			"--threshold" // Flag token.
	);
	OnlineOptions online_opts(opt, argc, argv);
	opt.parse(argc, argv);
	opt.syntax = "./yao-player.x [OPTIONS] <progname>";
	if (opt.lastArgs.size() == 1)
	{
		progname = *opt.lastArgs[0];
	}
	else
	{
		string usage;
		opt.getUsage(usage);
		cerr << usage;
		exit(1);
	}

	int my_num;
	int pnb;
	string hostname;
	int threshold;
	opt.get("-p")->getInt(my_num);
	opt.get("-pn")->getInt(pnb);
	opt.get("-h")->getString(hostname);
	bool continuous = opt.get("-C")->isSet;
	opt.get("-t")->getInt(threshold);

	ThreadMasterBase* master;
	if (my_num == 0)
	    master = new YaoGarbleMaster(continuous, online_opts, threshold);
	else
	    master = new YaoEvalMaster(continuous, online_opts);

	server = Server::start_networking(master->N, my_num, 2, hostname, pnb);
	master->run(progname);
}

YaoPlayer::~YaoPlayer()
{
	if (server)
		delete server;
}
