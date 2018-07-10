#include "Generator_android.h"

namespace ax_gen {

void Generator_android::onInit() {
	if (!g_ws->compiler) g_ws->compiler = "gcc";
}

void Generator_android::onGenerate() {
	gen_workspace();
}

void Generator_android::onBuild() {

}

void Generator_android::gen_workspace() {
	gen_AndroidManifest();

	String o;

	for (auto& proj : g_ws->projects) {
		gen_project(proj);
		o.append("include ", proj.name, ".make\n");
	}

	String outFilename(g_ws->buildDir, "/jni/Android.mk");
	FileUtil::writeTextFile(outFilename, o);
}

void Generator_android::gen_project(Project& proj) {
	String o;

	String projBuildDir(g_ws->buildDir, proj.name);

	proj.genData_makefile.makefile.set(proj.name, ".make");
	Log::info("gen_project ", proj.genData_makefile.makefile);

	o.append("config ?= ", g_ws->defaultConfigName(), "\n");

	o.append("include $(CLEAR_VARS)\n");
	o.append("LOCAL_PATH := .\n");
	o.append("LOCAL_MODULE := ", proj.name, "\n");
	o.append("#LOCAL_STATIC_LIBRARIES := libax\n");
	o.append("#LOCAL_LDLIBS := -lGLESv1_CM -ldl -llog -lz\n");

//		o += "LOCAL_CFLAGS := -std=c++14 -DANDROID_NDK\\\n";

	for (auto& config : proj.configs) {
		o.append(config.name, "__LOCAL_CFLAGS := \\\n");
		String include_dirs;
		for (auto& q : config.include_dirs._final) {
			o.append("\t-I", quotePath(q.path()), "\\\n");
		}
		o.append("\n");
	}	
//	o += "LOCAL_CFLAGS := -std=c++14 -DANDROID_NDK\\\n";
	o.append("LOCAL_CFLAGS := $($(config)__LOCAL_CFLAGS\n");

	o += "LOCAL_SRC_FILES := \\\n";
	for (auto& q : proj.fileEntries) {
		if (q.excludedFromBuild) continue;
		o.append("\t", q.path(), "\\\n");
	}
	o += "\n";
	o += "include $(BUILD_STATIC_LIBRARY)\n";

	//o += "\n#================\n";
	//o += "include $(CLEAR_VARS)\n";
	//o += String("LOCAL_MODULE    := ", proj.name, "-dummy\n");
	//o += "LOCAL_CFLAGS 	:= -DANDROID_NDK\n";
	////o += "LOCAL_SRC_FILES := jni/main.cpp\n";
	//o += String("LOCAL_STATIC_LIBRARIES := ", proj.name, "\n");
	//o += "include $(BUILD_EXECUTABLE)\n";

	String outFilename(g_ws->buildDir, proj.genData_makefile.makefile);
	FileUtil::writeTextFile(outFilename, o);
}

void Generator_android::gen_AndroidManifest() {
	String o =	"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
				"<manifest xmlns:android=\"http://schemas.android.com/apk/res/android\"\n"
					"package=\"com.awenix.libax\"\n"
					"android:versionCode=\"1\"\n"
					"android:versionName=\"1.0\">\n"
					"<application android:icon=\"@drawable/icon\" android:label=\"@string/app_name\" android:debuggable=\"true\">\n"
					"</application>\n"
				"</manifest>\n";

	String filename(g_ws->buildDir, "/AndroidManifest.xml");
	FileUtil::writeTextFile(filename, o);

}

String Generator_android::quotePath(const StrView& v) {
	String o;
	o.append('\"');
	for (auto ch : v) {
		switch (ch) {
			case '\"':	o.append("\\\"");	break;
			default:	o.append(ch);		break;
		}
	}
	o.append('\"');
	return o;
}
} //namespace