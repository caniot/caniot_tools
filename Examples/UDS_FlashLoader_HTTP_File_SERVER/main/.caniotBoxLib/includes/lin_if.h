#ifdef __cplusplus
extern "C" {
#endif
#include "driver/uart.h"
typedef enum
{
    EN_NO_MSG,       /**< no message */
    EN_IN_PROGRESS,  /**< in progress */
    EN_COMPLETED,    /**< completed */
    EN_FAILED,       /**< failed */
    EN_N_AS_TIMEOUT, /**< N_As timeout */
    EN_N_CR_TIMEOUT, /**< N_Cr timeout */
    EN_WRONG_SN      /**< wrong sequence number */
} isoTp_lin_message_status;
/***************************************************************************
* @name      LinIf_uart_deinit
* @brief     Uninstall UART driver . 
*
* @attention 1. This API should be called after caniotBox_init
* @param     void
*
* @return    void
*****************************************************************************/
extern void LinIf_uart_deinit();
/***************************************************************************
* @name      LinIf_uart_init
* @brief     Install UART LIN driver and set the UART to the given configuration. 
*
* @attention 1. This API should be called after caniotBox_init and after LinIf_uart_deinit
* @param     - uart_config_t : UART configuration parameters (see uart_types.h)
*            - tx_buffer_size UART TX ring buffer size.
*               If set to zero, driver will not use TX buffer, TX function will block task until all data have been sent out.
*            - set to true if LIN bit time measurements are needed
* @return    uart_port_t UART port number, can be UART_NUM_0 ~ (UART_NUM_MAX -1)
*****************************************************************************/
extern uart_port_t LinIf_uart_init(uart_config_t* uart_config,int tx_buffer_size, bool enable_bittime_measurement);
/***************************************************************************
* @name      LinIf_uart_deinit
* @brief     Uninstall UART driver . 
*
* @attention 1. This API should be called after caniotBox_init
* @param     void
*
* @return    void
*****************************************************************************/
extern void LinIf_uart_deinit();
/***************************************************************************
* @name      LinIf_onReceive
* @brief     configure receives handler to  be call on receive of specified LIN id 
*
* @attention 1. This API should be called after caniotBox_init
* @param     - lin_id : specify the message id used on receive   
*            - onMessageReceive : application function to  be call on receive of lin_id
*
* @return    void
*****************************************************************************/
extern void LinIf_onReceive(uint32_t lin_id, void (*onMessageReceive)(uint8_t dlc, uint8_t *data));
/***************************************************************************
* @name      LinIf_update_frame_data
* @brief      update lin frame data for  specified LIN id  
*
* @attention 1. This API should be called after caniotBox_init
* @param     - lin_id : specify the message id  
*            - inputData : pointer to the new data
*
* @return    void
*****************************************************************************/
extern void LinIf_update_frame_data(uint8_t id,  uint8_t *inputData);
/***************************************************************************
* @name      LinIf_restart_scheduler
* @brief     This function starts the internal scheduler, which begins a cyclical traversal of a last or first configured schedule table. 
*
* @attention 1. This API should be called after caniotBox_init
* @param     void
*
* @return    void
*****************************************************************************/
extern void LinIf_restart_scheduler();
/***************************************************************************
* @name      LinIf_set_scheduler_sync
* @brief     This function starts the given internal scheduler from the given entry.  
*
* @attention 1. This API should be called after caniotBox_init
* @param     scheduler :  scheduler index to be started
*            entry:  scheduler entry index used for the start
*
* @return    void
*****************************************************************************/
extern void LinIf_set_scheduler_sync(uint8_t scheduler, uint8_t entry);

/* ISO TO Interfaces*/

/***************************************************************************
* @name      LinIf_TpReceiveSync
* @brief     wait for ms_receiveTimeOut to receive  diagnostic data with the given NAD handle. 
*            The number of bytes received is stored in length.
*
* @attention 1. This API should be called after caniotBox_init
* @param     handle : handle created by LinIf_TpTransmitSync
*            NAD : pointer to save the Slave NAD
*            data_length : pointer for The number of bytes received (max 4096)
*            data : pointer to store the diagnostic data received. 
*
* @return    void
*****************************************************************************/
extern void LinIf_TpReceiveSync(int32_t handle,uint8_t *NAD, uint16_t *length, uint8_t *data, uint32_t ms_receiveTimeOut);
/***************************************************************************
* @name      CanIf_attach_receive
* @brief     send diagnostic data with the given data length for the given NAD slave over Iso tp protocol.
*
* @attention 1. This API should be called after caniotBox_init
* @param     handle : handle  associated to this request: if -1 then no request can be send 
*            NAD :  specify the Slave NAD
*            data_length : specify the data length(max 4096)
*            data : pointer to the diagnostic data       
* @return    void
*****************************************************************************/
extern void LinIf_TpTransmitSync(int32_t *handle,uint8_t NAD, uint16_t data_length,const uint8_t * data);
/***************************************************************************
* @name      LinIf_Tp_tx_status
* @brief     get transmit isotp status, call this fucntion after LinIf_TpTransmitSync
*            to check either the last iso tp request has been sent.
*
* @attention 1. This API should be called after caniotBox_init
* @param     isoTp_lin_message_status : pointer to the enum too store the current status
*
* @return    void
*****************************************************************************/
extern void LinIf_Tp_tx_status(isoTp_lin_message_status *message_status);
/***************************************************************************
* @name      LinIf_Tp_rx_status
* @brief     get receive isotp status.
*
* @attention 1. This API should be called after caniotBox_init
* @param     isoTp_lin_message_status : pointer to the enum too store the current status
*
* @return    void
*****************************************************************************/
extern void LinIf_Tp_rx_status(isoTp_lin_message_status *message_status);
/***************************************************************************
* @name      LinIf_Tp_set_rx_ready
* @brief     set receive isotp status to ready.
*
* @attention 1. This API should be called after caniotBox_init
* @param     void
*
* @return    void
*****************************************************************************/
extern void LinIf_Tp_set_rx_ready(void);
/***************************************************************************
* @name      LinIf_TpResetReceiveSync
* @brief     reset receive queue before start new receive request.
*
* @attention 1. This API should be called after caniotBox_init
* @param     void
*
* @return    void
*****************************************************************************/
extern void LinIf_TpResetReceiveSync(void);
/***************************************************************************
* @name      LinIf_send_diagnose_header
* @brief     schedule diagnose request header with the ID 0x3D from the master.
*
* @attention 1. This API should be called after caniotBox_init
* @param      handle : handle  associated to this request: if -1 then no request can be send 
*
* @return    void
*****************************************************************************/
void LinIf_send_diagnose_header(int32_t *handle);
#ifdef __cplusplus
}
#endif