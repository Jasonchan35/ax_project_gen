#include <stdio.h>
#include <MyLib.h>

void MyLib_testing() {
	#if ax_GEN_OS_android
		
	#else
		std::cout << "MyLib_testing";
	#endif

}