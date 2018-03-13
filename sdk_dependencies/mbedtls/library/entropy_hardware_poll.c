/* Copyright(C) 2016 Verizon. All rights reserved. */

/*
 * Provide the function used by mbedtls to collect entropy from a hardware
 * source.  For now, we assume that source is a 32-bit hardware random number
 * generator.
 */

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#ifdef MBEDTLS_ENTROPY_HARDWARE_ALT

#include <stdbool.h>
#include <stdint.h>
#include "hwrng_hal.h"

static bool generator_ready;

/*
 * Get len bytes of entropy from the hardware RNG.
 */
int mbedtls_hardware_poll( void *data,
			   unsigned char *output, size_t len, size_t *olen )
{
    size_t i, j;
    uint32_t num;
    ((void) data);

    if ( !generator_ready ) {
        if ( !hwrng_module_init() )
            return -1;

        generator_ready = true;
    }

    for ( i = 0; i < len; ) {

        if ( ! hwrng_read_random( &num )) {
            generator_ready = false;
            return -1;
        }

        for ( j = 0; j < sizeof(num) && i < len; ) {
            *(output + i) = num & 0xff;
            num >>= 8;
            i++; j++;
        }
    }
    *olen = len;
    return 0;
}

#endif /* MBEDTLS_ENTROPY_HARDWARE_ALT */
