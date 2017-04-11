#pragma once

#include "common.h"
#include "Config/Workspace.h"
#include "Generators/Generator.h"

namespace ax_gen {

class App : public NonCopyable {
public:
	App();
	int		run(int argc, char* argv[]);

	struct Options {
		bool help	{false};
		bool gen    {false};
		bool build  {false};
		bool ide	{false};
		bool run    {false};
		bool verbose{false};
		bool _hasAction{false};

		String	workspaceFile;
		String	config; //config for -build / -run
	};

	Options	options;

	Workspace				workspace;
	unique_ptr<Generator>	_generator;

private:
	int  _run(int argc, char* argv[]);
	void readArgs(int argc, char* argv[]);
};

extern App* g_app;

} //namespace
