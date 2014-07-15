#ifndef FULLDUPLEXTRANSPORT_HPP
#define FULLDUPLEXTRANSPORT_HPP

#include "rpc/buffer.hpp"

#include <boost/lockfree/spsc_queue.hpp>

#include <atomic>
#include <chrono>
#include <functional>
#include <thread>

#include <cassert>

class FullDuplexTransport {
public:
    FullDuplexTransport ()
        : mServiceToProxyThread(&FullDuplexTransport::serviceToProxy, this)
        , mProxyToServiceThread(&FullDuplexTransport::proxyToService, this) { }

    ~FullDuplexTransport () {
        mKillThreads = true;
        mProxyToServiceThread.join();
        mServiceToProxyThread.join();
    }

    std::function<void(const rpc::Buffer<256>&)>
    replyPoster () {
        return [this] (const rpc::Buffer<256>& payload) {
            auto success = mServiceToProxyQueue.push(payload);
            assert(success);
        };
    }

    std::function<void(const rpc::Buffer<256>&)>
    requestPoster () {
        return [this] (const rpc::Buffer<256>& payload) {
            auto success = mProxyToServiceQueue.push(payload);
            assert(success);
        };
    }

    void onRequest (std::function<void(const rpc::Buffer<256>&)> handler) {
        mRequestHandler = handler;
    }

    void onReply (std::function<void(const rpc::Buffer<256>&)> handler) {
        mReplyHandler = handler;
    }

private:
    std::function<void(const rpc::Buffer<256>&)> mReplyHandler;
    std::function<void(const rpc::Buffer<256>&)> mRequestHandler;

    void serviceToProxy () {
        while (!mKillThreads) {
            if (mReplyHandler) {
                mServiceToProxyQueue.consume_all(mReplyHandler);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    void proxyToService () {
        while (!mKillThreads) {
            if (mRequestHandler) {
                mProxyToServiceQueue.consume_all(mRequestHandler);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    using Queue = boost::lockfree::spsc_queue<rpc::Buffer<256>, boost::lockfree::capacity<16>>;

    Queue mProxyToServiceQueue;
    Queue mServiceToProxyQueue;

    std::atomic_bool mKillThreads = { false };

    std::thread mServiceToProxyThread;
    std::thread mProxyToServiceThread;
};

#endif
