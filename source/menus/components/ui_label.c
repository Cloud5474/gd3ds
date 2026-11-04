
#include <citro2d.h>
#include "menus/core/ui_screen.h"
#include "menus/core/ui_props.h"
#include <stdarg.h>

static const UIFloatEnumEntry alignment_table[] = {
    { "LEFT",   0.f },
    { "CENTER", 0.5f },
    { "RIGHT",  1.f }
};

static void ui_label_draw(UIElement* e, UITransform *transform) {
    UILabel *label = (UILabel *) e;
    int font_id = label->font;

    // Set to pusab if invalid
    if (font_id >= NUM_FONTS) font_id = 0;

    const LabelFont *font = &fonts[font_id];

    float scale = transform->scaleX;

    int width = e->w;

    if (width > 0){
        float length = get_longest_line_length(font->charset, e->scaleX, label->text);

        if (width < length && length > 0) {
            scale *= (width / length);
        }
    }

    draw_text(font->charset, font->sheet, transform->x, transform->y, scale, scale, label->alignment, label->parse_tags, "%s", label->text);
}

static void ui_label_destroy(UIElement* e) {
    if (e) {
        if(((UILabel *)e)->text){
            free(((UILabel *)e)->text);
        }
        free(e);
        e = NULL;
    }
}

void ui_label_snprintf(UILabel *label, const char *format, ...) {
    va_list args;
    va_start(args, format);

    va_list args_copy;
    va_copy(args_copy, args);
    size_t length = vsnprintf(NULL, 0, format, args_copy);
    va_end(args_copy);

    if (length < 0) {
        va_end(args);
        return;
    }

    char *text = malloc(length + 1);
    if (text == NULL) {
        va_end(args);
        return;
    }

    vsnprintf(text, length + 1, format, args);
    va_end(args);

    free(label->text);
    label->text = text;
}

void ui_label_set_text(UILabel *e, const char *text) {
    if (!e || !text) return;

    //get rid of original text
    if(e->text){
        free(e->text);
    }

    e->text = strdup(text);
}

UILabel *ui_create_label(UIScreen *screen) {
    UILabel *e = malloc(sizeof(UILabel));

    if (!e) return NULL;

    memset(e, 0, sizeof(UILabel));
    e->base.type = UI_LABEL;
    e->base.enabled = true;
    
    ui_element_apply_default_properties(&e->base, screen);

    e->parse_tags = true;

    e->base.draw = ui_label_draw;
    e->base.destroy = ui_label_destroy;

    return e;
}

UIElement *ui_create_label_from_props(UIScreen *screen, const UIPropertyList *props) {
    UILabel *label = ui_create_label(screen);

    if (!label) return NULL;

    ui_element_apply_properties(&label->base, screen, props);

    ui_label_set_text(label, ui_prop_string(props, "text", ""));
    
    label->font = ui_prop_int(props, "font", 0);
    label->alignment = ui_prop_float_enum(props, "align", alignment_table, ARRAY_LEN(alignment_table), 0);
    label->parse_tags = ui_prop_bool(props, "parseTags", true);

    return &label->base;
}

