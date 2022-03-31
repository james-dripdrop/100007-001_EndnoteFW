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
#define SW_VERSION 30
#else 
#warning This Version is for internal use only
#define SW_VERSION 0
#endif
#endif /* VERSION_FILE_H */