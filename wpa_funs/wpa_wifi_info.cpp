#include "wpa_wifi_internal.h"
//
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cstddef>
#include <vector>

#include "wpa_tools.h"

#define DEBUG 0

static std::vector<WifiAP> mdata;
static pthread_mutex_t mlock = PTHREAD_MUTEX_INITIALIZER;
static int flag = 0;

void wpa_wifi_info_reset_flag() {
	flag = 0;
}

int wpa_wifi_info_get_flag() {
	return flag;
}

int wpa_wifi_infos_clear(){
	int ret=0;
	mdata.clear();
	return ret;
}

void wpa_wifi_info_dump2stdout(WifiAP *data) {
	if (DEBUG) printf("[%s, %d]\n", __FUNCTION__, __LINE__);
	if (data) {
		if (0) {
			printf("bssid:\n%s\n freq:\n%d\n dBm:\n%d\n flags:\n%s\n ssid:\n%s\n", data->bssid, data->frequency, data->signal_level_dBm, data->flag, data->ssid);
		} else
			printf("%s\t%d\t%d\t%s\t%s\n", data->bssid, data->frequency, data->signal_level_dBm, data->flag, data->ssid);
	}
}

int wpa_wifi_flag_wep(WifiAP* data){
	if(data==NULL) return 0;
	char *pos = get_substr_locate((const char *)data->flag, "WEP");
	return pos ? 1:0;
}


int wpa_wifi_flag_wpa(WifiAP* data){
	if(data==NULL) return 0;
	char *pos = get_substr_locate((const char *)data->flag, "WPA-PSK");
	return pos ? 1:0;
}


int wpa_wifi_flag_wpa2(WifiAP* data){
	if(data==NULL) return 0;
	char *pos = get_substr_locate((const char *)data->flag, "WPA2-PSK");
	return pos ? 1:0;
}

int wpa_wifi_infos_lock() { return pthread_mutex_lock(&mlock); }
int wpa_wifi_infos_get(WifiAP **data, int *data_cnt) {
	if (DEBUG) printf("[%s, %d]\n", __FUNCTION__, __LINE__);
	int ret = 0;
	if (data) {
		*data = &mdata[0];
	}
	if (data_cnt) {
		*data_cnt = mdata.size();
	}
	return ret;
}
void wpa_wifi_infos_dump2stdout(WifiAP *data, int data_cnt) {
	if (DEBUG) printf("[%s, %d]\n", __FUNCTION__, __LINE__);
	if (data) {
		for (int i = 0; i < data_cnt; i++) {
			wpa_wifi_info_dump2stdout(data + i);
		}
	}
}
int wpa_wifi_infos_unlock() { return pthread_mutex_unlock(&mlock); }

static inline int str_match(const char *a, const char *b) {
	return strncmp(a, b, strlen(b)) == 0;
}

static int wpa_wifi_get_info(struct wpa_ctrl *handle, int network_id) {
	if (DEBUG) printf("[%s, %d] (id=%d)\n", __FUNCTION__, __LINE__, network_id);
	int ret = 0;
	char cmd[32];
	char reply[2048];
	size_t reply_len = sizeof(reply) - 1;
	snprintf(cmd, sizeof(cmd), "BSS %d", network_id);
	ret = wpa_ctrl_request(handle, cmd, sizeof(cmd), reply, &reply_len, NULL);
	reply[reply_len] = '\0';
	if (ret == -1) {
		// command failed.
	} else if (ret == -2) {
		// timeout
	} else if (ret == 0) {
		// send command success
		if (strlen(reply) == 0) {
			// no more scan results available.
			ret = -1;
		} else if (!str_match(reply, "FAIL")) {
			if (DEBUG) printf("[%s, %d]\n", __FUNCTION__, __LINE__);
			WifiAP ap = {{0}};

			int cnt_r = 0;
			ret = get_last_appear_value_from_reply_lines(reply, "ssid=", NULL, ap.ssid, sizeof(ap.ssid));
			if (ret == 0) cnt_r++;
			ret = get_last_appear_value_from_reply_lines(reply, "bssid=", NULL, ap.bssid, sizeof(ap.bssid));
			if (ret == 0) cnt_r++;
			ret = get_last_appear_value_from_reply_lines(reply, "flags=", NULL, ap.flag, sizeof(ap.flag));
			if (ret == 0) cnt_r++;
			char freq_buf[16] = {0};
			ret = get_last_appear_value_from_reply_lines(reply, "freq=", NULL, freq_buf, sizeof(freq_buf));
			if (ret == 0) {
				ret = sscanf(freq_buf, "%d", &ap.frequency);
				if (ret == 1) cnt_r++;
			}
			char sig_buf[16] = {0};
			ret = get_last_appear_value_from_reply_lines(reply, "level=", NULL, sig_buf, sizeof(sig_buf));
			if (ret == 0) {
				ret = sscanf(sig_buf, "%d", &ap.signal_level_dBm);
				if (ret == 1) cnt_r++;
			}
			//wpa_wifi_info_dump2stdout(&ap);

			if (DEBUG) printf("[%s, %d]\n", __FUNCTION__, __LINE__);
			if (cnt_r == 5) {
				mdata.push_back(std::move(ap));
			}
			if (DEBUG) printf("[%s, %d]\n", __FUNCTION__, __LINE__);
		}
	}
	return ret;
}

static int g_thread_running = 0;
static pthread_t g_tid;
static void *wpa_wifi_get_infos_thread(void *userdata) {
	if (DEBUG) printf("[%s, %d]\n", __FUNCTION__, __LINE__);
	int i = 0;
	int ret = 0;
	struct wpa_ctrl *handle = (struct wpa_ctrl *)userdata;
	if (handle) {
		pthread_mutex_lock(&mlock);
		mdata.clear();
		while (g_thread_running && ret != -1 && i < WPA_RESULTS_MAX_LEN) {
			ret = wpa_wifi_get_info((struct wpa_ctrl *)userdata, i);
			i++;
		}
		if (mdata.size()) flag = 1;
		pthread_mutex_unlock(&mlock);
	}

	g_thread_running = 0;
	pthread_exit(NULL);
}

int wpa_wifi_get_infos_start(struct wpa_ctrl *handle) {
	if (DEBUG) printf("[%s, %d]\n", __FUNCTION__, __LINE__);
	int ret = 0;
	if (g_thread_running) {
		// printf("");
	} else {
		g_thread_running = 1;
		ret = pthread_create(&g_tid, NULL, &wpa_wifi_get_infos_thread, handle);
		if (ret) {
			g_thread_running = 0;
		} else {
			if (DEBUG) printf("[%s, %d]\n", __FUNCTION__, __LINE__);
			pthread_detach(g_tid);
			if (DEBUG) printf("[%s, %d]\n", __FUNCTION__, __LINE__);
		}
	}
	return ret;
}

int wpa_wifi_get_infos_stop() {
	if (DEBUG) printf("[%s, %d]\n", __FUNCTION__, __LINE__);
	int ret = 0;
	if (g_thread_running) {
		g_thread_running = 0;
	}
	return ret;
}
