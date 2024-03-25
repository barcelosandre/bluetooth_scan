#ifndef CONFIG_READER_H
#define CONFIG_READER_H

#include <stddef.h>

typedef struct Config {
    int max_devices;
    int scan_interval_minutes;
    int scan_duration_seconds;
    char **allowed_devices;
    size_t allowed_devices_count;
} Config;

Config read_config(const char *filename);

#endif

