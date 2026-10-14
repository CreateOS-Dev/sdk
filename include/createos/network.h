#ifndef CREATEOS_NETWORK_H
#define CREATEOS_NETWORK_H

#include <stdint.h>

int createos_net_tcp_listen(int interface_index, uint16_t port);
int createos_net_tcp_accept(int listener, uint32_t timeout_ms);
int createos_net_tcp_read(int stream, void* buffer, uint32_t capacity,
                          uint32_t timeout_ms);
int createos_net_tcp_write(int stream, const void* buffer, uint32_t length,
                           uint32_t timeout_ms);
int createos_net_tcp_close(int stream);
int createos_net_tcp_listener_close(int listener);

#endif
