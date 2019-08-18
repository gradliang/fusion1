/*
 */
#ifndef _PARAM_H
#define _PARAM_H

#ifndef MP600
#error "MP600 is not defined"
#define HZ 250
#else
//#define HZ 50
#define HZ 250                                  /* on MP652 */
#endif

#endif /* _PARAM_H */
