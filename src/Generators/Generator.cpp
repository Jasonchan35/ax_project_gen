#include "Generator.h"
#include "../App.h"

namespace ax_gen {

Generator::Generator() {
}

void Generator::run() {
	Log::info("=========== Run ===============");

	if (!g_ws->_startup_project) {
		Log::error("no startup project to run");
		return;
	}

	auto& config = g_ws->_startup_project->defaultConfig();
	if (!config.outputTarget) {
		Log::error("no output target to run in startup project ", g_ws->_startup_project->name);
		return;
	}

	System::createProcess(config.outputTarget, "");
}

} //namespace ax_gen


