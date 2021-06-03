#ifndef EI_DRAWING_TOOLS_H
#define EI_DRAWING_TOOLS_H

#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include "ei_draw.h"
#include "ei_event.h"
#include "ei_frame.h"
#include "ei_tools.h"
#include "ei_types.h"
#include "ei_utils.h"

/**
 * Returns a lighter version of the color passed as argument.
 *
 * @param	color		The original color.
 *
 * @return			The lighter version of the color.
 */
ei_color_t get_light_color_variation(const ei_color_t* color);

/**
 * Returns a darker version of the color passed as argument.
 *
 * @param	color		The original color.
 *
 * @return			The darker version of the color.
 */
ei_color_t get_dark_color_variation(const ei_color_t* color);

/**
 * \brief	Returns a list of points describing an arc.
 *
 * @param	center  	Center of the arc.
 * @param	radius          Radius of the arc.
 * @param	init_angle 	Position of the first point of the arc.
 * @param	fin_angle	Position of the last point of the arc.
 *                              This function uses anticlockwise direction and fin_angle needs
 *                              to be higher than init_angle.
 *
 * @return			The list of points describing the arc.
 */
ei_linked_point_t *arc(ei_point_t centre,int radius,float angle_init,float angle_fin);

/**
 * \brief	Returns a list of points describing a rounded_frame, or a part (upper or lower) of it.
 *
 * @param	rectangle  	The rectangle used to know dimensions of the rounded frame.
 * @param	radius          Radius of the rounded corners.
 * @param	part 	        The part to describe.
 *
 * @return			The list of points describing the rounded frame.
 */
ei_linked_point_t *rounded_frame(ei_rect_t rectangle, int radius, char partie);

/**
 * \brief	Returns the intersection of two rectangles.
 *
 * @param	first_rect  	The first rectangle. If NULL, returns sec_rect.
 * @param	sec_rect        The second rectangle.
 *
 * @return			A rectangle which is the intersection of first_rect and sec_rect.
 *                              If there is no intersection, it returns a rectangle with all its
 *                              parameters set to 0.
 */
ei_rect_t rectangle_intersect(ei_rect_t* first_rect, ei_rect_t* sec_rect);

/**
 * \brief	Sets the coordinates of a topleft point regarding the anchor and the size of
 *              the object to anchor.
 *
 * @param	anchor  	The anchor.
 * @param	where           The point which will be used as a topleft.
 * @param	screen_loc      The screen location of the anchored object.
 * @param	object_size     The size of the anchored object.
 */
void anchoring(ei_anchor_t anchor, ei_point_t* where, ei_rect_t* screen_loc, ei_size_t* object_size);

#endif //EI_DRAWING_TOOLS_H