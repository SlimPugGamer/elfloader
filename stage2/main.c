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
#include <gsKit.h>
#include <loadfile.h>

#define MAX_ELFS 256
#define MAX_PATH 256

char elfPaths[MAX_ELFS][MAX_PATH];
int elfCount = 0;
int selectedIndex = 0;

void listELFs(const char* path) {
    DIR* dir;
    struct dirent* entry;

    dir = opendir(path);
    if (dir == NULL) {
        scr_printf("Failed to open directory: %s\n", path);
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        const char* ext = strrchr(entry->d_name, '.');
        if (ext && strcmp(ext, ".elf") == 0) {
            snprintf(elfPaths[elfCount], MAX_PATH, "%s%s", path, entry->d_name);
            elfCount++;
            if (elfCount >= MAX_ELFS) {
                scr_printf("Too many ELF files, truncating list.\n");
                break;
            }
        }
    }

    closedir(dir);
}

void displayMenu() {
    scr_clear();
    scr_printf("\nPS2 ELF Loader\n==============\n");
    int i;
    for (i = 0; i < elfCount; i++) {
        if (i == selectedIndex)
            scr_printf("-> %s\n", elfPaths[i]);
        else
            scr_printf("   %s\n", elfPaths[i]);
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

int main(int argc, char* argv[]) {
    SifInitRpc(0);
    init_scr();
    padInit(0);

    scr_printf("\nPS2 ELF Loader\n==============\n");

    listELFs("mass:/");
    listELFs("mc0:/");
    listELFs("mc1:/");

    if (elfCount == 0) {
        scr_printf("No ELF files found.\nPress any button to exit.\n");
        while (1) {
            if (padRead(0, 0, NULL))
                break;
        }
        return 0;
    }

    while (1) {
        displayMenu();

        struct padButtonStatus buttons;
        padRead(0, 0, &buttons);

        unsigned int paddata = 0xffff ^ buttons.btns;

        if (paddata & PAD_UP) {
            selectedIndex--;
            if (selectedIndex < 0)
                selectedIndex = elfCount - 1;
        }
        else if (paddata & PAD_DOWN) {
            selectedIndex++;
            if (selectedIndex >= elfCount)
                selectedIndex = 0;
        }
        else if (paddata & PAD_CROSS) {
            executeELF(elfPaths[selectedIndex]);
        }

        sleep(1);
    }

    return 0;
}
