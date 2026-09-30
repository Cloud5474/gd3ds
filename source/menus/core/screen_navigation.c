#include "screen_navigation.h"
#include "ui_screen.h"
#include "ui_stack.h"
#include "main.h"
#include "menus/components/ui_checkbox.h"
#include "menus/components/ui_button.h"
#include <stdlib.h>

#define SELECTION_CORNER_W 27
#define SELECTION_CORNER_H 27
#define PADDING 5

/*
    TO DO:
        LISTS:
            -make them navigable, and exclude their children from navigable searches
            -Clicking the A button with a list highlighted does one of two things:
                -If it has children, select first child and limit navigation to list's 
                children
                -If it does not, set locked_selected to true and make it so that pressing
                up/down simply scrolls the list
        SLIDERS:
            -make them navigable, and make it so that selectingn them sets locked_selected
            to true and pressing left/right slides it up and down

        locked_selected can be toggled off by pressing B
*/

typedef enum {
    DIR_NONE,
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} NavigationDir;

bool navigation_switching_screen = false;
static UIScreen *screen = NULL;

bool is_navigating = false;
static float nav_anim_progress = 0.f;
static float selector_fade = 0.f;
bool locked_selected = false;

static float get_entry_x(NavigationEntry entry){
    return entry.t.x;
}

static float get_entry_y(NavigationEntry entry){
    return entry.t.y;
}

static float get_entry_corner_x(NavigationEntry entry){
    return entry.t.x - abs(entry.e->w * entry.t.scaleX) / 2.f;
}

static float get_entry_corner_y(NavigationEntry entry){
    return entry.t.y - abs(entry.e->h * entry.t.scaleY) / 2.f;
}


static float get_entry_w(NavigationEntry entry){
    return fabsf(entry.e->w * entry.t.scaleX);
};

static float get_entry_h(NavigationEntry entry){
    return fabsf(entry.e->h * entry.t.scaleY);
};

static void navigate_to(NavigationEntry entry) {
    if (!entry.e)
        return;

    if (!screen->selected.e) {
        screen->selected = entry;
        screen->last_selected = entry;
        nav_anim_progress = 1.f;
        return;
    }

    screen->last_selected = screen->selected;
    screen->selected = entry;
    nav_anim_progress = 0.f;
}


static bool add_navigation_entry(
    NavigationEntry **entries,
    size_t *count,
    size_t *capacity,
    UIElement *e,
    UITransform *parent
) {
    if (!e || !e->enabled) return true;

    UITransform world = ui_transform_combine(parent, e);

    if (e->modify_transform)
        e->modify_transform(e, &world);

    if (e->navigable) {
        if (*count == *capacity) {
            size_t new_capacity = *capacity * 2;

            NavigationEntry *new_entries =
                realloc(*entries, new_capacity * sizeof(**entries));

            if (!new_entries)
                return false;

            *entries = new_entries;
            *capacity = new_capacity;
        }

        (*entries)[(*count)++] = (NavigationEntry) {
            .e = e,
            .t = world
        };
    }

    //list children are not included in base navigation entries (the list must be selected)
    if(e->type == UI_LIST) return true;

    for (UIElement *child = e->first_child; child; child = child->next_sibling) {
        if (!add_navigation_entry(
                entries,
                count,
                capacity,
                child,
                &world)) {
            return false;
        }
    }

    return true;
}

static NavigationEntry *get_navigation_entries(UIElement **elements, size_t e_count, size_t *out_count){
    if(!elements || !out_count) {
        if(out_count) *out_count = 0;
        return NULL;
    }

    size_t capacity = 4;
    size_t count = 0;
    NavigationEntry *navigable = malloc(capacity * sizeof(*navigable));

    if(!navigable){
        *out_count = 0;
        return NULL;
    }

    UITransform identity = {
        .x = 0.f,
        .y = 0.f,
        .scaleX = 1.f,
        .scaleY = 1.f
    };


    for(size_t i = 0; i < e_count; i++){
        if (!add_navigation_entry(
                &navigable,
                &count,
                &capacity,
                elements[i],
                &identity)) {

            free(navigable);
            *out_count = 0;
            return NULL;
        }
    }

    *out_count = count;
    return navigable;
}

static NavigationEntry find_closest_entry(NavigationEntry *entries, size_t count, float from_x, float from_y){
    if(!entries) return (NavigationEntry){ 0 };

    NavigationEntry closest = (NavigationEntry){ 0 };
    float closest_distance = __FLT_MAX__;
    for(size_t i = 0; i < count; i++){
        NavigationEntry entry = entries[i];
        float dx = get_entry_x(entry) - from_x;
        float dy = get_entry_y(entry) - from_y;
        float distance = sqrtf(dx * dx + dy * dy);
        if(distance < closest_distance){
            closest_distance = distance;
            closest = entries[i];
        }
    }

    return closest;
}

