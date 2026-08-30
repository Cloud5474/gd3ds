#include "ui_stack.h"
#include "ui_screen.h"
#include "screen_definitions.h"
#include "mp3_player.h"

static UIStack stack = { 0 };

static size_t ui_stack_btm_index() {
    return stack.current_anchor;
}

static size_t ui_stack_top_index() {
    if(stack.active_scenes == 0){
        return 0;
    }
    return stack.current_anchor + stack.active_scenes - 1;
}

static size_t ui_stack_next_index() {
    return stack.current_anchor + stack.active_scenes;
}

static void open_anchor(){
    for(size_t i = ui_stack_btm_index(); i < ui_stack_top_index(); i++){
        UIScene *scene = &stack.scenes[i];
        for(int j = 0; j < 2; j++){
            UIScreen *screen = &scene->screens[j];
            ui_unload_screen(screen);
        }
    }

    size_t index = ui_stack_next_index();
    UIScene *scene = &stack.scenes[index];

    UIScreen *top = &scene->screens[SCREEN_TOP];
    UIScreen *btm = &scene->screens[SCREEN_BOTTOM];

    ui_load_screen(top);
    ui_load_screen(btm);

    stack.active_scenes = 1;
    stack.current_anchor = index;
}

static void close_anchor(){
    for(size_t i = ui_stack_btm_index(); i < ui_stack_top_index(); i++){
        for(int j = 0; j < 2; j++){
            UIScreen *screen = &stack.scenes[i].screens[j];
            ui_unload_screen(screen);
        }
    }

    //find previous anchor
    size_t new_anchor = 0;
    for(int i = stack.current_anchor - 1; i >= 0 ; i--){
        if(stack.scenes[i].anchor){
            new_anchor = i;
            break;
        }
    }

    for(size_t i = new_anchor; i < stack.current_anchor; i++){
        UIScene *scene = &stack.scenes[i];
        for(int j = 0; j < 2; j++){
            UIScreen *screen = &scene->screens[j];
            ui_load_screen(screen);
        }
    }

    stack.active_scenes = stack.current_anchor - new_anchor;

    stack.current_anchor = new_anchor;
}

void ui_stack_update(UIInput *input){
    if(stack.active_scenes == 0) return;

    bool close_top = false;

    for(size_t i = ui_stack_btm_index(); i <= ui_stack_top_index(); i++){
        UIScene *scene = &stack.scenes[i];

        bool updating_topmost = i == ui_stack_top_index() && stack.transition == UI_TRANSITION_NONE;
        int top_closing = 0;
    
        for(int j = 0; j < 2; j++) {
            UIInput dummy_input = { 0 };
            //only update top screen with input
            UIInput *input_to_use = updating_topmost ? input : &dummy_input;

            UIScreen *screen = &scene->screens[j];
            if(screen && screen->loaded){
                ui_screen_update_transition(screen, delta);

                if(screen->closing){
                    if(updating_topmost) top_closing++;
                    continue;
                }
    
                ui_screen_update(screen, input_to_use);
            }
        }

        if(top_closing >= 2){
            close_top = true;
        }
    }

    if(close_top){
        UIScene *top_scene = &stack.scenes[ui_stack_top_index()];

        UIScreen *top = &top_scene->screens[SCREEN_TOP];
        UIScreen *btm = &top_scene->screens[SCREEN_BOTTOM];

        ui_unload_screen(top);
        ui_unload_screen(btm);

        stack.active_scenes--;
    }

    bool transition_switch = false;
    if(stack.fade_time >= 1.f){
        switch(stack.fade){
            case FADE_STATUS_IN:
                stack.fade_time = 0.f;
                stack.fade = FADE_STATUS_OUT;
                transition_switch = true;
                break;
            case FADE_STATUS_OUT:
                stack.fade_time = 0.f;
                stack.fade = FADE_STATUS_NONE;
                stack.transition = UI_TRANSITION_NONE;
                break;
            case FADE_STATUS_NONE:
                stack.fade_time = 0.f;
                stack.transition = UI_TRANSITION_NONE;
                break;
        }
    }

    if(transition_switch) {
        switch(stack.transition){
            case UI_TRANSITION_OPENING:
                open_anchor();
                break;
            case UI_TRANSITION_CLOSING:
                close_anchor();
                break;
            case UI_TRANSITION_NONE:
                break;
        }
    }

    stack.fade_time += delta;
}
void ui_stack_draw(Screens target){
    if(stack.active_scenes == 0) return;

    float fade = 0.f;
    if(stack.fade == FADE_STATUS_IN){
        fade = stack.fade_time;
    } else if(stack.fade == FADE_STATUS_OUT){
        fade = 1.f - stack.fade_time;
    }
    C2D_Fade(C2D_Color32f(0.f, 0.f, 0.f, fade));

    for(size_t i = ui_stack_btm_index(); i <= ui_stack_top_index(); i++){
        UIScreen *screen = &stack.scenes[i].screens[target];
        if(screen && screen->loaded){
            ui_screen_draw(screen);
        }
    }
}

