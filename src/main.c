/*
 * Copyright (c) 2016-2018 Intel Corporation.
 * Copyright (c) 2018-2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 * 
 * 
 * Modified by Tristan Richmond
 * Copyright (c) 2022 Rich Effects
 * 
 * HID Usage Tables
 * https://usb.org/sites/default/files/hut1_3_0.pdf
 */

#include <zephyr/zephyr.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/class/usb_hid.h>

#define LOG_LEVEL LOG_LEVEL_INF
LOG_MODULE_REGISTER(main);

/*
 * Devicetree node identifiers for the buttons and LEDs
 */
#define SW0_NODE DT_ALIAS(sw0)
#define SW1_NODE DT_ALIAS(sw1)
#define SW2_NODE DT_ALIAS(sw2)
#define SW3_NODE DT_ALIAS(sw3)
#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)
#define LED3_NODE DT_ALIAS(led3)

/*
 * Helper macro for initializing a gpio_dt_spec from the devicetree
 * with fallback values when the nodes are missing.
 */
#define GPIO_SPEC(node_id) GPIO_DT_SPEC_GET_OR(node_id, gpios, {0})


/*
 * Macro for byte position in HID report packet
*/
#define KEYPAD_BTN_MODIFIER_REPORT_POS 0
#define KEYPAD_BTN_LED_REPORT_POS 2
#define KEYPAD_BTN_CODE_REPORT_POS 7

/*
 * Create gpio_dt_spec structures from the devicetree.
 */
static const struct gpio_dt_spec 
	sw0 = GPIO_SPEC(SW0_NODE),
	sw1 = GPIO_SPEC(SW1_NODE),
	sw2 = GPIO_SPEC(SW2_NODE),
	sw3 = GPIO_SPEC(SW3_NODE),
	led0 = GPIO_SPEC(LED0_NODE),
	led1 = GPIO_SPEC(LED1_NODE),
	led2 = GPIO_SPEC(LED2_NODE),
	led3 = GPIO_SPEC(LED3_NODE);

static const uint8_t hid_report_desc[] = HID_KEYBOARD_REPORT_DESC();

static uint8_t def_val[4];
static volatile uint8_t status[8];
static K_SEM_DEFINE(sem, 0, 1); /* starts off "not available" */
static struct gpio_callback callback[4];
static enum usb_dc_status_code usb_status;

static void status_cb(enum usb_dc_status_code status, const uint8_t *param)
{
	usb_status = status;
}

static void one_button(const struct device *gpio, struct gpio_callback *cb,
			uint32_t pins)
{
	int ret;
	uint8_t state = status[KEYPAD_BTN_CODE_REPORT_POS];

	if (IS_ENABLED(CONFIG_USB_DEVICE_REMOTE_WAKEUP)) {
		if (usb_status == USB_DC_SUSPEND) {
			usb_wakeup_request();
			return;
		}
	}

	ret = gpio_pin_get(gpio, sw0.pin);
	if (ret < 0) {
		LOG_ERR("Failed to get the state of port %s pin %u, error: %d",
			gpio->name, sw0.pin, ret);
		return;
	}

	if (def_val[0] != (uint8_t)ret) {
		state = HID_KEY_R;
	} else {
		state = HID_KBD_MODIFIER_NONE;
	}

	if (status[KEYPAD_BTN_CODE_REPORT_POS] != state) {
		status[KEYPAD_BTN_CODE_REPORT_POS] = state;
		k_sem_give(&sem);
	}
}

static void two_button(const struct device *gpio, struct gpio_callback *cb,
			 uint32_t pins)
{
	int ret;
	uint8_t state = status[KEYPAD_BTN_CODE_REPORT_POS];

	if (IS_ENABLED(CONFIG_USB_DEVICE_REMOTE_WAKEUP)) {
		if (usb_status == USB_DC_SUSPEND) {
			usb_wakeup_request();
			return;
		}
	}

	ret = gpio_pin_get(gpio, sw1.pin);
	if (ret < 0) {
		LOG_ERR("Failed to get the state of port %s pin %u, error: %d",
			gpio->name, sw1.pin, ret);
		return;
	}

	if (def_val[1] != (uint8_t)ret) {
		state = HID_KEY_I;
	} else {
		state = HID_KBD_MODIFIER_NONE;
	}

	if (status[KEYPAD_BTN_CODE_REPORT_POS] != state) {
		status[KEYPAD_BTN_CODE_REPORT_POS] = state;
		k_sem_give(&sem);
	}
}

static void three_button(const struct device *gpio, struct gpio_callback *cb,
			 uint32_t pins)
{
	int ret;
	uint8_t state = status[KEYPAD_BTN_CODE_REPORT_POS];

	if (IS_ENABLED(CONFIG_USB_DEVICE_REMOTE_WAKEUP)) {
		if (usb_status == USB_DC_SUSPEND) {
			usb_wakeup_request();
			return;
		}
	}

	ret = gpio_pin_get(gpio, sw2.pin);
	if (ret < 0) {
		LOG_ERR("Failed to get the state of port %s pin %u, error: %d",
			gpio->name, sw2.pin, ret);
		return;
	}

	if (def_val[2] != (uint8_t)ret) {
		state = HID_KEY_C;
	} else {
		state = HID_KBD_MODIFIER_NONE;
	}

	if (status[KEYPAD_BTN_CODE_REPORT_POS] != state) {
		status[KEYPAD_BTN_CODE_REPORT_POS] = state;
		k_sem_give(&sem);
	}
}

