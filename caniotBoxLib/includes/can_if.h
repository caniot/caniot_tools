#ifdef __cplusplus
extern "C" {
#endif
/**************************************************************************************************************************
* @name       CanIf_send_sync
* @brief      send passing data over can bus
*
* @attention 1. This API should be called after caniotBox_init
* @param     - channel: 0 or 1
*            - can_id : specify the tx message id 
*            - data : buffer pointer contents the data
*            - datalength :data length to be send
*            - cycle_time_ms :cycle time in milliseconds, (-1 for single shot message, 0ms to stop the cyclic message again)
*
* @return    - ESP_OK: succeed
*            - ESP_FAIL: failed
***************************************************************************************************************************/
extern esp_err_t CanIf_send_sync(uint8_t channel, uint32_t can_id, uint8_t *data,uint8_t datalength,int32_t cycle_time_ms);
/***************************************************************************
* @name      CanIf_attach_preSendFnc
* @brief     configure presend  handler to  be called 
*            every time a CAN message with the given ID is scheduled to be sent.
*
* @attention 1. This API should be called after caniotBox_init
* @param     - channel: 0 or 1
*            - can_id : specify the message id used on pre send   
*            - preSendFnc : application function to  be call on pre send of can_id
*
* @return    - ESP_OK: succeed
*            - ESP_FAIL: failed
*****************************************************************************/
extern esp_err_t CanIf_attach_preSendFnc(uint8_t channel, uint32_t can_id, void (*preSendFnc)(uint8_t dataLength, uint8_t *data));
/***************************************************************************
* @name      CanIf_attach_receive
* @brief     configure receives handler to  be called on receive of specified id
*
* @attention 1. This API should be called after caniotBox_init
* @param     - channel: 0 or 1
*            - can_id : specify the message id used on receive   
*            - onMessageReceive : application function to  be call on receive of can_id
*
* @return    - ESP_OK: succeed
*            - ESP_FAIL: failed
*****************************************************************************/
extern esp_err_t CanIf_attach_receive(uint8_t channel, uint32_t can_id, void (*onMessageReceive)(uint8_t dataLength, uint8_t *data));
/***************************************************************************
* @name      CanIsoTpIf_configure
* @brief     init can iso tp protocol, this function should be called at first before using the iso tp protocol.
*
* @attention 1. This API should be called after caniotBox_init
* @param     - channel: 0 or 1
*            - handler_id: 0...7 : handle id or transport descriptor (max is 7)
*            - can_tx_id : specify the message id used on transmit 
*            - can_rx_id,specify the message id used on receive  
*            - onMessageReceive_ application function to  be call on transmit or receive COMPLETED
*            - isFDFrame : true: support can fd transport specification with  upto 64bytes on single frame(only availabe on channel 1)
*
* @return    - ESP_OK: succeed
*            - ESP_FAIL: failed
*
*****************************************************************************/
extern esp_err_t CanIsoTpIf_configure(uint8_t channel, uint16_t handler_id, uint32_t can_tx_id, uint32_t can_rx_id, void (*onMessageReceive)(uint16_t handler_id, uint16_t DataLength, uint8_t *data,int is_Tx_Rx_finished), bool isFDFrame);
/***************************************************************************
* @name      CanIsoTpIf_configure_StMin 
* @brief     call this funtction after CanIsoTpIf_configure to The minimum separation time sent in the flow control message. 
*            Indicates the amount of time to wait between 2 consecutive frame. 
*            Values from 1 to 127 means milliseconds. 
*            Values from 0xF1 to 0xF9 means 100us to 900us. 0 Means no timing requirements.
*
* @attention 1. This API should be called after caniotBox_init
* @param     - handler_id: 0...7 : handle id or transport descriptor (max is 7)
*            - StMin : Values from 1 to 127 means milliseconds. 
*                      Values from 0xF1 to 0xF9 means 100us to 900us.
*                      0 Means no timing requirements.
* @return    - ESP_OK: succeed
*            - ESP_FAIL: failed
*
*****************************************************************************/
extern esp_err_t CanIsoTpIf_configure_StMin(uint16_t EN_IsoTp_Handler, uint8_t StMin);
/***************************************************************************
* @name      CanIsoTpIf_send
* @brief     send passing data over transport protocol
*
* @attention 1. This API should be called after caniotBox_init
* @param     - handler_id: 0...7 : handle id or transport descriptor (max is 7)
*            - uint16_dataLe : data lenght 
*            - Puint8_t_BufferAddress : buffer pointer to the data
*
* @return    - ESP_OK: succeed
*            - ESP_FAIL: failed
*
*****************************************************************************/
extern esp_err_t CanIsoTpIf_send(uint16_t handler_id, uint16_t uint16_dataLe, uint8_t *Puint8_t_BufferAddress);
#ifdef __cplusplus
}
#endif