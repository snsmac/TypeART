#ifndef TYCART_H
#define TYCART_H

#ifdef __cplusplus
#include <cstddef>
extern "C" {
#else
#include <stddef.h>
#endif  // __cplusplus

/*
 * Private functions, should only be accessed through the TyCart Macros
 */

/*
 * In TyCart this function first checks the assert, then registers the memory region
 * into the TyCart runtime table, i.e., we can run the asserts before a checkpoint
 * again.
 * Then calls the target-CPR library specific register function.
 */
void __tycart_assert(int id, void* addr, size_t count, size_t typeSize, int typeId);
/*
 * The stub is inserted with Macro expansion. It is replaced by the TyCart compiler
 * pass to the actual RT call.
 */
void __tycart_assert_stub(void* pointer, void* __stub_ptr, size_t count, int id);

void __tycart_assert_auto(int id, void* pointer, size_t typeSize, int typeId);
void __tycart_assert_auto_stub(void* pointer, void* stub_ptr, int id);

/*
 * Iterates the stored CP map, to re-assert all stored assumptions before calling the final
 * checkpointing mechanism.
 */
void __tycart_cp_assert();

/*
 * Used to deregister a memory region for checkpointing, as supported by the
 * backend libraries.
 */
void __tycart_deregister_mem(int id);

/*
 * Given the type ID, it registers the FTI user-defined type to our Runtimne
 */
void __tycart_register_FTI_t(int typeId);

/*
 * Stub that is replaced by the TypeART compiler pass.
 */
void __tycart_register_FTI_t_stub(void* ptr);

/*
 * Returns the number of elements allocated
 */
size_t __tycart_alloc_count(void* ptr);

#ifdef __cplusplus
}
#endif  // __cplusplus

/*
 * Define the used backend library based on the include in user code.
 */
#ifdef __VELOC_H
#define VELOC_CP(name, version) VELOC_Checkpoint(name, version);

#define FTI_CP(id, level)
#define MCPR_CP(name, version)
#endif

#ifdef __FTI_H__
#define FTI_CP(id, level) FTI_Checkpoint(id, level);

#define VELOC_CP(name, version)
#define MCPR_CP(name, version)
#endif

#ifdef MINI_CPR_MINI_CPR_H
#define MCPR_CP(name, version) mini_cpr_checkpoint(name, version);

#define VELOC_CP(name, version)
#define FTI_CP(id, level)
#endif

// Taken from
// https://stackoverflow.com/questions/1597007/creating-c-macro-with-and-line-token-concatenation-with-positioning-macr
#define TOKENPASTE(x, y) x##y
#define PASTELINE(x, y) TOKENPASTE(x, y)

// clang-format off
//

// Common interface to init the different libraries
#define TY_init(__cfg_file) __tycart_init(__cfg_file);

// mark a pointer for checkpointing
#define TY_protect(id, pointer, count, type)                            \
  {                                                                         \
    type* PASTELINE(__stub_ptr_, __LINE__) = NULL; __tycart_assert_stub((void*)pointer, PASTELINE(__stub_ptr_, __LINE__), count, id); \
  }

// mark a pointer for checkpointing. TyCart determines the size of the allocation
#define TY_protect_auto(id, pointer, type)                            \
  {                                                                         \
    type* PASTELINE(__stub_ptr_, __LINE__) = NULL; __tycart_assert_auto_stub((void*)pointer, PASTELINE(__stub_ptr_, __LINE__), id); \
  }

// Take the ckeckpoint
#define TY_checkpoint(name, id, version, level) \
  __tycart_cp_assert();                         \
  VELOC_CP(name, version)                      \
  FTI_CP(id, level)                            \
  MCPR_CP(name, version)

// Specifically for FTI: register a type
#define TY_register_type(type)                            \
  {                                                       \
    type* PASTELINE(__stub_ptr_, __LINE__) ; __tycart_register_FTI_t_stub((void*) PASTELINE(__stub_ptr_, __LINE__) ); \
  }

// Remove a region from being marked for checkpointing
#define TY_unregister_mem(id) __tycart_deregister_mem(id);

// Common API for restart
#define TY_recover() { __tycart_cp_recover(); }

// Returns the number of elements allocated
#define TY_alloc_count(pointer)  __tycart_alloc_count(pointer)

//
// clang-format on

#endif  // header guard
