#ifndef WPA_TOOLS_H
#define WPA_TOOLS_H

#include <stddef.h>

#ifdef __cplusplus
#define EXTERNC extern "C" 
#else
#define EXTERNC 
#endif

EXTERNC char* get_substr_locate(const char* reply, const char *substr);

EXTERNC int get_first_appear_value_from_reply_line(char* reply, const char* key, const char* terminate_key, char* value_out, size_t value_out_len) ;
EXTERNC int get_last_appear_value_from_reply_line(char* reply, const char* key, const char* terminate_key, char* value_out, size_t value_out_len) ;

EXTERNC int get_first_appear_value_from_reply_lines(char* reply, const char* key, const char* terminate_key, char* value_out, size_t value_out_len) ;
EXTERNC int get_last_appear_value_from_reply_lines(char* reply, const char* key, const char* terminate_key, char* value_out, size_t value_out_len) ;


#endif