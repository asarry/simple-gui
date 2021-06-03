#include "ei_toplevel.h"

static int toplevel_loc = 0; // Which part is active : 1 = top bar, 2 = SE corner
static int x_dif = 0;        // Difference between mouse x and topleft x
static int y_dif = 0;        // Difference between mouse y and topleft y
static ei_size_t ei_toplevel_minsize = {160,120};
static int default_toplevel_border_width = 4;
static char default_toplevel_title[9] = "Toplevel";

ei_widget_t* toplevel_alloc (void)
{
        ei_toplevel_t *toplevel = (ei_toplevel_t*) calloc(1, sizeof(ei_toplevel_t));
        toplevel->color = calloc(1, sizeof(ei_color_t));
        toplevel->title = calloc(1, sizeof(char*));
        toplevel->border_width = calloc(1, sizeof(int));
        toplevel->closable = calloc(1,sizeof(ei_bool_t));
        toplevel->resizable = calloc(1, sizeof(ei_axis_set_t));
        toplevel->min_size = calloc(1, sizeof(ei_size_t*));
        return (ei_widget_t*) toplevel;
}

void toplevel_release (ei_widget_t* widget)
{
        ei_toplevel_t *toplevel = (ei_toplevel_t*) widget;
        free(toplevel->color);
        free(*toplevel->title);
        free(toplevel->title);
        free(toplevel->border_width);
        free(toplevel->closable);
        free(toplevel->resizable);
        free(toplevel->min_size);
        free(toplevel->widget.content_rect);
        free(toplevel);
}

void draw_content(ei_widget_t* widget, ei_surface_t surface, ei_surface_t pick_surface,
                  ei_rect_t* clipper)
{
        ei_rect_t new_clipper = rectangle_intersect(clipper, widget->content_rect);
        ei_widget_t* widget_child = widget->children_head;
        while (widget_child) {
                ei_placer_run(widget_child);
                widget_child->wclass->drawfunc(widget_child, surface, pick_surface, &new_clipper);
                widget_child = widget_child->next_sibling;
        }
}

