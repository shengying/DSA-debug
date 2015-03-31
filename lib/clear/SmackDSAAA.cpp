#include "smack/SmackDSAAA.h"

#include "llvm/IR/Module.h"

using namespace llvm;


AliasAnalysis::AliasResult SmackDSAAA::alias(const AliasAnalysis::Location& l1, const AliasAnalysis::Location& l2)
{
    return NoAlias;
}

void SmackDSAAA::deleteValue(llvm::Value* v) {
    BU->deleteValue(v);
    TD->deleteValue(v);
}

void SmackDSAAA::copyValue(llvm::Value* from, llvm::Value* to)
{
    if (from == to) return;
    BU->copyValue(from, to);
    TD->copyValue(from, to);
}

// getGraphForValue - Return the DSGraph to use for queries about the specified
// value...
//
DSGraph *SmackDSAAA::getGraphForValue(const Value *V) {
  if (const Instruction *I = dyn_cast<Instruction>(V))
    return TD->getDSGraph(*(I->getParent()->getParent()));
  else if (const Argument *A = dyn_cast<Argument>(V))
    return TD->getDSGraph(*(A->getParent()));
  else if (const BasicBlock *BB = dyn_cast<BasicBlock>(V))
    return TD->getDSGraph(*(BB->getParent()));
  return 0;
}

bool SmackDSAAA::pointsToConstantMemory(const Location& loc, bool orLocal)
{
//	NodeIndex node = (anders->nodeFactory).getValueNodeFor(loc.Ptr);
//	if (node == AndersNodeFactory::InvalidIndex)
//		return AliasAnalysis::pointsToConstantMemory(loc, orLocal);
//
//	auto itr = (anders->ptsGraph).find(node);
//	if (itr == (anders->ptsGraph).end())
//		// Not a pointer?
//		return AliasAnalysis::pointsToConstantMemory(loc, orLocal);
//
//	const AndersPtsSet& ptsSet = itr->second;
//	for (auto const& idx: ptsSet)
//	{
//		if (const Value* val = (anders->nodeFactory).getValueForNode(idx))
//		{
//			if (!isa<GlobalValue>(val) || (isa<GlobalVariable>(val) && !cast<GlobalVariable>(val)->isConstant()))
//        		return AliasAnalysis::pointsToConstantMemory(loc, orLocal);
//		}
//		else
//		{
//			if (idx != (anders->nodeFactory).getNullObjectNode())
//				return AliasAnalysis::pointsToConstantMemory(loc, orLocal);
//		}
//	}

	return true;
}

void SmackDSAAA::getAnalysisUsage(AnalysisUsage &AU) const
{
	AliasAnalysis::getAnalysisUsage(AU);
    AU.addRequired<TDDataStructures>(); // Uses TD Datastructures
    AU.addRequired<BUDataStructures>(); // Uses BU Datastructures
	AU.addRequired<DataLayoutPass>();
	AU.setPreservesAll();
}

void* SmackDSAAA::getAdjustedAnalysisPointer(AnalysisID PI)
{
	if (PI == &AliasAnalysis::ID)
		return (AliasAnalysis *)this;
	return this;
}

bool SmackDSAAA::runOnModule(Module &M)
{
	InitializeAliasAnalysis(this);

//	anders = &getAnalysis<Andersen>();
	dataLayout = &(getAnalysis<DataLayoutPass>().getDataLayout());
    TD = &getAnalysis<TDDataStructures>();
    BU = &getAnalysis<BUDataStructures>();
	return false;
}

char SmackDSAAA::ID = 0;
static RegisterPass<SmackDSAAA> X("smack-ds-aa", "Smack - Data Structure Analysis Alias Analysis", true, true);
static RegisterAnalysisGroup<AliasAnalysis> Y(X);
