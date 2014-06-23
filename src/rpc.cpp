#if __GNU__ && __AVR__
#define assert(x)
#else
#include <cassert>
#endif

#include "rpc.pb.h"
#include "gen-rpc.pb.hpp"
#include "potqueue.hpp"

#include <pb_encode.h>
#include <pb_decode.h>

#include <cstdio>

#include <inttypes.h>

struct Buffer {
    uint8_t buffer[256];
};

void onGet (const Get& get) {
    printf("GET { messageId : %" PRId32 ", id : %" PRId32 " }\n", get.messageId, get.id);
}

void onSet (const Set& set) {
    printf("SET { messageId : %" PRId32 ", id : %" PRId32, set.messageId, set.id);

    for (auto ext = set.value.extensions; ext; ext = ext->next) {
        if (ext->found) {
            auto& u = rpc::getStaticExtensionUnion<rpc::extensions::com::barobo::Robot>();
            (void)u;
        }
    }
    printf(" }\n");
}

void onSubscribe (const Subscribe& subscribe) {
    (void)subscribe;
}

void onUnsubscribe (const Unsubscribe& unsubscribe) {
    (void)unsubscribe;
}

void onFire (const Fire& fire) {
    (void)fire;
}

//////////////////////////////////////////////////////////////////////////////

void onBroadcast (const Broadcast& broadcast) {
    (void)broadcast;
}

void onReply (const Reply& reply) {
    (void)reply;
}

//////////////////////////////////////////////////////////////////////////////

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
        case RpcMessage_Type_BROADCAST:
            assert(message.has_broadcast);
            onBroadcast(message.broadcast);
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

//////////////////////////////////////////////////////////////////////////////

template <class SQ, class MQ>
bool sendSet (SQ& sizeQueue, MQ& messageQueue) {
    RpcMessage outMessage;
    memset(&outMessage, 0, sizeof(outMessage));

    outMessage.type = RpcMessage_Type_SET;
    outMessage.has_set = true;
    outMessage.set.messageId = 666;
    outMessage.set.id = 333;

    int32_t int32 = 123;

    pb_extension_t ext_int32;

    ext_int32.type = &UserValue_int32;
    ext_int32.dest = &int32;
    ext_int32.next = nullptr;
    ext_int32.found = true;

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

    Buffer buf;
    auto ostream = pb_ostream_from_buffer(buf.buffer, sizeof(buf));
    bool success = true;
    if (!pb_encode(&ostream, RpcMessage_fields, &outMessage)) {
        success = false;
        printf("encoding failed: %s\n", PB_GET_ERROR(&ostream));
    }
    sizeQueue.push(ostream.bytes_written);
    messageQueue.push(buf);

    return success;
}

//////////////////////////////////////////////////////////////////////////////

bool recvRpcMessage (const size_t size, Buffer& buffer,
        pb_extension_t* extensions) {
    RpcMessage rpcMessage;
    memset(&rpcMessage, 0, sizeof(rpcMessage));

    rpcMessage.set.value.extensions = extensions;

    auto istream = pb_istream_from_buffer(buffer.buffer, size);
    bool success = true;
    if (!pb_decode(&istream, RpcMessage_fields, &rpcMessage)) {
        success = false;
        printf("decoding failed: %s\n", PB_GET_ERROR(&istream));
    }
    else {
        onRpcMessage(rpcMessage);
    }

    return success;
}

int main () {
    PotQueue<Buffer, 2> bufferQueue;
    PotQueue<size_t, 2> sizeQueue;

    sendSet(sizeQueue, bufferQueue);
    recvRpcMessage(sizeQueue.front(), bufferQueue.front(),
            &rpc::getStaticExtensionChain<rpc::extensions::com::barobo::Robot>());

    sizeQueue.pop();
    bufferQueue.pop();
}
