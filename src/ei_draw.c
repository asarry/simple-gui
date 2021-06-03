#include "ei_draw.h"
#include "ei_drawing_tools.h"
#include "ei_utils.h"

/**
 * \brief	Draws a line that can be made of many line segments.
 *
 * @param	surface 	Where to draw the line. The surface must be *locked* by
 *				\ref hw_surface_lock.
 * @param	first_point 	The head of a linked list of the points of the polyline. It can be NULL
 *				(i.e. draws nothing), can have a single point, or more.
 *				If the last point is the same as the first point, then this pixel is
 *				drawn only once.
 * @param	color		The color used to draw the line. The alpha channel is managed.
 * @param	clipper		If not NULL, the drawing is restricted within this rectangle.
 */
void ei_draw_polyline(ei_surface_t surface, const ei_linked_point_t* first_point, ei_color_t color,
                      const ei_rect_t* clipper){
        const ei_linked_point_t *head = first_point;
        ei_size_t surf_size = hw_surface_get_size(surface);
        uint32_t *rst_pixel_ptr = (uint32_t *) hw_surface_get_buffer(surface);
        if (first_point && first_point->next) {
                do {
                        uint32_t *pixel_ptr = rst_pixel_ptr;
                        const ei_linked_point_t *second_point = first_point->next;
                        int32_t dx = second_point->point.x - first_point->point.x;
                        int32_t dy = second_point->point.y - first_point->point.y;
                        int32_t abs_dx = (dx >= 0) ? dx : -dx;
                        int32_t abs_dy = (dy >= 0) ? dy : -dy;
                        int sg_x = (dx > 0) ? 1 : -1;
                        int sg_y = (dy > 0) ? 1 : -1;
                        int32_t E = 0;
                        pixel_ptr += first_point->point.y * surf_size.width + first_point->point.x;
                        if (abs_dx > abs_dy) {
                                for (uint32_t i = 0; i <= (abs_dy * surf_size.width + abs_dx); i++,
                                        pixel_ptr += sg_x) {
                                        E += abs_dy;
                                        if (2 * E > abs_dx) {
                                                E -= abs_dx;
                                                pixel_ptr += sg_y * surf_size.width;
                                                i += surf_size.width;
                                        }
                                        if (!clipper ||
                                            (clipper->top_left.y <(pixel_ptr-rst_pixel_ptr)/surf_size.width &&
                                             (pixel_ptr-rst_pixel_ptr)/surf_size.width< clipper->top_left.y+clipper->size.height &&
                                             clipper->top_left.x <(pixel_ptr-rst_pixel_ptr)%surf_size.width&&
                                             (pixel_ptr-rst_pixel_ptr)%surf_size.width< clipper->top_left.x+clipper->size.width)){
                                                *pixel_ptr = ei_map_rgba(surface, color);
                                        }
                                }
                        } else {
                                for (uint32_t i = 0; i <= (abs_dy * surf_size.width + abs_dx); i +=
                                        surf_size.width, pixel_ptr+=sg_y*surf_size.width) {
                                        E += abs_dx;
                                        if (2 * E > abs_dy) {
                                                E -= abs_dy;
                                                pixel_ptr += sg_x;
                                                i++;
                                        }
                                        if (!clipper ||
                                            (clipper->top_left.y <(pixel_ptr-rst_pixel_ptr)/surf_size.width &&
                                             (pixel_ptr-rst_pixel_ptr)/surf_size.width< clipper->top_left.y+clipper->size.height &&
                                             clipper->top_left.x <(pixel_ptr-rst_pixel_ptr)%surf_size.width&&
                                             (pixel_ptr-rst_pixel_ptr)%surf_size.width< clipper->top_left.x+clipper->size.width)){
                                                *pixel_ptr = ei_map_rgba(surface, color);
                                        }
                                }
                        }
                        first_point = first_point->next;
                } while(first_point->next && first_point!=head);
        } else if (first_point) {
                *rst_pixel_ptr = ei_map_rgba(surface, color);
        }

}

