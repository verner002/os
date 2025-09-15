/**
 * Driver
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "hal/driver.h"

/**
 * __link_symbol
*/

/*uint32_t __link_symbol(DRIVER *driver, char const *name) {
    uint32_t symbols_count = driver->symbols_count;
    SYMBOL *symbols = driver->symbols;
    
    for (uint32_t i = 0; i < symbols_count; ++i) {
        SYMBOL *symbol = &symbols[i];

        if (!strcmp((char const *)symbol->name, name))
            return symbol->address;
    }

    return 0;
}*/