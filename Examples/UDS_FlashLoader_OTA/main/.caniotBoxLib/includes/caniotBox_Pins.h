/* Copyright c CaniotBox. All Rights Reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>. *
 */

#ifndef CANIOTBOXPIN_H_
#define CANIOTBOXPIN_H_


#if CONFIG_IDF_TARGET_ESP32/*caniot*/
#define APPL_UART_CONST 2000000
#define HW_VERSION 0x20
/*define application GPIO*/

#define LCD_PIN_NUM_MISO           GPIO_NUM_19 // SPI MISO
#define LCD_PIN_NUM_MOSI           GPIO_NUM_23 // SPI MOSI
#define LCD_PIN_NUM_CLK            GPIO_NUM_18   // SPI CLOCK pin
#define LCD_PIN_NUM_CS             GPIO_NUM_5     // Display CS pin
#define LCD_PIN_NUM_DC             GPIO_NUM_2    // Display command/data pin
#define LCD_PIN_NUM_TCS            GPIO_NUM_21  // Touch screen CS pin NOT used if USE_TOUCH=0
#define LCD_PIN_NUM_RST            GPIO_NUM_15 // GPIO used for RESET control #16
#define LCD_PIN_NUM_BCKL           GPIO_NUM_15 // GPIO used for backlight control
#define LCD_PIN_BCKL_ON            GPIO_NUM_15  // GPIO value for backlight ON
#define LCD_PIN_BCKL_OFF           GPIO_NUM_15 // GPIO value for backlight OFF


#define LCD_PIN_NUM_SD_CS          GPIO_NUM_4  //value for SD chip select
#define LCD_PIN_NUM_SD_CD          -1 // GPIO value for SD card detect

#define CAN0_TX_GPIO_NUM             GPIO_NUM_27
#define CAN0_RX_GPIO_NUM             GPIO_NUM_22
#define CAN0_BUS_OFF_IND             -1

#define CAN1_PIN_NUM_CANFD_MISO   GPIO_NUM_12
#define CAN1_PIN_NUM_CANFD_MOSI   GPIO_NUM_13
#define CAN1_PIN_NUM_CANFD_CLK    GPIO_NUM_14
#define CAN1_PIN_NUM_CANFD_CS     GPIO_NUM_0// GPIO value for CANFD chip25

#define LIN_TXD_PIN                 GPIO_NUM_33           // GPIO used LIN TX
#define LIN_RXD_PIN                 GPIO_NUM_27           // GPIO value LIN RX
#define LIN_NOT_SLEEP_PIN           GPIO_NUM_25 // LIN Sleep not Ssleep by high

#define KLINE_TXD_PIN                 GPIO_NUM_26           // GPIO used KLINE TX
#define KLINE_RXD_PIN                 GPIO_NUM_34           // GPIO value KLINE RX
#define KLINE_NOT_SLEEP_PIN           GPIO_NUM_32 // KLINELIN Sleep not Sleep by high

#define RMT_RX_GPIO_NUM             GPIO_NUM_34              /*!< GPIO_NUM_2 GPIO number for receiver */
#define APPL_BAT_ADC_INPUT          GPIO_NUM_36          /*!< GPIO number for batterie voltage measurement input*/
#define APPL_BAT_CHARGE_STATE_INPUT GPIO_NUM_39             /*!< GPIO number for batterie charge status input from batterie charger IC's*/

#elif  CONFIG_IDF_TARGET_ESP32S3/*caniot Pro*/

/*define application GPIO*/

#define LCD_PIN_NUM_MISO           GPIO_NUM_19 // SPI MISO
#define LCD_PIN_NUM_MOSI           GPIO_NUM_23 // SPI MOSI
#define LCD_PIN_NUM_CLK            GPIO_NUM_18   // SPI CLOCK pin
#define LCD_PIN_NUM_CS             GPIO_NUM_5     // Display CS pin
#define LCD_PIN_NUM_DC             GPIO_NUM_2    // Display command/data pin
#define LCD_PIN_NUM_TCS            GPIO_NUM_21  // Touch screen CS pin NOT used if USE_TOUCH=0
#define LCD_PIN_NUM_RST            GPIO_NUM_15 // GPIO used for RESET control #16
#define LCD_PIN_NUM_BCKL           GPIO_NUM_15 // GPIO used for backlight control
#define LCD_PIN_BCKL_ON            GPIO_NUM_15  // GPIO value for backlight ON
#define LCD_PIN_BCKL_OFF           GPIO_NUM_15 // GPIO value for backlight OFF

/*sdcard*/
#define LCD_PIN_NUM_SD_CS          GPIO_NUM_4  //value for SD chip select
#define LCD_PIN_NUM_SD_CD          -1 // GPIO value for SD card detect

#define CAN0_TX_GPIO_NUM             GPIO_NUM_16
#define CAN0_RX_GPIO_NUM             GPIO_NUM_8
#define CAN0_BUS_OFF_IND             -1

#define CAN1_PIN_NUM_CANFD_MISO   GPIO_NUM_12
#define CAN1_PIN_NUM_CANFD_MOSI   GPIO_NUM_13
#define CAN1_PIN_NUM_CANFD_CLK    GPIO_NUM_14
#define CAN1_PIN_NUM_CANFD_CS     GPIO_NUM_0// GPIO value for CANFD chip25

#define LIN_TXD_PIN                 GPIO_NUM_17           // GPIO used LIN TX
#define LIN_RXD_PIN                 GPIO_NUM_18           // GPIO value LIN RX
#define LIN_NOT_SLEEP_PIN           GPIO_NUM_46 // LIN Sleep not Ssleep by high

#define KLINE_TXD_PIN                 GPIO_NUM_6           // GPIO used KLINE TX
#define KLINE_RXD_PIN                 GPIO_NUM_7          // GPIO value KLINE RX
#define KLINE_NOT_SLEEP_PIN           GPIO_NUM_3 // KLINELIN Sleep not Ssleep by high


#define PMOD0_GPIO_NUM             GPIO_NUM_42             
#define PMOD1_GPIO_NUM             GPIO_NUM_47             
#define PMOD2_GPIO_NUM             GPIO_NUM_41             
#define PMOD3_GPIO_NUM             GPIO_NUM_48             
#define PMOD4_GPIO_NUM             GPIO_NUM_40             
#define PMOD5_GPIO_NUM             GPIO_NUM_45             
#define PMOD6_GPIO_NUM             GPIO_NUM_39             
#define PMOD7_GPIO_NUM             GPIO_NUM_38             
#define PMOD8_GPIO_NUM             GPIO_NUM_1             


#endif /*Board*/
#endif /*  */