void toplevel_draw(ei_widget_t* widget, ei_surface_t surface, ei_surface_t pick_surface,
                   ei_rect_t* clipper)
{
        ei_toplevel_t *toplevel = (ei_toplevel_t*) widget;
        int *border_width = toplevel->border_width;
        const ei_color_t *color = toplevel->color;
        int text_width = 0;
        int text_height = 0;

        ei_color_t light_color = {0xcf, 0xcf, 0xcf, 0xff};
        ei_color_t dark_color = {0x4f, 0x4f, 0x4f, 0xff};

        // Title bar
        hw_text_compute_size(*toplevel->title, ei_default_font, &text_width, &text_height);

        int bar_radius = 16;

        ei_rect_t bar = {toplevel->widget.screen_location.top_left,
                         {toplevel->widget .screen_location.size.width +
                         (2 * *toplevel->border_width), 2 * text_height}};

        struct ei_linked_point_t *upper_bar = rounded_frame(bar, bar_radius, 'h');
        ei_draw_polygon(surface, upper_bar, dark_color, clipper);
        struct ei_linked_point_t *lower_bar = rounded_frame(bar, bar_radius, 'l');
        ei_draw_polygon(surface, lower_bar, dark_color, clipper);
        ei_rect_t bar_clipper = rectangle_intersect(clipper, &bar);
        ei_fill(pick_surface, toplevel->widget.pick_color, &bar_clipper);
        free_linked_points(upper_bar);
        free_linked_points(lower_bar);

        // Frame
        ei_rect_t frame = {{toplevel->widget.screen_location.top_left.x, toplevel->widget.screen_location
                           .top_left.y + text_height}, {toplevel->widget.screen_location
                           .size.width + (2* *toplevel->border_width), +2* *
                           toplevel->border_width + toplevel->widget.screen_location.size.height}};
        ei_rect_t frame_clipper = rectangle_intersect(clipper, &frame);
        ei_fill(surface, &light_color, &frame_clipper);
        ei_fill(pick_surface, toplevel->widget.pick_color, &frame_clipper);


        // Content background
        ei_rect_t* content_rect = malloc(sizeof(ei_rect_t));
        content_rect->top_left.x = toplevel->widget.screen_location.top_left.x + *toplevel->border_width;
        content_rect->top_left.y = toplevel->widget.screen_location.top_left.y + text_height +
        *toplevel->border_width;
        content_rect->size = toplevel->widget.screen_location.size;
        if (toplevel->widget.content_rect != &toplevel->widget.screen_location) free(toplevel->widget.content_rect);
        toplevel->widget.content_rect = content_rect;
        ei_rect_t bg_clipper = rectangle_intersect(clipper,widget->content_rect);
        ei_fill(surface,color, &bg_clipper);
        ei_fill(pick_surface, toplevel->widget.pick_color, &bg_clipper);
        draw_content(widget, surface, pick_surface, clipper);

        // Resize icon
        if (*toplevel->resizable) {

                int min_icon_size = (10 < *toplevel->border_width) ? *toplevel->border_width : 10;
                ei_rect_t res_icon = {{toplevel->widget.screen_location.top_left.x +
                                      2* *toplevel->border_width +
                                      toplevel->widget.screen_location.size.width - min_icon_size,
                                      toplevel->widget.screen_location.top_left.y +
                                      text_height+toplevel->widget.screen_location.size.height +
                                      2* *toplevel->border_width-min_icon_size},
                                      {min_icon_size, min_icon_size}};

                ei_rect_t res_icon_clipper = rectangle_intersect(clipper, &res_icon);
                ei_fill(surface, &dark_color, &res_icon_clipper);
                ei_fill(pick_surface, toplevel->widget.pick_color, &res_icon_clipper);
        }
        // closing icon
        int offset = 4;
        int closing_icon_size = text_height- 2 * offset;
        if (*toplevel->closable) {
                const ei_color_t red = {0xff, 0x00, 0x00, 0xff};
                // red arc

                int corner_radius = 9;
                int icon_border = 1;
                ei_color_t light_red = {0xff, 0x33, 0x33, 0xff};
                ei_color_t dark_red = {0x99, 0x00, 0x00, 0xff};
                ei_rect_t empty_rect = {{widget->screen_location.top_left.x+2*offset,
                                         widget->screen_location.top_left.y+offset},{closing_icon_size,
                                                                                     closing_icon_size}};
                struct ei_linked_point_t *upper_part = rounded_frame(empty_rect, corner_radius, 'h');
                struct ei_linked_point_t *lower_part = rounded_frame(empty_rect, corner_radius, 'l');

                ei_draw_polygon(surface, upper_part, light_red, clipper);
                ei_draw_polygon(surface, lower_part, dark_red, clipper);

                empty_rect.top_left.y += icon_border;
                empty_rect.top_left.x += icon_border;
                empty_rect.size.width -= 2*icon_border;
                empty_rect.size.height-= 2*icon_border;

                struct ei_linked_point_t *main_part = rounded_frame(empty_rect, corner_radius, 't');
                ei_draw_polygon(surface, main_part, red, clipper);
                free_linked_points(upper_part);
                free_linked_points(lower_part);
                free_linked_points(main_part);
        }

        // title
        if (*toplevel->title != NULL) {
                ei_point_t where;
                if (*toplevel->closable) {
                        where.x = toplevel->widget.screen_location.top_left.x +
                                closing_icon_size+3*offset;
                } else {
                        where.x = toplevel->widget.screen_location.top_left.x;
                }
                where.y = toplevel->widget.screen_location.top_left.y;
                ei_rect_t screen_loc_adjusted = {widget->screen_location.top_left,
                                                 {widget->screen_location.size.width+2* *border_width,
                                                  widget->screen_location.size.height+2* *border_width}};
                ei_rect_t text_clipper = rectangle_intersect(clipper,&screen_loc_adjusted);
                ei_draw_text(surface,&where,*toplevel->title,ei_default_font,ei_font_default_color,
                             &text_clipper);
        }
}

void toplevel_setdefault (struct ei_widget_t* widget)
{
        ei_toplevel_t* toplevel = (ei_toplevel_t*) widget;
        *toplevel->color = ei_default_background_color;
        *toplevel->border_width = default_toplevel_border_width;
        *toplevel->title = default_toplevel_title;
        *toplevel->closable = EI_TRUE;
        *toplevel->resizable = ei_axis_both;
        *toplevel->min_size = &ei_toplevel_minsize;
}

void toplevel_geomnotify (struct ei_widget_t* widget, ei_rect_t rect)
{
        ei_widget_t *head = widget->children_head;
        while (widget->children_head != NULL) {
                widget->children_head->wclass->geomnotifyfunc(widget->children_head,rect);
                widget->children_head = widget->children_head->next_sibling;
        }
        widget->children_head = head;
}