static NavigationEntry find_closest_entry_in_dir(NavigationEntry *entries, size_t count, float from_x, float from_y, NavigationDir dir){
    if(!entries) return (NavigationEntry){ 0 };

    NavigationEntry closest = (NavigationEntry){ 0 };
    float closest_score = __FLT_MAX__;
    for(size_t i = 0; i < count; i++){
        NavigationEntry entry = entries[i];
        float dx = get_entry_x(entry) - from_x;
        float dy = get_entry_y(entry) - from_y;
        float distance = sqrtf(dx * dx + dy * dy);

        float dir_x = 0.f;
        float dir_y = 0.f;

        switch (dir) {
            case DIR_UP:
                dir_y = -1.f; 
                break;
            case DIR_DOWN:
                dir_y =  1.f; 
                break;
            case DIR_LEFT:
                dir_x = -1.f; 
                break;
            case DIR_RIGHT: 
                dir_x =  1.f; 
                break;
            default:
                break;
        }

        if(distance == 0) continue;

        float alignment = (dx * dir_x + dy * dir_y) / distance;
        float score = distance / (alignment + 0.25f);

        if(score < closest_score){
            closest_score = score;
            closest = entries[i];
        }
    }

    return closest;
}

static NavigationEntry get_in_direction(NavigationEntry *entries, size_t e_count, NavigationDir dir, float center_x, float center_y){
    if(!entries || dir == DIR_NONE) {
        return (NavigationEntry){ 0 };
    }

    size_t capacity = 4;
    size_t count = 0;
    NavigationEntry *entries_in_dir = malloc(capacity * sizeof(*entries_in_dir));

    if(!entries_in_dir){
        return (NavigationEntry){ 0 };
    }

    for(size_t i = 0; i < e_count; i++){
        NavigationEntry entry = entries[i];

        if(entry.e == screen->selected.e) continue;

        bool in_direction = false;
        switch(dir){
            case DIR_UP:
                in_direction = get_entry_y(entry) < center_y;
                break;
            case DIR_DOWN:
                in_direction = get_entry_y(entry) > center_y;
                break;
            case DIR_LEFT:
                in_direction = get_entry_x(entry) < center_x;
                break;
            case DIR_RIGHT:
                in_direction = get_entry_x(entry) > center_x;
                break;
            default:
                break;
        }
        if(in_direction){
            if(count == capacity){
                capacity *= 2;
                NavigationEntry *new_entries = realloc(
                    entries_in_dir,
                    capacity * sizeof(*entries_in_dir)
                );

                if (!new_entries) {
                    free(entries_in_dir);
                    return (NavigationEntry){ 0 };
                }

                entries_in_dir = new_entries;
            }
            entries_in_dir[count++] = entry;
        }
    }

    NavigationEntry closest = find_closest_entry_in_dir(
        entries_in_dir,
        count, 
        get_entry_x(screen->selected), 
        get_entry_y(screen->selected),
        dir
    );

    free(entries_in_dir);

    return closest;
}

static void navigate_in_direction(NavigationDir dir){
    size_t navigable_count = 0;
    NavigationEntry *entries = get_navigation_entries(
        screen->elements, screen->count, &navigable_count
    );

    NavigationEntry closest = get_in_direction(
        entries,
        navigable_count,
        dir,
        get_entry_x(screen->selected),
        get_entry_y(screen->selected)
    );

    free(entries);

    if(closest.e){
        navigate_to(closest);
    }
}

//try to select entry closest to center
void ui_reset_navigation(){
    if(!screen) return;

    size_t navigable_count = 0;
    NavigationEntry *entries = get_navigation_entries(
        screen->elements, screen->count, &navigable_count
    );

    float x = SCREEN_BOT_WIDTH / 2.f;
    float y = SCREEN_HEIGHT / 2.f;

    //if there is a leftover element pointer from the last time this screen was active, use cached transform
    if(screen->selected.e){
        x = screen->selected.t.x;
        y = screen->selected.t.y;
    }

    NavigationEntry closest = find_closest_entry(
        entries,
        navigable_count, 
        x, 
        y
    );

    free(entries);

    if(closest.e){
        navigate_to(closest);
    }

    nav_anim_progress = 1.f;
}

static void toggle_navigation(bool navigation){
    is_navigating = navigation;
    if(is_navigating){
        nav_anim_progress = 1.f;
    }
}

static bool restrict_navigation(){
    return !is_navigating || !screen || !screen->transition.done || !screen->selected.e || ui_stack_restrict_navigation() || navigation_switching_screen;
}

