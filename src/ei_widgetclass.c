#include <string.h>
#include "ei_application.h"
#include "ei_widgetclass.h"

static ei_widgetclass_t *root_widgetclass = NULL;

/**
 * @brief	Registers a class to the program so that widgets of this class can be created.
 *		This must be done only once per widged class in the application.
 *
 * @param	widgetclass	The structure describing the class.
 */
void ei_widgetclass_register(ei_widgetclass_t* widgetclass)
{
        ei_widgetclass_t* last_widgetclass = root_widgetclass;

        if (last_widgetclass != NULL) {
                while (last_widgetclass->next != NULL) last_widgetclass = last_widgetclass->next;
                last_widgetclass->next = widgetclass;
        } else {
                root_widgetclass = widgetclass;
        }
}

/**
 * @brief	Returns the structure describing a class, from its name.
 *
 * @param	name		The name of the class of widget.
 *
 * @return			The structure describing the class.
 */
ei_widgetclass_t* ei_widgetclass_from_name(ei_widgetclass_name_t name)
{
        ei_widgetclass_t* searched_widgetclass = root_widgetclass;

        while (searched_widgetclass != NULL) {
                if (strcmp(searched_widgetclass->name, name) == 0) break;
                else searched_widgetclass = searched_widgetclass->next;
        }

        return searched_widgetclass;
}