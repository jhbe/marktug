#include <stm32f4xx.h>
#include <usb.h>
#include "stdbulkusb.h"
//#include "delay.h"
//#include "led.h"

static usbd_device usbdDevice;

#define EP1_OUT 0x01
#define EP1_IN  0x81

#define EP0_SIZE 64
static uint32_t ep0Buffer[EP0_SIZE];

#define EP1_SIZE 64
//static uint32_t ep1Buffer[EP1_SIZE];

static onRx s_onRx;

/**
 * The OTG1_FS interrupt handler.
 */
void OTG_FS_IRQHandler(void) {
    usbd_poll(&usbdDevice);
}

/**
 * The USB device descriptor sent back when receiving a GET_DESCRIPTOR of type DEVICE.
 */
static struct usb_device_descriptor deviceDescriptor = {
        .bLength            = sizeof(struct usb_device_descriptor),
        .bDescriptorType    = USB_DTYPE_DEVICE,
        .bcdUSB             = VERSION_BCD(1, 1, 0),
        .bDeviceClass       = USB_CLASS_PER_INTERFACE,
        .bDeviceSubClass    = USB_SUBCLASS_NONE,
        .bDeviceProtocol    = USB_PROTO_NONE,
        .bMaxPacketSize0    = EP0_SIZE,
        .idVendor           = 0x3456, // Arbitrary number, hopefully not already taken.
        .idProduct          = 0x1150, // Arbitrary number.
        .bcdDevice          = VERSION_BCD(1, 0, 0),
        .iManufacturer      = 1,
        .iProduct           = 2,
        .iSerialNumber      = NO_DESCRIPTOR,
        .bNumConfigurations = 1,
};

/**
 * The USB configuration block sent back when receiving a GET_DESCRIPTOR of type CONFIGURATION.
 */
struct std_config {
    struct usb_config_descriptor config;
    struct usb_interface_descriptor interface;
//    struct usb_endpoint_descriptor ep0out;
    struct usb_endpoint_descriptor ep0in;
};

/**
 * Initalize the configuration block. It says that the USB interface has one interface with two BULK endpoints,
 * one OUT and one IN. It also says that the device is not self powered and will require 100mA of power.
 *
 * It states that this interface does not implement a predefined protocol, such as HID or CDC.
 */
static struct std_config config_desc = {
        .config = {
                .bLength                = sizeof(struct usb_config_descriptor),
                .bDescriptorType        = USB_DTYPE_CONFIGURATION,
                .wTotalLength           = sizeof(struct std_config),
                .bNumInterfaces         = 1,
                .bConfigurationValue    = 1,
                .iConfiguration         = NO_DESCRIPTOR,
                .bmAttributes           = USB_CFG_ATTR_RESERVED,
                .bMaxPower              = USB_CFG_POWER_MA(100),
        },
        .interface = {
                .bLength                = sizeof(struct usb_interface_descriptor),
                .bDescriptorType        = USB_DTYPE_INTERFACE,
                .bInterfaceNumber       = 0,
                .bAlternateSetting      = 0,
                .bNumEndpoints          = 1, //2,
                .bInterfaceClass        = USB_CLASS_VENDOR,
                .bInterfaceSubClass     = USB_SUBCLASS_VENDOR,
                .bInterfaceProtocol     = USB_PROTO_VENDOR,
                .iInterface             = NO_DESCRIPTOR,
        },
/*        .ep0out = {
                .bLength                = sizeof(struct usb_endpoint_descriptor),
                .bDescriptorType        = USB_DTYPE_ENDPOINT,
                .bEndpointAddress       = EP1_OUT,
                .bmAttributes           = USB_EPTYPE_BULK,
                .wMaxPacketSize         = EP1_SIZE,
                .bInterval              = 0x01,
        },
*/        .ep0in = {
                .bLength                = sizeof(struct usb_endpoint_descriptor),
                .bDescriptorType        = USB_DTYPE_ENDPOINT,
                .bEndpointAddress       = EP1_IN,
                .bmAttributes           = USB_EPTYPE_BULK,
                .wMaxPacketSize         = EP1_SIZE,
                .bInterval              = 0x01,
        }
};

/**
 * Define language, manufacturer and product strings.
 */
static struct usb_string_descriptor langDesc = USB_ARRAY_DESC(USB_LANGID_ENG_US);
static struct usb_string_descriptor manufDesc = USB_STRING_DESC("Johan");
static struct usb_string_descriptor prodDesc = USB_STRING_DESC("F405 UsbTest");

static struct usb_string_descriptor *const stringTable[] = {
        &langDesc,
        &manufDesc,
        &prodDesc,
};

/**
 * Invoked when an event has occured on endpoint EP1.
 *
 * @param dev The device.
 * @param event The event. One of usbd_evt_*.
 * @param ep The endpoint.
 */
void onEp1(usbd_device *dev, uint8_t event, uint8_t ep) {
    switch (event) {
        case usbd_evt_eprx: {
            /*
             * Read what arrived on the OUT endpoint. Remember, IN/OUT is always relative
             * the host, not the device.
             */
            char rxBuffer[64];
            int length = usbd_ep_read(dev, EP1_OUT, rxBuffer, 64);

            if (s_onRx != 0) {
                s_onRx(rxBuffer, length);
            }
            break;
        }
        case usbd_evt_eptx:
            break;

        case usbd_evt_epsetup:
            break;

        default: {
            break;
        }
    }
}

