#include "leqSolving/leqSolving.hpp"

Leq_Ba_t * Leq_Solve(Leq_Ba_t* pBaS, Leq_Ba_t* pBaF, unordered_set<string> &sU, unordered_set<string> &sAllInput, bool bDeterminize, bool bIsDeter, int qSolver){
  
  // Abc_NtkShow( Leq_BaTransCir(pBaF), 0, 0, 1 );
  // getchar();
  // input progressive for S and F
  // TODO

  // ! try
  // Leq_Support(pBaF, sAllInput);
  // Leq_Complement(pBaF);
  // Leq_Aba2Ba(pBaF);
  // cerr << "finish determinization" << endl;
  // Abc_NtkShow( Leq_BaTransCir(pBaF), 0, 0, 1 );
  // Leq_BaPrintInitialState(pBaF);
  // return pBaF;
  // !

  // joint S and F
  // Leq_BaPrintInitialState(pBaF);
  // Leq_BaPrintInitialState(pBaS);
  abctime clk = Abc_Clock();
  Leq_Complement(pBaS, bIsDeter);
  // Abc_NtkShow( Leq_BaTransCir(pBaS), 0, 0, 1 );
  // getchar();
  Leq_Ba_t * pBaX = new Leq_Ba_t();
  cerr << "---------start ba product---------" << endl;
  Abc_Ntk_t * pTmpNtk = Leq_ProductBaTransCir(Leq_BaTransCir(pBaS), Leq_BaTransCir(pBaF));
  // pTmpNtk = Abc_NtkDarLatchSweep( pTmpNtk, 1, 1, 1, 0, 1, 512, 1, 1 );
  cerr << "---------End ba product---------" << endl;
  cerr << "---------Start baX construction---------" << endl;
  // pTmpNtk = Abc_NtkDarLcorrNew( pTmpNtk, 1000, 1000, 1 );  
  // Abc_NtkShow( pTmpNtk, 0, 0, 1 );
  // getchar();
  Leq_BaInit(pBaX, pTmpNtk, 0, 0);
  cerr << "latch number = " << Abc_NtkLatchNum(Leq_BaTransCir(pBaX)) << endl;
  // Leq_BaPrintInitialState(pBaX);
  // Abc_NtkShow( Leq_BaTransCir(pBaX), 0, 0, 1 );
  // getchar();
  cerr << "---------End baX construction---------" << endl;
  // Abc_NtkShow( Leq_BaTransCir(pBaX), 0, 0, 1 );
  // getchar();
  
  cerr << "---------Start modify support---------" << endl;
  // Abc_NtkShow( Leq_BaTransCir(pBaX), 0, 0, 1 );
  Leq_Support(pBaX, sAllInput);
  // Abc_NtkShow( Leq_BaTransCir(pBaX), 0, 0, 1 );
  // getchar();
  cerr << "---------End modify support---------" << endl;
  cerr << "---------Start baX complement---------" << endl;
  Leq_Complement(pBaX, bIsDeter);
  // Abc_NtkShow( Leq_BaTransCir(pBaX), 0, 0, 1 );
  // getchar();
  cerr << "---------End baX complement---------" << endl;
  cerr << "---------Start prefix close---------" << endl;
  Leq_PrefixClose(pBaX);
  cerr << "latch number = " << Abc_NtkLatchNum(Leq_BaTransCir(pBaX)) << endl;
  // Abc_NtkShow( Leq_BaTransCir(pBaX), 0, 0, 1 );
  // getchar();
  // Leq_BaPrintInitialState(pBaX);
  cerr << "---------End prefix close---------" << endl;
  
  // Abc_Obj_t * pObj;
  // int i;
  // Abc_NtkForEachObj(Leq_BaTransCir(pBaX), pObj, i){
  //   cerr << Abc_ObjName(pObj) << endl;
  // }
  // Abc_NtkShow( Leq_BaTransCir(pBaX), 0, 0, 1 );
  cerr << "Runtime befor input progressive : " << (double)(Abc_Clock() - clk )/(double)CLOCKS_PER_SEC << endl;
  clk = Abc_Clock();
  cerr << "---------Start input progressive---------" << endl;
  Leq_InputProgressive2(pBaX, sU, qSolver);
  cerr << "Runtime after input progressive : " << (double)(Abc_Clock() - clk )/(double)CLOCKS_PER_SEC << endl;
  // Abc_NtkShow( Leq_BaTransCir(pBaX), 0, 0, 1 );
  // getchar();

  // Abc_Ntk_t * pNtkDup = Abc_NtkDup(Leq_BaTransCir(pBaX));
  // Leq_Ba_t * pBaXDup = new Leq_Ba_t();;
  // Leq_BaInit(pBaXDup, pNtkDup, 0, 0);
  // Leq_InputProgressive(pBaX, sU, 0);
  // Leq_InputProgressive2(pBaXDup, sU);
  // Fra_Sec_t SecPar, * pSecPar = &SecPar;

  // Fra_SecSetDefaultParams( pSecPar );
  // pSecPar->TimeLimit = 0;
  // pSecPar->fVerbose = 1;  // enables verbose reporting of statistics
  // pSecPar->fVeryVerbose = 1;  // enables very verbose reporting  
  // pSecPar->fReportSolution = 1;
  // Abc_NtkDarSec( Leq_BaTransCir(pBaX), Leq_BaTransCir(pBaXDup), pSecPar );

  // cerr << "first time ! " << endl;
  // // Abc_NtkShow( Leq_BaTransCir(pBaX), 0, 0, 1 );
  // // getchar();
  // cerr << "second time ! " << endl;
  // Leq_InputProgressive(pBaX, sU, 0);
  // Abc_NtkShow( Leq_BaTransCir(pBaX), 0, 0, 1 );
  // getchar();
  // Leq_BaPrintInitialState(pBaX);

  cerr << "---------End input progressive---------" << endl;
  cerr << "latch number = " << Abc_NtkLatchNum(Leq_BaTransCir(pBaX)) << endl;
  // getchar();

  // create file for X (optional)

  return pBaX;
}

