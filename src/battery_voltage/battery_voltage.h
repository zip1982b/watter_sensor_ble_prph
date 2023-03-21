#include <zephyr/drivers/adc.h>


void configure_adc_device(void);
int8_t get_battery_level(void);
uint8_t get_percent(int32_t val);