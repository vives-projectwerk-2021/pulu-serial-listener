#pragma once

#include "USBSerial.h"
#include "BufferedSerial.h"
#include "eepromConfig.h"

namespace Pulu {
    class SerialListener {
        public:
            SerialListener(BufferedSerial* bufferedSerial);
            SerialListener(USBSerial* usb);
        public:
            bool cmd_IS_request_id();
            bool cmd_IS_reset();
            void print_id();
            EEPROM_Config read_conf(bool &error);
        private:
            void read(char* data, uint8_t length, bool blocking = false);

        private:
            BufferedSerial* bufferedSerial = nullptr;
            USBSerial* usb = nullptr;
    };
};