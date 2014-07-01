#ifndef RPC_POTQUEUE_HPP
#define RPC_POTQUEUE_HPP

#include "rpc/potringbuffer.hpp"

namespace rpc {

template <class T, size_t N>
class PotQueue {
public:
    bool empty () const { return mRingbuffer.empty(); }
    bool full () const { return mRingbuffer.full(); }
    T& front () { return mRingbuffer.front(); }
    void push (const T& elem) { mRingbuffer.pushBack(elem); }
    void pop () { mRingbuffer.popFront(); }

private:
    PotRingbuffer<T, N> mRingbuffer;
};

} // namespace rpc

#endif
