#include <stdio.h>
#include <stdlib.h>
#include <json-c/json.h>

typedef struct Config {
    int max_devices;
    int scan_interval_minutes;
    int scan_duration_seconds;
    char **devices_allowed;
    size_t devices_allowed_count;
} Config;

Config read_config(const char *filename) {
    struct json_object *parsed_json;
    struct json_object *jmax_devices;
    struct json_object *jscan_interval_minutes;
    struct json_object *jscan_duration_seconds;
    struct json_object *jdevices_allowed;
    struct json_object *jdevice;
    Config config = {0};

    // Read the JSON file
    parsed_json = json_object_from_file(filename);
    if (!parsed_json) {
        fprintf(stderr, "Unable to load JSON file.\n");
        exit(1);
    }

    // Parse the JSON content
    json_object_object_get_ex(parsed_json, "max_devices", &jmax_devices);
    json_object_object_get_ex(parsed_json, "scan_interval_minutes", &jscan_interval_minutes);
    json_object_object_get_ex(parsed_json, "scan_duration_seconds", &jscan_duration_seconds);
    json_object_object_get_ex(parsed_json, "devices_allowed", &jdevices_allowed);

    config.max_devices = json_object_get_int(jmax_devices);
    config.scan_interval_minutes = json_object_get_int(jscan_interval_minutes);
    config.scan_duration_seconds = json_object_get_int(jscan_duration_seconds);

    config.devices_allowed_count = json_object_array_length(jdevices_allowed);
    config.devices_allowed = malloc(config.devices_allowed_count * sizeof(char*));

    for (size_t i = 0; i < config.devices_allowed_count; i++) {
        jdevice = json_object_array_get_idx(jdevices_allowed, i);
        config.devices_allowed[i] = strdup(json_object_get_string(jdevice));
    }

    // Cleanup JSON object
    json_object_put(parsed_json);

    return config;
}

// int main() {
//     const char *config_file = "config.json";
//     Config config = read_config(config_file);

//     printf("Max Devices: %d\n", config.max_devices);
//     printf("Scan Interval Minutes: %d\n", config.scan_interval_minutes);
//     printf("Scan Duration Seconds: %d\n", config.scan_duration_seconds);
//     printf("Devices Allowed:\n");
//     for (size_t i = 0; i < config.devices_allowed_count; i++) {
//         printf("  - %s\n", config.devices_allowed[i]);
//         free(config.devices_allowed[i]); // Free the duplicated string
//     }
//     free(config.devices_allowed); // Free the allocated array

//     return 0;
// }

