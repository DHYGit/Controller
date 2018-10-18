#ifndef __LED_H_
#define __LED_H_

#define LED_DBUS_NAME	"com.pi.gestureInterface"
#define LED_OBJ_PATH	"/com/pi/gestureInterface"

#define LED_INTERFACE       "/sys/kernel/leds-pca9635/brightness"
#define LED_CHANGE_DURATION 50 * 1000.0f

#define LED_FILE        "/home/nvidia/led"
#define LED_PROJ_ID     1
#define LED_MSG_TYPE    1

enum LED_ACTION {
	LED_ACTION_UP = 1,              // brightness +
	LED_ACTION_DOWN,                // brightness -
	LED_ACTION_RIGHT,               // warm
	LED_ACTION_LEFT,                // cold
	LED_ACTION_CLOCKWISE,           // move right
	LED_ACTION_CONTER_CLOCKWISE,    // move left
	LED_ACTION_OPEN,                // light up 1 layer
	LED_ACTION_CLOSE,               // close 1 layer
	LED_ACTION_ACTIVE,		// lock person
	LED_ACTION_EXIT,		// unlock person
};

extern int led_msgq_id;

int init_led(void);
int led_ctl(int action);

#endif
