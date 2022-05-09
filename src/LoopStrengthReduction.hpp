
#ifndef __LOOPSTRENGTHREDUCTION_H
#define __LOOPSTRENGTHREDUCTION_H


#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/LoopInfo.h" 
#include "llvm/Analysis/LoopPass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
//#include "IVIdentify.hpp"
#include "llvm//Analysis/IVUsers.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/DenseMap.h"

#include <tuple>
#include <map>
#include <iostream>

using namespace std;
using namespace llvm;

namespace {

    class LSRPass : public FunctionPass {
        private:


        public:
            static char ID;

            LSRPass() : FunctionPass(ID) {}
            bool checkUsage(Value* v, Value* icmp);
            bool replaceInductionVariable(Loop* L, DenseMap<Value*, tuple<Value*, int, int>>& IndVarMap);

            virtual void getAnalysisUsage(AnalysisUsage& AU) const;

            virtual bool runOnLoop(Loop *L, LPPassManager &no_use);

            virtual bool runOnFunction(Function &F);

            void identifyInductionVariable(Loop *L, DenseMap<Value*, tuple<Value*, int, int>>& IndVarMap);

            bool preprocess(Function& F, legacy::FunctionPassManager& FPM);

    };

}

#endif