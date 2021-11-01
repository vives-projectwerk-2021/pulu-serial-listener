#pragma once

#include "base64.h"
#include "eepromConfig.h"

#define BASE64_CONF_LENGTH 48

namespace Pulu {
    class SerialListener {
        public:
            bool cmd_IS_request_id();
            void print_id();
            EEPROM_Config read_conf(bool &error);
        
        private:
            Base64 base64;
    };
};