ei_bool_t toplevel_handle (struct ei_widget_t* widget, struct ei_event_t* event)
{
        ei_toplevel_t* toplevel = (ei_toplevel_t*) widget;

        int title_width = 0;
        int title_height = 0;
        hw_text_compute_size(*toplevel->title, ei_default_font, &title_width, &title_height);
        if (event->type == ei_ev_mouse_buttonup) {
                ei_event_set_active_widget(NULL);
                toplevel_loc = 0;
                x_dif = 0;
                y_dif = 0;
                return EI_TRUE;
        } else if (event->type == ei_ev_mouse_buttondown) {
                int corner_width = (*(toplevel->border_width) > 10) ? *(toplevel->border_width) : 10;

                if (*toplevel->closable &&
                    event->param.mouse.where.x >= widget->screen_location.top_left.x &&
                    event->param.mouse.where.x <= widget->screen_location.top_left.x + title_height &&
                    event->param.mouse.where.y >= widget->screen_location.top_left.y &&
                    event->param.mouse.where.y <= widget->screen_location.top_left.y + title_height) {
                        ei_app_invalidate_rect(&widget->screen_location);
                        ei_widget_destroy(widget);
                        return EI_TRUE;
                } else if (event->param.mouse.where.x >= widget->screen_location.top_left.x &&
                event->param.mouse.where.x <= widget->screen_location.top_left.x + widget->screen_location.size.width &&
                event->param.mouse.where.y >= widget->screen_location.top_left.y &&
                event->param.mouse.where.y <= widget->screen_location.top_left.y + title_height) {
                        toplevel_loc = 1;
                        x_dif = event->param.mouse.where.x - widget->screen_location.top_left.x;
                        y_dif = event->param.mouse.where.y - widget->screen_location.top_left.y;
                        return EI_TRUE;
                } else if (event->param.mouse.where.x >= widget->screen_location.top_left.x + widget->screen_location.size.width - corner_width + *toplevel->border_width &&
                event->param.mouse.where.x <= widget->screen_location.top_left.x + widget->screen_location.size.width + 2 * *toplevel->border_width &&
                event->param.mouse.where.y >= widget->screen_location.top_left.y + title_height + widget->screen_location.size.height - corner_width + *toplevel->border_width &&
                event->param.mouse.where.y <= widget->screen_location.top_left.y + title_height + widget->screen_location.size.height + 2 * *toplevel->border_width) {
                        toplevel_loc = 2;
                        return EI_TRUE;
                }
        } else if (event->type == ei_ev_mouse_move) {
                ei_toplevel_t *parent = (ei_toplevel_t *) widget->parent;
                if (toplevel_loc == 1) {            ///toplevel was activated by top bar
                        int new_x = event->param.mouse.where.x - x_dif -
                                widget->parent->screen_location.top_left.x - *parent->border_width;
                        int new_y = event->param.mouse.where.y - y_dif -
                                widget->parent->screen_location.top_left.y - *parent->border_width;
                        if (widget->parent->wclass == widget->wclass) new_y -= title_height;
                        ei_place(widget, NULL, &new_x, &new_y, &(widget->screen_location.size.width), &(widget->screen_location.size.height), NULL,
                                 NULL, NULL, NULL);
                        ei_rect_t new_geom;
                        new_geom.top_left.x = new_x;
                        new_geom.top_left.y = new_y;
                        new_geom.size.width = widget->screen_location.size.width;
                        new_geom.size.height = widget->screen_location.size.height;
                        toplevel_geomnotify(widget, new_geom);

                        ei_app_invalidate_rect(widget->parent->content_rect);
                        return EI_TRUE;
                } else if (toplevel_loc == 2) {     ///toplevel was activated by SE corner
                        int width = (*toplevel->resizable == ei_axis_both || *toplevel->resizable == ei_axis_x) ?
                                event->param.mouse.where.x - widget->screen_location.top_left.x - (int)(1.5 * *toplevel->border_width) : widget->screen_location.size.width;
                        int height = (*toplevel->resizable == ei_axis_both || *toplevel->resizable == ei_axis_y) ?
                                event->param.mouse.where.y - widget->screen_location.top_left.y - title_height - (int)(1.5 * *toplevel->border_width) : widget->screen_location.size.height;
                        ei_size_t* min_size = *toplevel->min_size;

                        int new_x = 0;
                        int new_y = 0;
                        if (widget->parent->wclass == widget->wclass) {
                                new_x = widget->screen_location.top_left.x - widget->parent->screen_location.top_left.x - *parent->border_width;
                                new_y = widget->screen_location.top_left.y - widget->parent->screen_location.top_left.y - title_height - *parent->border_width;
                        } else {
                                new_x = widget->screen_location.top_left.x;
                                new_y = widget->screen_location.top_left.y;
                        }
                        int new_width = (width > min_size->width) ? width : min_size->width;
                        int new_height = (height > min_size->height) ? height : min_size->height;
                        ei_place(widget, NULL, &new_x, &new_y, &new_width, &new_height, NULL, NULL, NULL, NULL);
                        ei_rect_t new_geom;
                        new_geom.top_left.x = widget->screen_location.top_left.x;
                        new_geom.top_left.y = widget->screen_location.top_left.y;
                        new_geom.size.width = new_width;
                        new_geom.size.height = new_height;
                        toplevel_geomnotify(widget, new_geom);

                        ei_app_invalidate_rect(widget->parent->content_rect);
                        return EI_TRUE;
                }
        }
        return EI_FALSE;
}

ei_widgetclass_t toplevelclass = {"toplevel",
                                  &toplevel_alloc,
                                  &toplevel_release,
                                  &toplevel_draw,
                                  &toplevel_setdefault,
                                  &toplevel_geomnotify,
                                  &toplevel_handle,
                                  NULL};