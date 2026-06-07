#include <3ds.h>
#include <citro2d.h>

#include "level_complete.h"

#include "menus/components/ui_element.h"
#include "menus/components/ui_screen.h"
#include "math_helpers.h"
#include "menus/components/ui_list.h"
#include "menus/components/ui_window.h"
#include "menus/components/ui_textbox.h"
#include "menus/components/ui_image.h"
#include "menus/components/ui_label.h"
#include "fonts/bigFont.h"
#include "fonts/chatFont.h"
#include "fonts/goldFont.h"
#include "main.h"
#include "easing.h"
#include "color_channels.h"
#include "mp3_player.h"
#include "graphics.h"
#include "main_menu.h"
#include "level_select.h"
#include "info_card.h"
#include "state.h"
#include "endwall.h"

#include "menus/settings.h"

#include "save/saving.h"

#define ANIM_DURATION 1.f
#define RESTART_ANIM_DURATION 0.5f

static bool yes_exit = false;
static bool restart = false;
static bool init = false;

static bool animating_down = false;
static bool animating_up = false;

static float anim_time = 0;

static float up_y_start = 0;
static float window_y_pos = 0;

static UIScreen screen_top;
static UIScreen screen;

static UIElement *attempt_text;
static UIElement *jumps_text;
static UIElement *time_text;

static UIElement *completion_text;

static UIElement *coin_1;
static UIElement *coin_2;
static UIElement *coin_3;

char *practice_completion_text = "Well done... Now try to complete it<p>without any checkpoints!";

char *do_not_completion_texts[] = {
    "Not 1 attempt",
    "That was kinda sloppy",
    "Well done... now beat it in the PC version",
    "Good, now beat it with your eyes closed",
    "!evisserpmI",
    "CBF detected, loser!",
    "Hacked. This level is clearly impossible",
    "Noclip Accuracy: 0.01%",
    "Would be better if it was a harder level",
    "Would be better if it was an easier level",
    "You beat this instead of Story Madness...",
    "Auto Safe Mode cheat detected: Using a 3DS",
    "Auto Safe Mode cheat detected:<p>Noclipped through the end wall",
    "I lied, you got 99%",
    "I have no words.... oh wait",
    "we really doing anything now"
};

char *completion_texts[] = {
    "Impressive",
    "Awesome!",
    "Not bad!",
    "Well done!",
    "Challenge Breaker!",
    "Warp Speed!",
    "You are... The One!",
    "How is this possible!?",
    "You beat me...",
    "Reflex Master!",
    "Skillful!",
    "Y u do dis?",
    "Good Job!",
    "Incredible!",
    "I am speechless...",
    "Brilliant!",
};

#define NUM_COMPLETION_TEXTS (sizeof(completion_texts) / sizeof(char *))
#define NUM_DO_NOCOMPLETION_TEXTS (sizeof(do_not_completion_texts) / sizeof(char *))


static void exit_level_complete(UIElement* e) {
    if (!animating_up) {
        play_sfx(&quit_sound, 1);
        yes_exit = true;
        animating_up = true;
        animating_down = false;
        anim_time = 0;
    }
}

static void restart_level(UIElement* e) {
    if (!animating_up) {
        play_sfx(&play_sound, 1);
        animating_up = true;
        animating_down = false;
        anim_time = 0;
        pause_playback_mp3();
    }
}

static UIAction actions[] = {
    { "restart", restart_level },
    { "exit", exit_level_complete },
};

// This runs the initial animation of the menu coming off screen
static void run_start_animation(float delta) {
    float fade_value = easeValue(BOUNCE_OUT, 0, 240, anim_time, ANIM_DURATION, 1.f);
    window_y_pos = -120 + fade_value;
    up_y_start = fade_value;

    ui_set_pos_on_tag(&screen, SCREEN_BOT_WIDTH / 2, window_y_pos, "window");
    ui_set_pos_on_tag(&screen_top, SCREEN_WIDTH / 2, window_y_pos, "window");

    // Animation end
    if (anim_time >= ANIM_DURATION) {
        animating_down = false;
        anim_time = 0;
    }
    anim_time += delta;
}

// This runs the animation that happens when you press "restart"
static void run_end_animation(float delta) {
    float fade_value = easeValue(QUAD_IN, up_y_start, 0, anim_time, RESTART_ANIM_DURATION, 1.f);
    window_y_pos = -120 + fade_value;

    ui_set_pos_on_tag(&screen, SCREEN_BOT_WIDTH / 2, window_y_pos, "window");
    ui_set_pos_on_tag(&screen_top, SCREEN_WIDTH / 2, window_y_pos, "window");
    

    // Animation end
    if (anim_time >= RESTART_ANIM_DURATION) {
        animating_up = false;
        restart = true;
        anim_time = 0;
    }
    anim_time += delta;
}

#define COMPLETION_TEXT_MAX_WIDTH 250.f

