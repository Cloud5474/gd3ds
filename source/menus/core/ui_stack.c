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
    if(stack.active_count == 0){
        return 0;
    }
    return stack.current_anchor + stack.active_count - 1;
}

static size_t ui_stack_next_index() {
    return stack.current_anchor + stack.active_count;
}

static void open_anchor(){
    for(size_t i = ui_stack_min_index(); i <= ui_stack_max_index(); i++){
        UIScreenPair *pair = &stack.screen_stack[i];
        for(int j = 0; j < 2; j++){
            UIScreen *screen = &pair->screens[j];
            ui_unload_screen(screen);
        }
    }

    size_t index = ui_stack_next_index();

    stack.current_anchor = index;
    stack.active_count = 1;
}

static void close_anchor(){
    if(stack.current_anchor == 0) return;

    for(size_t i = ui_stack_min_index(); i < ui_stack_max_index(); i++){
        for(int j = 0; j < 2; j++){
            UIScreen *screen = &stack.screen_stack[i].screens[j];
            ui_unload_screen(screen);
        }
    }

    //find previous anchor
    size_t new_anchor = 0;
    for(size_t i = stack.current_anchor; i-- > 0; ){
        if(stack.screen_stack[i].anchor){
            new_anchor = i;
            break;
        }
    }

    for(size_t i = new_anchor; i < stack.current_anchor; i++){
        UIScreenPair *pair = &stack.screen_stack[i];
        for(int j = 0; j < 2; j++){
            UIScreen *screen = &pair->screens[j];
            ui_load_screen(screen);
            finish_animation(screen);
        }
    }

    stack.active_count = stack.current_anchor - new_anchor;

    stack.current_anchor = new_anchor;
}

static bool resize_stack(size_t new_capacity){
    //thanks advexed 
    if(new_capacity > 32){
        return false;
    }

    size_t capacity = stack.stack_capacity;

    if(new_capacity <= capacity){
        return true;
    }

    while(capacity < new_capacity){
        capacity += 4;
    }

    UIScreenPair *temp = realloc(stack.screen_stack, sizeof(UIScreenPair) * capacity);

    if(!temp){
        return false;
    }

    stack.screen_stack = temp;
    stack.stack_capacity = capacity;

    //reset pair pointers in existing screens
    if(stack.stack_capacity > 0){
        for(size_t i = 0; i < stack.stack_capacity; i++){
            UIScreenPair *pair = &stack.screen_stack[i];
            for(size_t j = 0; j < 2; j++){
                pair->screens[j].pair = pair;
            }
        }
    }

    return true;
}

static UIScreenPair *ui_stack_push_base(const UIScreenDefPair *defs){
    if(!defs){
        return NULL;
    }

    int index = ui_stack_next_index();

    if(!resize_stack(index + 1)){
        return NULL;
    }

    UIScreenPair *pair = &stack.screen_stack[index];

    pair->name = defs->name;

    UIScreen *top = &pair->screens[SCREEN_TOP];
    UIScreen *btm = &pair->screens[SCREEN_BTM];

    *top = (UIScreen){ 0 };
    *btm = (UIScreen){ 0 };

    btm->isBottom = true;

    top->def = &defs->top;
    btm->def = &defs->btm;

    top->pair = pair;
    btm->pair = pair;

    return pair;
}

static void update_opening(){
    UIStackPush *push = &stack.push;

    if(push->type == PUSH_NEXT){
        push->push_now = true;
    }

    //do push
    if(push->push_now && push->type != PUSH_NONE){
        push->push_now = false;

        UIScreenPair *pair = ui_stack_push_base(push->defs);

        if(!pair){
            push->type = PUSH_NONE;
            return;
        }

        pair->anchor = (push->type == PUSH_ANCHOR);

        push->type = PUSH_NONE;

        UIScreen *top = &pair->screens[SCREEN_TOP];
        UIScreen *btm = &pair->screens[SCREEN_BTM];

        if(pair->anchor){
            open_anchor();
        } else{
            stack.active_count++;
        }

        ui_load_screen(top);
        ui_load_screen(btm);

        ui_screen_open(top, push->top_anim);
        ui_screen_open(btm, push->btm_anim);
    }
}

static void update_closing(){
    UIScreenPair *max_pair = &stack.screen_stack[ui_stack_max_index()];

    if(!max_pair->anchor){
        UIScreen *max_top = &max_pair->screens[SCREEN_TOP];
        UIScreen *max_btm = &max_pair->screens[SCREEN_BTM];

        bool top_closing = max_top->loaded ? max_top->closing : true;
        bool btm_closing = max_btm->loaded ? max_btm->closing : true;

        if(stack.transition == UI_TRANSITION_NONE && top_closing && btm_closing){
            ui_unload_screen(max_top);
            ui_unload_screen(max_btm);

            stack.active_count--;

            if(stack.push.type == PUSH_AFTER_CLOSE){
                stack.push.push_now = true;
            }
        }
    }
}

