#include <ble_main.hpp>

#include <cstring>
#include <vector>

#include <esp_log.h>
#define TAG "MAIN_BLE"

namespace ble_main {

void start_scan() {
    struct ble_gap_disc_params disc_params = {0};
    disc_params.passive = 0;
    disc_params.itvl = 0x0010;
    disc_params.window = 0x0010;
    disc_params.filter_policy = 0;
    disc_params.limited = 0;

    int rc = ble_gap_disc(0, BLE_HS_FOREVER, &disc_params, ble_gap_event_handler, NULL);
    if (rc != 0) {
        ESP_LOGE(TAG, "Failed to start scan: %d", rc);
    }
}

int ble_gap_event_handler(struct ble_gap_event* event, void* arg) {
    switch (event->type) {
        case BLE_GAP_EVENT_DISC: {
            struct ble_hs_adv_fields fields;
            int rc = ble_hs_adv_parse_fields(&fields, event->disc.data, event->disc.length_data);
            if (rc != 0) {
                ESP_LOGE(TAG, "Failed to parse advertisement data: %d", rc);
            }
            char name[32] = {0};
            if (fields.name != NULL && fields.name_len < sizeof(name)) {
                memcpy(name, fields.name, fields.name_len);
                name[fields.name_len] = '\0';

                if (memcmp(name, target_ble_address, fields.name_len) == 0) {
                    ESP_LOGI(TAG, "Match found: %s", name);
                    ble_gap_disc_cancel();
                    int rc = ble_gap_connect(BLE_OWN_ADDR_PUBLIC, &event->disc.addr, 30000, NULL, ble_gap_event_handler, NULL);
                    if (rc != 0) {
                        ESP_LOGE(TAG, "Connection attempt failed: %d", rc);
                    }
                }
            }
            break;
        }

        case BLE_GAP_EVENT_CONNECT:
            if (event->connect.status == 0) {
                ESP_LOGI(TAG, "Connected to device");
                client_ctx.conn_handle = event->connect.conn_handle;

                int rc = ble_gattc_disc_svc_by_uuid(client_ctx.conn_handle,
                                                   (const ble_uuid_t *)&service_uuid,
                                                   [](uint16_t conn_handle, const struct ble_gatt_error *error,
                                                      const struct ble_gatt_svc *service, void *arg) -> int {
                    if (error->status == 0) {
                        ESP_LOGI(TAG, "FDE3 Service discovered");

                        service_start_handle = service->start_handle;
                        service_end_handle = service->end_handle;

                        ble_gattc_disc_all_chrs(conn_handle, service_start_handle, service_end_handle,
                            [](uint16_t conn_handle, const struct ble_gatt_error *error,
                               const struct ble_gatt_chr *chr, void *arg) -> int {
                                if (error->status == 0) {
                                    ESP_LOGI(TAG, "Characteristic discovered: UUID=0x%04X handle=%d",
                                             chr->uuid.value, chr->val_handle);

                                    if (chr->uuid.value == 0xF002) {
                                        notify_chr_val_handle = chr->val_handle;
                                    }
                                    if (chr->uuid.value == 0xF001) {
                                        write_chr_val_handle = chr->val_handle;
                                    }
                                } else {
                                    ESP_LOGI(TAG, "Characteristic discovery complete");

                                    if (notify_chr_val_handle && write_chr_val_handle) {
                                        struct ble_gatt_subscribe_params params = {0};
                                        params.cccd_handle = notify_chr_val_handle + 1; // Client Characteristic Configuration Descriptor is usually next to the value handle
                                        params.value = BLE_GATT_SUBSCRIBE_INDICATION | BLE_GATT_SUBSCRIBE_NOTIFICATION;
                                        params.flags = 0;
                                        params.notify_cb = notification_handler; // Set callback for notifications

                                        ble_gattc_subscribe(client_ctx.conn_handle, &params);

                                        ble_gattc_write_flat(client_ctx.conn_handle, write_chr_val_handle,
                                                             login_credential, sizeof(login_credential),
                                                             [](uint16_t conn_handle, const struct ble_gatt_error *error,
                                                                struct ble_gatt_attr *attr, void *arg) -> int {
                                            if (error->status == 0) {
                                                ESP_LOGI(TAG, "Login credentials written successfully");
                                            } else {
                                                ESP_LOGE(TAG, "Failed to write login credentials");
                                            }
                                            return 0;
                                        }, NULL);
                                    } else {
                                        ESP_LOGE(TAG, "Required characteristics not found");
                                    }
                                    return BLE_HS_EDONE;
                                }
                                return 0;
                            }, NULL);

                    } else {
                        ESP_LOGE(TAG, "FDE3 service not found");
                    }
                    return 0;
                }, NULL);

                if (rc != 0) {
                    ESP_LOGE(TAG, "Service discovery failed: %d", rc);
                }

            } else {
                ESP_LOGE(TAG, "Failed to connect: %d", event->connect.status);
                start_scan();
            }
            break;

        case BLE_GAP_EVENT_DISCONNECT:
            ESP_LOGI(TAG, "Disconnected");
            start_scan();
            break;

        default:
            break;
    }
    return 0;
}

int notification_handler(uint16_t conn_handle, const struct ble_gatt_error* error, struct ble_gatt_attr* attr, void* arg) {
    if (error->status != 0) {
        ESP_LOGE(TAG, "Notification error: %d", error->status);
    }

    ESP_LOGI(TAG, "Notification received on F002, length: %d", attr->om->om_len);

    // Process the notification data here
    std::vector<uint8_t> notification_data(attr->om->om_data, attr->om->om_data + attr->om->om_len);

    // Decrypt BLE + pass to handler

    return 0;
}

} // namespace ble_main
