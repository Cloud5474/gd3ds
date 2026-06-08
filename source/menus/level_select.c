#include <stdlib.h>
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
#include "menus/components/ui_progress_bar.h"
#include "fonts/bigFont.h"
#include "main.h"
#include "easing.h"
#include "color_channels.h"
#include "mp3_player.h"
#include "graphics.h"
#include "level_select.h"

#include "level/main_levels.h"
#include "save/saving.h"

#include "menus/gameplay.h"

#include "state.h"

static UIScreen screen_top;
static UIScreen screen;

static bool start_level = false;
static bool exit_flag = false;

int curr_level_id = 0;

s8 scroll_dir = 0;

float scrolled = 0;

float anim_time = 0;

static bool dragging;
static int dragStartX;
static int lastTouchX;
static int dragDistance;
static int dragDir;

static UIElement *bg_gradient = NULL;
static UIElement *bg_gradient_top = NULL;
static UIElement *level_card_window = NULL;
static UIElement *level_card_title_top = NULL;

static UIElement *level_card_title = NULL;
static UIElement *level_card_stars = NULL;
static UIElement *level_card_face = NULL;

static UIElement *level_card_2_window = NULL;

static UIElement *level_card_2_title = NULL;
static UIElement *level_card_2_stars = NULL;


static UIElement *level_card_normal_progress = NULL;
static UIElement *level_card_normal_progress_val = NULL;
static UIElement *level_card_2_normal_progress = NULL;
static UIElement *level_card_2_normal_progress_val = NULL;

static UIElement *level_card_practice_progress = NULL;
static UIElement *level_card_practice_progress_val = NULL;
static UIElement *level_card_2_practice_progress = NULL;
static UIElement *level_card_2_practice_progress_val = NULL;

static UIElement *level_card_coin_1 = NULL;
static UIElement *level_card_coin_2 = NULL;
static UIElement *level_card_coin_3 = NULL;
static UIElement *level_card_2_coin_1 = NULL;
static UIElement *level_card_2_coin_2 = NULL;
static UIElement *level_card_2_coin_3 = NULL;

#define ANIM_DURATION 0.8f
#define COLOR_FADE_DURATION 0.1f

#define C2D_Color32Const(r, g, b, a) (r | (g << (u32)8) | (b << (u32)16) | (a << (u32)24))

const u32 default_lvl_colors[] = {
    C2D_Color32Const(0, 0, 232, 255),
    C2D_Color32Const(227, 0, 229, 255), 
    C2D_Color32Const(233, 0, 115, 255),
    C2D_Color32Const(233, 0, 0, 255),
    C2D_Color32Const(231, 112, 0, 255),
    C2D_Color32Const(233, 232, 0, 255),
    C2D_Color32Const(0, 231, 0, 255),
    C2D_Color32Const(0, 227, 228, 255),
    C2D_Color32Const(0, 112, 229, 255),
};

#define DOTS_SCALE 0.75f

#define DOTS_X (SCREEN_WIDTH / 2)
#define DOTS_Y 232
#define DOTS_WIDTH 8
#define DOTS_MARGIN 8

static void draw_dots(int level_id) {
    C2D_Sprite spr = { 0 };
    C2D_SpriteFromSheet(&spr, ui_sheet, 421);
    C3D_TexSetFilter(spr.image.tex, GPU_LINEAR, GPU_LINEAR);
    C2D_SpriteSetCenter(&spr, 0.5f, 0.5f);
    C2D_SpriteSetScale(&spr, DOTS_SCALE, DOTS_SCALE);
    
    C2D_ImageTint tint = {0};

    C2D_PlainImageTint(&tint, C2D_Color32(125, 125, 125, 255), 1.f);

    int left_side = DOTS_X - ((DOTS_WIDTH * MAIN_LEVELS_NUM) + (DOTS_MARGIN * (MAIN_LEVELS_NUM - 1))) * DOTS_SCALE / 2;

    for (int i = 0; i < MAIN_LEVELS_NUM; i++) {
        C2D_SpriteSetPos(&spr, left_side + i * (DOTS_WIDTH + DOTS_MARGIN) * DOTS_SCALE, DOTS_Y);
        
        // Do not tint if current level
        if (i == level_id) {
            C2D_DrawSprite(&spr);
        } else {
            C2D_DrawSpriteTinted(&spr, &tint);
        }
    }
}

void update_level_name(int level, int card);
void update_level_stars(int level, int card);

