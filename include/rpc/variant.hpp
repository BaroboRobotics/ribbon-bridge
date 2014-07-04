#ifndef RPC_VARIANT_HPP
#define RPC_VARIANT_HPP

namespace rpc {

template <class T, class U>
class Variant {
public:
    enum TypeCode {
        TYPE_T,
        TYPE_U,
        TYPE_OTHER
    };

    template <class V>
    Variant (V v) {
        set(v);
    }

    template <class V>
    bool isA () const {
        return typeCode(V()) == mTypeCode;
    }

    void set (T t) {
        mValue.t = t;
        mTypeCode = typeCode(T());
    }

    void set (U u) {
        mValue.u = u;
        mTypeCode = typeCode(U());
    }

    template <class V>
    void get () {
        if (typeCode(V()) == TYPE_T) {
            return mValue.t;
        }
        else if (typeCode(V()) == TYPE_U) {
            return mValue.u;
        }
        else {
            assert(false);
        }
    }

private:
    constexpr static TypeCode typeCode (T) { return TYPE_T; }
    constexpr static TypeCode typeCode (U) { return TYPE_U; }

    template <class V>
    constexpr static TypeCode typeCode (V) { return TYPE_OTHER; }

    TypeCode mTypeCode;
    union {
        T t;
        U u;
    } mValue;
};

template <class O, class E>
class MethodOutputVariant {
public:
    template <class T>
    MethodOutputVariant (T t) : mVariant(t) { }

    bool isOut () const {
        return mVariant.template isA<O>();
    }

    bool isError () const {
        return mVariant.template isA<E>();
    }

    O& getOut () {
        return mVariant.template get<O>();
    }

    E& getError () {
        return mVariant.template get<E>();
    }

private:
    Variant<O, E> mVariant;
};

} // namespace rpc

#endif
