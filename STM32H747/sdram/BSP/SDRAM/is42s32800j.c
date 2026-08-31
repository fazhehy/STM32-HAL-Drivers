
#include "is42s32800j.h"

#define IS42S32800J_REFRESH_COUNT 1699U
#define IS42S32800J_COMMAND_TIMEOUT 0x1000U

#define IS42S32800J_MODEREG_BURST_LENGTH_1 ((uint32_t)0x0000U)
#define IS42S32800J_MODEREG_BURST_TYPE_SEQUENTIAL ((uint32_t)0x0000U)
#define IS42S32800J_MODEREG_CAS_LATENCY_3 ((uint32_t)0x0030U)
#define IS42S32800J_MODEREG_OPERATING_MODE_STANDARD ((uint32_t)0x0000U)
#define IS42S32800J_MODEREG_WRITEBURST_MODE_SINGLE ((uint32_t)0x0200U)

static bool send_command(SDRAM_HandleTypeDef* hsdram, uint32_t mode, uint32_t auto_refresh_number,
                         uint32_t mode_register)
{
    FMC_SDRAM_CommandTypeDef command = {0};

    command.CommandMode = mode;
    command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK2;
    command.AutoRefreshNumber = auto_refresh_number;
    command.ModeRegisterDefinition = mode_register;

    return HAL_SDRAM_SendCommand(hsdram, &command, IS42S32800J_COMMAND_TIMEOUT) == HAL_OK;
}

bool is42s32800j_init(SDRAM_HandleTypeDef* hsdram)
{
    if (hsdram == NULL) {
        return false;
    }

    // 1. I/O 补偿单元
    //
    // STM32H7 的 FMC 可跑到 100MHz+, 信号边沿很陡. I/O 补偿单元实时
    // 调节 GPIO 输出驱动器的斜率和阻抗, 补偿温度/电压漂移对时序的影响.
    // 不加的话高速 FMC 可能间歇性读写错误.
    HAL_PWREx_EnableUSBVoltageDetector();
    __HAL_RCC_CSI_ENABLE();
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    HAL_EnableCompensationCell();

    // 2. MPU 配置
    //
    // main.c 的 MPU_Config() 把 Region0 (背景) 设成了 NO_ACCESS.
    // 这里在 0xD0000000 开一个 32MB 的"洞", 允许 CPU 访问 SDRAM.
    //
    // Cacheable + Bufferable: SDRAM 读写走 L1-Cache (WT 模式), 写操作
    //   可被 Store Buffer 合并, 减少总线等待.
    // NOT_SHAREABLE: 告诉 CPU 这块内存只有 CM7 用, 不需要维护一致性协议.
    // DISABLE_EXEC: 防止跑飞把 SDRAM 数据当代码执行.
    // MPU_Region_InitTypeDef mpu = {0};
    // HAL_MPU_Disable();

    // mpu.Enable = MPU_REGION_ENABLE;
    // mpu.Number = MPU_REGION_NUMBER6;
    // mpu.BaseAddress = 0xD0000000;
    // mpu.Size = MPU_REGION_SIZE_32MB;
    // mpu.SubRegionDisable = 0x0;
    // mpu.TypeExtField = MPU_TEX_LEVEL0;
    // mpu.AccessPermission = MPU_REGION_FULL_ACCESS;
    // mpu.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
    // mpu.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
    // mpu.IsCacheable = MPU_ACCESS_CACHEABLE;
    // mpu.IsBufferable = MPU_ACCESS_BUFFERABLE;
    // HAL_MPU_ConfigRegion(&mpu);

    // HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

    // 3. SDRAM 初始化命令序列 (JEDEC 标准流程)
    //
    // 上电后 SDRAM 内部状态不确定, 必须按顺序发送:
    //
    //   CLK_ENABLE  — 使能 SDCLK, 芯片开始响应命令
    //   wait 1ms    — 时钟稳定等待 (Datasheet: tPU >= 100us)
    //   PALL        — 预充电所有 Bank, 全部回到空闲态
    //   AUTOREFRESH — 8 次自动刷新, 初始化所有行 (规格只需 2 次)
    //   LOAD_MODE   — 写入模式寄存器: CAS Latency / Burst Length 等
    //   RefreshRate — 自动刷新间隔, 保证 64ms 内完成 4096 次刷新
    //
    // 模式寄存器: Burst Length=1, CAS Latency=3, Write Burst=Single
    // CL=3 是因为 SDCLK=110MHz (tck=9.09ns), CL=2 要求 tck>=10ns 不满足.
    uint32_t mode_register = IS42S32800J_MODEREG_BURST_LENGTH_1 | IS42S32800J_MODEREG_BURST_TYPE_SEQUENTIAL |
                             IS42S32800J_MODEREG_CAS_LATENCY_3 | IS42S32800J_MODEREG_OPERATING_MODE_STANDARD |
                             IS42S32800J_MODEREG_WRITEBURST_MODE_SINGLE;

    if (!send_command(hsdram, FMC_SDRAM_CMD_CLK_ENABLE, 1, 0)) {
        return false;
    }

    HAL_Delay(1);

    if (!send_command(hsdram, FMC_SDRAM_CMD_PALL, 1, 0)) {
        return false;
    }

    if (!send_command(hsdram, FMC_SDRAM_CMD_AUTOREFRESH_MODE, 8, 0)) {
        return false;
    }

    if (!send_command(hsdram, FMC_SDRAM_CMD_LOAD_MODE, 1, mode_register)) {
        return false;
    }

    if (HAL_SDRAM_ProgramRefreshRate(hsdram, IS42S32800J_REFRESH_COUNT) != HAL_OK) {
        return false;
    }

    return true;
}

