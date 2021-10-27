#include "leqSolving/leqSolving.hpp"

/**Function*************************************************************

  Synopsis    [Construct BA from seq cir]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Leq_BaInit(Leq_Ba_t* pBa, Abc_Ntk_t * pNtk, bool bS, bool bConvert)
{
  if(UINT_MAX < Abc_NtkLatchNum(pNtk))
    Abc_Print(-2, "Too many states\n");

  if ( !Abc_NtkCheck( pNtk ) )
  {
    printf( "pNtk: The origin network check has failed.\n" );
    Abc_NtkDelete( pNtk );
    return;
  }

  pNtk = Abc_NtkStrash( pNtk, 0, 1, 0 ); 

  if(!bConvert)
    pBa->_pNtk = pNtk;
  else 
    Leq_BaInitTranCir(pBa, pNtk);
  
  if(!bConvert)
    Abc_NtkSetName(pBa->_pNtk, "baX");
  else
    if(bS)
      Abc_NtkSetName(pBa->_pNtk, "baS");
    else
      Abc_NtkSetName(pBa->_pNtk, "baF");

  pBa->_pName = Abc_NtkName(pBa->_pNtk);
  pBa->_bNondeterType = 0;

  cerr << "name for new _pNtk is " << Abc_NtkName(pBa->_pNtk) << endl;
  
  Abc_NtkPrintStats( pBa->_pNtk, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );

  // Abc_NtkShow( pBa->_pNtk, 0, 0, 1 );

  Leq_BaResetByTranCir(pBa);

  // initial value
  pBa->_vIniVal.resize(Abc_NtkLatchNum(pBa->_pNtk));
  
  Abc_Obj_t * pLatch;
  int i;
  Abc_NtkForEachLatch( pBa->_pNtk, pLatch, i ){
    if ( Abc_LatchIsInit0(pLatch) )
      pBa->_vIniVal[i] = 0;
    else if ( Abc_LatchIsInit1(pLatch) )
      pBa->_vIniVal[i] = 1;
    else{
      Abc_LatchSetInit0(pLatch);
      pBa->_vIniVal[i] = 0;
    }
  }
}

void Leq_BaResetByTranCir( Leq_Ba_t * pBa){
  Abc_Obj_t * pObj;
  int i;
  pBa->_mInactInput.clear();
  pBa->_mActInput.clear();
  Abc_NtkForEachPi( pBa->_pNtk, pObj, i ){
    pBa->_mActInput[string(Abc_ObjName(pObj))] = pObj;
  }

  pBa->_nStates = Abc_NtkLatchNum(pBa->_pNtk);
  pBa->_nInput  = Abc_NtkPiNum(pBa->_pNtk);
  pBa->_vIniVal.resize(Abc_NtkLatchNum(pBa->_pNtk));
  Abc_NtkForEachLatch( pBa->_pNtk, pObj, i ){
    if ( Abc_LatchIsInit0(pObj) )
      pBa->_vIniVal[i] = 0;
    else if ( Abc_LatchIsInit1(pObj) )
      pBa->_vIniVal[i] = 1;
    else{
      Abc_LatchSetInit0(pObj);
      pBa->_vIniVal[i] = 0;
    }
  }
}

/**Function*************************************************************

  Synopsis    [Print act and inact support]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Leq_BaPrintSupport(Leq_Ba_t * pBa){
  unordered_map<string, Abc_Obj_t *>::iterator it;
  unordered_set<string>::iterator itS;
  cerr << "=================print support===============" << endl;
  cerr << "The Total number of current support is " << pBa->_mActInput.size() << endl;
  for (it = pBa->_mActInput.begin(); it != pBa->_mActInput.end(); it++){
    if(it->second){
      cerr << it->first << "(" << Abc_ObjId(it->second) << ")" << endl;
    }
    else{
      cerr << it->first << "(-1)" << endl;
    }
  }
  cerr << endl;
  cerr << "The Total number of hidden support is " << pBa->_mInactInput.size() << endl;
  for (it = pBa->_mInactInput.begin(); it != pBa->_mInactInput.end(); it++){
    if(it->second){
      cerr << it->first << "(" << Abc_ObjId(it->second) << ")" << endl;
    }
    else{
      cerr << it->first << "(-1)" << endl;
    }
  }
  cerr << endl;
  cerr << "The Total number of dc support is " << pBa->_sDcInput.size() << endl;
  for (itS = pBa->_sDcInput.begin(); itS != pBa->_sDcInput.end(); itS++){
    cerr << *itS << " ";
  }
  cerr << endl << "============================================" << endl;
}

void Leq_BaPrintInitialState(Leq_Ba_t * pBa){
  cerr << "Initial state of " << Leq_BaName(pBa) << " : ";
  for(bool b : pBa->_vIniVal)
    cerr << " " << b;
  cerr << endl;
}
/**Function*************************************************************

  Synopsis    [Construct tran cir of BA from seq cir]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Leq_BaInitTranCir(Leq_Ba_t * pBa, Abc_Ntk_t * pNtkOri){ //, bool bS){
  // unordered_map<char*, Abc_Obj_t*> mName2Pi;
  Abc_Obj_t * pNode, * pPi, * pObj, * pObjNew, * pNewPo;
  Vec_Ptr_t * vPairs;
  vPairs = Vec_PtrAlloc( 100 );
  int i;
  pBa->_pNtk = Abc_NtkAlloc( ABC_NTK_STRASH, ABC_FUNC_AIG, 1 );
  
  Abc_AigConst1(pNtkOri)->pCopy = Abc_AigConst1(pBa->_pNtk);
  Abc_AigConst1(pNtkOri)->pCopy = Abc_AigConst1(pBa->_pNtk);

  Abc_NtkForEachPi( pNtkOri, pObj, i ){
    pObjNew = Abc_NtkCreatePi( pBa->_pNtk );
    pObj->pCopy = pObjNew;
    Abc_ObjAssignName( pObjNew, Abc_ObjName(pObj), NULL );
  }
  vector<Abc_Obj_t *> vPi(Abc_NtkPoNum(pNtkOri));
  Abc_NtkForEachPo( pNtkOri, pNode, i ){
    // cerr << "add one ppi" << endl;
    pPi = Abc_NtkCreatePi( pBa->_pNtk );
    vPi[i] = pPi;
    Abc_ObjAssignName( pPi, Abc_ObjName(pNode), NULL );
  }


  cerr << "finish PI" << endl;
  pNewPo = Abc_NtkCreatePo( pBa->_pNtk );  
  // if(bS)  Abc_ObjAssignName( pNewPo, "f0_s", NULL );
  // else Abc_ObjAssignName( pNewPo, "f0_f", NULL );
  Abc_ObjAssignName( pNewPo, "f0", NULL );
  
  cerr << "finish po" << endl;
  // create the latches
  Abc_NtkForEachLatch( pNtkOri, pObj, i ){
    pObjNew = Abc_NtkDupBox( pBa->_pNtk, pObj, 0 );
    // add names
    Abc_ObjAssignName( pObjNew, Abc_ObjName(pObj), NULL );
    Abc_ObjAssignName( Abc_ObjFanin0(pObjNew),  Abc_ObjName(Abc_ObjFanin0(pObj)), NULL );
    Abc_ObjAssignName( Abc_ObjFanout0(pObjNew), Abc_ObjName(Abc_ObjFanout0(pObj)), NULL );
    if(Abc_LatchIsInit1(pObj))
      Abc_LatchSetInit1( pObjNew );
    else
      Abc_LatchSetInit0( pObjNew );
    
  }

  cerr << "finish Latch" << endl;

  Abc_AigForEachAnd( pNtkOri, pNode, i )
    pNode->pCopy = Abc_AigAnd( (Abc_Aig_t *)pBa->_pNtk->pManFunc, Abc_ObjChild0Copy(pNode), Abc_ObjChild1Copy(pNode) );

  Abc_NtkForEachPo( pNtkOri, pNode, i ){
    Vec_PtrPush( vPairs, Abc_AigXor( (Abc_Aig_t *)pBa->_pNtk->pManFunc, Abc_ObjChild0Copy(pNode), vPi[i] )); /////
  }

  cerr << "finish PO" << endl;

  Abc_NtkForEachLatch( pNtkOri, pNode, i )
    Abc_ObjAddFanin( Abc_ObjFanin0(pNode)->pCopy, Abc_ObjChild0Copy(Abc_ObjFanin0(pNode)) );

  cerr << "connect new latches" << endl;

  // create s for input progressive
  
  Abc_Obj_t * pLatchOut, * pLatch, * pLatchIn;
  pLatchOut = Abc_NtkCreateBo(pBa->_pNtk);
  pLatch    = Abc_NtkCreateLatch(pBa->_pNtk);
  pLatchIn  = Abc_NtkCreateBi(pBa->_pNtk);

  Abc_ObjAssignName( pLatch, "Ba_ipS", NULL );
  Abc_ObjAssignName( pLatchOut, Abc_ObjName(pLatch), "_lo" );
  Abc_ObjAssignName( pLatchIn,  Abc_ObjName(pLatch), "_li" );
  Abc_ObjAddFanin( pLatchOut, pLatch );
  Abc_ObjAddFanin( pLatch, pLatchIn );
  Abc_LatchSetInit1( pLatch );


  pNode = Leq_SetVectorOr( (Abc_Aig_t *)pBa->_pNtk->pManFunc, (Abc_Obj_t **)vPairs->pArray, vPairs->nSize);
    // if(bS)
    //   pNewPo = Abc_ObjNot(Abc_AigAnd( (Abc_Aig_t *)pBa->_pNtk->pManFunc, Abc_ObjNot(pNode), pLatchOut ));
    // else
  pNewPo = (Abc_AigAnd( (Abc_Aig_t *)pBa->_pNtk->pManFunc, Abc_ObjNot(pNode), pLatchOut ));
  // Abc_ObjAddFanin( Abc_NtkPo(pBa->_pNtk,0), pNewPo);

  Abc_ObjAddFanin( pLatchIn, pNewPo );

  // if(bS){
  //   Abc_ObjAddFanin( Abc_NtkPo(pBa->_pNtk,0), Abc_ObjNot(pNewPo));
  // }
  // else 
  Abc_ObjAddFanin( Abc_NtkPo(pBa->_pNtk,0), pNewPo);

  Abc_AigCleanup((Abc_Aig_t *)pBa->_pNtk->pManFunc);

  // make sure that everything is okay
  if ( !Abc_NtkCheck( pBa->_pNtk ) )
  {
    printf( "pNtk: The network check has failed.\n" );
    Abc_NtkDelete( pBa->_pNtk );
    return;
  }
  // pBa->_pNtk = Abc_NtkDarLcorrNew( pBa->_pNtk, 1000, 1000, 1 );  
  // Abc_NtkShow( pBa->_pNtk, 0, 0, 1 );
  // getchar();

}



/**Function*************************************************************

  Synopsis    [Convert Alternating BA to BA]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Leq_Aba2Ba(Leq_Ba_t * pBa){
  // create a new circuit with latch num sRS.size()
  cerr << "start Determinization" << endl;
  if(Leq_BaNumInactInput(pBa)==0) return;
  Leq_BaModifyCirPiOrder(pBa);
  Abc_Ntk_t * pNtk = Leq_BaTransCir(pBa), * pNtkTmp1, * pNtkTmp2;
  int i,j;
  int fVerbose = 0;
  // Abc_Obj_t * pObj;
  map<unsigned,unsigned> mOldState2NewState;
  map<unsigned,unsigned>::iterator iterM;
  unsigned initS = Leq_Bin2Dec(pBa->_vIniVal);
  // unsigned id = 2, cs;
  unsigned id = 1, cs;
  unordered_set<unsigned> sCS{initS};
  unordered_set<unsigned> sRS{initS};
  unordered_set<unsigned> sSimResult;
  unsigned stateNum = (unsigned)1 << Leq_BaNumState(pBa), piNum = (unsigned)1 << (Leq_BaNumInput(pBa)-Leq_BaNumInactInput(pBa)), nChoiceNum = (unsigned)1 << Leq_BaNumInactInput(pBa);
  unsigned long simNum = nChoiceNum ? nChoiceNum * (Leq_BaNumInput(pBa)+Leq_BaNumState(pBa)) : (Leq_BaNumInput(pBa)+Leq_BaNumState(pBa));
  vector<vector<set<unsigned> *> > vSimResult(stateNum, vector<set<unsigned> *>(piNum)); 
  Aig_Man_t * pMan;
  Fra_Sml_t * pSml;
  // mOldState2NewState[initS] = 1;
  set<unsigned> sF0{};
  set<unsigned> sF0Tmp{};
  set<unsigned> sNF0{};
  set<unsigned> sNF0Tmp{};
  mOldState2NewState[initS] = 0;
  // for( i = 0; i < stateNum; ++i ){
  //   for( j = 0; j < piNum; ++j ){
  //     vSimResult[i][j] = new set<unsigned>;
  //   }
  // }
  for( j = 0; j < piNum; ++j ){
    vSimResult[0][j] = new set<unsigned>;
  }


  cerr << "make pNtk comb" << endl;
  Abc_NtkMakeComb( pNtk, 0 );
  // cerr << "PI order : " ;
  // Abc_NtkForEachPi(pNtk, pObj, i){
  //   cerr << Abc_ObjName(pObj) << " ";
  // }
  // cerr << endl;
  // cerr << "PO order : " ;
  // Abc_NtkForEachPo(pNtk, pObj, i){
  //   cerr << Abc_ObjName(pObj) << " ";
  // }
  // cerr << endl;

  // generate sim pattern
  if ( Abc_NtkGetChoiceNum(pNtk) ){
    Abc_Print( 1, "Removing %d choices from the AIG.\n", Abc_NtkGetChoiceNum(pNtk) );
    Abc_AigCleanup((Abc_Aig_t *)pNtk->pManFunc);
  }
  pMan = Abc_NtkToDar( pNtk, 0, 0 );
  pSml = Fra_SmlStart( pMan, 0, 1, Abc_BitWordNum(simNum / Aig_ManCiNum(pMan)) );
  cerr << "[Info] start simulation " << endl;
  while( !sCS.empty() ){
    cs = *sCS.begin();
    cerr << "current state is " << cs << endl; 
    for( i = 0; i < piNum; ++i){
      Vec_Str_t * vSimPattern = Vec_StrAlloc(simNum);
      Leq_GenSimPattern(pBa, cs, i, nChoiceNum, vSimPattern);  
      Leq_SimCombNew( vSimPattern, pSml );
      // cerr << "[Info] get sim outputs" << endl;
      sSimResult.clear();
      sF0Tmp.clear();
      sNF0Tmp.clear();
      Leq_GetSimOutputs( pSml, nChoiceNum, sSimResult, sF0Tmp, sNF0Tmp) ;
      for(auto & s : sSimResult){
        iterM = mOldState2NewState.find(s);
        if(iterM == mOldState2NewState.end()){
          mOldState2NewState[s] = id;
          // cerr << "(create new state " << id << "for " << s << ") ";
          sCS.emplace(s);
          for( j = 0; j < piNum; ++j ){
            vSimResult[id][j] = new set<unsigned>;
          }
          ++id;
        }
        vSimResult[mOldState2NewState[cs]][i]->emplace(mOldState2NewState[s]);
      }
        for(auto & s : sF0Tmp){
          sF0.emplace(mOldState2NewState[s]);
        }
        for(auto & s : sNF0Tmp){
          sNF0.emplace(mOldState2NewState[s]);
        }
        // cerr << endl;
      // }
      // else{
      //   cerr << cs << " goes to state 0" << endl;
      //   vSimResult[mOldState2NewState[cs]][i]->emplace(0);
      // }
    }
    // getchar();
    sCS.erase(cs);
    // cerr << "----------------------" << endl;
    // cerr << "mOldState2NewState" << endl;
    // for(auto & m : mOldState2NewState){
    //   cerr << m.first << " is mapped to " << m.second << endl;
    // }
    // cerr << "----------------------" << endl;
    // getchar();
    // int tmp = 0;
    // i = 0;
    // for(auto & m : mOldState2NewState){
    //   cerr << m.first ;
    //   for(auto & s : vSimResult[m.first]){
    //     if(!s->size()) break;
    //     cerr << " with input " << tmp << " reach " ;
    //     ++tmp;
    //     for(const unsigned & x : (*s)){
    //       cerr << x << " " ;
    //     }
    //     cerr << endl;
    //   }
    // }
    // cerr << "----------------------" << endl;
    // cerr << "current state " ;
    // for(auto v : sCS){
    //   cerr << v << " " ;
    // }
    // cerr << endl <<  "----------------------" << endl;
    
    // getchar();
  }
    
  Fra_SmlStop( pSml );
  Aig_ManStop( pMan );
  // !
  // cerr << "----------------------" << endl;
  // cerr << "mOldState2NewState" << endl;
  // for(auto & m : mOldState2NewState){
  //   cerr << m.first << " is mapped to " << m.second << endl;
  // }
  // cerr << "----------------------" << endl;
  // Create new transition function for each new state s
  cerr << "[Info] get next states" << endl;
  // assert(id == 1+mOldState2NewState.size());
  assert(id == mOldState2NewState.size());
  vector<vector<set<unsigned> *> > vNextState(id, vector<set<unsigned> *>(piNum)); 
  vSimResult.resize(id);
  Leq_BaGetNextState(id, piNum, nChoiceNum, vSimResult, vNextState);
  
  // start ntk by a new eqn
  cerr << "[Info] write eqn" << endl;
  pNtkTmp1 = Leq_EqnWriteNetwork( pNtk, pBa, vNextState, sF0, sNF0 );

  for(i = 0; i < id; ++i){
    for(j = 0; j < piNum; ++j){
      delete(vNextState[i][j]);
      delete(vSimResult[i][j]);
    }
  }
  cerr << "[Info] optimize circuit" << endl;

  pNtkTmp1 = Abc_NtkToLogic( pNtkTmp2 = pNtkTmp1 );
  Abc_NtkPrintStats( pNtkTmp1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
  // Abc_NtkPrintStats( pNtkTmp1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
  // Abc_NtkDelete(pNtkTmp2);
  // modify final state
  cerr << endl;
  pBa->_vIniVal.clear();
  pBa->_vIniVal.resize(id,0);
  pBa->_vIniVal[0] = 1;
  
  cerr << "[Info] perform sweeping" << endl;
  Abc_NtkSweep( pNtkTmp1, 1 );

  cerr << "[Info] perform strash" << endl;
  pNtkTmp1 = Abc_NtkStrash( pNtkTmp2 = pNtkTmp1, 0, 1, 0 ); 
  
  Abc_NtkPrintStats( pNtkTmp1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );

  cerr << "[Info] perform dc2" << endl;
  pNtkTmp1 = Abc_NtkDC2( pNtkTmp2 = pNtkTmp1, 0, 0, 1, 0, fVerbose );
  Abc_NtkDelete(pNtkTmp2);
  pNtkTmp1 = Abc_NtkDC2( pNtkTmp2 = pNtkTmp1, 0, 0, 1, 0, fVerbose );
  Abc_NtkDelete(pNtkTmp2);
  Fraig_Params_t Params, * pParams = &Params;
  memset( pParams, 0, sizeof(Fraig_Params_t) );
  pParams->nPatsRand  = 2048; // the number of words of random simulation info
  pParams->nPatsDyna  = 2048; // the number of words of dynamic simulation info
  pParams->nBTLimit   =  100; // the max number of backtracks to perform
  pParams->fFuncRed   =    1; // performs only one level hashing
  pParams->fFeedBack  =    1; // enables solver feedback
  pParams->fDist1Pats =    1; // enables distance-1 patterns
  pParams->fDoSparse  =    1; // performs equiv tests for sparse functions
  pParams->fChoicing  =    0; // enables recording structural choices
  pParams->fTryProve  =    0; // tries to solve the final miter
  pParams->fVerbose   =    0; // the verbosiness flag
  pParams->fVerboseP  =    0; // the verbosiness flag
  pNtkTmp1 = Abc_NtkFraig( pNtkTmp2 = pNtkTmp1, &Params, 0, 0 );
  Abc_NtkDelete(pNtkTmp2);
  cerr << "[Info] make seq" << endl;
  Leq_NtkMakeSeq(pNtkTmp1, pBa->_vIniVal);

  cerr << "[Info] Trans to logic" << endl;
  pNtkTmp1 = Abc_NtkToLogic( pNtkTmp2 = pNtkTmp1 ); //
  Abc_NtkDelete(pNtkTmp2);
  Abc_NtkPrintStats( pNtkTmp1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );

  cerr << "[Info] optimize seq cir" << endl;
  Abc_NtkCleanupSeq( pNtkTmp1, 1, 0, fVerbose );
  
  
  Abc_NtkPrintStats( pNtkTmp1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );

  Abc_NtkCleanupSeq( pNtkTmp1, 1, 0, fVerbose );
  Abc_NtkPrintStats( pNtkTmp1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );

  pNtkTmp1 = Abc_NtkStrash( pNtkTmp2 = pNtkTmp1, 0, 1, 0 ); 
  Abc_NtkDelete(pNtkTmp2);
  // Abc_NtkShow( pNewNtk, 0, 0, 1 );
  // Abc_NtkPrintStats( pNtkNew, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
  // pNewNtk = Abc_NtkDarLcorr( pNewNtk, 0, 1000, 1 );
  // pNewNtk = Abc_NtkDarLcorrNew( pNewNtk, 1000, 1000, 1 );
  // Abc_NtkPrintStats( pNtkNew, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
  
  // Abc_NtkShow( pNewNtk, 0, 0, 1 );
  Leq_BaSetTransCir(pBa, pNtkTmp1);

  Leq_BaResetByTranCir(pBa);
  // free set vector
  return;
}

void Leq_Aba2Ba2(Leq_Ba_t * pBa){
  // create a new circuit with latch num sRS.size()
  cerr << "start Determinization" << endl;
  if(Leq_BaNumInactInput(pBa)==0) return;
  Leq_BaModifyCirPiOrder(pBa);
  Abc_Ntk_t * pNtk = Leq_BaTransCir(pBa), * pNtkTmp1, * pNtkTmp2;
  int i,j;
  int fVerbose = 0;
  // Abc_Obj_t * pObj;
  map<unsigned,unsigned> mOldState2NewState;
  map<unsigned,unsigned>::iterator iterM;
  unsigned initS = Leq_Bin2Dec(pBa->_vIniVal);
  // unsigned id = 2, cs;
  unsigned id = 1, cs;
  unordered_set<unsigned> sCS{initS};
  unordered_set<unsigned> sRS{initS};
  unordered_set<unsigned> sSimResult;
  unsigned stateNum = (unsigned)1 << Leq_BaNumState(pBa), piNum = (unsigned)1 << (Leq_BaNumInput(pBa)-Leq_BaNumInactInput(pBa)), nChoiceNum = (unsigned)1 << Leq_BaNumInactInput(pBa);
  unsigned long simNum = nChoiceNum ? nChoiceNum * (Leq_BaNumInput(pBa)+Leq_BaNumState(pBa)) : (Leq_BaNumInput(pBa)+Leq_BaNumState(pBa));
  // cerr << "522 " << stateNum << ", " << piNum << endl;
  vector<vector<set<unsigned> *> > vSimResult(stateNum, vector<set<unsigned> *>(piNum,NULL)); 
  Aig_Man_t * pMan;
  Fra_Sml_t * pSml;
  // cerr << "524 " << endl;
  // mOldState2NewState[initS] = 1;
  set<unsigned> sF0{};
  set<unsigned> sF0Tmp{};
  set<unsigned> sNF0{};
  set<unsigned> sNF0Tmp{};
  mOldState2NewState[initS] = 0;
  // for( i = 0; i < stateNum; ++i ){
  //   for( j = 0; j < piNum; ++j ){
  //     vSimResult[i][j] = new set<unsigned>;
  //   }
  // }
  // for( j = 0; j < piNum; ++j ){
  //   vSimResult[0][j] = new set<unsigned>;
  // }


  cerr << "make pNtk comb" << endl;
  Abc_NtkMakeComb( pNtk, 0 );
  // cerr << "PI order : " ;
  // Abc_NtkForEachPi(pNtk, pObj, i){
  //   cerr << Abc_ObjName(pObj) << " ";
  // }
  // cerr << endl;
  // cerr << "PO order : " ;
  // Abc_NtkForEachPo(pNtk, pObj, i){
  //   cerr << Abc_ObjName(pObj) << " ";
  // }
  // cerr << endl;

  // generate sim pattern
  if ( Abc_NtkGetChoiceNum(pNtk) ){
    Abc_Print( 1, "Removing %d choices from the AIG.\n", Abc_NtkGetChoiceNum(pNtk) );
    Abc_AigCleanup((Abc_Aig_t *)pNtk->pManFunc);
  }
  pMan = Abc_NtkToDar( pNtk, 0, 0 );
  cerr << "[Info] start simulation " << endl;
  while( !sCS.empty() ){
    pSml = Fra_SmlStart( pMan, 0, 1, Abc_BitWordNum(simNum / Aig_ManCiNum(pMan)) );
    cs = *sCS.begin();
    cerr << "current state is " << cs << endl; 
    for( i = 0; i < piNum; ++i){
      Vec_Str_t * vSimPattern = Vec_StrAlloc(simNum);
      Leq_GenSimPattern(pBa, cs, i, nChoiceNum, vSimPattern);  
      Leq_SimCombNew( vSimPattern, pSml );
      // cerr << "[Info] get sim outputs" << endl;
      sSimResult.clear();
      sF0Tmp.clear();
      sNF0Tmp.clear();
      Leq_GetSimOutputs( pSml, nChoiceNum, sSimResult, sF0Tmp, sNF0Tmp) ;
      for(auto & s : sSimResult){
        iterM = mOldState2NewState.find(s);
        if(iterM == mOldState2NewState.end()){
          mOldState2NewState[s] = id;
          sCS.emplace(s);
          // for( j = 0; j < piNum; ++j ){
          //   vSimResult[id][j] = new set<unsigned>;
          // }
          ++id;
        }
        if(vSimResult[mOldState2NewState[s]][i] == NULL){
          vSimResult[mOldState2NewState[s]][i]  = new set<unsigned>;
        }
        vSimResult[mOldState2NewState[s]][i]->emplace(mOldState2NewState[cs]);
        // vSimResult[mOldState2NewState[cs]][i]->emplace(mOldState2NewState[s]);
      }
        for(auto & s : sF0Tmp){
          sF0.emplace(mOldState2NewState[s]);
        }
        for(auto & s : sNF0Tmp){
          sNF0.emplace(mOldState2NewState[s]);
        }
    }
    sCS.erase(cs);
    Fra_SmlStop( pSml );
  }
    
  Aig_ManStop( pMan );
  // assert(id == mOldState2NewState.size());
  // vector<vector<set<unsigned> *> > vNextState(id, vector<set<unsigned> *>(piNum)); 
  vSimResult.resize(id);
  // Leq_BaGetNextState(id, piNum, nChoiceNum, vSimResult, vNextState);
  
  // start ntk by a new eqn
  cerr << "[Info] write eqn" << endl;
  pNtkTmp1 = Leq_EqnWriteNetwork( pNtk, pBa, vSimResult, sF0, sNF0 );

  for(i = 0; i < id; ++i){
    for(j = 0; j < piNum; ++j){
      // delete(vNextState[i][j]);
      delete(vSimResult[i][j]);
    }
  }
  cerr << "[Info] optimize circuit" << endl;

  pNtkTmp1 = Abc_NtkToLogic( pNtkTmp2 = pNtkTmp1 );
  Abc_NtkPrintStats( pNtkTmp1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
  cerr << endl;
  pBa->_vIniVal.clear();
  pBa->_vIniVal.resize(id,0);
  pBa->_vIniVal[0] = 1;
  
  cerr << "[Info] perform sweeping" << endl;
  Abc_NtkSweep( pNtkTmp1, 1 );

  cerr << "[Info] perform strash" << endl;
  pNtkTmp1 = Abc_NtkStrash( pNtkTmp2 = pNtkTmp1, 0, 1, 0 ); 
  
  Abc_NtkPrintStats( pNtkTmp1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );

  cerr << "[Info] perform dc2" << endl;
  pNtkTmp1 = Abc_NtkDC2( pNtkTmp2 = pNtkTmp1, 0, 0, 1, 0, fVerbose );
  Abc_NtkDelete(pNtkTmp2);
  pNtkTmp1 = Abc_NtkDC2( pNtkTmp2 = pNtkTmp1, 0, 0, 1, 0, fVerbose );
  Abc_NtkDelete(pNtkTmp2);
  Fraig_Params_t Params, * pParams = &Params;
  memset( pParams, 0, sizeof(Fraig_Params_t) );
  pParams->nPatsRand  = 2048; // the number of words of random simulation info
  pParams->nPatsDyna  = 2048; // the number of words of dynamic simulation info
  pParams->nBTLimit   =  100; // the max number of backtracks to perform
  pParams->fFuncRed   =    1; // performs only one level hashing
  pParams->fFeedBack  =    1; // enables solver feedback
  pParams->fDist1Pats =    1; // enables distance-1 patterns
  pParams->fDoSparse  =    1; // performs equiv tests for sparse functions
  pParams->fChoicing  =    0; // enables recording structural choices
  pParams->fTryProve  =    0; // tries to solve the final miter
  pParams->fVerbose   =    0; // the verbosiness flag
  pParams->fVerboseP  =    0; // the verbosiness flag
  pNtkTmp1 = Abc_NtkFraig( pNtkTmp2 = pNtkTmp1, &Params, 0, 0 );
  Abc_NtkDelete(pNtkTmp2);
  cerr << "[Info] make seq" << endl;
  Leq_NtkMakeSeq(pNtkTmp1, pBa->_vIniVal);

  cerr << "[Info] Trans to logic" << endl;
  pNtkTmp1 = Abc_NtkToLogic( pNtkTmp2 = pNtkTmp1 ); //
  Abc_NtkDelete(pNtkTmp2);
  Abc_NtkPrintStats( pNtkTmp1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );

  cerr << "[Info] optimize seq cir" << endl;
  Abc_NtkCleanupSeq( pNtkTmp1, 1, 0, fVerbose );
  
  
  Abc_NtkPrintStats( pNtkTmp1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );

  Abc_NtkCleanupSeq( pNtkTmp1, 1, 0, fVerbose );
  Abc_NtkPrintStats( pNtkTmp1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );

  pNtkTmp1 = Abc_NtkStrash( pNtkTmp2 = pNtkTmp1, 0, 1, 0 ); 
  Abc_NtkPrintStats( pNtkTmp1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
  Abc_NtkDelete(pNtkTmp2);
  Leq_BaSetTransCir(pBa, pNtkTmp1);

  Leq_BaResetByTranCir(pBa);
  // free set vector
  return;
}

void Leq_BaGetNextState(const int & stateNum, const int & piNum, const int & nChoiceNum, vector<vector<set<unsigned>* > > & vSimResult, vector<vector<set<unsigned>* > > & vNextState){
  int i, j;
  for(i = 0; i < stateNum; ++i){ //each state vector
    for(j = 0; j < piNum; ++j){ //each input
      vNextState[i][j] = new set<unsigned> ;
    }
  }
  for(i = 0; i < stateNum; ++i){ //each state vector
    for(j = 0; j < piNum; ++j){ //each input
      for(auto & cs : (*vSimResult[i][j])){
        vNextState[cs][j]->insert(i);   // rba case
        // vNextState[i][j]->insert(cs);      // ba case
      }
    }
  }
  // print transition function
  // for(i = 0; i < stateNum; ++i){ //each state vector
  //   for(j = 0; j < piNum; ++j){ //each input
  //     cerr << "state " << i << " with input " << j << " goes to state ";
  //     for( auto & ns : *vNextState[i][j])
  //       cerr << ns << " ";
  //     cerr << endl;
  //   }
  //   // getchar();
  // }
}


/**Function*************************************************************

  Synopsis    [Modify Circuit PI order according to the support]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Leq_BaModifyCirPiOrder(Leq_Ba_t * pBa){
  Abc_Obj_t * pObj;
  vector<Abc_Obj_t *> vpInactPi; 
  vector<int> vpInactPiPos; 
  vector<Abc_Obj_t *> vpActPi; 
  vector<int> vpActPiPos;  
  int i;
  Abc_Ntk_t * pNtk = Leq_BaTransCir(pBa);
  // print each PI
  // cerr << "Id before swapping:" << endl;
  // Leq_PrintNtkPi(pNtk);
  unordered_map<string, Abc_Obj_t *>::iterator itIn;
  int inactBegin = Leq_BaNumInactInput(pBa);
  // cerr << "inactBegin = " << inactBegin << endl;
  Abc_NtkForEachPi(pNtk, pObj, i){
    // cerr << "obj name : " << Abc_ObjName(pObj) << endl;
    itIn = pBa->_mInactInput.find(string(Abc_ObjName(pObj)));
    if(i < inactBegin && itIn == pBa->_mInactInput.end()){
      vpActPi.emplace_back(pObj);
      vpActPiPos.emplace_back(i);
      // cerr << "vpActPi add obj: " << Abc_ObjName(pObj) << endl;
    }
    if(i >= inactBegin && itIn != pBa->_mInactInput.end()){
      vpInactPi.emplace_back(pObj);
      vpInactPiPos.emplace_back(i);
      // cerr << "vpInactPi add obj: " << Abc_ObjName(pObj) << endl;
    }
  }
  // cerr << "modify ci" << endl;
  assert(vpActPi.size()==vpInactPi.size());
  for(i = 0; i < vpActPi.size(); ++i){
    // assert(Vec_PtrEntry( pNtk->vPis, Abc_ObjId(vpActPi[i]) ) == vpActPi[i]); id of obj doesn't equal to the position in pi vector 
    // ! May cause bug in simulation since we didn't modify id
    pNtk->vPis->pArray[vpActPiPos[i]] = vpInactPi[i];
    pNtk->vPis->pArray[vpInactPiPos[i]] = vpActPi[i];
    // modified Ci array
    pNtk->vCis->pArray[vpActPiPos[i]] = vpInactPi[i];
    pNtk->vCis->pArray[vpInactPiPos[i]] = vpActPi[i];
  }

  // cerr << "Id after swapping:" << endl;
  // Leq_PrintNtkPi(pNtk);
}

/**Function*************************************************************

  Synopsis    [Modify Circuit PI order according to the u,v]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Leq_BaModifyCirPiOrderU(Leq_Ba_t * pBa, unordered_set<string> & sU){
  Abc_Obj_t * pObj;
  vector<Abc_Obj_t *> vpVPi; 
  vector<int> vVPiPos; 
  vector<Abc_Obj_t *> vpUPi; 
  vector<int> vUPiPos;  
  int i;
  Abc_Ntk_t * pNtk = Leq_BaTransCir(pBa);
  // print each PI
  // cerr << "PI before swapping:" << endl;
  // Leq_PrintNtkPi(pNtk);
  unordered_set<string>::iterator itU;
  int uBegin = Leq_BaNumInput(pBa) - sU.size();
  // for (auto str : sU) cerr << "sU : " << str << ";" << endl;
  // cerr << endl;
  // char * p = "G0", * t = "G0";
  // cerr << &p << ",  " << &t << endl;
  // sU.emplace(p);
  // sU.emplace(t);
  // for (auto str : sU) cerr << "sU : " << str << ";" << endl;

  // Leq_BaNumInactInput(pBa) + sV.size();
  // cerr << "Leq_BaNumInput(pBa) = " << Leq_BaNumInput(pBa) << endl;
  // cerr << "sU.size() = " << sU.size() << endl;
  // cerr << "uBegin = " << uBegin << endl;
  // cerr << "Leq_BaNumInactInput(pBa) = " << Leq_BaNumInactInput(pBa) << endl;
  // cerr << "Abc_NtkPiNum(pNtk) = " << Abc_NtkPiNum(pNtk) << endl;

  for ( i = Leq_BaNumInactInput(pBa); (i < Abc_NtkPiNum(pNtk)) && (((pObj) = Abc_NtkPi(pNtk, i)), 1); i++ ){
    // cerr << "obj name : " << Abc_ObjName(pObj) << endl;
    itU = sU.find(string(Abc_ObjName(pObj)));
    // cerr << "i = " << i << ", ptr = " << (itU == sU.end()) << endl;
    if((i < uBegin) && (itU != sU.end())){
      vpUPi.emplace_back(pObj);
      vUPiPos.emplace_back(i);
      // cerr << "vpUPi add obj: " << Abc_ObjName(pObj) << endl;
    }
    if((i >= uBegin) && (itU == sU.end())){
      vpVPi.emplace_back(pObj);
      vVPiPos.emplace_back(i);
      // cerr << "vpVPi add obj: " << Abc_ObjName(pObj) << endl;
    }
  }
  // cerr << "modify ci" << endl;
  // cerr << "vpVPi.size() : " << vpVPi.size() << ", vpUPi.size() : " << vpUPi.size()  << endl;
  assert(vpVPi.size()==vpUPi.size());
  for(i = 0; i < vpVPi.size(); ++i){
    // assert(Vec_PtrEntry( pNtk->vPis, Abc_ObjId(vpActPi[i]) ) == vpActPi[i]); id of obj doesn't equal to the position in pi vector 
    // ! May cause bug in simulation since we didn't modify id
    pNtk->vPis->pArray[vUPiPos[i]] = vpVPi[i];
    pNtk->vPis->pArray[vVPiPos[i]] = vpUPi[i];
    // modified Ci array
    pNtk->vCis->pArray[vUPiPos[i]] = vpVPi[i];
    pNtk->vCis->pArray[vVPiPos[i]] = vpUPi[i];
  }

  // cerr << "PI after swapping:" << endl;
  // Leq_PrintNtkPi(pNtk);
}

void Leq_BaSetTransCir(Leq_Ba_t * pBa, Abc_Ntk_t * pNtk) {
  pBa->_pNtk = pNtk;
  Abc_Obj_t * pObj;
  int i;
  Abc_NtkForEachPi(pNtk, pObj, i){
    string str(Abc_ObjName(pObj));
    if(pBa->_mInactInput.find(str)==pBa->_mInactInput.end()){
      pBa->_mActInput[str] = pObj;
    }
    else{
      pBa->_mInactInput[str] = pObj;
    }
  }
}

Leq_Ba_t * Leq_BaDuplicate(Leq_Ba_t * pBa){
  Leq_Ba_t * pBaDup = new Leq_Ba_t();
  // dup name 
  Leq_BaSetName(pBaDup, Leq_BaName(pBa));  
  // dup num of state
  Leq_BaSetNumState(pBaDup, Leq_BaNumState(pBa));
  // dup num of input
  Leq_BaSetNumInput(pBaDup, Leq_BaNumInput(pBa));
  // dup nondeterType
  Leq_BaSetNondeterType(pBaDup, Leq_BaNondeterType(pBa));
  // duplitcate transition circuit
  Abc_Ntk_t * pNtkDup = Abc_NtkDup(Leq_BaTransCir(pBa));
  Leq_BaSetTransCir(pBaDup, pNtkDup);
  // dup inival
  for(bool val : pBa->_vIniVal){
    pBaDup->_vIniVal.emplace_back(val);
  }
  // dup ActInput
  for(pair<string, Abc_Obj_t *> pairNameObj : pBa->_mActInput){
    pBaDup->_mActInput.insert(pairNameObj);
  }
  // dup DCInput
  for(string str : pBa->_sDcInput){
    pBaDup->_sDcInput.emplace(str);
  }
  // dup DCObj
  for(Abc_Obj_t * pObj : pBa->_sDcObj){
    pBaDup->_sDcObj.emplace(pObj);
  }
  // dup InActinput
  for(pair<string, Abc_Obj_t *> pairNameObj : pBa->_mInactInput){
    pBaDup->_mInactInput.insert(pairNameObj);
  }
  return pBaDup;
}
