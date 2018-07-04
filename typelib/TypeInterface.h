#ifndef LLVM_MUST_SUPPORT_TYPEINTERFACE_H
#define LLVM_MUST_SUPPORT_TYPEINTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

// TODO: Support for missing types (e.g. long double)
// Type UNKNOWN is used for pointer types, when the underlying type is not specified.
// In conjunction with kind BUILTIN, UNKNOWN signifies an invalid type.
typedef enum typeart_builtin_type_t {
  C_CHAR = 0,
  C_UCHAR = 1,
  C_SHORT = 2,
  C_USHORT = 3,
  C_INT = 4,
  C_UINT = 5,
  C_LONG = 6,
  C_ULONG = 7,
  C_FLOAT = 8,
  C_DOUBLE = 9,
  UNKNOWN = 10,
  N_BUILTIN_TYPES
} typeart_builtin_type;

typedef enum typeart_type_kind_t { BUILTIN, STRUCT, POINTER } typeart_type_kind;

typedef struct typeart_type_info_t {
  typeart_type_kind kind;
  int id;
} typeart_type_info;

#ifdef __cplusplus
}
#endif

#endif  // LLVM_MUST_SUPPORT_TYPEINTERFACE_H