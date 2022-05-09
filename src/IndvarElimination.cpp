
#include "IndvarElimination.hpp"


using namespace llvm;
namespace {

    void IndvarEliminationPass::getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesCFG();
      //AU.addRequired<LoopInfoWrapperPass>();
      //AU.addRequired<TargetLibraryInfoWrapperPass>();
      //AU.addRequired<AssumptionCacheTracker>();
      AU.addRequired<IVUsersWrapperPass>();
    }

    bool IndvarEliminationPass::runOnLoop(Loop *L, LPPassManager &no_use) {

      //IVUsersWrapperPass* pass = (IVUsersWrapperPass* )createIVUsersPass();
      //pass->runOnLoop(L, no_use);

      //pass->print(llvm::outs());

      IVUsersWrapperPass& pass = getAnalysis<IVUsersWrapperPass>();
      pass.print(llvm::outs());

      outs() << "displayed induction variables!!!!!!!!!!!\n";

      return replaceInductionVariable(L);
    }

    //check to see if the icmp is the only usage of the basic induction variable
    bool IndvarEliminationPass::checkUsage(Value* v, Value* icmp){

      for(auto user : v->users()){
        user->print(outs());
        outs()<< "\n";
      }

      if(v->hasOneUse()){
        User* user = v->user_back();
        ICmpInst* cmp = dyn_cast<ICmpInst>(user);
        //if (auto I = dyn_cast<Instruction*>(user)){

        //}

        return cmp == icmp;
      }

      outs() <<"\n" << v->getName() << " has more than 1 users\n";
      return false;
    }

    bool isBaisInductionVariable(Instruction* I, Loop* L){
      if(PHINode* pn = dyn_cast<PHINode>(I)){
        int n = pn->getNumIncomingValues();
        for(int i = 0; i < n; i++){
          Value* cur_val = pn->getIncomingValue(i);
          if (auto *op = dyn_cast<BinaryOperator>(cur_val)){
            return true;
          }

          if(ConstantInt* ci = dyn_cast<ConstantInt>(cur_val)){
            continue;
          }

          if(PHINode* phi_val = dyn_cast<PHINode>(cur_val)){
            return false;
          }
        }
      }

      return false;
    }

    Instruction* randomSelectIndvar(Loop* L, Value* target){
        for (auto &I : *L->getHeader()) {
          if(isBaisInductionVariable(&I, L)){
            outs()<<"selected replace Instruction: \n";
            I.print(outs());
            outs()<<"\n";
            return &I;
          }
        }

        return NULL;
    }

    bool IndvarEliminationPass::replaceInductionVariable(Loop* L){

        vector<Instruction*> tobeRemoved;
        ICmpInst* icmp;
        Value* new_cmp;

        for (auto &I : *L->getHeader()) {
          // we insert at the first phi node
          if (icmp = dyn_cast<ICmpInst>(&I)) {

            Value * LHS = icmp->getOperand(0);
            Value * RHS = icmp->getOperand(1);

            outs() << "found the cmp Instruction\n";
            LHS->printAsOperand(outs(), false);
            outs() << "\n";
            RHS->printAsOperand(outs(), false);
            outs() << "\n";

            if(checkUsage(LHS, icmp)){
              //randomly select a basic induction variable
              outs() << "\nidentified the induction variable in the cmp Instruction: \n";
              LHS->printAsOperand(llvm::outs(), false);

              Value* selected = randomSelectIndvar(L, LHS);

              //create a icmp instruction
              if(PHINode* sel_phi = dyn_cast<PHINode>(selected)){
                outs() << "\nselected a induction variable: \n";
                selected->printAsOperand(outs(), false);
                outs()<<"\n";

                PHINodeValue phi_val_target((PHINode*)LHS);
                PHINodeValue phi_val_sel(sel_phi);

                IRBuilder<> head_builder(icmp);

                ConstantInt* CIR = dyn_cast<ConstantInt>(RHS);
                int steps = phi_val_target.getIterationSteps( CIR->getSExtValue());
                outs()<<"\nsteps of original cmp: "<< steps<<"\n";

                int new_limit = phi_val_sel.getLimitValue(steps);
                outs()<<"\nlimit of the new icmp: "<<new_limit<<"\n";

                Value* limit = ConstantInt::getSigned(CIR->getType(), new_limit);

                new_cmp = head_builder.CreateCmp(icmp->getSignedPredicate(), selected, limit);

                //remove the orignal cmp instruction and the basic induction variable
                //((Instruction*)LHS)->removeFromParent();
                //icmp->removeFromParent();

                //remove the instructions after the loop.
                tobeRemoved.push_back((Instruction*)LHS);
                tobeRemoved.push_back(icmp);

                break;
                outs()<<"\ncreated new icmp Instruction\n";
              }
            }
          }
        }

        if(icmp){
          icmp->replaceAllUsesWith(new_cmp);
        }

        while(tobeRemoved.size() > 0){
          tobeRemoved.back()->eraseFromParent();
          tobeRemoved.pop_back();
        }

        return true;
    }
}


char IndvarEliminationPass::ID = 0;
static RegisterPass<IndvarEliminationPass> X("indvar-el", "Induction Variable Elimination Pass",
                                    false /* Only looks at CFG */,
                                    true /* Not Analysis Pass */);

// Automatically enable the pass.
// http://adriansampson.net/blog/clangpass.html
static void registerSkeletonPass(const PassManagerBuilder &,
                         legacy::PassManagerBase &PM) {
  PM.add(new IndvarEliminationPass());
}

static RegisterStandardPasses
  RegisterMyPass(PassManagerBuilder::EP_LoopOptimizerEnd,
                 registerSkeletonPass);