void level_complete_init() {
    init = true;
    ui_load_screen(&screen_top, actions, sizeof(actions) / sizeof(actions[0]), "romfs:/menus/level_complete_top.txt");
    ui_load_screen(&screen, actions, sizeof(actions) / sizeof(actions[0]), "romfs:/menus/level_complete.txt");

    state.current_data.time_end = svcGetSystemTick() / (CPU_TICKS_PER_MSEC * 1000);

    char attempts[64];
    char jumps[64];
    char time[64];

    snprintf(attempts, sizeof(attempts), "Attempts: %d", state.current_data.attempts);
    snprintf(jumps, sizeof(jumps), "Jumps: %d", state.current_data.jumps);

    float timer = state.current_data.time_end - state.current_data.time_start;

    int hours   = (int)(timer) / (60 * 60);
    int minutes = (int)(timer / 60) % 60;
    int seconds = (int)(timer) % 60;

    if (hours) {
        snprintf(time, sizeof(time), "Time: %d:%02d:%02d", hours, minutes, seconds);
    } else {
        snprintf(time, sizeof(time), "Time: %02d:%02d", minutes, seconds);
    }

    attempt_text = ui_get_element_by_tag(&screen_top, "attempts");
    if (attempt_text) ui_label_set_text(attempt_text, attempts);

    jumps_text = ui_get_element_by_tag(&screen_top, "jumps");
    if (jumps_text) ui_label_set_text(jumps_text, jumps);

    time_text = ui_get_element_by_tag(&screen_top, "time");
    if (time_text) ui_label_set_text(time_text, time);

    yes_exit = false;
    restart = false;
    animating_down = true;
    animating_up = false;
    anim_time = 0;
    window_y_pos = 0;

    coin_1 = ui_get_element_by_tag(&screen_top, "coin1");
    coin_2 = ui_get_element_by_tag(&screen_top, "coin2");
    coin_3 = ui_get_element_by_tag(&screen_top, "coin3");

    // Set completion text
    completion_text = ui_get_element_by_tag(&screen_top, "funnytext");
    
    if(state.custom_level == true || state.practice_mode || cheated) {
        ui_run_func_on_tag(&screen_top, "coin1", ui_disable_element);
        ui_run_func_on_tag(&screen_top, "coin2", ui_disable_element);
        ui_run_func_on_tag(&screen_top, "coin3", ui_disable_element);
    
        int start_index = 0;

        // Skip the "Not 1 attempt" line
        if (doNot && state.current_data.attempts == 1) start_index++;

        int text_index = random_int(start_index, (doNot ? NUM_DO_NOCOMPLETION_TEXTS : NUM_COMPLETION_TEXTS)- 1);

        char *text = (doNot ? do_not_completion_texts[text_index] : completion_texts[text_index]);

        char tmp[512];

        if (state.practice_mode) {
            text = practice_completion_text;
        }
        
        if (cheated) {
            char enabled_cheats[256] = "";
            size_t len = 0;
            
            for (int i = 0; i < CHEAT_COUNT; i++) {
                if (cheats_used[i]) {
                    if (len > 0) {
                        len += snprintf(enabled_cheats + len,
                                        sizeof(enabled_cheats) - len,
                                        ", ");
                    }

                    len += snprintf(enabled_cheats + len,
                                    sizeof(enabled_cheats) - len,
                                    "%s",
                                    cheat_names[i]);
                }
            }
            snprintf(tmp, sizeof(tmp), "Safe mode - %s", enabled_cheats);
            text = tmp;
        }
        
        ui_label_set_text(completion_text, text);

        float text_scale;
        float scale = completion_text->label.scale;

        // Get text length in pixels
        float length = get_longest_line_length(&bigFont_fontCharset, scale, text);
    
        if (COMPLETION_TEXT_MAX_WIDTH < length) {
            text_scale = scale * (COMPLETION_TEXT_MAX_WIDTH / length);
        } else {
            text_scale = scale;
        }

        completion_text->label.scale = text_scale;
    } else {
        ui_run_func_on_tag(&screen_top, "funnytext", ui_disable_element);
        
        LevelData *level_data_sel = (state.custom_level ? &level_data : &main_level_data[curr_level_id]);

        ui_image_set_image(coin_1, (state.current_data.coin1 | level_data_sel->coin1 ? COMPLETE_COIN_FILLED_ID : COMPLETE_COIN_UNFILLED_ID), 1);
        ui_image_set_image(coin_2, (state.current_data.coin2 | level_data_sel->coin2 ? COMPLETE_COIN_FILLED_ID : COMPLETE_COIN_UNFILLED_ID), 1);
        ui_image_set_image(coin_3, (state.current_data.coin3 | level_data_sel->coin3 ? COMPLETE_COIN_FILLED_ID : COMPLETE_COIN_UNFILLED_ID), 1);
    }

    if (state.practice_mode) {
        ui_run_func_on_tag(&screen_top, "levelcomplete", ui_disable_element);

        if (doNot) ui_get_element_by_tag(&screen_top, "practicecomplete")->image.scaleX *= -1.f;
    } else {
        ui_run_func_on_tag(&screen_top, "practicecomplete", ui_disable_element);
        if (doNot) ui_get_element_by_tag(&screen_top, "levelcomplete")->image.scaleX *= -1.f;
    }
}

int level_complete_loop(float delta) {
    if (!init) return 0;

    if (animating_down) run_start_animation(delta);
    if (animating_up) run_end_animation(delta);

    if (yes_exit) {
        return 1;
    }

    if (restart) {
        return 2;
    }

    UIInput touch;
    touchPosition touchPos;
    hidTouchRead(&touchPos);
    touch.touchPosition = touchPos;
    touch.did_something = false;
    touch.interacted = false;
    ui_screen_update(&screen, &touch);
    ui_screen_update(&screen_top, &touch);

    return 0;
}

void level_complete_destroy() {
    init = false;
}

void draw_level_complete() {
    if (init) {
        if (get_fade_status()) {
            level_complete_loop(1.f/60);
        }

        ui_screen_draw(&screen);
    }
}
void draw_level_complete_top() {
    if (init) {
        if (get_fade_status()) {
            level_complete_loop(1.f/60);
        }
        
        ui_screen_draw(&screen_top);
    }
}