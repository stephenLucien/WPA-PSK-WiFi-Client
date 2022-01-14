#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "wpa_wifi.h"

static char ssid[128] = "note20";
static char psk[32] = "123454323";
static int is_wep_key = 0;
static int network_id = 0;

static void help(char* program) {
	printf("%s <ssid> <passwd> [is_wep:0/1]\n", program);
}

static void on_scan_results_ready() {
	WifiAP* mdata;
	int data_cnt;
	wpa_wifi_infos_lock();
	wpa_wifi_infos_get(&mdata, &data_cnt);
	printf("bssid\tfreq(MHz)\tsignal_level_dBm\tflags\tssid\n");
	wpa_wifi_infos_dump2stdout(mdata, data_cnt);
	wpa_wifi_infos_unlock();
}

static int on_wifi_connected() {
	int ret = 0;
	ret = wpa_save_config();
	if (ret) {
	}
	return ret;
}

static void test_event_results(enum WifiEventType event_type, const char* raw_msg, size_t raw_msg_len, void* userdata) {
	if (0) {
	} else if (event_type == WIFI_EVENT_SCAN_FINISHED) {
		printf("wifi scan finished.\n");
	} else if (event_type == WIFI_EVENT_SCAN_RESULTS_READY) {
		printf("wifi scan results ready.\n");
		if (1) on_scan_results_ready();
	} else if (event_type == WIFI_EVENT_CONNECTED) {
		printf("wifi connected.\n");
		if (1) on_wifi_connected();
	} else if (event_type == WIFI_EVENT_DISCONNECTED) {
		printf("wifi disconnected.\n");
	} else if (event_type == WIFI_EVENT_CONNECTED) {
		printf("wifi connected.\n");
	} else if (event_type == WIFI_EVENT_INVALID_PASSWD) {
		printf("wifi passwd incorrect.\n");
	}
}

int main(int argc, char* argv[]) {
	int ret = 0;
	if(argc<2){
		help(argv[0]);
		return ret;
	} 
	if(argc>=2){
		snprintf(ssid, sizeof(ssid), "%s", argv[1]);
	}
	if(argc>=3){
		snprintf(psk, sizeof(psk), "%s", argv[2]);
	}
	if(argc>=4){
		sscanf(argv[3], "%d", &is_wep_key);
	}	
	
	// init event function
	ret = wpa_wifi_event_start();
	if (ret) {
	}
	ret = wpa_wifi_event_callback_add(&test_event_results, NULL, 11);
	if (ret) {
	}

	// init control
	ret = wpa_ctrl_init();
	if (ret) {
	}

	// perform scan
	ret = wpa_wifi_scan();
	if (ret) {
	}

	//
	ret = wpa_wifi_remove(network_id);
	if (ret) {
	}

	// connect wifi
	ret = wpa_wifi_add(&network_id);
	if (ret) {
	}

	ret = wpa_wifi_set_ssid(network_id, ssid);
	if (ret) {
	}

	if (is_wep_key) {
		ret = wpa_wifi_set_wep_key(network_id, psk);
	} else {
		ret = wpa_wifi_set_psk(network_id, psk);
	}
	if (ret) {
	}

	ret = wpa_wifi_set_scan_ssid(network_id, 1);
	if (ret) {
	}

	ret = wpa_wifi_enable(network_id);
	if (ret) {
	}

	ret = wpa_wifi_autoreconnect(1);
	if (ret) {
	}

	ret = wpa_wifi_select(network_id);
	if (ret) {
	}

	// wait sometime to test event functions.
	int i = 0;
	while (i++ < 15) {
		sleep(1);
	}

	WiFiStatus status;
	ret = wpa_wifi_get_status(&status);
	if (ret) {
	}
	wpa_wifi_status_dump(&status);

	// deinit
	ret = wpa_ctrl_deinit();
	if (ret) {
	}

	ret = wpa_wifi_event_stop();
	if (ret) {
	}

	wpa_wifi_event_callback_clear();

	return ret;
}