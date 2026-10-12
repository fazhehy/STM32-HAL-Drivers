/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_cdc_if.c
  * @version        : v1.0_Cube
  * @brief          : Usb device for Virtual Com Port.
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
#include "usbd_cdc_if.h"

/* USER CODE BEGIN INCLUDE */
/* 本文件从 vcp 工程拷来，为复合设备改了六处，CubeMX 生成不了复合设备，重新生成会覆盖：
   1. 引复合构建器的头，取类的实例编号；
   2. CDC_Init_FS 里 USBD_CDC_SetTxBuffer 多一个 ClassId 参数（复合模式下才有）；
   3. CDC_Transmit_FS 不能再用 pClassData —— 它指向最后初始化的那个类；
   4. CDC_Receive_FS 把收到的字节交给 BSP 的环形缓冲，由主循环发回；
   5. 新增 CDC_IsTransmitReady_FS，供主循环判断发送缓冲区能不能复用；
   6. CDC_Receive_FS 按空位挂接收、新增 CDC_ResumeReceive_FS，收不满就 NAK 让主机等着。 */
#include "usbd_composite_builder.h"
#include "usb_cdc.h"

/* USER CODE END INCLUDE */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* OUT 接收是否已经挂上。库的 USBD_CDC_Init 在这个类的 Init 回调之后自己挂一次，
   之后由 CDC_Receive_FS 和主循环的 CDC_ResumeReceive_FS 轮流维护：
   环形缓冲放不下一整包时就不挂，端点保持 NAK，主机的写阻塞等着，字节不会丢。 */
static bool receive_armed;
/* USER CODE END PV */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @brief Usb device library.
  * @{
  */

/** @addtogroup USBD_CDC_IF
  * @{
  */

/** @defgroup USBD_CDC_IF_Private_TypesDefinitions USBD_CDC_IF_Private_TypesDefinitions
  * @brief Private types.
  * @{
  */

/* USER CODE BEGIN PRIVATE_TYPES */

/* USER CODE END PRIVATE_TYPES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Defines USBD_CDC_IF_Private_Defines
  * @brief Private defines.
  * @{
  */

/* USER CODE BEGIN PRIVATE_DEFINES */
/* USER CODE END PRIVATE_DEFINES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Macros USBD_CDC_IF_Private_Macros
  * @brief Private macros.
  * @{
  */

/* USER CODE BEGIN PRIVATE_MACRO */

/* USER CODE END PRIVATE_MACRO */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Variables USBD_CDC_IF_Private_Variables
  * @brief Private variables.
  * @{
  */
/* Create buffer for reception and transmission           */
/* It's up to user to redefine and/or remove those define */
/** Received data over USB are stored in this buffer      */
uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];

/** Data to send over USB CDC are stored in this buffer   */
uint8_t UserTxBufferFS[APP_TX_DATA_SIZE];

/* USER CODE BEGIN PRIVATE_VARIABLES */

/* USER CODE END PRIVATE_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Exported_Variables USBD_CDC_IF_Exported_Variables
  * @brief Public variables.
  * @{
  */

extern USBD_HandleTypeDef hUsbDeviceFS;

/* USER CODE BEGIN EXPORTED_VARIABLES */

/* USER CODE END EXPORTED_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_FunctionPrototypes USBD_CDC_IF_Private_FunctionPrototypes
  * @brief Private functions declaration.
  * @{
  */

static int8_t CDC_Init_FS(void);
static int8_t CDC_DeInit_FS(void);
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length);
static int8_t CDC_Receive_FS(uint8_t* pbuf, uint32_t *Len);
static int8_t CDC_TransmitCplt_FS(uint8_t *pbuf, uint32_t *Len, uint8_t epnum);

/* USER CODE BEGIN PRIVATE_FUNCTIONS_DECLARATION */

/* USER CODE END PRIVATE_FUNCTIONS_DECLARATION */

/**
  * @}
  */

