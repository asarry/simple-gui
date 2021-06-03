#include "ei_drawing_tools.h"

/**
 * Returns a lighter version of the color passed as argument.
 *
 * @param	color		The original color.
 *
 * @return			The lighter version of the color.
 */
ei_color_t get_light_color_variation(const ei_color_t* color)
{
        unsigned char light_red = min(color->red + (unsigned char)10, 0xff);
        unsigned char light_green = min(color->green + (unsigned char)10, 0xff);
        unsigned char light_blue = min(color->blue + (unsigned char)10, 0xff);
        return (ei_color_t) {light_red, light_green, light_blue, 0xff};
}

/**
 * Returns a darker version of the color passed as argument.
 *
 * @param	color		The original color.
 *
 * @return			The darker version of the color.
 */
ei_color_t get_dark_color_variation(const ei_color_t* color)
{
        unsigned char dark_red = max(color->red - (unsigned char) 10, 0x00);
        unsigned char dark_green = max(color->green - (unsigned char) 10, 0x00);
        unsigned char dark_blue = max(color->blue - (unsigned char) 10, 0x00);
        return (ei_color_t) {dark_red, dark_green, dark_blue, 0xff};
}

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
ei_linked_point_t *arc(ei_point_t center, int radius, float init_angle, float fin_angle)
{
        ei_linked_point_t *new_arc = malloc(sizeof(struct ei_linked_point_t));
        ei_point_t first_point = ei_point((int) floorf((float) center.x + (float) radius * cosf(init_angle)),
                                          (int) floorf((float) center.y - (float) radius * sinf(init_angle)));
        new_arc->point = first_point;

        float step = (float) M_PI / (float) (8 * radius);
        float angle = init_angle + step;

        struct ei_linked_point_t *sent = new_arc;
        while (angle < fin_angle) {
                ei_point_t new_point = ei_point((int) floorf((float) center.x + (float) radius * cosf(angle)),
                                                (int) floorf((float) center.y - (float) radius * sinf(angle)));
                struct ei_linked_point_t *next = malloc(sizeof(struct ei_linked_point_t));
                next->point = new_point;
                sent->next = next;
                sent = sent->next;
                angle += step;
        }

        ei_point_t final_point = ei_point((int)floorf((float) center.x + (float) radius * cosf(fin_angle)),
                                          (int)floorf((float) center.y - (float) radius * sinf(fin_angle)));
        struct ei_linked_point_t *end = malloc(sizeof(struct ei_linked_point_t));
        end->point = final_point;
        end->next = NULL;
        sent->next = end;

        return new_arc;
}

/**
 * \brief	Returns a list of points describing a rounded_frame, or a part (upper or lower) of it.
 *
 * @param	rectangle  	The rectangle used to know dimensions of the rounded frame.
 * @param	radius          Radius of the rounded corners.
 * @param	part 	        The part to describe.
 *
 * @return			The list of points describing the rounded frame.
 */
