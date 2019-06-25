#include <stdio.h>
#include "../Interface/IApplication.hpp"

using namespace GameEngine;

namespace GameEngine{
	extern IApplication *g_pApplication;
}

int main(){
	int ret;

	if((ret = g_pApplication->Initialize()) != 0){
		printf("App Initialize failed, will exit now.");
		return ret;
	}

	while(!g_pApplication->IsQuit()){
		printf("main loop!\n");
		g_pApplication->Tick();
	}

	g_pApplication->Finalize();

	return 0;
}