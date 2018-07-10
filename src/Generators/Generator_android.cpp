#include "Generator_android.h"

namespace ax_gen {

void Generator_android::onInit() {
	if (!g_ws->compiler) g_ws->compiler = "gcc";
	g_ws->os = "android";
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
	o.append("LOCAL_CPP_EXTENSION := .cxx .cpp .cc\n");
	o.append("#LOCAL_STATIC_LIBRARIES := libax\n");
	o.append("#LOCAL_LDLIBS := -lGLESv1_CM -ldl -llog -lz\n");

	for (auto& config : proj.configs) {
		String cpp_defines;
		String cpp_flags;
		String link_flags;
		String link_libs;
		String link_files;
		String include_files;
		String include_dirs;
		String cpp_obj_files;

		for (auto& q : config.cpp_defines._final) {
			cpp_defines.append("\\\n\t-D", q.path());
		}
		for (auto& q : config.cpp_flags._final) {
			cpp_flags.append("\\\n\t", q.path());
		}
		for (auto& q : config.link_flags._final) {
			link_flags.append("\\\n\t", q.path());
		}
		for (auto& q : config.link_dirs._final) {
			link_libs.append("\\\n\t-L", quotePath(q.path()));
		}
		for (auto& q : config.link_libs._final) {
			link_libs.append("\\\n\t-l", q.path());
		}
		for (auto& q : config.link_files._final) {
			link_files.append("\\\n\t", quotePath(q.path()));
		}
		for (auto& q : config.include_files._final) {
			include_files.append("\\\n\t-include ", quotePath(q.path()));
		}
		for (auto& q : config.include_dirs._final) {
			include_dirs.append("\\\n\t-I", quotePath(q.path()));
		}

		o.append(config.name, "__LOCAL_CFLAGS := ", cpp_flags, cpp_defines, include_dirs, include_files, "\n\n");
		o.append(config.name, "__LOCAL_LDFLAGS := ", link_flags, link_libs, link_files, "\n\n");
	}
	o.append("LOCAL_CFLAGS := $($(config)__LOCAL_CFLAGS)\n");

	{
		String dep_libs;
		String dep_dlls;
		for (auto& dp : proj._dependencies) {
			if (dp->type_is_lib()) {
				dep_libs.append(dp->name);
			} else if (dp->type_is_dll()) {
				dep_dlls.append(dp->name);
			}
		}

		o.append("LOCAL_STATIC_LIBRARIES := ", dep_libs, "\n");
		o.append("LOCAL_SHARED_LIBRARIES := ", dep_dlls, "\n");
	}
	
	{
		o += "LOCAL_SRC_FILES := ";
		for (auto& q : proj.fileEntries) {
			if (q.excludedFromBuild) continue;
			o.append("\\\n\t", escapeString(q.path()));
		}
		o += "\n\n";
	}

	if (proj.type_is_exe()) {
		o += "include $(BUILD_EXECUTABLE)\n";
	} else if (proj.type_is_dll()) {
		o += "include $(BUILD_SHARED_LIBRARY)\n";
	} else if (proj.type_is_lib()) {
		o += "include $(BUILD_STATIC_LIBRARY)\n";
	}

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

String Generator_android::escapeString(const StrView& v) {
	String o;
	for (auto ch : v) {
		switch (ch) {
			case ':': ax_fallthrough
			case ' ': {
				o.append('\\', ch);
			}break;
			default:	o.append(ch);		break;
		}
	}
	return o;
}
} //namespace