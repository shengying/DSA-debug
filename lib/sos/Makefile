LLVM_CONFIG=llvm-config
CXX=`$(LLVM_CONFIG) --bindir`/clang++
CXXFLAGS=`$(LLVM_CONFIG) --cppflags` -fPIC -fno-rtti -g -O0 -Wno-deprecated -std=c++11
LDFLAGS=`$(LLVM_CONFIG) --ldflags`

all: DSAA.a

#i_scdg.so: ControlDependencies.o SystemControlDependencies.o DepPrinter.o
#	$(CXX) -shared ControlDependencies.o SystemControlDependencies.o DepPrinter.o -o i_scdg.so $(LDFLAGS) -fPIC

#SystemControlDependencies.o: SystemControlDependenceGraph.cpp SystemControlDependenceGraph.h
#	$(CXX) -c SystemControlDependenceGraph.cpp -o SystemControlDependencies.o $(CXXFLAGS)

DSAA.a: DSAA.o
	$(CXX) -shared *.o -o DSAA.a $(LDFLAGS) -fPIC

DSAA.o:
	$(CXX) -I../../include -c AliasAnalysisEvaluator.cpp DSA_AA.cpp $(CXXFLAGS)

clean:
	rm -f *.o *.a
