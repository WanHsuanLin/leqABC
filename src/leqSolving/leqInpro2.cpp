#include "leqSolving/leqSolving.hpp"

void Leq_InputProgressive2(Leq_Ba_t* pBa, unordered_set<string> & sU, int qSolver){
  unsigned uBegin = Leq_BaNumInput(pBa) - sU.size() - 1;
  abctime clk = Abc_Clock();
  // change the order of pi, put inactive pi before active pi(v,u), modify both vector of Ntk and id of obj
  Leq_BaModifyCirPiOrder(pBa);
  Leq_BaModifyCirPiOrderU(pBa, sU);
  bool verbose = 0;
  // generate charateris function
  Abc_Ntk_t * pNtkCheck;
  Abc_Ntk_t * pNtkCheckOld = NULL; //, * pNewTran = Leq_BaTransCir(pBa);
  
  int i = 0;
  while(1){
    cerr << "input progressive: iter " << i << endl;
    // cerr << i << " iteration!" << endl;
    // pNtkCheck = Leq_makeOnePo(Abc_NtkDup(Leq_BaTransCir(pBa)));
    pNtkCheck = Abc_NtkDup(Leq_BaTransCir(pBa));
    if(verbose) Abc_NtkPrintStats( pNtkCheck, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
    // getchar();
    if(qSolver == 1){
      cerr << "Not avaliable!!!" << endl;
    }
      // Leq_EliminateVarCadet(pNtkCheck, uBegin, pBa);
    if(qSolver == 2){
      cerr << "Not avaliable!!!" << endl;
      // Leq_EliminateVarManthan(pNtkCheck, uBegin, pBa);
    }
    else
      Leq_EliminateVar(pNtkCheck, uBegin, pBa);
    // Leq_EliminateVarManthan(pNtkCheck, uBegin, pBa);
    // Leq_EliminateVar1(pNtkCheck, uBegin, pBa);
    // Io_Write( pNtkCheck, "NtkXCadet.blif", IO_FILE_BLIF );
    // return;
    Leq_BaSetTransCir(pBa, Leq_AddCheckCir(Leq_BaTransCir(pBa), pNtkCheck));
    // Leq_BaResetByTranCir(pBa);
    if(Leq_CheckEquivalent( pNtkCheckOld, pNtkCheck )) break;
    Leq_BaModifyCirPiOrder(pBa);
    Leq_BaModifyCirPiOrderU(pBa, sU);
    // Leq_PrintNtkPi(Leq_BaTransCir(pBa));
    Abc_NtkDelete(pNtkCheckOld);
    pNtkCheckOld = pNtkCheck;
    
    // cerr << "pNtkCheck : " << pNtkCheck << endl;
    ++i;
    // getchar();
  }
  Abc_NtkDelete(pNtkCheckOld);
  Abc_NtkDelete(pNtkCheck);
  return;
}

int Leq_CheckEquivalent(Abc_Ntk_t * pNtk1, Abc_Ntk_t * & pNtk2){
  // pNtk2 = Abc_NtkMakeOnePo(pNtk2, 0, 1);
  // Abc_NtkShow( pNtk2, 0, 0, 1 );
  if(pNtk1 == nullptr){  
    Abc_Obj_t * pChild = Abc_ObjChild0(Abc_NtkPo(pNtk2, 0));
    if ( Abc_AigNodeIsConst(pChild) ){
      assert( Abc_ObjRegular(pChild) == Abc_AigConst1(pNtk2) );
      if ( !Abc_ObjIsComplement(pChild) )
      {
          return 1;
      }
    }
    return 0;
  }
  // cerr << "1 : " << Abc_NtkPoNum(pNtk1) << " 2 : " << Abc_NtkPoNum(pNtk2) << endl;
  assert(Abc_NtkPoNum(pNtk1) == 1 && Abc_NtkPoNum(pNtk2) == 1);
  bool verbose = 0;
  return Abc_NtkDarCec( pNtk1, pNtk2, 10000, 0, verbose );
}


/**Function*************************************************************

  Synopsis    [Derives the miter of two networks.]

  Description [Preprocesses the networks to make sure that they are strashed.]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
Abc_Ntk_t * Leq_AddCheckCir( Abc_Ntk_t * pNtk, Abc_Ntk_t * pNtkCheck ){
  Abc_Ntk_t * pTemp = NULL, * pTemp1 = NULL;
  assert( Abc_NtkHasOnlyLatchBoxes(pNtk) );
  assert( Abc_NtkHasOnlyLatchBoxes(pNtkCheck) );
  // check that the networks have the same PIs/POs/latches
  Abc_NtkOrderObjsByName( pNtk, 0 );
  // Abc_NtkOrderObjsByName( pNtkCheck, 0 );
  // int i;
  // Abc_Obj_t * pObj;
  // cerr << "print latch order of pntk" << endl;
  // Abc_NtkForEachLatch(pNtk,pObj,i){
  //   cerr << Abc_ObjName(pObj) << endl;
  // }
  // cerr << endl;
  // cerr << "print latch order of pntkcheck" << endl;
  // Abc_NtkForEachLatch(pNtkCheck,pObj,i){
  //   cerr << Abc_ObjName(pObj) << " ";
  // }
  // cerr << endl;
  // Abc_NtkMakeComb(pNtkCheck, 0);
  // cerr << "print pi order of pntkcheck" << endl;
  // Abc_NtkForEachPi(pNtkCheck, pObj,i){
  //   cerr << Abc_ObjName(pObj) << endl;
  // }
  // cerr << endl;
  if ( pNtk && pNtkCheck ){
    // Fraig_Params_t Params, * pParams = &Params;
    // memset( pParams, 0, sizeof(Fraig_Params_t) );
    // pParams->nPatsRand  = 2048; // the number of words of random simulation info
    // pParams->nPatsDyna  = 2048; // the number of words of dynamic simulation info
    // pParams->nBTLimit   =  100; // the max number of backtracks to perform
    // pParams->fFuncRed   =    1; // performs only one level hashing
    // pParams->fFeedBack  =    1; // enables solver feedback
    // pParams->fDist1Pats =    1; // enables distance-1 patterns
    // pParams->fDoSparse  =    1; // performs equiv tests for sparse functions
    // pParams->fChoicing  =    0; // enables recording structural choices
    // pParams->fTryProve  =    0; // tries to solve the final miter
    // pParams->fVerbose   =    0; // the verbosiness flag
    // pParams->fVerboseP  =    0; // the verbosiness flag
    pTemp = Leq_AddCheckCirInt( pNtk, pNtkCheck);
    // Abc_NtkDelete(pNtk);
    pTemp1 = Abc_NtkDC2( pTemp, 0, 0, 1, 0, 0 );
    Abc_NtkDelete(pTemp);
    
    pTemp = Abc_NtkStrash( pTemp1, 0, 1, 0 ); 
    Abc_NtkDelete(pTemp1);
    // pTemp1 = Abc_NtkFraig( pTemp, &Params, 0, 0 );
    // Abc_NtkDelete(pTemp);
    // pTemp = Abc_NtkDarLatchSweep( pTemp1, 1, 1, 1, 0, 1, 512, 1, 1 );
    // Abc_NtkDelete(pTemp1);
    // Abc_NtkCleanupSeq( pTemp1, 1, 0, 1 );

  }
  return pTemp;
}

/**Function*************************************************************

  Synopsis    [Derives the miter of two sequential networks.]

  Description [Assumes that the networks are strashed. pNtk2 -> checkCircuit]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
Abc_Ntk_t * Leq_AddCheckCirInt( Abc_Ntk_t * pNtk1, Abc_Ntk_t * pNtk2){
  char Buffer[1000];
  Abc_Ntk_t * pNtkAnd;

  assert( Abc_NtkIsStrash(pNtk1) );
  assert( Abc_NtkIsStrash(pNtk2) );

  // start the new network
  pNtkAnd = Abc_NtkAlloc( ABC_NTK_STRASH, ABC_FUNC_AIG, 1 );
  sprintf( Buffer, "%s_%s_miter", pNtk1->pName, pNtk2->pName );
  pNtkAnd->pName = Extra_UtilStrsav(Buffer);

  // perform strashing
  Abc_AigConst1(pNtk1)->pCopy = Abc_AigConst1(pNtkAnd);
  Abc_AigConst1(pNtk2)->pCopy = Abc_AigConst1(pNtkAnd);
  Leq_AddCheckCirPrepare( pNtk1, pNtkAnd );
  Abc_NtkMiterAddOne( pNtk1, pNtkAnd );

  // Abc_NtkMiterAddOne( pNtk2, pNtkAnd );

  Leq_AddCheckCirFinalize( pNtk1, pNtk2, pNtkAnd);
  Abc_AigCleanup((Abc_Aig_t *)pNtkAnd->pManFunc);

  // make sure that everything is okay
  if ( !Abc_NtkCheck( pNtkAnd ) ){
    printf( "Abc_NtkMiter: The network check has failed.\n" );
    Abc_NtkDelete( pNtkAnd );
    return NULL;
  }
  return pNtkAnd;
}

void Leq_AddCheckCirPrepare( Abc_Ntk_t * pNtk1, Abc_Ntk_t * pNtkMiter){
  Abc_Obj_t * pObj, * pObjNew;
  int i;
  // clean the copy field in all objects
  // create new PIs and remember them in the old PIs
  Abc_NtkForEachPi( pNtk1, pObj, i ){
    pObjNew = Abc_NtkCreatePi( pNtkMiter );
    // remember this PI in the old PIs
    pObj->pCopy = pObjNew;
    Abc_ObjAssignName( pObjNew, Abc_ObjName(pObj), NULL );
    // pObj = Abc_NtkPi(pNtk2, i);  
    // pObj->pCopy = pObjNew;
  }
  pObjNew = Abc_NtkCreatePo( pNtkMiter );
  Abc_ObjAssignName( pObjNew, "product_f0", NULL );
  // create the latches
  Abc_NtkForEachLatch( pNtk1, pObj, i ){
    pObjNew = Abc_NtkDupBox( pNtkMiter, pObj, 0 );

    // add names
    Abc_ObjAssignName( pObjNew, Abc_ObjName(pObj), NULL );
    Abc_ObjAssignName( Abc_ObjFanin0(pObjNew),  Abc_ObjName(Abc_ObjFanin0(pObj)), NULL );
    Abc_ObjAssignName( Abc_ObjFanout0(pObjNew), Abc_ObjName(Abc_ObjFanout0(pObj)), NULL );
  }
}

/**Function*************************************************************

  Synopsis    [Finalizes the miter by adding the output part.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Leq_AddCheckCirFinalize( Abc_Ntk_t * pNtk1, Abc_Ntk_t * pNtk2, Abc_Ntk_t * pNtkAnd){
  Abc_Obj_t * pObjAnd, * pNode, * pObj;
  int i, lBegin = Abc_NtkPiNum(pNtk2)-Abc_NtkLatchNum(pNtk1);
  // cerr << "In Leq_AddCheckCirFinalize, latch num = " << lBegin << endl;
  // connect new latches
  Abc_NtkForEachLatch( pNtk1, pNode, i ){
    Abc_ObjAddFanin( Abc_ObjFanin0(pNode)->pCopy, Abc_ObjChild0Copy(Abc_ObjFanin0(pNode)) );
    pObj = Abc_NtkPi(pNtk2, lBegin+i);  
    pObj->pCopy = Abc_ObjChild0Copy(Abc_ObjFanin0(pNode));
  }
  // assert( Abc_NtkIsDfsOrdered(pNtk2) );
  //   Abc_AigForEachAnd( pNtk2, pNode, i ){
  //       cerr << pNode << " " << Abc_ObjChild0(pNode) << " " << Abc_ObjChild1(pNode) << endl;
  //       pNode->pCopy = Abc_AigAnd( (Abc_Aig_t *)pNtkAnd->pManFunc, Abc_ObjChild0Copy(pNode), Abc_ObjChild1Copy(pNode) );
    // }
  Abc_NtkMiterAddOne( pNtk2, pNtkAnd );
  
  // add the miter
  cerr << "the last latch name is " << Abc_ObjName(Abc_ObjFanin0(pNode)->pCopy) << endl;
  pObjAnd = Abc_AigAnd( (Abc_Aig_t *)pNtkAnd->pManFunc, Abc_ObjChild0Copy(Abc_NtkPo(pNtk1,0)), Abc_ObjChild0Copy(Abc_NtkPo(pNtk2,0)) );
  pObjAnd = Abc_AigAnd( (Abc_Aig_t *)pNtkAnd->pManFunc, pObjAnd, Abc_ObjFanout0(pNode)->pCopy) ;
  Abc_ObjAddFanin( Abc_NtkPo(pNtkAnd,0), pObjAnd );
  Abc_ObjRemoveFanins( Abc_ObjFanin0(pNode)->pCopy );
  Abc_ObjAddFanin( Abc_ObjFanin0(pNode)->pCopy, pObjAnd );
}
