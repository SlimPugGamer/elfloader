#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <tamtypes.h>
#include <kernel.h>
#include <debug.h>
#include <sifrpc.h>
#include <iopheap.h>
#include <fileio.h>
#include <ps2sdkapi.h>
#include <libpad.h>
#include <loadfile.h>
#include <sbv_patches.h>


#define MAX_APPS 256
#define MAX_PATH 256
#define MAX_NAME_LENGTH 128

typedef struct {
    char appName[MAX_NAME_LENGTH];
    char appPath[MAX_PATH];
} AppEntry;

AppEntry apps[MAX_APPS];
int appCount = 0;
int selectedIndex = 0;

const char* ROM0_OSDSYS_PATH = "rom0:OSDSYS";

void readAppsList(const char* appsListPath) {
    struct fio_stat stat;
    int result = fioStat(appsListPath, &stat);
    if (result < 0) {
        scr_printf("APPS.LST file does not exist: %s\n", appsListPath);
        return;
    }
    int file = fioOpen(appsListPath, O_RDONLY);
    if (file < 0) {
        scr_printf("Failed to open apps list: %s\n", appsListPath);
        return;
    }

    char line[MAX_PATH];
    int lineIndex = 0;
    while (fioRead(file, &line[lineIndex], 1) > 0) {
        char c = line[lineIndex];
        lineIndex++;

        if (c == '\n' || lineIndex >= MAX_PATH - 1) {
            line[lineIndex] = '\0';
            if (strstr(line, "APP") == line) {
                char* equalSign = strchr(line, '=');
                if (equalSign) {
                    *equalSign = '\0';
                    char* appName = line;
                    char* appPath = equalSign + 1;
                    if (appCount < MAX_APPS) {
                        strncpy(apps[appCount].appName, appName, MAX_NAME_LENGTH);
                        strncpy(apps[appCount].appPath, appPath, MAX_PATH);
                        appCount++;
                    }
                }
            }

            lineIndex = 0;
        }
    }

    fioClose(file);
}

void displayMenu() {
    scr_printf("\x1b[2J");
    scr_printf("PS2 ELF Loader\n==============\n");
    scr_printf("-> %s\n", ROM0_OSDSYS_PATH);
    for (int i = 0; i < appCount; i++) {
        scr_printf((i == selectedIndex) ? "-> %s\n" : "   %s\n", apps[i].appPath);
    }
}

void executeELF(const char* path) {
    scr_printf("\nLoading ELF: %s\n", path);
    SifLoadFileInit();

    int ret = SifLoadElf(path, NULL);
    if (ret < 0) {
        scr_printf("Failed to load ELF: %d\n", ret);
        SifLoadFileExit();
        return;
    }

    scr_printf("Execution passed to the ELF.\n");
    SleepThread();
}

void waitForExit() {
    scr_printf("No ELF files found.\nPress any button to exit.\n");

    while (1) {
        struct padButtonStatus buttons;
        if (padRead(0, 0, &buttons)) {
            break;
        }
    }
}

int main(int argc, char* argv[]) {
    sbv_patch_disable_prefix_check();
    init_scr();
    scr_clear();
    sleep(1);
    readAppsList("mc0:/APPS.LST");

    scr_printf("\nPS2 ELF Loader\n==============\n");
    while (1) {
        SifInitRpc(0);
        padInit(0);
        displayMenu();

        struct padButtonStatus buttons;
        padRead(0, 0, &buttons);

        unsigned int paddata = 0xFFFF ^ buttons.btns;

        if (paddata & PAD_UP) {
            selectedIndex = (selectedIndex - 1 + appCount) % appCount;
        }
        else if (paddata & PAD_DOWN) {
            selectedIndex = (selectedIndex + 1) % appCount;
        }
        else if (paddata & PAD_CROSS) {
            if (selectedIndex == 0) {

                executeELF(ROM0_OSDSYS_PATH);
            }
            else {
                executeELF(apps[selectedIndex].appPath);
            }
            break;
        }

        sleep(1);
    }

    scr_printf("End of list");
    return 0;
}
