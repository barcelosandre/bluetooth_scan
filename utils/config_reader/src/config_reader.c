#include "../include/config_reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <json-c/json.h>
#include <stddef.h>

Config read_config(const char *filename) {
    struct json_object *parsed_json;
    struct json_object *jmax_devices, *jscan_interval_minutes, *jscan_duration_seconds, *jdevices_allowed, *jdevice;
    Config config = {0};
    size_t devices_allowed_count;

    parsed_json = json_object_from_file(filename);
    if (!parsed_json) {
        fprintf(stderr, "Unable to load JSON file.\n");
        exit(1);
    }

    json_object_object_get_ex(parsed_json, "max_devices", &jmax_devices);
    json_object_object_get_ex(parsed_json, "scan_interval_minutes", &jscan_interval_minutes);
    json_object_object_get_ex(parsed_json, "scan_duration_seconds", &jscan_duration_seconds);
    json_object_object_get_ex(parsed_json, "devices_allowed", &jdevices_allowed);

    config.max_devices = json_object_get_int(jmax_devices);
    config.scan_interval_minutes = json_object_get_int(jscan_interval_minutes);
    config.scan_duration_seconds = json_object_get_int(jscan_duration_seconds);

    config.allowed_devices_count = json_object_array_length(jdevices_allowed);
    config.allowed_devices = malloc(config.allowed_devices_count * sizeof(char*));

    for (size_t i = 0; i < config.allowed_devices_count; i++) {
        jdevice = json_object_array_get_idx(jdevices_allowed, i);
        config.allowed_devices[i] = strdup(json_object_get_string(jdevice));
    }

    json_object_put(parsed_json);
    return config;
}

