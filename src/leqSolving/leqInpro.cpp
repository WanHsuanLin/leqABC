#include "leqSolving/leqSolving.hpp"

/**Function*************************************************************

  Synopsis    [make Ba input progressive]

  Description [assume the given aba is universal]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Leq_InputProgressive(Leq_Ba_t* pBa, unordered_set<string> & sU, bool bDeterminize){
  unsigned i, j, inId, u = sU.size();
  unsigned uNum = (unsigned)1 << sU.size(), vNum = (unsigned)1 << (Leq_BaNumInput(pBa)-Leq_BaNumInactInput(pBa)-sU.size());
  unsigned nChoiceNum = Leq_BaNumInactInput(pBa) ? (unsigned)1 << Leq_BaNumInactInput(pBa) : 0;
  abctime clk = Abc_Clock();
  // change the order of pi, put inactive pi before active pi, modify both vector of Ntk and id of obj
  Leq_BaModifyCirPiOrderU(pBa, sU);

  // prepare simulation cir
  Abc_Ntk_t * pNtk = Leq_BaTransCir(pBa);
  Abc_NtkMakeComb( pNtk, 0 );
  Leq_PrintNtkPo(pNtk);
  Abc_NtkShow( pNtk, 0, 0, 1 );
  getchar();
  Aig_Man_t * pMan;
  if ( Abc_NtkGetChoiceNum(pNtk) ){
    Abc_Print( 1, "Removing %d choices from the AIG.\n", Abc_NtkGetChoiceNum(pNtk) );
    Abc_AigCleanup((Abc_Aig_t *)pNtk->pManFunc);
  }
  pMan = Abc_NtkToDar( pNtk, 0, 0 );
  cerr << "The number of dangling node is " << Aig_ManCleanup( pMan ) << endl;
  Fra_Sml_t * pSml;

  cerr << "num of v " << vNum << ", num of u " << uNum << ", num of nChoice " << nChoiceNum << endl;
  
  // make ba progressive
  
  // maintain a set of reached state
  // maintain a set of new reach state
  // unordered_map<unsigned, vector<unordered_set<unsigned> *> > sRS;
  unordered_map<unsigned, vector<unordered_map <unsigned, unordered_set<unsigned> *> * > > sRS;
  unordered_set<unsigned> sCS;
  unsigned initS = Leq_Bin2Dec(pBa->_vIniVal);
  cerr << "initial state is " << initS << endl;
  // sRS[initS] = vector<unordered_set<unsigned> *>(uNum);
  sRS[initS] = vector<unordered_map <unsigned, unordered_set<unsigned> *> * >(uNum);
  for(i = 0; i < uNum; ++i){
    // sRS[initS][i] = new unordered_set<unsigned>;
    sRS[initS][i] = new unordered_map <unsigned, unordered_set<unsigned> *>;
  }
  sCS.emplace(initS);

  // while new reach state is not empty
    // for each u
      // for each v 
        // gen sim pattern for non-deter input
        // simulate
        // collect result and check
        // if there is one f0 = 0 
          // delete()
          // modify PO
        // else
          // if state is not in reached states
            // add to new reach states
          // record the reachable state s' from s
  unsigned cs;
  bool noV = false;
  unordered_set<unsigned>::iterator iter;
  // unordered_map<unsigned, vector<unordered_set<unsigned> *> >::iterator iterRS;
  unordered_map<unsigned, vector<unordered_map<unsigned, unordered_set<unsigned> *> * > >::iterator iterRS;
  // vector<unordered_set<unsigned> *> tmpNS;
  vector<unordered_map<unsigned, unordered_set<unsigned> *> * >  tmpNS;
  unordered_set<unsigned> tmpCS;
  unordered_set<unsigned> sSimResult;
  unsigned long simNum = nChoiceNum ? nChoiceNum * (Leq_BaNumInput(pBa)+Leq_BaNumState(pBa)) : (Leq_BaNumInput(pBa)+Leq_BaNumState(pBa));
  if ( simNum % Aig_ManCiNum(pMan) != 0 ){
    printf( "The number of binary digits (%d) is not divisible by the number of primary inputs (%d).\n", 
       simNum, Aig_ManCiNum(pMan) );
    return;
  }
  unsigned delNum = 0;
  bool newFra = 1;
  // pSml = Fra_SmlStart( pMan, 0, 1, Abc_BitWordNum(simNum / Aig_ManCiNum(pMan)) ); 
  while( !sCS.empty() ){
    // if(delNum % 20 == 0){
    //   Fra_SmlStop( pSml );
    //   pSml = Fra_SmlStart( pMan, 0, 1, Abc_BitWordNum(simNum / Aig_ManCiNum(pMan)) ); 
    // }
    iter = sCS.begin();
    cs = *iter;
    cerr << "current state is " << cs << endl; 
    tmpNS.reserve(uNum);
    for(i = 0; i < uNum; ++i){
      // tmpNS.emplace_back(new unordered_set<unsigned>);
      tmpNS.emplace_back(new unordered_map <unsigned, unordered_set<unsigned> *>);
    }
    tmpCS.clear();
    for( i=0; i<uNum; ++i){
      noV = true;
      for( j=0; j<vNum; ++j){
        // cerr << "simNum " << simNum << endl;
        Vec_Str_t * vSimPattern = Vec_StrAlloc(simNum);
        inId = i + (j << u);
        cerr << "input " << inId << " goes to state " << endl;
        // cerr << "[Info] collect pattern" << endl;
        Leq_GenSimPattern(pBa, cs, inId, nChoiceNum, vSimPattern);
        // Vec_StrIntPrint( vSimPattern );
        // cerr << "[Info] start simulation" << endl;
        if( newFra ){
          // cerr << "generate new sml " << endl;
          pSml = Leq_SimComb( pMan, vSimPattern );
          newFra = 0;
        }
        else{
          Leq_SimCombNew( vSimPattern, pSml );
        }
        // printf( "print p: %p\n", (void *)pSml );
        // cerr << "[Info] get sim outputs" << endl;
        sSimResult.clear();
        if( Leq_InproGetSimOutputs( pSml, nChoiceNum, sSimResult) ){
          noV = false;
          for(const unsigned & ns : sSimResult){
            // cerr << "next state is " << ns << endl;
            // cerr << "finish find, i =" << i << endl;
            // tmpNS[i]->emplace(ns);
            if(tmpNS[i]->find(j) == tmpNS[i]->end()){
              (*tmpNS[i])[j] = new unordered_set<unsigned>;
            }
            (*tmpNS[i])[j]->emplace(ns);
            tmpCS.emplace(ns);
          }
        }
      }
    // ! try
      if(noV){
        newFra = 1;
        cerr << "delete state " << cs << endl;
        getchar();
        // getchar();
        if(Leq_InProDeletState(initS, cs, sRS, pNtk, pMan)){
          //modify cir with const 0
          ++delNum;
          Leq_BaSetTransCir(pBa,Leq_Const0Cir(pBa));
          Leq_BaResetByTranCir(pBa);
          Fra_SmlStop( pSml );
          Aig_ManStop( pMan );
          cerr << "initial state is delete" << endl;
          cerr << "       Inpro Time: " << Abc_Clock() - clk << endl;
          return;
        }
        break;
      }
    }
    sCS.erase(iter);
    getchar();
    // the state is valid
    if(!noV){
      sRS[cs] = move(tmpNS);
      for(const unsigned & ns : tmpCS){
        iterRS = sRS.find(ns);
        if(iterRS==sRS.end()){
          // cerr << "add state " << ns << " to sCS and sRS" << endl;
          sCS.emplace(ns);
          // cerr << "success emplace sCS" << endl;
        }
      }
    //   int tmp = 0;
    //   cerr << "----------------------" << endl;
    //   for(auto v : sRS){
    //     cerr << v.first ;
    //     tmp = 0;
    //     for(auto & s : v.second){
    //       cerr << " with input " << tmp << " reach " ;
    //       ++tmp;
    //       for(const unsigned & x : (*s)){
    //         cerr << x << " " ;
    //       }
    //       cerr << endl;
    //     }
    //   }
    //   cerr << "----------------------" << endl;
    //   cerr << "current state " ;
    //   for(auto v : sCS){
    //     cerr << v << " " ;
    //   }
    //   cerr << endl <<  "----------------------" << endl;
    }
    // getchar();
  }
  // Vec_StrFree(vSimPattern);
  Fra_SmlStop( pSml );
  Aig_ManStop( pMan );
  // Abc_NtkShow( pNtk, 0, 0, 1 );
  Leq_NtkMakeSeq(pNtk, pBa->_vIniVal);
  cerr << "       Inpro Time: " << Abc_Clock() - clk << endl;
  Abc_NtkShow( pNtk, 0, 0, 1 );
  // getchar();
  Leq_BaSetTransCir(pBa,pNtk);

  if(bDeterminize){
    //TODO
    // vector<unsigned> vS;
    cerr << "[Info] Convert ABA to BA" << endl;
    map<unsigned,unsigned> mOldState2NewState;
    i = 1;
    for(auto itM : sRS){
      // vS.emplace_back(itM.first);
      // 0 reserve for dead state
      cerr << "old state " << itM.first << " is mapped to " << i << endl;
      mOldState2NewState[itM.first] = i;
      ++i;
    }
    // getchar();
    Leq_InproDeterminize(pBa, mOldState2NewState);
  }
}

// bool Leq_InProDeletState(const unsigned & initS, const unsigned & delS, unordered_map<unsigned, vector<unordered_set<unsigned> *> >  & sRS, Abc_Ntk_t * pNtk, Aig_Man_t * pMan){
bool Leq_InProDeletState(const unsigned & initS, const unsigned & delS, unordered_map<unsigned, vector<unordered_map<unsigned, unordered_set<unsigned> *> *> >  & sRS, Abc_Ntk_t * pNtk, Aig_Man_t * pMan){
  // for every state in sRS
    // if delS in its next state set
      // delete 
      // if vSuNS[s][u].empty()
        // delete s
  unordered_set<unsigned> vDelS{delS};
  unordered_set<unsigned> vtmpDelS;
  unordered_set<unsigned>::iterator iter;
  // unordered_map<unsigned, vector<unordered_set<unsigned> *> >::iterator iterMap;
  unordered_map<unsigned, vector<unordered_map<unsigned, unordered_set<unsigned> *> *> >::iterator iterMap;
  // cerr 
  while(!vDelS.empty()){
    // cerr << "enter while loop! " << endl;
    vtmpDelS.clear();
    for(const unsigned & s : vDelS){
      vtmpDelS.emplace(s);
    }
    // s.u <- {set};
    // NS.add(s);
    vDelS.clear();
    for(const unsigned & ds : vtmpDelS){
      cerr << "delete state " << ds << endl;
      // cerr << "initS = " << initS << endl;
      if(ds == initS){
        return true;
      }
      // cerr << "modify po" << endl;
      Leq_InProModifyNtkPO(ds, pNtk);
      Leq_InProModifyAigPO(ds, pMan);
      iterMap = sRS.find(ds);
      if(iterMap != sRS.end()){
        // cerr << "erase iterMap" << endl;
        for(auto& pSet : sRS[ds]){
          // cerr << pSet << endl;
          delete(pSet);
        }
        sRS.erase(iterMap);
      }
      vector<unsigned> delV;
      for(auto & itP : sRS){
        for(unordered_map<unsigned, unordered_set<unsigned>*>* itS : itP.second){
          delV.clear();
          for(pair<unsigned, unordered_set<unsigned>*> itM : (*itS)){ //map element
            if(itM.second->find(ds) != itM.second->end()){
              // cerr << "delete state in " << itM.first << endl;
              delV.emplace_back(itM.first);
            }
          } 
          for(unsigned v : delV){
            itS->erase(v);
            if(itS->empty() && itP.first != ds){
              // cerr << " empty ! delete " << itP.first << endl;
              vDelS.emplace(itP.first);
            }//
          }
          // iter = itS->find(ds);
          // if(iter != itS->end()){
          //   // cerr << "delete state in " << itP.first << endl;
          //   itS->erase(iter);
          //   if(itS->empty() && itP.first != ds){
          //     // cerr << " empty ! delete " << itP.first << endl;
          //     vDelS.emplace(itP.first);
          //   }
          // }
        }
      }
    }
  }
  return false;
}

void Leq_InProModifyNtkPO(const unsigned & cs, Abc_Ntk_t * pNtk){
  // add not cs to PO
  int i, latchNum = Abc_NtkPoNum(pNtk)-1;
  Abc_Obj_t * pF0 = Abc_NtkPo(pNtk, 0);
  Abc_Obj_t * pNewPoIn, * pNode, * pSo, * pSi = Abc_NtkPo(pNtk, latchNum);
  vector<bool> vResult(latchNum);
  Vec_Ptr_t * vPairs;
  vPairs = Vec_PtrAlloc( 200 );
  Leq_Dec2BinInt(cs, vResult);
  // i = Abc_NtkPiNum(pNtk)-latchNum;
  // for(const bool j : vResult){
  //   cerr << j ;
  //   if(j){
  //     Vec_PtrPush( vPairs, Abc_NtkPi(pNtk,i)); 
  //   }
  //   else{
  //     Vec_PtrPush( vPairs, Abc_ObjNot(Abc_NtkPi(pNtk,i))); 
  //   }
  //   ++i;
  // }
  i = 1;
  for(const bool j : vResult){
    cerr << j ;
    if(j){
      Vec_PtrPush( vPairs, Abc_ObjChild0(Abc_NtkPo(pNtk,i))); 
    }
    else{
      Vec_PtrPush( vPairs, Abc_ObjNot(Abc_ObjChild0(Abc_NtkPo(pNtk,i)))); 
    }
    ++i;
  }
  pSo = Abc_NtkPi(pNtk, Abc_NtkPiNum(pNtk)-1);
  pNode = Abc_ObjNot(Leq_SetVectorAnd( (Abc_Aig_t *)pNtk->pManFunc, (Abc_Obj_t **)vPairs->pArray, vPairs->nSize));
  pNewPoIn = Abc_AigAnd( (Abc_Aig_t *)pNtk->pManFunc, Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, pNode ,Abc_ObjChild0(pF0)), pSo);
  
  Abc_ObjRemoveFanins(pF0);
  Abc_ObjRemoveFanins(pSi);
  Abc_ObjAddFanin(pSi, pNewPoIn);
  Abc_ObjAddFanin( pF0, pNewPoIn);
  Vec_PtrFree(vPairs);
  Abc_NtkShow( pNtk, 0, 0, 1 );
  getchar();
}

void Leq_InProModifyAigPO(const unsigned & cs, Aig_Man_t * pMan){
  // add not cs to PO
  int i, latchNum = Aig_ManCoNum(pMan)-1;
  Aig_Obj_t * pF0 = Aig_ManCo(pMan, 0);
  Aig_Obj_t * pNewPoIn, * pNode, * pSo, * pSi = Aig_ManCo(pMan, latchNum);
  vector<bool> vResult(latchNum);
  Vec_Ptr_t * vPairs;
  vPairs = Vec_PtrAlloc( 200 );
  Leq_Dec2BinInt(cs, vResult);
  // i = Aig_ManCiNum(pMan)-latchNum;
  // for(const bool j : vResult){
  //   cerr << j ;
  //   if(j){
  //     Vec_PtrPush( vPairs, Aig_ManCi(pMan,i)); 
  //   }
  //   else{
  //     Vec_PtrPush( vPairs, Aig_Not(Aig_ManCi(pMan,i))); 
  //   }
  //   ++i;
  // }
  i = 1;
  for(const bool j : vResult){
    cerr << j ;
    if(j){
      Vec_PtrPush( vPairs, Aig_ObjChild0(Aig_ManCo(pMan,i))); 
    }
    else{
      Vec_PtrPush( vPairs, Aig_Not(Aig_ObjChild0(Aig_ManCo(pMan,i)))); 
    }
    ++i;
  }
  cerr << endl;
  pSo = Aig_ManCi(pMan, Aig_ManCiNum(pMan)-1);
  pNode = Aig_Not(Leq_SetVectorAigAnd( pMan, (Aig_Obj_t **)vPairs->pArray, vPairs->nSize));
  pNewPoIn = Aig_And( pMan, Aig_And(pMan, pNode ,Aig_ObjChild0(pF0)), pSo);

  Aig_ObjPatchFanin0(pMan, pSi, pNewPoIn);
  Aig_ObjPatchFanin0(pMan, pF0, pNewPoIn);
  Vec_PtrFree(vPairs);
}

/**Function*************************************************************

  Synopsis    [Convert Alternating BA to BA by input progressive]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Leq_InproDeterminize(Leq_Ba_t * pBa, map<unsigned,unsigned> & mOldState2NewState){
  // create a new circuit with latch num sRS.size()
  cerr << "start InproDeterminize" << endl;
  if(Leq_BaNumInactInput(pBa)==0) return;
  Abc_Ntk_t * pNtk = Leq_BaTransCir(pBa), * pNewNtk;

  cerr << "check the latch order" << endl;
  int i,j;
  Abc_Obj_t * pObj;
  Vec_PtrForEachEntryReverse( Abc_Obj_t *, pNtk->vBoxes, pObj, i ){
    cerr << "The latch " << i << " name is " << Abc_ObjName( pObj ) << ", with fanin node name "<< Abc_ObjName(Abc_ObjFanin0(pObj)) << " and fanout node name " << Abc_ObjName(Abc_ObjFanout0(pObj)) << endl;
  }
  
  // map<unsigned,unsigned> mOldState2NewState;
  

  cerr << "make pNtk comb" << endl;
  Abc_NtkMakeComb( pNtk, 0 );
  // generate sim pattern
  Aig_Man_t * pMan;
  if ( Abc_NtkGetChoiceNum(pNtk) ){
    Abc_Print( 1, "Removing %d choices from the AIG.\n", Abc_NtkGetChoiceNum(pNtk) );
    Abc_AigCleanup((Abc_Aig_t *)pNtk->pManFunc);
  }
  pMan = Abc_NtkToDar( pNtk, 0, 0 );
  unordered_set<unsigned> sSimResult;
  unsigned stateNum = mOldState2NewState.size()+1, piNum = (unsigned)1 << (Leq_BaNumInput(pBa)-Leq_BaNumInactInput(pBa)), nChoiceNum = (unsigned)1 << Leq_BaNumInactInput(pBa);
  unsigned long simNum = nChoiceNum ? nChoiceNum * (Leq_BaNumInput(pBa)+Leq_BaNumState(pBa)) : (Leq_BaNumInput(pBa)+Leq_BaNumState(pBa));
  Fra_Sml_t * pSml = Fra_SmlStart( pMan, 0, 1, Abc_BitWordNum(simNum / Aig_ManCiNum(pMan)) );
  vector<vector<set<unsigned> *> > vSimResult(stateNum, vector<set<unsigned> *>(piNum)); 
  for( i = 0; i < stateNum; ++i ){
    for( j = 0; j < piNum; ++j ){
      vSimResult[i][j] = new set<unsigned>;
    }
  }
  cerr << "[Info] start simulation " << endl;
  for(auto & iterM : mOldState2NewState){
    cerr << "current state is " << iterM.first << endl; 
    for( i = 0; i < piNum; ++i){
      Vec_Str_t * vSimPattern = Vec_StrAlloc(simNum);
      Leq_GenSimPattern(pBa, iterM.first, i, nChoiceNum, vSimPattern);  
      Leq_SimCombNew( vSimPattern, pSml );
      // cerr << "[Info] get sim outputs" << endl;
      sSimResult.clear();
      if( Leq_InproGetSimOutputs( pSml, nChoiceNum, sSimResult) ){
        if(iterM.first == 19) cerr << "19 goes to state ";
        for(auto & s : sSimResult){
          if(iterM.first == 19) cerr << s << " ";
          vSimResult[iterM.second][i]->emplace(mOldState2NewState[s]);
        }
        if(iterM.first == 19) cerr << endl;
      }
      else{
        if(iterM.first == 19) cerr << "19 goes to state 0" << endl;
        vSimResult[iterM.second][i]->emplace(0);
      }
    }
  }
  getchar();
  Fra_SmlStop( pSml );
  Aig_ManStop( pMan );
  // !

  // Create new transition function for each new state s
  cerr << "[Info] get next states" << endl;
  vector<vector<set<unsigned> *> > vNextState(stateNum, vector<set<unsigned> *>(piNum)); 
  Leq_BaGetNextState(stateNum, piNum, nChoiceNum, vSimResult, vNextState);
  
  set<unsigned> sF0{};
  set<unsigned> sNF0{0};
  for(auto & itM : mOldState2NewState){
    sF0.emplace(itM.second);
  }

  // start ntk by a new eqn
  cerr << "[Info] write eqn" << endl;
  pNewNtk = Leq_EqnWriteNetwork( pNtk, pBa, vNextState, sF0, sNF0 );

  pNewNtk = Abc_NtkToLogic( pNewNtk );
  Abc_NtkPrintStats( pNewNtk, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );

  // modify final state
  // ! deal with the initial state !!latch may be removed!!
  unsigned iniVal = mOldState2NewState[Leq_Bin2Dec(pBa->_vIniVal)];
  // for (i = 0; i < pBa->_vIniVal.size(); ++i){
  //   cerr << pBa->_vIniVal[i] << " " ;
  // }
  cerr << endl;
  pBa->_vIniVal.clear();
  pBa->_vIniVal.resize(stateNum,0);
  pBa->_vIniVal[iniVal] = 1;
  cerr << "state " << iniVal << " has initial value 1" << endl;

  // Leq_OptimizeLatch(pNewNtk, pBa->_vIniVal);

  Abc_NtkPrintStats( pNewNtk, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );

  cerr << "[Info] perform sweeping" << endl;
  Abc_NtkSweep( pNewNtk, 1 );

  cerr << "[Info] perform strash" << endl;
  pNewNtk = Abc_NtkStrash( pNewNtk, 0, 1, 0 ); 
  
  Abc_NtkPrintStats( pNewNtk, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );

  cerr << "[Info] perform dc2" << endl;
  pNewNtk = Abc_NtkDC2( pNewNtk, 0, 0, 1, 0, 1 );

  // Abc_NtkMakeSeq( pNewNtk, stateNum );

  cerr << "[Info] make seq" << endl;
  Leq_NtkMakeSeq(pNewNtk, pBa->_vIniVal);
  
  // char * pName;
  // Abc_NtkForEachLatch(pNewNtk, pObj, i){
  //   sprintf( pVarName, "s%d", i );
  //   Abc_ObjAssignName(pObj, pName);
  // }

  cerr << "[Info] Trans to logic" << endl;
  pNewNtk = Abc_NtkToLogic( pNewNtk ); //

  Abc_NtkPrintStats( pNewNtk, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );

  cerr << "[Info] optimize seq cir" << endl;
  Abc_NtkCleanupSeq( pNewNtk, 1, 0, 1 );
  
  
  Abc_NtkPrintStats( pNewNtk, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );

  Abc_NtkCleanupSeq( pNewNtk, 1, 0, 1 );
  Abc_NtkPrintStats( pNewNtk, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );

  pNewNtk = Abc_NtkStrash( pNewNtk, 0, 1, 0 ); 


  // Abc_NtkShow( pNewNtk, 0, 0, 1 );
  Abc_NtkPrintStats( pNewNtk, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
  // pNewNtk = Abc_NtkDarLcorr( pNewNtk, 0, 1000, 1 );
  // pNewNtk = Abc_NtkDarLcorrNew( pNewNtk, 1000, 1000, 1 );
  Abc_NtkPrintStats( pNewNtk, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
  
  // Abc_NtkShow( pNewNtk, 0, 0, 1 );
  Leq_BaSetTransCir(pBa, pNewNtk);

  Leq_BaResetByTranCir(pBa);
  // free set vector
  return;
}