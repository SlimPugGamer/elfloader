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
        printf("Failed to open directory: %s\n", path);
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        const char* ext = strrchr(entry->d_name, '.');
        if (ext && strcmp(ext, ".elf") == 0) {
            snprintf(elfPaths[elfCount], MAX_PATH, "%s%s", path, entry->d_name);
            elfCount++;
            if (elfCount >= MAX_ELFS) {
                printf("Too many ELF files, truncating list.\n");
                break;
            }
        }
    }

    closedir(dir);
}

void displayMenu(GSGLOBAL* gsGlobal, GSFONTM* font) {
    gsKit_clear(gsGlobal, 0x000000FF);

    gsKit_fontm_print(gsGlobal, font, 0, 0, 0, 0xFFFFFFFF, "PS2 ELF Loader\n==============");

    char menuText[256];
    int i;

    for (i = 0; i < elfCount; i++) {
        if (i == selectedIndex) {
            snprintf(menuText, sizeof(menuText), "-> %s", elfPaths[i]);
            gsKit_fontm_print(gsGlobal, font, 0, (i + 1) * 20, 0, 0xFFFF00FF, menuText);
        }
        else {
            snprintf(menuText, sizeof(menuText), "   %s", elfPaths[i]);
            gsKit_fontm_print(gsGlobal, font, 0, (i + 1) * 20, 0, 0xFFFFFFFF, menuText);
        }
    }

    gsKit_sync_flip(gsGlobal);
}

void executeELF(const char* path) {
    printf("\nLoading ELF: %s\n", path);
    SifLoadFileInit();

    int ret = SifLoadElf(path, NULL);
    if (ret < 0) {
        printf("Failed to load ELF: %d\n", ret);
        SifLoadFileExit();
        return;
    }

    printf("Execution passed to the ELF.\n");
    SleepThread();
}

int main(int argc, char* argv[]) {
    GSGLOBAL* gsGlobal = gsKit_init_global();
    if (gsGlobal == NULL) {
        printf("Error initializing GSKit!\n");
        return 1;
    }

    gsKit_mode_switch(gsGlobal, GS_MODE_NTSC);
    GSFONTM font;
    int ret = gsKit_fontm_upload(gsGlobal, &font);
    if (ret != 0) {
        printf("Error uploading font!\n");
        return 1;
    }

    padInit(0);

    printf("\nPS2 ELF Loader\n==============\n");

    listELFs("mass:/");
    listELFs("mc0:/");
    listELFs("mc1:/");

    if (elfCount == 0) {
        printf("No ELF files found.\nPress any button to exit.\n");
        while (1) {
            struct padButtonStatus buttons;
            if (padRead(0, 0, &buttons))
                break;
        }
        return 0;
    }

    while (1) {
        displayMenu(gsGlobal, &font);

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
