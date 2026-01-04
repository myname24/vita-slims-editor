// main.cpp
#include <psp2/kernel/processmgr.h>
#include "app.h"

int main(int argc, char* argv[]) {
    App app;
    
    if (!app.Initialize()) {
        sceKernelExitProcess(1);
        return 1;
    }
    
    app.Run();
    app.Shutdown();
    
    sceKernelExitProcess(0);
    return 0;
}
