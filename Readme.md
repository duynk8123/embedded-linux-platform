# Embedded Linux Platform — C/POSIX
## 1. Project overview:
**Aim** To build an Embedded Linux Platform in a production style way.
## 2. Competency goal:
        - C system programming trên Linux
        - POSIX API
        - pthread, mutex, condition variable, rwlock, barrier, atomic
        - Memory management và data structures trong C
        - Linux process/thread model
        - IPC: pipe, FIFO, Unix Domain Socket, shared memory, eventfd
        - Event-driven architecture
        - Hardware Abstraction Layer
        - SocketCAN
        - Linux kernel driver
        - Device Tree
        - systemd
        - watchdog và fault recovery
        - Yocto
        - QEMU
        - performance benchmarking
        - debugging và fault injection   
## 3. Final architect of project
```text
                         HARDWARE
                            │
              ┌─────────────┼─────────────┐
              │             │             │
             CAN           UART          GPIO
              │             │             │
              └─────────────┼─────────────┘
                            │
                            ▼
                    LINUX KERNEL
              ┌─────────────┼─────────────┐
              │             │             │
          CAN Driver     UART Driver    GPIO
              │
              ▼
                       HAL / PLATFORM
              ┌─────────────┼─────────────┐
              │             │             │
          CAN HAL       UART HAL       GPIO HAL
              │
              ▼
                     USERSPACE SERVICES
       ┌──────────────┬──────────────┬──────────────┐
       │              │              │              │
       ▼              ▼              ▼              ▼
  CAN Service    Sensor Service  Monitor       Diagnostics
       │              │              │
       └──────────────┼──────────────┘
                      │
                      ▼
                     IPC
                      │
                      ▼
                Device Manager
                      │
              ┌───────┴────────┐
              ▼                ▼
          Event Bus          Logger
              │
              ▼
          Application
```
## 4. Roadmap:
```text
    v0.1.0
        Event Queue with ring buffer
        Logger
        Device Manager
        Unit Test
    v0.2.0
        Blocking Queue Enhancement
        Stress Test
        AddressSanitizer
        ThreadSanitizer
    v0.3.0
        Memory Pool
        Config Module
        Improved Logger
    v0.4.0
        SocketCAN
        IPC
        Timer Service
    v1.0.0
        Stable Embedded Linux Platform
```

## 5.How to build and run test
```text
    gcc \
    -Wall \
    -Wextra \
    -fsanitize=address \
    -g \
    -pthread \
    -I./src/include \
    ./src/device_manager/event_queue.c \
    ./src/logger/logger.c \
    ./tests/device_manager/t_event_queue.c \
    -o t_event_queue_asan
```
```text
    gcc \
    -Wall \
    -Wextra \
    -fsanitize=thread \
    -g \
    -pthread \
    -I./src/include \
    ./src/device_manager/event_queue.c \
    ./src/logger/logger.c \
    ./tests/device_manager/t_event_queue.c \
    -o t_event_queue_tsan
```
```text
    gcc \
    -Wall \
    -Wextra \
    -g \
    -pthread \
    -I./src/include \
    ./src/device_manager/device_manager.c \
    ./src/device_manager/event_queue.c \
    ./src/logger/logger.c \
    ./tests/device_manager/t_device_manager.c \
    -o t_device_manager
```