/**
 * Invoked by the libstm32_usb framework when a SET_CONFIG request was received.
 *
 * @param dev The device.
 * @param cfg 0 when deconfiguring the device, 1 when configuring the device.
 * @return usbd_ack on success, usbd_fail otherwise.
 */
usbd_respond onConfig(usbd_device *dev, uint8_t cfg) {
    switch (cfg) {
        case 0:
            /*
             * Deconfiguring device, going from CONFIGURED to ADDRESSED
             */

            usbd_ep_deconfig(dev, EP1_OUT);
            usbd_ep_deconfig(dev, EP1_IN);
           // usbd_reg_endpoint(dev, EP1, 0);
            break;

        case 1:
            /*
             * Configuring device, going from ADDRESSED to CONFIGURED
             */

            /*
             * Register a callback for the endpoint. Note that there is one callback per endpoint; there are no separate
             * IN and OUT callbacks for the same endpoint.
             */
            //usbd_reg_endpoint(dev, EP1, onEp1);
            usbd_ep_config(dev, EP1_OUT, USB_EPTYPE_BULK, EP1_SIZE);
            usbd_ep_config(dev, EP1_IN, USB_EPTYPE_BULK, EP1_SIZE);
            break;
        default:
            return usbd_fail;
    }
    return usbd_ack;
}

/**
 * Invoked by the libstm32_usb framework when a GET_DESCRIPTOR request was received.
 *
 * @param req The USB request.
 * @param address Pointer to a pointer to be set to point to the descriptor.
 * @param length Pointer to an int to be assigned the length of the descriptor.
 * @return usbd_ack on success, usbd_fail otherwise.
 */
usbd_respond onDescriptor(usbd_ctlreq *req, void **address, uint16_t *length) {
    const uint8_t type = req->wValue >> 8;
    const uint8_t number = req->wValue & 0xFF;

    switch (type) {
        case USB_DTYPE_DEVICE:
            *address = &deviceDescriptor;
            *length = sizeof(deviceDescriptor);
            break;

        case USB_DTYPE_CONFIGURATION:
            *address = &config_desc;
            *length = sizeof(config_desc);
            break;

        case USB_DTYPE_STRING:
            if (number < 3) {
                *address = stringTable[number];
                *length = ((struct usb_header_descriptor *) stringTable[number])->bLength;
            } else {
                return usbd_fail;
            }
            break;
    }

    return usbd_ack;
}

void UsbInit(onRx onRxCallback) {
    s_onRx = onRxCallback;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_OTG_FS, ENABLE);

    /*
     * Configure the SOF (PA_8), D- (PA_11) and D+ (PA_12) USB pins to use alternative function OTG1.
     */
    GPIO_InitTypeDef gis;
    gis.GPIO_Pin = /*GPIO_Pin_8 | */GPIO_Pin_11 | GPIO_Pin_12;
    gis.GPIO_Speed = GPIO_Speed_100MHz;
    gis.GPIO_Mode = GPIO_Mode_AF;
    gis.GPIO_OType = GPIO_OType_PP;
    gis.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &gis);

//    GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_OTG1_FS);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource11, GPIO_AF_OTG1_FS);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource12, GPIO_AF_OTG1_FS);

    /*
     * Configure the VBUS (PA_9) as an analog input.
     */
    gis.GPIO_Pin = GPIO_Pin_9;
    gis.GPIO_Speed = GPIO_Speed_100MHz;
    gis.GPIO_Mode = GPIO_Mode_IN;
    gis.GPIO_OType = GPIO_OType_OD;
    gis.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &gis);

    /*
     * Configure the ID (PA_10) to alternative function OTG1.
     */
    gis.GPIO_Pin = GPIO_Pin_10;
    gis.GPIO_Speed = GPIO_Speed_100MHz;
    gis.GPIO_OType = GPIO_OType_OD;
    gis.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &gis);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_OTG1_FS);

    /*
     * Enable the OTG_FS_IRQn interrupt.
     */
    NVIC_InitTypeDef nis;
    nis.NVIC_IRQChannel = OTG_FS_IRQn;
    nis.NVIC_IRQChannelPreemptionPriority = 1;
    nis.NVIC_IRQChannelSubPriority = 3;
    nis.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nis);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

    /*
     * Initialize the libstm32_usb library.
     */
    usbd_init(&usbdDevice, &usbd_hw, EP0_SIZE, ep0Buffer, EP0_SIZE);
    usbd_reg_config(&usbdDevice, onConfig);
    usbd_reg_descr(&usbdDevice, onDescriptor);
    usbd_enable(&usbdDevice, true);
    usbd_connect(&usbdDevice, true);
}

void UsbWrite(char *buffer, int length) {
    usbd_ep_write(&usbdDevice, EP1_IN, buffer, length);
}

void UsbPoll(void) {
    usbd_poll(&usbdDevice);
}
