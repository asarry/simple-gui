#include "ei_picking.h"

static ei_surface_t *picking_surface = NULL;

/**
 * Returns the picking surface currently being used.
 *
 * @return			The picking surface currenlty being used, or NULL.
 */
ei_surface_t* ei_picking_get_picking_surface (void)
{
        return picking_surface;
}

/**
 * Sets a picking surface.
 *
 * @param	widget		The widget to declare as the picking surface.
 */
void ei_picking_set_picking_surface (ei_surface_t *surface)
{
        picking_surface = surface;
}