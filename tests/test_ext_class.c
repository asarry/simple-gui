//
//  test_ext_class.c
//  projet_c_IG
//
//  Created by François Bérard on 1/6/16.
//  Copyright © 2016 LIG-IIHM. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>

#include "ext_testclass.h"

#include "ei_utils.h"
#include "ei_application.h"
#include "ei_event.h"
#include "hw_interface.h"
#include "ei_widget.h"

ei_widget_t*		g_test_widget;
char*			g_label_ok	= "Ok";
char*			g_label_cancel	= "Cancel";




// Uncomment either one of the two following lines.
//	If true, the testclass drawfunc draws a widget *and its children*.
//	If false, the testclass drawfunc only draws the widget, not its children.

static const ei_bool_t	k_drawfunc_draws_children	= EI_TRUE;
// static const ei_bool_t	k_drawfunc_draws_children	= EI_FALSE;


// global_event_handler --

ei_bool_t global_event_handler(ei_event_t* event)
{
	if (event->param.key.key_code == SDLK_ESCAPE) {
		ei_app_quit_request();
		return EI_TRUE;
	}
        return EI_FALSE;
}


void button_callback(ei_widget_t* widget, struct ei_event_t* event, void* user_param)
{
	static ei_bool_t	flip	= EI_FALSE;
	char**			label;

	label			= flip ? &g_label_ok : &g_label_cancel;
	ei_button_configure(widget, NULL, NULL, NULL, NULL, NULL, label, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	flip			= !flip;
}


// ei_main --

int main(int argc, char** argv)
{
	ei_size_t	screen_size		= {800, 800};
        ei_color_t	root_bgcol		= {0x52, 0x7f, 0xb4, 0xff};
	ei_anchor_t	anchor_c		= ei_anc_center;
	float		fl_1_0			= 1.0;
	float		fl_0_5			= 0.5;
	
        ei_widget_t*	top;
        ei_size_t	top_size		= {400,400};
        char*		top_title		= "Test nouvelle classe de widget";
        ei_color_t	top_color		= {0xA0,0xA0,0xA0, 0xff};
        int		top_border_width	= 2;
        ei_bool_t	top_closable		= EI_TRUE;
        ei_axis_set_t	top_resizable		= ei_axis_both;
        ei_point_t	top_position		= {30, 10};
        ei_size_t	top_min_size		= {50, 50};
        ei_size_t*	top_min_size_ptr	= &top_min_size;

	ei_size_t	test_sz;

	ei_widget_t*	but;
	ei_anchor_t	but_text_anchor		= ei_anc_west;
	ei_callback_t	but_cb		 	= &button_callback;

	// Initialize the application, set the background color.

	ei_app_create(screen_size, EI_FALSE);
        ei_frame_configure(ei_app_root_widget(), NULL, &root_bgcol, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);        

	// Register the new "testclass" widget class.

	ext_testclass_register_class(k_drawfunc_draws_children); // See k_drawfunc_draws_children comment above.

	// Create a toplevel, place it on the screen.

        top = ei_widget_create("toplevel", ei_app_root_widget(), NULL, NULL);
        ei_toplevel_configure(top, &top_size, &top_color, &top_border_width, &top_title,
								&top_closable, &top_resizable, &top_min_size_ptr);
        ei_place(top, NULL, &(top_position.x), &(top_position.y), NULL, NULL, NULL, NULL, NULL, NULL);

	// Create the testclass widget as a child of the toplevel, use relative placement.

	g_test_widget = ei_widget_create("testclass", top, NULL, NULL);
	ext_testclass_configure(g_test_widget, 8);
	test_sz		= ei_size(-40, -40);
	ei_place(g_test_widget, &anchor_c, NULL, NULL, &(test_sz.width), &(test_sz.height), &fl_0_5, &fl_0_5, &fl_1_0, &fl_1_0);

	// Create a button as a child of the testclass widget, place it.

	but = ei_widget_create("button", g_test_widget, NULL, NULL);

	ei_button_configure(but, NULL, NULL, NULL, NULL, NULL, &g_label_ok, NULL, NULL, &but_text_anchor, NULL, NULL, NULL, &but_cb, NULL);
	ei_place(but, &anchor_c, NULL, NULL, NULL, NULL, &fl_1_0, &fl_0_5, &fl_1_0, NULL);

	// Run the main loop, bind the "Esc" key to quitting the application.

	ei_event_set_default_handle_func(global_event_handler);

        ei_app_run();

	ei_app_free();
	return (EXIT_SUCCESS);
}


