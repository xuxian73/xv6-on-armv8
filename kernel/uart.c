// driver for ARM PrimeCell UART (PL011)
#include "types.h"
#include "defs.h"
#include "param.h"
#include "arm.h"
#include "memlayout.h"
#include "spinlock.h"

static volatile uint *uart_base;
extern volatile int panicked;
void isr_uart ();

/*  if the FIFOs are enabled, data written to this 
    location is pushed onto the transmit FIFO
    if the FIFOs are not enabled, data is stored 
    in the transmitter holding register 
    (the bottom word of the transmit FIFO).*/
#define UART_DR		0	// data register
#define UART_RSR	1	// receive status register/error clear register
#define UART_FR		6	// flag register
#define	UART_IBRD	9	// integer baud rate register
#define UART_FBRD	10	// Fractional baud rate register
#define UART_LCR	11	// line control register
#define UART_CR		12	// control register
#define UART_IMSC	14	// interrupt mask set/clear register
#define UART_MIS	16	// masked interrupt status register
#define	UART_ICR	17	// interrupt clear register
// bits in registers
#define UARTFR_TXFF	(1 << 5)	// tramit FIFO full
#define UARTFR_RXFE	(1 << 4)	// receive FIFO empty
#define	UARTCR_RXE	(1 << 9)	// enable receive
#define UARTCR_TXE	(1 << 8)	// enable transmit
#define	UARTCR_EN	(1 << 0)	// enable UART
#define UARTLCR_FEN	(1 << 4)	// enable FIFO
#define UART_RXI	(1 << 4)	// receive interrupt
#define UART_TXI	(1 << 5)	// transmit interrupt
#define UART_BITRATE 19200

// the transmit output buffer
struct spinlock uart_tx_lock;
#define UART_TX_BUF_SIZE 32
char uart_tx_buf[UART_TX_BUF_SIZE];
int uart_tx_w;  // write next to uart_tx_buf[uart_tx_w++]
int uart_tx_r;  // read next from uart_tx_buf[uart_tx_r++]

void uartstart();

// enable uart
void uart_init (void *addr)
{
    uint left;

    uart_base = addr;

    // set the bit rate: integer/fractional baud rate registers
    uart_base[UART_IBRD] = UART_CLK / (16 * UART_BITRATE);

    left = UART_CLK % (16 * UART_BITRATE);
    uart_base[UART_FBRD] = (left * 4 + UART_BITRATE / 2) / UART_BITRATE;

    // enable trasmit and receive
    uart_base[UART_CR] |= (UARTCR_EN | UARTCR_RXE | UARTCR_TXE);

    // enable FIFO
    uart_base[UART_LCR] |= UARTLCR_FEN;

    initlock(&uart_tx_lock, "uart");
}

// enable the receive (interrupt) for uart (after GIC has initialized)
void uart_enable_rx ()
{
    uart_base[UART_IMSC] = UART_RXI;
    gic_set(PIC_UART0 + 32, isr_uart);
}

void uartputc (int c)
{
    acquire(&uart_tx_lock);

    if (panicked) {
        for (;;)
        ;
    }
    
    while(1) {
        if((uart_tx_w + 1) % UART_TX_BUF_SIZE == uart_tx_r) {
            // buffer is full
            // wait for uartstart() to open up space in the buffer
            sleep(&uart_tx_r, &uart_tx_lock);
        } else {
            uart_tx_buf[uart_tx_w] = c;
            uart_tx_w = (uart_tx_w + 1) % UART_TX_BUF_SIZE;
            uartstart();
            release(&uart_tx_lock);
            return;
        }
    }
}

void uartputc_sync(int c)
{
    push_off();
    if(panicked){
        for(;;)
        ;
    }
    while (uart_base[UART_FR] & UARTFR_TXFF)
        ;
    
    uart_base[UART_DR] = c;

    pop_off();
}

// if the UART is idle and a character is waiting
// in the transmit buffer, send it
// caller must hold uart_tx_lock
// called form both the top- and bottom-half
void
uartstart()
{
    while(1) {
        if(uart_tx_w == uart_tx_r) {
            // empty
            return;
        }

        if (uart_base[UART_FR] & UARTFR_TXFF) {
            // the UART transmit holding register is full,
            // so we cannot give it another byte.
            // it will interrupt when it's ready for a new byte.
            return;
        }

        int c = uart_tx_buf[uart_tx_r];
        uart_tx_r = (uart_tx_r + 1) % UART_TX_BUF_SIZE;

        wakeup(&uart_tx_r);

        uart_base[UART_DR] = c;
    }
}

//poll the UART for data
int uartgetc (void)
{
    if (uart_base[UART_FR] & UARTFR_RXFE) {
        return -1;
    }

    return uart_base[UART_DR];
}

void isr_uart ()
{
    if (uart_base[UART_MIS] & UART_RXI) {
        while(1){
            int c = uartgetc();
            if(c == -1)
                break;
            consoleintr(c);
        }
    }

    // send buffered characters.
    acquire(&uart_tx_lock);
    uartstart();
    release(&uart_tx_lock);

    // clear the interrupt
    uart_base[UART_ICR] = UART_RXI | UART_TXI;
}
