#include "wpa_wifi_internal.h"
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wpa_tools.h"

#define DEBUG 0
#define TRY_ON_INIT_FAIL 2

int wpa_enable_reset_config_on_init_fail = 1;

static struct wpa_ctrl* mctrl = NULL;

static inline int str_match(const char* a, const char* b) {
	return strncmp(a, b, strlen(b)) == 0;
}


static void on_wifi_scan_finished(enum WifiEventType event_type,
                                  const char* raw_msg, size_t raw_msg_len,
                                  void* userdata) {
	if (event_type == WIFI_EVENT_SCAN_FINISHED) {
		if (DEBUG) printf("[%s, %d] scan finished.\n", __FUNCTION__, __LINE__);
		wpa_wifi_get_infos_start(mctrl);
	}
}

int wpa_run_supplicant() {
	int ret = 0;
	char cmd[256];
	snprintf(cmd, sizeof(cmd), WPA_SUPPLICANT_BIN " -D" WPA_CTL_DRIVER " -i" WPA_CTL_IFACE " -c " WPA_CONF " -B");
	if (DEBUG) printf(cmd);
	system(cmd);
	return ret;
}

int wpa_stop_supplicant() {
	int ret = 0;
	char cmd[32];
	char reply[32];
	size_t reply_len = sizeof(reply) - 1;
	snprintf(cmd, sizeof(cmd), "TERMINATE");
	ret = wpa_ctrl_request(mctrl, cmd, sizeof(cmd), reply, &reply_len, NULL);
	reply[reply_len] = '\0';
	if (ret == -1) {
		// save or recv failed
		printf("[%s, %d] failed.\n", __FUNCTION__, __LINE__);
	} else if (ret == -2) {
		// timeout
		printf("[%s, %d] timeout.\n", __FUNCTION__, __LINE__);
	} else if (ret == 0) {
		// send command success
		if (str_match(reply, "FAIL")) {
			ret = -3;
		}

		if (DEBUG) printf("[%s, %d] %s\n", __FUNCTION__, __LINE__, reply);
	}
	return ret;
}

int wpa_ctrl_deinit() {
	int ret = 0;
	if (mctrl) {
		wpa_ctrl_close(mctrl);
		mctrl = NULL;
		wpa_wifi_event_callback_rm(WPA_SCAN_RESULTS_CALLBACK_ID);
	}
	return ret;
}

int wpa_ctrl_init() {
	int ret = -1;
	int retry = 0;

	if (mctrl) {
		return 0;
	} else {
	wpa_ctrl_init_retry:
		mctrl = wpa_ctrl_open(WPA_CTL_PATH "/" WPA_CTL_IFACE);
	}
	if (mctrl) {
		ret = 0;
		if (ret) {
			wpa_ctrl_deinit();
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
			goto wpa_ctrl_init_retry;
		} else {
			printf("[%s, %d] wpa_ctrl_open(" WPA_CTL_PATH "/" WPA_CTL_IFACE ") failed.\n", __FUNCTION__, __LINE__);
			return ret;
		}
	}
	wpa_wifi_event_callback_add(&on_wifi_scan_finished, NULL, WPA_SCAN_RESULTS_CALLBACK_ID);
	return ret;
}

int wpa_reset_config() {
	int ret = -1;
	FILE* fd = fopen(WPA_CONF, "w");
	if (fd) {
		if (DEBUG) {
			printf("[%s, %d] okay.\n", __FUNCTION__, __LINE__);
		}
		fprintf(fd, "ctrl_interface=%s\n", WPA_CTL_PATH);
		fprintf(fd, "update_config=1\n");
		fprintf(fd, "ap_scan=1\n");
		fclose(fd);
		ret = 0;
	} else {
		printf("[%s, %d] fopen(" WPA_CONF ", \"w\") failed.\n", __FUNCTION__, __LINE__);
	}
	return ret;
}

