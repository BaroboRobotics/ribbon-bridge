#ifndef IS_BASE_OF_HPP
#define IS_BASE_OF_HPP

/* No STL available, so roll our own std::is_base_of. Stolen from
 * http://stackoverflow.com/questions/2910979/how-does-is-base-of-work
 * which in turn is apparently a simplification of Boost's implementation. */
template <class Base, class Derived>
struct is_base_of {
    typedef char (&yes)[1];
    typedef char (&no)[2];

    template <class B, class D>
    struct Host {
        operator B*() const;
        operator D*();
    };

    template <class T> 
    static yes check(Derived*, T);
    static no check(Base*, int);

    static const bool value = sizeof(check(Host<Base,Derived>(), int())) == sizeof(yes);
};

#endif
