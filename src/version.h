#ifndef _PERIDOT_VERSION_H
#define _PERIDOT_VERSION_H

// Version related macros.

// Each version components in seperate macros
#define PD_VERSION_MAJOR 1
#define PD_VERSION_MINOR 0
#define PD_VERSION_PATCH 0

// All version components in a single int
// This is useful for comparison in external code, e.g
// #if PD_VERSION > 103
// To see if version is higher than 1.0.3
#define PD_VERSION 100

// All version components in a single string.
// Useful for printing.
#define PD_VERSION_STR "1.0.0"

// URL to the github repository
#define PD_GITHUB "https://github.com/peridot/peridot"

#endif // _PERIDOT_VERSION_H
