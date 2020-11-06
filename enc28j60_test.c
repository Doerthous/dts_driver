#include <stdlib.h>
#include <string.h>

#include "enc28j60.h"

int enc28j60_test(enc28j60_t *enc28j60)
{
    if (enc28j60_init(enc28j60)) {
        return 1;
    }
    
    return 0;
}
