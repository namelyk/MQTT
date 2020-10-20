#ifndef _SYSTEM_SETTING_H_
#define _SYSTEM_SETTING_H_


#define SYSTEM_SETTING_ENABLE
#ifdef SYSTEM_SETTING_ENABLE



extern int system_setting_reset_config(unsigned char *buffer);
extern int system_setting_set_position(unsigned char *buffer);
extern int system_setting_get_program(unsigned char *buffer,unsigned char size);
extern int system_setting_set_program(unsigned char *buffer,unsigned char size);
extern int Restore_factory_settings(void);
extern void reset_restore_timeout(void);

#endif
#endif



