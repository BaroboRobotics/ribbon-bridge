#ifndef RPC_STDLIBHEADERS_HPP
#define RPC_STDLIBHEADERS_HPP

#if __GNUC__ && __AVR__
// Use avr-libc
# include <assert.h>
# include <stdio.h>
# include <stdlib.h>
# include <stdint.h>
# include <inttypes.h>
#else
// Use the standard C++ library
# include <cassert>
# include <cstdio>
# include <cstdlib>
# include <cstdint>
# include <cinttypes>
#endif


#endif