USBD_CDC_ItfTypeDef USBD_Interface_fops_FS =
{
  CDC_Init_FS,
  CDC_DeInit_FS,
  CDC_Control_FS,
  CDC_Receive_FS,
  CDC_TransmitCplt_FS
};

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initializes the CDC media low layer over the FS USB IP
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Init_FS(void)
{
  /* USER CODE BEGIN 3 */
  /* Set Application Buffers */
  /* 这个回调由核心在 CDC 的 Init 里调用，此时 classId 就是 CDC 自己 */
  USBD_CDC_SetTxBuffer(&hUsbDeviceFS, UserTxBufferFS, 0, (uint8_t)hUsbDeviceFS.classId);
  USBD_CDC_SetRxBuffer(&hUsbDeviceFS, UserRxBufferFS);
  /* 上一次会话（拔插或重新 SET_CONFIGURATION）可能还剩着没发回去的字节，清掉，
     免得它们被当成新会话收到的数据发回主机。rx_dropped 是累计值，不跟着清。 */
  usb_cdc_reset();
  /* 核心马上会在 USBD_CDC_Init 里自己挂一次 OUT 接收，标志跟着置真，
     否则 CDC_ResumeReceive_FS 会以为没挂，在传输途中重复挂一次。 */
  receive_armed = true;
  return (USBD_OK);
  /* USER CODE END 3 */
}

/**
  * @brief  DeInitializes the CDC media low layer
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_DeInit_FS(void)
{
  /* USER CODE BEGIN 4 */
  return (USBD_OK);
  /* USER CODE END 4 */
}

/**
  * @brief  Manage the CDC class requests
  * @param  cmd: Command code
  * @param  pbuf: Buffer containing command data (request parameters)
  * @param  length: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length)
{
  /* USER CODE BEGIN 5 */
  switch(cmd)
  {
    case CDC_SEND_ENCAPSULATED_COMMAND:

    break;

    case CDC_GET_ENCAPSULATED_RESPONSE:

    break;

    case CDC_SET_COMM_FEATURE:

    break;

    case CDC_GET_COMM_FEATURE:

    break;

    case CDC_CLEAR_COMM_FEATURE:

    break;

  /*******************************************************************************/
  /* Line Coding Structure                                                       */
  /*-----------------------------------------------------------------------------*/
  /* Offset | Field       | Size | Value  | Description                          */
  /* 0      | dwDTERate   |   4  | Number |Data terminal rate, in bits per second*/
  /* 4      | bCharFormat |   1  | Number | Stop bits                            */
  /*                                        0 - 1 Stop bit                       */
  /*                                        1 - 1.5 Stop bits                    */
  /*                                        2 - 2 Stop bits                      */
  /* 5      | bParityType |  1   | Number | Parity                               */
  /*                                        0 - None                             */
  /*                                        1 - Odd                              */
  /*                                        2 - Even                             */
  /*                                        3 - Mark                             */
  /*                                        4 - Space                            */
  /* 6      | bDataBits  |   1   | Number Data bits (5, 6, 7, 8 or 16).          */
  /*******************************************************************************/
    case CDC_SET_LINE_CODING:

    break;

    case CDC_GET_LINE_CODING:

    break;

    case CDC_SET_CONTROL_LINE_STATE:

    break;

    case CDC_SEND_BREAK:

    break;

  default:
    break;
  }

  return (USBD_OK);
  /* USER CODE END 5 */
}

/**
  * @brief  Data received over USB OUT endpoint are sent over CDC interface
  *         through this function.
  *
  *         @note
  *         This function will issue a NAK packet on any OUT packet received on
  *         USB endpoint until exiting this function. If you exit this function
  *         before transfer is complete on CDC interface (ie. using DMA controller)
  *         it will result in receiving more data while previous ones are still
  *         not sent.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Receive_FS(uint8_t* Buf, uint32_t *Len)
{
  /* USER CODE BEGIN 6 */
  /* 中断里只入队：usb_cdc_receive 不阻塞也不发送，发送留给主循环，
     这样回环的耗时不会压在 OTG_FS 中断里（MSC 也在同一个中断里跑）。
     入队成功、且还放得下一整包时才重新挂接收；放不下就不挂，让端点 NAK，
     主机这一包会重发，主循环排空后由 CDC_ResumeReceive_FS 挂回去。 */
  if (usb_cdc_receive(Buf, *Len) && (usb_cdc_get_free_space() >= CDC_DATA_FS_OUT_PACKET_SIZE)) {
    USBD_CDC_SetRxBuffer(&hUsbDeviceFS, &Buf[0]);
    USBD_CDC_ReceivePacket(&hUsbDeviceFS);
    receive_armed = true;
  } else {
    receive_armed = false;
  }
  return (USBD_OK);
  /* USER CODE END 6 */
}

