#ifndef SMACK_DSA_AA_H
#define SMACK_DSA_AA_H

#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Module.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/Passes.h"
#include "dsa/DataStructure.h"
#include "dsa/DSGraph.h"
using namespace llvm;

class SmackDSAAA: public llvm::ModulePass, public llvm::AliasAnalysis
{
private:
    const llvm::DataLayout* dataLayout;
    TDDataStructures *TD;
    BUDataStructures *BU;

	//AliasResult andersenAlias(const llvm::Value* l1, const llvm::Value* l2);
public:
	static char ID;

	// Interfaces of AliasAnalysis.
	AliasResult alias(const Location& l1, const Location& l2) override;
	void deleteValue(llvm::Value* v) override;
	void copyValue(llvm::Value* from, llvm::Value* to) override;
	bool pointsToConstantMemory(const Location& loc, bool orLocal) override;
	//ModRefResult getModRefInfo (llvm::ImmutableCallSite cs, const Location &loc);


	SmackDSAAA(): ModulePass(ID), TD(nullptr), BU(nullptr), dataLayout(nullptr) {}
	bool runOnModule(llvm::Module &M) override;
	void getAnalysisUsage(llvm::AnalysisUsage &AU) const override;
	void* getAdjustedAnalysisPointer(llvm::AnalysisID PI) override;

    llvm::DSGraph *getGraphForValue(const llvm::Value *V);
};

#endif
