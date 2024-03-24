#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <json-c/json.h>

#define MAX_DEVICES 10 //and qty of connection temptatives
#define SCAN_INTERVAL_MINUTES 1
#define SCAN_DURATION_SECONDS 10

struct bt_device_info {
    char addr[18];
};

void scan_devices(int sock) {
    struct bt_device_info devices[MAX_DEVICES];
    struct timeval tv;
    fd_set readfds;
    int num_devices = 0;

    le_set_scan_parameters_cp scan_params;
    uint8_t own_type = 0x00; // Public Device Address
    uint8_t scan_type = 0x00; // Passive scanning
    uint16_t interval = htobs(0x0010);
    uint16_t window = htobs(0x0010);
    uint8_t filter_policy = 0x00; // Accept all

    memset(&scan_params, 0, sizeof(scan_params));
    scan_params.type = scan_type;
    scan_params.interval = interval;
    scan_params.window = window;
    scan_params.own_bdaddr_type = own_type;
    scan_params.filter = filter_policy;
    
    int conn_status = -99;
    u_int8_t idx = 0;
    while ((conn_status < 0)){
        printf("Trying to setup BLE parameters...[%d]:[%d]\n", conn_status, idx);
        conn_status = hci_le_set_scan_parameters(sock, scan_type, interval, window, own_type, filter_policy, 1000);
        sleep(3); // 3 seconds to next device open try
        
        if (idx >= MAX_DEVICES) {
            perror("Failed to set scan parameters");
            return;
        }
        idx++;
    }
    printf("BLE parameters setted up successfuly!\n");
    
    conn_status = -99;
    idx = 0;
    while ((conn_status < 0)){
        printf("Trying to open BLE interface...[%d]:[%d]\n", conn_status, idx);
        conn_status = hci_le_set_scan_enable(sock, 0x01, 0, 1000);
        sleep(3); // 3 seconds to next device open try
        
        if (idx >= MAX_DEVICES) {
            perror("Failed to enable scan");
            return;
        }
        idx++;
    }
    //TODO: dynamic alocation 'con' to free it
    printf("BLE device opened successfuly.\n");
    printf("Starting BLE scan attempts...\n");
    for (int i = 0; i < MAX_DEVICES; i++) {
        tv.tv_sec = 3;  //3 seconds timeout to data avail check
        tv.tv_usec = 0;
        // Scan for BLE devices
        le_advertising_info *info;
        uint8_t buff[HCI_MAX_EVENT_SIZE];
        evt_le_meta_event *meta_event;
        int sel = select(sock+1, &readfds, NULL, NULL, &tv);
        if (sel > 0){
            int len = read(sock, buff, sizeof(buff));
            if (len >= HCI_EVENT_HDR_SIZE) {
                meta_event = (evt_le_meta_event *)(buff + HCI_EVENT_HDR_SIZE + 1);
                if (meta_event->subevent == EVT_LE_ADVERTISING_REPORT) {
                    info = (le_advertising_info *)(meta_event->data + 1);
                    ba2str(&(info->bdaddr), devices[num_devices].addr);
                    printf("Device Address: %s\n", devices[num_devices].addr);
                    num_devices++;
                }
            }
        }else if (sel == 0) {
            // No data within the timeout period
            printf("Attempt[%d]: No BLE advert received within the timeout period.\n", i);
        } else {
            // Error occurred in select
            perror("Select failed");
            return;
        }
        
    }

    // Disable scanning after completion
    hci_le_set_scan_enable(sock, 0x00, 0, 1000);
    printf("Device scanning stopped.\n");

    json_object *jobj = json_object_new_object();
    json_object *jarray = json_object_new_array();

    for (int i = 0; i < num_devices; i++) {
        json_object *jdev = json_object_new_object();
        json_object_object_add(jdev, "MAC_Address", json_object_new_string(devices[i].addr));
        json_object_array_add(jarray, jdev);
    }

    json_object_object_add(jobj, "Devices", jarray);

    const char *json_str = json_object_to_json_string_ext(jobj, JSON_C_TO_STRING_PRETTY);
    FILE *fp = fopen("scanned_ble_devices.json", "w");
    if (fp != NULL) {
        fputs(json_str, fp);
        fclose(fp);
        printf("JSON Saved succesfuly!\n");
    } else {
        fprintf(stderr, "Error opening JSON file\n");
    }

    json_object_put(jobj);
}

int main() {
    if (geteuid() != 0) {
        fprintf(stderr, "This application must be run as root.\n");
        return 1; // Exit the program if not root
    }

    // Proceed with the rest of the application
    printf("Running as root, continue with the application.\n");
    printf("BLE Scanner v0.01\n");
    printf("------------------------------------\n\n\n");

    struct hci_dev_info dev_info;
    int dev_id = hci_get_route(NULL);
    int sock = hci_open_dev(dev_id);
    if (sock < 0) {
        perror("HCI device open failed");
        exit(1);
    }

    while (1) {
        printf("New scan instance initiated...\n");
        scan_devices(sock);
        printf("Scan instance finished.\n");


        printf("Sleep time to next scan started: %d min.\n", SCAN_INTERVAL_MINUTES);
        sleep(SCAN_INTERVAL_MINUTES * 60);
        printf("Sleep time elapsed!\n");
    }

    hci_close_dev(sock);
    return 0;
}
