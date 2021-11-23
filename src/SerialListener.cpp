#include "SerialListener.h"

#include "identifier.h"
#include "mbed.h"
#include "mbedtls/base64.h"

namespace Pulu {
    bool SerialListener::cmd_IS_request_id() {
        char cmd[4];
        for(auto&& c : cmd) {
            mbed_file_handle(STDOUT_FILENO)->read(&c, 1);
        }
        char decodedCmd[2];
        size_t olen;
        mbedtls_base64_decode((unsigned char*)decodedCmd, sizeof(decodedCmd), &olen, (unsigned char*)cmd, sizeof(cmd));
        char search[2] = {'i','d'};
        return memcmp(decodedCmd, search, sizeof(search)) == 0;
    }

    void SerialListener::print_id() {
        auto uid = get_uid_bytes();
        size_t encoded_size;
        mbedtls_base64_encode(nullptr, 0, &encoded_size, uid.data(), uid.size());
        char* encodedUid = (char*) malloc(encoded_size);
        mbedtls_base64_encode((unsigned char*)encodedUid, encoded_size, &encoded_size, uid.data(), uid.size());
        printf("%s\n", encodedUid);
        free(encodedUid);
    }

    EEPROM_Config SerialListener::read_conf(bool &error) {
        const uint8_t BASE64_CONF_LENGTH = 48;
        const uint8_t CONF_LENGTH = 34;

        EEPROM_Config config;
        char conf[BASE64_CONF_LENGTH];
        for(auto&& c : conf) {
            mbed_file_handle(STDOUT_FILENO)->read(&c, 1);
        }

        size_t decoded_size;
        mbedtls_base64_decode(nullptr, 0, &decoded_size, (unsigned char*)conf, sizeof(conf));

        if(decoded_size!=CONF_LENGTH) {
            printf("1\n");
            error = true;
            return config;
        }

        char decoded[CONF_LENGTH];
        mbedtls_base64_decode((unsigned char*)decoded, sizeof(decoded), &decoded_size, (unsigned char*)conf, sizeof(conf));

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