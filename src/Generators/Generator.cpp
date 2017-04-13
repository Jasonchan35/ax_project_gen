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
	if (!g_ws->startupProject) {
		Log::error("no startup project to run");
		return;
	}

	auto& config = g_ws->startupProject->configToBuild();
	if (!config.outputTarget) {
		Log::error("no output target to run in startup project ", g_ws->startupProject->name);
		return;
	}

	System::createProcess(config.outputTarget.absPath(), "");
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
	Log::info("=========== Build [", g_app->options.config ,"] ===============");
	onBuild();
}

void Generator::run() {
	Log::info("=========== Run [", g_app->options.config ,"] ===============");
	onRun();
}

} //namespace ax_gen


