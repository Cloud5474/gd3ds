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
#include "menus/components/ui_button.h"
#include "fonts/bigFont.h"
#include "main.h"
#include "easing.h"
#include "color_channels.h"
#include "mp3_player.h"
#include "graphics.h"
#include "main_menu.h"
#include "level_select.h"
#include "state.h"
#include "endwall.h"
#include "menus/components/ui_darken.h"
#include "menus/components/ui_use_effect.h"
#include "particles/circles.h"

#include "settings.h"
#include "generic_disclaimer.h"

#include "gameplay.h"

#include "save/config.h"
#include "info_card.h"

#include "practice.h"

#include "save/saving.h"

bool game_paused = false;
bool in_level_complete = false;
static bool in_disclaimer = false;
static bool in_settings = false;

static UIScreen screen;
static UIScreen screen_top;
static UIElement *bg_gradient;
static UIElement *progress_bar;
static UIElement *percent;
static UIElement *level_name;

static UIElement *normal_progress;
static UIElement *normal_progress_val;
static UIElement *practice_progress;
static UIElement *practice_progress_val;

static UIElement *coin_1;
static UIElement *coin_2;
static UIElement *coin_3;

static UIElement *coins_full[3];
static bool coins_got[3];
static float coin_anims[3];
static bool coins_circles_spawned[3];

static UIElement *coin_1_top;
static UIElement *coin_2_top;
static UIElement *coin_3_top;

int decimal;

static void update_progress_bars() {
    LevelData *level_data_sel = (state.custom_level ? &level_data : &main_level_data[curr_level_id]);

    normal_progress->progress_bar.value = level_data_sel->normal_progress;
    practice_progress->progress_bar.value = level_data_sel->practice_progress;

    char normal[32];
    char practice[32];
    snprintf(normal, sizeof(normal), "%d%%", level_data_sel->normal_progress);
    snprintf(practice, sizeof(practice), "%d%%", level_data_sel->practice_progress);

    ui_label_set_text(normal_progress_val, normal);
    ui_label_set_text(practice_progress_val, practice);

    ui_image_set_image(coin_1_top, (level_data_sel->coin1 ? PAUSE_COIN_FILLED_ID : PAUSE_COIN_UNFILLED_ID), 0);
    ui_image_set_image(coin_2_top, (level_data_sel->coin2 ? PAUSE_COIN_FILLED_ID : PAUSE_COIN_UNFILLED_ID), 0);
    ui_image_set_image(coin_3_top, (level_data_sel->coin3 ? PAUSE_COIN_FILLED_ID : PAUSE_COIN_UNFILLED_ID), 0);
}

static void reset_coin(LevelData *level_data_sel, int i){
    if((!level_data_sel->coin1 && i == 0)
        || (!level_data_sel->coin2 && i == 1)
        || (!level_data_sel->coin3 && i == 2)) { 
            coins_full[i]->enabled = false;
        } else {
            coins_full[i]->enabled = true;
    }
}

void reset_coins(){
    LevelData *level_data_sel = (state.custom_level ? &level_data : &main_level_data[curr_level_id]);

    ui_use_effect_clear(ui_get_element_by_tag(&screen, "coin_circle"));

    for(int i = 0; i < 3; i++){
        reset_coin(level_data_sel, i);
        coins_got[i] = false;
        coin_anims[i] = 0.f;
        coins_circles_spawned[i] = false;
    }
}

