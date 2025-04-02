idf_component_register(SRCS
                        "sensor/sensor.cpp"
                        "sensor/sensor_family.cpp"
                        "sensor/sensor_region.cpp"
                        "sensor/sensor_serial.cpp"
                        "sensor/sensor_state.cpp"
                        "sensor/sensor_types.cpp"
                        "crc.cpp"
                        "decrypt.cpp"
                        "fram.cpp"
                        "nfc.cpp"
                        "record.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES PN5180-Library)
