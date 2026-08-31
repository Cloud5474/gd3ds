#include "ui_stack.h"
#include "ui_screen.h"
#include "screen_definitions.h"
#include "mp3_player.h"
#include "fonts/bigFont.h"

static UIStack stack = { 0 };

static size_t ui_stack_min_index() {
    return stack.current_anchor;
}

static size_t ui_stack_max_index() {
    if(stack.active_scene_count == 0){
        return 0;
    }
    return stack.current_anchor + stack.active_scene_count - 1;
}

static size_t ui_stack_next_index() {
    return stack.current_anchor + stack.active_scene_count;
}

static void open_anchor(){
    for(size_t i = ui_stack_min_index(); i <= ui_stack_max_index(); i++){
        UIScene *scene = &stack.scenes[i];
        for(int j = 0; j < 2; j++){
            UIScreen *screen = &scene->screens[j];
            ui_unload_screen(screen);
        }
    }

    size_t index = ui_stack_next_index();
    UIScene *scene = &stack.scenes[index];

    UIScreen *top = &scene->screens[SCREEN_TOP];
    UIScreen *btm = &scene->screens[SCREEN_BTM];

    ui_load_screen(top);
    ui_load_screen(btm);

    stack.current_anchor = index;
    stack.active_scene_count = 1;
}

static void close_anchor(){
    if(stack.current_anchor == 0) return;

    for(size_t i = ui_stack_min_index(); i < ui_stack_max_index(); i++){
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
            screen->transition.time = screen->transition.duration;
        }
    }

    stack.active_scene_count = stack.current_anchor - new_anchor;

    stack.current_anchor = new_anchor;
}

void ui_stack_update(UIInput *input){
    if(stack.active_scene_count == 0) return;

    for(size_t i = ui_stack_min_index(); i <= ui_stack_max_index(); i++){
        UIScene *scene = &stack.scenes[i];

        bool updating_topmost = i == ui_stack_max_index() && stack.transition == UI_TRANSITION_NONE;
    
        for(int j = 0; j < 2; j++) {
            UIScreen *screen = &scene->screens[j];

            UIInput dummy_input = { 0 };
            //only update top screen with input
            UIInput *input_to_use = updating_topmost && (screen->transition.state == UI_TRANSITION_NONE) ? input : &dummy_input;

            if(screen && screen->loaded){
                ui_screen_update(screen, input_to_use);
            }
        }
    }

    UIScene *max_scene = &stack.scenes[ui_stack_max_index()];

    if(!max_scene->anchor){
        UIScreen *max_top = &max_scene->screens[SCREEN_TOP];
        UIScreen *max_btm = &max_scene->screens[SCREEN_BTM];

        bool top_closing = max_top->loaded ? max_top->closing : true;
        bool btm_closing = max_btm->loaded ? max_btm->closing : true;

        if(stack.transition == UI_TRANSITION_NONE && top_closing && btm_closing){
            ui_unload_screen(max_top);
            ui_unload_screen(max_btm);

            stack.active_scene_count--;
        }
    }

    //anchor transition open/close slop
    bool transition_switch = false;
    if(stack.fade_time >= 255.f){
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
                break;
        }
    }

    //for when the fade is fully black
    if(transition_switch) {
        switch(stack.transition){
            case UI_TRANSITION_OPENING:
                open_anchor();
                stack.fade_time = 0.f;
                stack.fade = FADE_STATUS_OUT;
                break;
            case UI_TRANSITION_CLOSING:
                close_anchor();
                stack.fade_time = 0.f;
                stack.fade = FADE_STATUS_OUT;
                break;
            case UI_TRANSITION_NONE:
                break;
        }
    }

    stack.fade_time += FADE_SPEED * DT;
}

static void draw_stack_debug(){
    char debug[2048];
    size_t pos = 0;

    for (size_t i = 0; i < stack.stack_capacity; i++) {
        UIScene *scene = &stack.scenes[i];

        int r = 0;
        if(scene->anchor){
            r = 128;
        }
        if(i == stack.current_anchor){
            r = 255;
        }
        int gb = 128;
        if(i > ui_stack_min_index() && i <= ui_stack_max_index()){
            gb = 255;
        }

        pos += snprintf(
            debug + pos,
            sizeof(debug) - pos,
            "<%d,%d,%d>%s</><p>",
            r, gb, gb,
            scene->name ? scene->name : "(unnamed)"
        );

        if (pos >= sizeof(debug)) break;
    }

    char anchorSlop[] = "Anchor:....";
    snprintf(anchorSlop, sizeof(anchorSlop), "Anchor: %d", stack.current_anchor);

    char activeSlop[] = "Active:....";
    snprintf(activeSlop, sizeof(activeSlop), "Active: %d", stack.active_scene_count);

    char capacitySlop[] = "Capacity:....";
    snprintf(capacitySlop, sizeof(capacitySlop), "Capacity: %d", stack.stack_capacity);

    draw_text(&bigFont_fontCharset, &bigFont_sheet, SCREEN_BOT_WIDTH / 2, SCREEN_HEIGHT / 2, 0.5f, 0.5f, 0, true, debug);
    draw_text(&bigFont_fontCharset, &bigFont_sheet, 10, 10, 0.5f, 0.5f, 0, true, anchorSlop);
    draw_text(&bigFont_fontCharset, &bigFont_sheet, 10, 25, 0.5f, 0.5f, 0, true, activeSlop);
    draw_text(&bigFont_fontCharset, &bigFont_sheet, 10, 40, 0.5f, 0.5f, 0, true, capacitySlop);
}

