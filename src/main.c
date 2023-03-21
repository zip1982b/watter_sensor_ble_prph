#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <dk_buttons_and_leds.h>
#include "remote.h"
#include "battery_voltage.h"



#define LOG_MODULE_NAME app
LOG_MODULE_REGISTER(LOG_MODULE_NAME);
#define RUN_STATUS_LED DK_LED1
#define CONN_STATUS_LED DK_LED2
#define RUN_LED_BLINK_INTERVAL 1000

static struct bt_conn *current_conn;


/*Declarations*/
void on_connected(struct bt_conn *conn, uint8_t err);
void on_disconnected(struct bt_conn *conn, uint8_t reason);
void on_notif_changed(enum bt_button_notifications_enabled status);
void on_data_received(struct bt_conn *conn, const uint8_t *const data, uint16_t len);

struct bt_conn_cb bluetooth_callbacks = {
	.connected = on_connected,
	.disconnected = on_disconnected
};

struct bt_remote_srv_cb remote_service_callbacks = {
	.notif_changed = on_notif_changed,
	.data_received = on_data_received,
};

/*Callbacks*/

void on_data_received(struct bt_conn *conn, const uint8_t *const data, uint16_t len){
	uint8_t temp_str[len+1];
	memcpy(temp_str, data, len);
	temp_str[len] = 0;

	LOG_INF("Received data on conn %p, len %d", (void*)conn, len);
	LOG_INF("Data: %s", temp_str);
}


void on_connected(struct bt_conn *conn, uint8_t err){
	if(err){
		LOG_ERR("connection error: %d", err);
		return;
	}
	LOG_INF("connected.");
	current_conn = bt_conn_ref(conn);
	dk_set_led_on(CONN_STATUS_LED);
}

void on_disconnected(struct bt_conn *conn, uint8_t reason){
	LOG_INF("Disconnected reason: %d", reason);
	dk_set_led_off(CONN_STATUS_LED);
	if(current_conn){
		bt_conn_unref(current_conn);
		current_conn = NULL;
	}
}



void on_notif_changed(enum bt_button_notifications_enabled status){
	if(status == BT_BUTTON_NOTIFICATIONS_ENABLED){
		LOG_INF("Notification enabled");
	}
	else LOG_INF("Notification disabled");
}

void button_handler(uint32_t button_state, uint32_t has_changed){
	int button_pressed = 0;
	int err;
	if(has_changed & button_state){
		switch(has_changed){
			case DK_BTN1_MSK:
				button_pressed = 1;
				break;
			case DK_BTN2_MSK:
				button_pressed = 2;
				break;
			case DK_BTN3_MSK:
				button_pressed = 3;
				break;
			case DK_BTN4_MSK:
				button_pressed = 4;
				break;
			default:
				break;
		}
		LOG_INF("button %d pressed", button_pressed);
		set_button_value(button_pressed);
		err = send_button_notification(current_conn, button_pressed, 1);
		if(err){
			LOG_ERR("couldn`t send notification (err:%d)", err);
		}
	}
}

static void configure_dk_buttons_leds(void){
	int err;
	err = dk_buttons_init(button_handler);
	if(err){
		LOG_ERR("Cannot init buttons (err:%d)", err);
	}
	err = dk_leds_init();
	if(err){
		LOG_ERR("Cannot init leds (err:%d)", err);
	}
}



void main(void)
{
	int err;
	int blink_interval = 0;
	LOG_INF("My watter sensor board - %s\n", CONFIG_BOARD);

	configure_dk_buttons_leds();
	configure_adc_device();

	err = bluetooth_init(&bluetooth_callbacks, &remote_service_callbacks);
	if (err){
		LOG_ERR("bluetooth_init() required %d", err);
	}
	LOG_INF("Running...");


	while(true){
		dk_set_led(RUN_STATUS_LED, (blink_interval++)%2);
		k_sleep(K_MSEC(RUN_LED_BLINK_INTERVAL));
		/* Battery level simulation */
		bas_notify();
	}
}