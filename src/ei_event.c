#include "ei_event.h"
#include "ei_application.h"
#include "ei_toplevel.h"

static ei_widget_t *active_widget = NULL;
static ei_default_handle_func_t default_handle = NULL;

/**
 * Puts the widget in the foreground.
 *
 * @param	widget		The widget to put on the foreground.
 */
static void focus(ei_widget_t *widget)
{
        while (widget->parent != NULL) {
                if (strcmp(ei_widgetclass_stringname(widget->wclass->name), "toplevel") == 0) {
                        if (widget->parent->children_tail != widget) {
                                if (widget->parent->children_head == widget)
                                        widget->parent->children_head = widget->next_sibling;
                                else {
                                        ei_widget_t sent;
                                        sent.next_sibling = widget->parent->children_head;
                                        ei_widget_t *senti = &sent;
                                        while (senti->next_sibling != widget) senti = senti->next_sibling;
                                        senti->next_sibling = senti->next_sibling->next_sibling;
                                }
                                widget->parent->children_tail->next_sibling = widget;
                                widget->next_sibling = NULL;
                                widget->parent->children_tail = widget;
                                ei_rect_t rect2invalidate = widget->screen_location;
                                ei_toplevel_t *toplevel = (ei_toplevel_t *) widget;
                                int title_width = 0;
                                int title_height = 0;
                                hw_text_compute_size(*toplevel->title, ei_default_font,
                                                     &title_width, &title_height);
                                rect2invalidate.size.height += title_height + 2 * *toplevel->border_width;
                                rect2invalidate.size.width += 2 * *toplevel->border_width;
                                ei_app_invalidate_rect(&rect2invalidate);
                        }
                }
                widget = widget->parent;
        }
}

/**
 * Sets the widget which is currently being manipulated by the user.
 *
 * @param	widget		The widget to declare as active, or NULL to declare
 *				that a widget is no more being manipulated.
 */
void ei_event_set_active_widget(ei_widget_t* widget)
{
        active_widget = widget;
        if (widget != NULL) focus(widget);
}

/**
 * Returns the widget currently being manipulated by the user.
 *
 * @return			The widget currenlty being manipulated, or NULL.
 */
ei_widget_t* ei_event_get_active_widget(void)
{
        return active_widget;
}

/**
 * Sets the function that must be called when an event has been received but no processed
 *	by any widget.
 *
 * @param	func		The event handling function.
 */
void ei_event_set_default_handle_func(ei_default_handle_func_t func)
{
        default_handle = func;
}

/**
 * Returns the function that must be called when an event has been received but no processed
 *	by any widget.
 *
 * @return			The address of the event handling function.
 */
ei_default_handle_func_t ei_event_get_default_handle_func(void)
{
        return default_handle;
}