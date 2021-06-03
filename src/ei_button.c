#include "ei_button.h"

ei_widget_t* button_alloc(void)
{
        ei_button_t *button = (ei_button_t*) calloc(1, sizeof(ei_button_t));
        button->color = calloc(1, sizeof(ei_color_t));
        button->border_width = calloc(1, sizeof(int));
        button->corner_radius = calloc(1, sizeof(int));
        button->relief = calloc(1, sizeof(ei_relief_t));
        button->text = calloc(1, sizeof(char*));
        button->text_font = calloc(1, sizeof(ei_font_t));
        button->text_color = calloc(1, sizeof(ei_color_t));
        button->text_anchor = calloc(1, sizeof(ei_anchor_t));
        button->img = calloc(1, sizeof(ei_surface_t));
        button->img_rect = calloc(1, sizeof(ei_rect_t*));
        button->img_anchor = calloc(1, sizeof(ei_anchor_t));
        button->callback = calloc(1, sizeof(ei_callback_t));
        button->user_param = calloc(1, sizeof(void*));
        return (ei_widget_t*) button;
}

void button_release(ei_widget_t* widget)
{
        ei_button_t *button = (ei_button_t*) widget;
        free(button->color);
        free(button->border_width);
        free(button->corner_radius);
        free(button->relief);
        free(*button->text);
        free(button->text);
        free(button->text_font);
        free(button->text_color);
        free(button->text_anchor);
        if (*button->img != NULL) hw_surface_free(*button->img);
        free(button->img);
        free(*button->img_rect);
        free(button->img_rect);
        free(button->img_anchor);
        free(button->callback);
        free(button->user_param);
        free(button);
}

void button_draw(ei_widget_t* widget, ei_surface_t surface, ei_surface_t pick_surface,
                 ei_rect_t* clipper)
{
        ei_button_t *button = (ei_button_t*) widget;
        ei_rect_t rectangle = button->widget.screen_location;
        ei_relief_t *relief = button->relief;
        int *border_width = button->border_width;
        int *corner_radius = button->corner_radius;
        const ei_color_t *color = button->color;
        int text_width = 0;
        int text_height = 0;

        ei_color_t light_color = get_light_color_variation(color);
        ei_color_t dark_color = get_dark_color_variation(color);

        struct ei_linked_point_t *upper_part = rounded_frame(rectangle, *corner_radius, 'h');
        struct ei_linked_point_t *lower_part = rounded_frame(rectangle, *corner_radius, 'l');

        if (*relief == ei_relief_raised){
                ei_draw_polygon(surface, upper_part, light_color, clipper);
                ei_draw_polygon(surface, lower_part, dark_color, clipper);
        } else {
                ei_draw_polygon(surface, upper_part, dark_color, clipper);
                ei_draw_polygon(surface, lower_part, light_color, clipper);
        }

        ei_draw_polygon(pick_surface, upper_part, *(button->widget.pick_color), clipper);
        ei_draw_polygon(pick_surface, lower_part, *(button->widget.pick_color), clipper);

        rectangle.size.height -= 2 * *border_width;
        rectangle.size.width -= 2 * *border_width;
        rectangle.top_left.y += *border_width;
        rectangle.top_left.x += *border_width;

        struct ei_linked_point_t *main_part = rounded_frame(rectangle, *corner_radius, 't');
        ei_draw_polygon(surface, main_part, *color, clipper);

        if (*button->img != NULL) {
                ei_rect_t img_clipper = rectangle_intersect(clipper,widget->content_rect);
                ei_point_t where;
                ei_size_t img_size;
                ei_point_t img_top_left;
                if (*button->img_rect) {
                        img_size = (*button->img_rect)->size;
                        img_top_left = (*button->img_rect)->top_left;
                } else {
                        img_size = hw_surface_get_size(*button->img);
                        img_top_left = ei_point_zero();
                }
                anchoring(*button->img_anchor,&where,&widget->screen_location,&img_size);

                ei_rect_t positioned_rect = {where, img_size};
                ei_rect_t intersection = rectangle_intersect(&img_clipper,&positioned_rect);
                ei_point_t new_origin_start;
                new_origin_start.x = (where.x >= img_clipper.top_left.x)? img_top_left.x :
                                     img_top_left.x -where.x + intersection.top_left.x;
                new_origin_start.y = (where.y >= img_clipper.top_left.y)? img_top_left.y :
                                     img_top_left.y + intersection.top_left.y - where.y;
                ei_rect_t img_intersect = {new_origin_start, intersection.size};
                hw_surface_lock(*button->img);
                hw_surface_lock(surface);
                ei_copy_surface(surface, &intersection, *button->img, &img_intersect, 1);
                hw_surface_unlock(surface);
                hw_surface_unlock(*button->img);
        }

        if (*button->text != NULL) {
                hw_text_compute_size(*button->text, *button->text_font, &text_width, &text_height);
                ei_point_t where;
                ei_size_t text_size = {text_width, text_height};
                anchoring(*button->text_anchor, &where, &widget->screen_location, &text_size);

                ei_rect_t text_clipper = rectangle_intersect(clipper,widget->content_rect);
                ei_draw_text(surface, &where, *button->text, *button->text_font, *button->text_color,
                             &text_clipper);

        }

        // Free points
        free_linked_points(upper_part);
        free_linked_points(lower_part);
        free_linked_points(main_part);
}

