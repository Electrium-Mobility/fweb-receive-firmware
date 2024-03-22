#include <stdio.h>


#ifndef BLE_CLIENT
#define BLE_CLIENT

static int ble_gattc_read_cb(uint16_t conn_handle, uint16_t attr_handle,
                             struct ble_gatt_access_ctxt *ctxt, void *arg);
void read_characteristic(uint16_t conn_handle, uint16_t chr_val_handle);
static int ble_gattc_write_cb(uint16_t conn_handle, uint16_t attr_handle,
                              struct ble_gatt_access_ctxt *ctxt, void *arg);
void write_characteristic(uint16_t conn_handle, uint16_t chr_val_handle, uint8_t *data, uint16_t len);
static void ble_gap_event_handler(struct ble_gap_event *event, void *arg);
void connect_to_server(const ble_addr_t *addr);
uint16_t get_characteristic_handle_by_uuid(const ble_uuid_t *svc_uuid, const ble_uuid_t *chr_uuid)
void read_write_test();


#endif

