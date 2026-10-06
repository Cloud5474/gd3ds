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



    -currently, vertical list navigation does not work
    -the selection box gets much too small on extremely small buttons
    -list gets unselected when returning to a screen which had said list selected (i know why but idk how to fix lols!)
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

//whether the controls are locked onto a specific element (slider sliding, list scrolling)
bool locked_selected = false;

NavigationEntry get_empty_navigation_entry(){
    NavigationEntry empty = (NavigationEntry){ 0 };
    empty.t.scaleX = 1.f;
    empty.t.scaleY = 1.f;
    empty.list_t.scaleX = 1.f;
    empty.list_t.scaleY = 1.f;

    return empty;
}

//lists have 2 possible selection modes: scroll mode and navigation mode. 
//scroll mode means you use up/down to control the list, navigation mode means you can use the dpad to navigate the list's children. 
//in scroll mode, screen->selected.list is NULL, but locked_selected is true.
//in navigation mode, screen->selected.list isn't NULL, and locked_selected is false.
static bool is_list_navigating(NavigationEntry entry){
    return entry.list;
}

static bool is_list_scrolling(NavigationEntry entry){
    return entry.e->type == UI_LIST && locked_selected;
}

static bool is_slider_scrolling(NavigationEntry entry){
    return entry.e->type == UI_SLIDER && locked_selected;
}

static float get_entry_x(NavigationEntry entry){
    return entry.t.x;
}

static float get_entry_y(NavigationEntry entry){
    return entry.t.y;
}

static float get_entry_x_render(NavigationEntry entry){
    return get_entry_x(entry) - abs((entry.e->w) * entry.t.scaleX) / 2.f;
}

static float get_entry_y_render(NavigationEntry entry){
    return get_entry_y(entry) - (abs((entry.e->h) * entry.t.scaleY) / 2.f) + (is_list_navigating(entry) ? entry.list->scrollSmoothY : 0.f);
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

        NavigationEntry entry = get_empty_navigation_entry();
        entry.e = e;
        entry.t = world;

        (*entries)[(*count)++] = entry;
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
    if(!entries) return get_empty_navigation_entry();

    NavigationEntry closest = get_empty_navigation_entry();

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
    if(!entries) return get_empty_navigation_entry();

    NavigationEntry closest = get_empty_navigation_entry();
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
        return get_empty_navigation_entry();
    }

    size_t capacity = 4;
    size_t count = 0;
    NavigationEntry *entries_in_dir = malloc(capacity * sizeof(*entries_in_dir));

    if(!entries_in_dir){
        return get_empty_navigation_entry();
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
                    return get_empty_navigation_entry();
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

static bool get_first_navigable_child(UIElement *e, UITransform *parent, NavigationEntry *out_entry){
    if (!e || !e->enabled) return true;

    UITransform world = ui_transform_combine(parent, e);

    if (e->modify_transform)
        e->modify_transform(e, &world);

    if (e->navigable) {
        out_entry->e = e;
        out_entry->t = world;
        return true;
    }

    if(e->type == UI_LIST) return false;

    for (UIElement *child = e->first_child; child; child = child->next_sibling) {
        if(get_first_navigable_child(child, &world, out_entry)){
            return true;
        }
    }

    return false;
}

static bool attempt_select_list(NavigationEntry list_entry){
    if(list_entry.e->type != UI_LIST) return false;

    UIList *list = (UIList *)list_entry.e;
    UITransform list_t = list_entry.t;

    NavigationEntry entry = get_empty_navigation_entry();
    bool found = false;

    float y = -list->base.h * 0.5f;

    for (UIElement *item = list->base.first_child; item; item = item->next_sibling) {
        UITransform t = list_t;
        t.y += (y + (item->h * 0.5f)) * list_t.scaleY;

        item->w = list->base.w;

        found = get_first_navigable_child(item, &t, &entry);
        if(found){
            entry.list_entry_top = y;
            entry.list_entry_bottom = y + item->h;
            break;
        };

        y += item->h;
    }

    if(!found) return false;

    entry.list = list;
    entry.list_t = list_t;

    navigate_to(entry);
    nav_anim_progress = 1.f;

    list->nav_deselect = false;

    return true;
}

static bool add_navigation_entry_list(
    NavigationEntry **entries,
    size_t *count,
    size_t *capacity,
    UIElement *e,
    UITransform *parent,
    UIList *list,
    UITransform list_t,
    int list_entry_top,
    int list_entry_bottom
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

        NavigationEntry entry = get_empty_navigation_entry();
        entry.e = e;
        entry.t = world;
        entry.list = list;
        entry.list_t = list_t;
        entry.list_entry_top = list_entry_top;
        entry.list_entry_bottom = list_entry_bottom;

        (*entries)[(*count)++] = entry;
    }

    //list children are not included in base navigation entries (the list must be selected)
    if(e->type == UI_LIST) return true;

    for (UIElement *child = e->first_child; child; child = child->next_sibling) {
        if (!add_navigation_entry_list(
                entries,
                count,
                capacity,
                child,
                &world,
                list,
                list_t,
                list_entry_top,
                list_entry_bottom)) {
            return false;
        }
    }

    return true;
}

