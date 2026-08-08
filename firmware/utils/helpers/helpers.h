#ifndef HELPERS_H
#define HELPERS_H

/* Small vendor-agnostic helper macros. */

#define ARRAY_SIZE(a)   (sizeof(a) / sizeof((a)[0]))
#define UNUSED(x)       ((void)(x))
#ifndef MIN
#define MIN(a, b)       ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b)       ((a) > (b) ? (a) : (b))
#endif

#endif /* HELPERS_H */
