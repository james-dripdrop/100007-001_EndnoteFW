/* Auto-generated config file clock_config.h */
#ifndef CLOCK_CONFIG_H
#define CLOCK_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

#ifndef F_CPU
#define F_CPU 14745000
#endif
// <h> SPI Clock Settings
// <y> SPI Clock source
// <CLKio"> CLKio
// <i> This defines the clock source for the SPI module
// <id> spi_clock_source
#define CONF_SPI_SRC CLKio

// </h>

// <h> TC1 Clock Settings
// <y> TC1 Clock source
// <CLKio"> CLKio
// <i> This defines the clock source for the TC1 module
// <id> tc16_clock_source
#define CONF_TC1_SRC CLKio

// </h>

// <h> TC3 Clock Settings
// <y> TC3 Clock source
// <CLKio"> CLKio
// <i> This defines the clock source for the TC3 module
// <id> tc16_clock_source
#define CONF_TC3_SRC CLKio

// </h>

// <h> USART Clock Settings
// <y> USART Clock source
// <CLKio"> CLKio
// <i> This defines the clock source for the USART module
// <id> usart_clock_source
#define CONF_USART_SRC CLKio

// </h>

// <<< end of configuration section >>>

#endif // CLOCK_CONFIG_H