int wpa_save_config() {
	int ret = 0;
	char cmd[32];
	char reply[32];
	size_t reply_len = sizeof(reply) - 1;
	snprintf(cmd, sizeof(cmd), "SAVE_CONFIG");
	ret = wpa_ctrl_request(mctrl, cmd, sizeof(cmd), reply, &reply_len, NULL);
	reply[reply_len] = '\0';
	if (ret == -1) {
		// save or recv failed
		printf("[%s, %d] failed.\n", __FUNCTION__, __LINE__);
	} else if (ret == -2) {
		// timeout
		printf("[%s, %d] timeout.\n", __FUNCTION__, __LINE__);
	} else if (ret == 0) {
		// send command success
		if (str_match(reply, "FAIL")) {
			ret = -3;
			printf(
			    "The configuration could not be saved.\n"
			    "\n"
			    "The update_config=1 configuration option\n"
			    "must be used for configuration saving to\n"
			    "be permitted.\n");
		}

		if (DEBUG) printf("[%s, %d] %s\n", __FUNCTION__, __LINE__, reply);
	}
	return ret;
}

int wpa_reload_config() {
	int ret = 0;
	char cmd[32];
	char reply[32];
	size_t reply_len = sizeof(reply) - 1;
	snprintf(cmd, sizeof(cmd), "RECONFIGURE");
	ret = wpa_ctrl_request(mctrl, cmd, sizeof(cmd), reply, &reply_len, NULL);
	reply[reply_len] = '\0';
	if (ret == -1) {
		// save or recv failed
		printf("[%s, %d] failed.\n", __FUNCTION__, __LINE__);
	} else if (ret == -2) {
		// timeout
		printf("[%s, %d] timeout.\n", __FUNCTION__, __LINE__);
	} else if (ret == 0) {
		// send command success
		if (!str_match(reply, "OK")) {
			ret = -3;
		}
		if (DEBUG) printf("[%s, %d] %s\n", __FUNCTION__, __LINE__, reply);
	}
	return ret;
}

int wpa_wifi_scan() {
	int ret = 0;
	char cmd[32];
	char reply[32];
	size_t reply_len = sizeof(reply) - 1;
	snprintf(cmd, sizeof(cmd), "SCAN");
	ret = wpa_ctrl_request(mctrl, cmd, sizeof(cmd), reply, &reply_len, NULL);
	reply[reply_len] = '\0';
	if (ret == -1) {
		// save or recv failed
		printf("[%s, %d] failed.\n", __FUNCTION__, __LINE__);
	} else if (ret == -2) {
		// timeout
		printf("[%s, %d] timeout.\n", __FUNCTION__, __LINE__);
	} else if (ret == 0) {
		// send command success
		if (!str_match(reply, "OK")) {
			ret = -3;
		}
		if (DEBUG) printf("[%s, %d] %s\n", __FUNCTION__, __LINE__, reply);
	}
	return ret;
}

int wpa_wifi_remove(int network_id) {
	int ret = 0;
	char cmd[32];
	char reply[32];
	size_t reply_len = sizeof(reply) - 1;
	snprintf(cmd, sizeof(cmd), "REMOVE_NETWORK %d", network_id);
	ret = wpa_ctrl_request(mctrl, cmd, sizeof(cmd), reply, &reply_len, NULL);
	reply[reply_len] = '\0';
	if (ret == -1) {
		// save or recv failed
		printf("[%s, %d] failed.\n", __FUNCTION__, __LINE__);
	} else if (ret == -2) {
		// timeout
		printf("[%s, %d] timeout.\n", __FUNCTION__, __LINE__);
	} else if (ret == 0) {
		// send command success
		if (!str_match(reply, "OK")) {
			ret = -3;
		}
		if (DEBUG) printf("[%s, %d] %s\n", __FUNCTION__, __LINE__, reply);
	}
	return ret;
}

int wpa_wifi_add(int* network_id) {
	int ret = 0;
	char cmd[32];
	char reply[32];
	size_t reply_len = sizeof(reply) - 1;
	snprintf(cmd, sizeof(cmd), "ADD_NETWORK");
	ret = wpa_ctrl_request(mctrl, cmd, sizeof(cmd), reply, &reply_len, NULL);
	reply[reply_len] = '\0';
	if (ret == -1) {
		// save or recv failed
		printf("[%s, %d] failed.\n", __FUNCTION__, __LINE__);
	} else if (ret == -2) {
		// timeout
		printf("[%s, %d] timeout.\n", __FUNCTION__, __LINE__);
	} else if (ret == 0) {
		// send command success
		if (str_match(reply, "FAIL")) {
			ret = -3;
		} else {
			if (network_id) {
				sscanf(reply, "%d", network_id);
			}
		}
		if (DEBUG) printf("[%s, %d] %s\n", __FUNCTION__, __LINE__, reply);
	}
	return ret;
}