void pause_game() {
    if (state.end_wall_anim_playing) return;

    update_progress_bars();
    game_paused = true;
    if (song_loaded || state.practice_mode) pause_playback_mp3();
    if (!state.custom_level){
        ui_run_func_on_tag(&screen_top, "coin_1", ui_enable_element);
        ui_run_func_on_tag(&screen_top, "coin_2", ui_enable_element);
        ui_run_func_on_tag(&screen_top, "coin_3", ui_enable_element);
        ui_run_func_on_tag(&screen, "coin_1", ui_disable_element);
        ui_run_func_on_tag(&screen, "coin_2", ui_disable_element);
        ui_run_func_on_tag(&screen, "coin_3", ui_disable_element);
        ui_run_func_on_tag(&screen, "coin_1_full", ui_disable_element);
        ui_run_func_on_tag(&screen, "coin_2_full", ui_disable_element);
        ui_run_func_on_tag(&screen, "coin_3_full", ui_disable_element);
        ui_run_func_on_tag(&screen, "coin_circle", ui_disable_element);
    }
    ui_run_func_on_tag(&screen_top, "pause_menu", ui_enable_element);
    ui_run_func_on_tag(&screen, "paused", ui_enable_element);
    ui_run_func_on_tag(&screen, "not_paused", ui_disable_element);
    in_settings = false;
}

void unpause_game() {
    game_paused = false;
    if (state.death_timer <= 0 && (song_loaded || state.practice_mode)) {
        unpause_playback_mp3();
    }
    if (!state.custom_level){
        ui_run_func_on_tag(&screen_top, "coin_1", ui_disable_element);
        ui_run_func_on_tag(&screen_top, "coin_2", ui_disable_element);
        ui_run_func_on_tag(&screen_top, "coin_3", ui_disable_element);
        ui_run_func_on_tag(&screen, "coin_1", ui_enable_element);
        ui_run_func_on_tag(&screen, "coin_2", ui_enable_element);
        ui_run_func_on_tag(&screen, "coin_3", ui_enable_element);
        for(int i = 0; i < 3; i++){
            reset_coin((state.custom_level ? &level_data : &main_level_data[curr_level_id]), i);
        }
        ui_run_func_on_tag(&screen, "coin_circle", ui_enable_element);
    }
    ui_run_func_on_tag(&screen_top, "pause_menu", ui_disable_element);
    ui_run_func_on_tag(&screen, "paused", ui_disable_element);
    ui_run_func_on_tag(&screen, "not_paused", ui_enable_element);
    in_settings = false;
}

static void exit_level() {
    if (!exiting_level && game_paused){
        play_sfx(&quit_sound, 1);
        exiting_level = true;
        set_fade_status(FADE_STATUS_OUT);
    }
}

static void restart_level() {
    init_variables();
    reload_level();
    if (state.practice_mode) {
        if (checkpoint_count > 0) {
            restore_checkpoint();
        }
    } else if (song_loaded) seek_mp3(level_info.song_offset);

    reset_coins();

    ui_get_element_by_tag(&screen, "endDarken")->opacity = 0.f;

    unpause_game();
}

void open_settings() {
    in_settings = true;
    settings_init();
}

static void action_pause(UIElement *e) { 
    pause_game();
}

static void action_unpause(UIElement *e) {
    unpause_game();
}

static void action_exit(UIElement *e) {
    exit_level();
}

static void action_restart(UIElement *e) {
    restart_level();
}

static void action_open_settings(UIElement *e) {
    open_settings();
}

static void action_practice_mode(UIElement *e) {
    if (!state.practice_mode) {
        start_practice_mode();
    } else {
        exit_practice_mode();
    }

    action_unpause(e);
}

static void action_add_checkpoint(UIElement *e) {
    new_checkpoint();
}
static void action_remove_checkpoint(UIElement *e) {
    delete_last_checkpoint();
}

static UIAction actions[] = {
    {"pause", action_pause },
    {"unpause", action_unpause },
    {"exit", action_exit },
    {"restart", action_restart },
    {"settings", action_open_settings },
    {"practice", action_practice_mode },
    {"add_check", action_add_checkpoint },
    {"remove_check", action_remove_checkpoint },
};

