#include "SerialListener.h"

#include "identifier.h"
#include "mbed.h"
#include "mbedtls/base64.h"

namespace Pulu {
    SerialListener::SerialListener(BufferedSerial* bufferedSerial) {
        this->bufferedSerial = bufferedSerial;
    }
    SerialListener::SerialListener(USBSerial* usb) {
        this->usb = usb;
    }

    bool SerialListener::cmd_IS_request_id() {
        char cmd[4];
        read(cmd, sizeof(cmd));
        char decodedCmd[2];
        size_t olen;
        mbedtls_base64_decode((unsigned char*)decodedCmd, sizeof(decodedCmd), &olen, (unsigned char*)cmd, sizeof(cmd));
        char search[2] = {'i','d'};
        return memcmp(decodedCmd, search, sizeof(search)) == 0;
    }

    bool SerialListener::cmd_IS_reset() {
        char cmd[4];
        read(cmd, sizeof(cmd), true);
        char decodedCmd[3];
        size_t olen;
        mbedtls_base64_decode((unsigned char*)decodedCmd, sizeof(decodedCmd), &olen, (unsigned char*)cmd, sizeof(cmd));
        char search[3] = {'r','e','s'};
        return memcmp(decodedCmd, search, sizeof(search)) == 0;
    }

    void SerialListener::print_id() {
        auto uid = get_uid_bytes();
        size_t encoded_size;
        mbedtls_base64_encode(nullptr, 0, &encoded_size, uid.data(), uid.size());
        char* encodedUid = (char*) malloc(encoded_size);
        mbedtls_base64_encode((unsigned char*)encodedUid, encoded_size, &encoded_size, uid.data(), uid.size());
        printf("%s\r\n", encodedUid);
        free(encodedUid);
    }

    EEPROM_Config SerialListener::read_conf(bool &error) {
        const uint8_t BASE64_CONF_LENGTH = 48;
        const uint8_t CONF_LENGTH = 34;

        EEPROM_Config config;
        char conf[BASE64_CONF_LENGTH];
        read(conf, sizeof(conf), true);

        size_t decoded_size;
        mbedtls_base64_decode(nullptr, 0, &decoded_size, (unsigned char*)conf, sizeof(conf));

        if(decoded_size!=CONF_LENGTH) {
            printf("1\r\n");
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

        printf("0\r\n");
        error = false;
        return config;
    }


    void SerialListener::read(char* data, uint8_t length, bool blocking) {
        const uint8_t MAX_WAIT_TIME_SECONDS = 20;
        const uint8_t TRY_INTERVAL_MS = 200;
        const uint8_t MAX_TRY_COUNT = (1000/TRY_INTERVAL_MS)*MAX_WAIT_TIME_SECONDS;
        for(uint8_t i = 0; i<length; i++) {
            if(blocking) {
                ssize_t ret;
                do { ret = mbed_file_handle(STDIN_FILENO)->read(&data[i], 1); }
                while(ret == 0);
            }
            else if(usb) {
                uint8_t try_count = 0;
                do {
                    if(usb->readable())
                    {
                        usb->read(&data[i], 1);
                        break;
                    }
                    else
                    {
                        ThisThread::sleep_for(TRY_INTERVAL_MS*1ms);
                    }
                }
                while(try_count++<MAX_TRY_COUNT);
                if(try_count>MAX_TRY_COUNT) return;
            }
            else if(bufferedSerial) {
                uint8_t try_count = 0;
                do {
                    if(bufferedSerial->readable())
                    {
                        bufferedSerial->read(&data[i], 1);
                        break;
                    }
                    else
                    {
                        ThisThread::sleep_for(TRY_INTERVAL_MS*1ms);
                    }
                }
                while(try_count++<MAX_TRY_COUNT);
                if(try_count>MAX_TRY_COUNT) return;
            }
        }
    }
};