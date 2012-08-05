/*----------------------------------------------------------------------------*
 * std::string => llvm::Type
 *----------------------------------------------------------------------------*/

#ifndef OpTypes_hh
#define OpTypes_hh

#include <map>

#include <llvm/Instructions.h>

namespace ska {

const unsigned TypeAnyID = llvm::Type::NumTypeIDs + 1;

std::map<std::string, unsigned> type_map = {
	// Primitive Types
	{ "void",			llvm::Type::VoidTyID },
	{ "half",			llvm::Type::HalfTyID },
	{ "float",			llvm::Type::FloatTyID },
	{ "double",			llvm::Type::DoubleTyID },
	{ "x87",				llvm::Type::X86_FP80TyID },

	// Derived Types
	{ "integer",		llvm::Type::IntegerTyID },
	{ "vector",			llvm::Type::VectorTyID },

	// Custom Types
	{ "any",				TypeAnyID }
};

} // namespace ska

#endif // OpTypes_hh
