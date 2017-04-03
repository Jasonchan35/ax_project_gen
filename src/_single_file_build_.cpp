#include "common.h"

#include "common.cpp"

#include "Util/Error.cpp"
#include "Util/FileUtil.cpp"
#include "Util/Path.cpp"
#include "Util/JsonReader.cpp"
#include "Util/JsonWriter.cpp"
#include "Util/XmlWriter.cpp"
#include "Util/Log.cpp"
#include "Util/String.cpp"
#include "Util/Uuid.cpp"
#include "Util/Glob.cpp"

#include "Config/Config.cpp"
#include "Config/Project.cpp"
#include "Config/Workspace.cpp"

#include "Config/FileEntry.cpp"
#include "Config/VirtualFolder.cpp"
#include "Config/ProjectCategory.cpp"

#include "Generators/Generator.cpp"

#include "App.cpp"
#include "main.cpp"

#include "Generators/Generator_vs2015.cpp"
#include "Generators/Generator_makefile.cpp"
#include "Generators/Generator_xcode.cpp"
#include "Generators/XCodePbxWriter.cpp"

