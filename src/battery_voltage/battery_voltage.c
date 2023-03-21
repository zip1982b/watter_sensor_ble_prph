#include "battery_voltage.h"
#include <zephyr/logging/log.h>

#define LOG_MODULE_NAME battery_voltage
LOG_MODULE_REGISTER(LOG_MODULE_NAME);
#define ADC_MAX_VALUE 530
#define ADC_MIN_VALUE 478

#define ADC_NODE DT_NODELABEL(adc)
const struct device *adc_dev = DEVICE_DT_GET(ADC_NODE);

#define ADC_RESOLUTION 10 //0-1023
#define ADC_CHANNEL 0

#define ADC_PORT SAADC_CH_PSELP_PSELP_AnalogInput0 //AIN0
#define ADC_REFERENCE ADC_REF_INTERNAL				//0.6V
#define ADC_GAIN ADC_GAIN_1_5						//ADC_REFERENCE*5

struct adc_channel_cfg chnl0_cfg = {
	.gain = ADC_GAIN,
	.reference = ADC_REFERENCE,
	.acquisition_time = ADC_ACQ_TIME_DEFAULT,
	.channel_id = ADC_CHANNEL,
#ifdef CONFIG_ADC_NRFX_SAADC
	.input_positive = ADC_PORT
#endif
};


int16_t sample_buffer[1];
struct adc_sequence sequence = {
	.channels = BIT(ADC_CHANNEL),
	.buffer = sample_buffer,
	.buffer_size = sizeof(sample_buffer),
	.resolution = ADC_RESOLUTION
};


void configure_adc_device(void){
    int err;

    if(!device_is_ready(adc_dev)){
        LOG_ERR("ADC controller device is not ready");
    }

    err = adc_channel_setup(adc_dev, &chnl0_cfg);
    if(err < 0){
        LOG_ERR("Could not setup channel #%d", err);
    }
}


int8_t get_battery_level(void){
    LOG_INF("ADC reading ... ");
    int err;
    uint8_t percent;
    err = adc_read(adc_dev, &sequence);
    if (err != 0){
        LOG_ERR("Could not read adc (%d)", err);
        return -1; //err
    }
    else {
        int32_t val_mv = sample_buffer[0];
        LOG_INF("ADC value = %d", val_mv);
        percent = get_percent(val_mv);

        int32_t adc_vref = adc_ref_internal(adc_dev);

        err = adc_raw_to_millivolts(adc_vref, ADC_GAIN, ADC_RESOLUTION, &val_mv);
        if(err < 0){
            LOG_ERR("value in mv not available");
        }
        else LOG_INF("ADC voltage = %d mv", val_mv);
        return percent;
    }
}



uint8_t get_percent(int32_t val){
    if(val < ADC_MIN_VALUE){
        return 0;
    }
    else if(val > ADC_MAX_VALUE){
        return 100;
    }
    return ((float)(val - ADC_MIN_VALUE)/(float)(ADC_MAX_VALUE - ADC_MIN_VALUE)) * 100;
}