/**Function*************************************************************

  Synopsis    [Change the support of BA]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Leq_Support(Leq_Ba_t* pBa, unordered_set<string> sSupport){
  unordered_map<string, Abc_Obj_t *>::iterator it = pBa->_mActInput.begin();
  unordered_map<string, Abc_Obj_t *>::iterator itIn = pBa->_mInactInput.begin();
  unordered_set<string>::iterator itS;
  // cerr << "Original support : " << endl;
  // Leq_BaPrintSupport(pBa);
  // delete the active support not in vSupport, delete the support in vSupport that already in active support
  while(it != pBa->_mActInput.end()){
    itS = sSupport.find(string(it->first)); 
    if(itS==sSupport.end()){
      pBa->_mInactInput[it->first] = it->second;
      it = pBa->_mActInput.erase(it);
    }
    else{
      sSupport.erase(itS);
      ++it;
    }
  }
  // add no use input, which means that this support is a dc for circuit
  for (auto & str : pBa->_sDcInput){
    itS = sSupport.find(string(str)); 
    if(itS!=sSupport.end()){
      sSupport.erase(itS);
    }
  }
  char * tmp;
  // add the support not in active support to inactive support
  // add the support to active support
  for(itS = sSupport.begin(); itS != sSupport.end(); ++itS){
    tmp = new char [itS->size()+1]; 
    strcpy(tmp, itS->c_str());
    itIn = pBa->_mInactInput.find(string(tmp));
    if(itIn==pBa->_mInactInput.end()){
      pBa->_sDcInput.emplace(tmp);
    }
    else{
      pBa->_mActInput[tmp] = pBa->_mInactInput[tmp];
      pBa->_mInactInput.erase(tmp);
    }
  }
  // change the order of pi, put inactive pi before active pi, modify both vector of Ntk and id of obj
  Leq_BaModifyCirPiOrder(pBa);

  // cerr << "After support operation : " << endl;
  // Leq_BaPrintSupport(pBa);
}


/**Function*************************************************************

  Synopsis    [Return product(A,B)]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
Abc_Ntk_t * Leq_ProductBaTransCir(Abc_Ntk_t * pNtkS, Abc_Ntk_t * pNtkF, bool bUseInDirectlyVerify){
  Abc_Ntk_t * pNtkMiter;
  pNtkMiter = Abc_NtkAlloc( ABC_NTK_STRASH, ABC_FUNC_AIG, 1 );
  pNtkMiter->pName = "baX";

  Abc_Obj_t * pObj, * pObjNew;
  int i, nPi;
  Abc_AigConst1(pNtkS)->pCopy = Abc_AigConst1(pNtkMiter);
  Abc_AigConst1(pNtkF)->pCopy = Abc_AigConst1(pNtkMiter);
  
  // create new PIs and remember them in the old PIs
  // nPi = Abc_NtkPiNum(pNtkS);
  Abc_NtkForEachPi( pNtkF, pObj, i ){
    pObjNew = Abc_NtkCreatePi( pNtkMiter );
    // remember this PI in the old PIs
    pObj->pCopy = pObjNew;
    // Abc_NtkFindNode(pNtkS, Abc_ObjName(pObj));
    // cerr << "Find " << Abc_ObjName(pObj) << " node" << endl; // !
    // add name
    Abc_ObjAssignName( pObjNew, Abc_ObjName(pObj), NULL );
    Abc_Obj_t * pObjTmp = pObj;
    pObj = Abc_NtkFindCi(pNtkS, Abc_ObjName(pObj));;  
    // cerr << "Abc_ObjIsPi: " << Abc_ObjIsPi(pObj) << endl;
    if(bUseInDirectlyVerify && pObj == NULL){
      char * prefix = "[0]";
      pObj = Abc_NtkFindCi(pNtkS, strcat(Abc_ObjName(pObjTmp),prefix));  
    }
    if(pObj != NULL && Abc_ObjIsPi(pObj)) // !
    // if(pObj != NULL )
      pObj->pCopy = pObjNew;
    else{
      cerr << "No " << Abc_ObjName(pObjTmp) << " node" << endl; // !
    } 
  }
  // Abc_NtkForEachPi( pNtkS, pObj, i ){
  //   cerr << "Pi names in S: "  << Abc_ObjName(pObj) << endl; // !
  // }
  cerr << "finish PI" << endl;
  // create POs
  pObjNew = Abc_NtkCreatePo( pNtkMiter );
  Abc_ObjAssignName( pObjNew, "product_f0", NULL );

  // create the latches
  Abc_NtkForEachLatch( pNtkS, pObj, i ){
    pObjNew = Abc_NtkDupBox( pNtkMiter, pObj, 0 );
    // add names
    Abc_ObjAssignName( pObjNew, Abc_ObjName(pObj), "_s" );
    Abc_ObjAssignName( Abc_ObjFanin0(pObjNew),  Abc_ObjName(Abc_ObjFanin0(pObj)), "_s" );
    Abc_ObjAssignName( Abc_ObjFanout0(pObjNew), Abc_ObjName(Abc_ObjFanout0(pObj)), "_s" );
  }
  Abc_NtkForEachLatch( pNtkF, pObj, i ){
    pObjNew = Abc_NtkDupBox( pNtkMiter, pObj, 0 );
    // add name
    Abc_ObjAssignName( pObjNew, Abc_ObjName(pObj), "_f" );
    Abc_ObjAssignName( Abc_ObjFanin0(pObjNew),  Abc_ObjName(Abc_ObjFanin0(pObj)), "_f" );
    Abc_ObjAssignName( Abc_ObjFanout0(pObjNew), Abc_ObjName(Abc_ObjFanout0(pObj)), "_f" );
  }

  Abc_Obj_t * pNode;
  cerr << "construct nodes for pNtkS" << endl;
  assert( Abc_NtkIsDfsOrdered(pNtkS) );
  Abc_AigForEachAnd( pNtkS, pNode, i ){
    // cerr << " copy node: " << Abc_ObjName(pNode) << ", with child: " << Abc_ObjName(Abc_ObjChild0(pNode)) << " and " << Abc_ObjName(Abc_ObjChild1(pNode)) << endl;
    pNode->pCopy = Abc_AigAnd( (Abc_Aig_t *)pNtkMiter->pManFunc, Abc_ObjChild0Copy(pNode), Abc_ObjChild1Copy(pNode) );
  }
  cerr << "construct nodes for pNtkF" << endl;
  assert( Abc_NtkIsDfsOrdered(pNtkF) );
  Abc_AigForEachAnd( pNtkF, pNode, i )
    pNode->pCopy = Abc_AigAnd( (Abc_Aig_t *)pNtkMiter->pManFunc, Abc_ObjChild0Copy(pNode), Abc_ObjChild1Copy(pNode) );

  cerr << "start connect PO" << endl;


  Vec_Ptr_t * vPairs;

  // collect the PO pairs from both networks
  vPairs = Vec_PtrAlloc( 100 );
  // collect the PO nodes for the miter
  Abc_NtkForEachPo( pNtkS, pNode, i ) {
    Vec_PtrPush( vPairs, Abc_ObjChild0Copy(pNode) );
    pNode = Abc_NtkPo( pNtkF, i );
    Vec_PtrPush( vPairs, Abc_ObjChild0Copy(pNode) );
  }
  // connect new latches
  Abc_NtkForEachLatch( pNtkS, pNode, i )
    Abc_ObjAddFanin( Abc_ObjFanin0(pNode)->pCopy, Abc_ObjChild0Copy(Abc_ObjFanin0(pNode)) );
  Abc_NtkForEachLatch( pNtkF, pNode, i )
    Abc_ObjAddFanin( Abc_ObjFanin0(pNode)->pCopy, Abc_ObjChild0Copy(Abc_ObjFanin0(pNode)) );

  // add the miter
  
  Abc_ObjAddFanin( Abc_NtkPo(pNtkMiter,0), Abc_AigAnd((Abc_Aig_t *)pNtkMiter->pManFunc, (Abc_Obj_t *)vPairs->pArray[0], (Abc_Obj_t *)vPairs->pArray[1]) );
  Vec_PtrFree( vPairs );
  
  return pNtkMiter;
}



/**Function*************************************************************

  Synopsis    [Language complement for BA]]

  Description [Add NOT gate at output f0]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Leq_Complement(Leq_Ba_t * pBa, bool bIsDeter){
  Abc_Ntk_t * pNtk = Leq_BaTransCir(pBa);

  // add not cs to PO
  Abc_Obj_t * pF0 = Abc_NtkPo(pNtk, 0);
  Abc_Obj_t * pNewPoIn = Abc_ObjNot(Abc_ObjChild0(pF0));
  
  Abc_ObjRemoveFanins(pF0);
  Abc_ObjAddFanin( pF0, pNewPoIn);
  if(Leq_BaNumInactInput(pBa) && !bIsDeter){
    pBa->_bNondeterType = !pBa->_bNondeterType;
  }
}


/**Function*************************************************************

  Synopsis    [Language prefix close for BA]]

  Description [Add latch s for prefix close]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Leq_PrefixClose(Leq_Ba_t * pBa){
  cerr << "creat new latches" << endl;
  
  Abc_Ntk_t * pNtk = Leq_BaTransCir(pBa);
  // create s for input progressive
  Abc_Obj_t * pF0 = Abc_NtkPo(pNtk, 0);
  Abc_Obj_t * pNewPoIn;

  Abc_Obj_t * pLatchOut, * pLatch, * pLatchIn;
  pLatchOut = Abc_NtkCreateBo(pNtk);
  pLatch    = Abc_NtkCreateLatch(pNtk);
  pLatchIn  = Abc_NtkCreateBi(pNtk);
  
  char tmpChar[20 + sizeof(char)];
  sprintf(tmpChar, "_%d", Abc_ObjId(pLatch));

  Abc_ObjAssignName( pLatch, "Ba_ipS" , tmpChar );
  Abc_ObjAssignName( pLatchOut, Abc_ObjName(pLatch), "_lo" );
  Abc_ObjAssignName( pLatchIn,  Abc_ObjName(pLatch), "_li" );
  Abc_ObjAddFanin( pLatchOut, pLatch );
  Abc_ObjAddFanin( pLatch, pLatchIn );
  Abc_LatchSetInit1( pLatch );

  
  cerr << "connect new latch to PO" << endl;
  // add new s to PO
  // cerr << Abc_ObjIsPi(pF0) << " " << Abc_ObjIsPo(pLatchOut) << endl;
  pNewPoIn = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, Abc_ObjChild0(pF0), pLatchOut);
  

  Abc_ObjRemoveFanins(pF0);
  Abc_ObjAddFanin( pF0, pNewPoIn);
  Abc_ObjAddFanin( pLatchIn, pNewPoIn );
  
  Abc_AigCleanup((Abc_Aig_t *)pBa->_pNtk->pManFunc);

  // make sure that everything is okay
  if ( !Abc_NtkCheck( pBa->_pNtk ) ){
    printf( "pNtk: The network check has failed.\n" );
    Abc_NtkDelete( pBa->_pNtk );
    return;
  }

  Leq_BaIncNumState(pBa);
  pBa->_vIniVal.emplace_back(1);
  return;
}


Abc_Ntk_t * Leq_XorBaTransCir(Abc_Ntk_t * pNtkS, Abc_Ntk_t * pNtkF, bool bUseInDirectlyVerify){
  Abc_Ntk_t * pNtkMiter;
  pNtkMiter = Abc_NtkAlloc( ABC_NTK_STRASH, ABC_FUNC_AIG, 1 );
  pNtkMiter->pName = "baX";

  Abc_Obj_t * pObj, * pObjNew;
  int i, nPi;
  Abc_AigConst1(pNtkS)->pCopy = Abc_AigConst1(pNtkMiter);
  Abc_AigConst1(pNtkF)->pCopy = Abc_AigConst1(pNtkMiter);
  
  // create new PIs and remember them in the old PIs
  // nPi = Abc_NtkPiNum(pNtkS);
  Abc_NtkForEachPi( pNtkF, pObj, i ){
    pObjNew = Abc_NtkCreatePi( pNtkMiter );
    // remember this PI in the old PIs
    pObj->pCopy = pObjNew;
    // Abc_NtkFindNode(pNtkS, Abc_ObjName(pObj));
    // cerr << "Find " << Abc_ObjName(pObj) << " node" << endl; // !
    // add name
    Abc_ObjAssignName( pObjNew, Abc_ObjName(pObj), NULL );
    Abc_Obj_t * pObjTmp = pObj;
    pObj = Abc_NtkFindCi(pNtkS, Abc_ObjName(pObj));;  
    // cerr << "Abc_ObjIsPi: " << Abc_ObjIsPi(pObj) << endl;
    if(bUseInDirectlyVerify && pObj == NULL){
      char * prefix = "[0]";
      pObj = Abc_NtkFindCi(pNtkS, strcat(Abc_ObjName(pObjTmp),prefix));  
    }
    if(pObj != NULL && Abc_ObjIsPi(pObj)) // !
    // if(pObj != NULL )
      pObj->pCopy = pObjNew;
    else{
      cerr << "No " << Abc_ObjName(pObjTmp) << " node" << endl; // !
    } 
  }
  // Abc_NtkForEachPi( pNtkS, pObj, i ){
  //   cerr << "Pi names in S: "  << Abc_ObjName(pObj) << endl; // !
  // }
  cerr << "finish PI" << endl;
  // create POs
  pObjNew = Abc_NtkCreatePo( pNtkMiter );
  Abc_ObjAssignName( pObjNew, "product_f0", NULL );

  // create the latches
  Abc_NtkForEachLatch( pNtkS, pObj, i ){
    pObjNew = Abc_NtkDupBox( pNtkMiter, pObj, 0 );
    // add names
    Abc_ObjAssignName( pObjNew, Abc_ObjName(pObj), "_s" );
    Abc_ObjAssignName( Abc_ObjFanin0(pObjNew),  Abc_ObjName(Abc_ObjFanin0(pObj)), "_s" );
    Abc_ObjAssignName( Abc_ObjFanout0(pObjNew), Abc_ObjName(Abc_ObjFanout0(pObj)), "_s" );
  }
  Abc_NtkForEachLatch( pNtkF, pObj, i ){
    pObjNew = Abc_NtkDupBox( pNtkMiter, pObj, 0 );
    // add name
    Abc_ObjAssignName( pObjNew, Abc_ObjName(pObj), "_f" );
    Abc_ObjAssignName( Abc_ObjFanin0(pObjNew),  Abc_ObjName(Abc_ObjFanin0(pObj)), "_f" );
    Abc_ObjAssignName( Abc_ObjFanout0(pObjNew), Abc_ObjName(Abc_ObjFanout0(pObj)), "_f" );
  }

  Abc_Obj_t * pNode;
  cerr << "construct nodes for pNtkS" << endl;
  assert( Abc_NtkIsDfsOrdered(pNtkS) );
  Abc_AigForEachAnd( pNtkS, pNode, i ){
    // cerr << " copy node: " << Abc_ObjName(pNode) << ", with child: " << Abc_ObjName(Abc_ObjChild0(pNode)) << " and " << Abc_ObjName(Abc_ObjChild1(pNode)) << endl;
    pNode->pCopy = Abc_AigAnd( (Abc_Aig_t *)pNtkMiter->pManFunc, Abc_ObjChild0Copy(pNode), Abc_ObjChild1Copy(pNode) );
  }
  cerr << "construct nodes for pNtkF" << endl;
  assert( Abc_NtkIsDfsOrdered(pNtkF) );
  Abc_AigForEachAnd( pNtkF, pNode, i )
    pNode->pCopy = Abc_AigAnd( (Abc_Aig_t *)pNtkMiter->pManFunc, Abc_ObjChild0Copy(pNode), Abc_ObjChild1Copy(pNode) );

  cerr << "start connect PO" << endl;


  Vec_Ptr_t * vPairs;

  // collect the PO pairs from both networks
  vPairs = Vec_PtrAlloc( 100 );
  // collect the PO nodes for the miter
  Abc_NtkForEachPo( pNtkS, pNode, i ) {
    Vec_PtrPush( vPairs, Abc_ObjChild0Copy(pNode) );
    pNode = Abc_NtkPo( pNtkF, i );
    Vec_PtrPush( vPairs, Abc_ObjChild0Copy(pNode) );
  }
  // connect new latches
  Abc_NtkForEachLatch( pNtkS, pNode, i )
    Abc_ObjAddFanin( Abc_ObjFanin0(pNode)->pCopy, Abc_ObjChild0Copy(Abc_ObjFanin0(pNode)) );
  Abc_NtkForEachLatch( pNtkF, pNode, i )
    Abc_ObjAddFanin( Abc_ObjFanin0(pNode)->pCopy, Abc_ObjChild0Copy(Abc_ObjFanin0(pNode)) );

  // add the miter
  
  Abc_ObjAddFanin( Abc_NtkPo(pNtkMiter,0), Abc_AigXor((Abc_Aig_t *)pNtkMiter->pManFunc, (Abc_Obj_t *)vPairs->pArray[0], (Abc_Obj_t *)vPairs->pArray[1]) );
  Vec_PtrFree( vPairs );
  
  return pNtkMiter;
}