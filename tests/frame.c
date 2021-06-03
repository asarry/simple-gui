#include <stdio.h>
#include <stdlib.h>

#include "ei_application.h"
#include "ei_event.h"
#include "hw_interface.h"
#include "ei_widget.h"

/*
 * process_key --
 *
 *	Callback called when any key is pressed by the user.
 *	Simply looks for the "Escape" key to request the application to quit.
 */
ei_bool_t process_key(ei_event_t* event)
{
        if (event->type == ei_ev_keydown)
                if (event->param.key.key_code == SDLK_ESCAPE) {
                        ei_app_quit_request();
                        return EI_TRUE;
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
	ei_size_t	screen_size		= {600, 600};
	ei_color_t	root_bgcol		= {0x52, 0x7f, 0xb4, 0xff};

	ei_widget_t*	frame;
	ei_size_t	frame_size		= {300,200};
	int		frame_x			= 300;
	int		frame_y			= 300;
	ei_color_t	frame_color		= {0x88, 0x88, 0x88, 0xff};
	ei_relief_t	frame_relief		= ei_relief_raised;
	int		frame_border_width	= 6;
        char*		frame_title		= "Ma première Frame !";
        ei_anchor_t 	frame_txt_anchor	= ei_anc_west;
        ei_color_t	frame_text_color	= {0x00, 0x00, 0x00, 0xff};

	/* Create the application and change the color of the background. */
	ei_app_create(screen_size, EI_FALSE);
	ei_frame_configure(ei_app_root_widget(), NULL, &root_bgcol, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
        ei_event_set_default_handle_func(process_key);

	/* Create, configure and place the frame on screen. */
        ei_font_t frame_font = hw_text_font_create(ei_default_font_filename,ei_style_italic,22);
        ei_surface_t frame_image = hw_image_load("misc/klimt.jpg",ei_app_root_surface());
        ei_rect_t frame_img_rect = {{100,100},{100,100}};
        ei_rect_t *frame_img_rect_ptr = &frame_img_rect;
	frame = ei_widget_create("frame", ei_app_root_widget(), NULL, NULL);
	ei_frame_configure	(frame, &frame_size, &frame_color,
				 &frame_border_width, &frame_relief, &frame_title, &frame_font,
				 &frame_text_color, &frame_txt_anchor,
				 &frame_image, &frame_img_rect_ptr, &frame_txt_anchor);

	ei_anchor_t frame_anc = ei_anc_center;
	ei_place(frame, &frame_anc, &frame_x, &frame_y, NULL, NULL, NULL, NULL, NULL, NULL );

	/* Run the application's main loop. */
	ei_app_run();

	/* We just exited from the main loop. Terminate the application (cleanup). */
	ei_app_free();

	return (EXIT_SUCCESS);
}
