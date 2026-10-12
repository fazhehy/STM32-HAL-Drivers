/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usb_device.c
  * @version        : v1.0_Cube
  * @brief          : This file implements the USB Device
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/

#include "usb_device.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_msc.h"
#include "usbd_storage_if.h"

/* USER CODE BEGIN Includes */
#include "usbd_cdc.h"
#include "usbd_cdc_if.h"
/* USBD_CMPSIT_SetClassID 的声明在这里（CLASS_TYPE_* 在 usbd_def.h 里已经有了） */
#include "usbd_composite_builder.h"

/* USER CODE END Includes */

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
/* 复合构建器只读这两张表，但形参是 uint8_t*，不能写 const。
   顺序即接口号：MSC 先注册占接口 0，CDC 后注册占接口 1 和 2。 */
static uint8_t msc_endpoints[] = {0x83U, 0x03U};         /* 批量IN, 批量OUT */
static uint8_t cdc_endpoints[] = {0x81U, 0x01U, 0x82U};  /* 批量IN, 批量OUT, 中断IN */

/* USER CODE END PV */

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USB Device Core handle declaration. */
USBD_HandleTypeDef hUsbDeviceFS;

/*
 * -- Insert your variables declaration here --
 */
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*
 * -- Insert your external function declaration here --
 */
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/**
  * Init USB device Library, add supported class and start the library
  * @retval None
  */
void MX_USB_DEVICE_Init(void)
{
  /* USER CODE BEGIN USB_DEVICE_Init_PreTreatment */

  /* USER CODE END USB_DEVICE_Init_PreTreatment */

  /* Init Device Library, add supported class and start the library. */
  if (USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS) != USBD_OK)
  {
    Error_Handler();
  }
  /* CubeMX 生成不了复合设备，这一段是手工写的，重新生成会覆盖。
     复合构建器把 pdev->classId 当"下一个空槽"用：RegisterClassComposite 收尾时 classId++。
     而 USBD_CMPSIT_SetClassID 会把 classId 拨到指定类并**留在那儿**，所以
     "注册类 → SetClassID → 注册 fops" 这种交替写法会让下一个类注册到上一个类的槽上：
     AddClass 见该槽 Active 已是 1 就静默跳过（描述符里不会生成这个类），
     但 pClass[]/EpAdd 已被覆盖、classId 和 NumClasses 照样自增，
     最后 SetClassID 在整个表里找不到这个类，返回 0xFF。
     只能先把所有类注册完（classId 停在 2，正好是下一个空槽），
     再逐个把 classId 拨到对应的类上注册 fops —— 两个 Register* 写的都是 pUserData[pdev->classId]。
     顺序错了的两种表现：fops 落错槽 → MSC 在 SET_CONFIGURATION 里解引用 pUserData[0]==NULL 硬件异常；
     或者 SetClassID 找不到类 → 卡在 Error_Handler，D+ 上拉永远不拉开，主机连设备都看不到。 */
  if (USBD_RegisterClassComposite(&hUsbDeviceFS, &USBD_MSC, CLASS_TYPE_MSC, msc_endpoints) != USBD_OK)
  {
    Error_Handler();
  }
  if (USBD_RegisterClassComposite(&hUsbDeviceFS, &USBD_CDC, CLASS_TYPE_CDC, cdc_endpoints) != USBD_OK)
  {
    Error_Handler();
  }
  if (USBD_CMPSIT_SetClassID(&hUsbDeviceFS, CLASS_TYPE_MSC, 0U) == 0xFFU)
  {
    Error_Handler();
  }
  if (USBD_MSC_RegisterStorage(&hUsbDeviceFS, &USBD_Storage_Interface_fops_FS) != USBD_OK)
  {
    Error_Handler();
  }
  if (USBD_CMPSIT_SetClassID(&hUsbDeviceFS, CLASS_TYPE_CDC, 0U) == 0xFFU)
  {
    Error_Handler();
  }
  if (USBD_CDC_RegisterInterface(&hUsbDeviceFS, &USBD_Interface_fops_FS) != USBD_OK)
  {
    Error_Handler();
  }
  if (USBD_Start(&hUsbDeviceFS) != USBD_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN USB_DEVICE_Init_PostTreatment */
  HAL_PWREx_EnableUSBVoltageDetector();

  /* USER CODE END USB_DEVICE_Init_PostTreatment */
}

/**
  * @}
  */

/**
  * @}
  */