int wpa_wifi_select(int network_id) {
	int ret = 0;
	char cmd[32];
	char reply[32];
	size_t reply_len = sizeof(reply) - 1;
	snprintf(cmd, sizeof(cmd), "SELECT_NETWORK %d", network_id);
	ret = wpa_ctrl_request(mctrl, cmd, sizeof(cmd), reply, &reply_len, NULL);
	reply[reply_len] = '\0';
	if (ret == -1) {
		// save or recv failed
		printf("[%s, %d] failed.\n", __FUNCTION__, __LINE__);
	} else if (ret == -2) {
		// timeout
		printf("[%s, %d] timeout.\n", __FUNCTION__, __LINE__);
	} else if (ret == 0) {
		// send command success
		if (!str_match(reply, "OK")) {
			ret = -3;
		}
		if (DEBUG) printf("[%s, %d] %s\n", __FUNCTION__, __LINE__, reply);
	}
	return ret;
}

int wpa_wifi_set_ssid(int network_id, char* ssid) {
	int ret = 0;
	char cmd[256];
	char reply[32];
	size_t reply_len = sizeof(reply) - 1;
	snprintf(cmd, sizeof(cmd), "SET_NETWORK %d ssid \"%s\"", network_id, ssid);
	ret = wpa_ctrl_request(mctrl, cmd, sizeof(cmd), reply, &reply_len, NULL);
	reply[reply_len] = '\0';
	if (ret == -1) {
		// save or recv failed
		printf("[%s, %d] failed.\n", __FUNCTION__, __LINE__);
	} else if (ret == -2) {
		// timeout
		printf("[%s, %d] timeout.\n", __FUNCTION__, __LINE__);
	} else if (ret == 0) {
		// send command success
		if (!str_match(reply, "OK")) {
			ret = -3;
		}
		if (DEBUG) printf("[%s, %d] %s\n", __FUNCTION__, __LINE__, reply);
	}
	return ret;
}

int wpa_wifi_set_key_mgmt(int network_id, char* key_mgmt) {
	int ret = 0;
	char cmd[256];
	char reply[32];
	size_t reply_len = sizeof(reply) - 1;
	snprintf(cmd, sizeof(cmd), "SET_NETWORK %d key_mgmt %s", network_id, key_mgmt);
	ret = wpa_ctrl_request(mctrl, cmd, sizeof(cmd), reply, &reply_len, NULL);
	reply[reply_len] = '\0';
	if (ret == -1) {
		// save or recv failed
		printf("[%s, %d] failed.\n", __FUNCTION__, __LINE__);
	} else if (ret == -2) {
		// timeout
		printf("[%s, %d] timeout.\n", __FUNCTION__, __LINE__);
	} else if (ret == 0) {
		// send command success
		if (!str_match(reply, "OK")) {
			ret = -3;
		}
		if (DEBUG) printf("[%s, %d] %s\n", __FUNCTION__, __LINE__, reply);
	}
	return ret;
}

int wpa_wifi_set_psk(int network_id, char* psk) {
	if (psk == NULL) {
		return wpa_wifi_set_key_mgmt(network_id, "NONE");
	}
	int ret = 0;
	char cmd[256];
	char reply[32];
	size_t reply_len = sizeof(reply) - 1;
	snprintf(cmd, sizeof(cmd), "SET_NETWORK %d psk \"%s\"", network_id, psk);
	ret = wpa_ctrl_request(mctrl, cmd, sizeof(cmd), reply, &reply_len, NULL);
	reply[reply_len] = '\0';
	if (ret == -1) {
		// save or recv failed
		printf("[%s, %d] failed.\n", __FUNCTION__, __LINE__);
	} else if (ret == -2) {
		// timeout
		printf("[%s, %d] timeout.\n", __FUNCTION__, __LINE__);
	} else if (ret == 0) {
		// send command success
		if (!str_match(reply, "OK")) {
			ret = -3;
		}
		if (DEBUG) printf("[%s, %d] %s\n", __FUNCTION__, __LINE__, reply);
	}
	return ret;
}

