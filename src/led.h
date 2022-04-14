#ifndef LED_H
#define LED_H

#ifdef __cplusplus
extern "C" {
#endif

void LedInit(void);

void LedRedOn(void);
void LedRedOff(void);
void LedRedToggle(void);

void LedBlueOn(void);
void LedBlueOff(void);
void LedBlueToggle(void);

#ifdef __cplusplus
}
#endif

#endif