ei_linked_point_t *rounded_frame(ei_rect_t rectangle, int radius, char part)
{
        if (part == 't') {
                ei_point_t center_arc = ei_point(rectangle.top_left.x + radius, rectangle.top_left.y + radius);
                struct ei_linked_point_t *points = arc(center_arc, radius, (float) M_PI_2, (float) M_PI);
                struct ei_linked_point_t *curr = points;
                while (curr->next != NULL) curr = curr->next;

                center_arc.y += rectangle.size.height - 2 * radius;
                curr->next = arc(center_arc, radius, (float) M_PI, 3 * (float) M_PI_2);
                while (curr->next != NULL) curr = curr->next;

                center_arc.x += rectangle.size.width - 2 * radius;
                curr->next = arc(center_arc, radius, 3 * (float) M_PI_2, 2 * (float) M_PI);
                while (curr->next != NULL) curr = curr->next;

                center_arc.y -= rectangle.size.height - 2 * radius;
                curr->next = arc(center_arc, radius, 0, (float) M_PI_2);

                return points;
        } else if (part == 'h') {
                ei_point_t center_arc = ei_point(rectangle.top_left.x + radius, rectangle.top_left.y + radius);
                struct ei_linked_point_t *points = arc(center_arc, radius, (float) M_PI_2, (float) M_PI);
                struct ei_linked_point_t *curr = points;
                while (curr->next != NULL) curr = curr->next;

                center_arc.y += rectangle.size.height - 2 * radius;
                curr->next = arc(center_arc, radius, (float) M_PI, 5 * (float) M_PI_4);
                while (curr->next != NULL) curr = curr->next;

                struct ei_linked_point_t *middle_pt_r = malloc(sizeof(struct ei_linked_point_t));
                middle_pt_r->point = ei_point(rectangle.top_left.x + rectangle.size.width - (int)(rectangle.size.height/2), rectangle.top_left.y + (int)(rectangle.size.height/2));
                struct ei_linked_point_t *middle_pt_l = malloc(sizeof(struct ei_linked_point_t));
                middle_pt_l->point = ei_point(rectangle.top_left.x + (int)(rectangle.size.height/2), rectangle.top_left.y + (int)(rectangle.size.height/2));
                middle_pt_l->next = middle_pt_r;
                curr->next = middle_pt_l;
                curr = curr->next->next;

                center_arc.x += rectangle.size.width - 2 * radius;
                center_arc.y -= rectangle.size.height - 2 * radius;
                curr->next = arc(center_arc, radius, (float) M_PI_4, (float) M_PI_2);

                return points;
        } else if (part == 'l') {
                ei_point_t center_arc = ei_point(rectangle.top_left.x + radius, rectangle.top_left.y + rectangle.size.height - radius);
                struct ei_linked_point_t *points = arc(center_arc, radius, 5 * (float) M_PI_4, 3 * (float) M_PI_2);
                struct ei_linked_point_t *curr = points;
                while (curr->next != NULL) curr = curr->next;

                center_arc.x += rectangle.size.width - 2 * radius;
                curr->next = arc(center_arc, radius, 3 * (float)M_PI_2, 2 * (float) M_PI);
                while (curr->next != NULL) curr = curr->next;

                center_arc.y -= rectangle.size.height - 2 * radius;
                curr->next = arc(center_arc, radius, 0, (float) M_PI_4);
                while (curr->next != NULL) curr = curr->next;

                struct ei_linked_point_t *middle_pt_l = malloc(sizeof(struct ei_linked_point_t));
                middle_pt_l->point = ei_point(rectangle.top_left.x + (int)(rectangle.size.height/2), rectangle.top_left.y + (int)(rectangle.size.height/2));
                struct ei_linked_point_t *middle_pt_r = malloc(sizeof(struct ei_linked_point_t));
                middle_pt_r->point = ei_point(rectangle.top_left.x + rectangle.size.width - (int)(rectangle.size.height/2), rectangle.top_left.y + (int)(rectangle.size.height/2));
                middle_pt_r->next = middle_pt_l;
                middle_pt_l->next = NULL;
                curr->next = middle_pt_r;

                return points;
        }
}

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
ei_rect_t rectangle_intersect(ei_rect_t* first_rect, ei_rect_t* sec_rect)
{
        if (first_rect == NULL) return * sec_rect;
        ei_point_t f_rect_topleft = first_rect->top_left;
        ei_point_t s_rect_topleft = sec_rect->top_left;
        ei_size_t f_size = first_rect->size;
        ei_size_t s_size = sec_rect->size;

        // If any intersection at all
        if ((f_rect_topleft.y + f_size.height > s_rect_topleft.y &&
            s_rect_topleft.y + s_size.height > f_rect_topleft.y) ||
            (f_rect_topleft.x + f_size.width > s_rect_topleft.x &&
            s_rect_topleft.x + s_size.width > f_rect_topleft.x)) {
                // Find the intersection between the two rectangles
                ei_point_t intersect_top_left;
                ei_size_t intersect_size;
                if (s_rect_topleft.x > f_rect_topleft.x) {
                        intersect_top_left.x = s_rect_topleft.x;
                        intersect_size.width = f_rect_topleft.x + f_size.width -
                                               intersect_top_left.x;
                } else {
                        intersect_top_left.x = f_rect_topleft.x;
                        intersect_size.width = s_rect_topleft.x + s_size.width -
                                               intersect_top_left.x;
                }
                if (s_rect_topleft.y > f_rect_topleft.y) {
                        intersect_top_left.y = s_rect_topleft.y;
                        intersect_size.height = f_rect_topleft.y + f_size.height -
                                                intersect_top_left.y;
                } else {
                        intersect_top_left.y = f_rect_topleft.y;
                        intersect_size.height = s_rect_topleft.y + s_size.height -
                                                intersect_top_left.y;
                }
                // if the width or height of first_rect is fully contained within sec_rect
                intersect_size.width = (intersect_size.width < f_size.width) ?
                                       intersect_size.width : f_size.width;
                intersect_size.height = (intersect_size.height < f_size.height) ?
                                        intersect_size.height : f_size.height;
                // if the width or height of sec_rect is fully contained within first_rect
                intersect_size.width = (intersect_size.width < s_size.width) ?
                                       intersect_size.width : s_size.width;
                intersect_size.height = (intersect_size.height < s_size.height) ?
                                        intersect_size.height : s_size.height;

                ei_rect_t intersection = {intersect_top_left, intersect_size};
                return intersection;
        }
        return ei_rect_zero();
}

