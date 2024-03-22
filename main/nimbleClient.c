#include "esp_log.h"
#include "host/ble_hs.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"

#define GATTC_TAG "CLIENT"

// Global variable to store the connection handle
static uint16_t g_conn_handle = BLE_HS_CONN_HANDLE_NONE;

float speed = 100.0;
float battery = 0.4;


// Callback for handling read responses
static int ble_gattc_read_cb(uint16_t conn_handle, uint16_t attr_handle,
                             struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    // Handle the received data
    ESP_LOGI(GATTC_TAG, "Received data from server");
    

    if (ctxt->om->om_len == sizeof(float)) 
    {
        memcpy(&battery, ctxt->om->om_data, sizeof(float));
        return 0;
    }

    return 0;
}

// Read a characteristic from the server
void read_characteristic(uint16_t conn_handle, uint16_t chr_val_handle)
{
    int rc;

    // Perform the read operation
    rc = ble_gattc_read(conn_handle, chr_val_handle, ble_gattc_read_cb, NULL);
    if (rc != 0) {
        ESP_LOGE(GATTC_TAG, "Failed to read characteristic: %d", rc);
    }
}

// Callback for handling write responses
static int ble_gattc_write_cb(uint16_t conn_handle, uint16_t attr_handle,
                              struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    // Handle the write response
    ESP_LOGI(GATTC_TAG, "Characteristic write response received");
    return 0;
}

// Write data to a characteristic on the server
void write_characteristic(uint16_t conn_handle, uint16_t chr_val_handle, uint8_t *data, uint16_t len)
{
    int rc;

    // Create an mbuf to hold the data
    struct os_mbuf *om = ble_hs_mbuf_from_flat(data, len);
    if (om == NULL) {
        ESP_LOGE(GATTC_TAG, "Failed to create mbuf for write operation");
        return;
    }

    // Perform the write operation
    rc = ble_gattc_write(conn_handle, chr_val_handle, om, ble_gattc_write_cb, NULL);
    if (rc != 0) {
        ESP_LOGE(GATTC_TAG, "Failed to write characteristic: %d", rc);
        os_mbuf_free_chain(om);
    }
}




// Callback for handling connection events
static void ble_gap_event_handler(struct ble_gap_event *event, void *arg)
{
    switch (event->type) {
        case BLE_GAP_EVENT_CONNECT:
            ESP_LOGI(GATTC_TAG, "Connected to server");
            g_conn_handle = event->connect.conn_handle; // Save the connection handle
            break;
        case BLE_GAP_EVENT_DISCONNECT:
            ESP_LOGI(GATTC_TAG, "Disconnected from server");
            g_conn_handle = BLE_HS_CONN_HANDLE_NONE; // Reset the connection handle
            break;
        default:
            break;
    }
}


// Connect to a server
void connect_to_server(const ble_addr_t *addr)
{
    struct ble_gap_conn_params conn_params = {
        .scan_itvl = 0x0010,
        .scan_window = 0x0010,
        .itvl_min = 0x0030,
        .itvl_max = 0x0030,
        .latency = 0,
        .supervision_timeout = 0x0040,
        .min_ce_len = 0x0010,
        .max_ce_len = 0x0010,
    };

    int rc = ble_gap_connect(BLE_OWN_ADDR_PUBLIC, addr, 10000, &conn_params, ble_gap_event_handler, NULL);
    if (rc != 0) {
        ESP_LOGE(GATTC_TAG, "Failed to connect to server: %d", rc);
    }
}


// Get the characteristic handle by UUID
uint16_t get_characteristic_handle_by_uuid(const ble_uuid_t *svc_uuid, const ble_uuid_t *chr_uuid)
{

    // Iterate through characteristics of the service
    struct ble_gatt_chr *chr;
    uint16_t *out_val_handle = 1;

    chr = ble_gatts_find_chr(svc_uuid, chr_uuid, NULL, out_val_handle);

    if (chr == 0) {
        ESP_LOGI(GATTC_TAG, "Characteristic handle found:");
    } else {
        ESP_LOGE(GATTC_TAG, "Characteristic not found");
    }

    return *out_val_handle;
}


void read_write_test()
{
    // UUID declaration must use the same hex values as they are declared with on the server
    ble_uuid_t *svc_uuid = BLE_UUID16_DECLARE(0x180);
    ble_uuid_t *read_chr_uuid = BLE_UUID16_DECLARE(0xFEF4);
    ble_uuid_t *write_chr_uuid = BLE_UUID16_DECLARE(0xDEAD);

    uint16_t read_chr_val_handle = get_characteristic_handle_by_uuid(svc_uuid, read_chr_uuid);
    uint16_t write_chr_val_handle = get_characteristic_handle_by_uuid(svc_uuid, write_chr_uuid);

    
    // Read the battery level from the server
    read_characteristic(g_conn_handle, read_chr_val_handle);

    
    // Convert the float value to a byte array
    uint8_t data[sizeof(float)];
    memcpy(data, &speed, sizeof(float));

    // Write the speed to the server
    write_characteristic(g_conn_handle, write_chr_val_handle, data, sizeof(data));

}