static NavigationEntry *get_navigation_entries_list(UIList *list, UITransform list_t, size_t *out_count){
    if(!list || !out_count) {
        if(out_count) *out_count = 0;
        return NULL;
    }

    UIElement *e = &list->base;

    size_t capacity = 4;
    size_t count = 0;
    NavigationEntry *navigable = malloc(capacity * sizeof(*navigable));

    if(!navigable){
        *out_count = 0;
        return NULL;
    }

    float y = -list->base.h * 0.5f;

    for (UIElement *child = e->first_child; child; child = child->next_sibling) {
        UITransform t = list_t;
        t.y += (y + (child->h * 0.5f)) * list_t.scaleY;

        if (!add_navigation_entry_list(
                &navigable,
                &count,
                &capacity,
                child,
                &t,
                list,
                list_t,
                y,
                y + child->h)) {

            free(navigable);
            *out_count = 0;
            return NULL;
        }

        y += child->h;
    }

    *out_count = count;
    return navigable;
}

static void navigate_in_direction_list(NavigationDir dir){
    size_t navigable_count = 0;
    NavigationEntry *entries = get_navigation_entries_list(
        screen->selected.list, screen->selected.list_t, &navigable_count
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

static void ensure_list_entry_visible(NavigationEntry entry) {
    if (!entry.list) return;

    UIList *list = entry.list;

    float list_top = -list->base.h * 0.5f;
    float list_bottom = list->base.h * 0.5f;

    if (entry.list_entry_top + list->scrollY < list_top) {
        list->scrollY += list_top - (entry.list_entry_top + list->scrollY);
    }
    else if (entry.list_entry_bottom + list->scrollY > list_bottom) {
        list->scrollY += list_bottom - (entry.list_entry_bottom + list->scrollY);
    }
}

static void deselect_list(){
    UIElement *list = &screen->selected.list->base;
    UITransform t = screen->selected.list_t;
    screen->selected = get_empty_navigation_entry();
    screen->selected.e = list;
    screen->selected.t = t;
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
        if(is_list_navigating(screen->selected)){
            navigate_in_direction_list(dir);
        } else if(is_list_scrolling(screen->selected)){

        } else if(is_slider_scrolling(screen->selected)){
            
        } else{
            navigate_in_direction(dir);
        }
    }

    if(is_list_navigating(screen->selected)){
        ensure_list_entry_visible(screen->selected);
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
                if(!attempt_select_list(screen->selected)){
                    //enables scrolling list controls if no navigable buttons are found
                    locked_selected = true;
                }
                break;
            case UI_SLIDER:
                //enables sliding controls
                locked_selected = true;
                break;
            default:
                break;
        }
    }

    if(input->down & KEY_B){
        //disables slider sliding/list scrolling
        locked_selected = false;

        //escape list if navigating
        if(is_list_navigating(screen->selected)){
            deselect_list();
        }
    }
    
    if(is_list_navigating(screen->selected) && screen->selected.list->nav_deselect){
        screen->selected.list->nav_deselect = false;
        deselect_list();
    }
}

void draw_selection_box(float x, float y, float w, float h, u32 col){
    C2D_Image selection_corner = C2D_SpriteSheetGetImage(*get_sheet(1), 58);
    C2D_ImageTint tint = { 0 };

    u32 fade_alpha = C2D_Color32f(0.0f, 0.0f, 0.0f, selector_fade);
    u32 faded_col = (col & C2D_Color32f(1.0f, 1.0f, 1.0f, 0.0f)) | fade_alpha;

    C2D_PlainImageTint(&tint, faded_col, 1.f);

    float scale = 0.75f;
    // + 4 is done to ensure a bit more space for each corner
    if(w < (SELECTION_CORNER_W + 4)){
        scale *= w / (SELECTION_CORNER_W + 4);
    } else if(h < (SELECTION_CORNER_H + 4)){
        scale *= h / (SELECTION_CORNER_H + 4);
    }

    if(scale < 0.2f) scale = 0.3f;

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

void ui_navigation_draw(){
    if(selector_fade <= 0.f) return;

    float amount = easeValue(EASE_OUT, 0.f, 1.f, nav_anim_progress, 1.f, 4.0f);

    float x = get_entry_x_render(screen->last_selected) + (get_entry_x_render(screen->selected) - get_entry_x_render(screen->last_selected)) * amount;
    float y = get_entry_y_render(screen->last_selected) + (get_entry_y_render(screen->selected) - get_entry_y_render(screen->last_selected)) * amount;
    float w = get_entry_w(screen->last_selected) + (get_entry_w(screen->selected) - get_entry_w(screen->last_selected)) * amount;
    float h = get_entry_h(screen->last_selected) + (get_entry_h(screen->selected) - get_entry_h(screen->last_selected)) * amount;

    u32 color = locked_selected ? C2D_Color32f(1.0f, 1.0f, 0.0f, 1.0f) : C2D_Color32f(1.0f, 1.0f, 1.0f, 1.0f);

    draw_selection_box(x, y, w, h, color);
    
    //draw list selection box
    if(!is_list_navigating(screen->selected)) return;

    float lx = screen->selected.list_t.x - abs(screen->selected.list->base.w * screen->selected.list_t.scaleX) / 2.f;
    float ly = screen->selected.list_t.y - abs(screen->selected.list->base.h * screen->selected.list_t.scaleY) / 2.f;
    float lw = fabsf(screen->selected.list->base.w * screen->selected.list_t.scaleX);
    float lh = fabsf(screen->selected.list->base.h * screen->selected.list_t.scaleY);

    draw_selection_box(lx, ly, lw, lh, C2D_Color32f(1.0f, 1.0f, 0.0f, 1.0f));
}

void ui_switch_navigation_screen(UIScreen *s){
    if(!s || !s->isBottom) return; //navigation only on bottom screen for now

    navigation_switching_screen = false;

    screen = s;
    nav_anim_progress = 1.f;

    if(!screen->selected.e) ui_reset_navigation();
}