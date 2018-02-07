#!/bin/bash
PARENT=/Users/Ira/source/embedded/ts_sdk_c/scripts/openocd-configs
openocd -f ${PARENT}/stm32f4/board/st_nucleo_f4.cfg \
	-c init \
	-c "reset halt" \
	-c "flash write_image erase ${1}" \
	-c reset \
	-c shutdown

