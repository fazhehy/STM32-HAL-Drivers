#include "cm7_main.h"

#include "log.h"
#include "mpu.h"

// Region 基地址必须按 Region 大小对齐
// 数组正好占满 128B，不会把相邻变量一起设为只读
alignas(128) static volatile uint8_t mpu_test_data[128];

void cm7_main(void)
{
    log_info("MPU experiment started on CM7");

    // Region 8 尚未配置，数组由 Region 1 管理，可以正常读写
    mpu_test_data[0] = 0x5AU;
    log_info("Before protection: write OK, value=0x%02X", mpu_test_data[0]);

    // Region 8 优先级高于 Region 1，将重叠的 128B 配置为只读
    mpu_set_protection(
        static_cast<uint32_t>(reinterpret_cast<uintptr_t>(mpu_test_data)), // 保护区域的起始地址
        MPU_REGION_SIZE_128B,                                             // 保护区域大小为 128B
        MPU_REGION_NUMBER8,                                              // 使用 Region 8，覆盖 Region 1
        MPU_INSTRUCTION_ACCESS_DISABLE,                                  // 禁止执行该区域中的指令
        MPU_REGION_PRIV_RO_URO,                                          // 特权级和用户级都只能读取
        MPU_ACCESS_NOT_SHAREABLE,                                        // 不与其他总线主设备共享
        MPU_ACCESS_NOT_CACHEABLE,                                        // 不使用 Cache
        MPU_ACCESS_NOT_BUFFERABLE);                                      // 不使用写缓冲

    // 使能 MemManage Fault，否则违规访问会升级成 HardFault
    SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk;
    __DSB();
    __ISB();

    log_info("Protection enabled: read OK, value=0x%02X", mpu_test_data[0]);
    log_info("Writing again; the next message should come from MemManage_Handler");

    mpu_test_data[0] = 0xA5U;

    // MPU 正常工作时不会运行到这里
    log_error("MPU experiment failed: protected write did not fault");
    for (;;) {
    }
}
