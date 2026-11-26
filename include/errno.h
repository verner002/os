/**
 * @file errno.h
 * @author verner002
 * @date 10/09/2026
*/

#pragma once

#define EPERM           1       // operation not permitted
#define ENOENT          2       // no such file or directory
#define ENXIO           6       // no such device or address
#define EBADF           9       // bad file descriptor
#define EAGAIN          11      // resource temporarily unavailable
#define ENOMEM          12      // cannot allocate memory
#define EACCES          13      // permission denied
#define EBUSY           16      // device or resource busy
#define EEXIST          17      // file exists
#define ENODEV          19      // no such device
#define ENOTDIR         20      // not a directory
#define EISDIR          21      // is a directory
#define EINVAL          22      // invalid argument
#define EMFILE          24      // too many open files
#define ENOSYS          38      // function not implemented
#define EWOULDBLOCK     EAGAIN  // operation would block
#define EOVERFLOW       75      // value too large for defined data type

extern int errno;