#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>


#define STACK_SIZE   1024

#define PRIO_BUTTON_TASK 5
#define PRIO_LED_TASK 5

#define LED0   DT_ALIAS(led0) 
#define LED1   DT_ALIAS(led1) 
#define LED2   DT_ALIAS(led2) 
#define LED3   DT_ALIAS(led3) 
#define BTN_NODE DT_ALIAS(btn)

K_SEM_DEFINE(sem, 0, 1)
K_MUTEX_DEFINE(mtx);

struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0, gpios); 
struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1, gpios); 
struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2, gpios); 
struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(LED3, gpios); 
struct gpio_dt_spec btn = GPIO_DT_SPEC_GET(BTN_NODE, gpios);


struct blinky_data {
    struct gpio_dt_spec *led;
    int delay;
};

static struct blinky_data led_list[] = {
    {&led0, 100},
    {&led1, 200},
    {&led2, 300},
    {&led3, 500}
};

static struct gpio_callback btn_cb_data;
static int mtx_index=0;

void button_isr() {
    k_sem_give(&sem);
}

/* Initialization function */
void init() {
    gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
    gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
    gpio_pin_configure_dt(&led2, GPIO_OUTPUT_ACTIVE);
    gpio_pin_configure_dt(&led3, GPIO_OUTPUT_ACTIVE);

    gpio_pin_configure_dt(&btn, GPIO_INPUT);

    /* Registers interrupt on edge to active level */
    gpio_pin_interrupt_configure_dt(&btn, GPIO_INT_EDGE_TO_ACTIVE);

    /* Initializes callback struct with ISR and the pins on which ISR should trigger  */
    gpio_init_callback(&btn_cb_data, button_isr, BIT(btn.pin));

    /* Adds ISR callback to the device */
    gpio_add_callback_dt(&btn, &btn_cb_data);
}


void blinky_task(void *arg1, void *arg2, void *arg3) 
{
    init();
    for (;;){
        k_mutex_lock(&mtx, K_FOREVER);
        struct blinky_data *data = &led_list[mtx_index];
        k_mutex_unlock(&mtx);
        gpio_pin_toggle_dt(data->led); 
        k_msleep(data->delay); 
    }
}

void button_task() {
    init();
    for (;;) {
        /* Wait until semaphore is given by ISR (meaning button is pressed), 
        take semaphore and toggle LED*/
        k_sem_take(&sem, K_FOREVER);
        k_mutex_lock(&mtx, K_FOREVER);
        mtx_index =(mtx_index+1)%4;
        k_mutex_unlock(&mtx);
    }
}


K_THREAD_DEFINE(blinky_tid, STACK_SIZE, blinky_task, NULL, NULL, NULL, PRIO_LED_TASK, 0, 0);
K_THREAD_DEFINE(button_tid, STACK_SIZE, button_task, NULL, NULL, NULL, PRIO_BUTTON_TASK, 0, 0);