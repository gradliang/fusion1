
#include <linux/device.h>
#include <linux/usb.h>
#include <linux/workqueue.h>

#include "os_mp52x.h"
#include "mpTrace.h"

#define MAX_DRIVERS 2
struct usb_driver *global_drivers[MAX_DRIVERS];

int usb_register_driver(struct usb_driver *new_driver, struct module *owner,
			const char *mod_name)
{
	int retval = 0;
    short i;

    for (i=0; i<MAX_DRIVERS; i++)
    {
        if (!global_drivers[i])
            break;
    }

    MP_ASSERT(i < MAX_DRIVERS);

    if (i == MAX_DRIVERS)
        retval = -1;
    else
        global_drivers[i] = new_driver;

    return retval;
}

void usb_deregister(struct usb_driver *driver)
{
    short i;
    for (i=0; i<MAX_DRIVERS; i++)
    {
        if ((unsigned long)global_drivers[i] == (unsigned long)driver)
            break;
    }

    if (i < MAX_DRIVERS)
        global_drivers[i] = NULL;

}
EXPORT_SYMBOL_GPL_FUTURE(usb_deregister);

struct usb_driver * usb_get_driver(u16 vid, u16 pid)
{
	struct usb_device_id *id;
    bool found = false;
    short i;

//    __asm("break 100");
    for (i=0; i< MAX_DRIVERS; i++)
    {
        if (!global_drivers[i])
            break;

        id = global_drivers[i]->id_table;
        do
        {
            if (id->idVendor == vid &&
                    id->idProduct == pid)
            {
                found = true;
                break;
            }
            id++;
        } while (id->idVendor);

        if (found)
            break;
    }

    MP_ASSERT(found);

    if (found)
        return global_drivers[i];
    else
    {
        __asm("break 100");
        return NULL;
    }
}