void disable_card_2(UIElement *e) {
    e->enabled = false;
}

void enable_card_2(UIElement *e) {
    e->enabled = true;
}

void level_card_move_right(UIElement *e) {
    e->x += 320;
}

void level_card_move_left(UIElement *e) {
    e->x -= 320;
}

void update_level_progress(int level, int card) {
    if (level < 0) level = MAIN_LEVELS_NUM-1;
    if (level >= MAIN_LEVELS_NUM) level = 0;

    LevelData *data = &main_level_data[level]; 

    char normal[256];
    snprintf(normal, sizeof(normal), "<#ff00ff>Normal</>: %d%%", data->normal_progress);
    
    char practice[256];
    snprintf(practice, sizeof(practice), "<#ffa54b>Practice</>: %d%%", data->practice_progress);

    UIElement *normal_prog = (card) ? level_card_2_normal_progress : level_card_normal_progress;
    UIElement *practice_prog = (card) ? level_card_2_practice_progress : level_card_practice_progress;
    UIElement *normal_progval = (card) ? level_card_2_normal_progress_val : level_card_normal_progress_val;
    UIElement *practice_progval = (card) ? level_card_2_practice_progress_val : level_card_practice_progress_val;

    UIElement *coin_1 = (card) ? level_card_2_coin_1 : level_card_coin_1;
    UIElement *coin_2 = (card) ? level_card_2_coin_2 : level_card_coin_2;
    UIElement *coin_3 = (card) ? level_card_2_coin_3 : level_card_coin_3;

    normal_prog->progress_bar.value = data->normal_progress;
    practice_prog->progress_bar.value = data->practice_progress;

    snprintf(normal, sizeof(normal), "%d%%", data->normal_progress);
    snprintf(practice, sizeof(practice), "%d%%", data->practice_progress);

    ui_label_set_text(normal_progval, normal);
    ui_label_set_text(practice_progval, practice);
    
    ui_image_set_image(coin_1, (data->coin1 ? MENU_COIN_FILLED_ID : MENU_COIN_UNFILLED_ID), 0);
    ui_image_set_image(coin_2, (data->coin2 ? MENU_COIN_FILLED_ID : MENU_COIN_UNFILLED_ID), 0);
    ui_image_set_image(coin_3, (data->coin3 ? MENU_COIN_FILLED_ID : MENU_COIN_UNFILLED_ID), 0);
}

void update_level_name(int level, int card) {
    if (level < 0) level = MAIN_LEVELS_NUM-1;
    if (level >= MAIN_LEVELS_NUM) level = 0;

    UIElement *e = (card) ? level_card_2_title : level_card_title;
    level_card_title_top = ui_get_element_by_tag(&screen_top, "levelname");
    float length = get_text_length(&bigFont_fontCharset, 1 / 0.85f, main_levels[level].level_name);

    float txt_scale;
    if (level_card_window->w < length) {
        txt_scale = (level_card_window->w / length);
    } else {
        txt_scale = 0.85f;
    }

    e->label.scale = txt_scale;

    ui_label_set_text(e, main_levels[level].level_name);
}

void update_level_stars(int level, int card) {
    if (level < 0) level = MAIN_LEVELS_NUM-1;
    if (level >= MAIN_LEVELS_NUM) level = 0;

    UIElement *e = (card) ? level_card_2_stars : level_card_stars;
    char stars[10] = { 0 };
    snprintf(stars, 9, "%d", main_levels[level].stars);
    ui_label_set_text(e, stars);
}

void update_level_face(int level) {
    if (level < 0) level = MAIN_LEVELS_NUM-1;
    if (level >= MAIN_LEVELS_NUM) level = 0;

    ui_image_set_image(level_card_face, 239 + main_levels[level].difficulty, 0);
}
void update_level_top(int level){
    LevelData *data = &main_level_data[level]; 

    char attempts[256];
    snprintf(attempts, sizeof(attempts), "<#40e348>Total Attempts</>: %d", data->attempts);
    
    char jumps[256];
    snprintf(jumps, sizeof(jumps), "<#60abef>Total Jumps</>: %d", data->jumps);

    char normal[256];
    snprintf(normal, sizeof(normal), "<#ff00ff>Normal</>: %d%%", data->normal_progress);
    
    char practice[256];
    snprintf(practice, sizeof(practice), "<#ffa54b>Practice</>: %d%%", data->practice_progress);

    ui_label_set_text(level_card_title_top, main_levels[level].level_name);
    ui_label_set_text(ui_get_element_by_tag(&screen_top, "totalattempts"), attempts);
    ui_label_set_text(ui_get_element_by_tag(&screen_top, "totaljumps"), jumps);
    ui_label_set_text(ui_get_element_by_tag(&screen_top, "normalprogress"), normal);
    ui_label_set_text(ui_get_element_by_tag(&screen_top, "practiceprogress"), practice);
}

