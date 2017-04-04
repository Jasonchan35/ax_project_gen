#include "common.h"
#include "App.h"
#include "Generators/Generator_vs2015.h"
#include "Generators/Generator_makefile.h"
#include "Generators/Generator_xcode.h"

namespace ax_gen {

App* g_app;

App::App() {
	g_app = this;
}

int App::run(int argc, char* argv[]) {
	int ret = -1;
	try{
		ret = _run(argc, argv);
		Log::info("======== End ========");
	}catch(...){
		ret = -1;
	}

	Log::closeLogFile();
	return ret;
}

void App::readArgs(int argc, char* argv[]) {
	for (int i=1; i<argc; i++) {
		auto s = StrView_c_str(argv[i]);
		Log::info("\targ[", i, "]: ", s);

		if (s == "-gen") {
			options.gen = true;
		}else if (s == "-build") {
			options.build = true;
		}else if (s == "ide") {
			options.ide = true;

		}else if (s == "-verbose") {
			options.verbose = true;

		}else if (auto v = s.getFromPrefix("ws=")) {
			options.workspaceFile = v;

		}else if (auto v = s.getFromPrefix("os=")) {
			workspace.os = v;

		}else if (auto v = s.getFromPrefix("compiler=")) {
			workspace.compiler = v;

		}else if (auto v = s.getFromPrefix("gen=")) {
			workspace.generator = v;

		}else{
			Log::info("Unknown arg ", s);
		}
	}
}

int App::_run(int argc, char* argv[]) {
	Log::info("==== ax_gen ====");

	readArgs(argc, argv);

	if (!workspace.generator) { 
		throw Error("please specific generator");

	}else if (workspace.generator == "vs2015"  ) { 

		if (g_ws->os != "windows") {
			throw Error("Unsupported os ", g_ws->os);
		}

		_generator.reset(new Generator_vs2015());

	}else if (workspace.generator == "vs2015_linux") {

		g_ws->os = "linux";
		_generator.reset(new Generator_vs2015() );

	}else if (workspace.generator == "makefile") { 
		_generator.reset(new Generator_makefile());

	}else if (workspace.generator == "xcode"   ) { 
		_generator.reset(new Generator_xcode());

	}else{
		throw Error("Unsupported generator '", workspace.generator, "'");
	}

	if (!workspace.compiler) {
		throw Error("please specific compiler");
	}

	if (!options.workspaceFile) {
		throw Error("please specific workspace file");
	}

	workspace.readFile(options.workspaceFile);

	StringStream ss;

	workspace.resolve();
	workspace.dump(ss);

	Log::info(ss.str());

	if (options.gen) {
		_generator->generate();	
	}

	if (options.ide) {
		_generator->ide();
	}

	if (options.build) {
		_generator->build();
	}

	if (options.run) {
		_generator->run();
	}

	return 0;
}

} //namespace