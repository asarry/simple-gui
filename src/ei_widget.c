#include "ei_application.h"
#include "ei_button.h"
#include "ei_frame.h"
#include "ei_picking.h"
#include "ei_tools.h"
#include "ei_toplevel.h"
#include "ei_widget.h"

static uint32_t pick_id = 0;

/**
 * @brief	Creates a new instance of a widget of some particular class, as a descendant of
 *		an existing widget.
 *
 *		The widget is not displayed on screen until it is managed by a geometry manager.
 *		When no more needed, the widget must be released by calling \ref ei_widget_destroy.
 *
 * @param	class_name	The name of the class of the widget that is to be created.
 * @param	parent 		A pointer to the parent widget. Can not be NULL.
 * @param	user_data	A pointer provided by the programmer for private use. May be NULL.
 * @param	destructor	A pointer to a function to call before destroying a widget structure. May be NULL.
 *
 * @return			The newly created widget, or NULL if there was an error.
 */
ei_widget_t* ei_widget_create(ei_widgetclass_name_t class_name, ei_widget_t* parent, void* user_data,
                              ei_widget_destructor_t destructor)
{
        ei_widgetclass_t* wclass = ei_widgetclass_from_name(class_name);
        ei_widget_t* widget = wclass->allocfunc();
        widget->wclass = wclass;
        widget->pick_color = id_to_color(pick_id);
        widget->pick_id = pick_id++;
        widget->user_data = user_data;
        widget->destructor = destructor;

        widget->parent = parent;
        widget->children_head = NULL;
        widget->children_tail = NULL;
        widget->next_sibling = NULL;

        if (widget->parent) {
                if (widget->parent->children_head == NULL) {
                        widget->parent->children_head = widget;
                        widget->parent->children_tail = widget;
                } else {
                        widget->parent->children_tail->next_sibling = widget;
                        widget->parent->children_tail = widget;
                }
                ei_app_invalidate_rect(ei_app_root_widget()->content_rect);
        }

        widget->content_rect = &widget->screen_location;
        widget->placer_params = malloc(sizeof(ei_placer_params_t));
        widget->wclass->setdefaultsfunc(widget);
        return widget;
}
/**
 * @brief	A recursive function used by ei_widget_destroy.
 *
 * @param	widget		The widget that is to be destroyed.
 */
static void ei_widget_destroy_rec(ei_widget_t* widget)
{
        while (widget->children_head) {
                ei_widget_t* to_be_destroyed = widget->children_head;
                widget->children_head = widget->children_head->next_sibling;
                ei_widget_destroy_rec(to_be_destroyed);
        }
        if (widget->destructor != NULL) widget->destructor(widget);
        free(widget->pick_color);
        free(widget->placer_params);
        widget->wclass->releasefunc(widget);
}

/**
 * @brief	Destroys a widget. Calls its destructor if it was provided.
 * 		Removes the widget from the screen if it is currently managed by the placer.
 * 		Destroys all its descendants.
 *
 * @param	widget		The widget that is to be destroyed.
 */
void ei_widget_destroy(ei_widget_t* widget)
{
        ei_event_set_active_widget(NULL);
        if (widget != ei_app_root_widget()) {
                ei_app_invalidate_rect(widget->parent->content_rect);
                ei_widget_t *prev_widget = NULL;
                ei_widget_t *curr_widget = widget->parent->children_head;

                if (curr_widget == widget) {
                        if (widget->parent->children_head == widget->parent->children_tail) {
                                widget->parent->children_head = NULL;
                                widget->parent->children_tail = NULL;
                        } else {
                                widget->parent->children_head = widget->next_sibling;
                        }
                } else {
                        prev_widget = curr_widget;

                        while (curr_widget != widget && curr_widget != NULL) {
                                prev_widget = curr_widget;
                                curr_widget = curr_widget->next_sibling;
                        }

                        if (curr_widget != NULL) {
                                prev_widget->next_sibling = curr_widget->next_sibling;
                                if (curr_widget->next_sibling == NULL)
                                        widget->parent->children_tail = prev_widget;
                        }
                }
        }

        ei_widget_destroy_rec(widget);
}

/**
 * @brief	Gets a widget thanks to its picking id.
 *
 * @param	widget		Needs to be root.
 *
 * @return			The widget matching the picking id, or NULL if there is no match.
 */
