#ifndef RPC_BUFFER_HPP
#define RPC_BUFFER_HPP

#include "rpc.pb.h"

#include <pb_encode.h>
#include <pb_decode.h>

#define RPC_MESSAGE_MAX_SIZE 256

namespace rpc {

/* Implementation details */
namespace detail {

template <typename PbStruct>
struct PbFields;

} // namespace detail

/* Buffer to hold encoded rpc messages. */
class Buffer {
public:
    /* TODO: use a PbFields metafunction to get the pb_field_t* in a typesafe
     * way. */
    template <typename PbStruct>
    bool encode (const PbStruct& pbStruct, const pb_field_t* pbFields) {
        auto stream = pb_ostream_from_buffer(mBytes, sizeof(mBytes));
        bool success = true;
        if (!pb_encode(&stream, pbFields, &pbStruct)) {
            success = false;
            printf("encoding failed: %s\n", PB_GET_ERROR(&stream));
        }
        else {
            printf("encoded %zu bytes\n", stream.bytes_written);
        }
        mSize = stream.bytes_written;
        return success;
    }

    template <typename PbStruct>
    bool decode (PbStruct& pbStruct, const pb_field_t* pbFields) {
        auto stream = pb_istream_from_buffer(mBytes, mSize);
        bool success = true;
        auto nBytesLeft = stream.bytes_left;
        if (!pb_decode(&stream, pbFields, &pbStruct)) {
            success = false;
            printf("decoding failed: %s\n", PB_GET_ERROR(&stream));
        }
        else {
            printf("decoded %zu bytes\n", nBytesLeft - stream.bytes_left);
        }
        return success;
    }


    size_t size () const { return mSize; }
    const uint8_t* bytes () const { return mBytes; }

private:
    size_t mSize;
    uint8_t mBytes[RPC_MESSAGE_MAX_SIZE];
};

} // namespace rpc

#endif
