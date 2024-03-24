#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/socket.h>
#include <time.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <json-c/json.h>

#include "../utils/config_reader/include/config_reader.h"


struct bt_device_info {
    char name[256];
    char addr[18];
};

void scan_devices(int sock, Config config) {
    struct bt_device_info devices[config.max_devices];
    int device_i = 0;

    inquiry_info *info = NULL;
    int max_rsp = config.max_devices;
    int flags = IREQ_CACHE_FLUSH;
    printf("Scanning for Bluetooth devices: [%d sec.]\n", config.scan_duration_seconds);
    int devices_found = hci_inquiry(0, config.scan_duration_seconds, max_rsp, NULL, &info, flags);
    if (devices_found < 0) {
        perror("HCI Request FAILED");
        return;
    }
    printf("Scan completed successfuly.\nTotal of [%d] bluetooth devices found!\n", devices_found);

    int i = 0; //declared here to use twice (json loop)
    for (i = 0; i < devices_found; i++) {
        ba2str(&(info+i)->bdaddr, devices[device_i].addr);

        //check if MAC ADDRESS is not valid, not spend time retrieving the device name
        if (strcmp(devices[device_i].addr, "00:00:00:00:00:00") != 0){ 
            memset(devices[device_i].name, 0, sizeof(devices[device_i].name));
            if (hci_read_remote_name(sock, &(info+i)->bdaddr, sizeof(devices[device_i].name), devices[device_i].name, 0) < 0) {
                strcpy(devices[device_i].name, "Unknown");
            }
            printf("Device Name: %s, Address: %s\n", devices[device_i].name, devices[device_i].addr);
            device_i++;
        }

        //TODO: Strange behavior acumulating new 'Unknown' devices. Could be a bug?
    }

    json_object *jobj = json_object_new_object();
    json_object *jarray = json_object_new_array();

    for (i = 0; i < device_i; i++) {
        json_object *jdev = json_object_new_object();
        json_object_object_add(jdev, "Name", json_object_new_string(devices[i].name));
        json_object_object_add(jdev, "MAC_Address", json_object_new_string(devices[i].addr));
        json_object_array_add(jarray, jdev);
    }

    json_object_object_add(jobj, "Devices", jarray);

    const char *json_str = json_object_to_json_string_ext(jobj, JSON_C_TO_STRING_PRETTY);
    FILE *fp = fopen("scanned_devices.json", "w");
    if (fp != NULL) {
        fputs(json_str, fp);
        fclose(fp);
    } else {
        fprintf(stderr, "Error opening JSON file\n");
    }

    json_object_put(jobj);
    free(info);
    //TODO: check to free all used memory
}

int main(int argc, char *argv[]) {
    //Check is its root user because we had issue to access the interface at this level
    if (geteuid() != 0) {
        fprintf(stderr, "This application must be run as root.\n");
        return 1; // Exit the program if not root
    }

    // Proceed with the rest of the application
    printf("Running as root, continue with the application.\n");
    printf("Bluetooth Scanner v0.01\n");
    printf("------------------------------------\n");
    Config config_json;

    if (argc < 2) {
        printf("JSON config file NOT found: Setting DEFAULT configs.\n");
        config_json.max_devices = 255;
        config_json.scan_interval_minutes = 1;
        config_json.scan_duration_seconds = 30;
        config_json.devices_allowed[0] = "4C:D5:77:44:09:88";
        config_json.devices_allowed_count = 1;
    }else{
        printf("JSON config file found: Loading file...\n");
        const char *filename = argv[1];
        config_json = read_config(filename);
    }

    //TODO: Attribute validator for partial config OR bad JSON config format

    printf("Bluetooth scanner params\n");
    printf("Max Devices: %d\n", config_json.max_devices);
    printf("Scan Interval Minutes: %d\n", config_json.scan_interval_minutes);
    printf("Scan Duration Seconds: %d\n", config_json.scan_duration_seconds);
    printf("Allowed Devices: [%d]\n", config_json.devices_allowed_count);
    for (size_t i = 0; i < config_json.devices_allowed_count ; ++i) {
        printf("  - %s\n", config_json.devices_allowed[i]);
        free(config_json.devices_allowed[i]); 
    }
    free(config_json.devices_allowed);
    printf("------------------------------------\n");
    
    struct hci_dev_info dev_info;
    int dev_id = hci_get_route(NULL);
    int sock = hci_open_dev(dev_id);
    if (sock < 0) {
        perror("HCI device open failed");
        exit(1);
    }

    while (1) {
        printf("New scan instance initiated...\n");
        scan_devices(sock, config_json);
        printf("Scan instance finished.\n");


        printf("Sleep time to next scan start: %d min.\n", config_json.scan_interval_minutes);
        sleep(config_json.scan_interval_minutes * 60);
        printf("Sleep time elapsed!\n");
    }

    hci_close_dev(sock);
    return 0;
}
