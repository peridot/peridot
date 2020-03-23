#ifndef _PERIDOT_VALUE_H
#define _PERIDOT_VALUE_H

#include <stdint.h>
#include "object.h"
#include "peridot.h"
#include "debug.h"
#include "dyn_arr.h"
PERIDOT_EXTERN_C_BEGIN

// TODO This file doesn't really follow the naming convention of PD_ prefix, fix this.

typedef uint64_t pd_value;

// Value representation with NaN Boxing
// Sign (1 Bit)
// | Exponent (11 Bits)
// | |           Mantissa (52 Bits)
// | |           |
// S EEEEEEEEEEE MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
// First bit of mantissa is the 'is_quiet' flag, it is always set to 1 to make the NaN a quiet NaN
// Leaving us with 51 Bits of space for a "payload" where we stuff our things.
// All exponent bits are set for NaN values, if it is unset it is a normal double.
// Pointers on 64 Bits machines are actually using 48 Bits so it can fit on our 51 Bit space
// Our Payload still has 3 Bits left so we use them to tag our types.
// Summing up all the above points our mantissa looks like:
//
// "is_quiet" flag (1 Bit)
// |                                                  Type tag (3 Bits)
// | Payload (48 Bits)                                |
// | |                                                |
// Q MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM TTT

// Represents the sign bit
#define SIGN_BIT ((uint64_t)1 << 63)

// The Base Quiet NaN with empty payload and no sign bit
#define QNAN (0x7ffc000000000000)

// To know if the value is a double we check if it is a NaN by checking if the exponent bits are set.
#define IS_DOUBLE(v) (((v) & QNAN) != QNAN)

// Objects are identified with the sign bit.
#define IS_OBJECT(v) (((v) & (QNAN | SIGN_BIT)) == (QNAN | SIGN_BIT))

// Represents the types a value can be
typedef enum {
  PD_TRUE = 0, // 0
  PD_FALSE,    // 1
  PD_NULL,     // 2
  PD_NUMBER,   // 3
  PD_OBJECT,    // 4
  PD_UNDEFINED  // 5
} pd_vtype;

// The mask for the 3 Bits of type tag.
#define TAG_MASK (0x7)

// Retrieve the type tag
#define PD_TAG(v) ((int)((v) & TAG_MASK))

// Get typeof a value, doubles and objects aren't exactly tagged so we check for those.
#define PD_TYPE(v) (IS_DOUBLE(v) ? PD_NUMBER : IS_OBJECT(v) ? PD_OBJECT : PD_TAG(v))

// Singleton Constants, booleans and null
#define TRUE_VALUE ((pd_value)(uint64_t)(QNAN | PD_TRUE))
#define FALSE_VALUE ((pd_value)(uint64_t)(QNAN | PD_FALSE))
#define NULL_VALUE ((pd_value)(uint64_t)(QNAN | PD_NULL))

// Undefined is a special value, it isn't exposed to the user but used internally by the VM
// to represent undefined values, we could use null but null is a valid value
// unlike JavaScript undefined is a truly undefined value.
#define UNDEFINED_VALUE ((pd_value)(uint64_t)(QNAN | PD_UNDEFINED))
#define IS_UNDEFINED(v) ((v) == UNDEFINED_VALUE)
// Not needed but allows us to change value representations anytime and not redo the checks.
#define IS_NULL(v) ((v) == NULL_VALUE)

// Converts a C boolean to a value.
#define BOOL_VAL(v) ((v) ? TRUE_VALUE : FALSE_VALUE)

// Checks if a value is a literal true or false
#define IS_BOOL(v) ((v) == TRUE_VALUE || (v) == FALSE_VALUE)

// Cast a value to a C boolean, everything is true except for false and null and zero
//#define AS_BOOL(v) (PD_TAG(v) != PD_FALSE && PD_TAG(v) != PD_NULL && (IS_DOUBLE(v) ? AS_DOUBLE(v) != 0.0 : true))

// Constructs an object value given a pointer.
#define PD_FROM(ptr) ((pd_value)(SIGN_BIT | QNAN | (uint64_t)(uintptr_t)(ptr)))

// Unboxes a value to an object pointer
#define AS_OBJECT(ptr) ((pd_object*)(uintptr_t)((ptr) & ~(SIGN_BIT | QNAN)))

// Boxes a double.
#define NUMBER_VAL(n) (pd_number_value((n)))

// Unboxes a double
#define AS_DOUBLE(v) (pd_as_double((v)))
// Boxes a double
#define DOUBLE_VAL(v) (pd_number_value((v)))

// The annoying part of NaN boxing is the casting of double to bits and the other way.
// We do this by creating a union that holds the bits and num
// The C spec allows accessing an unused union field by reinterpreting the bits of the used one
// therefore casting our double.

union pd_double_bits {
  double num;
  uint64_t bits;
};

// Linter on my editor is killing me so we disable the unused warning here.
// This warning isn't produced on compile time just on the header itself while i'm editing it...
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#else
// Assume GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif // __clang__

static PD_INLINE pd_value pd_number_value(double n) {
  union pd_double_bits bits;
  bits.num = n;
  return bits.bits;
}

static PD_INLINE double pd_as_double(pd_value value) {
  pd_assert(IS_DOUBLE(value), "Attempt to get double value out of a non-double value.");
  union pd_double_bits bits;
  bits.bits = value;
  return bits.num;
}

// Cast a value to a C boolean, everything is true except for false and null and zero
// This is an inline fn instead of a macro because if it was a macro it would evaluate the expression multiples times
// e.g AS_BOOL(pvm_pop(vm)) would pop more than once!
// So we put it in a function and hope the compiler will inline it for us.
static PD_INLINE bool AS_BOOL(pd_value value) {
  return (PD_TAG(value) != PD_FALSE && PD_TAG(value) != PD_NULL && (IS_DOUBLE(value) ? AS_DOUBLE(value) != 0.0 : true));
}

#ifdef __clang__
#pragma clang diagnostic pop
#else
#pragma GCC diagnostic pop
#endif // __clang__

// Forward declare VM to avoid dependency cycle since including VM includes this file.
typedef struct pvm_t pvm_t;

// Dynamic array that holds values.
// TODO: This is used for storing constants in a chunk, reuse this to also implement arrays in the language itself.
PERIDOT_DEC_DYN_ARR(pd_value_array, pd_value)

PERIDOT_EXTERN_C_END

#endif // _PERIDOT_VALUE_H
