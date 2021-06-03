#include "ei_tools.h"

/**
 * \brief	Gets the minimum between two unsigned char.
 *
 * @param	element_1	The first element.
 * @param	element_2	The second element.
 *
 * @return	        	The unsigned char which is the lower one.
 */
unsigned char min(unsigned char element_1, unsigned char element_2)
{
        return (element_1 < element_2) ? element_1 : element_2;
}

/**
 * \brief	Gets the maximum between two unsigned char.
 *
 * @param	element_1	The first element.
 * @param	element_2	The second element.
 *
 * @return	        	The unsigned char which is the bigger one.
 */
unsigned char max(unsigned char element_1, unsigned char element_2)
{
        return (element_1 > element_2) ? element_1 : element_2;
}

/**
 * \brief	Gets the color matching with a picking ID.
 *
 * @param	pick_id 	The picking ID.
 *
 * @return	        	The color which match.
 */
ei_color_t* id_to_color(uint32_t pick_id)
{
        ei_color_t *color = calloc(1, sizeof(ei_color_t));
        color->red = 0;
        color->green = 0;
        color->blue = 0;
        color->alpha = -1;

        while (pick_id >= 65536) {
                pick_id -= 65536;
                color->red++;
        }

        while (pick_id >= 256) {
                pick_id -= 256;
                color->green++;
        }

        color->blue = pick_id;
        return color;
}

/**
 * \brief	Gets the color from a pixel.
 *
 * @param	surface 	The surface which defines the channel ordering.
 * @param	pixel    	The pixel.
 *
 * @return	        	The color of the pixel.
 */
ei_color_t pixel_to_color(ei_surface_t surface, uint32_t pixel)
{
        int ir, ig, ib, ia;
        hw_surface_get_channel_indices(surface, &ir, &ig, &ib, &ia);
        int bitmask = 0xFF;
        uint8_t red = (pixel) >> (ir * 8)&bitmask;
        uint8_t green = (pixel) >> (ig * 8)&bitmask;
        uint8_t blue = (pixel) >> (ib * 8)&bitmask;
        uint8_t alpha = (pixel) >> (ia * 8)&bitmask;
        return (ei_color_t) {red, green, blue, alpha};
}

/**
 * \brief	Computes a picking ID using a color.
 *
 * @param	color    	The color to use.
 *
 * @return	        	The computed picking ID.
 */
uint32_t color_to_id(ei_color_t color)
{
        return color.red * 65536 + color.green * 256 + color.blue;
}

/**
 * \brief	Frees a linked_point structure.
 *
 * @param	point    	The head of the linked_point structure.
 *
 */
void free_linked_points(ei_linked_point_t* point)
{
        ei_linked_point_t *tmp;
        while (point != NULL) {
                tmp = point->next;
                free(point);
                point = tmp;
        }
}