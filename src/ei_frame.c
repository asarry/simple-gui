#include "ei_frame.h"

static int default_frame_border_width = 0;

ei_widget_t* frame_alloc (void)
{
        ei_frame_t *frame = (ei_frame_t*) calloc(1, sizeof(ei_frame_t));
        frame->color = calloc(1, sizeof(ei_color_t));
        frame->border_width = calloc(1, sizeof(int));
        frame->relief = calloc(1, sizeof(ei_relief_t));
        frame->text = calloc(1, sizeof(char*));
        frame->text_font = calloc(1, sizeof(ei_font_t));
        frame->text_color = calloc(1, sizeof(ei_color_t));
        frame->text_anchor = calloc(1, sizeof(ei_anchor_t));
        frame->img = calloc(1, sizeof(ei_surface_t));
        frame->img_rect = calloc(1,sizeof(ei_rect_t*));
        frame->img_anchor = calloc(1, sizeof(ei_anchor_t));
        return (ei_widget_t*) frame;
}

void frame_release(ei_widget_t* widget)
{
        ei_frame_t *frame = (ei_frame_t*) widget;
        free(frame->color);
        free(frame->border_width);
        free(frame->relief);
        free(*frame->text);
        free(frame->text);
        free(frame->text_font);
        free(frame->text_color);
        free(frame->text_anchor);
        if (*frame->img != NULL) hw_surface_free(*frame->img);
        free(frame->img);
        free(*frame->img_rect);
        free(frame->img_rect);
        free(frame->img_anchor);
        free(frame);
}

void frame_draw(ei_widget_t* widget, ei_surface_t surface, ei_surface_t pick_surface,
                ei_rect_t* clipper)
{
        ei_frame_t *frame = (ei_frame_t*) widget;
        ei_rect_t *rectangle = &frame->widget.screen_location;
        ei_relief_t *relief = frame->relief;
        int *border_width = frame->border_width;
        const ei_color_t *color = frame->color;
        int text_width = 0;
        int text_height = 0;

        int x_min, x_max, y_min, y_max;

        if (rectangle != NULL) {
                x_min = rectangle->top_left.x;
                x_max = x_min + rectangle->size.width;
                y_min = rectangle->top_left.y;
                y_max = y_min + rectangle->size.height;
        } else {
                x_min = 0;
                x_max = 0;
                y_min = 0;
                y_max = 0;
        }


        ei_color_t light_color = get_light_color_variation(color);
        ei_color_t dark_color = get_dark_color_variation(color);
        ei_rect_t frame_content = rectangle_intersect(clipper,frame->widget.content_rect);
        ei_fill(surface,color,&frame_content);
        ei_fill(pick_surface,frame->widget.pick_color,&frame_content);
        if (*relief != ei_relief_none) {
                ei_rect_t top_h_bar = {frame->widget.screen_location.top_left, {frame->widget
                .screen_location.size.width,*frame->border_width}};
                ei_rect_t top_v_bar = {frame->widget.screen_location.top_left, {*frame->border_width,
                frame->widget.screen_location.size.height-*frame->border_width}};
                ei_rect_t bot_h_bar = {{frame->widget.screen_location.top_left.x,frame->widget.screen_location.top_left.y +
                frame->widget.screen_location.size.height-*frame->border_width},{frame->widget.screen_location
                .size.width-*frame->border_width,*frame->border_width}};
                ei_rect_t bot_v_bar = {{frame->widget.screen_location.top_left.x+
                frame->widget.screen_location.size.width-*frame->border_width,frame->widget
                .screen_location.top_left.y+*frame->border_width},{*frame->border_width,frame->widget
                .screen_location.size
                .height-*frame->border_width}};
                ei_rect_t frame_top_h_part = rectangle_intersect(clipper, &top_h_bar);
                ei_rect_t frame_top_v_part = rectangle_intersect(clipper, &top_v_bar);
                ei_rect_t frame_bot_h_part = rectangle_intersect(clipper, &bot_h_bar);
                ei_rect_t frame_bot_v_part = rectangle_intersect(clipper, &bot_v_bar);
                if (*relief == ei_relief_raised) {
                        // draw top part
                        ei_fill(surface, &light_color, &frame_top_h_part);
                        ei_fill(surface, &light_color, &frame_top_v_part);
                        // draw bottom part
                        ei_fill(surface, &dark_color, &frame_bot_h_part);
                        ei_fill(surface, &dark_color, &frame_bot_v_part);
                } else {
                        // draw top part
                        ei_fill(surface, &dark_color, &frame_top_h_part);
                        ei_fill(surface, &dark_color, &frame_top_v_part);
                        // draw bottom part
                        ei_fill(surface, &light_color, &frame_bot_h_part);
                        ei_fill(surface, &light_color, &frame_bot_v_part);
                }
        }

        if (*frame->img != NULL) {
                ei_rect_t img_clipper = rectangle_intersect(clipper, widget->content_rect);
                ei_point_t where;
                ei_size_t img_size;
                ei_point_t img_top_left;
                if (*frame->img_rect) {
                        img_size = (*frame->img_rect)->size;
                        img_top_left = (*frame->img_rect)->top_left;
                } else {
                        img_size = hw_surface_get_size(*frame->img);
                        img_top_left = ei_point_zero();
                }
                anchoring(*frame->img_anchor, &where, &widget->screen_location, &img_size);

                ei_rect_t positioned_rect = {where, img_size};
                ei_rect_t intersection = rectangle_intersect(&img_clipper, &positioned_rect);
                ei_point_t new_origin_start;
                new_origin_start.x = (where.x >= img_clipper.top_left.x) ? img_top_left.x :
                         img_top_left.y -where.x + intersection.top_left.x;
                new_origin_start.y = (where.y >= img_clipper.top_left.y) ? img_top_left.y :
                        img_top_left.x + intersection.top_left.y - where.y;
                ei_rect_t img_intersect = {new_origin_start, intersection.size};
                ei_copy_surface(surface, &intersection,* frame->img, &img_intersect, 1);
        }

        if (*frame->text != NULL) {
                hw_text_compute_size(*frame->text, *frame->text_font, &text_width, &text_height);
                ei_point_t where;
                ei_size_t text_size = {text_width, text_height};
                anchoring(*frame->text_anchor, &where, &widget->screen_location, &text_size);

                ei_rect_t text_clipper = rectangle_intersect(clipper, widget->content_rect);
                ei_draw_text(surface, &where, *frame->text, *frame->text_font, *frame->text_color,
                             &text_clipper);
        }
}

void frame_setdefaults(ei_widget_t* widget)
{
        ei_frame_t* frame = (ei_frame_t*) widget;
        *frame->color = ei_default_background_color;
        *frame->border_width = default_frame_border_width;
        *frame->relief = ei_relief_none;
        *frame->text = NULL;
        *frame->text_font = ei_default_font;
        *frame->text_color = ei_font_default_color;
        *frame->text_anchor = ei_anc_center;
        *frame->img = NULL;
        *frame->img_rect = NULL;
        *frame->img_anchor = ei_anc_center;
}

void frame_geomnotify(ei_widget_t* widget, ei_rect_t rect)
{

}

ei_bool_t frame_handle(ei_widget_t* widget, ei_event_t* event)
{
        return EI_FALSE;
}

ei_widgetclass_t frameclass = {"frame",
                               &frame_alloc,
                               &frame_release,
                               &frame_draw,
                               &frame_setdefaults,
                               &frame_geomnotify,
                               &frame_handle,
                               NULL};