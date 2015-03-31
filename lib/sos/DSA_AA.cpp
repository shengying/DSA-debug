#include "sos/DSA_AA.h"

#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"
using namespace llvm;

//static inline bool isSetContainingOnly(const AndersPtsSet& set, NodeIndex i)
//{
//	return (set.getSize() == 1) && (*set.begin() == i);
//}

//AliasAnalysis::AliasResult AndersenAA::andersenAlias(const llvm::Value* v1, const llvm::Value* v2)
//{
//	NodeIndex n1 = (anders->nodeFactory).getMergeTarget((anders->nodeFactory).getValueNodeFor(v1));
//	NodeIndex n2 = (anders->nodeFactory).getMergeTarget((anders->nodeFactory).getValueNodeFor(v2));
//
//	if (n1 == n2)
//		return AliasAnalysis::MustAlias;
//
//	auto itr1 = (anders->ptsGraph).find(n1), itr2 = (anders->ptsGraph).find(n2);
//	if (itr1 == (anders->ptsGraph).end() || itr2 == (anders->ptsGraph).end())
//		// We knows nothing about at least one of (v1, v2)
//		return AliasAnalysis::MayAlias;
//
//	AndersPtsSet& s1 = itr1->second, s2 = itr2->second;
//	bool isNull1 = isSetContainingOnly(s1, (anders->nodeFactory).getNullObjectNode());
//	bool isNull2 = isSetContainingOnly(s2, (anders->nodeFactory).getNullObjectNode());
//	if (isNull1 || isNull2)
//		// If any of them is null, we know that they must not alias each other
//		return AliasAnalysis::NoAlias;
//
//	if (s1.getSize() == 1 && s2.getSize() == 1 && *s1.begin() == *s2.begin())
//		return AliasAnalysis::MustAlias;
//
//	// Compute the intersection of s1 and s2
//	for (auto const& idx: s1)
//	{
//		if (idx == (anders->nodeFactory).getNullObjectNode())
//			continue;
//		if (s2.has(idx))
//			return AliasAnalysis::MayAlias;
//	}
//
//	return AliasAnalysis::NoAlias;
//}

AliasAnalysis::AliasResult DSAAA::alias(const AliasAnalysis::Location& l1, const AliasAnalysis::Location& l2)
{
	if (l1.Size == 0 || l2.Size == 0)
		return NoAlias;

	const Value* v1 = (l1.Ptr)->stripPointerCasts();
	const Value* v2 = (l2.Ptr)->stripPointerCasts();

    if (llvm::DebugFlag) {
        errs() << "Checking the alias pairs ...\n";
        errs() << "v1 = " << *(l1.Ptr) << "; v2 = " << *(l2.Ptr) << "\n";
    }

	if (!v1->getType()->isPointerTy() || !v2->getType()->isPointerTy())
		return NoAlias;

	if (v1 == v2) return MustAlias;

    DSGraph *G1 = getGraphForValue(v1);
    DSGraph *G2 = getGraphForValue(v2);
    assert((!G1 || !G2 || G1 == G2) && "Alias query for 2 different functions?");

    // Get the graph to use...
    DSGraph* G = G1 ? G1 : (G2 ? G2 : TD->getGlobalsGraph());

    const DSGraph::ScalarMapTy &GSM = G->getScalarMap();
    DSGraph::ScalarMapTy::const_iterator I = GSM.find((Value*)v1);
    if (I == GSM.end()) return NoAlias;
    DSGraph::ScalarMapTy::const_iterator J = GSM.find((Value*)v2);
    if (J == GSM.end()) return NoAlias;

    DSNode  *N1 = I->second.getNode(),  *N2 = J->second.getNode();
    unsigned O1 = I->second.getOffset(), O2 = J->second.getOffset();
    if (N1 == nullptr || N2 == nullptr) {
        // Can't tell whether anything aliases null.
        errs() << "[DEBUG] nullptr for this value. \n";
        return AliasAnalysis::alias(l1, l2);
    }

    if (!N1->isCompleteNode() && !N2->isCompleteNode()) {
        if (llvm::DebugFlag) {
            errs() << "We calculate MayAlias here.\n";
            errs() << "v1 = " << *(l1.Ptr) << "; v2 = " << *(l2.Ptr) << "\n";
            errs() << "N1 = " << N1 << "; N2 = " << N2 << "\n";
            errs() << "N1 complete? " << N1->isCompleteNode() << "; N2 complete? " << N2->isCompleteNode() << "\n";
        }
        return MayAlias;
    }

    // We can only make a judgment if one of the nodes is complete.
    // if (N1->isCompleteNode() || N2->isCompleteNode()) {
    if (N1 != N2) return NoAlias;   // Completely different nodes.

    // See if they point to different offsets...  if so, we may be able to
    // determine that they do not alias...
    if (O1 != O2) {
        uint64_t V1Size = l1.Size;
        uint64_t V2Size = l2.Size;
        if (O2 < O1) {    // Ensure that O1 <= O2
            std::swap(v1, v2);
            std::swap(O1, O2);
            std::swap(V1Size, V2Size);
        }

        if (O1+V1Size <= O2) return NoAlias;
    }


  /**
   * Below added by Zhiyuan
   */
//    if (N1 == N2 && N1->isCompleteNode() && N2->isCompleteNode()) return MustAlias;

    if (llvm::DebugFlag) {
        errs() << "We need to consult other alias analysis for better results.\n";
        errs() << "v1 = " << *(l1.Ptr) << "; v2 = " << *(l2.Ptr) << "\n";
        errs() << "N1 = " << N1 << "; N2 = " << N2 << "\n";
        errs() << "N1 complete? " << N1->isCompleteNode() << "; N2 complete? " << N2->isCompleteNode() << "\n";
    }
  /**
   * Above added by Zhiyuan
   */

  // FIXME: we could improve on this by checking the globals graph for aliased
  // global queries...
    return AliasAnalysis::alias(l1, l2);
}

