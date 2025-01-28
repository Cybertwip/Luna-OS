
#ifndef __STDBOOL_H__
#define __STDBOOL_H__

#ifndef __cplusplus

#define bool _Bool
#define true 1
#define false 0

#else

/* In C++, `bool`, `true`, and `false` are already defined as keywords. */
#define _Bool bool

#endif

#define __bool_true_false_are_defined 1

#endif /* __STDBOOL_H__ */