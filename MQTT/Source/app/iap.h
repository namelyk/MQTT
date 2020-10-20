#ifndef _IAP_H_
#define _IAP_H_


#define IAP_ENABLE
#ifdef IAP_ENABLE

enum{
	LOCAL_UPDATE,
	REMOTE_UPDATE,
	MQTT_UPDATE
};

extern void system_reboot(void);
extern void iap_set_update_source(unsigned char source);
extern unsigned char iap_get_update_source(void);
//如果当前正在重启 不处理任何网络包 2020-07-01
extern char system_reboot_get_enable(void);

extern const struct app_interface_t iap_interface;

#endif
#endif
