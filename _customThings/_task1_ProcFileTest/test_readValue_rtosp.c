// SPDX-License-Identifier: GPL-2.0+
//
// Copyright (C) 2022 RTOSP ISEP
// Author: João Passos <lrg@slimlogic.co.uk>
//         Marco Silva <richard@openedhand.com>
//		   
// Test procfile

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void main(int argc, char **argv)
{
	int pid;

	sscanf(argv[1], "%d", &pid);
	printf("pid = %d\n", pid);

	char filename[1000];

	sprintf(filename, "/proc/%d/rtosp", pid);
	FILE *f = fopen(filename, "r");

	char comm[4];
	
	fscanf(f, "%s", comm);
	printf("rtosp value = %s\n", comm);
	fclose(f);
}
