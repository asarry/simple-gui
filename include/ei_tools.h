#ifndef EI_TOOLS_H
#define EI_TOOLS_H

#include <stdint.h>
#include "ei_types.h"
#include "hw_interface.h"

/**
 * \brief	Gets the minimum between two unsigned char.
 *
 * @param	element_1	The first element.
 * @param	element_2	The second element.
 *
 * @return	        	The unsigned char which is the lower one.
 */
unsigned char min(unsigned char element_1, unsigned char element_2);

/**
 * \brief	Gets the maximum between two unsigned char.
 *
 * @param	element_1	The first element.
 * @param	element_2	The second element.
 *
 * @return	        	The unsigned char which is the bigger one.
 */
unsigned char max(unsigned char element_1, unsigned char element_2);

/**
 * \brief	Gets the color matching with a picking ID.
 *
 * @param	pick_id 	The picking ID.
 *
 * @return	        	The color which match.
 */
ei_color_t* id_to_color(uint32_t pick_id);

/**
 * \brief	Gets the color from a pixel.
 *
 * @param	surface 	The surface which defines the channel ordering.
 * @param	pixel    	The pixel.
 *
 * @return	        	The color of the pixel.
 */
ei_color_t pixel_to_color(ei_surface_t surfacae, uint32_t pixel);

/**
 * \brief	Computes a picking ID using a color.
 *
 * @param	color    	The color to use.
 *
 * @return	        	The computed picking ID.
 */
uint32_t  color_to_id(ei_color_t color);

/**
 * \brief	Frees a linked_point structure.
 *
 * @param	point    	The head of the linked_point structure.
 *
 */
void free_linked_points(ei_linked_point_t* point);

#endif //EI_TOOLS_H
