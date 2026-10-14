#include "createos/network.h"
#include "user_syscall.h"

int createos_net_tcp_listen(int interface_index, uint16_t port)
{
    if (interface_index < 0 || interface_index >= sys_net_count() || !port)
        return -1;
    return sys_net_tcp_listen(interface_index, port);
}

int createos_net_tcp_accept(int listener, uint32_t timeout_ms)
{
    if (listener <= 0 || !timeout_ms || timeout_ms > 30000)
        return -1;
    return sys_net_tcp_accept(listener, timeout_ms);
}

int createos_net_tcp_read(int stream, void* buffer, uint32_t capacity,
                          uint32_t timeout_ms)
{
    if (stream <= 0 || !buffer || !capacity || capacity > 4096 ||
        !timeout_ms || timeout_ms > 30000)
        return -1;
    return sys_net_tcp_stream_read(stream, buffer, capacity, timeout_ms);
}

int createos_net_tcp_write(int stream, const void* buffer, uint32_t length,
                           uint32_t timeout_ms)
{
    if (stream <= 0 || (!buffer && length) || !length || length > 4096 ||
        !timeout_ms || timeout_ms > 30000)
        return -1;
    return sys_net_tcp_stream_write(stream, buffer, length, timeout_ms);
}

int createos_net_tcp_close(int stream)
{
    if (stream <= 0)
        return -1;
    return sys_net_tcp_stream_close(stream);
}

int createos_net_tcp_listener_close(int listener)
{
    if (listener <= 0)
        return -1;
    return sys_net_tcp_listener_close(listener);
}