struct side_table
{
        int32_t y_max; // max y at which the side is supposed to be drawn
        int32_t xk_min; // min x intersecting the scanline and the side
        int32_t args[3]; // {Error, dx, dy}
        struct side_table *next; // linked list of sides
};

/**
 * \brief	Updates the active side table by removing all the sides that have a y_max equal or
 *              superior to the current scanline and adding all the new sides in the side table, while
 *              keeping the sides x_mink sorted.
 *
 * @param	ast      	The current active side table which contains all the sides that are
 *                              currently being handled by the ei_polygon_draw.
 * @param	st      	The side table containing all the sides that need to drawn, linked at
 *                              index corresponding to the first scanline they need to be considered
 *                              active.
 * @param	y		The y coordinate corresponding to the current scanline.
 */
static void update_ast(struct side_table** ast, struct side_table** st, int32_t y) {
        struct side_table *head = *ast;
        struct side_table *prev = NULL;
        struct side_table *tmp;

        while (*ast) {
                tmp = (*ast)->next;
                if (!prev) {
                        if ((*ast)->y_max<=y) {
                                head = tmp;
                                free(*ast);
                        } else {
                                prev = *ast;
                        }
                } else {
                        if ((*ast)->y_max <= y) {
                                (prev)->next = tmp;
                                free(*ast);
                        } else {
                                prev = *ast;
                        }
                }
                *ast = tmp;
        }

        *ast = head;
        if (!*ast) {
                *ast = st[y];
                st[y] = NULL;
        } else {
                prev = NULL;
                struct side_table *top;
                while (st[y]) {
                        top = st[y]->next;
                        if (!*ast) {
                                prev->next = st[y];
                                top = NULL;
                        } else if ((*ast)->xk_min < st[y]->xk_min) {
                                prev = *ast;
                                *ast = (*ast)->next;
                                top = st[y];
                        } else {
                                if (prev) (prev)->next = st[y];
                                else head = st[y];
                                prev = st[y];
                                st[y]->next = *ast;
                        }
                        st[y] = top;
                }
                *ast = head;
        }
}

/**
 * \brief	Draws a filled polygon.
 *
 * @param	surface 	Where to draw the polygon. The surface must be *locked* by
 *				\ref hw_surface_lock.
 * @param	first_point 	The head of a linked list of the points of the line. It is either
 *				NULL (i.e. draws nothing), or has more than 2 points. The last point
 *				is implicitly connected to the first point, i.e. polygons are
 *				closed, it is not necessary to repeat the first point.
 * @param	color		The color used to draw the polygon. The alpha channel is managed.
 * @param	clipper		If not NULL, the drawing is restricted within this rectangle.
 */
