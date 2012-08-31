/*----------------------------------------------------------------------------*
 * Copyright (c) 2012 Los Alamos National Security, LLC
 * All rights reserved
 *----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*
 * Simulator class.
 *----------------------------------------------------------------------------*/

#ifndef Simulator_hh
#define Simulator_hh

#if defined(USE_MANGLED_CALL_NAMES)
#include <cxxabi.h>
#endif

#include <string>
#include <cstring>
#include <vector>
#include <list>

#include <llvm/Module.h>
#include <llvm/Constants.h>
#include <llvm/Function.h>
#include <llvm/Instruction.h>
#include <llvm/Instructions.h>
#include <llvm/LLVMContext.h>
#include <llvm/ADT/APInt.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/IRReader.h>
#include <llvm/Support/InstIterator.h>

#include <Decode.hh>
#include <Instruction.hh>
#include <MachineState.hh>
#include <Statistics.hh>
#include <OpCodes.hh>
#include <OpTypes.hh>
#include <Core.hh>
#include <Utils.hh>

namespace ska {

#ifndef SKA_VERSION_TAG
#define SKA_VERSION_TAG "undefined"
#endif

/*----------------------------------------------------------------------------*
 * Simulator class.
 *----------------------------------------------------------------------------*/

class simulator_t
{
public:

	/*-------------------------------------------------------------------------*
	 * Public types
	 *-------------------------------------------------------------------------*/

	typedef std::map<llvm::Value *, instruction_t *> instruction_map_t;
	typedef std::vector<instruction_t *> instruction_vector_t;
	typedef std::list<instruction_t *> instruction_list_t;

	/*-------------------------------------------------------------------------*
	 * Constructor
	 *-------------------------------------------------------------------------*/

	simulator_t(const char * ir_file, std::ostream & stream);

	/*-------------------------------------------------------------------------*
	 * Destructor
	 *-------------------------------------------------------------------------*/

	~simulator_t() {}

private:

#if 0
	/*-------------------------------------------------------------------------*
	 * Decode an llvm::Instruction.  This gets property information about
	 * the instruction (latency, issue latency, opcode, optype) that is used
	 * to contruct a instruction_t instance.
	 *-------------------------------------------------------------------------*/

	instruction_properties_t decode(llvm::Instruction * instruction);
#endif
	
	/*-------------------------------------------------------------------------*
	 * Update statistic based on the execution of 'instruction'.
	 *-------------------------------------------------------------------------*/

	void update_stats(llvm::Instruction * instruction);

	/*-------------------------------------------------------------------------*
	 * Compute the number of bytes associated with a given llvm::Type.  This
	 * is used to compute load and store sizes.
	 *-------------------------------------------------------------------------*/

	size_t bytes(llvm::Type * type);

