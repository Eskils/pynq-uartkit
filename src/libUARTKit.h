#include <libpynq.h>
#include <stdlib.h>
#include <stdio.h>
#include <glib-2.0/glib.h>
#include <pthread.h>
#include <string.h>

typedef struct uart_handle_t {
    char *client_name;
    GHashTable *listener_registrations;
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

extern void uartkit_init(uart_handle_t *uart_handle, char *client_name);
extern void uartkit_send(uart_handle_t *uart_handle, char *to_client_name, uint8_t *data, int size);
extern void uartkit_add_listener(uart_handle_t *uart_handle, uart_receive_handler receive_handler);
extern void uartkit_remove_listener(uart_handle_t *uart_handle, uart_receive_handler receive_handler);
extern void uartkit_destroy(uart_handle_t *uart_handle);