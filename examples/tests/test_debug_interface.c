// Copyright (c) 2018 Verizon Inc. All rights reserved
#include "ts_status.h"
#include "ts_platform.h"

int main(void)
{
	ts_platform_initialize();
	ts_status_set_level(TsStatusLevelDebug);
	ts_status_debug("This is a test message\n");
	ts_platform_assert(0);
	/* Program should not reach beyond the assert(0). */
	return 0;
}
