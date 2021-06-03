#include <stdlib.h>
#include <math.h>
#include "ei_types.h"
#include "ei_event.h"
#include "ei_utils.h"
#include "hw_interface.h"
#include "ei_button.h"


int main(int argc, char* argv[])
{
	ei_surface_t			main_window		= NULL;
	ei_size_t			main_window_size	= ei_size(220, 30);
	ei_event_t			event;
        ei_color_t		white		= { 0xe9, 0x97, 0x7a, 0xff };
        //ei_color_t		white		= { 0xff, 0xff, 0xff, 0xff };
	uint32_t*			pixel_ptr;
        ei_rect_t*		clipper_ptr	= NULL;
        ei_rect_t		clipper		= ei_rect(ei_point(0, 0),ei_size(400, 300));
        //clipper_ptr		= &clipper;

	// Init acces to hardware.
	hw_init();

	// Create the main window.
	main_window = hw_create_window(main_window_size, EI_FALSE);

	// Lock the surface for drawing, fill in white, unlock, update screen.
	hw_surface_lock(main_window);
	
        char*		button_title		= "Mon premier Bouton !";
        ei_color_t	button_text_color	= {0x00, 0x00, 0x00, 0xff};
        int            text_width              = 0;
        int            text_height             = 0;

        ei_fill		(main_window, &white, NULL);
        ei_point_t where = {0,0};
        ei_font_t font = hw_text_font_create("misc/font.ttf",ei_style_normal,1*ei_font_default_size);
        //hw_text_compute_size(button_title,ei_default_font,&text_width,&text_height);
        //clipper_ptr->size.width = text_width;
        //clipper_ptr->size.height = text_height;
        ei_draw_text(main_window,&where,button_title,font,button_text_color, clipper_ptr);
	hw_surface_unlock(main_window);

	hw_surface_update_rects(main_window, NULL);

	// Wait for a key press.
	event.type = ei_ev_none;
	while (event.type != ei_ev_mouse_buttondown)
		hw_event_wait_next(&event);

	// Free hardware resources.
	hw_quit();

	// Terminate program with no error code.
	return 0;
}
