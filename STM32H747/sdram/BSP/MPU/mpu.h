#ifndef BSP_MPU_H
#define BSP_MPU_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32h7xx_hal.h"

void mpu_set_protection(uint32_t base_address,
                        uint32_t size,
                        uint32_t region_number,
                        uint32_t disable_exec,
                        uint32_t access_permission,
                        uint32_t shareable,
                        uint32_t cacheable,
                        uint32_t bufferable);
void mpu_memory_protection(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_MPU_H */
