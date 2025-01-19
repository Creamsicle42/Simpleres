#ifndef TYPES_H_
#define TYPES_H_


// Rust style type definition, just for personal prefference


#include <stdint.h>
typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;



typedef struct {
	i8 *data;
	u64 len;
} i8_slice;

typedef struct {
	i16 *data;
	u64 len;
} i16_slice;

typedef struct {
	i32 *data;
	u64 len;
} i32_slice;

typedef struct {
	i64 *data;
	u64 len;
} i64_slice;

typedef struct {
	u8 *data;
	u64 len;
} u8_slice;

typedef struct {
	u16 *data;
	u64 len;
} u16_slice;

typedef struct {
	u32 *data;
	u64 len;
} u32_slice;

typedef struct {
	u64 *data;
	u64 len;
} u64_slice;

#endif
