#include <pspkernel.h>
#include <pspctrl.h>
#include <pspdebug.h>
#include <pspsdk.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "unzipToDir.h"

#define RGB(r, g, b) ((b << 16) | (g << 8) | r)
#define YELLOW RGB(255, 255, 0)
#define BLACK RGB(0, 0, 0)
#define WHITE RGB(255, 255, 255)
#define RED RGB(255, 0, 0)

#define VERSION "1.0.1"
#define SEPARATOR "------------------------------------------------------------------"

PSP_MODULE_INFO("P-SPot_Extractor", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);
PSP_HEAP_SIZE_KB(6*1024);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Globals:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int runningFlag = 1;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Callbacks:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Exit callback */
int exit_callback(int arg1, int arg2, void *common) {
    runningFlag = 0;
    return 0;
}

/* Callback thread */
int CallbackThread(SceSize args, void *argp) {
    int cbid;
    cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
    sceKernelRegisterExitCallback(cbid);
    sceKernelSleepThreadCB();
    return 0;
}

/* Sets up the callback thread and returns its thread id */
int SetupCallbacks(void) {
    int thid = 0;
    thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, PSP_THREAD_ATTR_USER, 0);
    if(thid >= 0)
        sceKernelStartThread(thid, 0, 0);
    return thid;
}

double getMSfreeSpace(){
    unsigned int buf[5];
    unsigned int *pbuf = buf;
    sceIoDevctl("ms0:", 0x02425818, &pbuf, sizeof(pbuf), 0, 0);
    return buf[1]*buf[3]*buf[4];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Main:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(){
    char ebootDirectory[262] = "";
    char buffer[262] = "";
    
	pspDebugScreenInit();
	SetupCallbacks();

    getcwd(ebootDirectory, 262);
    pspDebugScreenSetTextColor(YELLOW);
    pspDebugScreenPrintf("P-SPot Extractor vers. %s\n", VERSION);
    pspDebugScreenPrintf("by Sakya (based on Tipster's UnzipUnrar)\n");
    pspDebugScreenPrintf("Source available at http://sakya.altervista.org\n");
    pspDebugScreenPrintf("%s\n\n", SEPARATOR);
    pspDebugScreenSetTextColor(WHITE);

    pspDebugScreenPrintf("Reading config file: ");
    if (readConfig()){
        pspDebugScreenSetTextColor(RED);
        pspDebugScreenPrintf("ERROR\n");
        sceKernelDelayThread(3000000);
    	sceKernelExitGame();
        return 0;
    }else
        pspDebugScreenPrintf("OK\n");
        
    pspDebugScreenPrintf("Zip file     : %s\n", config.zipFile);
    pspDebugScreenPrintf("App name     : %s\n", config.appName);
    pspDebugScreenPrintf("Destination  : %s\n", config.destDir);
    pspDebugScreenPrintf("Needed space : %i kb\n", (int)(config.bytesNeeded/1024));
    //sceIoRmdir(config.destDir);
    double freeSpace = getMSfreeSpace();
    pspDebugScreenPrintf("MS free space: %i kb\n\n", (int)(freeSpace/1024));

    if (config.bytesNeeded > freeSpace){
        pspDebugScreenSetTextColor(RED);
        pspDebugScreenPrintf("ERROR: not enough free space on Memory Stick!");
        sceKernelDelayThread(3000000);
    	sceKernelExitGame();
        return 0;
    }

    pspDebugScreenSetTextColor(YELLOW);
    pspDebugScreenPrintf("%s\n", SEPARATOR);
    pspDebugScreenSetTextColor(RED);
    pspDebugScreenPrintf("\nPress X to confirm, O to cancel\n");
    pspDebugScreenSetTextColor(WHITE);
    SceCtrlData pad;
    while(runningFlag){
        sceCtrlReadBufferPositive(&pad, 1);
        if (pad.Buttons & PSP_CTRL_CROSS)
            break;
        else if (pad.Buttons & PSP_CTRL_CIRCLE){
            pspDebugScreenPrintf("Aborted by user!");
            sceKernelDelayThread(3000000);
        	sceKernelExitGame();
        	return 0;
        }
    }

    pspDebugScreenSetTextColor(YELLOW);
    int unzip = unzipToDir(config.zipFile, config.destDir, NULL);
    if (unzip){
        pspDebugScreenSetTextColor(RED);
        pspDebugScreenPrintf("ERROR: unzip failed!");
        sceKernelDelayThread(3000000);
    	sceKernelExitGame();
        return 0;
    }

    if (config.autoDelete){
        pspDebugScreenPrintf("Removing temp file: ");
        // motolegacy (22/07/2021) -- replace sprintf() with strcpy() and strcat()
        strcpy(buffer, ebootDirectory);
        strcat(buffer, "/");
        strcat(buffer, "EBOOT.PBP");
        sceIoRemove(buffer);
        // motolegacy (22/07/2021) -- ^^^
        strcpy(buffer, ebootDirectory);
        strcat(buffer, "/");
        strcat(buffer, "unzip.cfg");
        sceIoRemove(buffer);
        chdir(ebootDirectory);
        sceIoRemove(config.zipFile);
        sceIoRmdir(ebootDirectory);
        pspDebugScreenPrintf("OK\n");
    }
    pspDebugScreenPrintf("\n%s\n", SEPARATOR);
    pspDebugScreenSetTextColor(WHITE);
    
    pspDebugScreenSetTextColor(RED);
    pspDebugScreenPrintf("\n\nPress X to quit\n");
    while(runningFlag){
        sceCtrlReadBufferPositive(&pad, 1);
        if (pad.Buttons & PSP_CTRL_CROSS){
            break;
        }
    }
	sceKernelExitGame();
    return 0;

}
