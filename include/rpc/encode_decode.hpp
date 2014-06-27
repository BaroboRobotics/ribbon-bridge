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

/* TODO: use a PbFields metafunction to get the pb_field_t* in a typesafe
 * way. Wrap these functions in a template function which uses the PbFields
 * metafunction to infer the pbFields argument. */
bool encodeProtobuf (const void* pbStruct, const pb_field_t* pbFields, uint8_t* bytes, size_t size, size_t& bytesWritten);

bool decodeProtobuf (void* pbStruct, const pb_field_t* pbFields, uint8_t* bytes, size_t size);

} // namespace rpc

#endif
