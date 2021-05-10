/*
 * bt_spp.c
 *
 *  Created on: 2021. 5. 8.
 *      Author: baram
 */


#include "bt_spp.h"
#include "btstack/btstack_port.h"
#include "gpio.h"
#include "qbuffer.h"


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "btstack.h"


#define HEARTBEAT_PERIOD_MS   500
#define TEST_COD              0x1234
#define RFCOMM_SERVER_CHANNEL 1


static void packet_handler (uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
static void spp_service_setup(void);

static bool is_init = false;
static bool is_open = false;
static uint16_t rfcomm_channel_id;
static uint16_t rfcomm_channel_mtu;
static uint8_t  spp_service_buffer[150];
static btstack_packet_callback_registration_t hci_event_callback_registration;


static uint8_t tx_buf[1024];
static uint8_t rx_buf[1024];
static uint8_t wr_buf[1024];

static qbuffer_t q_rx;
static qbuffer_t q_tx;




bool btSppInit(void)
{
  gpioPinWrite(_PIN_GPIO_BT_RST_N, _DEF_LOW);
  delay(10);
  gpioPinWrite(_PIN_GPIO_BT_RST_N, _DEF_HIGH);
  delay(50);


  qbufferCreate(&q_rx, rx_buf, 1024);
  qbufferCreate(&q_tx, tx_buf, 1024);


  btstackPortInit();
  spp_service_setup();

  gap_set_local_name("ART-Pi SPP 00:00:00:00:00:00");
  gap_discoverable_control(1);

  // short-cut to find other SPP Streamer
  gap_set_class_of_device(TEST_COD);

  // turn on!
  hci_power_control(HCI_POWER_ON);

  is_init = true;
  return true;
}

bool btSppIsOpen(void)
{
  return is_open;
}

bool btSppExcute(void)
{

  btstackPortExecute();

  return true;
}

uint32_t btSppAvailable(void)
{
  return qbufferAvailable(&q_rx);
}

uint8_t  btSppRead(void)
{
  uint8_t ret = 0;

  qbufferRead(&q_rx, &ret, 1);

  return ret;
}

uint32_t btSppWrite(uint8_t *p_data, uint32_t length)
{
  uint32_t ret;
  uint32_t tx_buf_len;
  uint32_t pre_time;

  if (is_open != true)
  {
    return 0;
  }

  pre_time = millis();
  while(pre_time < 100)
  {
    tx_buf_len = (q_tx.len - qbufferAvailable(&q_tx)) - 1;
    if (tx_buf_len >= length)
    {
      break;
    }
    delay(1);
  }

  if (qbufferWrite(&q_tx, p_data, length) == true)
  {
    ret = length;
  }
  else
  {
    ret = 0;
  }

  return ret;
}

void spp_service_setup(void)
{
  // register for HCI events
  hci_event_callback_registration.callback = &packet_handler;
  hci_add_event_handler(&hci_event_callback_registration);

  // init L2CAP
  l2cap_init();

  // init RFCOMM
  rfcomm_init();
  // reserved channel, mtu limited by l2cap, 1 credit
  rfcomm_register_service(&packet_handler, RFCOMM_SERVER_CHANNEL, 0xffff);

  // init SDP, create record for SPP and register with SDP
  sdp_init();
  memset(spp_service_buffer, 0, sizeof(spp_service_buffer));
  spp_create_sdp_record(spp_service_buffer, 0x10001, 1, "SPP Counter");
  sdp_register_service(spp_service_buffer);
  printf("SDP service buffer size: %u\n\r", (uint16_t) de_get_len(spp_service_buffer));
}

void packet_handler (uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
  UNUSED(channel);

  bd_addr_t event_addr;
  uint8_t   rfcomm_channel_nr;
  uint32_t tx_len;

  switch (packet_type)
  {
    case HCI_EVENT_PACKET:
      switch (hci_event_packet_get_type(packet))
      {
        case HCI_EVENT_COMMAND_COMPLETE:
          if (HCI_EVENT_IS_COMMAND_COMPLETE(packet, hci_read_bd_addr)){
              reverse_bd_addr(&packet[6], event_addr);
              printf("BD-ADDR: %s\n\r", bd_addr_to_str(event_addr));
              break;
          }
          break;

        case HCI_EVENT_PIN_CODE_REQUEST:
          // inform about pin code request
          printf("Pin code request - using '0000'\n");
          hci_event_pin_code_request_get_bd_addr(packet, event_addr);
          gap_pin_code_response(event_addr, "0000");
          break;

        case RFCOMM_EVENT_INCOMING_CONNECTION:
          // data: event (8), len(8), address(48), channel (8), rfcomm_cid (16)
          rfcomm_event_incoming_connection_get_bd_addr(packet, event_addr);
          rfcomm_channel_nr = rfcomm_event_incoming_connection_get_server_channel(packet);
          rfcomm_channel_id = rfcomm_event_incoming_connection_get_rfcomm_cid(packet);
          printf("RFCOMM channel %u requested for %s\n", rfcomm_channel_nr, bd_addr_to_str(event_addr));
          rfcomm_accept_connection(rfcomm_channel_id);
          break;

        case RFCOMM_EVENT_CHANNEL_OPENED:
          // data: event(8), len(8), status (8), address (48), server channel(8), rfcomm_cid(16), max frame size(16)
          if (rfcomm_event_channel_opened_get_status(packet)) {
              printf("RFCOMM channel open failed, status %u\n", rfcomm_event_channel_opened_get_status(packet));
          } else {
              rfcomm_channel_id = rfcomm_event_channel_opened_get_rfcomm_cid(packet);
              rfcomm_channel_mtu = rfcomm_event_channel_opened_get_max_frame_size(packet);
              is_open = true;
              printf("RFCOMM channel open succeeded. New RFCOMM Channel ID %u, max frame size %u\n", rfcomm_channel_id, rfcomm_channel_mtu);
          }
          break;

        case RFCOMM_EVENT_CHANNEL_CLOSED:
          rfcomm_channel_id = 0;
          is_open = false;
          break;

        case RFCOMM_EVENT_CAN_SEND_NOW:
          tx_len = qbufferAvailable(&q_tx);
          if (tx_len > rfcomm_channel_mtu)
          {
            tx_len = rfcomm_channel_mtu;
          }
          if (tx_len > 0)
          {
            qbufferRead(&q_tx, &wr_buf[0], tx_len);
            rfcomm_send(rfcomm_channel_id, wr_buf, tx_len);
          }
          else
          {
            rfcomm_send(rfcomm_channel_id, NULL, 0);
          }
          rfcomm_request_can_send_now_event(rfcomm_channel_id);
          break;

        default:
          break;
      }
      break;

    case RFCOMM_DATA_PACKET:
        qbufferWrite(&q_rx, &packet[0], size);
        break;

    default:
        break;
  }
}







