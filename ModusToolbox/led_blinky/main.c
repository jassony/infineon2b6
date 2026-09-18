/* Minimal port of M0_BSW/SOURCE/ld_gpio: LED on P0.0, initially high.
 * The vendor CM0+ startup initializes SRAM/ECC and disables the internal WDT.
 * This application deliberately does not start CM4 or initialize power outputs.
 */
#include "cy_device_headers.h"
#include "cy_gpio.h"
#include "cy_syslib.h"

#define LED_PORT GPIO_PRT0
#define LED_PIN  (0u)
#define LED_TOGGLE_MS (500u)

/* Debugger watch values; no XCP or A2L interface is introduced. */
volatile uint32_t led_toggle_count = 0u;
volatile uint32_t led_output_level = 1u;
volatile uint32_t led_init_status = 0u;

int main(void)
{
    const cy_stc_gpio_pin_config_t led_config = {
        .outVal = 1u,
        .driveMode = CY_GPIO_DM_STRONG,
        .hsiom = HSIOM_SEL_GPIO,
        .intEdge = 0u,
        .intMask = 0u
    };

    led_init_status = (uint32_t)Cy_GPIO_Pin_Init(LED_PORT, LED_PIN, &led_config);
    if (led_init_status != (uint32_t)CY_GPIO_SUCCESS)
    {
        for (;;) { __NOP(); }
    }

    __enable_irq();
    for (;;)
    {
        Cy_SysLib_Delay(LED_TOGGLE_MS);
        Cy_GPIO_Inv(LED_PORT, LED_PIN);
        led_output_level ^= 1u;
        ++led_toggle_count;
    }
}