void ei_draw_polygon(ei_surface_t surface, const ei_linked_point_t* first_point, ei_color_t color,
                     const ei_rect_t* clipper)
{
        if (first_point && first_point->next && first_point->next->next) {
                const struct ei_linked_point_t *loop_first_p = first_point;
                ei_size_t surf_size = hw_surface_get_size(surface);

                // INIT side_table
                struct side_table **st = calloc(sizeof(struct side_table *), surf_size.height);
                int32_t glob_y_min = surf_size.height; int32_t glob_x_min = surf_size.width;
                int32_t glob_y_max = 0; int32_t glob_x_max = 0;

                while (first_point) {
                        const ei_linked_point_t *second_point = first_point->next;
                        if (!second_point) second_point = loop_first_p;

                        // Horizontal sides are ignored
                        if (second_point->point.y - first_point->point.y == 0) {
                                first_point = first_point->next;
                                continue;
                        }

                        struct side_table *curr_st = calloc(1, sizeof(struct side_table));
                        int32_t y_min, y_max, x_min, x_max;

                        if (second_point->point.y > first_point->point.y) {
                               y_min = first_point->point.y;
                               y_max = second_point->point.y;
                               curr_st->xk_min = first_point->point.x;
                        } else {
                                y_min = second_point->point.y;
                                y_max = first_point->point.y;
                                curr_st->xk_min = second_point->point.x;
                        }

                        if (second_point->point.x > first_point->point.x) {
                                x_min = first_point->point.x;
                                x_max = second_point->point.x;
                        } else {
                                x_min = second_point->point.x;
                                x_max = first_point->point.x;
                        }

                        if (y_max < 1 || y_min >= surf_size.height) {
                                first_point = first_point->next;
                                free(curr_st);
                                continue;
                        }
                        // get the global max and min to compute the size
                        glob_y_min = (y_min < glob_y_min) ? y_min : glob_y_min;
                        glob_y_max = (y_max > glob_y_max) ? y_max : glob_y_max;
                        glob_x_min = (x_min < glob_x_min) ? x_min : glob_x_min;
                        glob_x_max = (x_max > glob_x_max) ? x_max : glob_x_max;

                        curr_st->y_max = y_max;
                        curr_st->args[0] = 0;
                        curr_st->args[1] = second_point->point.x - first_point->point.x;
                        curr_st->args[2] = second_point->point.y - first_point->point.y;

                        if (y_min < 0) {
                                curr_st->xk_min -= curr_st->args[1] * y_min / curr_st->args[2];
                                y_min = 0;
                        }

                        curr_st->next = NULL;

                        if (!st[y_min]) {
                                st[y_min] = curr_st;
                        } else {
                                // Insert at the end while keeping the sides xk_min ordered
                                struct side_table *top = st[y_min];
                                int END = 0;

                                if (top->xk_min > curr_st->xk_min) {
                                        curr_st->next = st[y_min];
                                        st[y_min] = curr_st;
                                        END = 1;
                                } else if (top->xk_min == curr_st->xk_min) {
                                        if ((float) curr_st->args[1] / (float) curr_st->args[2] <
                                            (float) top->args[1] / (float) top->args[2]) {
                                                curr_st->next = st[y_min];
                                                st[y_min] = curr_st;
                                                END = 1;
                                        }
                                }

                                while (!END) {
                                        if (!(top->next)) {
                                                top->next = curr_st;
                                                END = 1;
                                        } else if(top->next->xk_min > curr_st->xk_min) {
                                                curr_st->next = top->next;
                                                top->next = curr_st;
                                                END = 1;
                                        } else if (top->xk_min == curr_st->xk_min) {
                                                if ((float) curr_st->args[1] / (float) curr_st->args[2] <
                                                    (float) top->next->args[1] / (float) top->next->args[2]) {
                                                        curr_st->next = top->next;
                                                        top->next = curr_st;
                                                        END = 1;
                                                } else {
                                                        top = top->next;
                                                }
                                        } else {
                                                top = top->next;
                                        }
                                }
                        }
                        first_point = first_point->next;
                }

                // create an offscreen surface of the exact size of the later drawn polygon
                glob_y_max = (glob_y_max < surf_size.height) ? glob_y_max : surf_size.height;
                glob_y_min = (glob_y_min < 0) ? 0 : glob_y_min;
                ei_size_t offscreen_size;

                if (glob_y_max > 0) offscreen_size = ei_size(glob_x_max - glob_x_min, glob_y_max - glob_y_min);
                else offscreen_size = ei_size(0,0);

                ei_surface_t offscreen = hw_surface_create(surface, offscreen_size, 1);
                hw_surface_lock(offscreen);
                uint32_t *rst_pixel_ptr = (uint32_t *) hw_surface_get_buffer(offscreen);
                // as we use the transparency channel, verify that the color to draw has a max alpha
                // channel
                ei_color_t offscreen_color = color;
                offscreen_color.alpha = 0xff;

                // MAIN LOOP
                struct side_table *ast = NULL;
                int y = glob_y_min;

                while (y < glob_y_max) {
                        update_ast(&ast, st, y);
                        struct side_table *curr_ast = ast;
                        while (curr_ast) {
                                int32_t x = curr_ast->xk_min-glob_x_min;
                                uint32_t *pixel_ptr = rst_pixel_ptr + (y-glob_y_min)*offscreen_size
                                        .width + x;
                                for (; x <
                                       curr_ast->next->xk_min-glob_x_min; pixel_ptr++, x++) {
                                        *pixel_ptr = ei_map_rgba(offscreen, offscreen_color);
                                }
                                curr_ast = curr_ast->next->next;
                        }
                        y++;
                        //update the xk_min in ast SO also the order in ast which is subject to change
                        struct side_table *head = NULL;
                        struct side_table *prev = NULL;
                        while (ast) {
                                int32_t sg_x = (ast->args[1]>0)?1 : -1;
                                int32_t sg_y = (ast->args[2]>0)?1 : -1;
                                int32_t abs_dx = (ast->args[1] >= 0) ? ast->args[1] : -ast->args[1];
                                int32_t abs_dy = (ast->args[2] >= 0) ? ast->args[2] : -ast->args[2];
                                if (abs_dy>abs_dx) {
                                        ast->args[0] += abs_dx;
                                        if (2 * ast->args[0] > abs_dy) {
                                                ast->args[0] -= abs_dy;
                                                ast->xk_min += sg_y*sg_x;
                                        }
                                } else {
                                        do {
                                                ast->xk_min += sg_y*sg_x;
                                                ast->args[0] += abs_dy;
                                        } while (2*ast->args[0]<= abs_dx);
                                        ast->args[0] -= abs_dx;
                                }
                                struct side_table *working_head = head;
                                prev = NULL;
                                int END = 1;
                                while (END) {
                                        if (working_head && working_head->xk_min <= ast->xk_min) {
                                                prev = working_head;
                                                working_head = working_head->next;
                                        } else {
                                                if (prev) prev->next = ast;
                                                else head = ast;
                                                prev = ast;
                                                ast = ast->next;
                                                prev->next = working_head;
                                                END = 0;
                                        }
                                }

                        }
                        ast = head;
                }

                struct side_table *tmp;
                while (ast != NULL) {
                        tmp = ast->next;
                        free(ast);
                        ast = tmp;
                }
                free(st);

                // copy the surface drawn on the offscreen to the main surface thanks to transparency
                ei_rect_t offscreen_rect = {{glob_x_min,glob_y_min}, offscreen_size};
                ei_rect_t intersection;
                if (clipper) {
                        intersection = rectangle_intersect(clipper, &offscreen_rect);

                        ei_point_t new_origin_start;
                        new_origin_start.x = (glob_x_min > clipper->top_left.x) ? 0 :
                                             offscreen_size.width - intersection.size.width;
                        new_origin_start.y = (glob_y_min > clipper->top_left.y) ? 0 :
                                             offscreen_size.height - intersection.size.height;
                        ei_rect_t zero_intersect = {new_origin_start, intersection.size};
                        ei_copy_surface(surface, &intersection, offscreen, &zero_intersect, 1);
                } else {
                        intersection = offscreen_rect;
                        ei_rect_t zero_intersect = {ei_point_zero(), intersection.size};
                        ei_copy_surface(surface, &intersection, offscreen, &zero_intersect, 1);
                }
                hw_surface_unlock(offscreen);
                hw_surface_free(offscreen);
        }
}

