#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

#ifdef DBUS
#include <gio/gio.h>
#include "dbus-led.h"
#endif

#include "led.h"


/*** Global variables ***/
key_t led_key;
int led_msgq_id;
pthread_t thread_led;
pthread_t thread_led_internal;
pthread_mutex_t led_lock;
int led_next_action = 0;
int led_on[2][30], led_p = 0;
int led_brightness = 0;
int led_temperature = 5;
int led_center = 16;
int led_layer = 6;

#ifdef DBUS
static GestureInterface *proxy = NULL;
#endif



/*** Private methods ***/

void led_change_brightness(int action, int brightness, int temperature) {
    FILE *fbrightness = fopen(LED_INTERFACE, "w");
    const int steps = 16;
    char led[122], single[5];

    for (int i=1; i<=steps; i++) {
        led[0] = 0;
        if (action == LED_ACTION_UP || action == LED_ACTION_DOWN) {
            float diff = (brightness - led_brightness) / (float)steps;
            int warm = (int) ((led_brightness + diff * i) * 16 / 10.0f * (10 - led_temperature));
            int cold = (int) ((led_brightness + diff * i) * 16 / 10.0f * led_temperature);
            sprintf(single, "%02x%02x", warm, cold);
            for (int j = 0; j < 30; j++) {
                if (led_on[led_p][j]) strcat(led, single);
                else strcat(led, "0000");
            }
        } else if (action == LED_ACTION_LEFT || action == LED_ACTION_RIGHT) {
            float diff = (temperature - led_temperature) / (float)steps;
            int warm = (int) (led_brightness * 16 / 10.0f * (10 - led_temperature - diff * i));
            int cold = (int) (led_brightness * 16 / 10.0f * (led_temperature + diff * i));
            sprintf(single, "%02x%02x", warm, cold);
            for (int j = 0; j < 30; j++) {
                if (led_on[led_p][j]) strcat(led, single);
                else strcat(led, "0000");
            }
        } else {
            float diff = led_brightness / (float)steps;
            int warm = (int) (led_brightness * 16 / 10.0f * (10 - led_temperature));
            int cold = (int) (led_brightness * 16 / 10.0f * led_temperature);
            int warmUp = (int) ((diff * i) * 16 / 10.0f * (10 - led_temperature));
            int coldUp = (int) ((diff * i) * 16 / 10.0f * led_temperature);
            int warmDown = (int) ((led_brightness - diff * i) * 16 / 10.0f * (10 - led_temperature));
            int coldDown = (int) ((led_brightness - diff * i) * 16 / 10.0f * led_temperature);
            for (int j = 0; j < 30; j++) {
                if (led_on[led_p][j] && led_on[1-led_p][j]) {
                    sprintf(single, "%02x%02x", warm, cold);
                    strcat(led, single);
                } else if (led_on[led_p][j] && !led_on[1-led_p][j]) {
                    sprintf(single, "%02x%02x", warmUp, coldUp);
                    strcat(led, single);
                } else if (!led_on[led_p][j] && led_on[1-led_p][j]) {
                    sprintf(single, "%02x%02x", warmDown, coldDown);
                    strcat(led, single);
                } else strcat(led, "0000");
            }
        }
        strcat(led, "\n");
        fseek(fbrightness, 0L, SEEK_SET);
        fputs(led, fbrightness);
        fflush(fbrightness);
        usleep(LED_CHANGE_DURATION / steps);
    }
    fclose(fbrightness);
    led_brightness = brightness;
    led_temperature = temperature;
}

void led_move(int next) {
}

void led_spread(void) {
}

void led_focus(void) {
    led_p = 1 - led_p;
    memcpy(led_on[led_p], led_on[1 - led_p], sizeof(led_on[led_p]));
    if (led_layer > 1) {
        for (int i=0; i<30; i++) {
            if (led_on[led_p][i] == led_layer) led_on[led_p][i] = 0;
        }
        led_layer--;
    }
}

void led_action(int action) {
}

void* pthread_led_internal(void *p) {
    while (1) {
        if (led_next_action) {
            int ret = pthread_mutex_lock(&led_lock);
            if (ret) continue;
            led_action(led_next_action);
            led_next_action = 0;
            pthread_mutex_unlock(&led_lock);
        }
        usleep(10);
    }
    return NULL;
}

void* pthread_led(void *pv) {
    int ret;

    while (1) {
    }

    return NULL;
}

#ifdef DBUS
int init_led_dbus(void) {
	GError *error = NULL;
	gboolean ret;

	proxy = gesture_interface_proxy_new_for_bus_sync(
		G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE,
		LED_DBUS_NAME, LED_OBJ_PATH, NULL, &error);

	if (!proxy) {
		printf("%s: get dbus failed: %s\n", __func__, error->message);
		return -1;
	}

	return 0;
}
#endif

int init_led_ball(void) {

    return 0;
}

int init_led_msg(void) {
    char cmd[256];
    sprintf(cmd, "touch %s", LED_FILE);
    system(cmd);
    sprintf(cmd, "chmod 777 %s", LED_FILE);
    system(cmd);

    led_key = ftok(LED_FILE, LED_PROJ_ID);
    if (led_key < 0) {
        printf("%s: ftok failed: %d(%d)\n", __func__, led_key, errno);
        return led_key;
    }

    led_msgq_id = msgget(led_key, 0666 | IPC_CREAT);
    if (led_msgq_id < 0) {
        printf("%s: msgget failed: %d(%d)\n", __func__, led_msgq_id, errno);
        return led_msgq_id;
    }

    return 0;
}

int init_led_thread(void) {
    int ret = pthread_mutex_init(&led_lock, NULL);
    if (ret) {
        printf("%s: init led mutex failed: %d\n", __func__, ret);
        return ret;
    }

    ret = pthread_create(&thread_led, NULL, pthread_led, NULL);
    if (ret) {
        printf("%s: create pthread led failed: %d\n", __func__, ret);
        return ret;
    }

        /*
    ret = pthread_create(&thread_led_internal, NULL, pthread_led_internal, NULL);
    if (ret) {
        printf("%s: create pthread led internal failed: %d\n", __func__, ret);
        return ret;
    }
    */

    return 0;
}



/*** Public methods ***/

int init_led(void) {
    int ret = 0;

#ifdef DBUS
	ret = init_led_dbus();
	if (ret) {
		printf("%s: init dbus failed: %d\n", __func__, ret);
		return ret;
	}

#else

    ret = init_led_ball();
    if (ret) {
        printf("%s: init ball failed: %d\n", __func__, ret);
        return ret;
    }

    ret = init_led_msg();
    if (ret) {
        printf("%s: init message failed: %d\n", __func__, ret);
        return ret;
    }

    ret = init_led_thread();
    if (ret) {
        printf("%s: init thread failed: %d\n", __func__, ret);
        return ret;
    }
#endif

    return 0;
}

int led_ctl(int action) {


    return 0;
}
