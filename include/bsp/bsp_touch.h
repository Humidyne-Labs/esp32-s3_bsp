#ifndef BSP_TOUCH_H
#define BSP_TOUCH_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "bsp/pinout.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BSP_TOUCH_I2C_ADDR      (0x38)

/**
 * @brief Initialize FT6336 touch controller over shared I2C bus
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t bsp_touch_init(void);

/**
 * @brief Perform hardware reset of FT6336 touch panel
 */
void bsp_touch_reset(void);

/**
 * @brief Read current touch point coordinates
 * 
 * @param x Pointer to store X coordinate (0-199)
 * @param y Pointer to store Y coordinate (0-199)
 * @return true if touch detected and coordinates valid, false otherwise
 */
bool bsp_touch_read(uint16_t *x, uint16_t *y);

#ifdef __cplusplus
}
#endif

#endif /* BSP_TOUCH_H */
