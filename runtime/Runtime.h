#ifndef RUNTIME_RUNTIME_H_
#define RUNTIME_RUNTIME_H_

#include "RuntimeInterface.h"
#include "TypeDB.h"

#include <deque>
#include <map>

extern "C" {
void __typeart_alloc(void* addr, int typeId, size_t count, size_t typeSize, int isLocal);
void __typeart_free(void* addr);
void __typeart_enter_scope();
void __typeart_leave_scope();
}

namespace typeart {

struct PointerInfo {
  const void* addr;
  int typeId;
  size_t count;
  size_t typeSize;
};

class TypeArtRT {
 public:
  using TypeArtStatus = typeart_status;

  static TypeArtRT& get() {
    static TypeArtRT instance;
    return instance;
  }

  /**
   * Determines the type and array element count at the given address.
   * For nested types with classes/structs, the containing type is resolved recursively, until an exact with the address
   * is found.
   *
   * Note that this function will always return the outermost type lining up with the address.
   * Given a pointer to the start of a struct, the returned type will therefore be that of the struct, not of the first
   * member.
   *
   * Depending on the result of the query, one of the following status codes is returned:
   *  - TA_OK: The query was successful and the contents of type and count are valid.
   *  - TA_UNKNOWN_ADDRESS: The given address is either not allocated, or was not correctly recorded by the runtime.
   *  - TA_BAD_ALIGNMENT: The given address does not line up with the start of the atomic type at that location.
   *  - TA_INVALID_ID: Encountered unregistered ID during lookup.
   */
  TypeArtStatus getTypeInfo(const void* addr, typeart::TypeInfo* type, size_t* count) const;

  /**
   * Determines the outermost type and array element count at the given address.
   * Unlike in getTypeInfo(), there is no further resolution of subtypes.
   * Instead, additional information about the position of the address within the containing type is returned.
   *
   * \param[in] addr The address.
   * \param[out] count Number of elements in the containing buffer, not counting elements before the given address.
   * \param[out] baseAddress The index of the element within the containing buffer.
   * \param[out] offset The byte offset within that buffer element.
   *
   * \return A status code. For an explanation of errors, refer to getTypeInfo().
   *
   */
  TypeArtStatus getContainingTypeInfo(const void* addr, typeart::TypeInfo* type, size_t* count,
                                      const void** baseAddress, size_t* offset) const;

  /**
   * Determines the subtype at the given offset w.r.t. a base address and a corresponding containing type.
   * Note that if the subtype is itself a struct, you may have to call this function again.
   * If it returns with *subTypeOffset == 0, the address has been fully resolved.
   *
   * \param[in] baseAddr Pointer to the start of the containing type.
   * \param[in] offset Byte offset within the containing type.
   * \param[in] containerInfo typeart_struct_layout corresponding to the containing type
   * \param[out] subType TypeInfo corresponding to the subtype.
   * \param[out] subTypeBaseAddr Pointer to the start of the subtype.
   * \param[out] subTypeOffset Byte offset within the subtype.
   * \param[out] subTypeCount Number of elements in subarray.
   *
   * \return One of the following status codes:
   *  - TA_OK: Success.
   *  - TA_BAD_ALIGNMENT: Address corresponds to location inside an atomic type or padding.
   *  - TA_BAD_OFFSET: The provided offset is invalid.
   */
  TypeArtStatus getSubTypeInfo(const void* baseAddr, size_t offset, typeart_struct_layout containerInfo,
                               typeart::TypeInfo* subType, const void** subTypeBaseAddr, size_t* subTypeOffset,
                               size_t* subTypeCount) const;

  /**
   * Wrapper function using StructTypeInfo.
   */
  TypeArtStatus getSubTypeInfo(const void* baseAddr, size_t offset, const StructTypeInfo& containerInfo,
                               typeart::TypeInfo* subType, const void** subTypeBaseAddr, size_t* subTypeOffset,
                               size_t* subTypeCount) const;

  /**
   * Returns the builtin type at the given address.
   *
   * \param[in] addr The address.
   * \param[out] type The builtin type.
   * \return TA_OK, if the type is a builtin, TA_WRONG_KIND otherwise.
   */
  TypeArtStatus getBuiltinInfo(const void* addr, typeart::BuiltinType* type) const;

  /**
   * Given a type ID, this function provides information about the corresponding struct type.
   *
   * \param[in] id The type ID.
   * \param[out] structInfo Pointer to the StructTypeInfo corresponding to the type ID.
   *
   * \return One of the following status codes:
   *  - TA_OK: Sucess.
   *  - TA_WRONG_KIND: ID does not correspond to a struct type.
   *  - TA_INVALID_ID: ID is not valid.
   */
  TypeArtStatus getStructInfo(int id, const StructTypeInfo** structInfo) const;

  // TypeArtStatus resolveType(int id, int* len, typeart::TypeInfo* types[], int* count[], int* offsets[], int* extent);

  /**
   * Returns the name of the type corresponding to the given ID.
   * This can be used for debugging and error messages.
   *
   * \param[in] id The type ID.
   * \return The name of the type.
   */
  const std::string& getTypeName(int id) const;

  void onAlloc(const void* addr, int typeID, size_t count, size_t typeSize, bool isLocal);

  void onFree(const void* addr);

  void onEnterScope();

  void onLeaveScope();

 private:
  TypeArtRT();

  /**
   * If a given address points inside a known struct, this method is used to recursively resolve the exact member type.
   */
  TypeArtStatus getTypeInfoInternal(const void* baseAddr, size_t offset, const StructTypeInfo& containingType,
                                    typeart::TypeInfo* type, size_t* count) const;

  /**
   * Finds the struct member corresponding to the given byte offset.
   * If the offset is greater than the extent of the struct, the last member is returned.
   * Therefore, the caller must either ensure that the given offset is valid or explicitly check for this case.
   */
  size_t getMemberIndex(typeart_struct_layout structInfo, size_t offset) const;

  void printTraceStart();

  /**
   * Loads the type file created by the LLVM pass.
   */
  bool loadTypes(const std::string& file);

  /**
   * Given an address, this method searches for the pointer that corresponds to the start of the allocated block.
   * Returns null if the memory location is not registered as allocated.
   */
  const void* findBaseAddress(const void* addr) const;

  // Class members

  TypeDB typeDB;

  std::map<const void*, PointerInfo> typeMap;

  std::deque<std::vector<const void*>> scopes;

  static std::string defaultTypeFileName;
};

}  // namespace typeart

#endif