/**
 * \brief	Sets the coordinates of a topleft point regarding the anchor and the size of
 *              the object to anchor.
 *
 * @param	anchor  	The anchor.
 * @param	where           The point which will be used as a topleft.
 * @param	screen_loc      The screen location of the anchored object.
 * @param	object_size     The size of the anchored object.
 */
void anchoring(ei_anchor_t anchor, ei_point_t* where, ei_rect_t* screen_loc, ei_size_t* object_size)
{
        int obj_width = object_size->width;
        int obj_height = object_size->height;
        switch (anchor) {
                case ei_anc_northwest:
                        where->x = screen_loc->top_left.x;
                        where->y = screen_loc->top_left.y;
                        break;
                case ei_anc_north:
                        where->x = screen_loc->top_left.x + (int) (screen_loc->size.width / 2) - obj_width / 2;
                        where->y = screen_loc->top_left.y;
                        break;
                case ei_anc_northeast:
                        where->x = screen_loc->top_left.x + screen_loc->size.width - obj_width;
                        where->y = screen_loc->top_left.y;
                        break;
                case ei_anc_east:
                        where->x = screen_loc->top_left.x + screen_loc->size.width - obj_width;
                        where->y = screen_loc->top_left.y + (int) (screen_loc->size.height / 2) - obj_height / 2;
                        break;
                case ei_anc_southeast:
                        where->x = screen_loc->top_left.x + screen_loc->size.width - obj_width;
                        where->y = screen_loc->top_left.y + screen_loc->size.height - obj_height;
                        break;
                case ei_anc_south:
                        where->x = screen_loc->top_left.x + (int) (screen_loc->size.width / 2) - obj_width / 2;
                        where->y = screen_loc->top_left.y + screen_loc->size.height - obj_height;
                        break;
                case ei_anc_southwest:
                        where->x = screen_loc->top_left.x;
                        where->y = screen_loc->top_left.y + screen_loc->size.height - obj_height;
                        break;
                case ei_anc_west:
                        where->x = screen_loc->top_left.x;
                        where->y = screen_loc->top_left.y + (int)(screen_loc->size.height / 2) - obj_height / 2;
                        break;
                case ei_anc_center:
                        where->x = screen_loc->top_left.x + (screen_loc->size.width / 2) - obj_width / 2;
                        where->y = screen_loc->top_left.y + (screen_loc->size.height / 2) - obj_height / 2;
                        break;
                default:
                        break;
        }
}