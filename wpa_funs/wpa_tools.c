#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
//
#include "kmp_search.h"

#define DEBUG 0

static inline int str_match(const char* a, const char* b) {
	return strncmp(a, b, strlen(b)) == 0;
}

typedef struct _TmpData {
	char* begin;
	char* pos;
} TmpData;


static void _locate_first_str_at_line(char* found_str, void* userdata) {
	TmpData* data = (TmpData*)userdata;
	if (data && found_str) {
		if (DEBUG) printf("[%s, %d]:\n", __FUNCTION__, __LINE__);
		if (!data->pos) {
			if (DEBUG) printf("[%s, %d]:\n", __FUNCTION__, __LINE__);
			data->pos = found_str;
		}
	}
}

static void _locate_last_str_at_line(char* found_str, void* userdata) {
	TmpData* data = (TmpData*)userdata;
	if (data && found_str) {
		if (DEBUG) printf("[%s, %d]:\n", __FUNCTION__, __LINE__);
		data->pos = found_str;
	}
}

static void _locate_first_str_at_lines(char* found_str, void* userdata) {
	TmpData* data = (TmpData*)userdata;
	if (data && data->begin && found_str) {
		if (DEBUG) printf("[%s, %d]:\n", __FUNCTION__, __LINE__);
		if (found_str == data->begin) {
			if (DEBUG) printf("[%s, %d]:\n", __FUNCTION__, __LINE__);
			if (!data->pos) {
				if (DEBUG) printf("[%s, %d]:\n", __FUNCTION__, __LINE__);
				data->pos = found_str;
			}
		} else if (*(found_str - 1) == '\n') {
			if (DEBUG) printf("[%s, %d]:\n", __FUNCTION__, __LINE__);
			if (!data->pos) {
				if (DEBUG) printf("[%s, %d]:\n", __FUNCTION__, __LINE__);
				data->pos = found_str;
			}
		}
	}
}

static void _locate_last_str_at_lines(char* found_str, void* userdata) {
	TmpData* data = (TmpData*)userdata;
	if (data && data->begin && found_str) {
		if (DEBUG) printf("[%s, %d]:\n", __FUNCTION__, __LINE__);
		if (found_str == data->begin) {
			if (DEBUG) printf("[%s, %d]:\n", __FUNCTION__, __LINE__);
			data->pos = found_str;
		} else if (*(found_str - 1) == '\n') {
			if (DEBUG) printf("[%s, %d]:\n", __FUNCTION__, __LINE__);
			data->pos = found_str;
		}
		if (DEBUG) {
			char buf[16];
			snprintf(buf, sizeof(buf), "%s", found_str);
			printf("reply:\n%s\n\nsnapshot:\n%s\n", data->begin, found_str);
		}
	}
	if (DEBUG) printf("\n\n");
}


char* get_substr_locate(const char* reply, const char *substr) {
	if (reply ==NULL || strlen(reply) == 0 || substr==NULL || strlen(substr) == 0)
		return NULL;
	TmpData data = {0};
	data.begin = (char*)reply;
	kmp_search((char*)substr, (char*)reply, &_locate_first_str_at_line, &data);
	return data.pos;
}


int get_first_appear_value_from_reply_line(char* reply, const char* key, const char* terminate_key, char* value_out, size_t value_out_len) {
	if (DEBUG) printf("[%s, %d]:\n", __FUNCTION__, __LINE__);
	if (strlen(reply) == 0 || strlen(key) == 0 || value_out_len <= 1)
		return -1;
	if (DEBUG) printf("reply:%p\n", reply);
	if (DEBUG) printf("key: %s\n", key);
	if (DEBUG) printf("terminate key: %s\n", terminate_key ? terminate_key : "[space].*=");
	TmpData data = {0};
	TmpData data2 = {0};
	data.begin = reply;
	kmp_search((char*)key, reply, &_locate_first_str_at_line, &data);
	if (data.pos && str_match(data.pos, key)) {
		if (DEBUG) printf("locate:%p\n", data.pos);
		char* pos = data.pos + strlen(key);
		char* end = NULL;
		int len = 0;
		if (terminate_key && strlen(terminate_key)) {
			data2.begin = pos;
			kmp_search((char*)terminate_key, pos, &_locate_last_str_at_line, &data2);
			end = data2.pos;
		} else {
			end = strchr(pos, '=');
			if (end) {
				while (end > pos) {
					end--;
					if (isspace(*end)) {
						break;
					}
				}
			}
		}
		if (DEBUG) printf("end:%p\n", end ? end : "NULL\n");
		len = end ? end - pos : strlen(pos);
		if (len >= value_out_len) {
			//
			printf("value_out space is too small.\n");
            len = value_out_len - 1;
		}
		strncpy(value_out, pos, len);
		value_out[len] = '\0';
		return 0;
	} else {
		if (DEBUG) printf("cannot locate.\n");
	}
	return -1;
}

