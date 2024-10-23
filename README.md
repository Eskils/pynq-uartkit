# PYNQ UARTKit

This is a library to communicate asynchronously between multiple clients using 
UART.

This program is written using TU Eindhoven’s 
[libpynq](https://pynq.tue.nl/5ewc0/libpynq/) library.

> **NOTE:** This is a work in progress

## Example usage

### Sender

```c
#include <string.h>
#include <libpynq.h>
#include <libUARTKit.h>
#include <stdio.h>

int main() {

    pynq_init();

    switchbox_init();

    switchbox_set_pin(IO_PMODA1, SWB_UART0_RX);
    switchbox_set_pin(IO_PMODA4, SWB_UART0_TX);

    uart_handle_t uartHandle;
    uartkit_init(&uartHandle, "Node0");

    char *messageTextToSend = "Hello world!";
    int size = strlen(messageTextToSend);
    uartkit_send(&uartHandle, "Node1", (uint8_t*)messageTextToSend, size);

    printf("Sent message\n");

    printf("Destroying handler\n");

    uartkit_destroy(&uartHandle);

    pynq_destroy();

    return 0;
}
```

### Receiver

```c
#include <string.h>
#include <libpynq.h>
#include <libUARTKit.h>
#include <stdio.h>

void didReceiveMessage(uart_message_t message) {
    int contentLength = message.header->content_length;
    
    char *messageText = malloc(contentLength + 1);
    messageText[contentLength] = 0;
    memcpy(messageText, message.data, contentLength);

    printf("Message: %s\n", messageText);

    free(messageText);
}

int main() {

    pynq_init();

    switchbox_init();

    switchbox_set_pin(IO_PMODA1, SWB_UART0_RX);
    switchbox_set_pin(IO_PMODA4, SWB_UART0_TX);

    uart_handle_t uartHandle;
    uartkit_init(&uartHandle, "Node1");

    sleep_msec(50);

    uartkit_add_listener(&uartHandle, didReceiveMessage);

    printf("Added listener\n");

    do {
      sleep_msec(1000);
    } while (1)

    printf("Removing listener and destroying handle \n");

    uartkit_remove_listener(&uartHandle, didReceiveMessage);

    uartkit_destroy(&uartHandle);

    pynq_destroy();

    return 0;
}

```

## Reference

### uart_handle_t
The structure you receive from `uartkit_init`.

Set __verbose__ to __1__ for verbose logging.

```c
typedef struct uart_handle_t {
    char *client_name;
    GHashTable *listener_registrations;
    int verbose;
} uart_handle_t;
```

### uart_header_t
```c
typedef struct uart_header_t {
    char *to_client_name;
    char *from_client_name;
    int content_length;
} uart_header_t;
```

### uart_message_t
```c
typedef struct uart_message_t {
    uart_header_t *header;
    void *data;
} uart_message_t;
```

### uartkit_init

Initialize a new instance for sending and receiving messages.
Also initializes UART0 from libPYNQ and resets the FIFO buffers.
You need to specify which pins to use for UART yourself.

Remember to call `uartkit_destroy` to clean up.

```c
extern void uartkit_init(uart_handle_t *uart_handle, char *client_name);
```

### uartkit_send

Send a message to the client with the specified name.

```c
extern void uartkit_send(uart_handle_t *uart_handle, char *to_client_name, uint8_t *data, size_t size);
```

### uartkit_add_listener

Register a listener for messages.
You specify a callback function which takes a type of `uart_message`.
You can only add a single listener per callback funcition.

Remember to call `uartkit_remove_listener` to clean up.

```c
extern void uartkit_add_listener(uart_handle_t *uart_handle, uart_receive_handler receive_handler);
```

### uartkit_remove_listener

Removes a listener for messages.

```c
extern void uartkit_remove_listener(uart_handle_t *uart_handle, uart_receive_handler receive_handler);
```

### uartkit_destroy
Destroys the instance for sending and receiving messages.
Also deinitializes the UART0-channel from libPYNQ.

```c
extern void uartkit_destroy(uart_handle_t *uart_handle);
```

## Instructions

The Makefile for this program has been written in such a way as to cross compile
from a separate machine and transfer to the PYNQ-board.

You might want to configure the build before running `make`. 

In broad terms, steps to build are as follows:

1. Make sure you have a compiled version of `libpynq` from TU/e.
   + The compiled library is expected at __../../libpynq/lib/libpynq.a__
   + Headers are expected at __../../libpynq/include/…__
   + Change the `LIBRARIES` variable in `Makefile` if necessary.

2. Make sure you have a sysroot from the PYNQ board.
   + You can ssh into the PYNQ-board and archive `/lib /usr/include /usr/lib /usr/local/lib /usr/local/include`
   + The sysroot is expected at __../../sysroot__
   + Change the `SYSROOT` variable in `Makefile` if necessary.

3. Make sure you have a compiler and linker that supports the PYNQ-board
   + The target triple for PYNQ-Z2 is `arm-linux-gnueabihf`
   + I had success with Apple `clang` bundled with Xcode and and `lld` installed with Homebrew. 
   + In Makefile, clang is told to use lld for linking with `fuse-ld=lld`

4. Compile
   + Run `make` in order to compile.
   + Object files are put into __build/artifacts__
   + The output library is put into __build/bin__
   + Headers are put into __build/include__

## Roadmap
- [ ] Possible linker issues
- [x] Using a map to store listener registrations
- [ ] Testing communication between two boards
- [ ] Testing communication between multiple boards
- [ ] Locking and unlocking channel so messages are not mangled

## Editing source code in VS Code

For code completion, error checking and navigation I recommend using the `clangd` extension in VS Code.

You can add a __.clangd__ file in the project root specifying where to find the libpynq headers like so:

```yaml
CompileFlags:
  Add:
    - "-I/Users/.../Documents/Projects/PYNQ/libpynq/include"
    - "--sysroot=/Users/.../Documents/Projects/PYNQ/sysroot"
    - "--target=arm-linux-gnueabihf"
```