//initiates the fade to black transition
void ui_stack_push_anchor(const UIScreenDefPair* defs, bool instant){
    if(stack.transition != UI_TRANSITION_NONE) {
        printf("Cannot push to stack during transition!");
        return;
    }

    UIScene *scene = &stack.scenes[stack.current_anchor + stack.active_scenes];

    UIScreen *top = &scene->screens[SCREEN_TOP];
    UIScreen *btm = &scene->screens[SCREEN_BOTTOM];

    top->def = &defs->top;
    btm->def = &defs->btm;

    scene->anchor = true;

    if(instant){
        open_anchor();
    } else{
        stack.transition = UI_TRANSITION_OPENING;
        stack.fade = FADE_STATUS_IN;
        stack.fade_time = 0.f;
    }
}

void ui_stack_push(
    const UIScreenDefPair* defs, 
    UIAnimation top_anim, 
    UIAnimation btm_anim
){
    if(stack.transition != UI_TRANSITION_NONE) {
        printf("Cannot push to stack during transition!");
        return;
    }

    UIScene *scene = &stack.scenes[stack.current_anchor + stack.active_scenes];

    UIScreen *top = &scene->screens[SCREEN_TOP];
    UIScreen *btm = &scene->screens[SCREEN_BOTTOM];

    btm->isBottom = true;

    top->def = &defs->top;
    btm->def = &defs->btm;
    
    stack.active_scenes++;

    ui_load_screen(top);
    ui_load_screen(btm);

    ui_screen_open(top, top_anim);
    ui_screen_open(btm, btm_anim);
}

void ui_stack_pop(){
    if(stack.transition != UI_TRANSITION_NONE) {
        printf("Cannot pop stack during transition!");
        return;
    }

    UIScene *scene = &stack.scenes[stack.current_anchor + stack.active_scenes - 1];
    if(scene->anchor){
       stack.transition = UI_TRANSITION_CLOSING;
       stack.fade = FADE_STATUS_IN;
       stack.fade_time = 0.f;
    } else{
        for(int i = 0; i < 2; i++){
            scene->screens[i].closing = true;
        }
    }
}

void test_loop(){
    play_mp3("romfs:/songs/SogLoop.mp3", true, 0);

    ui_stack_push_anchor(&soggy_def, true);

    while (aptMainLoop()) {
        hidScanInput();

        UIInput touch;
        touchPosition touchPos;
        hidTouchRead(&touchPos);
        touch.touchPosition = touchPos;
        touch.did_something = false;
        touch.interacted = false;
        touch.down = hidKeysDown();
        touch.held = hidKeysHeld();
        touch.up = hidKeysUp();
        hidCircleRead(&touch.cpad);

        if (touch.down & KEY_SELECT) {
            game_state = STATE_EXIT;
            stop_mp3();
            break; // break in order to return to hbmenu
        }

        if(touch.down & KEY_A){
            ui_stack_push_anchor(&soggy_def, false);
        }
        if(touch.down & KEY_B){
            ui_stack_push(&soggy_def, ANIM_ZOOM, ANIM_ZOOM);
        }

        ui_stack_update(&touch);
        
        // Frees a render target, so keep it out of the frame below
        update_stereo_target();

        update_touch_effect(DT);
        
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        
        // Bottom screen
        C2D_TargetClear(bot, C2D_Color32(0, 0, 0, 255));
        C2D_SceneBegin(bot);
        draw_fade();

        ui_stack_draw(SCREEN_BOTTOM);

        change_blending(true);
        draw_touch_effect();
        change_blending(false);

        // Top screen, drawn once per eye when 3D is on
        for (int eye = 0; begin_top_eye(eye); eye++) {
            begin_eye_layer(DEPTH_UI);
            ui_stack_draw(SCREEN_TOP);
            end_eye_layer();
        }
        C2D_ViewReset();
        C3D_FrameEnd(0);
    }
    C2D_TargetClear(bot, C2D_Color32(0, 0, 0, 255));
}