#include <MyLib.h>

int main() {
	#if ax_GEN_OS_android
		
	#else
		std::cout << "Hello World !!\n";
	#endif

	MyLib_testing();
	return 0;
}