int get_last_appear_value_from_reply_line(char* reply, const char* key, const char* terminate_key, char* value_out, size_t value_out_len) {
	if (DEBUG) printf("[%s, %d]:\n", __FUNCTION__, __LINE__);
	if (strlen(reply) == 0 || strlen(key) == 0 || value_out_len <= 1)
		return -1;
	if (DEBUG) printf("reply:%p\n", reply);
	if (DEBUG) printf("key: %s\n", key);
	if (DEBUG) printf("terminate key: %s\n", terminate_key ? terminate_key : "[space].*=");
	TmpData data = {0};
	TmpData data2 = {0};
	data.begin = reply;
	kmp_search((char*)key, reply, &_locate_last_str_at_line, &data);
	if (data.pos && str_match(data.pos, key)) {
		if (DEBUG) printf("locate:%p\n", data.pos);
		char* pos = data.pos + strlen(key);
		char* end = NULL;
		int len = 0;
		if (terminate_key && strlen(terminate_key)) {
			data2.begin = pos;
			kmp_search((char*)terminate_key, pos, &_locate_last_str_at_line, &data2);
			end = data2.pos;
		} else {
			end = strchr(pos, '=');
			if (end) {
				while (end > pos) {
					end--;
					if (isspace(*end)) {
						break;
					}
				}
			}
		}
		if (DEBUG) printf("end:%p\n", end ? end : "NULL\n");
		len = end ? end - pos : strlen(pos);
		if (len >= value_out_len) {
			//
			printf("value_out space is too small.\n");
            len = value_out_len - 1;
		}
		strncpy(value_out, pos, len);
		value_out[len] = '\0';
		return 0;
	} else {
		if (DEBUG) printf("cannot locate.\n");
	}
	return -1;
}

int get_first_appear_value_from_reply_lines(char* reply, const char* key, const char* terminate_key, char* value_out, size_t value_out_len) {
	if (DEBUG) printf("[%s, %d]:\n", __FUNCTION__, __LINE__);
	if (strlen(reply) == 0 || strlen(key) == 0 || value_out_len <= 1)
		return -1;
	if (DEBUG) printf("reply:%p\n", reply);
	if (DEBUG) printf("key: %s\n", key);
	if (DEBUG) printf("terminate key: %s\n", terminate_key ? terminate_key : "\\n");
	TmpData data = {0};
	TmpData data2 = {0};
    char* default_terminate_key = "\n";
	data.begin = reply;
	kmp_search((char*)key, reply, &_locate_first_str_at_lines, &data);
	if (data.pos && str_match(data.pos, key)) {
		if (DEBUG) printf("locate:%p\n", data.pos);
		char* pos = data.pos + strlen(key);
		char* end = NULL;
		int len = 0;
		data2.begin = pos;
		kmp_search((terminate_key && strlen(terminate_key)) ? (char*)terminate_key : (char*)default_terminate_key, pos, &_locate_first_str_at_line, &data2);
		end = data2.pos;
		if (DEBUG) printf("end:%p\n", end ? end : "NULL\n");
		len = end ? end - pos : strlen(pos);
		if (len >= value_out_len) {
			//
			printf("value_out space is too small.\n");
            len = value_out_len - 1;
		}
		strncpy(value_out, pos, len);
		value_out[len] = '\0';
		return 0;
	} else {
		if (DEBUG) printf("cannot locate.\n");
	}
	return -1;
}

int get_last_appear_value_from_reply_lines(char* reply, const char* key, const char* terminate_key, char* value_out, size_t value_out_len) {
	if (DEBUG) printf("[%s, %d]:\n", __FUNCTION__, __LINE__);
	if (strlen(reply) == 0 || strlen(key) == 0 || value_out_len <= 1)
		return -1;
	if (DEBUG) printf("reply:%p\n", reply);
	if (DEBUG) printf("key: %s\n", key);
	if (DEBUG) printf("terminate key: %s\n", terminate_key ? terminate_key : "\\n");
	TmpData data = {0};
	TmpData data2 = {0};
	char* default_terminate_key = "\n";
	data.begin = reply;
	kmp_search((char*)key, reply, &_locate_last_str_at_lines, &data);
	if (data.pos && str_match(data.pos, key)) {
		if (DEBUG) printf("locate:%p\n", data.pos);
		char* pos = data.pos + strlen(key);
		char* end = NULL;
		int len = 0;
		data2.begin = pos;
		kmp_search((terminate_key && strlen(terminate_key)) ? (char*)terminate_key : (char*)default_terminate_key, pos, &_locate_first_str_at_line, &data2);
		end = data2.pos;

		if (DEBUG) printf("end:%p\n", end ? end : "NULL\n");
		len = end ? end - pos : strlen(pos);
		if (len >= value_out_len) {
			//
			printf("value_out space is too small.\n");
            len = value_out_len - 1;
		}
		strncpy(value_out, pos, len);
		value_out[len] = '\0';
		//printf("key:\"%s\", terminate key%s: \"%s\" -->\n%s\n\n", key, end?"(found)":"(not found)", terminate_key ? terminate_key : "\\n", value_out);
		return 0;
	} else {
		if (DEBUG) printf("cannot locate.\n");
	}
	return -1;
}