static void four_button(const struct device *gpio, struct gpio_callback *cb,
			 uint32_t pins)
{
	int ret;
	uint8_t state = status[KEYPAD_BTN_CODE_REPORT_POS];

	if (IS_ENABLED(CONFIG_USB_DEVICE_REMOTE_WAKEUP)) {
		if (usb_status == USB_DC_SUSPEND) {
			usb_wakeup_request();
			return;
		}
	}

	ret = gpio_pin_get(gpio, sw3.pin);
	if (ret < 0) {
		LOG_ERR("Failed to get the state of port %s pin %u, error: %d",
			gpio->name, sw3.pin, ret);
		return;
	}

	if (def_val[3] != (uint8_t)ret) {
		state = HID_KEY_H;
	} else {
		state = HID_KBD_MODIFIER_NONE;
	}

	if (status[KEYPAD_BTN_CODE_REPORT_POS] != state) {
		status[KEYPAD_BTN_CODE_REPORT_POS] = state;
		k_sem_give(&sem);
	}
}

int callbacks_configure(const struct gpio_dt_spec *spec,
			gpio_callback_handler_t handler,
			struct gpio_callback *callback, uint8_t *val)
{
	const struct device *gpio = spec->port;
	gpio_pin_t pin = spec->pin;
	int ret;

	if (gpio == NULL) {
		/* Optional GPIO is missing. */
		return 0;
	}

	if (!device_is_ready(gpio)) {
		LOG_ERR("GPIO port %s is not ready", gpio->name);
		return -ENODEV;
	}

	ret = gpio_pin_configure_dt(spec, GPIO_INPUT);
	if (ret < 0) {
		LOG_ERR("Failed to configure port %s pin %u, error: %d",
			gpio->name, pin, ret);
		return ret;
	}

	ret = gpio_pin_get(gpio, pin);
	if (ret < 0) {
		LOG_ERR("Failed to get the state of port %s pin %u, error: %d",
			gpio->name, pin, ret);
		return ret;
	}

	*val = (uint8_t)ret;

	gpio_init_callback(callback, handler, BIT(pin));
	ret = gpio_add_callback(gpio, callback);
	if (ret < 0) {
		LOG_ERR("Failed to add the callback for port %s pin %u, "
			"error: %d",
			gpio->name, pin, ret);
		return ret;
	}

	ret = gpio_pin_interrupt_configure_dt(spec, GPIO_INT_EDGE_BOTH);
	if (ret < 0) {
		LOG_ERR("Failed to configure interrupt for port %s pin %u, "
			"error: %d",
			gpio->name, pin, ret);
		return ret;
	}

	return 0;
}

void main(void)
{
	LOG_INF("Starting application");

	int ret;
	uint8_t report[8] = { 0x00 };
	const struct device *hid_dev;

	if (!device_is_ready(led0.port)) {
		LOG_ERR("LED device %s is not ready", led0.port->name);
		return;
	}
	if (!device_is_ready(led1.port)) {
		LOG_ERR("LED device %s is not ready", led1.port->name);
		return;
	}
	if (!device_is_ready(led2.port)) {
		LOG_ERR("LED device %s is not ready", led2.port->name);
		return;
	}
	if (!device_is_ready(led3.port)) {
		LOG_ERR("LED device %s is not ready", led3.port->name);
		return;
	}

	hid_dev = device_get_binding("HID_0");
	if (hid_dev == NULL) {
		LOG_ERR("Cannot get USB HID Device");
		return;
	}

	ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT);
	if (ret < 0) {
		LOG_ERR("Failed to configure the LED pin, error: %d", ret);
		return;
	}
	ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT);
	if (ret < 0) {
		LOG_ERR("Failed to configure the LED pin, error: %d", ret);
		return;
	}
	ret = gpio_pin_configure_dt(&led2, GPIO_OUTPUT);
	if (ret < 0) {
		LOG_ERR("Failed to configure the LED pin, error: %d", ret);
		return;
	}
	ret = gpio_pin_configure_dt(&led3, GPIO_OUTPUT);
	if (ret < 0) {
		LOG_ERR("Failed to configure the LED pin, error: %d", ret);
		return;
	}

	if (callbacks_configure(&sw0, &one_button, &callback[0], &def_val[0])) {
		LOG_ERR("Failed configuring one button callback.");
		return;
	}
	if (callbacks_configure(&sw1, &two_button, &callback[1], &def_val[1])) {
		LOG_ERR("Failed configuring two button callback.");
		return;
	}
	if (callbacks_configure(&sw2, &three_button, &callback[2], &def_val[2])) {
		LOG_ERR("Failed configuring three button callback.");
		return;
	}
	if (callbacks_configure(&sw3, &four_button, &callback[3], &def_val[3])) {
		LOG_ERR("Failed configuring four button callback.");
		return;
	}

	usb_hid_register_device(hid_dev, hid_report_desc, 
				sizeof(hid_report_desc),NULL);
	
	usb_hid_init(hid_dev);

	ret = usb_enable(status_cb);
	if (ret != 0) {
		LOG_ERR("Failed to enable USB");
		return;
	}

	while (true) {
		k_sem_take(&sem, K_FOREVER);

		report[KEYPAD_BTN_CODE_REPORT_POS] = status[KEYPAD_BTN_CODE_REPORT_POS];
		ret = hid_int_ep_write(hid_dev, report, sizeof(report), NULL);
		if (ret) {
			LOG_ERR("HID write error, %d", ret);
		}

		/* Toggle LED on sent report */
		ret = gpio_pin_toggle(led0.port, led0.pin);
		if (ret < 0) {
			LOG_ERR("Failed to toggle the LED pin, error: %d", ret);
		}
	}
}