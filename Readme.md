How to build test 
    gcc \
    -Wall \
    -Wextra \
    -g \
    -pthread \
    -I./src/include \
    ./src/device_manager/event_queue.c \
    ./src/logger/logger.c \
    ./tests/device_manager/t_event_queue.c \
    -o t_event_queue

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