#include "rpc.pb.h"

#include <pb_encode.h>
#include <pb_decode.h>

#include <iostream>
#include <iomanip>

#include <cassert>

void dumpGet (const Get& get) {
    std::cout << "GET { messageId : " << get.messageId << ", name : " << get.name << " }\n";
}

void dumpSet (const Set& set) {
    std::cout << "SET { messageId : " << set.messageId
        << ", name : " << set.name;

    for (auto ext = set.value.extensions; ext; ext = ext->next) {
        if (ext->found) {
            std::cout << ", (extension)";
        }
    }
    std::cout << " }\n";
}

void dumpSubscribe (const Subscribe& subscribe) {
}

void dumpUnsubscribe (const Unsubscribe& unsubscribe) {
}

void dumpFire (const Fire& fire) {
}

void dumpReply (const Reply& reply) {
}

void dumpRpcMessage (const RpcMessage& message) {
    switch (message.type) {
        case RpcMessage_Type_GET:
            assert(message.has_get);
            dumpGet(message.get);
            break;
        case RpcMessage_Type_SET:
            assert(message.has_set);
            dumpSet(message.set);
            break;
        case RpcMessage_Type_SUBSCRIBE:
            assert(message.has_subscribe);
            dumpSubscribe(message.subscribe);
            break;
        case RpcMessage_Type_UNSUBSCRIBE:
            assert(message.has_unsubscribe);
            dumpUnsubscribe(message.unsubscribe);
            break;
        case RpcMessage_Type_FIRE:
            assert(message.has_fire);
            dumpFire(message.fire);
            break;
        case RpcMessage_Type_REPLY:
            assert(message.has_reply);
            dumpReply(message.reply);
            break;
        default:
            std::cout << "(unknown RpcMessage type)\n";
            break;
    }
}

int main () {
    uint8_t buf[128];

    {
        RpcMessage outMessage;
        memset(&outMessage, 0, sizeof(outMessage));

        outMessage.type = RpcMessage_Type_SET;
        outMessage.has_set = true;
        outMessage.set.messageId = 666;
        outMessage.set.name = 333;

        int32_t int32 = 123;

        pb_extension_t ext_int32;

        ext_int32.type = &UserValue_int32;
        ext_int32.dest = &int32;
        ext_int32.next = nullptr;

#if 1
        float float_ = 3.14159;

        pb_extension_t ext_float;
        ext_int32.next = &ext_float;

        ext_float.type = &UserValue_float;
        ext_float.dest = &float_;
        ext_float.next = nullptr;
#endif

        outMessage.set.value.extensions = &ext_int32;

        dumpRpcMessage(outMessage);

        auto ostream = pb_ostream_from_buffer(buf, sizeof(buf));
        std::cout << "ostream.bytes_written: " << ostream.bytes_written << '\n';
        if (!pb_encode(&ostream, RpcMessage_fields, &outMessage)) {
            std::cout << "encoding failed: " << PB_GET_ERROR(&ostream) << '\n';
        }
        std::cout << "ostream.bytes_written: " << ostream.bytes_written << '\n';
    }

    {
        RpcMessage inMessage;
        memset(&inMessage, 0, sizeof(inMessage));

        int32_t int32 = 987654321;
        float float_ = 1.23456789;

        pb_extension_t ext_int32;
        pb_extension_t ext_float;

        ext_int32.type = &UserValue_int32;
        ext_int32.dest = &int32;
        ext_int32.next = &ext_float;
        ext_int32.found = false;

        ext_float.type = &UserValue_float;
        ext_float.dest = &float_;
        ext_float.next = nullptr;
        ext_float.found = false;

        inMessage.set.value.extensions = &ext_int32;

        std::cout << "(before) int32 = " << int32 << ", float_ = " << float_ << '\n';

        auto istream = pb_istream_from_buffer(buf, sizeof(buf));
        std::cout << "istream.bytes_left: " << istream.bytes_left << '\n';
        if (!pb_decode(&istream, RpcMessage_fields, &inMessage)) {
            std::cout << "decoding failed: " << PB_GET_ERROR(&istream) << '\n';
        }
        std::cout << "istream.bytes_left: " << istream.bytes_left << '\n';

        std::cout << "(after) int32 = " << int32 << ", float_ = " << float_ << '\n';
        dumpRpcMessage(inMessage);
    }
}
