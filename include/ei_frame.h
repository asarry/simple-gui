#ifndef EI_FRAME_H
#define EI_FRAME_H

#include "ei_drawing_tools.h"
#include "ei_types.h"
#include "ei_widget.h"
#include "ei_widgetclass.h"

typedef struct ei_frame_t {
        ei_widget_t widget;
        ei_color_t* color;
        int* border_width;
        ei_relief_t* relief;
        char** text;
        ei_font_t* text_font;
        ei_color_t* text_color;
        ei_anchor_t* text_anchor;
        ei_surface_t* img;
        ei_rect_t** img_rect;
        ei_anchor_t* img_anchor;
} ei_frame_t;

extern ei_widgetclass_t frameclass;

#endif // EI_FRAME_H
