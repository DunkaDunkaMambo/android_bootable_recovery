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

void choose_bootanimation_menu(const char *ba_path)
{
    if (ensure_path_mounted(ba_path) != 0) {
        LOGE("Can't mount %s\n", ba_path);
        return;
    }

    static char* headers[] = {  "Choose a boot animation",
                                "",
                                NULL
    };

    char* ba_file = choose_file_menu(ba_path, ".zip", headers);
    if (ba_file == NULL)
        return;

    if (confirm_selection("Confirm change boot animation?", "Yes - Change")) {
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

    char* list[] = { "Choose boot animation from internal sdcard",
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

    static char* list[] = { "Change boot animation",
                            "Toggle smaller confirmation sub-menus",
                            "Toggle nandroid progress",
                            "Toggle backup of internal storage",
                            "Toggle restore of internal storage",
                            "Toggle restore of oem info",
                            "Create zip file with the current ROM",
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
                   ui_print("Smaller confirmation sub-menus disabled\n");
                } else {
                   __system("touch /emmc/clockworkmod/.one_confirm");
                   ui_print("Smaller confirmation sub-menus enabled\n");
                }
		break;
	    case 2:
                ensure_path_mounted("/emmc");
                if( access("/emmc/clockworkmod/.hidenandroidprogress", F_OK ) != -1 ) {
                   __system("rm -rf /emmc/clockworkmod/.hidenandroidprogress");
                   ui_print("Nandroid progress will be shown\n");
                } else {
                   __system("touch /emmc/clockworkmod/.hidenandroidprogress");
                   ui_print("Nandroid progress will be hidden\n");
                }
                break;
	    case 3:
                ensure_path_mounted("/emmc");
                if( access("/emmc/clockworkmod/.backup_internal", F_OK ) != -1 ) {
                   __system("rm -rf /emmc/clockworkmod/.backup_internal");
                   ui_print("Backup of internal storage disabled\n");
                } else {
                   __system("touch /emmc/clockworkmod/.backup_internal");
                   ui_print("Backup of internal storage enabled\n");
                }
                break;
	    case 4:
                ensure_path_mounted("/emmc");
                if( access("/emmc/clockworkmod/.restore_internal", F_OK ) != -1 ) {
                   __system("rm -rf /emmc/clockworkmod/.restore_internal");
                   ui_print("Restore of internal storage disabled\n");
                } else {
                   __system("touch /emmc/clockworkmod/.restore_internal");
                   ui_print("Restore of internal storage enabled\n");
                }
                break;
	    case 5:
                ensure_path_mounted("/emmc");
                if( access("/emmc/clockworkmod/.restore_imei", F_OK ) != -1 ) {
                   __system("rm -rf /emmc/clockworkmod/.restore_imei");
                   ui_print("Restore of imei disabled\n");
                } else {
                   __system("touch /emmc/clockworkmod/.restore_imei");
                   ui_print("Restore of imei enabled\n");
                }
                break;
	    case 6:
		ensure_path_mounted("/system");
		ensure_path_mounted("/emmc");
                if (confirm_selection("Create a zip from system and boot?", "Yes - Create zip file")) {
		ui_print("Dumping current rom to a zip file...\n");
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
        ui_print("Dump to zip file completed!\n");
        ui_print("Zip created in /emmc/clockworkmod/zips/\n");
	}
		ensure_path_unmounted("/system");
		break;
	}
    }
}
