#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/reboot.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <sys/wait.h>
#include <sys/limits.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/statfs.h>

#include <signal.h>
#include <sys/wait.h>

#include "bootloader.h"
#include "common.h"
#include "cutils/properties.h"
#include "firmware.h"
#include "install.h"
#include "make_ext4fs.h"
#include "minui/minui.h"
#include "minzip/DirUtil.h"
#include "roots.h"
#include "recovery_ui.h"

#include "extendedcommands.h"
#include "nandroid.h"
#include "mounts.h"
#include "flashutils/flashutils.h"
#include "edify/expr.h"
#include <libgen.h>
#include "mtdutils/mtdutils.h"
#include "bmlutils/bmlutils.h"
#include "cutils/android_reboot.h"
#include "extras.h"

int create_customzip(const char* custompath)
{
    char command[PATH_MAX];
    sprintf(command, "create_update_zip.sh %s", custompath);
    __system(command);
    return 0;
}

#define SCRIPT_COMMAND_SIZE 512

void choose_bootanimation_menu(const char *ba_path)
{
    if (ensure_path_mounted(ba_path) != 0) {
        LOGE("Can't mount %s\n", ba_path);
        return;
    }

    static char* headers[] = {  "Choose a Boot Animation",
                                "",
                                NULL
    };

    char* ba_file = choose_file_menu(ba_path, ".zip", headers);
    if (ba_file == NULL)
        return;

    if (confirm_selection("Confirm change Boot Animation?", "Yes - Change")) {
        char tmp[PATH_MAX];
	sprintf(tmp, "change_ba.sh %s", ba_file);
	ensure_path_mounted("/system");
	__system(tmp);
	ensure_path_unmounted("/system");
	ui_print("Boot Animation has been changed.\n");
    }
}

void show_bootanimation_menu() {
    static char* headers[] = {  "Boot Animation Menu",
                                "",
                                NULL
    };

    char* list[] = { "Choose Boot Bnimation from internal sdcard",
        "Choose Boot Animation from external sdcard",
        NULL
    };

    int chosen_item = get_menu_selection(headers, list, 0, 0);
    switch (chosen_item) {
        case 0:
                choose_bootanimation_menu("/emmc/");
                break;
        case 1:
                choose_bootanimation_menu("/sdcard/");
                break;
    }
}

void show_extras_menu()
{
    static char* headers[] = {  "Extras Menu",
                                "",
                                NULL
    };

    static char* list[] = { "Change Boot Animation",
                            "Toggle smaller confirmation sub-menus",
                            "Toggle backup & restore progress",
                            "Toggle backup of android_secure internal or external",
                            "Create custom backup zip",
                            NULL
    };

    for (;;)
    {
        int chosen_item = get_filtered_menu_selection(headers, list, 0, 0, sizeof(list) / sizeof(char*));
        if (chosen_item == GO_BACK)
            break;
        switch (chosen_item)
        {
            case 0:
		show_bootanimation_menu();
		break;
	    case 1:
		ensure_path_mounted("/emmc");
                if( access("/emmc/clockworkmod/.one_confirm", F_OK ) != -1 ) {
                   __system("rm -rf /emmc/clockworkmod/.one_confirm");
                   ui_print("one confirm disabled\n");
                } else {
                   __system("touch /emmc/clockworkmod/.one_confirm");
                   ui_print("one confirm enabled\n");
                }
		break;
	    case 2:
                ensure_path_mounted("/emmc");
                if( access("/emmc/clockworkmod/.hidenandroidprogress", F_OK ) != -1 ) {
                   __system("rm -rf /emmc/clockworkmod/.hidenandroidprogress");
                   ui_print("nandroid progress will be shown\n");
                } else {
                   __system("touch /emmc/clockworkmod/.hidenandroidprogress");
                   ui_print("nandroid progress will be hidden\n");
                }
                break;
	    case 3:
                ensure_path_mounted("/emmc");
                if( access("/emmc/clockworkmod/.is_as_external", F_OK ) != -1 ) {
                   __system("rm -rf /emmc/clockworkmod/.is_as_external");
                   ui_print("android_secure will be set to internal\n");
                } else {
                   __system("touch /emmc/clockworkmod/.is_as_external");
                   ui_print("android_secure will be set to external\n");
                }
                break;
	    case 4:
		ensure_path_mounted("/system");
		ensure_path_mounted("/emmc");
                if (confirm_selection("Create a zip from system and boot?", "Yes - Create custom zip")) {
		ui_print("Creating custom zip...\n");
		ui_print("This may take a while. Be Patient.\n");
                    char custom_path[PATH_MAX];
                    time_t t = time(NULL);
                    struct tm *tmp = localtime(&t);
                    if (tmp == NULL)
                    {
                        struct timeval tp;
                        gettimeofday(&tp, NULL);
                        sprintf(custom_path, "/emmc/clockworkmod/zips/%d", tp.tv_sec);
                    }
                    else
                    {
                        strftime(custom_path, sizeof(custom_path), "/emmc/clockworkmod/zips/%F.%H.%M.%S", tmp);
                    }
                    create_customzip(custom_path);
		ui_print("custom zip created in /emmc/clockworkmod/zips/\n");
	}
		ensure_path_unmounted("/system");
		break;
	}
    }
}