int wpa_wifi_set_scan_ssid(int network_id, int enable) {
	int ret = 0;
	char cmd[256];
	char reply[32];
	size_t reply_len = sizeof(reply) - 1;
	snprintf(cmd, sizeof(cmd), "SET_NETWORK %d scan_ssid %d", network_id, enable);
	ret = wpa_ctrl_request(mctrl, cmd, sizeof(cmd), reply, &reply_len, NULL);
	reply[reply_len] = '\0';
	if (ret == -1) {
		// save or recv failed
		printf("[%s, %d] failed.\n", __FUNCTION__, __LINE__);
	} else if (ret == -2) {
		// timeout
		printf("[%s, %d] timeout.\n", __FUNCTION__, __LINE__);
	} else if (ret == 0) {
		// send command success
		if (!str_match(reply, "OK")) {
			ret = -3;
		}
		if (DEBUG) printf("[%s, %d] %s\n", __FUNCTION__, __LINE__, reply);
	}
	return ret;
}

int wpa_wifi_enable(int network_id) {
	int ret = 0;
	char cmd[32];
	char reply[32];
	size_t reply_len = sizeof(reply) - 1;
	snprintf(cmd, sizeof(cmd), "ENABLE_NETWORK %d", network_id);
	ret = wpa_ctrl_request(mctrl, cmd, sizeof(cmd), reply, &reply_len, NULL);
	reply[reply_len] = '\0';
	if (ret == -1) {
		// save or recv failed
		printf("[%s, %d] failed.\n", __FUNCTION__, __LINE__);
	} else if (ret == -2) {
		// timeout
		printf("[%s, %d] timeout.\n", __FUNCTION__, __LINE__);
	} else if (ret == 0) {
		// send command success
		if (!str_match(reply, "OK")) {
			ret = -3;
		}
		if (DEBUG) printf("[%s, %d] %s\n", __FUNCTION__, __LINE__, reply);
	}
	return ret;
}

int wpa_wifi_disable(int network_id) {
	int ret = 0;
	char cmd[32];
	char reply[32];
	size_t reply_len = sizeof(reply) - 1;
	snprintf(cmd, sizeof(cmd), "DISABLE_NETWORK %d", network_id);
	ret = wpa_ctrl_request(mctrl, cmd, sizeof(cmd), reply, &reply_len, NULL);
	reply[reply_len] = '\0';
	if (ret == -1) {
		// save or recv failed
		printf("[%s, %d] failed.\n", __FUNCTION__, __LINE__);
	} else if (ret == -2) {
		// timeout
		printf("[%s, %d] timeout.\n", __FUNCTION__, __LINE__);
	} else if (ret == 0) {
		// send command success
		if (!str_match(reply, "OK")) {
			ret = -3;
		}
		if (DEBUG) printf("[%s, %d] %s\n", __FUNCTION__, __LINE__, reply);
	}
	return ret;
}

int wpa_wifi_autoreconnect(int enable) {
	int ret = 0;
	char cmd[32];
	char reply[32];
	size_t reply_len = sizeof(reply) - 1;
	snprintf(cmd, sizeof(cmd), "STA_AUTOCONNECT %d", enable);
	ret = wpa_ctrl_request(mctrl, cmd, sizeof(cmd), reply, &reply_len, NULL);
	reply[reply_len] = '\0';
	if (ret == -1) {
		// save or recv failed
		printf("[%s, %d] failed.\n", __FUNCTION__, __LINE__);
	} else if (ret == -2) {
		// timeout
		printf("[%s, %d] timeout.\n", __FUNCTION__, __LINE__);
	} else if (ret == 0) {
		// send command success
		if (!str_match(reply, "OK")) {
			ret = -3;
		}
		if (DEBUG) printf("[%s, %d] %s\n", __FUNCTION__, __LINE__, reply);
	}
	return ret;
}

void wpa_wifi_status_dump(WiFiStatus* status) {
	if(status==NULL) return;
	printf("[WIFI STATUS]:\n");
	printf("ssid=%s\n",status->ap.ssid);
	printf("bssid=%s\n",status->ap.bssid);
	printf("key_mgmt=%s\n",status->key_mgmt);
	printf("wpa_state=%s\n",status->wpa_state);
	printf("freq(MHz)=%d\n",status->ap.frequency);
	printf("dBm=%d\n",status->ap.signal_level_dBm);
	printf("link_speed(Mbps)=%d\n",status->linkspeed);
	printf("flags:\n%s\n", status->ap.flag);
}

