#ifndef STDBULKUSB_H
#define STDBULKUSB_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void (* onRx)(char *buffer, int length);

void UsbInit(onRx onRxCallback);
void UsbPoll(void);

void UsbWrite(char *buffer, int length);

#ifdef __cplusplus
}
#endif

#endif
