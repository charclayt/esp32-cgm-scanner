extern "C" {
#include "nimble/ble.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
// #include "host/ble_gattc.h"
#include "host/util/util.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
}

namespace ble_main {

static const ble_uuid16_t service_uuid = BLE_UUID16_INIT(0xFDE3);
static const ble_uuid16_t notify_chr_uuid = BLE_UUID16_INIT(0xF002);
static const ble_uuid16_t write_chr_uuid = BLE_UUID16_INIT(0xF001);

static uint16_t service_start_handle = 0;
static uint16_t service_end_handle = 0;
static uint16_t notify_chr_val_handle = 0;
static uint16_t write_chr_val_handle = 0;

uint8_t target_ble_address[6] = {0};
uint8_t login_credential[4] = {0};
bool ble_target_set = false;

typedef struct {
    uint16_t conn_handle;
} ble_client_ctx_t;

static ble_client_ctx_t client_ctx;

/**
 * @brief Called when a peripheral device is discovered during scanning
 * 
 * @param event 
 * @param arg 
 * @return int 
 */
int ble_gap_event_handler(struct ble_gap_event* event, void* arg);

void start_scan();

int notification_handler(uint16_t conn_handle, const struct ble_gatt_error* error, struct ble_gatt_attr* attr, void* arg);

} // namespace ble_main