/**
 * \brief	Converts the red, green, blue and alpha components of a color into a 32 bits integer
 * 		than can be written directly in the memory returned by \ref hw_surface_get_buffer.
 * 		The surface parameter provides the channel order.
 *
 * @param	surface		The surface where to store this pixel, provides the channels order.
 * @param	color		The color to convert.
 *
 * @return 			The 32 bit integer corresponding to the color. The alpha component
 *				of the color is ignored in the case of surfaces that don't have an
 *				alpha channel.
 */
uint32_t ei_map_rgba (ei_surface_t surface, ei_color_t color)
{
        int ir, ig, ib, ia;
        hw_surface_get_channel_indices(surface, &ir, &ig, &ib, &ia);
        if (ia != -1) return (color.alpha << (ia * 8)) + (color.red << (ir * 8)) + (color.green << (ig * 8)) + (color.blue << (ib * 8));
        else return (color.red << (ir * 8)) + (color.green << (ig * 8)) + (color.blue << (ib * 8));
}

/**
 * \brief	Fills the surface with the specified color.
 *
 * @param	surface		The surface to be filled. The surface must be *locked* by
 *				\ref hw_surface_lock.
 * @param	color		The color used to fill the surface. If NULL, it means that the
 *				caller want it painted black (opaque).
 * @param	clipper		If not NULL, the drawing is restricted within this rectangle.
 */
