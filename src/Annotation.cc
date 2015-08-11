#define DEBUG_TYPE "annot"
#include <llvm/IR/Module.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/IntrinsicInst.h>
#include <llvm/IR/Metadata.h>
#include <llvm/IR/Constants.h>
#include <llvm/Pass.h>
#include <llvm/Analysis/ValueTracking.h>
#include <llvm/Support/InstIterator.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/SmallPtrSet.h>
#include <llvm/Transforms/Utils/Local.h>

#include "Annotation.h"

using namespace llvm;


static inline bool needAnnotation(Value *V) {
	if (PointerType *PTy = dyn_cast<PointerType>(V->getType())) {
		Type *Ty = PTy->getElementType();
		return isFunctionPointer(Ty);
	}
	return false;
}

std::string getAnnotation(Value *V, Module *M) {
	std::string id;

	if (GlobalVariable *GV = dyn_cast<GlobalVariable>(V))
		id = getVarId(GV);
	else {
		User::op_iterator is, ie; // GEP indices
		Type *PTy = NULL;         // Type of pointer in GEP
		if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(V)) {
			// GEP instruction
			is = GEP->idx_begin();
			ie = GEP->idx_end() - 1;
			PTy = GEP->getPointerOperandType();
		} else if (ConstantExpr *CE = dyn_cast<ConstantExpr>(V)) {
			// constant GEP expression
			if (CE->getOpcode() == Instruction::GetElementPtr) {
				is = CE->op_begin() + 1;
				ie = CE->op_end() - 1;
				PTy = CE->getOperand(0)->getType();
			}
		}
		// id is in the form of struct.[name].[offset]
		if (PTy) {
			SmallVector<Value *, 4> Idx(is, ie);
			Type *Ty = GetElementPtrInst::getIndexedType(PTy, Idx);
			ConstantInt *Offset = dyn_cast<ConstantInt>(ie->get());
			if (Offset && isa<StructType>(Ty))
				id = getStructId(Ty, M, Offset->getLimitedValue());
		}
	}

	return id;
}

static bool annotateLoadStore(Instruction *I) {
	std::string Anno;
	LLVMContext &VMCtx = I->getContext();
	Module *M = I->getParent()->getParent()->getParent();

	if (LoadInst *LI = dyn_cast<LoadInst>(I)) {
		llvm::Value *V = LI->getPointerOperand();
		if (needAnnotation(V))
			Anno = getAnnotation(V, M);
	} else if (StoreInst *SI = dyn_cast<StoreInst>(I)) {
		Value *V = SI->getPointerOperand();
		if (needAnnotation(V))
			Anno = getAnnotation(V, M);
	}

	if (Anno.empty())
		return false;

	MDNode *MD = MDNode::get(VMCtx, MDString::get(VMCtx, Anno));
	I->setMetadata(MD_ID, MD);
	return true;
}

bool AnnotationPass::runOnFunction(Function &F) {
	bool Changed = false;

	for (inst_iterator i = inst_begin(F), e = inst_end(F); i != e; ++i) {
		Instruction *I = &*i;
		if (isa<LoadInst>(I) || isa<StoreInst>(I)) {
			Changed |= annotateLoadStore(I);
		} 
	}
	return Changed;
}

bool AnnotationPass::doInitialization(Module &M) {
	this->M = &M;
	return true;
}


char AnnotationPass::ID;

static RegisterPass<AnnotationPass>
X("anno", "add id annotation for load/stores");
