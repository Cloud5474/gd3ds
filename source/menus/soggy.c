#include <3ds.h>
#include <citro2d.h>
#include "menus/components/ui_element.h"
#include "menus/components/ui_screen.h"
#include "math_helpers.h"
#include "menus/components/ui_list.h"
#include "menus/components/ui_window.h"
#include "menus/components/ui_textbox.h"
#include "menus/components/ui_image.h"
#include "menus/components/ui_progress_bar.h"
#include "menus/components/ui_label.h"
#include "menus/components/ui_external_level_card.h"
#include "fonts/bigFont.h"
#include "main.h"
#include "easing.h"
#include "color_channels.h"
#include "mp3_player.h"
#include "graphics.h"
#include "main_menu.h"
#include "level_select.h"
#include "state.h"

#include "settings.h"
#include "saved_levels.h"
#include "external_levels.h"
#include "search_menu.h"
#include "creator_menu.h"

#include "gameplay.h"

#include "save/config.h"
#include "utils/folders.h"
#include "level_loading.h"

static bool exit_flag = false;

bool gotSogged = false;

static UIScreen screen;
static UIScreen screen_top;

static void action_exit(UIElement *e) {
    exit_flag = true;
    set_fade_status(FADE_STATUS_OUT);
}

static void action_boop(UIElement *e) {
    play_sfx(&honk, 1);
}

static UIAction actions[] = {
    {"exit", action_exit },
    {"boop", action_boop},
};

void soggy_menu_loop() {
    exit_flag = false;
    gotSogged = true;
    cfg_save(); // You got sogged

    ui_load_screen(&screen, actions, sizeof(actions) / sizeof(actions[0]), "romfs:/menus/soggy.txt");
    ui_load_screen(&screen_top, actions, sizeof(actions) / sizeof(actions[0]), "romfs:/menus/soggy_top.txt");

    set_fade_status(FADE_STATUS_IN);

    stop_mp3();
    play_mp3("romfs:/songs/SogLoop.mp3", true, 0);

    while (aptMainLoop()) {
        hidScanInput();

        UIInput touch;
        touchPosition touchPos;
        hidTouchRead(&touchPos);
        touch.touchPosition = touchPos;
        touch.did_something = false;
        touch.interacted = false;

        ui_screen_update(&screen, &touch);
        
        do {
            update_touch_effect(DT);
            
            C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
            
            // Bottom screen
            C2D_TargetClear(bot, C2D_Color32(0, 0, 0, 255));
            C2D_SceneBegin(bot);
            draw_fade();

            ui_screen_draw(&screen);

            change_blending(true);
            draw_touch_effect();
            change_blending(false);

            // Top screen
            C2D_TargetClear(top, C2D_Color32(0, 0, 0, 255));
            C2D_SceneBegin(top);
            draw_fade();

            ui_screen_draw(&screen_top);
            C2D_ViewReset();
            C3D_FrameEnd(0);
        } while (handle_fading());

        if (exit_flag) {
            stop_mp3();
            game_state = STATE_CREATOR_MENU;
            break;
        }
    }
    C2D_TargetClear(bot, C2D_Color32(0, 0, 0, 255));
}
