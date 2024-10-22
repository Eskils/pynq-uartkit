#include "libUARTKit.h"

#define RECIEVE_BUFFER_SIZE 1024

void uartkit_send_header(char *headerName, char *headerValue);
void uartkit_send_text(char *text);

typedef struct uartkit_listener_context_t {
    uart_handle_t *uart_handle;
    uart_receive_handler receive_handler;
} uartkit_listener_context_t;

typedef enum uart_receive_mode {
    /// Parse header
    kUARTReceiveModeHeader,
    /// Parse body
    kUARTReceiveModeBody,
    /// Finished receiving body.
    /// Reset and start parsing header on next received byte.
    kUARTReceiveModeFinished
} uart_receive_mode;

void uartkit_init(uart_handle_t *uart_handle, char *client_name) {
    uart_handle->client_name = client_name;
    // uart_handle->listener_registrations = g_hash_table_new(g_str_hash, g_str_equal);

    uart_init(UART0);
    uart_reset_fifos(UART0);
}

void uartkit_send(uart_handle_t *uart_handle, char *to_client_name, uint8_t *data, int size) {
    uart_header_t header = {
        to_client_name,
        uart_handle->client_name,
        size
    };

    // Send headers
    uartkit_send_header("TO_CLIENT_NAME", header.to_client_name);
    uartkit_send_header("FROM_CLIENT_NAME", header.from_client_name);

    char* sContentLength = malloc(sizeof(char) * 20);
    sprintf(sContentLength, "%d", header.content_length);
    uartkit_send_header("CONTENT_LENGTH", sContentLength);
    free(sContentLength);

    // Send body
    uart_send(UART0, '\n');
    for (int i = 0; i < size; i++) {
        uart_send(UART0, data[i]);
    }
}

void uartkit_send_header(char *headerName, char *headerValue) {
    uartkit_send_text(headerName);
    uart_send(UART0, ':');
    uartkit_send_text(headerValue);
    uart_send(UART0, '\n');
}

void uartkit_send_text(char *text) {
    int i = 0;
    while (text[i] != '\0') {
        uart_send(UART0, text[i]);
        i++;
    }
}

/// Allocates on heap, needs balanced free.
char *make_listener_key(uart_receive_handler receive_handler) {
    char* listenerKey = malloc(sizeof(char) * 64);
    sprintf(listenerKey, "%p", receive_handler);
    return listenerKey;
}

