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
		bool gen    {false};
		bool build  {false};
		bool ide	{false};
		bool run    {false};
		bool verbose{false};

		String	workspaceFile;
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