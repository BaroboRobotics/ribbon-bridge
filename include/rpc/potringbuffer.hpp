#ifndef RPC_POTRINGBUFFER_HPP
#define RPC_POTRINGBUFFER_HPP

namespace rpc {

/* Power-of-two sized ringbuffer. */
template <class T, size_t N>
class PotRingbuffer {
    static_assert(N, "PotRingbuffer capacity must be greater than zero");
    static_assert(!(N % 2), "PotRingbuffer capacity must be a power of two");

public:
    size_t size () const {
        return full() ? N : (mEnd - mBegin) & (N - 1);
    }
    
    bool empty () const {
        return mBegin == mEnd;
    }

    bool full () const {
        return (mBegin ^ N) == mEnd;
    }

    /* Array-like access, counting forward from begin. */
    T& at (size_t index) {
        return wrappedAccess(mBegin + index);
    }

    /* Array-like access, counting backward from end. */
    T& reverseAt (size_t index) {
        return wrappedAccess(mEnd - index);
    }

    /* Array-like access, where negative indices count backward from end. */
    T& comboAt (size_t index) {
        return wrappedAccess((index < 0 ? mEnd : mBegin) + index);
    }

    T& front () {
        return at(0);
    }

    T& back () {
        return reverseAt(1);
    }

    void pushBack (const T& elem) {
        if (full()) {
            incr(mBegin);
        }
        incr(mEnd);
        back() = elem;
    }

    void pushFront (const T& elem) {
        if (full()) {
            decr(mEnd);
        }
        decr(mBegin);
        front() = elem;
    }

    void popFront () {
        assert(!empty());
        incr(mBegin);
    }

    void popBack () {
        assert(!empty());
        decr(mEnd);
    }

private:
    T& wrappedAccess (size_t index) {
        return mData[index & (N - 1)];
    }

    void add (size_t& beginOrEnd, size_t amount) {
        beginOrEnd = (beginOrEnd + amount) & (2 * N - 1);
    }

    void incr (size_t& beginOrEnd) {
        add(beginOrEnd, 1);
    }

    void decr (size_t& beginOrEnd) {
        add(beginOrEnd, -1);
    }

    size_t mBegin = 0;
    size_t mEnd = 0;
    T mData[N];
};

} // namespace rpc

#endif