void gameplay_screen_init() {
    ui_load_screen(&screen, actions, sizeof(actions) / sizeof(actions[0]), "romfs:/menus/gameplay.txt");
    bg_gradient = ui_get_element_by_tag(&screen, "gradient");

    ui_load_screen(&screen_top, actions, sizeof(actions) / sizeof(actions[0]), "romfs:/menus/gameplay_top.txt");;
    progress_bar = ui_get_element_by_tag(&screen_top, "progressalert");
    percent = ui_get_element_by_tag(&screen_top, "percent");
    level_name = ui_get_element_by_tag(&screen_top, "level_title");

    Color color = get_white_if_black(p1_color);

    ui_progress_bar_set_tint(progress_bar, C2D_Color32(color.r, color.g, color.b, 255));
    
    ui_window_set_tint(ui_get_element_by_tag(&screen_top, "bgwindow"), C2D_Color32(0, 0, 0, 127));

    ui_label_set_text(level_name, level_info.level_name);

    normal_progress = ui_get_element_by_tag(&screen_top, "normalprogress");
    normal_progress_val = ui_get_element_by_tag(&screen_top, "normalprogressvalue");
    practice_progress = ui_get_element_by_tag(&screen_top, "practiceprogress");
    practice_progress_val = ui_get_element_by_tag(&screen_top, "practiceprogressvalue");
    
    ui_progress_bar_set_tint(normal_progress, C2D_Color32(0, 255, 0, 255));
    ui_progress_bar_set_tint(practice_progress, C2D_Color32(0, 255, 255, 255));

    // hide coins if level is a custom level
    if(state.custom_level == true){
        ui_run_func_on_tag(&screen, "coin_1", ui_disable_element);
        ui_run_func_on_tag(&screen, "coin_2", ui_disable_element);
        ui_run_func_on_tag(&screen, "coin_3", ui_disable_element);
    }
    
    // Hide pause menu
    ui_run_func_on_tag(&screen_top, "coin_1", ui_disable_element);
    ui_run_func_on_tag(&screen_top, "coin_2", ui_disable_element);
    ui_run_func_on_tag(&screen_top, "coin_3", ui_disable_element);
    ui_run_func_on_tag(&screen_top, "pause_menu", ui_disable_element);
    ui_run_func_on_tag(&screen, "paused", ui_disable_element);
    ui_run_func_on_tag(&screen, "practice_buttons", ui_disable_element);

    coin_1 = ui_get_element_by_tag(&screen, "coin_1");
    coin_2 = ui_get_element_by_tag(&screen, "coin_2");
    coin_3 = ui_get_element_by_tag(&screen, "coin_3");

    coins_full[0] = ui_get_element_by_tag(&screen, "coin_1_full");
    coins_full[1] = ui_get_element_by_tag(&screen, "coin_2_full");
    coins_full[2] = ui_get_element_by_tag(&screen, "coin_3_full");
    
    reset_coins();

    coin_1_top = ui_get_element_by_tag(&screen_top, "coin_1");
    coin_2_top = ui_get_element_by_tag(&screen_top, "coin_2");
    coin_3_top = ui_get_element_by_tag(&screen_top, "coin_3");

    ui_get_element_by_tag(&screen, "endDarken")->opacity = 0.f;
}

int gameplay_screen_top_loop() { 
    UIInput touch;
    touchPosition touchPos;
    hidTouchRead(&touchPos);

    decimal = 0;
    if (decimalPercent) decimal = 2;
    if (ultraDecimalPercent) decimal = MAX_DECIMAL_PERCENT;

    progress_bar->progress_bar.value = state.level_progress;
    snprintf(percent->label.text, 32, "%.*f%%", decimal, state.level_progress);

    ui_run_func_on_tag(&screen_top, "progressalert", ui_disable_element);
    ui_run_func_on_tag(&screen_top, "percent", ui_disable_element);
    ui_set_pos_on_tag(&screen_top, 200, 11, "percent");
    percent->label.alignment = 0.5;

    if (showProgressBar) {
        ui_run_func_on_tag(&screen_top, "progressalert", ui_enable_element);
        ui_set_pos_on_tag(&screen_top, 282, 11, "percent");
        percent->label.alignment = 0;
    }

    if (showProgressPercent) {
        ui_run_func_on_tag(&screen_top, "percent", ui_enable_element);
    }

    ui_screen_update(&screen_top, &touch);
    ui_screen_draw(&screen_top);

    return false;
}