void action_open_level(UIElement* e) { 
    play_sfx(&play_sound, 1);
    set_fade_status(FADE_STATUS_OUT);
    start_level = true; 
}

void recenter(){
    update_level_name(curr_level_id, 0);
    update_level_stars(curr_level_id, 0);
    update_level_progress(curr_level_id, 0);
}

void handle_card_movement() {
    if (!dragging) {
        if (anim_time > ANIM_DURATION) {
            update_level_name(curr_level_id, 0);
            update_level_stars(curr_level_id, 0);
            update_level_progress(curr_level_id, 0);

            ui_run_func_on_tag(&screen, "level_card_2", disable_card_2);
            ui_set_pos_on_tag(&screen, 160, LEVEL_CARD_Y_POS, "level_card");
            ui_set_pos_on_tag(&screen, 160, LEVEL_CARD_Y_POS, "level_card_2");
            scroll_dir = 0;
            dragDistance = 0;

            return;
        }

        float start = (scroll_dir == 0) ? dragDistance : dragDistance * scroll_dir;
        float end = (scroll_dir == 0) ? 0 : 320;

        float fade_value = easeValue(ELASTIC_OUT, start, end, anim_time, ANIM_DURATION, 0.6f);
        float value = (scroll_dir == 0) ? 160 + fade_value : 160 + fade_value * scroll_dir;
        anim_time += 0.016666f;

        ui_set_pos_on_tag(&screen, value, LEVEL_CARD_Y_POS, "level_card");
        ui_set_pos_on_tag(&screen, value - 320 * scroll_dir, LEVEL_CARD_Y_POS, "level_card_2");
    }
}

void action_move_right(UIElement* e) { 
    curr_level_id++;
    scroll_dir = -1;
    scrolled = 0;
    anim_time = 0;
    
    if (curr_level_id >= MAIN_LEVELS_NUM) curr_level_id = 0;
    
    ui_set_pos_on_tag(&screen, 160, LEVEL_CARD_Y_POS, "level_card");
    ui_run_func_on_tag(&screen, "level_card_2", enable_card_2);
    ui_run_func_on_tag(&screen, "level_card_2", level_card_move_right);
    
    upload_color_to_buffer(0, default_lvl_colors[curr_level_id % NUM_MENU_COLORS], COLOR_FADE_DURATION);

    update_level_name(curr_level_id - 1, 0);
    update_level_stars(curr_level_id - 1, 0);
    update_level_progress(curr_level_id - 1, 0);
    
    update_level_face(curr_level_id);
    update_level_top(curr_level_id);

    update_level_name(curr_level_id, 1);
    update_level_stars(curr_level_id, 1);
    update_level_progress(curr_level_id, 1);
};

void action_move_left(UIElement* e) { 
    curr_level_id--;
    scroll_dir = 1;
    scrolled = 0;
    anim_time = 0;

    if (curr_level_id < 0) curr_level_id = MAIN_LEVELS_NUM-1;

    ui_set_pos_on_tag(&screen, 160, LEVEL_CARD_Y_POS, "level_card");
    ui_run_func_on_tag(&screen, "level_card_2", enable_card_2);
    ui_run_func_on_tag(&screen, "level_card_2", level_card_move_left);

    upload_color_to_buffer(0, default_lvl_colors[curr_level_id % NUM_MENU_COLORS], COLOR_FADE_DURATION);

    update_level_name(curr_level_id + 1, 0);
    update_level_stars(curr_level_id + 1, 0);
    update_level_progress(curr_level_id + 1, 0);
    
    update_level_face(curr_level_id);
    update_level_top(curr_level_id);

    update_level_name(curr_level_id, 1);
    update_level_stars(curr_level_id, 1);
    update_level_progress(curr_level_id, 1);
}

void lerp_level_colors(u32 color1, u32 color2){
    float frac = abs(dragDistance) / 320.f;
    frac = clampf(frac, 0.f, 1.f);
    u32 lerpCol = color_lerp_u32(color1, color2, frac);
    upload_color_to_buffer(0, lerpCol, 0);
}

