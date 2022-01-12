#ifndef DEFINE_CALLBACK_LIST_H
#define DEFINE_CALLBACK_LIST_H
#include <sys/queue.h>
#include <pthread.h>
#include <malloc.h>

#ifdef __cplusplus
#define EXTERNC extern "C" 
#else
#define EXTERNC 
#endif

// in header
#define DECLARE_CALLBACK_MAINTAIN_FUNCTIONS(ns, uniqflag, callbackname) \
EXTERNC int ns##_##uniqflag##_callback_add(callbackname callback, void *userdata, int id); \
EXTERNC int ns##_##uniqflag##_callback_rm(int id); \
EXTERNC int ns##_##uniqflag##_callback_clear() 


// in .c file
#define DEFINE_CALLBACK_MAINTAIN_STRUCTURE(ns, uniqflag, callbackname) \
struct callbackname##Node { \
	LIST_ENTRY(callbackname##Node) node; \
	callbackname callback; \
	void *userdata; \
	int id; \
}; \
struct callbackname##List { \
	LIST_HEAD(callbackname##ListHead, callbackname##Node) head; \
	pthread_mutex_t mutex; \
}; 

// in .c file
#define DEFINE_CALLBACK_MAINTAIN_FUNCTIONS(ns, uniqflag, callbackname) \
static struct callbackname##List _##ns##_##uniqflag##_callbacklist = {{0},PTHREAD_MUTEX_INITIALIZER}; \
int ns##_##uniqflag##_callback_add(callbackname callback, void *userdata, int id) { \
	if (!callback) { \
		return -1; \
	} \
	struct callbackname##List *list = &_##ns##_##uniqflag##_callbacklist; \
	struct callbackname##Node *node = (struct callbackname##Node *)malloc(sizeof(struct callbackname##Node)); \
	if (!node) return -1; \
	node->callback = callback; \
	node->id = id; \
	node->userdata = userdata; \
	ns##_##uniqflag##_callback_rm(id); \
	pthread_mutex_lock(&list->mutex); \
	LIST_INSERT_HEAD(&list->head, node, node); \
	pthread_mutex_unlock(&list->mutex); \
	return 0; \
} \
int ns##_##uniqflag##_callback_rm(int id) { \
	struct callbackname##List *list = &_##ns##_##uniqflag##_callbacklist; \
	struct callbackname##Node *node_ir = NULL, *target_node = NULL; \
    int is_rm=0; \
	pthread_mutex_lock(&list->mutex); \
	LIST_FOREACH(node_ir, &list->head, node) { \
		if (node_ir->id == id) { \
			target_node = node_ir; \
			break; \
		} \
	} \
	if (target_node) { \
		LIST_REMOVE(target_node, node); \
		free(target_node); \
        is_rm=1; \
	} \
	pthread_mutex_unlock(&list->mutex); \
	return is_rm; \
} \
int ns##_##uniqflag##_callback_clear() { \
	struct callbackname##List *list = &_##ns##_##uniqflag##_callbacklist; \
	struct callbackname##Node *first_node; \
	pthread_mutex_lock(&list->mutex); \
	while ((first_node = LIST_FIRST(&list->head))) { \
		LIST_REMOVE(first_node, node); \
		free(first_node); \
	} \
	pthread_mutex_unlock(&list->mutex); \
	return 0; \
} 

#define RUN_CALLBACK_FUNCTIONS(ns, uniqflag, callbackname, args...) do { \
	struct callbackname##List *list = &_##ns##_##uniqflag##_callbacklist; \
	struct callbackname##Node *node = NULL; \
	pthread_mutex_lock(&list->mutex); \
	LIST_FOREACH(node, &list->head, node) { \
		node->callback(args, node->userdata); \
	} \
	pthread_mutex_unlock(&list->mutex); \
} while(0)


#endif