void ui_navigation_update(UIInput *input){
    if(input->down & KEY_SELECT){
        toggle_navigation(!is_navigating);
    }

    if(input->down & KEY_TOUCH){
        toggle_navigation(false);
    }

    if(nav_anim_progress <= 1.f){
        nav_anim_progress += (delta * 8.f);
    } else{
        nav_anim_progress = 1.f;
    }

    if(restrict_navigation()){
        if(selector_fade >= 0.f){
            selector_fade -= (delta * 8.f);
        } else{
            selector_fade = 0.f;
        }
        return;
    }

    if(selector_fade <= 1.f){
        selector_fade += (delta * 8.f);
    } else{
        selector_fade = 1.f;
    }

    NavigationDir dir = DIR_NONE;

    if (input->down & (KEY_DUP | KEY_CPAD_UP | KEY_CSTICK_UP)) {
        dir = DIR_UP;
    } else if (input->down & (KEY_DDOWN | KEY_CPAD_DOWN | KEY_CSTICK_DOWN)) {
        dir = DIR_DOWN;
    } else if (input->down & (KEY_DLEFT | KEY_CPAD_LEFT | KEY_CSTICK_LEFT)) {
        dir = DIR_LEFT;
    } else if (input->down & (KEY_DRIGHT | KEY_CPAD_RIGHT | KEY_CSTICK_RIGHT)) {
        dir = DIR_RIGHT;
    }

    if(dir){
        navigate_in_direction(dir);
    }

    if(input->down & KEY_A){
        switch(screen->selected.e->type){
            case UI_BUTTON:
            case UI_COLOR_BUTTON:
            case UI_ICON:
            case UI_WINDOW_BUTTON:
                ui_button_pressed_key(screen->selected.e);
                break; 
            case UI_CHECKBOX:
                ui_button_pressed_key(screen->selected.e);
                ui_set_checkbox_checked((UICheckBox *)screen->selected.e, !((UICheckBox *)screen->selected.e)->checked);
                break; 
            case UI_LIST:
                
                break;
            case UI_SLIDER:
                break;
            default:
                break;
        }
    }
}

void ui_navigation_draw(){
    if(selector_fade <= 0.f) return;

    float amount = easeValue(EASE_OUT, 0.f, 1.f, nav_anim_progress, 1.f, 4.0f);

    float x = get_entry_corner_x(screen->last_selected) + (get_entry_corner_x(screen->selected) - get_entry_corner_x(screen->last_selected)) * amount;
    float y = get_entry_corner_y(screen->last_selected) + (get_entry_corner_y(screen->selected) - get_entry_corner_y(screen->last_selected)) * amount;
    float w = get_entry_w(screen->last_selected) + (get_entry_w(screen->selected) - get_entry_w(screen->last_selected)) * amount;
    float h = get_entry_h(screen->last_selected) + (get_entry_h(screen->selected) - get_entry_h(screen->last_selected)) * amount;

    C2D_Image selection_corner = C2D_SpriteSheetGetImage(*get_sheet(1), 58);
    C2D_ImageTint tint = { 0 };
    C2D_PlainImageTint(&tint, C2D_Color32f(1.f, 1.f, 1.f, selector_fade), 1.f);

    float scale = 0.75f;
    // + 4 is done to ensure a bit more space for each corner
    if(w < (SELECTION_CORNER_W + 4)){
        scale *= w / (SELECTION_CORNER_W + 4);
    } else if(h < (SELECTION_CORNER_H + 4)){
        scale *= h / (SELECTION_CORNER_H + 4);
    }

    C2D_DrawImageAt(
        selection_corner, 
        x - PADDING, 
        y - PADDING, 
        0.f, &tint, scale, -scale
    );
    C2D_DrawImageAt(
        selection_corner, 
        x - PADDING, 
        y + h - (SELECTION_CORNER_H * scale) + PADDING, 
        0.f, &tint, scale, scale
    );
    C2D_DrawImageAt(
        selection_corner, 
        x + w - (SELECTION_CORNER_W * scale) + PADDING, 
        y - PADDING, 
        0.f, &tint, -scale, -scale
    );
    C2D_DrawImageAt(
        selection_corner, 
        x + w - (SELECTION_CORNER_W * scale) + PADDING, 
        y + h - (SELECTION_CORNER_H * scale) + PADDING, 
        0.f, &tint, -scale, scale
    );
}

void ui_switch_navigation_screen(UIScreen *s){
    if(!s || !s->isBottom) return; //navigation only on bottom screen for now

    navigation_switching_screen = false;

    screen = s;
    nav_anim_progress = 1.f;

    ui_reset_navigation();
}