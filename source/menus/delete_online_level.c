#include <3ds.h>
#include <citro2d.h>

#include "menus/core/ui_screen.h"
#include "menus/components/ui_label.h"
#include "save/saving.h"
#include "menus/online_level_menu.h"

void action_delete_level(UIElement* e, const UIPropertyList *args) {
    delete_level();
}

static UIActionDef delete_level_actions[] = {
    { "delete", action_delete_level },
};

const UIScreenDefPair delete_level_def = {
    .name = "delete_level",
    .btm = {
        .path = "romfs:/menus/delete_level.txt",
        .action_list = {
            .action_count = ARRAY_LEN(delete_level_actions),
            .actions = delete_level_actions
        }
    }
};