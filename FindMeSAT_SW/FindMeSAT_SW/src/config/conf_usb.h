/**
 * \file
 *
 * \brief USB configuration file for CDC application
 *
 * Copyright (c) 2009-2015 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */

#ifndef _CONF_USB_H_
#define _CONF_USB_H_

#include "compiler.h"
#include "usb_protocol_cdc.h"


/**
 * USB Device Configuration
 * @{
 */

#define  USB_PID_ATMEL_FINDMESAT          0x2701


//! Device definition (mandatory)
#define  USB_DEVICE_VENDOR_ID             USB_VID_ATMEL
#define  USB_DEVICE_PRODUCT_ID            USB_PID_ATMEL_FINDMESAT
#define  USB_DEVICE_MAJOR_VERSION         1
#define  USB_DEVICE_MINOR_VERSION         0
#define  USB_DEVICE_POWER                 40  // Consumption on Vbus line (mA)
#define  USB_DEVICE_ATTR                  \
(USB_CONFIG_ATTR_REMOTE_WAKEUP|USB_CONFIG_ATTR_BUS_POWERED)
// (USB_CONFIG_ATTR_BUS_POWERED)
// (USB_CONFIG_ATTR_SELF_POWERED)
// (USB_CONFIG_ATTR_REMOTE_WAKEUP|USB_CONFIG_ATTR_SELF_POWERED)

//! USB Device string definitions (Optional)
#define  USB_DEVICE_MANUFACTURE_NAME      "DF4IAH Solutions"
#define  USB_DEVICE_PRODUCT_NAME          "FindMeSAT V1"
#define  USB_DEVICE_SERIAL_NAME           "0"


// By default USB interrupt have low priority
#define UDD_USB_INT_LEVEL USB_INTLVL_MED_gc



/**
 * Device speeds support
 * Low speed not supported by CDC
 * @{
 */
//! To authorize the High speed
#if (UC3A3||UC3A4)
#define  USB_DEVICE_HS_SUPPORT
#endif
//@}


/**
 * USB Device Callbacks definitions (Optional)
 * @{
 */
// #define  UDC_VBUS_EVENT(b_vbus_high)      user_callback_vbus_action(b_vbus_high)
// extern void user_callback_vbus_action(bool b_vbus_high);
// #define  UDC_SOF_EVENT()                  user_callback_sof_action()
// extern void user_callback_sof_action(void);
#define  UDC_SUSPEND_EVENT()              usb_callback_suspend_action()
extern void usb_callback_suspend_action(void);
#define  UDC_RESUME_EVENT()               usb_callback_resume_action()
extern void usb_callback_resume_action(void);
// Mandatory when USB_DEVICE_ATTR authorizes remote wakeup feature
#define  UDC_REMOTEWAKEUP_ENABLE()        usb_callback_remotewakeup_enable()
extern void usb_callback_remotewakeup_enable(void);
#define  UDC_REMOTEWAKEUP_DISABLE()       usb_callback_remotewakeup_disable()
extern void usb_callback_remotewakeup_disable(void);
// When an extra string descriptor must be supported
// other than manufacturer, product and serial string
// #define  UDC_GET_EXTRA_STRING()
//@}

//@}


/**
 * USB Interface Configuration
 * @{
 */
/**
 * Configuration of CDC interface
 * @{
 */

//! Number of communication port used (1 to 3)
#define  UDI_CDC_PORT_NB 1

//! Interface callback definition
//#define  UDI_CDC_ENABLE_EXT(port)          true
//#define  UDI_CDC_DISABLE_EXT(port)
//#define  UDI_CDC_RX_NOTIFY(port)
//#define  UDI_CDC_TX_EMPTY_NOTIFY(port)
//#define  UDI_CDC_SET_CODING_EXT(port,cfg)
//#define  UDI_CDC_SET_DTR_EXT(port,set)
//#define  UDI_CDC_SET_RTS_EXT(port,set)

#define UDI_CDC_ENABLE_EXT(port) usb_callback_cdc_enable()
extern bool usb_callback_cdc_enable(void);
#define UDI_CDC_DISABLE_EXT(port) usb_callback_cdc_disable()
extern void usb_callback_cdc_disable(void);
#define  UDI_CDC_RX_NOTIFY(port) usb_callback_rx_notify(port)
extern void usb_callback_rx_notify(uint8_t port);
#define  UDI_CDC_TX_EMPTY_NOTIFY(port) usb_callback_tx_empty_notify(port)
extern void usb_callback_tx_empty_notify(uint8_t port);
#define  UDI_CDC_SET_CODING_EXT(port,cfg) usb_callback_config(port,cfg)
extern void usb_callback_config(uint8_t port, usb_cdc_line_coding_t * cfg);
#define  UDI_CDC_SET_DTR_EXT(port,set) usb_callback_cdc_set_dtr(port,set)
extern void usb_callback_cdc_set_dtr(uint8_t port, bool b_enable);
#define  UDI_CDC_SET_RTS_EXT(port,set) usb_callback_cdc_set_rts(port,set)
extern void usb_callback_cdc_set_rts(uint8_t port, bool b_enable);

//! Define it when the transfer CDC Device to Host is a low rate (<512000 bauds)
//! to reduce CDC buffers size
//#define  UDI_CDC_LOW_RATE

//! Default configuration of communication port
#define  UDI_CDC_DEFAULT_RATE             115200
#define  UDI_CDC_DEFAULT_STOPBITS         CDC_STOP_BITS_1
#define  UDI_CDC_DEFAULT_PARITY           CDC_PAR_NONE
#define  UDI_CDC_DEFAULT_DATABITS         8

#include "udi_cdc_conf.h"

//@}
//@}


/**
 * USB Device Driver Configuration
 * @{
 */
//@}

//! The includes of classes and other headers must be done at the end of this file to avoid compile error
#include "udi_cdc_conf.h"

#endif // _CONF_USB_H_