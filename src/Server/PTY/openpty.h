#pragma once
#include <termios.h>
#include <sys/ioctl.h>

int openpty(int *amaster, int *aslave, char *name,
            const struct termios *termp, const struct winsize *winp);
