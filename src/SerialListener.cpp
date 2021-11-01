#include "SerialListener.h"

#include "identifier.h"
#include "mbed.h"
#include <array>

namespace Pulu {
    bool SerialListener::cmd_IS_request_id() {
        char cmd[4];
        scanf("%4s", (char*)&cmd);
        size_t decodedCmdLength;
        char* decodedCmd = base64.Decode(cmd, 4, &decodedCmdLength);
        return strcmp(decodedCmd,"id") == 0;
    }

    void SerialListener::print_id() {
        auto uid = get_uid_bytes();
        size_t encodedUidLength;
        char* encodedUid = base64.Encode((char*)uid.data(), uid.size(), &encodedUidLength);
        printf("%s\n", encodedUid);
    }

    EEPROM_Config SerialListener::read_conf(bool &error) {
        EEPROM_Config config;
        char conf[BASE64_CONF_LENGTH];
        char pattern[8];
        sprintf(pattern, "%%%ds", BASE64_CONF_LENGTH);
        scanf(pattern, &conf);

        size_t decodedLength;
        char* decoded = base64.Decode(conf, BASE64_CONF_LENGTH, &decodedLength);

        if(decodedLength!=34) {
            printf("1\n");
            error = true;
            return config;
        }

        config.version = 0x03;
        memcpy(&config.keys.devEui, decoded, 8);
        memcpy(&config.keys.appEui, decoded+8, 8);
        memcpy(&config.keys.appKey, decoded+16, 16);
        uint16_t wait_time;
        memcpy(&wait_time, decoded+32, 2);
        wait_time = __REV16(wait_time); // big endian to little endian
        memcpy(&config.wait_time, &wait_time, 2);

        printf("0\n");
        error = false;
        return config;
    }
};