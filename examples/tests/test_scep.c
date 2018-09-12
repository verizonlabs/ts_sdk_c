// Copyright (c) 2018 Verizon Inc. All rights reserved
#include "ts_status.h"
#include "unistd.h"
#include "string.h"
#include "ts_file.h"
#include "ts_scep.h"
#include "ts_controller.h"

int main(void)
{
	ts_scep_initialize();
#warning "testing scep >>>>>>>>>>>>>>"




	ts_scep_assert(0);
	/* Program should not reach beyond the assert(0). */





	return 0;
}
