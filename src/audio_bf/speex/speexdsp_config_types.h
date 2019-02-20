#ifndef __SPEEX_TYPES_H__
#define __SPEEX_TYPES_H__

#if defined HAVE_STDINT_H
#  include <stdint.h>
#elif defined HAVE_INTTYPES_H
#  include <inttypes.h>
#elif defined HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif

typedef short spx_int16_t;
typedef unsigned short spx_uint16_t;
typedef long spx_int32_t;
typedef unsigned long spx_uint32_t;


#endif
