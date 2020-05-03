#ifndef __STDARG_H
#define __STDARG_H

//#ifdef __YAPP__
//#ifdef __i386__
typedef char *va_list;
#define va_start(list, last)                                                   \
    list = ((char *)&(last)) + ((sizeof(last) + 3) & ~3)
#define va_arg(list, type)                                                     \
    (list += (sizeof(type) + 3) & ~3, *(type *)(list - ((sizeof(type) + 3) & ~3)))
#define va_copy(dest, src) (dest) = (src)
#define va_end(list)       ((void)0)
//#endif
//#endif

typedef va_list __gnuc_va_list;
#define _VA_LIST_DEFINED

#endif /* __STDARG_H */
