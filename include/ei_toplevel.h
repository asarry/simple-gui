#ifndef EI_TOPLEVEL_H
#define EI_TOPLEVEL_H

#include <math.h>
#include "ei_application.h"
#include "ei_drawing_tools.h"
#include "ei_event.h"
#include "ei_placer.h"
#include "ei_types.h"
#include "ei_widget.h"
#include "ei_widgetclass.h"
#include "hw_interface.h"

typedef struct ei_toplevel_t {
        struct ei_widget_t widget;
        ei_color_t *color;
        int *border_width;
        char **title;
        ei_bool_t *closable;
        ei_axis_set_t *resizable;
        ei_size_t **min_size;
} ei_toplevel_t;

extern ei_widgetclass_t toplevelclass;

#endif //EI_TOPLEVEL_H
