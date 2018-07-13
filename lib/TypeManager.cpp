//
// Created by sebastian on 27.04.18.
//

#include "TypeManager.h"
#include "TypeIO.h"
#include "support/Logger.h"
#include "support/TypeUtil.h"
#include "support/Util.h"

#include <iostream>

namespace tu = typeart::util::type;

namespace typeart {

using namespace llvm;

TypeManager::TypeManager(std::string file) : file(file), structCount(0) {
}

bool TypeManager::load() {
  TypeIO cio(typeDB);
  if (cio.load(file)) {
    structMap.clear();
    for (auto& structInfo : typeDB.getStructList()) {
      structMap.insert({structInfo.name, structInfo.id});
    }
    structCount = structMap.size();
    return true;
  }
  return false;
}

bool TypeManager::store() {
  TypeIO cio(typeDB);
  return cio.store(file);
}

int TypeManager::getOrRegisterType(llvm::Type* type, const llvm::DataLayout& dl) {
  auto& c = type->getContext();
  switch (type->getTypeID()) {
    case llvm::Type::IntegerTyID:

      if (type == Type::getInt8Ty(c)) {
        return TA_INT8;
      } else if (type == Type::getInt16Ty(c)) {
        return TA_INT16;
      } else if (type == Type::getInt32Ty(c)) {
        return TA_INT32;
      } else if (type == Type::getInt64Ty(c)) {
        return TA_INT64;
      } else {
        return TA_UNKNOWN_TYPE;
      }
    // TODO: Unsigned types are not supported as of now
    case llvm::Type::FloatTyID:
      return TA_FLOAT;
    case llvm::Type::DoubleTyID:
      return TA_DOUBLE;
    case llvm::Type::FP128TyID:
      return TA_FP128;
    case llvm::Type::X86_FP80TyID:
      return TA_X86_FP80;
    case llvm::Type::PPC_FP128TyID:
      return TA_PPC_FP128;
    case llvm::Type::StructTyID:
      return getOrRegisterStruct(dyn_cast<StructType>(type), dl);
    default:
      break;
  }
  return TA_UNKNOWN_TYPE;
}

int TypeManager::getOrRegisterStruct(llvm::StructType* type, const llvm::DataLayout& dl) {
  const auto getName = [](auto type) -> std::string {
    if (type->isLiteral()) {
      return "LiteralS" + std::to_string(reinterpret_cast<long int>(type));
    }
    return type->getStructName();
  };

  auto name = getName(type);
  // std::cerr << "Looking up struct " << name.str() << std::endl;
  auto it = structMap.find(name);
  if (it != structMap.end()) {
    // std::cerr << "Found!" << std::endl;
    return it->second;
  }

  // std::cerr << "Registered structs: " << std::endl;
  // for (auto info : typeDB.getStructList()) {
  //  std::cerr << info.name <<", " << info.id << std::endl;
  //}

  // Get next ID and register struct
  int id = N_BUILTIN_TYPES + structCount;
  structCount++;

  size_t n = type->getStructNumElements();

  std::vector<size_t> offsets;
  std::vector<size_t> arraySizes;
  std::vector<TypeInfo> memberTypeInfo;

  const StructLayout* layout = dl.getStructLayout(type);

  for (uint32_t i = 0; i < n; i++) {
    auto memberType = type->getStructElementType(i);
    int memberID = TA_UNKNOWN_TYPE;
    TypeKind kind;

    size_t arraySize = 1;

    if (memberType->isArrayTy()) {
      arraySize = memberType->getArrayNumElements();
      memberType = memberType->getArrayElementType();
    }

    if (memberType->isStructTy()) {
      kind = STRUCT;
      if (memberType->getStructName() == name) {
        memberID = id;
      } else {
        // TODO: Infinite cycle possible?
        memberID = getOrRegisterType(memberType, dl);
      }
    } else if (memberType->isPointerTy()) {
      kind = POINTER;
      // TODO: Do we need a type ID for pointers?
    } else if (memberType->isSingleValueType()) {
      kind = BUILTIN;
      memberID = getOrRegisterType(memberType, dl);
    } else {
      // TODO: Any other types?
      LOG_ERROR("Encountered unhandled type: " << typeart::util::dump(*memberType));
      assert(false && "Encountered unhandled type");
    }

    size_t offset = layout->getElementOffset(i);
    offsets.push_back(offset);
    arraySizes.push_back(arraySize);
    memberTypeInfo.push_back({kind, memberID});
  }

  size_t numBytes = layout->getSizeInBytes();

  StructTypeInfo structInfo{id, name, numBytes, n, offsets, memberTypeInfo, arraySizes};
  typeDB.registerStruct(structInfo);

  structMap.insert({name, id});
  return id;
}
}  // namespace typeart
