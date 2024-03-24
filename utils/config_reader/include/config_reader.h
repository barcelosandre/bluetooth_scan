#ifndef CONFIG_READER_H
#define CONFIG_READER_H

#include <stddef.h>

typedef struct Config {
    int max_devices;
    int scan_interval_minutes;
    int scan_duration_seconds;
    char **devices_allowed;
    size_t devices_allowed_count;
} Config;

Config read_config(const char *filename);

#endif

