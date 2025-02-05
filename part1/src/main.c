#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#define DELAY0_MS    100
#define DELAY1_MS    200
#define DELAY2_MS    300
#define DELAY3_MS    500

#define STACK_SIZE   500 

#define LED0   DT_ALIAS(led0) 
#define LED1   DT_ALIAS(led1) 
#define LED2   DT_ALIAS(led2) 
#define LED3   DT_ALIAS(led3) 

struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0, gpios); 
struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1, gpios); 
struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2, gpios); 
struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(LED3, gpios); 


struct blinky_data {
    struct gpio_dt_spec *led;
    int delay;
};

void blinky_task(void *arg1, void *arg2, void *arg3) 
{
    struct blinky_data *data = (struct blinky_data *)arg1;

    gpio_pin_configure_dt(data->led, GPIO_OUTPUT_ACTIVE); 

    for (;;) {
        gpio_pin_toggle_dt(data->led); 
        k_msleep(data->delay); 
    }
}

struct blinky_data blinky0 = { &led0, DELAY0_MS };
struct blinky_data blinky1 = { &led1, DELAY1_MS };
struct blinky_data blinky2 = { &led2, DELAY2_MS };
struct blinky_data blinky3 = { &led3, DELAY3_MS };

K_THREAD_DEFINE(blinky0_thread, STACK_SIZE, blinky_task, &blinky0, NULL, NULL, 5, 0, 0);
K_THREAD_DEFINE(blinky1_thread, STACK_SIZE, blinky_task, &blinky1, NULL, NULL, 5, 0, 0);
K_THREAD_DEFINE(blinky2_thread, STACK_SIZE, blinky_task, &blinky2, NULL, NULL, 5, 0, 0);
K_THREAD_DEFINE(blinky3_thread, STACK_SIZE, blinky_task, &blinky3, NULL, NULL, 5, 0, 0);