void button_setdefault(ei_widget_t* widget)
{
        ei_button_t* button = (ei_button_t*) widget;
        *button->color = ei_default_background_color;
        *button->border_width = k_default_button_border_width;
        *button->corner_radius = k_default_button_corner_radius;
        *button->relief = ei_relief_none;
        *button->text = NULL;
        *button->text_font = ei_default_font;
        *button->text_color = ei_font_default_color;
        *button->text_anchor = ei_anc_center;
        *button->img = NULL;
        *button->img_rect = NULL;
        *button->img_anchor = ei_anc_center;
        *button->callback = NULL;
        *button->user_param = NULL;
}

void button_geomnotify(ei_widget_t* widget, ei_rect_t rect)
{

}

ei_bool_t button_handle(ei_widget_t* widget, ei_event_t* event)
{
        ei_button_t* button = (ei_button_t*) widget;
        ei_rect_t rect2invalidate = widget->parent->screen_location;
        if (strcmp(ei_widgetclass_stringname(widget->parent->wclass->name), "toplevel") == 0) {
                ei_toplevel_t *parent = (ei_toplevel_t*) widget->parent;
                int title_width = 0;
                int title_height = 0;
                hw_text_compute_size(*parent->title, ei_default_font, &title_width, &title_height);
                rect2invalidate.size.height += title_height + 2 * *parent->border_width;
                rect2invalidate.size.width += 2 * *parent->border_width;
        }

        if (event->type == ei_ev_mouse_buttondown) {
                *button->relief = ei_relief_sunken;
                ei_rect_t rect2add = rectangle_intersect(&rect2invalidate, widget->parent->content_rect);
                ei_app_invalidate_rect(&rect2add);
                return EI_TRUE;
        } else if (event->type == ei_ev_mouse_buttonup) {
                if (event->param.mouse.where.x < widget->screen_location.top_left.x ||
                    event->param.mouse.where.x > (widget->screen_location.top_left.x +
                    widget->screen_location.size.width) ||
                    event->param.mouse.where.y < widget->screen_location.top_left.y ||
                    event->param.mouse.where.y > (widget->screen_location.top_left.y +
                    widget->screen_location.size.height)) {
                        ei_event_set_active_widget(NULL);
                        return EI_TRUE;
                } else {
                        *button->relief = ei_relief_raised;
                        ei_rect_t rect2add = rectangle_intersect(&rect2invalidate, widget->parent->content_rect);
                        ei_app_invalidate_rect(&rect2add);
                        if (*button->callback != NULL) {
                                ei_callback_t callback = *(button->callback);
                                callback(widget, event, *button->user_param);
                        }
                        ei_event_set_active_widget(NULL);
                        return EI_TRUE;
                }
        } else if (event->type == ei_ev_mouse_move) {
                if (event->param.mouse.where.x < widget->screen_location.top_left.x ||
                    event->param.mouse.where.x > (widget->screen_location.top_left.x +
                    widget->screen_location.size.width) ||
                    event->param.mouse.where.y < widget->screen_location.top_left.y ||
                    event->param.mouse.where.y > (widget->screen_location.top_left.y +
                    widget->screen_location.size.height)) {
                        if (button->relief != (ei_relief_t *) ei_relief_raised) {
                                *button->relief = ei_relief_raised;
                                ei_rect_t rect2add = rectangle_intersect(&rect2invalidate, widget->parent->content_rect);
                                ei_app_invalidate_rect(&rect2add);
                                return EI_TRUE;
                        }
                } else {
                        if (button->relief != (ei_relief_t *) ei_relief_sunken) {
                                *button->relief = ei_relief_sunken;
                                ei_rect_t rect2add = rectangle_intersect(&rect2invalidate, widget->parent->content_rect);
                                ei_app_invalidate_rect(&rect2add);
                                return EI_TRUE;
                        }
                }
        }
        return EI_FALSE;
}

ei_widgetclass_t buttonclass = {"button",
                                &button_alloc,
                                &button_release,
                                &button_draw,
                                &button_setdefault,
                                &button_geomnotify,
                                &button_handle,
                                NULL};