void ei_fill (ei_surface_t surface, const ei_color_t* color, const ei_rect_t* clipper)
{
        ei_size_t surf_size = hw_surface_get_size(surface);
        if (clipper) {
                uint32_t *rst_pixel_ptr = (uint32_t*) hw_surface_get_buffer(surface);
                for (int32_t j=0; j<clipper->size.height;j++) {
                        uint32_t *pixel_ptr = rst_pixel_ptr + clipper->top_left.x +
                                              (j + clipper->top_left.y) * surf_size.width;
                        for (int32_t i = 0; i < clipper->size.width; i++, pixel_ptr++)
                                *pixel_ptr = ei_map_rgba(surface, *color);
                }
        } else {
                uint32_t *pixel_ptr = (uint32_t*) hw_surface_get_buffer(surface);
                for (int32_t i = 0; i < (surf_size.width * surf_size.height); i++) {
                        *pixel_ptr = ei_map_rgba(surface, *color);
                        pixel_ptr++;
                }
        }
}

/**
 * \brief	Copies pixels from a source surface to a destination surface.
 *		The source and destination areas of the copy (either the entire surfaces, or
 *		subparts) must have the same size before considering clipping.
 *		Both surfaces must be *locked* by \ref hw_surface_lock.
 *
 * @param	destination	The surface on which to copy pixels.
 * @param	dst_rect	If NULL, the entire destination surface is used. If not NULL,
 *				defines the rectangle on the destination surface where to copy
 *				the pixels.
 * @param	source		The surface from which to copy pixels.
 * @param	src_rect	If NULL, the entire source surface is used. If not NULL, defines the
 *				rectangle on the source surface from which to copy the pixels.
 * @param	alpha		If true, the final pixels are a combination of source and
 *				destination pixels weighted by the source alpha channel and
 *				the transparency of the final pixels is set to opaque.
 *				If false, the final pixels are an exact copy of the source pixels,
 				including the alpha channel.
 *
 * @return			Returns 0 on success, 1 on failure (different sizes between source and destination).
 */
