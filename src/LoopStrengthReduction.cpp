
#include "LoopStrengthReduction.hpp"
#include "llvm/Analysis/AssumptionCache.h"


using namespace llvm;
namespace {

    void LSRPass::getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesCFG();
      AU.addRequired<LoopInfoWrapperPass>();
      AU.addRequired<TargetLibraryInfoWrapperPass>();
      //AU.addRequired<AssumptionCacheTracker>();
      //AU.addRequired<IVUsersWrapperPass>();
    }

    bool LSRPass::runOnLoop(Loop *L, LPPassManager &no_use) {

      //IVUsersWrapperPass* pass = (IVUsersWrapperPass* )createIVUsersPass();
      //pass->runOnLoop(L, no_use);

      //pass->print(llvm::outs());

      IVUsersWrapperPass& pass = getAnalysis<IVUsersWrapperPass>();
      pass.print(llvm::outs());

      outs() << "displayed induction variables!!!!!!!!!!!\n";

      return false;
    }


    bool LSRPass::preprocess(Function& F, legacy::FunctionPassManager& FPM) {
      // craete llvm function with
      LLVMContext &Ctx = F.getContext();
      std::vector<Type*> paramTypes = {Type::getInt32Ty(Ctx)};
      Type *retType = Type::getVoidTy(Ctx);
      FunctionType *logFuncType = FunctionType::get(retType, paramTypes, false);
      Module* module = F.getParent();
      //Constant *logFunc = module->getOrInsertFunction("logop", logFuncType);
      module->getOrInsertFunction("logop", logFuncType);

      FPM.add(llvm::createCorrelatedValuePropagationPass());
      //FPM.add(createIndVarSimplifyPass());
      FPM.add(createDeadCodeEliminationPass());
      FPM.add(createLoopSimplifyPass());
      //FPM.add(createIVUsersPass());
      FPM.doInitialization();
      bool changed = FPM.run(F);
      FPM.doFinalization();

      return changed;
    }

    void identifyBasicInductionVariable(Loop *L, DenseMap<Value*, tuple<Value*, int, int>>& IndVarMap ){

      // the header block
      BasicBlock* b_header = L->getHeader();
      BasicBlock* b_preheader = L->getLoopPreheader();

      for (auto &I : *b_header) {
        if (PHINode *PN = dyn_cast<PHINode>(&I)) {
          //IndVarMap[&I] = make_tuple(&I, 1, 0);
          int num_income = PN->getNumIncomingValues();
          assert(num_income == 2);
          // find the preheader value of the phi node
          for (int i = 0; i < num_income; i++) {
            if(PN->getIncomingBlock(i) != b_preheader) {
              BasicBlock* body = PN->getIncomingBlock(i);
              for(Instruction& I : *body){
                Value* value = PN->getIncomingValue(i);
                if(value == (Value*)&I){
                  outs()<<"found the incoming value in the body, name = " << value->getName() << "\n";
                  if (auto *op = dyn_cast<BinaryOperator>(&I)) 
                     if(op->getOpcode() == Instruction::Add || op->getOpcode() == Instruction::FAdd) {
                        Value *lhs = op->getOperand(0);
                        Value *rhs = op->getOperand(1);

                        ConstantInt* CIL = dyn_cast<ConstantInt>(lhs);
                        ConstantInt* CIR = dyn_cast<ConstantInt>(rhs);

                        //one of the operands is constant, the other one is the Phi node definition
                        //in the loop header.
                        if(lhs == (Value*)PN && CIR || rhs == (Value*)PN && CIL){
                          outs()<<"idengtified the basic induction variable: " << PN->getName()  <<"\n";
                          IndVarMap[PN] = make_tuple(PN, 1, 0);
                          break;
                        }
                      }
                  }
                }
            }
          }
        }
      }
    }

    void LSRPass::identifyInductionVariable(Loop *L, DenseMap<Value*, tuple<Value*, int, int>>& IndVarMap) {

        // the header block
        BasicBlock* b_header = L->getHeader();

        // for (auto &I : *b_header) {
        //   if (PHINode *PN = dyn_cast<PHINode>(&I)) {
        //     IndVarMap[&I] = make_tuple(&I, 1, 0);
        //   }
        // }
        
        identifyBasicInductionVariable(L, IndVarMap);

        // get the total number of blocks as well as the block list
        //cout << L->getNumBlocks() << "\n";
        auto blks = L->getBlocks();

        // find all indvars
        // keep modifying the set until the size does not change
        // notice that over here, our set of induction variables is not precise
        while (true) {
          int start_size = IndVarMap.size();
          // iterate through all blocks in the loop
          for (auto B: blks) {
            // iterate through all its instructions
            for (auto &I : *B) {
              // we only accept multiplication, addition, and subtraction
              // we only accept constant integer as one of theoperands
              if (auto *op = dyn_cast<BinaryOperator>(&I)) {
                Value *lhs = op->getOperand(0);
                Value *rhs = op->getOperand(1);
                // check if one of the operands belongs to indvars
                if (IndVarMap.count(lhs) || IndVarMap.count(rhs)) {
                  ////case: Add
                  if (I.getOpcode() == Instruction::Add || I.getOpcode() == Instruction::FAdd ) {
                    ConstantInt* CIL = dyn_cast<ConstantInt>(lhs);
                    ConstantInt* CIR = dyn_cast<ConstantInt>(rhs);
                    if (IndVarMap.count(lhs) && CIR) {

                      tuple<Value*, int, int> t = IndVarMap[lhs];
                      int new_val = CIR->getSExtValue() + get<2>(t);
                      IndVarMap[&I] = make_tuple(get<0>(t), get<1>(t), new_val);
                    } 
                    else if (IndVarMap.count(rhs) && CIL) {
                      
                      tuple<Value*, int, int> t = IndVarMap[rhs];
                      int new_val = CIL->getSExtValue() + get<2>(t);
                      IndVarMap[&I] = make_tuple(get<0>(t), get<1>(t), new_val);
                    }
                  } 
                  // case: Sub
                  else if (I.getOpcode() == Instruction::Sub || I.getOpcode() == Instruction::FSub) {
                    ConstantInt* CIL = dyn_cast<ConstantInt>(lhs);
                    ConstantInt* CIR = dyn_cast<ConstantInt>(rhs);
                    if (IndVarMap.count(lhs) &&  CIR) {
                      tuple<Value*, int, int> t = IndVarMap[lhs];
                      int new_val = get<2>(t) - CIR->getSExtValue();
                      IndVarMap[&I] = make_tuple(get<0>(t), get<1>(t), new_val);
                    } 
                    else if (IndVarMap.count(rhs) &&  CIL) {
                      tuple<Value*, int, int> t = IndVarMap[rhs];
                      int new_val = get<2>(t) - CIL->getSExtValue();
                      IndVarMap[&I] = make_tuple(get<0>(t), get<1>(t), new_val);
                    }
                  }
                  // case: Mul
                  else if (I.getOpcode() == Instruction::Mul || I.getOpcode() == Instruction::FMul) {
                    ConstantInt* CIL = dyn_cast<ConstantInt>(lhs);
                    ConstantInt* CIR = dyn_cast<ConstantInt>(rhs);
                    if (IndVarMap.count(lhs) && CIR) {
                      tuple<Value*, int, int> t = IndVarMap[lhs];
                      int new_val = CIR->getSExtValue() * get<1>(t);
                      IndVarMap[&I] = make_tuple(get<0>(t), new_val, get<2>(t));
                    } 
                    else if (IndVarMap.count(rhs) &&  CIL) {
                      tuple<Value*, int, int> t = IndVarMap[rhs];
                      int new_val = CIL->getSExtValue() * get<1>(t);
                      IndVarMap[&I] = make_tuple(get<0>(t), new_val, get<2>(t));
                    }
                  }
                  //case: div
                  else if (I.getOpcode() == Instruction::SDiv || 
                             I.getOpcode() == Instruction::UDiv || 
                             I.getOpcode() == Instruction::FDiv) {
                    ConstantInt* CIL = dyn_cast<ConstantInt>(lhs);
                    ConstantInt* CIR = dyn_cast<ConstantInt>(rhs);
                    if (IndVarMap.count(lhs) && CIR) {
                      tuple<Value*, int, int> t = IndVarMap[lhs];
                      int new_val =  get<1>(t)/ CIR->getSExtValue();
                      IndVarMap[&I] = make_tuple(get<0>(t), new_val, get<2>(t));
                    } 
                    else if (IndVarMap.count(rhs) &&  CIL) {
                      tuple<Value*, int, int> t = IndVarMap[rhs];
                      int new_val = get<1>(t) / CIL->getSExtValue();
                      IndVarMap[&I] = make_tuple(get<0>(t), new_val, get<2>(t));
                    }
                  }
                } // if operand in indvar
              } // if op is binop
            } // auto &I: B
          } // auto &B: blks

          if (IndVarMap.size() == start_size){

            outs()<<"nubmer of IVs: " << start_size << "\n";
            for(auto& kv : IndVarMap){
              kv.first->printAsOperand(llvm::outs(), false);
              outs()<< " ,name = " << kv.first->getName() << ";\n";
            }
            break;
          } 
        }
    }

    //check to see if the icmp is the only usage of the basic induction variable
    bool LSRPass::checkUsage(Value* v, Value* icmp){

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

    bool LSRPass::replaceInductionVariable(Loop* L, DenseMap<Value*, tuple<Value*, int, int>>& IndVarMap){
        
        for (auto &I : *L->getHeader()) {
          // we insert at the first phi node
          if (ICmpInst *icmp = dyn_cast<ICmpInst>(&I)) {

            Value * LHS = icmp->getOperand(0);
            Value * RHS = icmp->getOperand(1);

            outs() << "found the cmp Instruction\n";
            LHS->printAsOperand(outs(), false);
            outs() << "\n";
            RHS->printAsOperand(outs(), false);
            outs() << "\n";

            if(IndVarMap.count(LHS) > 0 && checkUsage(LHS, icmp)){
              //randomly select a basic induction variable
              outs() << "identified the induction variable in the cmp Instruction: \n";
              LHS->printAsOperand(llvm::outs(), false);

              Value* selected;
              for(auto& kv :  IndVarMap){
                if(get<0>(kv.second) == LHS && kv.first != LHS){
                  selected = kv.first;
                  break;
                }
              }

              //create a icmp instruction
              if(selected){
                outs() << "selected a induction variable: \n";
                selected->printAsOperand(outs(), false);

                /*IRBuilder<> head_builder(icmp);

                ConstantInt* CIR = dyn_cast<ConstantInt>(RHS);
                int new_val = CIR->getSExtValue() * get<1>(IndVarMap[selected]) + get<2>(IndVarMap[selected]);
                Value* limit = ConstantInt::getSigned(CIR->getType(), new_val);

                head_builder.CreateCmp(icmp->getSignedPredicate(), selected, limit);

                //remove the orignal cmp instruction and the basic induction variable
                ((Instruction*)LHS)->removeFromParent();
                icmp->removeFromParent();*/
              }
            }
          }
        }

        return true;
    }

    bool LSRPass::runOnFunction(Function &F) {

      Module* module = F.getParent();
      // apply useful passes
      legacy::FunctionPassManager FPM(module);

      this->preprocess(F, FPM);

      outs() << "run onfunction has been called!!!!!!!!!!!\n";

      // perform constant prop and loop analysis
      // should not call other passes with runOnFunction
      // which may overwrite the original pass manager
      LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();

      // find all loop induction variables within a loop
      for(auto* L : LI) {

        // IndVarMap = {indvar: indvar tuple}
        // indvar tuple = (basic_indvar, scale, const)
        // indvar = basic_indvar * scale + const
        DenseMap<Value*, tuple<Value*, int, int>> IndVarMap;

        this->identifyInductionVariable(L, IndVarMap);

        // now modify the loop to apply strength reduction
        map<Value*, PHINode*> PhiMap;
        // note that after loop simplification
        // we will only have a unique header and preheader


        // all induction variables should have phi nodes in the header
        // notice that this might add additional variables, they are treated as basic induction
        // variables for now
        // the preheader block
        BasicBlock* b_preheader = L->getLoopPreheader();
        // the header block
        BasicBlock* b_header = L->getHeader();
        // the body block
        BasicBlock* b_body;

        // modify the preheader block by inserting new phi nodes
        Value* preheader_val;
        Instruction* insert_pos = b_preheader->getTerminator();
        for (auto &I : *b_header) {
          // we insert at the first phi node
          if (PHINode *PN = dyn_cast<PHINode>(&I)) {
            int num_income = PN->getNumIncomingValues();
            assert(num_income == 2);
            // find the preheader value of the phi node
            for (int i = 0; i < num_income; i++) {
              if (PN->getIncomingBlock(i) == b_preheader) {
                preheader_val = PN->getIncomingValue(i);
                } else {
                b_body = PN->getIncomingBlock(i);
              }
            }
            IRBuilder<> head_builder(&I);
            IRBuilder<> preheader_builder(insert_pos);
            for (auto &indvar: IndVarMap) {
              tuple<Value*, int, int> t = indvar.second;
              if (get<0>(t) == PN && (get<1>(t) != 1 || get<2>(t) != 0)) { // not a basic indvar
                // calculate the new indvar according to the preheader value
                Value* new_incoming = preheader_builder.CreateMul(preheader_val, 
                  ConstantInt::getSigned(preheader_val->getType(), get<1>(t)));
                new_incoming = preheader_builder.CreateAdd(new_incoming, 
                  ConstantInt::getSigned(preheader_val->getType(), get<2>(t)));
                PHINode* new_phi = head_builder.CreatePHI(preheader_val->getType(), 2);
                new_phi->addIncoming(new_incoming, b_preheader);
                PhiMap[indvar.first] = new_phi;
              }
            }
          }
        }

        // modify the new body block by inserting cheaper computation
        for (auto &indvar : IndVarMap) {
          tuple<Value*, int, int> t = indvar.second;
          if (PhiMap.count(indvar.first) && (get<1>(t) != 1 || get<2>(t) != 0)) { // not a basic indvar
            for (auto &I : *b_body) {
              if (auto op = dyn_cast<BinaryOperator>(&I)) {
                Value *lhs = op->getOperand(0);
                Value *rhs = op->getOperand(1);
                if (lhs == get<0>(t) || rhs == get<0>(t)) {
                  IRBuilder<> body_builder(&I);
                  tuple<Value*, int, int> t_basic = IndVarMap[&I];
                  int new_val = get<1>(t) * get<2>(t_basic);
                  PHINode* phi_val = PhiMap[indvar.first];
                  Value* new_incoming = body_builder.CreateAdd(phi_val, 
                      ConstantInt::getSigned(phi_val->getType(), new_val));
                  phi_val->addIncoming(new_incoming, b_body);
                }
              }
            }
          }
        }

        // replace all the original uses with phi-node
        for (auto &phi_val : PhiMap) {
          (phi_val.first)->replaceAllUsesWith(phi_val.second);
        }
      } //finish all loops

      // do another round of optimization
      FPM.doInitialization();
      FPM.run(F);
      FPM.doFinalization();

      return true;
    } // finish processing loops
}


char LSRPass::ID = 0;
static RegisterPass<LSRPass> X("lsr", "Stregth Reduction Pass",
                                    false /* Only looks at CFG */,
                                    true /* Not Analysis Pass */);

// Automatically enable the pass.
// http://adriansampson.net/blog/clangpass.html
static void registerSkeletonPass(const PassManagerBuilder &,
                         legacy::PassManagerBase &PM) {
  PM.add(new LSRPass());
}

static RegisterStandardPasses
  RegisterMyPass(PassManagerBuilder::EP_LoopOptimizerEnd,
                 registerSkeletonPass);
