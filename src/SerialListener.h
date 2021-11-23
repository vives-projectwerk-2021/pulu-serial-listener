#pragma once

#include "eepromConfig.h"

namespace Pulu {
    class SerialListener {
        public:
            bool cmd_IS_request_id();
            void print_id();
            EEPROM_Config read_conf(bool &error);
        private:
            void read(char* data, uint8_t length);
    };
};