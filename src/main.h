#pragma once
#include "stdafx.h"

#define MIN 0
#define MAX 1

#define SIGBREAK 21 // windows only


void		tidy_up(void);
unsigned	get_random(unsigned low, unsigned high);
void		sig_handler(int sig);
