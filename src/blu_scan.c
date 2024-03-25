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
#include <bluetooth/rfcomm.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>
#include <json-c/json.h>

#include "../utils/config_reader/include/config_reader.h"


struct bt_device_info {
    char name[256];
    char addr[18];
};

int check_minutes_passed(int x, int reset) {
    static time_t start_time = 0; //static to be zero at first call
    
    if (reset) //reset the counter
        start_time = 0;

    time_t current_time;
    time(&current_time);

    if (start_time == 0) {
        start_time = current_time;
        return 1; //in case of the first check, it will be allowed
    }

    double minutes_passed = difftime(current_time, start_time) / 60.0;
    if (minutes_passed >= x) {
        start_time = 0; 
        return 1; 
    }

    return 0; // X minutes have not passed yet
}

int is_allowed_device(char address[18], Config *conf) {
    printf("Comparing if scanned device is in allowed devices list.\n");
 
    for (size_t i = 0; i < conf->allowed_devices_count ; ++i) {
        printf("[%s]:[%s] - [%d]\n", address, conf->allowed_devices[i], i);
        if (strcmp(address, conf->allowed_devices[i]) == 0) {
            return 1;
        }
    }

    printf("Device [%s] not allowed to connect.\n", address);
    return 0; //device is not in allowed list
}

int get_rfcomm_channel(const char *device_addr) {
    bdaddr_t target;
    uuid_t svc_uuid;
    sdp_list_t *response_list = NULL, *search_list, *attrid_list;
    int channel = -1;

    str2ba(device_addr, &target);

    sdp_session_t *session = sdp_connect(BDADDR_ANY, &target, SDP_RETRY_IF_BUSY);
    if (!session) {
        fprintf(stderr, "Can't open SDP session\n");
        return -1;
    }

    // Specify the UUID of the RFCOMM service
    sdp_uuid16_create(&svc_uuid, RFCOMM_UUID);
    search_list = sdp_list_append(NULL, &svc_uuid);

    // Getting RFCOMM attribute channel number
    uint32_t range = 0x0000ffff;
    attrid_list = sdp_list_append(NULL, &range);

    if (sdp_service_search_attr_req(session, search_list, SDP_ATTR_REQ_RANGE, attrid_list, &response_list) == 0) {
        sdp_list_t *proto_list = NULL;
        sdp_list_t *r = response_list;

        for (; r; r = r->next) {
            sdp_record_t *rec = (sdp_record_t *) r->data;

            if (sdp_get_access_protos(rec, &proto_list) == 0) {
                sdp_list_t *p = proto_list;

                for (; p; p = p->next) {
                    sdp_list_t *pds = (sdp_list_t *)p->data;

                    for (; pds; pds = pds->next) {
                        sdp_data_t *d = (sdp_data_t *)pds->data;
                        int proto = 0;

                        for (; d; d = d->next) {
                            switch (d->dtd) { 
                                case SDP_UUID16:
                                case SDP_UUID32:
                                case SDP_UUID128:
                                    proto = sdp_uuid_to_proto(&d->val.uuid);
                                    break;
                                case SDP_UINT8:
                                    if (proto == RFCOMM_UUID) {
                                        channel = d->val.int8;
                                        goto done;
                                    }
                                    break;
                            }
                        }
                    }
                    sdp_list_free((sdp_list_t *)p->data, 0);
                }
                sdp_list_free(proto_list, 0);
            }
            sdp_record_free(rec);
        }
    }

done:
    sdp_list_free(response_list, 0);
    sdp_list_free(search_list, 0);
    sdp_list_free(attrid_list, 0);
    sdp_close(session);

    return channel;
}

int send_command_to_device(int sock, const char *device_addr, const u_int8_t *command, size_t command_len, int channel) {
    struct sockaddr_rc addr = { 0 };
    int client_sock, bytes_written, result = 0;

    // Creating RFCOMM connection
    client_sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    if (client_sock < 0) {
        perror("Cannot create socket");
        return -1;
    }

    addr.rc_family = AF_BLUETOOTH;
    addr.rc_channel = channel;
    str2ba(device_addr, &addr.rc_bdaddr);

    // Conneting
    if (connect(client_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Faild to connect");
        close(client_sock);
        return -1;
    }

    // Sending the comman
    bytes_written = write(client_sock, command, command_len);
    if (bytes_written < 0) {
        perror("Failed to write to socket");
        result = -1;
    } else {
        printf("Successfully sent command to the device [%s]\n", device_addr);
    }

    // Close the socket
    close(client_sock);
    return result;
}

void scan_devices(int sock, Config *config) {
    struct bt_device_info devices[config->max_devices];
    int device_i = 0;

    inquiry_info *info = NULL;
    int max_rsp = config->max_devices;
    int flags = IREQ_CACHE_FLUSH;
    printf("Scanning for Bluetooth devices: [%d sec.]\n", config->scan_duration_seconds);
    int devices_found = hci_inquiry(0, config->scan_duration_seconds, max_rsp, NULL, &info, flags);
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
            //checking for allowed device
            if (is_allowed_device(devices[device_i].addr, config)) {
                printf("Allowed device found!\nTrying to connect and collect data...\n");
                int channel = get_rfcomm_channel(devices[device_i].addr);
                uint8_t command[] = {0xFF, 0xF0}; // notify device
                size_t command_len = sizeof(command);
                //implement a menu of commands from datasheet

                send_command_to_device(sock, devices[device_i].addr, command, command_len, channel);
            }
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
        config_json.allowed_devices[0] = "4C:D5:77:44:09:88";
        config_json.allowed_devices_count = 1;
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
    printf("Allowed Devices: [%d]\n", config_json.allowed_devices_count);
    for (size_t i = 0; i < config_json.allowed_devices_count ; ++i) {
        printf("  - %s\n", config_json.allowed_devices[i]); 
    }
    printf("------------------------------------\n");
    
    struct hci_dev_info dev_info;
    int dev_id = hci_get_route(NULL);
    int sock = hci_open_dev(dev_id);
    if (sock < 0) {
        perror("HCI device open failed");
        exit(1);
    }

    //controls to connect in case to find an allowed device to connect at first scan
    int first_time = 1;
    while (1) {
        printf("New scan instance initiated...\n");
        scan_devices(sock, &config_json);
        printf("Scan instance finished.\n");

        //if the interval time is bigger than 30, it will impact into the connection time 
        printf("Sleep time to next scan start: %d min.\n", config_json.scan_interval_minutes);
        sleep(config_json.scan_interval_minutes * 60);
        printf("Sleep time elapsed!\n");
    }

    hci_close_dev(sock);
    return 0;
}