static int _wpa_wifi_status(WiFiStatus* status) {
	int ret = 0;
	char cmd[32];
	char reply[512];
	size_t reply_len = sizeof(reply) - 1;
	snprintf(cmd, sizeof(cmd), "STATUS");
	ret = wpa_ctrl_request(mctrl, cmd, sizeof(cmd), reply, &reply_len, NULL);
	reply[reply_len] = '\0';
	if (ret == -1) {
		// save or recv failed
		printf("[%s, %d] failed.\n", __FUNCTION__, __LINE__);
	} else if (ret == -2) {
		// timeout
		printf("[%s, %d] timeout.\n", __FUNCTION__, __LINE__);
	} else if (ret == 0) {
		// send command success
		if (str_match(reply, "FAIL")) {
			ret = -3;
		} else {
			// reply:
			// bssid=22:8c:0a:e5:2c:ef
			// freq=2412
			// ssid=note20
			// id=0
			// mode=station
			// wifi_generation=4
			// pairwise_cipher=CCMP
			// group_cipher=CCMP
			// key_mgmt=WPA2-PSK
			// wpa_state=COMPLETED
			// ip_address=192.168.17.64
			// address=2c:d2:6b:f7:59:87
			if(status){
				char buf[32];
				get_last_appear_value_from_reply_lines(reply, "ssid=",  "\n", (char*)&status->ap.ssid, sizeof(status->ap.ssid));
				get_last_appear_value_from_reply_lines(reply, "bssid=", "\n", (char*)&status->ap.bssid, sizeof(status->ap.bssid));
				get_last_appear_value_from_reply_lines(reply, "freq=",  "\n",(char*)&buf, sizeof(buf));
				sscanf(buf, "%d", &status->ap.frequency);
				get_last_appear_value_from_reply_lines(reply, "wpa_state=", "\n",(char*)&status->wpa_state, sizeof(status->wpa_state));
				get_last_appear_value_from_reply_lines(reply, "key_mgmt=",  "\n",(char*)&status->key_mgmt, sizeof(status->key_mgmt));			
			}
		}
		if (DEBUG) printf("[%s, %d] %s\n", __FUNCTION__, __LINE__, reply);
	}
	return ret;
}

static int _wpa_wifi_signal(WiFiStatus* status) {
	int ret = 0;
	char cmd[32];
	char reply[512];
	size_t reply_len = sizeof(reply) - 1;
	snprintf(cmd, sizeof(cmd), "SIGNAL_POLL");
	ret = wpa_ctrl_request(mctrl, cmd, sizeof(cmd), reply, &reply_len, NULL);
	reply[reply_len] = '\0';
	if (ret == -1) {
		// save or recv failed
		printf("[%s, %d] failed.\n", __FUNCTION__, __LINE__);
	} else if (ret == -2) {
		// timeout
		printf("[%s, %d] timeout.\n", __FUNCTION__, __LINE__);
	} else if (ret == 0) {
		// send command success
		if (str_match(reply, "FAIL")) {
			ret = -3;
		} else {
			// reply:
			// RSSI=-39
			// LINKSPEED=72
			// NOISE=9999
			// FREQUENCY=2412
			if(status){
				char buf[32]={0};
				get_last_appear_value_from_reply_lines(reply, "RSSI=", NULL, (char*)buf, sizeof(buf));
				sscanf(buf, "%d", &status->ap.signal_level_dBm);
				memset(buf, 0, sizeof(buf));
				get_last_appear_value_from_reply_lines(reply, "LINKSPEED=", NULL, (char*)buf, sizeof(buf));
				sscanf(buf, "%d", &status->linkspeed);								
			}
		}
		if (DEBUG) printf("[%s, %d] %s\n", __FUNCTION__, __LINE__, reply);
	}
	return ret;
}


int wpa_wifi_get_status(WiFiStatus* status){
	if(status){
		memset(status, 0, sizeof(WiFiStatus));
		_wpa_wifi_status(status);
		_wpa_wifi_signal(status);
	}
	return 0;
}