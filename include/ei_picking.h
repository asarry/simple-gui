#ifndef EI_PICKING_H
#define EI_PICKING_H

#include "hw_interface.h"

/**
 * Returns the picking surface currently being used.
 *
 * @return			The picking surface currenlty being used, or NULL.
 */
ei_surface_t* ei_picking_get_picking_surface (void);

/**
 * Sets a picking surface.
 *
 * @param	widget		The widget to declare as the picking surface.
 */
void ei_picking_set_picking_surface (ei_surface_t *surface);

#endif //EI_PICKING_H
