#define DEBUG_TYPE "helper"
#include "Helper.h"
namespace llvm {
	namespace slicing {
	const Function* getParent(const Value *V) {
		if( !V )
			return NULL;

		DEBUG( errs() << "getParent\t" );
		DEBUG( V->print( errs() ) );
		DEBUG( errs() << "\n" );

	  if (const Instruction *inst = dyn_cast<const Instruction>(V))
	    return inst->getParent()->getParent();
	
	  if (const Argument *arg = dyn_cast<const Argument>(V))
	    return arg->getParent();
	
		if ( dyn_cast<const GlobalVariable>(V) )
			return NULL;

		// Shouldn't be called with any other value
		assert( false );
		return NULL;
	}

	// The value being added should either be of the same
	// function or should be global.
	bool Helper::checkValid( const Pointee &var1, const Pointee &var2 ){
		MemoryLocation op1 = var1.first;
		MemoryLocation op2 = var2.first;
		if( dyn_cast<InlineAsm>( op1 ) )
			return false;
		if( dyn_cast<InlineAsm>( op2 ) )
			return false;

		const Function *F1 = getParent(op1);
		const Function *F2 = getParent(op2);

		if( F1 && F2 && F1 != F2 ){
			errs() << "checkValid failed for the variables ";
			op1->print( errs() );
			errs() << F1->getName(); 
			errs() << "\t";
			op2->print( errs() );
			errs() << F2->getName(); 
			errs() << "\n";
			assert( false );
		}
		return true;
	}

//	AliasResult Helper::sameValues(const Pointee &val1, const Pointee &val2, AliasAnalysis* AA) {
//		AliasResult def = AliasResult::NoAlias;
//		if( !checkValid( val1, val2 ) )
//			return def;
//
//		return AA->alias(val1.first, val2.first);
//	}

	// C is the call instruction. F is the function being called.
	void Helper::fillParamsToArgs(CallInst const* const C,
			Function const* const F,
			ParamsToArgs& toArgs) {
		DEBUG( errs() << "fillParamsToArgs ");
		DEBUG( C->print(errs()));
		DEBUG( errs() << F->getName() << "\n");
		if ( F->isVarArg()) {
			DEBUG( errs() << "fillParamsToArgs VarArg, ignore ");
			return;
		}

		// Function parameter in the method signature
		Function::const_arg_iterator p = F->arg_begin();
		std::size_t a = 0;
		for ( ; a < C->getNumArgOperands(); ++a, ++p) {
			// F does not take variable argument and p fell over arg list
			if ( p == F->arg_end() ) {
				errs() << "Check the call instruction and function being passed\n";
				C->print( errs() );
				errs() << "\t";
				errs() << F->getName();
				errs() << "\n";
				assert( false );
			} 
			const Value *P = &*p;
			// Function parameter in the call instruction
			const Value *A = C->getArgOperand(a);
			if (P && A && !isConstantValue(A)){
				DEBUG( errs() << "Add Param to Args ");
				DEBUG( P->print( errs() ) );
				DEBUG( A->print( errs() ) );
				DEBUG( errs() << "\n");
				toArgs.insert(ParamsToArgs::value_type(Pointee(P, -1), Pointee(A, -1)));
			}
		}
	}

	// C is the call instruction. F is the function being called.
	void Helper::fillArgsToParams(CallInst const* const C,
			Function const* const F,
			ArgsToParams& toParams) {
		if ( F->isVarArg()) {
			DEBUG( errs() << "fillArgsToParams VarArg, ignore ");
			return;
		}
		// Function parameter in the method signature
		Function::const_arg_iterator p = F->arg_begin();
		std::size_t a = 0;
		for ( ; a < C->getNumArgOperands(); ++a, ++p)
		{
			const Value *P = &*p;
			// Function parameter in the call instruction
			const Value *A = C->getArgOperand(a);
			if (!isConstantValue(A)){
				toParams.insert(ArgsToParams::value_type(Pointee(A, -1), Pointee(P, -1)));
			}
		}
	}
}}