int gameplay_screen_bot_loop() {
    UIInput touch;
    touchPosition touchPos;
    hidTouchRead(&touchPos);

    ColorChannel channel = channels[get_col_channel_index(CHANNEL_BG)];
    // If flash is happening, use lbg
    if (state.flash_data.use_lbg) channel = channels[get_col_channel_index(CHANNEL_LBG_NOLERP)];
    
    Color color = channel.color;

    ui_image_set_tint(bg_gradient, C2D_Color32(color.r, color.g, color.b, 255));

    LevelData *level_data_sel = (state.custom_level ? &level_data : &main_level_data[curr_level_id]);

    coins_got[0] = state.current_data.coin1 && !level_data_sel->coin1;
    coins_got[1] = state.current_data.coin2 && !level_data_sel->coin2;
    coins_got[2] = state.current_data.coin3 && !level_data_sel->coin3;

    for(int i = 0; i < 3; i++){
        if(coins_got[i] && !game_paused){
            float scale = easeValue(BOUNCE_OUT, 2.f, 0.65f, coin_anims[i], 0.35f, 1.f);
            float opacity = easeValue(EASE_LINEAR, 0.f, 1.f, coin_anims[i], 0.1f, 1.f);

            coins_full[i]->enabled = true;
            coins_full[i]->image.scaleX = scale;
            coins_full[i]->image.scaleY = scale;
            ui_image_set_tint(coins_full[i], C2D_Color32f(1, 1, 1, opacity));

            if(coin_anims[i] >= 0.05f){
                if(!coins_circles_spawned[i]){
                    ui_set_use_effect_col(
                        ui_add_use_effect(
                            ui_get_element_by_tag(&screen, "coin_circle"), 
                        coins_full[i]->x, coins_full[i]->y, &end_wall_firework_circle),
                    1.f, 0.75f, 0.f);
                    
                    coins_circles_spawned[i] = true;
                }
            }

            coin_anims[i] += delta;
        }
    }

    //level end buttons retraction animation
    float cutscene_timer = state.player.cutscene_timer;
    float complete_y_offset = easeValue(EASE_IN, 20.f, -20.f, cutscene_timer, 0.3f, 1.3f);
    float complete_y_offset_practice = easeValue(EASE_IN, 200.f, 270.f, cutscene_timer, 0.3f, 1.8f);

    coin_1->y = complete_y_offset;
    coin_2->y = complete_y_offset;
    coin_3->y = complete_y_offset;
    coins_full[0]->y = complete_y_offset;
    coins_full[1]->y = complete_y_offset;
    coins_full[2]->y = complete_y_offset;

    UIElement *pause_btn = ui_get_element_by_tag(&screen, "pause_btn");
    pause_btn->y = complete_y_offset;
    
    UIElement *add_checkpoint = ui_get_element_by_tag(&screen, "add_checkpoint");
    UIElement *remove_checkpoint = ui_get_element_by_tag(&screen, "remove_checkpoint");
    add_checkpoint->y = complete_y_offset_practice;
    remove_checkpoint->y = complete_y_offset_practice;

    if (state.practice_mode) {
        ui_button_set_image(ui_get_element_by_tag(&screen, "practice_mode"), 124, 0);
    } else {
        ui_run_func_on_tag(&screen, "practice_buttons", ui_disable_element);
        ui_button_set_image(ui_get_element_by_tag(&screen, "practice_mode"), 146, 0);
    }

    touch.touchPosition = touchPos;
    touch.did_something = false;
    touch.interacted = false;
    if (!in_settings && !in_disclaimer && !in_info_card) {
        ui_screen_update(&screen, &touch);
    }

    ui_screen_draw(&screen);

    if (in_settings) {
        int returned = settings_loop();
        if (returned) {
            in_settings = false;
        }
    }

    if (in_disclaimer) {
        int returned = disclaimer_loop();
        if (returned) {
            in_disclaimer = false;
        }
    }

    if (in_info_card) {
        int returned = info_card_loop();
        if (returned) {
            in_info_card = false;
        }
    }

    return false;
}
