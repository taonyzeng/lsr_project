
#ifndef __INDVARELIMINATION_H
#define __INDVARELIMINATION_H


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

    class PHINodeValue{

        int init_val;
        int increament;
        PHINode* pn;

        int max_recursive = 5;

    public:
        PHINodeValue(PHINode* phi_node){
            this->pn = phi_node;
        }

        //get the non constant incoming value from the loop body recursively.
        Value* getNonConstantValue(PHINode* phi_val){
            this->max_recursive--;
            if(!this->max_recursive){
                return NULL;
            }

          int n = phi_val->getNumIncomingValues();
          for(int i = 0; i < n; i++){
            Value* cur_val = phi_val->getIncomingValue(i);
            if (auto *op = dyn_cast<BinaryOperator>(cur_val)){
                outs() << "\nop cur_val " << i << ": ";  
                op->print(outs());
                outs() << "\n";
                return op;
            }

            if(ConstantInt* ci = dyn_cast<ConstantInt>(cur_val)){
                outs() << "\nconst cur_val " << i << ": ";  
                ci->print(outs());
                outs() << "\n";
                continue;
            }

            if(PHINode* phi_sub = dyn_cast<PHINode>(cur_val)){
                outs() << "\nphi cur_val " << i << ": ";
                phi_sub->print(outs());
                outs() << "\n";
                return getNonConstantValue(phi_sub);
            }
          }

          return NULL;
        }

        int getIncreament(){
            Value* val = getNonConstantValue(this->pn);
            if (auto *op = dyn_cast<BinaryOperator>(val)){
                Value* lhs = op->getOperand(0);
                Value* rhs = op->getOperand(1);

                ConstantInt* CIL = dyn_cast<ConstantInt>(lhs);
                ConstantInt* CIR = dyn_cast<ConstantInt>(rhs);

                if(CIL){
                    return CIL->getSExtValue();
                }
                else if(CIR){
                    return CIR->getSExtValue();
                }
            }

            return 1;
        }

        int getInitValue(){

            int n = pn->getNumIncomingValues();
            for(int i = 0; i < n; i++){
                Value* cur_val = pn->getIncomingValue(i);

                if(ConstantInt* ci = dyn_cast<ConstantInt>(cur_val)){
                    return ci->getSExtValue();
                }
            }

            return 0;
        }

        int getIterationSteps(int limit){
            return (limit - getInitValue())/getIncreament();
        } 

        int getLimitValue(int steps){
            return getInitValue() + getIncreament() * steps;
        }

    };

    class IndvarEliminationPass : public LoopPass {
        private:

        public:
            static char ID;

            IndvarEliminationPass() : LoopPass(ID) {}

            bool checkUsage(Value* v, Value* icmp);

            bool replaceInductionVariable(Loop* L);

            virtual void getAnalysisUsage(AnalysisUsage& AU) const;

            virtual bool runOnLoop(Loop *L, LPPassManager &no_use);

    };

}

#endif