void DSAAA::deleteValue(llvm::Value* v) {
    InvalidateCache();
    BU->deleteValue(v);
    TD->deleteValue(v);
}

void DSAAA::copyValue(llvm::Value* from, llvm::Value* to)
{
    if (from == to) return;
    InvalidateCache();
    BU->copyValue(from, to);
    TD->copyValue(from, to);
}

// getGraphForValue - Return the DSGraph to use for queries about the specified
// value...
//
DSGraph *DSAAA::getGraphForValue(const Value *V) {
  if (const Instruction *I = dyn_cast<Instruction>(V))
    return TD->getDSGraph(*(I->getParent()->getParent()));
  else if (const Argument *A = dyn_cast<Argument>(V))
    return TD->getDSGraph(*(A->getParent()));
  else if (const BasicBlock *BB = dyn_cast<BasicBlock>(V))
    return TD->getDSGraph(*(BB->getParent()));
  return 0;
}

bool DSAAA::pointsToConstantMemory(const Location& loc, bool orLocal)
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

void DSAAA::getAnalysisUsage(AnalysisUsage &AU) const
{
	AliasAnalysis::getAnalysisUsage(AU);
    AU.addRequired<TDDataStructures>(); // Uses TD Datastructures
    AU.addRequired<BUDataStructures>(); // Uses BU Datastructures
	AU.addRequired<DataLayoutPass>();
	AU.setPreservesAll();
}

void* DSAAA::getAdjustedAnalysisPointer(AnalysisID PI)
{
	if (PI == &AliasAnalysis::ID)
		return (AliasAnalysis *)this;
	return this;
}

bool DSAAA::runOnModule(Module &M)
{
	InitializeAliasAnalysis(this);

//	anders = &getAnalysis<Andersen>();
	dataLayout = &(getAnalysis<DataLayoutPass>().getDataLayout());
    TD = &getAnalysis<TDDataStructures>();
    BU = &getAnalysis<BUDataStructures>();
	return false;
}

char DSAAA::ID = 0;
static RegisterPass<DSAAA> X("dsa-aa", "Data Structure Analysis Alias Analysis", true, true);
static RegisterAnalysisGroup<AliasAnalysis> Y(X);