// 只读写基地址 memory[0], 用 6 种 pattern 逐位验证 32 根数据线.
static bool test_data_bus(void)
{
    volatile uint32_t* const memory = (volatile uint32_t*)IS42S32800J_BASE_ADDR;
    const uint32_t patterns[] = {0x00000000UL, 0xFFFFFFFFUL, 0xAAAAAAAAUL, 0x55555555UL, 0x12345678UL, 0x87654321UL};

    for (uint32_t i = 0; i < sizeof(patterns) / sizeof(patterns[0]); i++) {
        memory[0] = patterns[i];

        if (memory[0] != patterns[i]) {
            return false;
        }
    }

    return true;
}

// 在不同地址偏移写入唯一值 (偏移量编码进数据), 验证每根地址线能正确寻址.
static bool test_address_bus(void)
{
    volatile uint32_t* const memory = (volatile uint32_t*)IS42S32800J_BASE_ADDR;
    const uint32_t offsets[] = {0x00000000UL, 0x00000004UL, 0x00000100UL,
                                0x00010000UL, 0x00100000UL, 0x00400000UL,
                                0x00800000UL, 0x01000000UL, IS42S32800J_SIZE_BYTES - 4UL};

    for (uint32_t i = 0; i < sizeof(offsets) / sizeof(offsets[0]); i++) {
        uint32_t index = offsets[i] / sizeof(uint32_t);
        memory[index] = 0xA5A50000UL ^ offsets[i];
    }

    for (uint32_t i = 0; i < sizeof(offsets) / sizeof(offsets[0]); i++) {
        uint32_t index = offsets[i] / sizeof(uint32_t);

        if (memory[index] != (0xA5A50000UL ^ offsets[i])) {
            return false;
        }
    }

    return true;
}

// 前 1MB 逐字写入递增唯一值, 验证大范围连续读写和刷新下的数据保持.
static bool test_block_fill(void)
{
    volatile uint32_t* const memory = (volatile uint32_t*)IS42S32800J_BASE_ADDR;
    const uint32_t test_words = (1024UL * 1024UL) / sizeof(uint32_t);

    for (uint32_t i = 0; i < test_words; i++) {
        memory[i] = 0x5A5A0000UL + i;
    }

    for (uint32_t i = 0; i < test_words; i++) {
        if (memory[i] != 0x5A5A0000UL + i) {
            return false;
        }
    }

    return true;
}

bool is42s32800j_test(void)
{
    if (!test_data_bus()) {
        return false;
    }

    if (!test_address_bus()) {
        return false;
    }

    if (!test_block_fill()) {
        return false;
    }

    return true;
}
