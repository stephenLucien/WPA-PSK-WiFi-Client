#include "define_timer_function.h"
#include "wpa_wifi_internal.h"
//
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//
#include "wpa_tools.h"

#define DEBUG 0
#define TRY_ON_INIT_FAIL 2
#define BUF_LEN (256)

DEFINE_CALLBACK_MAINTAIN_STRUCTURE(wpa, wifi_event, WifiEventReceiver);
DEFINE_CALLBACK_MAINTAIN_FUNCTIONS(wpa, wifi_event, WifiEventReceiver);

static struct wpa_ctrl* mctrl = NULL;

static inline int str_match(const char* a, const char* b) {
	return strncmp(a, b, strlen(b)) == 0;
}

static void reply_handler(char* reply, size_t reply_len) {
	char* pos = reply;
	//int priority = 2;

	if (*pos == '<') {
		/* skip priority */
		//pos++;
		//priority = atoi(pos);
		pos = strchr(pos, '>');
		if (pos)
			pos++;
		else
			pos = reply;
	}

	enum WifiEventType event_type;
	if (str_match(pos, WPA_EVENT_SCAN_RESULTS)) {
		event_type = WIFI_EVENT_SCAN_FINISHED;
	} else if (str_match(pos, WPA_EVENT_DISCONNECTED)) {
		event_type = WIFI_EVENT_DISCONNECTED;
	} else if (str_match(pos, WPA_EVENT_CONNECTED)) {
		event_type = WIFI_EVENT_CONNECTED;
	} else if (str_match(pos, WPA_EVENT_TEMP_DISABLED)) {
		// <3>CTRL-EVENT-SSID-TEMP-DISABLED id=0 ssid="MyPlace" auth_failures=1 duration=10 reason=WRONG_KEY
		char buf[16] = {0};
		int id = -1;
		int ret = 0;
		get_first_appear_value_from_reply_line(reply, " id=", NULL, buf, sizeof(buf));
		ret = sscanf(buf, "%d", &id);

		char reason[16];
		get_last_appear_value_from_reply_line(reply, " reason=", NULL, reason, sizeof(reason));
		if (ret == 1 && id == WPA_NETWORK_ID && str_match(reason, "WRONG_KEY")) {
			event_type = WIFI_EVENT_INVALID_PASSWD;
		}
	} else {
		event_type = WIFI_EVENT_OTHER;
	}
	RUN_CALLBACK_FUNCTIONS(wpa, wifi_event, WifiEventReceiver, event_type, reply, reply_len);
}


static void wpa_wifi_event_process() {
	int ret = 0;
	char reply[BUF_LEN] = {0};
	size_t reply_len = BUF_LEN;
	if (mctrl && 1 == wpa_ctrl_pending(mctrl)) {
		reply_len = BUF_LEN - 1;
		ret = wpa_ctrl_recv(mctrl, reply, &reply_len);
		reply[reply_len] = '\0';
		if (ret == 0) {
			reply_handler(reply, reply_len);
		}
	} else {
		usleep(500000);
	}
}
DEFINE_TIMER_FUNCTIONS(wpa, wifi_event_handler, 0, wpa_wifi_event_process);

static void wpa_wifi_scan_results_ready_check() {
	int ret = 0;
	enum WifiEventType event_type;
	ret = wpa_wifi_info_get_flag();
	if (ret) {
		event_type = WIFI_EVENT_SCAN_RESULTS_READY;
		RUN_CALLBACK_FUNCTIONS(wpa, wifi_event, WifiEventReceiver, event_type, NULL, 0);
		wpa_wifi_info_reset_flag();
	}
}
DEFINE_TIMER_FUNCTIONS(wpa, wifi_event_result_update, 500, wpa_wifi_scan_results_ready_check);

static int wpa_wifi_event_deinit() {
	int ret = 0;
	if (mctrl) {
		wpa_ctrl_detach(mctrl);
		wpa_ctrl_close(mctrl);
	}
	return ret;
}

static int wpa_wifi_event_init() {
	int ret = -1;
	int retry = 0;

	if (mctrl) {
		return 0;
	} else {
	wpa_wifi_event_init_retry:
		mctrl = wpa_ctrl_open(WPA_CTL_PATH "/" WPA_CTL_IFACE);
	}
	if (mctrl) {
		ret = wpa_ctrl_attach(mctrl);
		if (ret) {
			printf("[%s, %d] wpa_ctrl_attach failed.\n", __FUNCTION__, __LINE__);
			wpa_wifi_event_deinit();
			return ret;
		}
	} else {
		// if wpa_ctrl_open failed, wpa_supplicant may not run,
		// try to run wpa_supplicant first, and if it still fails to
		// run wpa_supplicant, then reset config and try again.
		if (retry++ < TRY_ON_INIT_FAIL) {
			if (retry > 1 && wpa_enable_reset_config_on_init_fail) {
				wpa_reset_config();
			}
			wpa_run_supplicant();
			sleep(2);
			goto wpa_wifi_event_init_retry;
		} else {
			printf("[%s, %d] wpa_ctrl_open(" WPA_CTL_PATH "/" WPA_CTL_IFACE ") failed.\n", __FUNCTION__, __LINE__);
			return ret;
		}
	}
	return ret;
}

int wpa_wifi_event_stop() {
	if (DEBUG) printf("[%s, %d]\n", __FUNCTION__, __LINE__);
	int ret = 0;

	ret = wpa_wifi_event_result_update_timer_off();
	if (ret) {
	}	
	ret = wpa_wifi_event_handler_timer_off();
	if (ret) {
	}
	ret = wpa_wifi_event_deinit();
	if (ret) {
	}
	return ret;
}

int wpa_wifi_event_start() {
	if (DEBUG) printf("[%s, %d]\n", __FUNCTION__, __LINE__);
	int ret = 0;
	ret = wpa_wifi_event_init();
	if (ret) {
		printf("[%s, %d] wpa_wifi_event_init failed.\n", __FUNCTION__, __LINE__);
		return ret;
	}
	ret = wpa_wifi_event_handler_timer_on();
	if (ret) {
	}
	ret = wpa_wifi_event_result_update_timer_on();
	if (ret) {
	}
	return ret;
}
