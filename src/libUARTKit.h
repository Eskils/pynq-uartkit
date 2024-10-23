#include <libpynq.h>
#include <stdlib.h>
#include <stdio.h>
#include <glib-2.0/glib.h>
#include <pthread.h>
#include <string.h>

typedef struct uart_handle_t {
    char *client_name;
    GHashTable *listener_registrations;
    int verbose;
} uart_handle_t;

typedef struct uart_header_t {
    char *to_client_name;
    char *from_client_name;
    int content_length;
} uart_header_t;

typedef struct uart_message_t {
    uart_header_t *header;
    void *data;
} uart_message_t;

typedef void (*uart_receive_handler)(uart_message_t);

/**
*   Initialize a new instance for sending and receiving messages.
*   Also initializes UART0 from libPYNQ and resets the FIFO buffers.
*   You need to specify which pins to use for UART yourself.
*
*   Remember to call `uartkit_destroy` to clean up.
*/
extern void uartkit_init(uart_handle_t *uart_handle, char *client_name);

/**
*   Send a message to the client with the specified name.
*/
extern void uartkit_send(uart_handle_t *uart_handle, char *to_client_name, uint8_t *data, size_t size);

/**
*   Register a listener for messages.
*   You specify a callback function which takes a type of `uart_message`.
*   You can only add a single listener per callback funcition.
*
*   Remember to call `uartkit_remove_listener` to clean up.
*/
extern void uartkit_add_listener(uart_handle_t *uart_handle, uart_receive_handler receive_handler);

/**
*   Removes a listener for messages.
*/
extern void uartkit_remove_listener(uart_handle_t *uart_handle, uart_receive_handler receive_handler);

/**
*   Destroys the instance for sending and receiving messages.
*   Also deinitializes the UART0-channel from libPYNQ.
*/
extern void uartkit_destroy(uart_handle_t *uart_handle);