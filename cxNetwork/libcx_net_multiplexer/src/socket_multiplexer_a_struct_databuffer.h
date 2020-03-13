#ifndef SOCKET_MULTIPLEXER_A_STRUCT_DATABUFFER_H
#define SOCKET_MULTIPLEXER_A_STRUCT_DATABUFFER_H

#include <stdint.h>
#include <stdlib.h>

struct sDataBuffer {

    sDataBuffer()
    {
        len=0;
        data=nullptr;
    }
    ~sDataBuffer()
    {
        if (data) free(data);
    }

    bool setData(void * data, uint16_t len)
    {
        if (data && len)
        {
            this->data = malloc(len);
            if (!this->data) return false;
            this->len = len;
            memcpy(this->data, data, len);
            return true;
        }
        return true;
    }

    void * data;
    uint16_t len;
};

#endif // SOCKET_MULTIPLEXER_A_STRUCT_DATABUFFER_H