static ei_widget_t* find_widget_from_id(ei_widget_t *widget, uint32_t id)
{
        if (widget != NULL) {
                if (widget->pick_id == id) {
                        return widget;
                } else {
                        ei_widget_t *children = widget->children_head;
                        ei_widget_t *sibling = widget->next_sibling;
                        ei_widget_t *result = find_widget_from_id(children, id);
                        return (result == NULL) ? find_widget_from_id(sibling, id) : result;
                }
        } else {
                return NULL;
        }
}

/**
 * @brief	Returns the widget that is at a given location on screen.
 *
 * @param	where		The location on screen, expressed in the root window coordinates.
 *
 * @return			The top-most widget at this location, or NULL if there is no widget
 *				at this location (except for the root widget).
 */
ei_widget_t* ei_widget_pick(ei_point_t* where)
{
        ei_surface_t *picking_surface = ei_picking_get_picking_surface();
        uint32_t *pixel = (uint32_t*) hw_surface_get_buffer(picking_surface);
        pixel += where->x + hw_surface_get_size(picking_surface).width * where->y;
        uint32_t id = color_to_id(pixel_to_color(picking_surface, *pixel));
        return (ei_app_root_widget()->pick_id == id) ? NULL : find_widget_from_id(ei_app_root_widget(), id);
}

/**
 * @brief	Configures the attributes of widgets of the class "frame".
 *
 *		Parameters obey the "default" protocol: if a parameter is "NULL" and it has never
 *		been defined before, then a default value should be used (default values are
 *		specified for each parameter). If the parameter is "NULL" but was defined on a
 *		previous call, then its value must not be changed.
 *
 * @param	widget		The widget to configure.
 * @param	requested_size	The size requested for this widget, including the widget's borders.
 *				The geometry manager may override this size due to other constraints.
 *				Defaults to the "natural size" of the widget, ie. big enough to
 *				display the border and the text or the image. This may be (0, 0)
 *				if the widget has border_width=0, and no text and no image.
 * @param	color		The color of the background of the widget. Defaults to
 *				\ref ei_default_background_color.
 * @param	border_width	The width in pixel of the border decoration of the widget. The final
 *				appearance depends on the "relief" parameter. Defaults to 0.
 * @param	relief		Appearance of the border of the widget. Defaults to
 *				\ref ei_relief_none.
 * @param	text		The text to display in the widget, or NULL. Only one of the
 *				parameter "text" and "img" should be used (i.e. non-NULL). Defaults
 *				to NULL.
 * @param	text_font	The font used to display the text. Defaults to \ref ei_default_font.
 * @param	text_color	The color used to display the text. Defaults to
 *				\ref ei_font_default_color.
 * @param	text_anchor	The anchor of the text, i.e. where it is placed within the widget.
 *				Defines both the anchoring point on the parent and on the widget.
 *				Defaults to \ref ei_anc_center.
 * @param	img		The image to display in the widget, or NULL. Any surface can be
 *				used, but usually a surface returned by \ref hw_image_load. Only one
 *				of the parameter "text" and "img" should be used (i.e. non-NULL).
 				Defaults to NULL.
 * @param	img_rect	If not NULL, this rectangle defines a subpart of "img" to use as the
 *				image displayed in the widget. Defaults to NULL.
 * @param	img_anchor	The anchor of the image, i.e. where it is placed within the widget
 *				when the size of the widget is bigger than the size of the image.
 *				Defaults to \ref ei_anc_center.
 */
