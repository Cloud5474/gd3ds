#include <3ds.h>
#include <citro2d.h>
#include "menus/components/ui_element.h"
#include "menus/components/ui_screen.h"
#include "math_helpers.h"
#include "menus/components/ui_list.h"
#include "menus/components/ui_window.h"
#include "menus/components/ui_textbox.h"
#include "menus/components/ui_image.h"
#include "menus/components/ui_label.h"
#include "menus/components/ui_statistic_card.h"
#include "fonts/bigFont.h"
#include "main.h"
#include "easing.h"
#include "color_channels.h"
#include "mp3_player.h"
#include "graphics.h"
#include "main_menu.h"
#include "level_select.h"
#include "statistics.h"

#include "save/saving.h"

static bool yes_exit = false;

static UIScreen screen;

static UIElement *list;

typedef struct StatisticEntries {
    char *name;
    int *value;
} StatisticEntries;

static const StatisticEntries stats[] = {
    { "Total Attempts", &total_attempts },
    { "Total Jumps", &total_jumps },
    { "Collected Stars", &total_stars }, 
    { "Completed Levels", &completed_main_levels },
    { "Completed Ext. Levels", &completed_external_levels },
    { "Completed Demon Levels", &total_demons },
    { "Collected Secret Coins", &total_coins },
    { "Players Destroyed", &players_destroyed }

};

#define NUM_STATS_ENTRIES (sizeof(stats) / sizeof(StatisticEntries))

UIElement entries[NUM_STATS_ENTRIES];

void exit_statistics(UIElement* e) {
    yes_exit = true;
}

static UIAction actions[] = {
    { "exit", exit_statistics },
};

void statistics_init() {
    ui_load_screen(&screen, actions, sizeof(actions) / sizeof(actions[0]), "romfs:/menus/statistics.txt");

    list = ui_get_element_by_tag(&screen, "list");

    if (list) {
        for (int i = 0; i < NUM_STATS_ENTRIES; i++) {
            char *name = stats[i].name;
            int value = *stats[i].value;

            entries[i] = ui_create_statistic_card(0, 0, i & 1, name, value, NULL);
            ui_list_add(list, &entries[i]);
        }
    }

    yes_exit = false;
}

int statistics_loop() {
    if (yes_exit) {
        return true;
    }

    UIInput touch;
    touchPosition touchPos;
    hidTouchRead(&touchPos);
    touch.touchPosition = touchPos;
    touch.did_something = false;
    touch.interacted = false;
    ui_screen_update(&screen, &touch);

    ui_screen_draw(&screen);

    return false;
}