void *uartkit_listen_to_messages(void *erased_listener_context) {
    pthread_detach(pthread_self()); 

    uartkit_listener_context_t *listenerContext = erased_listener_context;
    uart_receive_mode receiveMode = kUARTReceiveModeHeader;

    uart_header_t header;

    uint8_t buffer[RECIEVE_BUFFER_SIZE];
    int bufferIndex = 0;
    int lastHeaderCharacterWasNewline = 0;
    char *headerName;

    uint8_t *bodyBuffer;
    int bodyBufferIndex = 0;

    do {
        if (receiveMode == kUARTReceiveModeFinished) {
            header.content_length = 0;
            header.from_client_name = NULL;
            header.to_client_name = NULL;
            bufferIndex = 0;
            bodyBufferIndex = 0;
            free(bodyBuffer);

            receiveMode = kUARTReceiveModeHeader;
        }

        uint8_t receivedByte = uart_recv(UART0);

        switch (receiveMode) {
        case kUARTReceiveModeHeader:
            if (receivedByte == ':') {
                char newHeaderName[bufferIndex];
                memcpy(newHeaderName, buffer, bufferIndex);
                headerName = newHeaderName;
                bufferIndex = 0;
            } else if (receivedByte == '\n') {
                if (lastHeaderCharacterWasNewline) {
                    if (header.to_client_name == NULL) {
                        printf("Bad message. Missing header TO_CLIENT_NAME. Skipping");
                        receiveMode = kUARTReceiveModeFinished;
                        break;
                    }

                    if (header.content_length == 0) {
                        printf("Bad message. Missing header CONTENT_LENGTH. Skipping");
                        receiveMode = kUARTReceiveModeFinished;
                        break;
                    }

                    bodyBuffer = malloc(sizeof(char) * header.content_length);
                    receiveMode = kUARTReceiveModeBody;
                    break;
                }

                char newHeaderValue[bufferIndex];
                memcpy(newHeaderValue, buffer, bufferIndex);
                
                if (strcmp(headerName, "TO_CLIENT_NAME") == 0) {
                    header.to_client_name = strdup(newHeaderValue);
                } else if ((strcmp(headerName, "FROM_CLIENT_NAME") == 0)) {
                    header.from_client_name = strdup(newHeaderValue);
                } else if (strcmp(headerName, "CONTENT_LENGTH") == 0) {
                    int contentLength = atoi(newHeaderValue);
                    header.content_length = contentLength;
                } else {
                    printf("Unknown header name %s. Skipping\n", headerName);
                }

                bufferIndex = 0;
            } else {
                if (bufferIndex == RECIEVE_BUFFER_SIZE) {
                    printf("Buffer size for headers exceeded. Exiting\n");
                    pthread_exit(NULL);
                    return 0;
                }

                buffer[bufferIndex] = receivedByte;
                bufferIndex++;
            }
            
            lastHeaderCharacterWasNewline = receivedByte == '\n';
            break;
        case kUARTReceiveModeBody:
            bodyBuffer[bodyBufferIndex] = receivedByte;
            bodyBufferIndex++;

            if (bodyBufferIndex == header.content_length) {
                uart_header_t headerCopy = header;
                size_t size = sizeof(char) * header.content_length;
                void* data = malloc(size);
                memcpy(data, bodyBuffer, size);
                uart_message_t message = {
                    &headerCopy,
                    data
                };

                if (strcmp(header.to_client_name, listenerContext->uart_handle->client_name) == 0) {
                    listenerContext->receive_handler(message);
                }

                receiveMode = kUARTReceiveModeFinished;
            }
            break;
        case kUARTReceiveModeFinished:
            break;
        }
        
    } while (1);
  
    pthread_exit(NULL); 
}

void uartkit_add_listener(uart_handle_t *uart_handle, uart_receive_handler receive_handler) {
    // char *listenerKey = make_listener_key(receive_handler);
    // gpointer gListenerKey = g_strdup(listenerKey);
    // free(listenerKey);

    // if (g_hash_table_contains(uart_handle->listener_registrations, gListenerKey)) {
    //     printf("Listener to this handler already exists. Exiting.\n");
    //     return;
    // }

    uartkit_listener_context_t listenerContext = {
        uart_handle,
        receive_handler
    };

    pthread_t threadID; 
  
    pthread_create(
        &threadID,
        NULL,
        uartkit_listen_to_messages,
        &listenerContext
    );

    printf("Started listening to UART messages.\n");
    
    // g_hash_table_insert(uart_handle->listener_registrations, gListenerKey, threadID);
  
    // Wait for the created thread to terminate 
    pthread_join(threadID, NULL); 
  
    printf("Stopped listening to UART messages.\n");
  
    pthread_exit(NULL);
}

void uartkit_remove_listener(uart_handle_t *uart_handle, uart_receive_handler receive_handler) {
    // char *listenerKey = make_listener_key(receive_handler);
    // gpointer gListenerKey = g_strdup(listenerKey);
    // free(listenerKey);

    // pthread_t threadID = g_hash_table_lookup(uart_handle->listener_registrations, gListenerKey);

    // if (threadID == NULL) {
    //     printf("Could not find listener to this handler. Exiting.\n");
    //     return;
    // }

    // pthread_cancel(threadID); 
}

void uartkit_destroy(uart_handle_t *uart_handle) {
    uart_handle->client_name = NULL;
    // g_hash_table_destroy(uart_handle->listener_registrations);
    uart_destroy(UART0);
}