#ifndef RPC_SERVER_HPP
#define RPC_SERVER_HPP

#include "rpc.pb.hpp"

#include <rpc/stdlibheaders.hpp>
#include <rpc/enableif.hpp>
#include <rpc/componenttraits.hpp>
#include <rpc/status.hpp>
#include <rpc/version.hpp>

namespace rpc {

template <class T, class Interface>
class Server {
public:
    Server () { (void)AssertServerImplementsInterface<T, Interface>(); }

    template <class C>
    Status broadcast (C args, ONLY_IF(IsBroadcast<C>::value)) {
        auto message = barobo_rpc_ServerMessage{};
        message.type = barobo_rpc_ServerMessage_Type_BROADCAST;
        message.has_inReplyTo = false;
        message.has_broadcast = true;
        message.broadcast.id = componentId(C());

        auto stream = pb_ostream_from_buffer(message.broadcast.payload.bytes,
                sizeof(message.broadcast.payload.bytes));
        if (!nanopb::encode(stream, args)) {
            return Status::ENCODING_FAILURE;
        }

        message.broadcast.payload.size = stream.bytes_written;
        static_cast<T*>(this)->messageToClient(message);

        return Status::OK;
    }

    void refuseConnection (const barobo_rpc_ClientMessage& clMessage, barobo_rpc_ServerMessage& svMessage) {
        svMessage.type = barobo_rpc_ServerMessage_Type_REPLY;
        svMessage.has_reply = true;
        svMessage.reply.type = barobo_rpc_Reply_Type_STATUS;
        svMessage.reply.has_status = true;
        svMessage.reply.status.value = barobo_rpc_Status_CONNECTION_REFUSED;
        svMessage.has_inReplyTo = true;
        svMessage.inReplyTo = clMessage.id;
    }

    void refuseRequest (const barobo_rpc_ClientMessage& clMessage, barobo_rpc_ServerMessage& svMessage) {
        svMessage.type = barobo_rpc_ServerMessage_Type_REPLY;
        svMessage.has_reply = true;
        svMessage.reply.type = barobo_rpc_Reply_Type_STATUS;
        svMessage.reply.has_status = true;
        svMessage.reply.status.value = barobo_rpc_Status_NOT_CONNECTED;
        svMessage.has_inReplyTo = true;
        svMessage.inReplyTo = clMessage.id;
    }

    void receiveClientRequest (const barobo_rpc_ClientMessage& clMessage, barobo_rpc_ServerMessage& svMessage) {
        svMessage.type = barobo_rpc_ServerMessage_Type_REPLY;
        svMessage.has_inReplyTo = true;
        svMessage.inReplyTo = clMessage.id;

        svMessage.has_reply = true;
        switch (clMessage.request.type) {
            case barobo_rpc_Request_Type_CONNECT:
                svMessage.reply.type = barobo_rpc_Reply_Type_VERSIONS;
                svMessage.reply.has_versions = true;
                svMessage.reply.versions.rpc.major = Version<>::major;
                svMessage.reply.versions.rpc.minor = Version<>::minor;
                svMessage.reply.versions.rpc.patch = Version<>::patch;
                svMessage.reply.versions.interface.major = Version<Interface>::major;
                svMessage.reply.versions.interface.minor = Version<Interface>::minor;
                svMessage.reply.versions.interface.patch = Version<Interface>::patch;
                break;
            case barobo_rpc_Request_Type_DISCONNECT:
                svMessage.reply.type = barobo_rpc_Reply_Type_STATUS;
                svMessage.reply.has_status = true;
                svMessage.reply.status.value = barobo_rpc_Status_OK;
                break;
            case barobo_rpc_Request_Type_FIRE:
                if (!clMessage.request.has_fire) {
                    svMessage.reply.type = barobo_rpc_Reply_Type_STATUS;
                    svMessage.reply.has_status = true;
                    svMessage.reply.status.value = barobo_rpc_Status_PROTOCOL_ERROR;
                }
                else {
                    MethodInUnion<Interface> argument;
                    Status status;
                    argument.invoke(static_cast<T&>(*this),
                        clMessage.request.fire.id,
                        clMessage.request.fire.payload,
                        svMessage.reply.result.payload,
                        status);
                    if (hasError(status)) {
                        svMessage.reply.type = barobo_rpc_Reply_Type_STATUS;
                        svMessage.reply.has_status = true;
                        svMessage.reply.status.value = decltype(svMessage.reply.status.value)(status);
                    }
                    else {
                        svMessage.reply.type = barobo_rpc_Reply_Type_RESULT;
                        svMessage.reply.has_result = true;
                        svMessage.reply.result.id = clMessage.request.fire.id;
                    }
                }
                break;
            default:
                svMessage.reply.type = barobo_rpc_Reply_Type_STATUS;
                svMessage.reply.has_status = true;
                svMessage.reply.status.value = barobo_rpc_Status_PROTOCOL_ERROR;
                break;
        }
    }
};

} // namespace rpc

#endif
