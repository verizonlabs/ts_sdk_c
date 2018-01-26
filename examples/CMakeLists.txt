# Copyright (C) 2017, Verizon, Inc. All rights reserved.

# component and platform selection
add_definitions( -DTS_PLATFORM_UNIX )       # UNIX, THREADX, NONE or CUSTOM
add_definitions( -DTS_BOARD=unix )          # unix, unix/raspberry_pi, threadX/renesas_s5_d9, none/freedom, none/nucleo

# example platforms
add_subdirectory( platforms )

# example tests
add_subdirectory( tests )

# example applications
add_subdirectory( applications )
