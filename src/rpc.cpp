#include "stdlibheaders.hpp"

#include "rpc.pb.h"
//#include "gen-rpc.pb.hpp"
#include "potqueue.hpp"

#include <pb_encode.h>
#include <pb_decode.h>

struct Buffer {
    uint8_t buffer[256];
};

void onGet (const Get& get) {
    printf("GET { messageId : %" PRId32 ", id : %" PRId32 " }\n", get.messageId, get.id);
}

void onSet (const Set& set) {
    printf("SET { messageId : %" PRId32
               ", objectId : %" PRId32
               ", interfaceId : %" PRId32
               ", id : %" PRId32,
               set.messageId, set.objectId, set.interfaceId, set.id);

    if (set.value.size) {
        printf(", value :");
        for (size_t i = 0; i < set.value.size; ++i) {
            printf(" %x", int(set.value.bytes[i]));
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
    outMessage.set.objectId = 123;
    outMessage.set.interfaceId = 456;
    outMessage.set.id = 333;
    outMessage.set.value.bytes[0] = 0x99;
    outMessage.set.value.size = 1;

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

bool recvRpcMessage (const size_t size, Buffer& buffer) {
    RpcMessage rpcMessage;
    memset(&rpcMessage, 0, sizeof(rpcMessage));

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
    recvRpcMessage(sizeQueue.front(), bufferQueue.front());

    sizeQueue.pop();
    bufferQueue.pop();
}