	// private data members
	llvm::SMDiagnostic llvm_err_;
	llvm::LLVMContext llvm_context_;
	llvm::Module * llvm_module_;

}; // class simulator_t

/*----------------------------------------------------------------------------*
 * Simulator class.
 *----------------------------------------------------------------------------*/

simulator_t::simulator_t(const char * ir_file, std::ostream & stream)
	: llvm_module_(nullptr)
{
	parameters_t & arch = parameters_t::instance();
	machine_state_t & machine = machine_state_t::instance();
	statistics_t & stats = statistics_t::instance();
	instruction_map_t processed;

	/*-------------------------------------------------------------------------*
	 * Write header information.
	 *-------------------------------------------------------------------------*/

	stream << "#---------------------------------------" <<
		"---------------------------------------#" << std::endl;
	stream << "# Static Kernel Analyzer (SKA)" << std::endl;
	stream << "# Header Section" << std::endl;
	stream << "#---------------------------------------" <<
		"---------------------------------------#" << std::endl;
	stream << "KEYWORD_SKA_VERSION " <<
		DEFINE_TO_STRING(SKA_VERSION) << std::endl;

	std::string architecture;
	arch.getval(architecture, "name");
	stream << "KEYWORD_ARCHITECTURE " << architecture <<
		std::endl << std::endl;
	
	/*-------------------------------------------------------------------------*
	 * Initialize core.
	 *-------------------------------------------------------------------------*/
	
	size_t max_issue;
	arch.getval(max_issue, "core::max_issue");
	core_t core(max_issue);

	size_t lus;
	arch.getval(lus, "lus");

	for(size_t i(0); i<lus; ++i) {
		lu_t * lu = new lu_t(i);

		// parse instructions that this LU can handle
		char key[256];
		std::string tmp;

		sprintf(key, "lu::%d::instructions", int(i));
		arch.getval(tmp, key);

		char * ops = new char[tmp.size()+1];
		strcpy(ops, tmp.c_str());

		char * tok = strtok(ops, " ");

		while(tok != nullptr) {
			lu->add_op(code_map[tok]);
			tok = strtok(nullptr, " ");
		} // while

		delete[] ops;
		tok = nullptr;

		// parse types that this LU can handle
		sprintf(key, "lu::%d::types", int(i));
		arch.getval(tmp, key);

		char * types = new char[tmp.size()+1];
		strcpy(types, tmp.c_str());

		tok = strtok(types, " ");

		while(tok != nullptr) {
			lu->add_type(type_map[tok]);
			tok = strtok(nullptr, " ");
		} // while

		delete[] types;

		// add unit to the core
		core.add_unit(lu);
	} // for

	/*-------------------------------------------------------------------------*
	 * Get parsed LLVM IR.
	 *-------------------------------------------------------------------------*/

	llvm_module_ = ParseIRFile(ir_file, llvm_err_, llvm_context_);

	if(llvm_module_ == nullptr) {
		ExitOnError("LLVM parse failed on " << ir_file << std::endl <<
			llvm_err_.getMessage(), ska::LLVMError);
	} // if

	/*-------------------------------------------------------------------------*
	 * Visit modules.
	 *-------------------------------------------------------------------------*/

	for(llvm::Module::iterator fita = llvm_module_->begin();
		fita != llvm_module_->end(); ++fita) {

#if 0
for(llvm::Function::iterator bita = fita->begin();
	bita != fita->end(); ++bita) {
	llvm::errs() << "Basic Block: " << bita->getName() << " has " <<
		bita->size() << " instructions\n";
} // for
#endif

		llvm::inst_iterator iita = inst_begin(fita);
		
		if(iita == inst_end(fita)) {
			continue;
		} // if

		stream << "#---------------------------------------" <<
			"---------------------------------------#" << std::endl;
		stream << "# Module Section: " << fita->getName().str() << std::endl;
		stream << "#---------------------------------------" <<
			"---------------------------------------#" << std::endl;
		stream << "BEGIN_MODULE" << std::endl;
		stream << "KEYWORD_NAME " << fita->getName().str() << std::endl;

		instruction_list_t active;
		instruction_vector_t instructions;

		machine.clear();
		stats.clear();

		llvm::Value * value = nullptr;
		instruction_t * inst = nullptr;

		size_t strahler_number(1);
		size_t expression_depth(1);

	/*-------------------------------------------------------------------------*
	 * Visit instructions.
	 *-------------------------------------------------------------------------*/

		while(iita != inst_end(fita) || active.size() > 0) {
			size_t issued(0);
			bool issue(true);
			std::vector<instruction_t *> cycle_issue;

			while(iita != inst_end(fita) && issue &&
				issued < core.max_issue()) {
				value = &*iita;

				/*----------------------------------------------------------------*
				 * Create instruction and add dependencies
				 *----------------------------------------------------------------*/

				if(inst == nullptr) {

					inst = new instruction_t(decode(&*iita));

					for(unsigned i(0); i<iita->getNumOperands(); ++i) {
						auto op = processed.find(iita->getOperand(i));
						if(op != processed.end()) {
							inst->add_dependency(op->second);
						} // if
					} // for

					/*-------------------------------------------------------------*
					 * Keep track of branching complexity.
					 *-------------------------------------------------------------*/

					inst->update_tree_properties();
					strahler_number = std::max(strahler_number,
						inst->strahler_number());
					expression_depth = std::max(expression_depth,
						inst->depth());

					/*-------------------------------------------------------------*
					 * If an instruction that was previously issed and stalled
					 * is now ready to execute, we can try for multiple-issue.
					 *-------------------------------------------------------------*/

					if(issued == 0) {
						// Check for pending instructions from previous cycles
						// NOTE: This has to happen before the current instruciton
						// is added to active!
						for(auto a = active.begin(); a != active.end(); ++a) {
							if((*a)->state() == instruction_t::stalled &&
								(*a)->ready()) {
								// reset state to staging
								(*a)->set_state(instruction_t::staging);

								// add to instructions issued this cycle
								cycle_issue.push_back(*a);

								// update count
								++issued;
							} // if
						} // for
					} // if

					/*-------------------------------------------------------------*
					 * Add instruction to active list
					 *-------------------------------------------------------------*/

					active.push_back(inst);
				} // if

				/*-------------------------------------------------------------*
				 * Check for dependencies of this instruction
				 *
				 * If the currently queued instruction has any dependencies
				 * that have not retired, multiple issue cannot happen.
				 *-------------------------------------------------------------*/

				issue = inst->ready();

				/*-------------------------------------------------------------*
				 * Check for dependencies within this issue
				 *
				 * If the current instruction depends on any other instructions
				 * that are to be issued this cycle, multiple issue cannot
				 * happen because the current instruction will immediately
				 * stall.
				 *-------------------------------------------------------------*/

				bool cycle_dependency(false);
				if(issued > 0) {
					for(auto cita = cycle_issue.begin();
						cita != cycle_issue.end(); ++cita) {
						for(auto dita = inst->dependencies().begin();
							dita != inst->dependencies().end(); ++dita) {
								if(*dita == *cita) {
									cycle_dependency = true;
									break;
								} // if
						} // for

						if(cycle_dependency) {
							break;
						} // if
					} // for
				} // if

				/*-------------------------------------------------------------*
				 * Check for stalls
				 *
				 * If any instruction is stalled, no new instructions
				 * can be issued.
				 *-------------------------------------------------------------*/

				bool cycle_stall(false);
				if(!cycle_dependency) {
					for(auto a = active.begin(); /* (*a) != inst && */
						a != active.end(); ++a) {
						// check for any type of stall from an active instruction
						if((*a)->state() == instruction_t::stalled) {
							cycle_stall = true;
							break;
						} // if
					} // for
				} // if

				/*-------------------------------------------------------------*
				 * Check to see if this instruction will stall on issue.
				 *
				 * The previous check will not catch this because the
				 * instruction hasn't yet been advanced.
				 *-------------------------------------------------------------*/

				if(issued > 0 && !issue) {
					cycle_stall = true;
				} // if

				/*-------------------------------------------------------------*
				 * Try to issue
				 *-------------------------------------------------------------*/

				int32_t id = core.accept(inst);
				if(!cycle_dependency && !cycle_stall && id >= 0) {

					/*##############################################################
					 ###############################################################
					 # Everything that makes it to this point actually gets
					 # issued and executed.
					 ###############################################################
					 *#############################################################*/
				
					/*-------------------------------------------------------------*
					 * Update statistics
					 *-------------------------------------------------------------*/

					update_stats(&*iita);
				
					/*-------------------------------------------------------------*
					 * Add instruction to this issue
					 *-------------------------------------------------------------*/
				
					cycle_issue.push_back(inst);

					/*-------------------------------------------------------------*
					 * Add instruction to hash
					 *-------------------------------------------------------------*/

					processed[value] = inst;

					/*-------------------------------------------------------------*
					 * Add instruction to issued instructions
					 *-------------------------------------------------------------*/

					instructions.push_back(inst);

					/*-------------------------------------------------------------*
					 * Advance LLVM instruction stream
					 *-------------------------------------------------------------*/

					++issued;
					++iita;
					inst = nullptr;
				}
				else {
					// cleanup failed multiple issue attempt
					if(cycle_stall || issued > 0) {

						// remove current instruction from active list
						for(auto a = active.begin(); a != active.end(); ++a) {
							if((*a) == inst) {
								active.erase(a);
								break;
							} // if
						} // for

						// free the ALU for the current instruction
						if(id != -1) {
							core.release(id);
						} // if

						// delete the instruction
						delete inst;
						inst = nullptr;
					} // if

					issue = false;
					continue;
				} // if

				// if multiple issue was possible, update the affected
				// instruction states
				if(issued > 1) {
					for(auto cita = cycle_issue.begin();
						cita != cycle_issue.end(); ++cita) {
						(*cita)->set_multiple(issued);
					} // for
				} // if
			} // while

			// update executing instructions
			auto a = active.begin();
			while(a != active.end()) {
				(*a)->advance();

				if((*a)->state() == instruction_t::retired) {
					active.erase(a++);
				}
				else {
					++a;
				} // if
			} // while

			// set state for next cycle
			core.advance();
		} // while

		stream << "# Primitive Statistics" << std::endl;
		stream << "KEYWORD_STACK_ALLOCATIONS " <<
			stats["allocas"] << std::endl;
		stream << "KEYWORD_STACK_ALLOCATION_BYTES " <<
			stats["alloca bytes"] << std::endl;
		stream << "KEYWORD_FLOPS " << stats["flops"] << std::endl;
		stream << "KEYWORD_LOADS " << stats["loads"] << std::endl;
		stream << "KEYWORD_LOAD_BYTES " << stats["load bytes"] << std::endl;
		stream << "KEYWORD_STORES " << stats["stores"] << std::endl;
		stream << "KEYWORD_STORE_BYTES " << stats["store bytes"] << std::endl;
		stream << "KEYWORD_CYCLES " << machine.current() << std::endl;
		stream << "# Derived Statistics" << std::endl;
		stream << "KEYWORD_STRAHLER " << strahler_number << std::endl;
		stream << "KEYWORD_DEPTH " << expression_depth << std::endl;
		stream << "KEYWORD_BETA " <<
			double(strahler_number)/expression_depth << std::endl;
		stream << "# Pipeline" << std::endl;
		stream << "BEGIN_INSTRUCTION_STREAM" << std::endl;

		for(auto out = instructions.begin(); out != instructions.end(); ++out) {
			stream << (*out)->string() << std::endl;
		} // for

		stream << "END_INSTRUCTION_STREAM" << std::endl;
		stream << "END_MODULE" << std::endl;
	} // for
} // simulator_t::simulator_t

void simulator_t::update_stats(llvm::Instruction * instruction) {
	statistics_t & stats = statistics_t::instance();

	switch(instruction->getOpcode()) {
		case llvm::Instruction::Ret:
			break;
		case llvm::Instruction::Br:
			break;
		case llvm::Instruction::Switch:
			break;
		case llvm::Instruction::IndirectBr:
			break;
		case llvm::Instruction::Invoke:
			break;
		case llvm::Instruction::Resume:
			break;
		case llvm::Instruction::Unreachable:
			break;
		case llvm::Instruction::Add:
			break;
		case llvm::Instruction::FAdd:
			stats["flops"]++;
			break;
		case llvm::Instruction::Sub:
			break;
		case llvm::Instruction::FSub:
			stats["flops"]++;
			break;
		case llvm::Instruction::Mul:
			break;
		case llvm::Instruction::FMul:
			stats["flops"]++;
			break;
		case llvm::Instruction::UDiv:
			break;
		case llvm::Instruction::SDiv:
			break;
		case llvm::Instruction::FDiv:
			stats["flops"]++;
			break;
		case llvm::Instruction::URem:
			break;
		case llvm::Instruction::SRem:
			break;
		case llvm::Instruction::FRem:
			stats["flops"]++;
			break;
		case llvm::Instruction::Shl:
		case llvm::Instruction::LShr:
		case llvm::Instruction::AShr:
		case llvm::Instruction::And:
		case llvm::Instruction::Or:
		case llvm::Instruction::Xor:
			break;
		case llvm::Instruction::Alloca:
			{
			stats["allocas"]++;
			llvm::AllocaInst * ainst =
				llvm::cast<llvm::AllocaInst>(instruction);
			stats["alloca bytes"] += bytes(ainst->getType());
			break;
			} // scope
		case llvm::Instruction::Load:
			{
			stats["loads"]++;
			llvm::Value * value = instruction->getOperand(0);
			stats["load bytes"] += bytes(value->getType());
			break;
			} // scope
		case llvm::Instruction::Store:
			{
			stats["stores"]++;
			llvm::Value * value = instruction->getOperand(0);
			stats["store bytes"] += bytes(value->getType());
			break;
			} // scope
		case llvm::Instruction::GetElementPtr:
			break;
		case llvm::Instruction::Fence:
			break;
		case llvm::Instruction::AtomicCmpXchg:
			break;
		case llvm::Instruction::AtomicRMW:
			break;
		case llvm::Instruction::Trunc:
			break;
		case llvm::Instruction::ZExt:
			break;
		case llvm::Instruction::SExt:
			break;
		case llvm::Instruction::FPToUI:
			break;
		case llvm::Instruction::FPToSI:
			break;
		case llvm::Instruction::UIToFP:
			break;
		case llvm::Instruction::SIToFP:
			break;
		case llvm::Instruction::FPTrunc:
			break;
		case llvm::Instruction::FPExt:
			break;
		case llvm::Instruction::PtrToInt:
			break;
		case llvm::Instruction::IntToPtr:
			break;
		case llvm::Instruction::BitCast:
			break;
		case llvm::Instruction::ICmp:
			break;
		case llvm::Instruction::FCmp:
			break;
		case llvm::Instruction::PHI:
			break;
		case llvm::Instruction::Call:
			break;
		case llvm::Instruction::Select:
			break;
		case llvm::Instruction::UserOp1:
			break;
		case llvm::Instruction::UserOp2:
			break;
		case llvm::Instruction::VAArg:
			break;
		case llvm::Instruction::ExtractElement:
			break;
		case llvm::Instruction::InsertElement:
			break;
		case llvm::Instruction::ShuffleVector:
			break;
		case llvm::Instruction::ExtractValue:
			break;
		case llvm::Instruction::InsertValue:
			break;
		case llvm::Instruction::LandingPad:
			break;
		default:
			{
				ExitOnError("Unhandled Instruction", ska::UnknownCase);
			} // scope
	} // switch
} // simulator_t::update_stats

size_t simulator_t::bytes(llvm::Type * type) {
	switch(type->getTypeID()) {

		case llvm::Type::VoidTyID:
			return sizeof(nullptr);

		case llvm::Type::HalfTyID:
			break;

		case llvm::Type::FloatTyID:
			return 4;

		case llvm::Type::DoubleTyID:
			return 8;

		case llvm::Type::X86_FP80TyID:
			break;

		case llvm::Type::FP128TyID:
			break;

		case llvm::Type::PPC_FP128TyID:
			break;

		case llvm::Type::LabelTyID:
			break;

		case llvm::Type::MetadataTyID:
			break;

		case llvm::Type::X86_MMXTyID:
			break;

		case llvm::Type::IntegerTyID:
			break;

		case llvm::Type::FunctionTyID:
			break;

		case llvm::Type::StructTyID:
			break;

		case llvm::Type::ArrayTyID:
			return type->getArrayNumElements() *
				bytes(type->getArrayElementType());

		case llvm::Type::PointerTyID:
			return bytes(type->getPointerElementType());

		case llvm::Type::VectorTyID:
			break;

		case llvm::Type::NumTypeIDs:
			break;

		default:
			break;
	} // switch

	return 0;
} // simulator_t::bytes

} // namespace ska

#endif // Simulator_hh

/*----------------------------------------------------------------------------*
 * Local Variables: 
 * mode:c++
 * c-basic-offset:3
 * indent-tabs-mode:t
 * tab-width:3
 * End:
 *
 * vim: set ts=3 :
 *----------------------------------------------------------------------------*/