void peek_right(){
    ui_run_func_on_tag(&screen, "level_card_2", enable_card_2);

    int card2id = (curr_level_id + 1) % MAIN_LEVELS_NUM;

    update_level_name(card2id, 1);
    update_level_stars(card2id, 1);
    update_level_progress(card2id, 1);

    u32 col1 = default_lvl_colors[curr_level_id % NUM_MENU_COLORS];
    u32 col2 = default_lvl_colors[card2id % NUM_MENU_COLORS];
    lerp_level_colors(col1, col2);
}

void peek_left(){
    ui_run_func_on_tag(&screen, "level_card_2", enable_card_2);

    int card2id = (curr_level_id - 1) % MAIN_LEVELS_NUM;

    update_level_name(card2id, 1);
    update_level_stars(card2id, 1);
    update_level_progress(card2id, 1);

    u32 col1 = default_lvl_colors[curr_level_id % NUM_MENU_COLORS];
    u32 col2 = default_lvl_colors[card2id % NUM_MENU_COLORS];
    lerp_level_colors(col1, col2);
}

void action_exit(UIElement* e) {
    exit_flag = true;
    set_fade_status(FADE_STATUS_OUT);
}

void tint_ground(UIElement *e) {
    ColorChannel channel = channels[0];
    ui_image_set_tint(e, C2D_Color32(channel.color.r, channel.color.g, channel.color.b, 255));
}

static UIAction actions[] = {
    {"open_level", action_open_level},
    {"exit", action_exit},
    {"move_right", action_move_right},
    {"move_left", action_move_left}
};

static UIAction actions_top[] = {

};

int mode = 0;

