#include "Generator.h"
#include "../App.h"

namespace ax_gen {

Generator::Generator() {
}

void Generator::init() {
	onInit();

	if (!g_ws->os ) g_ws->os  = g_ws->host_os;
	if (!g_ws->cpu) g_ws->cpu = g_ws->host_cpu;
}

void Generator::onRun() {
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

void Generator::generate() {
	Log::info("=========== generate ===============");
	onGenerate();
}

void Generator::ide() {
	Log::info("=========== Open IDE ===============");
	onIde();
}

void Generator::build() {
	Log::info("=========== Build ===============");
	onBuild();
}

void Generator::run() {
	Log::info("=========== Run ===============");
	onRun();
}

} //namespace ax_gen


