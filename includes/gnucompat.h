#ifndef _GNU_COMPAT_H
#define _GNU_COMPAT_H

#define __extension__ 
#define double float
#define __inline
#define __builtin_bswap32(x)
#define __builtin_bswap64(x)
#define __uint64_t unsigned int

#endif /* _GNU_COMPAT_H */