void level_select_loop() {
    start_level = false;
    exit_flag = false;
    state.custom_level = false;
    ui_load_screen(&screen, actions, sizeof(actions) / sizeof(actions[0]), "romfs:/menus/level_select.txt");
    ui_load_screen(&screen_top, actions_top, sizeof(actions_top) / sizeof(actions_top[0]), "romfs:/menus/level_select_top.txt");

    // Set window color
    level_card_window = ui_get_element_by_tag(&screen, "card_window");
    ui_window_set_tint(level_card_window, C2D_Color32(0, 0, 0, 127));

    level_card_2_window = ui_get_element_by_tag(&screen, "card_window_2");
    ui_window_set_tint(level_card_2_window, C2D_Color32(0, 0, 0, 127));
    
    ui_window_set_tint(ui_get_element_by_tag(&screen_top, "face_card"), C2D_Color32(0, 0, 0, 127));

    // Get level card components
    level_card_title = ui_get_element_by_tag(&screen, "level_title");
    level_card_stars = ui_get_element_by_tag(&screen, "level_stars");
    level_card_face = ui_get_element_by_tag(&screen_top, "level_face");

    level_card_2_title = ui_get_element_by_tag(&screen, "level_title_2");
    level_card_2_stars = ui_get_element_by_tag(&screen, "level_stars_2");

    level_card_normal_progress = ui_get_element_by_tag(&screen, "normalprogress");
    level_card_normal_progress_val = ui_get_element_by_tag(&screen, "normalprogressvalue");
    level_card_2_normal_progress = ui_get_element_by_tag(&screen, "normalprogress_2");
    level_card_2_normal_progress_val = ui_get_element_by_tag(&screen, "normalprogressvalue_2");
    
    level_card_practice_progress = ui_get_element_by_tag(&screen, "practiceprogress");
    level_card_practice_progress_val = ui_get_element_by_tag(&screen, "practiceprogressvalue");
    level_card_2_practice_progress = ui_get_element_by_tag(&screen, "practiceprogress_2");
    level_card_2_practice_progress_val = ui_get_element_by_tag(&screen, "practiceprogressvalue_2");
    
    level_card_coin_1 = ui_get_element_by_tag(&screen, "coin_1");
    level_card_coin_2 = ui_get_element_by_tag(&screen, "coin_2");
    level_card_coin_3 = ui_get_element_by_tag(&screen, "coin_3");
    level_card_2_coin_1 = ui_get_element_by_tag(&screen, "coin_1_2");
    level_card_2_coin_2 = ui_get_element_by_tag(&screen, "coin_2_2");
    level_card_2_coin_3 = ui_get_element_by_tag(&screen, "coin_3_2");

    ui_progress_bar_set_tint(level_card_normal_progress, C2D_Color32(0, 255, 0, 255));
    ui_progress_bar_set_tint(level_card_2_normal_progress, C2D_Color32(0, 255, 0, 255));

    ui_progress_bar_set_tint(level_card_practice_progress, C2D_Color32(0, 255, 255, 255));
    ui_progress_bar_set_tint(level_card_2_practice_progress, C2D_Color32(0, 255, 255, 255));

    update_level_name(curr_level_id, 0);
    update_level_stars(curr_level_id, 0);
    update_level_face(curr_level_id);
    update_level_top(curr_level_id);
    update_level_progress(curr_level_id, 0);
    
    ui_run_func_on_tag(&screen, "level_card_2", disable_card_2);

    u32 color = default_lvl_colors[curr_level_id % NUM_MENU_COLORS];

    channels[0].color.r = GET_R(color);
    channels[0].color.g = GET_G(color);
    channels[0].color.b = GET_B(color);

    scroll_dir = 0;

    set_fade_status(FADE_STATUS_IN);
        
    // Set bg color
    bg_gradient = ui_get_element_by_tag(&screen, "gradient");
    bg_gradient_top = ui_get_element_by_tag(&screen_top, "gradient");

    if (!playing_menu_loop) {
        play_mp3("romfs:/songs/menuLoop.mp3", true, 0);
        playing_menu_loop = true;
    }

    while (aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();
        u32 kHeld = hidKeysHeld();
        u32 kUp = hidKeysUp();

        if (kDown & (KEY_START | KEY_A)) {
            action_open_level(NULL);
        }

        UIInput touch;
        touchPosition touchPos;
        hidTouchRead(&touchPos);
        touch.touchPosition = touchPos;
        touch.did_something = false;
        touch.interacted = false;

        handle_col_channel(0);

        ColorChannel channel = channels[0];

        ui_image_set_tint(bg_gradient, C2D_Color32(channel.color.r, channel.color.g, channel.color.b, 255));
        ui_image_set_tint(bg_gradient_top, C2D_Color32(channel.color.r, channel.color.g, channel.color.b, 255));
        ui_run_func_on_tag(&screen, "ground", tint_ground);

        if((kDown & KEY_TOUCH)){
            dragStartX = touch.touchPosition.px;
            dragDistance = 0;
            dragDir = 0;
            recenter();
        }
        if((kHeld & KEY_TOUCH)){
            if(dragging){
                int delta = touch.touchPosition.px - lastTouchX;
                lastTouchX = touch.touchPosition.px;
                dragDistance += delta;

                if(dragDistance > 20){
                    dragDir = -1;
                    peek_left();
                } else if(dragDistance < -20){
                    dragDir = 1;
                    peek_right();
                } else{
                    dragDir = 0;
                    ui_run_func_on_tag(&screen, "level_card_2", disable_card_2);
                }

                ui_set_pos_on_tag(&screen, 160 + dragDistance, LEVEL_CARD_Y_POS, "level_card");
                ui_set_pos_on_tag(&screen, 160 + dragDistance + dragDir * 320, LEVEL_CARD_Y_POS, "level_card_2");
            } else{
                int dragXDiff = touch.touchPosition.px - dragStartX;
                if(abs(dragXDiff) > 10){
                    dragging = true;
                }
                lastTouchX = touch.touchPosition.px;
            }
        }
        if((kUp & KEY_TOUCH)){
            if(dragging){
                if(dragDir == -1){
                    action_move_left(NULL);
                } else if(dragDir == 1){
                    action_move_right(NULL);
                } else{
                    scroll_dir = 0;
                    anim_time = 0.f;
                }
            }
            dragging = false;
            dragDir = 0;
        }

        handle_card_movement();

        ui_screen_update(&screen, &touch);
        ui_screen_update(&screen_top, &touch);
        
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

            ui_screen_draw(&screen_top);
            draw_dots(curr_level_id);

            C2D_ViewReset();
            C3D_FrameEnd(0);
        } while (handle_fading());

        if (start_level) {
            stop_mp3();
            game_state = STATE_GAME;
            playing_menu_loop = false;
            break;
        }

        if (exit_flag) {
            game_state = STATE_MAIN_MENU;
            break;
        }
    }
    C2D_TargetClear(bot, C2D_Color32(0, 0, 0, 255));
}
