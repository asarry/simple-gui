#include "ei_application.h"
#include "ei_button.h"
#include "ei_event.h"
#include "ei_frame.h"
#include "ei_picking.h"
#include "ei_placer.h"
#include "ei_toplevel.h"

static ei_surface_t *root_surface = NULL;
static ei_widget_t *root_widget = NULL;
static ei_surface_t *picking_surface = NULL;
static ei_bool_t quit_request = EI_FALSE;
static ei_linked_rect_t *invalidate_list = NULL;

/**
 * \brief	Creates an application.
 *		<ul>
 *			<li> initializes the hardware (calls \ref hw_init), </li>
 *			<li> registers all classes of widget, </li>
 *			<li> creates the root window (either in a system window, or the entire screen), </li>
 *			<li> creates the root widget to access the root window. </li>
 *		</ul>
 *
 * @param	main_window_size	If "fullscreen is false, the size of the root window of the
 *					application.
 *					If "fullscreen" is true, the current monitor resolution is
 *					used as the size of the root window. \ref hw_surface_get_size
 *					can be used with \ref ei_app_root_surface to get the size.
 * @param	fullScreen		If true, the root window is the entire screen. Otherwise, it
 *					is a system window.
 */
void ei_app_create(ei_size_t main_window_size, ei_bool_t fullscreen)
{
        // Initialize the graphical layer
        hw_init();

        // Register the available widget classes
        ei_widgetclass_register(&frameclass);
        ei_widgetclass_register(&buttonclass);
        ei_widgetclass_register(&toplevelclass);

        // Create the root window
        root_surface = hw_create_window(main_window_size, fullscreen);

        // Create the root widget
        root_widget = ei_widget_create("frame", NULL, NULL, NULL);
        root_widget->screen_location = hw_surface_get_rect(root_surface);

        // Create an offscreen surface for the picking
        ei_picking_set_picking_surface(hw_surface_create(root_surface, main_window_size, EI_TRUE));
        picking_surface = ei_picking_get_picking_surface();
}

/**
 * \brief	Releases all the resources of the application, and releases the hardware
 *		(ie. calls \ref hw_quit).
 */
void ei_app_free(void)
{
        ei_linked_rect_t *to_free = invalidate_list;
        ei_linked_rect_t *temp = NULL;
        while (to_free) {
                temp = to_free->next;
                free(to_free);
                to_free = temp;
        }
        ei_widget_destroy(root_widget);
        hw_surface_free(root_surface);
        hw_surface_free(picking_surface);
        hw_quit();
}

/**
 * \brief	Runs the application: enters the main event loop. Exits when
 *		\ref ei_app_quit_request is called.
 */
void ei_app_run(void)
{
        ei_event_t *event = calloc(1, sizeof(ei_event_t));
        ei_widget_t *active_widget = NULL;
        ei_default_handle_func_t default_handle_func = ei_event_get_default_handle_func();
        ei_bool_t handled = EI_FALSE;
        ei_linked_rect_t *first_inv_rect = calloc(1, sizeof(ei_linked_rect_t));
        first_inv_rect->rect = root_widget->screen_location;
        first_inv_rect->next = NULL;
        invalidate_list = first_inv_rect;

        while (!quit_request) {
                if (invalidate_list != NULL) {
                        hw_surface_lock(root_surface);
                        ei_linked_rect_t* curr_rect = invalidate_list;
                        while (curr_rect != NULL) {
                                root_widget->wclass->drawfunc(root_widget, root_surface, picking_surface,
                                                              &curr_rect->rect);
                                ei_widget_t *widget = root_widget->children_head;
                                while (widget != NULL) {
                                        ei_placer_run(widget);
                                        widget->wclass->drawfunc(widget, root_surface, picking_surface,
                                                                 &curr_rect->rect);
                                        widget = widget->next_sibling;
                                }
                                curr_rect = curr_rect->next;
                        }
                        hw_surface_unlock(root_surface);
                        hw_surface_update_rects(root_surface, invalidate_list);
                }

                ei_linked_rect_t *to_free = invalidate_list;
                ei_linked_rect_t *temp = NULL;
                while (to_free) {
                        temp = to_free->next;
                        free(to_free);
                        to_free = temp;
                }
                invalidate_list = NULL;

                hw_event_wait_next(event);

                if (event->type == ei_ev_mouse_buttondown) ei_event_set_active_widget(ei_widget_pick(&event->param.mouse.where));

                active_widget = ei_event_get_active_widget();
                if (active_widget != NULL) {
                        handled = active_widget->wclass->handlefunc(active_widget, event);
                        if (!handled) default_handle_func(event);
                } else if (default_handle_func != NULL) {
                        default_handle_func(event);
                }
        }
        free(event);
}

/**
 * \brief	Adds a rectangle to the list of rectangles that must be updated on screen. The real
 *		update on the screen will be done at the right moment in the main loop.
 *
 * @param	rect		The rectangle to add, expressed in the root window coordinates.
 *				A copy is made, so it is safe to release the rectangle on return.
 */
void ei_app_invalidate_rect(ei_rect_t* rect)
{
        if (rect->size.height == 0 && rect->size.width == 0) return;
        ei_rect_t inside_rect = rectangle_intersect(root_widget->content_rect,rect);
        if (inside_rect.size.height == 0 && inside_rect.size.width == 0) return;
        ei_linked_rect_t *new_rect = calloc(1, sizeof(ei_linked_rect_t));
        new_rect->rect = inside_rect;
        if (invalidate_list == NULL) {
                invalidate_list = new_rect;
        } else {
                ei_linked_rect_t* explore = invalidate_list;
                while (explore->next) {
                        explore = explore->next;
                }
                explore->next = new_rect;
        }
}

/**
 * \brief	Tells the application to quite. Is usually called by an event handler (for example
 *		when pressing the "Escape" key).
 */
void ei_app_quit_request(void)
{
        quit_request = EI_TRUE;
}

/**
 * \brief	Returns the "root widget" of the application: a "frame" widget that span the entire
 *		root window.
 *
 * @return 			The root widget.
 */
ei_widget_t* ei_app_root_widget(void)
{
        return root_widget;
}

/**
 * \brief	Returns the surface of the root window. Can be usesd to create surfaces with similar
 * 		r, g, b channels.
 *
 * @return 			The surface of the root window.
 */
ei_surface_t ei_app_root_surface(void)
{
        return root_surface;
}