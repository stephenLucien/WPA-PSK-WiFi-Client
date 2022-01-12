#ifndef KMP_SEARCH_H
#define KMP_SEARCH_H

#ifdef __cplusplus
#define EXTERNC extern "C" 
#else
#define EXTERNC 
#endif

EXTERNC void kmp_search(char* pat, char* txt, void (*str_handler)(char* found_str, void* userdata), void* userdata);

#endif