void ei_frame_configure (ei_widget_t* widget,
                         ei_size_t* requested_size,
                         const ei_color_t* color,
                         int* border_width,
                         ei_relief_t* relief,
                         char** text,
                         ei_font_t* text_font,
                         ei_color_t* text_color,
                         ei_anchor_t* text_anchor,
                         ei_surface_t* img,
                         ei_rect_t** img_rect,
                         ei_anchor_t* img_anchor)
{
        ei_frame_t *frame = (ei_frame_t*) widget;
        if (color != NULL) *frame->color = *color;
        if (border_width != NULL) *frame->border_width = *border_width;
        if (relief != NULL) *frame->relief = *relief;
        if (text != NULL) {
                free(*frame->text);
                if (*text != NULL) {
                        *frame->text = calloc(strlen(*text) + 1, sizeof(char));
                        strcpy(*frame->text, *text);
                } else {
                        *frame->text = NULL;
                }
        }
        if (text_font != NULL) *frame->text_font = *text_font;
        if (text_color != NULL) *frame->text_color = *text_color;
        if (text_anchor != NULL) *frame->text_anchor = *text_anchor;
        if (img != NULL) {
                if (*img != NULL) {
                        if (*frame->img != NULL) hw_surface_free(*frame->img);
                        ei_size_t img_surf_size = hw_surface_get_size(*img);
                        ei_surface_t cpy_img = hw_surface_create(ei_app_root_surface(), img_surf_size, 1);
                        ei_copy_surface(cpy_img, NULL, *img, NULL, 0);
                        *frame->img = cpy_img;
                } else {
                        *frame->img = NULL;
                }
        }
        if (img_rect != NULL && *img_rect != NULL) {
                free(*frame->img_rect);
                ei_rect_t* cpy_img_rect = malloc(sizeof(ei_rect_t));
                cpy_img_rect->top_left.x = (**img_rect).top_left.x;
                cpy_img_rect->top_left.y = (**img_rect).top_left.y;
                cpy_img_rect->size = (**img_rect).size;
                (*frame->img_rect) = cpy_img_rect;
        }
        if (img_anchor != NULL) *frame->img_anchor = *img_anchor;
        if (requested_size != NULL) {
                widget->requested_size = *requested_size;
        } else {
                // Initialize the size of the text and of the image
                int text_width = 0;
                int text_height = 0;
                ei_size_t img_size = {0, 0};

                // Get those sizes
                if (*frame->text != NULL) hw_text_compute_size(*frame->text, *frame->text_font,
                                                               &text_width, &text_height);
                if (*frame->img_rect != NULL)  img_size = (*frame->img_rect)->size;

                // Set the minimal size according to the size of the text and of the image
                ei_size_t min_size = {text_width + img_size.width, text_height + img_size.height};

                // Set requested_size to a proper size
                widget->requested_size.width = min_size.width;
                widget->requested_size.height = min_size.height;
        }
        ei_app_invalidate_rect(widget->content_rect);
}

/**
 * @brief	Configures the attributes of widgets of the class "button".
 *
 * @param	widget, requested_size, color, border_width, relief,
 *		text, text_font, text_color, text_anchor,
 *		img, img_rect, img_anchor
 *				See the parameter definition of \ref ei_frame_configure. The only
 *				difference is that relief defaults to \ref ei_relief_raised
 *				and border_width defaults to \ref k_default_button_border_width.
 * @param	corner_radius	The radius (in pixels) of the rounded corners of the button.
 *				0 means straight corners. Defaults to \ref k_default_button_corner_radius.
 * @param	callback	The callback function to call when the user clicks on the button.
 *				Defaults to NULL (no callback).
 * @param	user_param	A programmer supplied parameter that will be passed to the callback
 *				when called. Defaults to NULL.
 */
void ei_button_configure (ei_widget_t* widget,
                          ei_size_t* requested_size,
                          const ei_color_t* color,
                          int* border_width,
                          int* corner_radius,
                          ei_relief_t* relief,
                          char** text,
                          ei_font_t* text_font,
                          ei_color_t* text_color,
                          ei_anchor_t* text_anchor,
                          ei_surface_t* img,
                          ei_rect_t** img_rect,
                          ei_anchor_t* img_anchor,
                          ei_callback_t* callback,
                          void** user_param)
{
        ei_button_t *button = (ei_button_t*) widget;
        if (color != NULL) *button->color = *color;
        if (border_width != NULL) *button->border_width = *border_width;
        if (corner_radius != NULL) *button->corner_radius = *corner_radius;
        if (relief != NULL) *button->relief = *relief;
        if (text != NULL) {
                free(*button->text);
                if (*text != NULL) {
                        *button->text = calloc(strlen(*text) + 1, sizeof(char));
                        strcpy(*button->text, *text);
                } else {
                        *button->text = NULL;
                }
        }
        if (text_font != NULL) *button->text_font = *text_font;
        if (text_color != NULL) *button->text_color = *text_color;
        if (text_anchor != NULL) *button->text_anchor = *text_anchor;
        if (img != NULL) {
                if (*img != NULL) {
                        if (*button->img != NULL) hw_surface_free(*button->img);
                        ei_size_t img_surf_size = hw_surface_get_size(*img);
                        ei_surface_t cpy_img = hw_surface_create(ei_app_root_surface(), img_surf_size, 1);
                        ei_copy_surface(cpy_img, NULL, *img, NULL, 0);
                        *button->img = cpy_img;
                } else {
                        *button->img = NULL;
                }
        }
        if (img_rect != NULL && *img_rect != NULL) {
                free(*button->img_rect);
                ei_rect_t* cpy_img_rect = malloc(sizeof(ei_rect_t));
                cpy_img_rect->top_left.x = (**img_rect).top_left.x;
                cpy_img_rect->top_left.y = (**img_rect).top_left.y;
                cpy_img_rect->size = (**img_rect).size;
                (*button->img_rect) = cpy_img_rect;
        }
        if (img_anchor != NULL) *button->img_anchor = *img_anchor;
        if (callback != NULL) *button->callback = *callback;
        if (user_param != NULL) *button->user_param = *user_param;
        if (requested_size != NULL) {
                widget->requested_size = *requested_size;
        } else {
                // Initialize the size of the text and of the image
                int text_width = 0;
                int text_height = 0;
                ei_size_t img_size = {0, 0};

                // Get those sizes
                if (*button->text != NULL) {
                        hw_text_compute_size(*button->text, *button->text_font,
                                             &text_width, &text_height);
                }
                if (*button->img != NULL)  {
                        if (*button->img_rect != NULL) img_size = (*button->img_rect)->size;
                        else img_size = hw_surface_get_size(*button->img);
                }

                // Set the minimal size according to the size of the text and of the image
                ei_size_t min_size = {text_width + img_size.width, text_height + img_size.height};

                // Set requested_size to a proper size
                widget->requested_size.width = min_size.width;
                widget->requested_size.height = min_size.height;
        }
        ei_app_invalidate_rect(widget->content_rect);
}

