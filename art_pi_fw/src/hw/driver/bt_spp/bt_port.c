/*
 * bt_port.c
 *
 *  Created on: 2021. 5. 8.
 *      Author: baram
 */



#include "bt_port.h"
#include "uart.h"

#include "btstack.h"
#include "btstack_run_loop_embedded.h"


static uint8_t uart_ch = _DEF_UART2;


// rx state
static int  bytes_to_read = 0;
static uint8_t * rx_buffer_ptr = 0;

// tx state
static int bytes_to_write = 0;
static uint8_t * tx_buffer_ptr = 0;

static void dummy_handler(void) {};
static void (*rx_done_handler)(void) = dummy_handler;
static void (*tx_done_handler)(void) = dummy_handler;


static hci_transport_config_uart_t config =
  {
      HCI_TRANSPORT_CONFIG_UART,
      115200,
      115200,
      0, // flow control
      "ART-Pi",
  };


void btPortInit(void)
{
  // setup BTstack memory pools
  btstack_memory_init();

  // select embedded run loop
  btstack_run_loop_init(btstack_run_loop_embedded_get_instance());

  // use logger: format HCI_DUMP_PACKETLOGGER, HCI_DUMP_BLUEZ or HCI_DUMP_STDOUT
  //hci_dump_open(NULL, HCI_DUMP_STDOUT);

  // init HCI
  hci_transport_t  *transport = (hci_transport_t  *)hci_transport_h4_instance(btstack_uart_block_embedded_instance());
  hci_init(transport, &config);

  // setup Link Key DB
  hci_set_link_key_db(btstack_link_key_db_memory_instance());
}


void btPortExecute(void)
{
  int rx_avail;
  int num_rx_bytes;
  //int tx_avail;
  int rx_bytes;
  int tx_bytes;
  //int ret;

  while (bytes_to_read)
  {
    rx_avail = uartAvailable(uart_ch);
    if (!rx_avail)
      break;

    if (bytes_to_read > rx_avail)
      num_rx_bytes = rx_avail;
    else
      num_rx_bytes = bytes_to_read;

    for (int i=0; i<num_rx_bytes; i++)
    {
      rx_buffer_ptr[i] = uartRead(uart_ch);
    }
    rx_bytes = num_rx_bytes;

    rx_buffer_ptr += rx_bytes;
    bytes_to_read -= rx_bytes;

    if (bytes_to_read < 0)
    {
      bytes_to_read = 0;
    }

    if (bytes_to_read == 0)
    {
      (*rx_done_handler)();
    }
  }

  while (bytes_to_write)
  {
#if 0
    tx_avail = UART_NumWriteAvail(MXC_UART_GET_UART(CC256X_UART_ID));
    if (!tx_avail)
      break;

    if (bytes_to_write > tx_avail)
      tx_bytes = tx_avail;
    else
      tx_bytes = bytes_to_write;

    ret = UART_Write(MXC_UART_GET_UART(CC256X_UART_ID), tx_buffer_ptr, tx_bytes);
    if (ret < 0)
      break;
#else
    tx_bytes = bytes_to_write;
    uartWrite(uart_ch, tx_buffer_ptr, tx_bytes);
#endif
    bytes_to_write -= tx_bytes;
    tx_buffer_ptr += tx_bytes;
    if (bytes_to_write < 0)
    {
      bytes_to_write = 0;
    }

    if (bytes_to_write == 0)
    {
      (*tx_done_handler)();
    }
  }

  btstack_run_loop_embedded_execute_once();
}


uint32_t hal_time_ms(void)
{
  return millis();
}

void hal_cpu_disable_irqs(void)
{
}

void hal_cpu_enable_irqs(void)
{
}

void hal_cpu_enable_irqs_and_sleep(void)
{
}

void hal_uart_dma_set_csr_irq_handler( void (*csr_irq_handler)(void))
{
}

void hal_uart_dma_set_sleep(uint8_t sleep)
{

}

void hal_uart_dma_init(void)
{
  uartOpen(uart_ch, 115200);
}

void hal_uart_dma_set_block_received(void (*block_handler)(void))
{
  rx_done_handler = block_handler;
}

void hal_uart_dma_set_block_sent(void (*block_handler)(void))
{
  tx_done_handler = block_handler;
}

int  hal_uart_dma_set_baud(uint32_t baud)
{
  uartOpen(uart_ch, baud);
  logPrintf("uartOpen : %d bps\n", baud);
  return 0;
}

void hal_uart_dma_send_block(const uint8_t *buffer, uint16_t len)
{
  tx_buffer_ptr = (uint8_t *)buffer;
  bytes_to_write = len;
}

void hal_uart_dma_receive_block(uint8_t *buffer, uint16_t len)
{
  rx_buffer_ptr = buffer;
  bytes_to_read = len;
}




