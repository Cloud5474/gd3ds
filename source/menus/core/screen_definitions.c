#include "screen_definitions.h"

#include "menus/clear_search_filters.h"
#include "menus/creator_menu.h"
#include "menus/online_menu.h"
#include "menus/credits.h"
#include "menus/delete_online_level.h"
#include "menus/external_level_infobox.h"
#include "menus/external_levels.h"
#include "menus/external_popup.h"
#include "menus/first_boot_disclaimer.h"
#include "menus/gameplay.h"
#include "menus/generic_disclaimer.h"
#include "menus/how_to_play.h"
#include "menus/icon_kit.h"
#include "menus/info_card.h"
#include "menus/length_filter.h"
#include "menus/level_complete.h"
#include "menus/level_select.h"
#include "menus/loading_screen.h"
#include "menus/main_menu.h"
#include "menus/online_level_comments.h"
#include "menus/online_level_infobox.h"
#include "menus/online_level_menu.h"
#include "menus/palette_kit.h"
#include "menus/refresh_online_level.h"
#include "menus/saved_levels.h"
#include "menus/search_filters.h"
#include "menus/server_switcher.h"
#include "menus/search_menu.h"
#include "menus/settings_hub.h"
#include "menus/settings.h"
#include "menus/soggy.h"
#include "menus/song_filter.h"
#include "menus/songs.h"
#include "menus/statistics.h"

const UIScreenDefPair *defs[] = {
    &main_menu_def,
    &icon_kit_def,
    &creator_menu_def,
    &palette_kit_def,
    &credits_def,
    &soggy_def,
    &search_menu_def,
    &external_def,
    &settings_hub_def,
    &songs_def,
    &statistics_def,
    &how_to_play_def,
    &settings_def,
    &online_def,
    &saved_levels_def,
    &song_filter_def,
    &length_filter_def,
    &search_filters_def,
    &server_switcher_def,
    &delete_level_def,
    &clear_filters_def,
};

const UIScreenDefPair *ui_get_screen_def(const char* name){
    for(size_t i = 0; i < ARRAY_LEN(defs); i++){
        if(strcmp(name, defs[i]->name) == 0){
            return defs[i];
        }
    }
    return &soggy_def;
}