/**
  * @brief  CDC_Transmit_FS
  *         Data to send over USB IN endpoint are sent over CDC interface
  *         through this function.
  *         @note
  *
  *
  * @param  Buf: Buffer of data to be sent
  * @param  Len: Number of data to be sent (in bytes)
  * @retval USBD_OK if all operations are OK else USBD_FAIL or USBD_BUSY
  */
uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len)
{
  uint8_t result = USBD_OK;
  /* USER CODE BEGIN 7 */
  /* 主循环里调用时 pdev->classId 可能停在别的类上（比如刚处理完 MSC 的批量传输），
     所以按类查自己的实例编号，并把它传给复合模式下的库函数。 */
  uint8_t class_id = (uint8_t)USBD_CMPSIT_GetClassID(&hUsbDeviceFS, CLASS_TYPE_CDC, 0);
  if (class_id == 0xFFU) {
    return USBD_FAIL;
  }
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassDataCmsit[class_id];
  if (hcdc->TxState != 0){
    return USBD_BUSY;
  }
  USBD_CDC_SetTxBuffer(&hUsbDeviceFS, Buf, Len, class_id);
  result = USBD_CDC_TransmitPacket(&hUsbDeviceFS, class_id);
  /* USER CODE END 7 */
  return result;
}

/**
  * @brief  CDC_IsTransmitReady_FS
  *         上一次发送是否已经结束
  * @note   USBD_CDC_SetTxBuffer 只记录指针不拷贝数据，数据要一直有效到 DataIn 回调。
  *         调用方把数据放在自己的缓冲区里循环复用时，必须先用这个函数确认上一次发完了，
  *         否则会在传输途中改掉正在发送的内容。
  * @retval 1 空闲，0 忙或 CDC 还没初始化
  */
uint8_t CDC_IsTransmitReady_FS(void)
{
  /* USER CODE BEGIN 8 */
  uint8_t class_id = (uint8_t)USBD_CMPSIT_GetClassID(&hUsbDeviceFS, CLASS_TYPE_CDC, 0);
  if (class_id == 0xFFU) {
    return 0U;
  }

  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassDataCmsit[class_id];
  if (hcdc == NULL || hcdc->TxState != 0U) {
    return 0U;
  }

  return 1U;
  /* USER CODE END 8 */
}

/**
  * @brief  CDC_ResumeReceive_FS
  *         环形缓冲腾出空位后重新挂 OUT 接收
  * @note   主循环每轮调用。缓冲放不下一整包时 CDC_Receive_FS 不再重新挂接收，
  *         端点保持 NAK、主机的写阻塞等着；排空之后必须由这里挂回去，主机才会继续写。
  *         这里直接调 USBD_LL_PrepareReceive 而不调 USBD_CDC_ReceivePacket：
  *         后者从 pdev->classId 取自己的句柄和端点，而主循环里这个字段可能停在 MSC 上。
  * @retval 无
  */
void CDC_ResumeReceive_FS(void)
{
  /* USER CODE BEGIN 9 */
  if (!receive_armed && (usb_cdc_get_free_space() >= CDC_DATA_FS_OUT_PACKET_SIZE)) {
    (void)USBD_LL_PrepareReceive(&hUsbDeviceFS, CDC_OUT_EP, UserRxBufferFS, CDC_DATA_FS_OUT_PACKET_SIZE);
    receive_armed = true;
  }
  /* USER CODE END 9 */
}

/**
  * @brief  CDC_TransmitCplt_FS
  *         Data transmitted callback
  *
  *         @note
  *         This function is IN transfer complete callback used to inform user that
  *         the submitted Data is successfully sent over USB.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_TransmitCplt_FS(uint8_t *Buf, uint32_t *Len, uint8_t epnum)
{
  uint8_t result = USBD_OK;
  /* USER CODE BEGIN 13 */
  UNUSED(Buf);
  UNUSED(Len);
  UNUSED(epnum);
  /* USER CODE END 13 */
  return result;
}

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */

/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */

/**
  * @}
  */

/**
  * @}
  */
