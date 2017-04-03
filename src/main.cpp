#include "common.h"
#include "App.h"

#if _MSC_VER
	#include <windows.h>
	#include <conio.h>
#endif

int main(int argc, char *argv[]) {	
	int ret = 0;
	ax_gen::App app;

	ret = app.run(argc, argv);
	std::cout << "==== Program Ended return: " << ret << " ====\n";

	#if _MSC_VER
		if (IsDebuggerPresent() ) {
			std::cout << "! Press any key to exit\n";
			_getch();
		}
	#endif

	return 0;
}
