#include <stdio.h>
#include <stdlib.h>

#include "ei_application.h"
#include "ei_event.h"
#include "hw_interface.h"
#include "ei_widget.h"


/*
 * button_press --
 *
 *	Callback called when a user clicks on the button.
 */
void button_press(ei_widget_t* widget, ei_event_t* event, void* user_param)
{
	printf("Click !\n");
}

/*
 * process_key --
 *
 *	Callback called when any key is pressed by the user.
 *	Simply looks for the "Escape" key to request the application to quit.
 */
ei_bool_t process_key(ei_event_t* event)
{
	if (event->type == ei_ev_keydown) {
                if (event->param.key.key_code == SDLK_ESCAPE) {
                        ei_app_quit_request();
                        return EI_TRUE;
                }
                if (event->param.key.key_code == SDLK_UP) {
                        printf("UP !\n");
                }
        }
	return EI_FALSE;
}

/*
 * ei_main --
 *
 *	Main function of the application.
 */
int main(int argc, char** argv)
{
	ei_size_t	screen_size		= {800, 600};
	ei_color_t	root_bgcol		= {0x52, 0x7f, 0xb4, 0xff};

	ei_widget_t*	button;
	ei_anchor_t	button_anchor		= ei_anc_northwest;
	int		button_x		= 50;
	int		button_y		= 50;
	float		button_rel_x		= 0;
	float		button_rel_y		= 0;
	float		button_rel_width	= 0.5;
	ei_color_t	button_color		= {0x88, 0x88, 0x88, 0xff};
	char*		button_title		= "click";
	ei_color_t	button_text_color	= {0x00, 0x00, 0x00, 0xff};
	ei_relief_t	button_relief		= ei_relief_raised;
	int		button_border_width	= 2;
	ei_callback_t	button_callback 	= button_press;

	ei_widget_t*	window;
	ei_size_t	window_size		= {400,400};
	char*		window_title		= "Hello World";
	//char*           window_title            = NULL;
	ei_color_t	window_color		= {0xA0,0xA0,0xA0, 0xff};
	int		window_border_width	= 20;
	ei_bool_t	window_closable		= EI_TRUE;
	ei_axis_set_t	window_resizable	= ei_axis_both;
	ei_point_t	window_position		= {50, 50};
	ei_size_t       window_min_size         = {100,100};
	//ei_size_t*      window_min_size_ptr     = &window_min_size;
	ei_size_t*      window_min_size_ptr     = NULL;

        ei_widget_t*	window2;
        ei_size_t	window2_size		= {200,200};
        char*           window2_title            = "HELLOOOOOOOOOO";
        ei_color_t	window2_color		= {0xA0,0xA0,0xA0, 0xff};
        int		window2_border_width	= 2;
        float		window2_rel_x		= 0.5;
        float		window2_rel_y		= 0.5;
        float		window2_rel_w		= 0.5;
        ei_anchor_t     window2_anc               = ei_anc_center;
        ei_bool_t	window2_closable		= EI_TRUE;
        ei_axis_set_t	window2_resizable	= ei_axis_both;
        ei_point_t	window2_position		= {0, 0};
        ei_size_t       window2_min_size         = {150,100};
        ei_size_t*      window2_min_size_ptr     = &window_min_size;

        ei_widget_t*	frame;
        ei_size_t	frame_size		= {512,512};
        ei_anchor_t     frame_anc               = ei_anc_northwest;
        int		frame_x			= 0;
        int		frame_y			= 0;
        float		frame_rel_x		= 0;
        float		frame_rel_y		= 0;
        float		frame_rel_width	        = 1;
        ei_color_t	frame_color		= {0x88, 0x88, 0x88, 0xff};
        ei_relief_t	frame_relief		= ei_relief_raised;
        int		frame_border_width	= 6;
        //char*		frame_title		= "Ma première Frame !";
        char*           frame_title             = NULL;
        ei_anchor_t 	frame_txt_anchor	= ei_anc_center;
        ei_color_t	frame_text_color	= {0x00, 0x00, 0x00, 0xff};

	/* Create the application and change the color of the background. */
	ei_app_create(screen_size, EI_FALSE); 
	ei_frame_configure(ei_app_root_widget(), NULL, &root_bgcol, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	ei_event_set_default_handle_func(process_key);

	/* Create, configure and place a toplevel window on screen. */
	window = ei_widget_create("toplevel", ei_app_root_widget(), NULL, NULL);
	ei_toplevel_configure(window, &window_size, NULL, &window_border_width,
				&window_title, &window_closable, &window_resizable, &window_min_size_ptr);
	ei_place(window, NULL, &window_position.x, &window_position.y, NULL, NULL, NULL, NULL,
          NULL, NULL);

	window2 = ei_widget_create("toplevel", ei_app_root_widget(), NULL, NULL);
	ei_toplevel_configure(window2, &window2_size, NULL, &window2_border_width,
				&window2_title, &window2_closable, &window2_resizable, &window2_min_size_ptr);
	ei_place(window2, &window2_anc, &window2_position.x, &window2_position.y, NULL, NULL,
          &window2_rel_x,
          &window2_rel_y,
          NULL, &window2_rel_w);

	/* Create, configure and place a button as a descendant of the toplevel window. */
	button = ei_widget_create("button", window, NULL, NULL);
	ei_button_configure	(button, NULL, &button_color, 
				 &button_border_width, NULL, &button_relief, &button_title, NULL, &button_text_color, NULL,
				NULL, NULL, NULL, &button_callback, NULL);
	ei_place(button, &button_anchor, &button_x, &button_y, NULL, NULL, &button_rel_x, &button_rel_y, &button_rel_width, NULL);

        /* Create, configure and place the frame on screen. */
        ei_font_t frame_font = hw_text_font_create(ei_default_font_filename,ei_style_italic,22);
        ei_surface_t frame_image = hw_image_load("misc/klimt.jpg",ei_app_root_surface());
        ei_rect_t frame_img_rect = {{0,0},{512,512}};
        ei_rect_t *frame_img_rect_ptr = &frame_img_rect;
        frame = ei_widget_create("frame", window2, NULL, NULL);
        ei_frame_configure	(frame, &frame_size, &frame_color,
                                   &frame_border_width, &frame_relief, NULL, &frame_font,
                                   &frame_text_color, &frame_txt_anchor,
                                   &frame_image, &frame_img_rect_ptr, &frame_anc);

        ei_place(frame, &frame_anc, &frame_x, &frame_y, NULL, NULL, &frame_rel_x, &frame_rel_y,
                 NULL,NULL);
	/* Run the application's main loop. */
	ei_app_run();

	ei_app_free();

	return (EXIT_SUCCESS);
}
