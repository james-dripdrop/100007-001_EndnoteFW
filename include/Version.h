/*
 * Version.h
 *
 * Created: 16/07/2021 16:25:33
 * contains version information for the software.
 *  Author: jguyt
 */ 


#ifndef VERSION_FILE_H
#define VERSION_FILE_H
#ifdef RELEASE
#define SW_VERSION 40
/* Versions before 40 will not fail without a bootloader present - 40+ requires a bootloader present */
#else 
#warning This Version is for internal use only
#define SW_VERSION 0
#endif
#endif /* VERSION_FILE_H */