#include <3ds.h>
#include <citro2d.h>
#include "menus/components/ui_button.h"
#include "menus/core/ui_stack.h"
#include "menus/soggy.h"
#include "state.h"

void creator_menu_init(UIScreen *s){
    if (gotSogged) ui_button_set_image((UIButton *) ui_get_element_by_tag(&default_screen, "create_button"), 20, 1);
    play_menu_song();
}
