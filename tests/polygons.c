//
// Created by humeur on 09/05/2021.
//


#include <stdlib.h>
#include <stdio.h>

#include "hw_interface.h"
#include "ei_utils.h"
#include "ei_draw.h"
#include "ei_types.h"
#include "ei_event.h"

struct side_table
{
        int32_t y_max;
        int32_t xk_min;
        int32_t args[3];
        struct side_table *next;
};

static void update_ast(struct side_table** ast, struct side_table** st, int32_t y) {
        struct side_table* head = *ast;
        struct side_table* prev = NULL;
        while (*ast) {
                if (!prev) {
                        if ((*ast)->y_max==y) head = (*ast)->next;
                        else prev = *ast;
                } else {
                        if ((*ast)->y_max == y) {
                                (prev)->next = (*ast)->next;
                        }
                }
                *ast = (*ast)->next;
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
                        if ((*ast)->xk_min < st[y]->xk_min) {
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

void test_square(ei_surface_t surface, ei_rect_t* clipper)
{
        ei_color_t		color		= { 255, 0, 255, 255 };
        ei_linked_point_t	pts[4];

        pts[0].point.x = 20; pts[0].point.y = 20; pts[0].next = &pts[1];
        pts[1].point.x = 40; pts[1].point.y = 20; pts[1].next = &pts[2];
        pts[2].point.x = 40; pts[2].point.y = 40; pts[2].next = &pts[3];
        pts[3].point.x = 20; pts[3].point.y = 40; pts[3].next = NULL;

        ei_draw_polygon(surface, pts, color, clipper);
}




int main(void)
{
//        struct side_table * st[10];
//        struct side_table AB = {10, 5, {0,1,1}, NULL};
//        struct side_table BC = {7, 4, {0,1,1}, &AB};
//        struct side_table CD = {8,10,{0,1,1}, NULL};
//        struct side_table DE = {8, 6, {0,1,2}, &CD};
//        struct side_table EF = {5, 3, {0,1,1}, &DE};
//        st[5] = &BC;
//        struct side_table * ast = NULL;
//        update_ast(&ast,st,5);
        ei_size_t		win_size	= ei_size(50, 50);
        ei_surface_t		main_window	= NULL;
        ei_color_t		white		= { 0xff, 0xff, 0xff, 0xff };
        ei_rect_t*		clipper_ptr	= NULL;
//	ei_rect_t		clipper		= ei_rect(ei_point(200, 150), ei_size(400, 300));
//	clipper_ptr		= &clipper;
        ei_event_t		event;

        hw_init();

        main_window = hw_create_window(win_size, EI_FALSE);

        /* Lock the drawing surface, paint it white. */
        hw_surface_lock	(main_window);
        ei_fill		(main_window, &white, clipper_ptr);

        /* Draw polylines. */
        test_square	(main_window, clipper_ptr);
        /* Unlock and update the surface. */
        hw_surface_unlock(main_window);
        hw_surface_update_rects(main_window, NULL);

        /* Wait for a character on command line. */
        event.type = ei_ev_none;
        while (event.type != ei_ev_keydown)
                hw_event_wait_next(&event);

        hw_quit();
        return 0;
}