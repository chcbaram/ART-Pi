/*
 * bt_spp.c
 *
 *  Created on: 2021. 5. 8.
 *      Author: baram
 */


#include "bt_spp.h"
#include "bt_spp/bt_port.h"




#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "btstack.h"


#define HEARTBEAT_PERIOD_MS 500
#define TEST_COD 0x1234
#define RFCOMM_SERVER_CHANNEL 1

static void packet_handler (uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
static void spp_service_setup(void);

static uint16_t rfcomm_channel_id;
static uint8_t  spp_service_buffer[150];
static btstack_packet_callback_registration_t hci_event_callback_registration;



bool btSppInit(void)
{
  btPortInit();

  spp_service_setup();

  puts("SPP FlowControl Demo: simulates processing on received data...\n\r");
  gap_set_local_name("ART-Pi SPP");
  gap_discoverable_control(1);

  // short-cut to find other SPP Streamer
  gap_set_class_of_device(TEST_COD);

  // turn on!
  hci_power_control(HCI_POWER_ON);

  return true;
}

bool btSppExcute(void)
{

  btPortExecute();

  return true;
}

/* @section SPP Service Setup
 *
 * @text Listing explicitFlowControl shows how to
 * provide one initial credit during RFCOMM service initialization. Please note
 * that providing a single credit effectively reduces the credit-based (sliding
 * window) flow control to a stop-and-wait flow control that limits the data
 * throughput substantially.
 */

/* LISTING_START(explicitFlowControl): Providing one initial credit during RFCOMM service initialization */
static void spp_service_setup(void)
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
/* LISTING_END */

/* @section Periodic Timer Setup
 *
 * @text Explicit credit management is
 * recommended when received RFCOMM data cannot be processed immediately. In this
 * example, delayed processing of received data is simulated with the help of a
 * periodic timer as follows. When the packet handler receives a data packet, it
 * does not provide a new credit, it sets a flag instead, see Listing phManual.
 * If the flag is set, a new
 * credit will be granted by the heartbeat handler, introducing a delay of up to 1
 * second. The heartbeat handler code is shown in Listing hbhManual.
 */


/* LISTING_START(phManual): Packet handler with manual credit management */
// Bluetooth logic
static void packet_handler (uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
/* LISTING_PAUSE */
    UNUSED(channel);

    bd_addr_t event_addr;
    uint8_t   rfcomm_channel_nr;
    uint16_t  mtu;
    int i;

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
                        mtu = rfcomm_event_channel_opened_get_max_frame_size(packet);
                        printf("RFCOMM channel open succeeded. New RFCOMM Channel ID %u, max frame size %u\n", rfcomm_channel_id, mtu);
                    }
                    break;

                case RFCOMM_EVENT_CHANNEL_CLOSED:
                    rfcomm_channel_id = 0;
                    break;

                default:
                    break;
            }
            break;

/* LISTING_RESUME */
        case RFCOMM_DATA_PACKET:
            for (i=0;i<size;i++)
            {
              putchar(packet[i]);
            };
            putchar('\n');
            break;

/* LISTING_PAUSE */
        default:
            break;
    }
/* LISTING_RESUME */
}
/* LISTING_END */






