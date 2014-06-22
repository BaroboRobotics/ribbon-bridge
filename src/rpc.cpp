#include "rpc.pb.h"

#include <pb_encode.h>
#include <pb_decode.h>

#include <stdio.h>

#include <inttypes.h>

#define assert(x)

void onGet (const Get& get) {
    printf("GET { messageId : %" PRId32 ", name : %" PRId32 " }\n", get.messageId, get.name);
}

void onSet (const Set& set) {
    printf("SET { messageId : %" PRId32 ", name : %" PRId32, set.messageId, set.name);

    for (auto ext = set.value.extensions; ext; ext = ext->next) {
        if (ext->found) {
            printf(", (extension)");
        }
    }
    printf(" }\n");
}

void onSubscribe (const Subscribe& subscribe) {
}

void onUnsubscribe (const Unsubscribe& unsubscribe) {
}

void onFire (const Fire& fire) {
}

void onReply (const Reply& reply) {
}

void onRpcMessage (const RpcMessage& message) {
    switch (message.type) {
        case RpcMessage_Type_GET:
            assert(message.has_get);
            onGet(message.get);
            break;
        case RpcMessage_Type_SET:
            assert(message.has_set);
            onSet(message.set);
            break;
        case RpcMessage_Type_SUBSCRIBE:
            assert(message.has_subscribe);
            onSubscribe(message.subscribe);
            break;
        case RpcMessage_Type_UNSUBSCRIBE:
            assert(message.has_unsubscribe);
            onUnsubscribe(message.unsubscribe);
            break;
        case RpcMessage_Type_FIRE:
            assert(message.has_fire);
            onFire(message.fire);
            break;
        case RpcMessage_Type_REPLY:
            assert(message.has_reply);
            onReply(message.reply);
            break;
        default:
            printf("(unknown RpcMessage type)\n");
            break;
    }
}

int main () {
    uint8_t buf[128];
    size_t nBytesWritten;

    {
        RpcMessage outMessage;
        memset(&outMessage, 0, sizeof(outMessage));

        outMessage.type = RpcMessage_Type_SET;
        outMessage.has_set = true;
        outMessage.set.messageId = 666;
        outMessage.set.name = 333;

        int32_t int32 = 123;

        pb_extension_t ext_int32;
        memset(&ext_int32, 0, sizeof(ext_int32));

        ext_int32.type = &UserValue_int32;
        ext_int32.dest = &int32;
        ext_int32.next = nullptr;

#if 0
        float float_ = 3.14159;

        pb_extension_t ext_float;
        memset(&ext_float, 0, sizeof(ext_float));
        ext_int32.next = &ext_float;

        ext_float.type = &UserValue_float;
        ext_float.dest = &float_;
        ext_float.next = nullptr;
#endif

        outMessage.set.value.extensions = &ext_int32;

        onRpcMessage(outMessage);

        auto ostream = pb_ostream_from_buffer(buf, sizeof(buf));
        if (!pb_encode(&ostream, RpcMessage_fields, &outMessage)) {
            printf("encoding failed: %s\n", PB_GET_ERROR(&ostream));
        }
        nBytesWritten = ostream.bytes_written;
    }

    {
        RpcMessage inMessage;
        memset(&inMessage, 0, sizeof(inMessage));

        int32_t int32 = 987654321;
        float float_ = 1.23456789;

        pb_extension_t ext_int32;
        memset(&ext_int32, 0, sizeof(ext_int32));
        pb_extension_t ext_float;
        memset(&ext_float, 0, sizeof(ext_float));

        ext_int32.type = &UserValue_int32;
        ext_int32.dest = &int32;
        ext_int32.next = &ext_float;
        ext_int32.found = false;

        ext_float.type = &UserValue_float;
        ext_float.dest = &float_;
        ext_float.next = nullptr;
        ext_float.found = false;

        inMessage.set.value.extensions = &ext_int32;

        printf("(before) int32 = %" PRId32 ", float_ = %f\n", int32, float_);

        auto istream = pb_istream_from_buffer(buf, nBytesWritten);
        if (!pb_decode(&istream, RpcMessage_fields, &inMessage)) {
            printf("decoding failed: %s\n", PB_GET_ERROR(&istream));
        }

        printf("(after) int32 = %" PRId32 ", float_ = %f\n", int32, float_);
        onRpcMessage(inMessage);
    }
}
