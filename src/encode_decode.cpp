#include "rpc/stdlibheaders.hpp"
#include "rpc/encode_decode.hpp"

#include "pb_encode.h"
#include "pb_decode.h"

namespace rpc {

/* TODO: use a PbFields metafunction to get the pb_field_t* in a typesafe
 * way. Wrap these functions in a template function which uses the PbFields
 * metafunction to infer the pbFields argument. */
bool encodeProtobuf (const void* pbStruct, const pb_field_t* pbFields, uint8_t* bytes, size_t size, size_t& bytesWritten) {
    printf("encoding to buffer of size %zu\n", size);
    auto stream = pb_ostream_from_buffer(bytes, size);
    bool success = true;
    if (!pb_encode(&stream, pbFields, pbStruct)) {
        success = false;
        printf("encoding failed: %s\n", PB_GET_ERROR(&stream));
    }
    else {
    }
        printf("encoded %zu bytes\n", stream.bytes_written);
    bytesWritten = stream.bytes_written;
    return success;
}

bool decodeProtobuf (void* pbStruct, const pb_field_t* pbFields, uint8_t* bytes, size_t size) {
    auto stream = pb_istream_from_buffer(bytes, size);
    bool success = true;
    auto nBytesLeft = stream.bytes_left;
    if (!pb_decode(&stream, pbFields, pbStruct)) {
        success = false;
        printf("decoding failed: %s\n", PB_GET_ERROR(&stream));
    }
    else {
        printf("decoded %zu bytes\n", nBytesLeft - stream.bytes_left);
    }
    return success;
}

} // namespace rpc
