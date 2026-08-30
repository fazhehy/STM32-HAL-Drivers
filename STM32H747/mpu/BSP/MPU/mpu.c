#include "mpu.h"

void mpu_set_protection(uint32_t base_address,
                        uint32_t size,
                        uint32_t region_number,
                        uint32_t disable_exec,
                        uint32_t access_permission,
                        uint32_t shareable,
                        uint32_t cacheable,
                        uint32_t bufferable)
{
    MPU_Region_InitTypeDef mpu_region = {0};

    // 每次修改一个 Region 时，先关闭 MPU，配置完成后再重新启用
    HAL_MPU_Disable();

    mpu_region.Enable = MPU_REGION_ENABLE;
    mpu_region.Number = region_number;
    mpu_region.BaseAddress = base_address;
    mpu_region.Size = size;
    mpu_region.SubRegionDisable = 0x00U;
    mpu_region.TypeExtField = MPU_TEX_LEVEL0;
    mpu_region.AccessPermission = access_permission;
    mpu_region.DisableExec = disable_exec;
    mpu_region.IsShareable = shareable;
    mpu_region.IsCacheable = cacheable;
    mpu_region.IsBufferable = bufferable;
    HAL_MPU_ConfigRegion(&mpu_region);

    HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

void mpu_memory_protection(void)
{
    MPU_Region_InitTypeDef mpu_region = {0};

    // 修改 Region 配置前必须先关闭 MPU
    HAL_MPU_Disable();

    // Region 0：外部地址空间保护背景区
    // 4GB 被分成 8 个 512MB 子区域，0x87 关闭子区域 0、1、2、7
    // 实际禁止访问 0x60000000～0xDFFFFFFF，高编号 Region 可覆盖该区域
    mpu_region.Enable = MPU_REGION_ENABLE;
    mpu_region.Number = MPU_REGION_NUMBER0;
    mpu_region.BaseAddress = 0x00000000U;
    mpu_region.Size = MPU_REGION_SIZE_4GB;
    mpu_region.SubRegionDisable = 0x87U;
    mpu_region.TypeExtField = MPU_TEX_LEVEL0;
    mpu_region.AccessPermission = MPU_REGION_NO_ACCESS;
    mpu_region.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
    mpu_region.IsShareable = MPU_ACCESS_SHAREABLE;
    mpu_region.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
    mpu_region.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
    HAL_MPU_ConfigRegion(&mpu_region);

    // Region 1：CM7 DTCM，128KB；TCM 直接连接内核，不经过 D-Cache
    mpu_region.Number = MPU_REGION_NUMBER1;
    mpu_region.BaseAddress = 0x20000000U;
    mpu_region.Size = MPU_REGION_SIZE_128KB;
    mpu_region.SubRegionDisable = 0x00U;
    mpu_region.TypeExtField = MPU_TEX_LEVEL0;
    mpu_region.AccessPermission = MPU_REGION_FULL_ACCESS;
    mpu_region.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
    mpu_region.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
    mpu_region.IsCacheable = MPU_ACCESS_CACHEABLE;
    mpu_region.IsBufferable = MPU_ACCESS_BUFFERABLE;
    HAL_MPU_ConfigRegion(&mpu_region);

    // Region 2：D1 域 AXI SRAM，512KB，允许 Cache 和 Buffer
    mpu_region.Number = MPU_REGION_NUMBER2;
    mpu_region.BaseAddress = 0x24000000U;
    mpu_region.Size = MPU_REGION_SIZE_512KB;
    mpu_region.AccessPermission = MPU_REGION_FULL_ACCESS;
    mpu_region.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
    mpu_region.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
    mpu_region.IsCacheable = MPU_ACCESS_CACHEABLE;
    mpu_region.IsBufferable = MPU_ACCESS_BUFFERABLE;
    HAL_MPU_ConfigRegion(&mpu_region);

    // Region 3：D2 域 SRAM 地址窗口，512KB
    mpu_region.Number = MPU_REGION_NUMBER3;
    mpu_region.BaseAddress = 0x30000000U;
    mpu_region.Size = MPU_REGION_SIZE_512KB;
    mpu_region.AccessPermission = MPU_REGION_FULL_ACCESS;
    mpu_region.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
    mpu_region.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
    mpu_region.IsCacheable = MPU_ACCESS_CACHEABLE;
    mpu_region.IsBufferable = MPU_ACCESS_BUFFERABLE;
    HAL_MPU_ConfigRegion(&mpu_region);

    // Region 4：D3 域 SRAM4，64KB
    mpu_region.Number = MPU_REGION_NUMBER4;
    mpu_region.BaseAddress = 0x38000000U;
    mpu_region.Size = MPU_REGION_SIZE_64KB;
    mpu_region.AccessPermission = MPU_REGION_FULL_ACCESS;
    mpu_region.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
    mpu_region.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
    mpu_region.IsCacheable = MPU_ACCESS_CACHEABLE;
    mpu_region.IsBufferable = MPU_ACCESS_BUFFERABLE;
    HAL_MPU_ConfigRegion(&mpu_region);

    // Region 5：FMC/LCD 地址窗口，禁止 Cache 和 Buffer
    mpu_region.Number = MPU_REGION_NUMBER5;
    mpu_region.BaseAddress = 0x60000000U;
    mpu_region.Size = MPU_REGION_SIZE_64MB;
    mpu_region.AccessPermission = MPU_REGION_FULL_ACCESS;
    mpu_region.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
    mpu_region.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
    mpu_region.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
    mpu_region.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
    HAL_MPU_ConfigRegion(&mpu_region);

    // Region 6：外部 SDRAM，32MB，允许 Cache 和 Buffer
    mpu_region.Number = MPU_REGION_NUMBER6;
    mpu_region.BaseAddress = 0xD0000000U;
    mpu_region.Size = MPU_REGION_SIZE_32MB;
    mpu_region.AccessPermission = MPU_REGION_FULL_ACCESS;
    mpu_region.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
    mpu_region.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
    mpu_region.IsCacheable = MPU_ACCESS_CACHEABLE;
    mpu_region.IsBufferable = MPU_ACCESS_BUFFERABLE;
    HAL_MPU_ConfigRegion(&mpu_region);

    // Region 7：NAND 地址窗口，禁止执行、Cache 和 Buffer
    mpu_region.Number = MPU_REGION_NUMBER7;
    mpu_region.BaseAddress = 0x80000000U;
    mpu_region.Size = MPU_REGION_SIZE_256MB;
    mpu_region.AccessPermission = MPU_REGION_FULL_ACCESS;
    mpu_region.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
    mpu_region.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
    mpu_region.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
    mpu_region.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
    HAL_MPU_ConfigRegion(&mpu_region);

    // 未命中任何 Region 的特权访问继续使用芯片默认内存映射
    HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}
