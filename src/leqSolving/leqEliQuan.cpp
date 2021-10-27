#include "leqSolving/leqSolving.hpp"

// use abc quantifier
void Leq_EliminateVar(Abc_Ntk_t * &pNtk, unsigned uBegin, Leq_Ba_t * pBa){ //type 0 -> exist, 1-> universal
  bool type;
  int i, j;
  int fVerbose = 0;
  Abc_Ntk_t * pNtkNew;
  Abc_Obj_t * pObj;
  Fraig_Params_t Params, * pParams = &Params;
  int origLatchNum = Abc_NtkLatchNum(pNtk);
  // !
  Abc_NtkMakeComb(pNtk, 0);
  Abc_NtkPrintStats( pNtk, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
  // !
  pNtk = Abc_NtkMakeOnePo(pNtk, 0, 1);
  Abc_NtkPrintStats( pNtk, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
  

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
  for(i = Abc_NtkPiNum(pNtk)-1-origLatchNum; i >= 0 ; --i){
    if ( Abc_NtkGetChoiceNum( pNtk ) ){
      Abc_Print( -1, "This command cannot be applied to an AIG with choice nodes.\n" );
    }
    type = (i < Leq_BaNumInactInput(pBa) ) ? pBa->_bNondeterType : (i > uBegin);
    if(fVerbose) cerr << "cofactor input " << Abc_ObjName(Abc_NtkPi(pNtk, i)) << " with type " << type << endl; 
    Abc_NtkQuantify( pNtk, type, i, fVerbose );
    // cerr << "finish input " << endl;
    // clean temporary storage for the cofactors
    // Abc_NtkCleanData( pNtk );
    Abc_NtkForEachObj( pNtk, pObj, j ){
      if(!Abc_ObjIsLatch(pObj))
        pObj->pData = NULL;
    }
    Abc_AigCleanup( (Abc_Aig_t *)pNtk->pManFunc );
    // if(i % 10 == 0){
    //   pNtkNew = Abc_NtkFraig( pNtk, &Params, 0, 0 );
    //   Abc_NtkDelete(pNtk);
    //   pNtk = pNtkNew;
    // }
    pNtkNew = Abc_NtkStrash( pNtk, 0, 1, 0 ); 
    Abc_NtkDelete(pNtk);
    pNtk = pNtkNew;
    if(i % 3 == 0 && Abc_NtkObjNum(pNtk) > 500){
      pNtkNew = Abc_NtkDC2( pNtk, 0, 0, 1, 0, fVerbose );
      Abc_NtkDelete(pNtk);
      pNtk = pNtkNew;
    }
    // cerr << "show pNtk after Leq_EliminateVar" << endl;
    // getchar();
    // Abc_NtkShow( pNtk, 0, 0, 1 );
    // getchar();
    if(fVerbose) Abc_NtkPrintStats( pNtk, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
  }

  // cerr << "[Info] perform dc2" << endl;
  if(Abc_NtkObjNum(pNtk) > 1000){
    pNtkNew = Abc_NtkDC2( pNtk, 0, 0, 1, 0, fVerbose );
    Abc_NtkDelete(pNtk);
  }
  else{
    pNtkNew = pNtk;
  }
  pNtk = Abc_NtkFraig( pNtkNew, &Params, 0, 0 );
  Abc_NtkDelete(pNtkNew);
  // pNtk = pNtkNew;
  // cerr << "show pNtk after all Leq_EliminateVar" << endl;
  // Abc_NtkShow( pNtk, 0, 0, 1 );
  // getchar();
  if(fVerbose)Abc_NtkPrintStats( pNtk, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
  // pNtk = Abc_NtkDarLatchSweep( pNtk, 1, 1, 1, 0, 1, 512, 1, 1 );
  return;
}

// use leq_quantifiy
void Leq_EliminateVar1(Abc_Ntk_t * &pNtk, unsigned uBegin, Leq_Ba_t * pBa){ //type 0 -> exist, 1-> universal
  bool type;
  int i, j;
  int fVerbose = 0;
  Abc_Ntk_t * pNtkNew;
  Abc_Obj_t * pObj;
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
  for(i = Abc_NtkPiNum(pNtk)-1; i >= 0 ; --i){
    if ( Abc_NtkGetChoiceNum( pNtk ) ){
      Abc_Print( -1, "This command cannot be applied to an AIG with choice nodes.\n" );
    }
    type = (i < Leq_BaNumInactInput(pBa) ) ? pBa->_bNondeterType : (i > uBegin);
    cerr << "cofactor input " << Abc_ObjName(Abc_NtkPi(pNtk, i)) << " with type " << type << endl; 
    pNtk = Leq_NtkQuantify( pNtk, type, i, 1 );
    cerr << "finish input " << endl;
    // clean temporary storage for the cofactors
    // Abc_NtkCleanData( pNtk );
    Abc_NtkForEachObj( pNtk, pObj, j ){
      if(!Abc_ObjIsLatch(pObj))
        pObj->pData = NULL;
    }
    Abc_AigCleanup( (Abc_Aig_t *)pNtk->pManFunc );
    // if(i % 10 == 0){
    //   pNtkNew = Abc_NtkFraig( pNtk, &Params, 0, 0 );
    //   Abc_NtkDelete(pNtk);
    //   pNtk = pNtkNew;
    // }
    pNtkNew = Abc_NtkStrash( pNtk, 0, 1, 0 ); 
    Abc_NtkDelete(pNtk);
    pNtk = pNtkNew;
    if(i % 3 == 0){
      pNtkNew = Abc_NtkDC2( pNtk, 0, 0, 1, 0, fVerbose );
      Abc_NtkDelete(pNtk);
      pNtk = pNtkNew;
    }
    // cerr << "show pNtk after Leq_EliminateVar" << endl;
    // getchar();
    // Abc_NtkShow( pNtk, 0, 0, 1 );
    // getchar();
    Abc_NtkPrintStats( pNtk, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
  }

  // cerr << "[Info] perform dc2" << endl;
  pNtkNew = Abc_NtkDC2( pNtk, 0, 0, 1, 0, fVerbose );
  Abc_NtkDelete(pNtk);
  pNtk = Abc_NtkFraig( pNtkNew, &Params, 0, 0 );
  Abc_NtkDelete(pNtkNew);
  // pNtk = pNtkNew;
  // cerr << "show pNtk after all Leq_EliminateVar" << endl;
  // Abc_NtkShow( pNtk, 0, 0, 1 );
  // getchar();
  Abc_NtkPrintStats( pNtk, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
  // pNtk = Abc_NtkDarLatchSweep( pNtk, 1, 1, 1, 0, 1, 512, 1, 1 );
  return;
}


Abc_Ntk_t * Leq_NtkQuantify( Abc_Ntk_t * pNtk, bool type, int pi, bool verbose ){ // 0 for exist, 1 for universal
  Abc_Ntk_t * pNtkNew;
  // Abc_Ntk_t * pNtkQ;
  Abc_Ntk_t * pNtkDup = Abc_NtkDup(pNtk);
  char Buffer[100];
  // pNtkQ = Abc_NtkToLogic(pNtkDup);
  // Abc_NtkDelete(pNtkDup);
  // Abc_Obj_t * pPi = Abc_NtkPi(pNtkQ, pi);
  // if(type){
  //   Abc_ObjReplaceByConstant( pPi, 0 );
  // }
  // else{
  //   Abc_ObjReplaceByConstant( pPi, 1 );
  // }
  // pNtkDup = Abc_NtkStrash( pNtkQ, 0, 1, 0 ); 
  // Abc_NtkDelete(pNtkQ);
  Abc_NtkOrderObjsByName( pNtk, 0 );
  Abc_NtkOrderObjsByName( pNtkDup, 0 );
  // Abc_NtkMakeComb(pNtkDup, 0);
  // pNtkDup = Abc_NtkMakeOnePo(pNtkDup, 0, 1);

  pNtkNew = Abc_NtkAlloc( ABC_NTK_STRASH, ABC_FUNC_AIG, 1 );
  sprintf( Buffer, "quantifyCir" );
  pNtkNew->pName = Extra_UtilStrsav(Buffer);

  // perform strashing
  // cerr << "Leq_AddQuantifyCirPrepare" << endl;
  Leq_AddQuantifyCirPrepare( pNtk, pNtkDup, pNtkNew, pi, type );
  // cerr << "Leq_AddQuantifyCirFinalize" << endl;
  Leq_AddQuantifyCirFinalize( pNtk, pNtkDup, pNtkNew, pi);
  // cerr << "Abc_AigCleanup" << endl;
  Abc_AigCleanup((Abc_Aig_t *)pNtkNew->pManFunc);

  // make sure that everything is okay
  if ( !Abc_NtkCheck( pNtkNew ) ){
    printf( "Abc_NtkMiter: The network check has failed.\n" );
    Abc_NtkDelete( pNtkNew );
    return NULL;
  }
  Abc_NtkDelete(pNtk);
  Abc_NtkDelete(pNtkDup);
  return pNtkNew;
} 


void Leq_AddQuantifyCirPrepare( Abc_Ntk_t * pNtk, Abc_Ntk_t * pNtkQ, Abc_Ntk_t * pNtkMiter, int qPi, bool type ){
  Abc_Obj_t * pObj, * pObjNew;
  int i;
  Abc_AigConst1(pNtk)->pCopy = Abc_AigConst1(pNtkMiter);
  Abc_AigConst1(pNtkQ)->pCopy = Abc_AigConst1(pNtkMiter);

  // create new PIs and remember them in the old PIs
  Abc_NtkForEachPi( pNtkQ, pObj, i ){
    if(i == qPi) {
      if(type)
        pObj->pCopy = Abc_ObjNot(Abc_AigConst1(pNtkMiter));
      else
        pObj->pCopy = Abc_AigConst1(pNtkMiter);
      continue;
    }
    pObjNew = Abc_NtkCreatePi( pNtkMiter );
    // remember this PI in the old PIs
    pObj->pCopy = pObjNew;
    pObj = Abc_NtkPi(pNtk, i);  
    pObj->pCopy = pObjNew;
    // add name
    Abc_ObjAssignName( pObjNew, Abc_ObjName(pObj), NULL );
  }

  pObjNew = Abc_NtkCreatePo( pNtkMiter );
  Abc_ObjAssignName( pObjNew, "miter", NULL );
  // create the latches
  vector<Abc_Obj_t *> vLatch;
  Abc_NtkForEachLatch( pNtkQ, pObj, i ){
    pObjNew = Abc_NtkDupBox( pNtkMiter, pObj, 0 );
    // add names
    Abc_ObjAssignName( pObjNew, Abc_ObjName(pObj), "_1" );
    Abc_ObjAssignName( Abc_ObjFanin0(pObjNew),  Abc_ObjName(Abc_ObjFanin0(pObj)), "_1" );
    Abc_ObjAssignName( Abc_ObjFanout0(pObjNew), Abc_ObjName(Abc_ObjFanout0(pObj)), "_1" );
    vLatch.emplace_back(pObjNew);
  }
  int j = 0;
  Abc_NtkForEachLatch( pNtk, pObj, i ){
    pObj -> pCopy = vLatch[j];
    Abc_ObjFanin0(pObj) -> pCopy = Abc_ObjFanin0(vLatch[j]);
    Abc_ObjFanout0(pObj) -> pCopy = Abc_ObjFanout0(vLatch[j]);
    ++j;
  }
}


void Leq_AddQuantifyCirFinalize( Abc_Ntk_t * pNtk, Abc_Ntk_t * pNtkQ, Abc_Ntk_t * pNtkNew, int qPi){
  Abc_Obj_t * pNode;
  int i;
  // cerr << "and for pNtkQ" << endl;
  assert( Abc_NtkIsDfsOrdered(pNtkQ) );
  Abc_AigForEachAnd( pNtkQ, pNode, i ){
    // cerr << pNode << ", " << Abc_ObjChild0Copy(pNode) << ", " << Abc_ObjChild1Copy(pNode) << endl;
    pNode->pCopy = Abc_AigAnd( (Abc_Aig_t *)pNtkNew->pManFunc, Abc_ObjChild0Copy(pNode), Abc_ObjChild1Copy(pNode) );
  }
  // cerr << "qpi for pNtk" << endl;
  Abc_NtkPi(pNtk, qPi)->pCopy = Abc_ObjChild0Copy(Abc_NtkPo(pNtkQ,0));
  assert( Abc_NtkIsDfsOrdered(pNtk) );
  // cerr << "and for pNtk" << endl;
  Abc_AigForEachAnd( pNtk, pNode, i )
      pNode->pCopy = Abc_AigAnd( (Abc_Aig_t *)pNtkNew->pManFunc, Abc_ObjChild0Copy(pNode), Abc_ObjChild1Copy(pNode) );
  // cerr << "latch for pNtkQ" << endl;
  Abc_NtkForEachLatch( pNtkQ, pNode, i )
    Abc_ObjAddFanin( Abc_ObjFanin0(pNode)->pCopy, Abc_ObjChild0Copy(Abc_ObjFanin0(pNode)) );

  // add the miter
  // cerr << "po for pNtkQ" << endl;
  Abc_ObjAddFanin( Abc_NtkPo(pNtkNew,0), Abc_ObjChild0Copy(Abc_NtkPo(pNtk,0)) );
}
