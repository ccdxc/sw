#ifndef OSAL_SETUP_H
#define OSAL_SETUP_H

typedef int (*osal_init_fn_t)();
typedef int (*osal_body_fn_t)();
typedef void (*osal_fini_fn_t)();
#ifndef __KERNEL__

#define OSAL_LICENSE(...)
#define OSAL_AUTHOR(...)
#define OSAL_DESCRIPTION(...)
#define OSAL_EXPORT_SYMBOL(...)

#define OSAL_SETUP(init, body, fini)					\
void __attribute__ ((constructor)) osal_init() {			\
	if (init != NULL) {						\
		init();							\
	}								\
}									\
void __attribute__ ((destructor)) osal_fini() {				\
	if (fini != NULL) {						\
		fini();							\
	}								\
}									\
int main(int argc, char** argv)						\
{									\
	if (body != NULL) {						\
		return body();						\
	} else {							\
		return 0;						\
	}								\
}									\

#else

#define OSAL_LICENSE(...) MODULE_LICENSE(__VA_ARGS__)
#define OSAL_AUTHOR(...) MODULE_AUTHOR(__VA_ARGS__)
#define OSAL_DESCRIPTION(...) MODULE_DESCRIPTION(__VA_ARGS__)
#define OSAL_EXPORT_SYMBOL(...) EXPORT_SYMBOL(__VA_ARGS__)

#define OSAL_SETUP(init, body, fini)					\
static int __init osal_init(void)					\
{									\
	int rv;								\
	rv = init();							\
	if (rv == 0 && body != NULL) {					\
		rv = body();						\
	}								\
	return rv;							\
}									\
static void __exit osal_fini(void)					\
{									\
	return fini();							\
}									\
module_init(osal_init)							\
module_exit(osal_fini);							\

#endif










#endif