int ei_copy_surface (ei_surface_t destination, const ei_rect_t* dst_rect, ei_surface_t source,
                     const ei_rect_t* src_rect, ei_bool_t alpha)
{
        ei_size_t true_dest_size = hw_surface_get_size(destination);
        ei_size_t dest_size = true_dest_size;
        uint32_t *dest_pixel_ptr = (uint32_t*) hw_surface_get_buffer(destination);
        ei_size_t true_src_size = hw_surface_get_size(source);
        ei_size_t src_size = true_src_size;
        uint32_t *src_pixel_ptr = (uint32_t*) hw_surface_get_buffer(source);

        if (dst_rect) {
                dest_pixel_ptr += dst_rect->top_left.y * dest_size.width + dst_rect->top_left.x;
                dest_size = dst_rect->size;
        }
        if (src_rect) {
                src_pixel_ptr += src_rect->top_left.y * src_size.width + src_rect->top_left.x;
                src_size = src_rect->size;
        }
        if (dest_size.width != src_size.width || dest_size.height != src_size.height) return 1;

        for (int32_t j = 0; j < dest_size.height; j++) {
                uint32_t *pst_pixel_ptr = dest_pixel_ptr + j * true_dest_size.width;
                uint32_t *cpy_pixel_ptr = src_pixel_ptr + j * true_src_size.width;
                for (int32_t i = 0; i < src_size.width; i++, cpy_pixel_ptr++, pst_pixel_ptr++) {
                        if (!alpha) {
                                *pst_pixel_ptr = *cpy_pixel_ptr;
                        } else {
                                //get colour
                                int ir, ig, ib, ia;
                                hw_surface_get_channel_indices(source, &ir, &ig, &ib, &ia);
                                if (ia == -1) return 1;
                                int bitmask = 0xFF;
                                uint32_t red_src = (*cpy_pixel_ptr) >> (ir * 8) & bitmask;
                                uint32_t green_src = (*cpy_pixel_ptr) >> (ig * 8) & bitmask;
                                uint32_t blue_src = (*cpy_pixel_ptr) >> (ib * 8) & bitmask;
                                uint8_t alpha_src = (*cpy_pixel_ptr) >> (ia * 8) & bitmask;
                                uint32_t red_dest = (*pst_pixel_ptr)>>(ir * 8) & bitmask;
                                uint32_t green_dest = (*pst_pixel_ptr) >> (ig * 8) & bitmask;
                                uint32_t blue_dest = (*pst_pixel_ptr) >> (ib * 8) & bitmask;
                                uint64_t red_res = (red_src * alpha_src + (255 - alpha_src) * red_dest) / 255;
                                uint64_t blue_res = (blue_src * alpha_src + (255 - alpha_src) * blue_dest) / 255;
                                uint64_t green_res = (green_src * alpha_src + (255 - alpha_src) * green_dest) / 255;
                                *pst_pixel_ptr = (red_res << (ir * 8)) + (green_res << (ig * 8)) + (blue_res << (ib * 8));
                        }
                }
        }
        return 0;
}

/**
 * \brief	Draws text by calling \ref hw_text_create_surface.
 *
 * @param	surface 	Where to draw the text. The surface must be *locked* by
 *				\ref hw_surface_lock.
 * @param	where		Coordinates, in the surface, where to anchor the top-left corner of
 *				the rendered text.
 * @param	text		The string of the text. Can't be NULL.
 * @param	font		The font used to render the text. If NULL, the \ref ei_default_font
 *				is used.
 * @param	color		The text color. Can't be NULL. The alpha parameter is not used.
 * @param	clipper		If not NULL, the drawing is restricted within this rectangle.
 */
void ei_draw_text (ei_surface_t	surface, const ei_point_t* where, const char* text, ei_font_t font,
                   ei_color_t color, const ei_rect_t* clipper)
{
        if (text == NULL) return;
        if (font == NULL) font = ei_default_font;
        ei_surface_t text_surface = hw_text_create_surface(text, font, color);
        ei_size_t text_size = hw_surface_get_size(text_surface);
        ei_rect_t positioned_rect = {*where, text_size};
        // Find the intersection of two rectangles
        if (clipper) {
                ei_rect_t intersection = rectangle_intersect(clipper, &positioned_rect);
                ei_point_t new_origin_start;
                new_origin_start.x = (where->x >= clipper->top_left.x) ?
                                      0 : intersection.top_left.x - where->x;
                new_origin_start.y = (where->y >= clipper->top_left.y) ?
                                      0 : intersection.top_left.y - where->y;
                ei_rect_t zero_intersect = {new_origin_start, intersection.size};
                ei_copy_surface(surface, &intersection, text_surface, &zero_intersect, 1);
        } else {
                // Copy all of it, positioned correctly
                ei_rect_t text_rect = {ei_point_zero(), text_size};
                ei_copy_surface(surface,&positioned_rect, text_surface, &text_rect, 1);
        }
        hw_surface_free(text_surface);
}