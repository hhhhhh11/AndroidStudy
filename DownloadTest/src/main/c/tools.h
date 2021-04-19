#ifndef TOOLS_H_
#define TOOLS_H_

#include "NDK.h"
#include "rsa.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>

void DrawTitles(uint x, uint y, uint w, uint h, const char * s, int invert);
int DrawList(const char * title, char * lists[], int nr, int start);
void DeviceSetup(void);
int DownloadEntry(void);
int firmware_load(const char * name, void **api);
int firm_rollback(void);
extern uint lcd_w,lcd_h, font_w, font_h;
extern uint8_t glanguage;
extern uint8_t main_icons[4][32];
extern uint8_t bat_icons[4][16];
extern uint8_t bat_var[4];
extern uint8_t arrow_down[8];
extern uint8_t arrow_up[8];
extern uint8_t arrow_left[8];
extern const char *main_titles[4][2];
extern const char * devmana_titles[2];
extern const char * devmana_lists[2][3];
extern const char * app_prompt[2][2];
extern const char * dwn_prompt[2];
extern const char * pwd_prompt[2];
extern const char * firm_prompt[2][2];
extern const char * const file_prompt[1][2];
extern const char * mana_titles[2];
extern const char * mana_lists[2][7];
extern const char * datetime_str[][2];
extern const char * changedate_str[2][2];
extern const char * changetime_str[2][2];
extern const char * lang_str[4];
extern const char * switch_string[2][2];
extern const char * errs_prompt[2];
extern const char * support_prompt[2];
extern const char * bt_titles[2];
extern const char * bt_lists[2][4];
extern const char * bt_prompt[5][2];
extern const char * all_prompt[2][2];
extern const char * rfid_types[2];
extern const char * sec_prompt[2][2];
extern const char * sec_error[6];
extern param_env_t master_envs;
extern const char * const datetime_titles[2];
extern const char * const datetime_lists[2][2];
extern const char * pin_mana_lists[2][6];

int NDK_Init(void);

#endif /* TOOLS_H_ */
