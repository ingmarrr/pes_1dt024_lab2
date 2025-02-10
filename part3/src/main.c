#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <stdio.h> 
#include <zephyr/drivers/gpio.h>
#include <zephyr/timing/timing.h>

#define STACK_SIZE  2048

#define BTN_NODE DT_ALIAS(btn)

K_SEM_DEFINE(sem, 0, 1);
 
static int mode = 0;
uint64_t total_cycles;
double total_s;
uint64_t start_time, end_time;

struct gpio_dt_spec btn = GPIO_DT_SPEC_GET(BTN_NODE, gpios);
static struct gpio_callback btn_cb_data;

/* Corrected ISR signature */
void button_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
    k_sem_give(&sem);
}

void init() {
    if (!device_is_ready(btn.port)) {
        printk("Error: Button device not ready\n");
        return;
    }

    /* Configure button pin as input with pull-up */
    gpio_pin_configure_dt(&btn, GPIO_INPUT | GPIO_PULL_UP);

    /* Registers interrupt on edge to active level */
    gpio_pin_interrupt_configure_dt(&btn, GPIO_INT_EDGE_TO_ACTIVE);

    /* Initializes callback struct with ISR */
    gpio_init_callback(&btn_cb_data, button_isr, BIT(btn.pin));

    /* Adds ISR callback to the device */
    gpio_add_callback_dt(&btn, &btn_cb_data);

    /* Initialize and start timing */
    timing_init();
    timing_start();
}

void button_task() {
    init();
    printk("Reaction time measurement. Push button GP20, then push it again after 3s!\n");
    for (;;) {
        /* Wait until semaphore is given by ISR (button press detected) */
        k_sem_take(&sem, K_FOREVER);
        if (mode == 0) {
            printf("Button pushed\n");
            fflush(stdout);
            start_time = timing_counter_get();
            mode = 1;
        } else if (mode == 1) {
            end_time = timing_counter_get();
            total_cycles = end_time - start_time;
            total_s = timing_cycles_to_ns(total_cycles) / 1e9; // Conversion from ns to s
            printf("Reaction time: %.4f s. ", total_s);
            fflush(stdout);
            printf("You were off by %.4f s.\n\n", total_s-3);
            fflush(stdout);
            printf("Push button to start again!\n");
            fflush(stdout);
            mode = 0;
        }
    }
}

K_THREAD_DEFINE(button_tid, STACK_SIZE, button_task, NULL, NULL, NULL, 5, 0, 0);