/**
 * @brief	Configures the attributes of widgets of the class "toplevel".
 *
 * @param	widget		The widget to configure.
 * @param	requested_size	The content size requested for this widget, this does not include
 *				the decorations	(border, title bar). The geometry manager may
 *				override this size due to other constraints.
 *				Defaults to (320x240).
 * @param	color		The color of the background of the content of the widget. Defaults
 *				to \ref ei_default_background_color.
 * @param	border_width	The width in pixel of the border of the widget. Defaults to 4.
 * @param	title		The string title displayed in the title bar. Defaults to "Toplevel".
 *				Uses the font \ref ei_default_font.
 * @param	closable	If true, the toplevel is closable by the user, the toplevel must
 *				show a close button in its title bar. Defaults to \ref EI_TRUE.
 * @param	resizable	Defines if the widget can be resized horizontally and/or vertically
 *				by the user. Defaults to \ref ei_axis_both.
 * @param	min_size	For resizable widgets, defines the minimum size of its content.
 *				The default minimum size of a toplevel is (160, 120).
 *				If *min_size is NULL, this requires the toplevel to be configured to
 *				its default size.
 */
void			ei_toplevel_configure		(ei_widget_t*		widget,
                                                                  ei_size_t*		requested_size,
                                                                  ei_color_t*		color,
                                                                  int*			border_width,
                                                                  char**			title,
                                                                  ei_bool_t*		closable,
                                                                  ei_axis_set_t*		resizable,
                                                                  ei_size_t**		min_size)
{
        ei_toplevel_t *toplevel = (ei_toplevel_t*) widget;
        if (color != NULL) *toplevel->color = *color;
        if (border_width != NULL) *toplevel->border_width = *border_width;
        if (title != NULL & *title != NULL) {
                if (strcmp(*toplevel->title,"Toplevel")) free(*toplevel->title);
                *toplevel->title = calloc(strlen(*title)+1, sizeof(char));
                strcpy(*toplevel->title, *title);
        }
        if (closable != NULL) *toplevel->closable = *closable;
        if (resizable != NULL) *toplevel->resizable = *resizable;
        if (min_size != NULL && *min_size != NULL) *toplevel->min_size = *min_size;
        if (requested_size != NULL) {
                // Set requested_size to a proper size
                if (min_size != NULL && *min_size != NULL) {
                        requested_size->width = ((*min_size)->width < requested_size->width) ?
                        requested_size->width : (*min_size)->width;
                        requested_size->height = ((*min_size)->height < requested_size->height) ?
                                                 requested_size->height : (*min_size)->height;
                }
                // Apply to the widget
                widget->requested_size = *requested_size;
        } else {
                if (min_size != NULL && *min_size != NULL) widget->requested_size = **min_size;
                else widget->requested_size = **toplevel->min_size;
        }
        ei_app_invalidate_rect(widget->content_rect);
}