static void handle_stack_fading(){
    //anchor transition open/close slop
    //instant if the active count is 0 (main menu doesn't fade in)
    bool transition_switch = false;
    if(stack.fade_time >= 255.f || stack.active_count == 0){
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

    //when the fade is fully black, either go back to the previous anchor or open a new one depending on transition
    if(transition_switch) {
        switch(stack.transition){
            case UI_TRANSITION_OPENING:
                if(stack.push.type == PUSH_ANCHOR) stack.push.push_now = true;
                stack.fade_time = 0.f;
                //don't fade out if opening the first menu
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

void ui_stack_update(UIInput *input){
    handle_stack_fading();

    update_opening();

    if(stack.active_count > 0){
        for(size_t i = ui_stack_min_index(); i <= ui_stack_max_index(); i++){
            UIScreenPair *pair = &stack.screen_stack[i];

            bool updating_topmost = i == ui_stack_max_index() && stack.transition == UI_TRANSITION_NONE;
        
            for(int j = 0; j < 2; j++) {
                UIScreen *screen = &pair->screens[j];

                UIInput dummy_input = { 0 };
                //only update top screen with input
                UIInput *input_to_use = updating_topmost && screen->transition.done ? input : &dummy_input;

                if(screen){
                    ui_screen_update(screen, input_to_use);
                }
            }
        }

        update_closing();
    }
}

static void draw_stack_debug(){
    char debug[2048];
    size_t pos = 0;

    for (size_t i = 0; i <= ui_stack_max_index(); i++) {
        UIScreenPair *pair = &stack.screen_stack[i];

        int r = 0;
        if(pair->anchor){
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
            "<%d,%d,%d>%s</>\n",
            r, gb, gb,
            pair->name ? pair->name : "(unnamed)"
        );

        if (pos >= sizeof(debug)) break;
    }

    char anchorSlop[] = "Anchor:....";
    snprintf(anchorSlop, sizeof(anchorSlop), "Anchor: %d", stack.current_anchor);

    char activeSlop[] = "Active:....";
    snprintf(activeSlop, sizeof(activeSlop), "Active: %d", stack.active_count);

    char capacitySlop[] = "Capacity:....";
    snprintf(capacitySlop, sizeof(capacitySlop), "Capacity: %d", stack.stack_capacity);

    draw_text(&bigFont_fontCharset, &bigFont_sheet, SCREEN_BOT_WIDTH / 2, SCREEN_HEIGHT / 2, 0.5f, 0.5f, 0, true, debug);
    draw_text(&bigFont_fontCharset, &bigFont_sheet, 10, 10, 0.5f, 0.5f, 0, true, anchorSlop);
    draw_text(&bigFont_fontCharset, &bigFont_sheet, 10, 25, 0.5f, 0.5f, 0, true, activeSlop);
    draw_text(&bigFont_fontCharset, &bigFont_sheet, 10, 40, 0.5f, 0.5f, 0, true, capacitySlop);
}

void ui_stack_draw(Screens target){
    if(stack.active_count == 0) return;

    for(size_t i = ui_stack_min_index(); i <= ui_stack_max_index(); i++){
        UIScreen *screen = &stack.screen_stack[i].screens[target];
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

void ui_stack_push(
    const UIScreenDefPair* defs, 
    UIAnimation top_anim, 
    UIAnimation btm_anim,
    UIStackPushType type
){
    if(type == PUSH_NONE){
        return;
    }

    if(stack.transition != UI_TRANSITION_NONE){
        printf("Cannot push during transition!");
        return;
    }

    if(stack.push.type != PUSH_NONE){
        printf("Cannot queue multiple pushes!");
        return;
    }

    stack.push.defs = defs;
    stack.push.top_anim = top_anim;
    stack.push.btm_anim = btm_anim;
    stack.push.type = type;

    if(type == PUSH_ANCHOR){
        stack.transition = UI_TRANSITION_OPENING;
        stack.fade = FADE_STATUS_IN;
        stack.fade_time = 0.f;
    }
}

void ui_stack_push_name(
    const char* name, 
    UIAnimation top_anim, 
    UIAnimation btm_anim,
    UIStackPushType type
){
    const UIScreenDefPair *defs = ui_get_screen_def(name);

    if(!defs){
        return;
    }

    ui_stack_push(defs, top_anim, btm_anim, type);
}

void ui_stack_push_anchor_instant(const UIScreenDefPair *defs){
    UIScreenPair *pair = ui_stack_push_base(defs);

    if (!pair) {
        return;
    }

    pair->anchor = true;

    open_anchor();

    UIScreen *top = &pair->screens[SCREEN_TOP];
    UIScreen *btm = &pair->screens[SCREEN_BTM];

    ui_load_screen(top);
    ui_load_screen(btm);

    finish_animation(top);
    finish_animation(btm);
}

void ui_stack_pop(){
    if(stack.transition != UI_TRANSITION_NONE) {
        printf("Cannot pop stack during transition!");
        return;
    }

    if(stack.active_count == 0) return;

    UIScreenPair *pair = &stack.screen_stack[ui_stack_max_index()];
    if(pair->anchor){
       stack.transition = UI_TRANSITION_CLOSING;
       stack.fade = FADE_STATUS_IN;
       stack.fade_time = 0.f;
    } else{
        for(int i = 0; i < 2; i++){
            ui_screen_close(&pair->screens[i]);
        }
    }
}

void ui_stack_fini(){
    for(size_t i = 0; i < stack.stack_capacity; i++){
        for(int j = 0; j < 2; j++){
            UIScreen *s = &stack.screen_stack[i].screens[j];
            if(s->loaded){
                ui_unload_screen(s);
            }
        }
    }

    free(stack.screen_stack);
    stack.stack_capacity = 0;
}

UIScreen *ui_stack_get_screen(const char *name, Screens screen) {
    UIScreenPair *pair = NULL;

    for(size_t i = ui_stack_min_index(); i <= ui_stack_max_index(); i++){
        UIScreenPair *pair1 = &stack.screen_stack[i];

        if(!pair1) continue;

        if(strcmp(pair1->name, name) == 0){
            pair = pair1;
        }
    }

    if(!pair) return NULL;
    
    return &pair->screens[screen];
}