void ui_stack_draw(Screens target){
    if(stack.active_scene_count == 0) return;

    for(size_t i = ui_stack_min_index(); i <= ui_stack_max_index(); i++){
        UIScreen *screen = &stack.scenes[i].screens[target];
        if(screen && screen->loaded){
            ui_screen_draw(screen);
        }
    }

    float fade = 0;
    if(stack.fade == FADE_STATUS_IN){
        fade = stack.fade_time;
    } else if(stack.fade == FADE_STATUS_OUT){
        fade = 255 - stack.fade_time;
    }

    //fade
    C2D_DrawRectSolid(0.f, 0.f, 0.f, SCREEN_WIDTH, SCREEN_HEIGHT, C2D_Color32(0, 0, 0, fade));

    if(target == SCREEN_TOP) draw_stack_debug();
}

static bool resize_stack(size_t new_capacity){
    //thanks advexed 
    if(new_capacity > 64){
        return false;
    }

    size_t capacity = stack.stack_capacity;

    if(new_capacity <= capacity){
        return true;
    }

    //stack is expanded in chunks of 4
    capacity += 4;

    UIScene *temp = realloc(stack.scenes, sizeof(UIScene) * capacity);

    if(!temp){
        return false;
    }

    stack.stack_capacity = capacity;
    stack.scenes = temp;

    //reset scene pointers in existing screens
    if(stack.stack_capacity > 0){
        for(size_t i = 0; i < stack.stack_capacity; i++){
            UIScene *scene = &stack.scenes[i];
            for(size_t j = 0; j < 2; j++){
                scene->screens[j].scene = scene;
            }
        }
    }

    return true;
}

static UIScene *ui_stack_push_base(const UIScreenDefPair* defs){
    if(stack.transition != UI_TRANSITION_NONE) {
        printf("Cannot push to stack during transition!");
        return NULL;
    }

    int index = ui_stack_next_index();

    if(!resize_stack(index + 1)){
        return NULL;
    }

    UIScene *scene = &stack.scenes[index];

    scene = &stack.scenes[index];

    if(!scene){
        return NULL;
    }

    scene->name = defs->name;

    UIScreen *top = &scene->screens[SCREEN_TOP];
    UIScreen *btm = &scene->screens[SCREEN_BTM];

    *top = (UIScreen){ 0 };
    *btm = (UIScreen){ 0 };

    btm->isBottom = true;

    top->def = &defs->top;
    btm->def = &defs->btm;

    top->scene = scene;
    btm->scene = scene;

    return scene;
}

//initiates the fade to black transition
void ui_stack_push_anchor(const UIScreenDefPair* defs, bool instant){
    UIScene *scene = ui_stack_push_base(defs);

    if (!scene) {
        return;
    }

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
    UIScene *scene = ui_stack_push_base(defs);

    if (!scene){
        return;
    }

    scene->anchor = false;

    UIScreen *top = &scene->screens[SCREEN_TOP];
    UIScreen *btm = &scene->screens[SCREEN_BTM];
    
    stack.active_scene_count++;

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

    if(stack.active_scene_count == 0) return;

    UIScene *scene = &stack.scenes[ui_stack_max_index()];
    if(scene->anchor){
       stack.transition = UI_TRANSITION_CLOSING;
       stack.fade = FADE_STATUS_IN;
       stack.fade_time = 0.f;
    } else{
        for(int i = 0; i < 2; i++){
            ui_screen_close(&scene->screens[i]);
        }
    }
}

void ui_stack_fini(){
    free(stack.scenes);
    stack.stack_capacity = 0;
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
            ui_stack_push(&credits_def, ANIM_ZOOM, ANIM_ZOOM);
        }
        if(touch.down & KEY_Y){
            ui_stack_pop();
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

        ui_stack_draw(SCREEN_BTM);

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

    ui_stack_fini();
}