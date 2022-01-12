#ifndef DEFINE_TIMER_FUNCTION_H
#define DEFINE_TIMER_FUNCTION_H
#include <pthread.h>
#include <malloc.h>

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

// in header
#define DECLARE_TIMER_FUNCTIONS(ns, uniqflag) \
EXTERNC int ns##_##uniqflag##_timer_on(); \
EXTERNC int ns##_##uniqflag##_timer_off(); \
EXTERNC void ns##_##uniqflag##_timer_tv_ms(int timeval_ms)



#define DEFINE_TIMER_FUNCTIONS(ns, uniqflag, default_tv_ms, call) \
static int _##ns##_##uniqflag##_tv_ms = default_tv_ms; \
static unsigned char _##ns##_##uniqflag##_running = 0; \
static pthread_t _##ns##_##uniqflag##_pid; \
static pthread_mutex_t _##ns##_##uniqflag##_plock = PTHREAD_MUTEX_INITIALIZER; \
void ns##_##uniqflag##_timer_tv_ms(int timeval_ms) { \
    _##ns##_##uniqflag##_tv_ms = timeval_ms; \
} \
static void* _##ns##_##uniqflag##_thread(void *userdata) { \
	do { \
		call (); \
		usleep(1000 * _##ns##_##uniqflag##_tv_ms); \
	} while (_##ns##_##uniqflag##_running); \
	pthread_exit(NULL); \
} \
int ns##_##uniqflag##_timer_on() { \
	int ret = 0; \
	pthread_mutex_lock(&_##ns##_##uniqflag##_plock); \
	if (_##ns##_##uniqflag##_running == 0) { \
		_##ns##_##uniqflag##_running = 1; \
		ret = pthread_create(&_##ns##_##uniqflag##_pid, NULL, &_##ns##_##uniqflag##_thread, NULL); \
		if (ret) { \
			_##ns##_##uniqflag##_running = 0; \
			printf("%d %s: FAILED. %s\n", __LINE__, __FUNCTION__, strerror(ret)); \
		} \
	} \
	pthread_mutex_unlock(&_##ns##_##uniqflag##_plock); \
	return ret; \
} \
int ns##_##uniqflag##_timer_off() { \
	int ret = 0; \
	pthread_mutex_lock(&_##ns##_##uniqflag##_plock); \
	if (_##ns##_##uniqflag##_running) { \
		_##ns##_##uniqflag##_running = 0; \
		ret = pthread_join(_##ns##_##uniqflag##_pid, NULL); \
	} \
	pthread_mutex_unlock(&_##ns##_##uniqflag##_plock); \
	return ret; \
}


#endif