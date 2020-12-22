
// Internal 
#include "user_config.h"
#include "partition.h"
#include "wifi.h"
#include "params.h" 
#include "debug.h"
#include "status.h"

// SDK
#include <ets_sys.h>
#include <osapi.h>
#include <mem.h>
#include <user_interface.h>
#include <driver/uart.h>
#include <upgrade.h>
#include <c_types.h>
#include <ip_addr.h> 
#include <espconn.h>


#define __version__     "0.1.0"

static Params params;
static struct mdns_info mdns;


static ICACHE_FLASH_ATTR 
void reboot_appmode() {
	system_upgrade_flag_set(UPGRADE_FLAG_FINISH);
	system_upgrade_reboot();
}


static ICACHE_FLASH_ATTR 
void _mdns_init() {
	struct ip_info ipconfig;
	wifi_set_broadcast_if(STATIONAP_MODE);
	wifi_get_ip_info(STATION_IF, &ipconfig);
	mdns.ipAddr = ipconfig.ip.addr; //ESP8266 Station IP
	mdns.host_name = params.name;
	mdns.server_name = "ESPWebAdmin";
	mdns.server_port = 80;
	mdns.txt_data[0] = "version = "__version__;
	espconn_mdns_init(&mdns);
}


void wifi_connect_cb(uint8_t status) {
    if(status == STATION_GOT_IP) {
		_mdns_init();
        INFO("Fota image version: "__version__"\r\n");
        INFO("Reboot in %d seconds\r\n", REBOOTDELAY);
        status_update(500, 500, REBOOTDELAY, reboot_appmode);
    } else {
		espconn_mdns_close();
    }
}


void user_init(void) {
    uart_init(BIT_RATE_115200, BIT_RATE_115200);
    os_delay_us(60000);
	bool ok = params_load(&params);
	if (!ok) {
		ERROR("Cannot load Params\r\n");
#if !WIFI_ENABLE_SOFTAP
		return;
#endif
		if(!params_defaults(&params)) {
			ERROR("Cannot save params\r\n");
			return;
		}
	}
    
    PARAMS_PRINT(&params);
	
    // Disable wifi led before infrared
    wifi_status_led_uninstall();

    // Status LED
    status_init();
    status_update(500, 500, INFINITE, NULL);

#if WIFI_ENABLE_SOFTAP
    wifi_start(STATIONAP_MODE, &params, wifi_connect_cb);
#else
    wifi_start(STATION_MODE, &params, wifi_connect_cb);
#endif
	webadmin_start(&params);

    INFO("System started ...\r\n");
}


void ICACHE_FLASH_ATTR user_pre_init(void)
{
    if(!system_partition_table_regist(at_partition_table, 
				sizeof(at_partition_table)/sizeof(at_partition_table[0]),
				SPI_FLASH_SIZE_MAP)) {
		FATAL("system_partition_table_regist fail\r\n");
		while(1);
	}
}

