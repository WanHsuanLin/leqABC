#include "leqSolving/leqSolving.hpp"

int Leq_VerifyFXS(Leq_Ba_t* pBaS, Leq_Ba_t* pBaF, Leq_Ba_t* pBaX, unordered_set<string> &sProject, unordered_set<string> &sAll ){
  abctime clk = Abc_Clock();
  Leq_Ba_t * pBaXCopy = Leq_BaDuplicate(pBaX);
  printf( "[LEQ] - Check solution X is a valid implementation under spec S\n" );
  cerr << "---------Start modify support BaX---------" << endl;
  Leq_Support(pBaXCopy, sAll);
  cerr << "---------End modify support BaX---------" << endl;
  cerr << "---------Start modify pi for BaX---------" << endl;
  Abc_Obj_t * pObj;
  int fVerbose = 0;
  char cStr[20]; 
  for(auto & name : pBaXCopy->_sDcInput){
    pObj = Abc_NtkCreatePi(Leq_BaTransCir(pBaXCopy));
    pBaXCopy->_sDcObj.emplace(pObj);
    strcpy(cStr, name.c_str());
    Abc_ObjAssignName(pObj,cStr, NULL);
  }
  // for(auto & name : pBaX->_mInactInput){
  //   pObj = Abc_NtkCreatePi(Leq_BaTransCir(pBaF));
  //   pBaF->_sDcObj.emplace(pObj);
  //   strcpy(cStr, name.first.c_str());
  //   Abc_ObjAssignName(pObj,cStr, NULL);
  // }
  // for(auto & name : pBaX->_mInactInput){
  //   pObj = Abc_NtkCreatePi(Leq_BaTransCir(pBaS));
  //   pBaS->_sDcObj.emplace(pObj);
  //   strcpy(cStr, name.first.c_str());
  //   Abc_ObjAssignName(pObj,cStr, NULL);
  // }
  cerr << "---------End modify pi for BaX---------" << endl;;
  Leq_Ba_t * pBaProduct = new Leq_Ba_t();
  cerr << "---------start ba product---------" << endl;
  Abc_Ntk_t * pNtkTmp = Leq_ProductBaTransCir(Leq_BaTransCir(pBaXCopy), Leq_BaTransCir(pBaF));
  pNtkTmp = Abc_NtkDarLatchSweep( pNtkTmp, 1, 1, 1, 0, 1, 512, 1, fVerbose );
  // Abc_NtkDelete(pNtkTmp);
  pNtkTmp = Abc_NtkDC2( pNtkTmp, 0, 0, 1, 0, fVerbose );
  // Abc_NtkDelete(pNtkNew);
  // Abc_NtkShow( pTmpNtk, 0, 0, 1 );
  cerr << "---------End ba product---------" << endl;
  Leq_BaInit(pBaProduct, pNtkTmp, 0, 0);
  cerr << "---------Start modify support pBaProduct---------" << endl;
  Leq_Support(pBaProduct, sProject);
  cerr << "---------End modify support pBaProduct---------" << endl;
  for(auto name : sAll){
    if(sProject.find(name)==sProject.end()){
      pObj = Abc_NtkCreatePi(Leq_BaTransCir(pBaS));
      pBaS->_sDcObj.emplace(pObj);
      Abc_ObjAssignName(pObj,&name[0], NULL);
    }
  }
  // contain prod.aut supps.aut
  if(Leq_LanguageContain(Leq_BaTransCir(pBaProduct), Leq_BaTransCir(pBaS))){
    for(auto & pObj : pBaXCopy->_sDcObj){
      Abc_NtkDeleteObj(pObj);
    }
    for(auto & pObj : pBaS->_sDcObj){
      Abc_NtkDeleteObj(pObj);
    }
    cerr << "Runtime for Leq_VerifyFXS : " << (double)(Abc_Clock() - clk )/(double)CLOCKS_PER_SEC << endl;
    return 1;
  }
  for(auto & pObj : pBaXCopy->_sDcObj){
    Abc_NtkDeleteObj(pObj);
  }
  for(auto & pObj : pBaS->_sDcObj){
    Abc_NtkDeleteObj(pObj);
  }
  cerr << "Runtime for Leq_VerifyFXS : " << (double)(Abc_Clock() - clk )/(double)CLOCKS_PER_SEC << endl;
  return 0;
}

/**Function*************************************************************

  Synopsis    [Check language containment relation]

  Description [1: pBa1 < pBa2. else 0]
               
  SideEffects []

  SeeAlso     [Abc_CommandDSec]

***********************************************************************/
int Leq_LanguageContain(Abc_Ntk_t * pBaNtk1, Abc_Ntk_t * pBaNtk2){
  Fra_Sec_t SecPar, * pSecPar = &SecPar;

  Fra_SecSetDefaultParams( pSecPar );
  pSecPar->TimeLimit = 0;
  pSecPar->fVerbose = 1;  // enables verbose reporting of statistics
  pSecPar->fVeryVerbose = 1;  // enables very verbose reporting  
  pSecPar->fReportSolution = 1;
  pSecPar->nFramesMax = 10;

  // perform verification
  // get the miter of the two networks
  Abc_Ntk_t * pMiter = Leq_NtkMiter( pBaNtk1, pBaNtk2);
  if(Leq_NtkDarSec( pMiter, pSecPar )){
    return 1;
  }
  return 0;
}


/**Function*************************************************************

  Synopsis    [Check language containment relation]

  Description [1: pBa1 < pBa2. else 0]
               
  SideEffects []

  SeeAlso     [Abc_NtkDarSec]

***********************************************************************/
int Leq_NtkDarSec( Abc_Ntk_t * pMiter, Fra_Sec_t * pSecPar ){
    Aig_Man_t * pMan;
    int RetValue;
 
    // Leq_PrintNtkLatchValue(pMiter);
    // Abc_NtkShow( pMiter, 0, 0, 1 );
    // getchar();

    if ( pMiter == NULL )
    {
        Abc_Print( 1, "Miter computation has failed.\n" );
        return -1;
    }
    RetValue = Abc_NtkMiterIsConstant( pMiter );
    if ( RetValue == 0 )
    {
        extern void Abc_NtkVerifyReportErrorSeq( Abc_Ntk_t * pNtk1, Abc_Ntk_t * pNtk2, int * pModel, int nFrames );
        Abc_Print( 1, "Networks are NOT EQUIVALENT after structural hashing.\n" );
        // report the error
        pMiter->pModel = Abc_NtkVerifyGetCleanModel( pMiter, pSecPar->nFramesMax );
        ABC_FREE( pMiter->pModel );
        Abc_NtkDelete( pMiter );
        return 0;
    }
    if ( RetValue == 1 )
    {
        Abc_NtkDelete( pMiter );
        Abc_Print( 1, "Networks are equivalent after structural hashing.\n" );
        return 1;
    }

    pMan = Abc_NtkToDar( pMiter, 0, 1 );
    // Abc_NtkDelete( pMiter );
    if ( pMan == NULL )
    {
        Abc_Print( 1, "Converting miter into AIG has failed.\n" );
        return -1;
    }
    assert( pMan->nRegs > 0 );

    // perform verification
    RetValue = Fra_FraigSec( pMan, pSecPar, NULL );
    Aig_ManStop( pMan );
    return RetValue;
}

/**Function*************************************************************

  Synopsis    [Derives the miter of two networks.]

  Description [Preprocesses the networks to make sure that they are strashed.]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
Abc_Ntk_t * Leq_NtkMiter( Abc_Ntk_t * pNtk1, Abc_Ntk_t * pNtk2 ){
  Abc_Ntk_t * pTemp = NULL;
  int fRemove1, fRemove2;
  assert( Abc_NtkHasOnlyLatchBoxes(pNtk1) );
  assert( Abc_NtkHasOnlyLatchBoxes(pNtk2) );
  // check that the networks have the same PIs/POs/latches
  if ( !Abc_NtkCompareSignals( pNtk1, pNtk2, 1, 0 ) )
    return NULL;
  // make sure the circuits are strashed 
  fRemove1 = (!Abc_NtkIsStrash(pNtk1) || Abc_NtkGetChoiceNum(pNtk1)) && (pNtk1 = Abc_NtkStrash(pNtk1, 0, 0, 0));
  fRemove2 = (!Abc_NtkIsStrash(pNtk2) || Abc_NtkGetChoiceNum(pNtk2)) && (pNtk2 = Abc_NtkStrash(pNtk2, 0, 0, 0));
  if ( pNtk1 && pNtk2 )
    pTemp = Leq_NtkMiterInt( pNtk1, pNtk2);
  if ( fRemove1 )  Abc_NtkDelete( pNtk1 );
  if ( fRemove2 )  Abc_NtkDelete( pNtk2 );
  return pTemp;
}

/**Function*************************************************************

  Synopsis    [Derives the miter of two sequential networks.]

  Description [Assumes that the networks are strashed.]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
Abc_Ntk_t * Leq_NtkMiterInt( Abc_Ntk_t * pNtk1, Abc_Ntk_t * pNtk2){
  char Buffer[1000];
  Abc_Ntk_t * pNtkMiter;

  assert( Abc_NtkIsStrash(pNtk1) );
  assert( Abc_NtkIsStrash(pNtk2) );

  // start the new network
  pNtkMiter = Abc_NtkAlloc( ABC_NTK_STRASH, ABC_FUNC_AIG, 1 );
  sprintf( Buffer, "%s_%s_miter", pNtk1->pName, pNtk2->pName );
  pNtkMiter->pName = Extra_UtilStrsav(Buffer);

  // perform strashing
  Abc_NtkMiterPrepare( pNtk1, pNtk2, pNtkMiter, 0, 0, 0 );
  Abc_NtkMiterAddOne( pNtk1, pNtkMiter );
  Abc_NtkMiterAddOne( pNtk2, pNtkMiter );
  Leq_NtkMiterFinalize( pNtk1, pNtk2, pNtkMiter);
  Abc_AigCleanup((Abc_Aig_t *)pNtkMiter->pManFunc);

  // make sure that everything is okay
  if ( !Abc_NtkCheck( pNtkMiter ) ){
    printf( "Abc_NtkMiter: The network check has failed.\n" );
    Abc_NtkDelete( pNtkMiter );
    return NULL;
  }
  return pNtkMiter;
}



/**Function*************************************************************

  Synopsis    [Finalizes the miter by adding the output part.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Leq_NtkMiterFinalize( Abc_Ntk_t * pNtk1, Abc_Ntk_t * pNtk2, Abc_Ntk_t * pNtkMiter){
  Abc_Obj_t * pMiter, * pNode;
  int i;
  // collect the PO pairs from both networks
  assert(Abc_NtkPoNum(pNtk1)==1 && Abc_NtkPoNum(pNtk2)==1);
  // connect new latches
  Abc_NtkForEachLatch( pNtk1, pNode, i )
    Abc_ObjAddFanin( Abc_ObjFanin0(pNode)->pCopy, Abc_ObjChild0Copy(Abc_ObjFanin0(pNode)) );
  Abc_NtkForEachLatch( pNtk2, pNode, i )
    Abc_ObjAddFanin( Abc_ObjFanin0(pNode)->pCopy, Abc_ObjChild0Copy(Abc_ObjFanin0(pNode)) );
  // add the miter
  pMiter = Abc_AigAnd( (Abc_Aig_t *)pNtkMiter->pManFunc, Abc_ObjChild0Copy(Abc_NtkPo(pNtk1,0)), Abc_ObjNot(Abc_ObjChild0Copy(Abc_NtkPo(pNtk2,0))) );
  Abc_ObjAddFanin( Abc_NtkPo(pNtkMiter,0), pMiter );
}

int Leq_VerifyX(Abc_Ntk_t* pNtkXv, Leq_Ba_t* pBaX, int fUseCommand){
  abctime clk = Abc_Clock();
  Leq_Ba_t * pBaXCopy = Leq_BaDuplicate(pBaX);
  printf( "[LEQ] - Check origin X is contain by solution X\n" );
  Abc_Ntk_t * pMiter;
  Fra_Sec_t SecPar, * pSecPar = &SecPar;
  // int fUseCommand = 0; // 0 for dsec, 1 for pdr, 2 for dprove, 3 for int
  // consturct miter
  Leq_Complement(pBaXCopy, 0);
  cerr << "finish complement" << endl;
  pMiter = Leq_ProductBaTransCir(pNtkXv, Leq_BaTransCir(pBaXCopy), 1);
  cerr << "finish product" << endl;
  
  if(fUseCommand == 0){ // dsec 
    Fra_SecSetDefaultParams( pSecPar );
    pSecPar->TimeLimit = 0;
    pSecPar->fVerbose = 1;  // enables verbose reporting of statistics
    pSecPar->fVeryVerbose = 1;  // enables very verbose reporting  
    pSecPar->fReportSolution = 1;
    if(Leq_NtkDarSec( pMiter, pSecPar )){
      cerr << "Runtime for Leq_VerifyX : " << (double)(Abc_Clock() - clk )/(double)CLOCKS_PER_SEC << endl;
      return 1;  
    }
  }
  else if(fUseCommand == 1){ // pdr
    Pdr_Par_t Pars, * pPars = &Pars;
    Pdr_ManSetDefaultParams( pPars );
    pPars->fVerbose = 1;
    if(Abc_NtkDarPdr( pMiter, pPars )!=0){
      cerr << "Runtime for Leq_VerifyX : " << (double)(Abc_Clock() - clk )/(double)CLOCKS_PER_SEC << endl;
      return 1;  
    }
  }
  else if(fUseCommand == 2){ // dprove
    Fra_SecSetDefaultParams( pSecPar );
    pSecPar->TimeLimit = 0;
    pSecPar->fVerbose = 1;  // enables verbose reporting of statistics
    pSecPar->fVeryVerbose = 1;  // enables very verbose reporting  
    pSecPar->fReportSolution = 1;
    if(Abc_NtkDarProve( pMiter, pSecPar, 20, 2000 )!=0){
      cerr << "Runtime for Leq_VerifyX : " << (double)(Abc_Clock() - clk )/(double)CLOCKS_PER_SEC << endl;
      return 1;  
    }
  }
  else if(fUseCommand == 3){ // int
    Inter_ManParams_t Pars, * pPars = &Pars;
    Inter_ManSetDefaultParams( pPars );
    pPars->fVerbose = 1;
    if(Abc_NtkDarBmcInter( pMiter, pPars, NULL )!=0){
      cerr << "Runtime for Leq_VerifyX : " << (double)(Abc_Clock() - clk )/(double)CLOCKS_PER_SEC << endl;
      return 1;  
    }
  }
  cerr << "Runtime for Leq_VerifyX : " << (double)(Abc_Clock() - clk )/(double)CLOCKS_PER_SEC << endl;
  return 0;
}

int Leq_VerifyEqX(Abc_Ntk_t* pNtkXv, Leq_Ba_t* pBaX, int fUseCommand){
  abctime clk = Abc_Clock();
  Leq_Ba_t * pBaXCopy = Leq_BaDuplicate(pBaX);
  printf( "[LEQ] - Check origin X is contain by solution X\n" );
  Abc_Ntk_t * pMiter;
  Fra_Sec_t SecPar, * pSecPar = &SecPar;
  // int fUseCommand = 0; // 0 for dsec, 1 for pdr, 2 for dprove, 3 for int
  // consturct miter
  pMiter = Leq_XorBaTransCir(pNtkXv, Leq_BaTransCir(pBaXCopy), 1);
  cerr << "finish xor" << endl;
  
  if(fUseCommand == 0){ // dsec 
    Fra_SecSetDefaultParams( pSecPar );
    pSecPar->TimeLimit = 0;
    pSecPar->fVerbose = 1;  // enables verbose reporting of statistics
    pSecPar->fVeryVerbose = 1;  // enables very verbose reporting  
    pSecPar->fReportSolution = 1;
    if(Leq_NtkDarSec( pMiter, pSecPar )){
      cerr << "Runtime for Leq_VerifyX : " << (double)(Abc_Clock() - clk )/(double)CLOCKS_PER_SEC << endl;
      return 1;  
    }
  }
  else if(fUseCommand == 1){ // pdr
    Pdr_Par_t Pars, * pPars = &Pars;
    Pdr_ManSetDefaultParams( pPars );
    pPars->fVerbose = 1;
    if(Abc_NtkDarPdr( pMiter, pPars )!=0){
      cerr << "Runtime for Leq_VerifyX : " << (double)(Abc_Clock() - clk )/(double)CLOCKS_PER_SEC << endl;
      return 1;  
    }
  }
  else if(fUseCommand == 2){ // dprove
    Fra_SecSetDefaultParams( pSecPar );
    pSecPar->TimeLimit = 0;
    pSecPar->fVerbose = 1;  // enables verbose reporting of statistics
    pSecPar->fVeryVerbose = 1;  // enables very verbose reporting  
    pSecPar->fReportSolution = 1;
    if(Abc_NtkDarProve( pMiter, pSecPar, 20, 2000 )!=0){
      cerr << "Runtime for Leq_VerifyX : " << (double)(Abc_Clock() - clk )/(double)CLOCKS_PER_SEC << endl;
      return 1;  
    }
  }
  else if(fUseCommand == 3){ // int
    Inter_ManParams_t Pars, * pPars = &Pars;
    Inter_ManSetDefaultParams( pPars );
    pPars->fVerbose = 1;
    if(Abc_NtkDarBmcInter( pMiter, pPars, NULL )!=0){
      cerr << "Runtime for Leq_VerifyX : " << (double)(Abc_Clock() - clk )/(double)CLOCKS_PER_SEC << endl;
      return 1;  
    }
  }
  cerr << "Runtime for Leq_VerifyX : " << (double)(Abc_Clock() - clk )/(double)CLOCKS_PER_SEC << endl;
  return 0;
}

// int Leq_XBalm( Abc_Ntk_t * pNtkX, Abc_Ntk_t * pNtkB, int fUseCommand ){
//   abctime clk = Abc_Clock();
//   printf( "[LEQ] - Equivalence checking with ours and balmII\n" );
//   Abc_Ntk_t * pMiter;
//   Fra_Sec_t SecPar, * pSecPar = &SecPar;
//   // int fUseCommand = 0; // 0 for dsec, 1 for pdr, 2 for dprove, 3 for int
//   // consturct miter
//   pMiter = Leq_XorTransCir(pNtkX, pNtkB);
//   cerr << "finish miter" << endl;
  
//   if(fUseCommand == 0){ // dsec 
//     Fra_SecSetDefaultParams( pSecPar );
//     pSecPar->TimeLimit = 0;
//     pSecPar->fVerbose = 1;  // enables verbose reporting of statistics
//     pSecPar->fVeryVerbose = 1;  // enables very verbose reporting  
//     pSecPar->fReportSolution = 1;
//     if(Leq_NtkDarSec( pMiter, pSecPar )){
//       cerr << "Runtime for Leq_VerifyX : " << (double)(Abc_Clock() - clk )/(double)CLOCKS_PER_SEC << endl;
//       return 1;  
//     }
//   }
//   else if(fUseCommand == 1){ // pdr
//     Pdr_Par_t Pars, * pPars = &Pars;
//     Pdr_ManSetDefaultParams( pPars );
//     pPars->fVerbose = 1;
//     if(Abc_NtkDarPdr( pMiter, pPars )!=0){
//       cerr << "Runtime for Leq_VerifyX : " << (double)(Abc_Clock() - clk )/(double)CLOCKS_PER_SEC << endl;
//       return 1;  
//     }
//   }
//   else if(fUseCommand == 2){ // dprove
//     Fra_SecSetDefaultParams( pSecPar );
//     pSecPar->TimeLimit = 0;
//     pSecPar->fVerbose = 1;  // enables verbose reporting of statistics
//     pSecPar->fVeryVerbose = 1;  // enables very verbose reporting  
//     pSecPar->fReportSolution = 1;
//     if(Abc_NtkDarProve( pMiter, pSecPar, 20, 2000 )!=0){
//       cerr << "Runtime for Leq_VerifyX : " << (double)(Abc_Clock() - clk )/(double)CLOCKS_PER_SEC << endl;
//       return 1;  
//     }
//   }
//   else if(fUseCommand == 3){ // int
//     Inter_ManParams_t Pars, * pPars = &Pars;
//     Inter_ManSetDefaultParams( pPars );
//     pPars->fVerbose = 1;
//     if(Abc_NtkDarBmcInter( pMiter, pPars, NULL )!=0){
//       cerr << "Runtime for Leq_VerifyX : " << (double)(Abc_Clock() - clk )/(double)CLOCKS_PER_SEC << endl;
//       return 1;  
//     }
//   }
//   cerr << "Runtime for Leq_VerifyX : " << (double)(Abc_Clock() - clk )/(double)CLOCKS_PER_SEC << endl;
//   return 0;
// }

// /**Function*************************************************************

//   Synopsis    [Return product(A,B)]

//   Description []
               
//   SideEffects []

//   SeeAlso     []

// ***********************************************************************/
// Abc_Ntk_t * Leq_XorTransCir(Abc_Ntk_t * pNtkX, Abc_Ntk_t * pNtkB){
//   if ( !Abc_NtkIsStrash(pNtkB) )
//     pNtkB = Abc_NtkStrash( pNtkB, 0, 1, 0 );
//   Abc_Ntk_t * pNtkMiter;
//   pNtkMiter = Abc_NtkAlloc( ABC_NTK_STRASH, ABC_FUNC_AIG, 1 );
//   pNtkMiter->pName = "baX";

//   Abc_Obj_t * pObj, * pObjNew;
//   int i, nPi;
//   Abc_AigConst1(pNtkX)->pCopy = Abc_AigConst1(pNtkMiter);
//   Abc_AigConst1(pNtkB)->pCopy = Abc_AigConst1(pNtkMiter);
  
//   // create new PIs and remember them in the old PIs
//   nPi = Abc_NtkPiNum(pNtkX);
//   Abc_NtkForEachPi( pNtkB, pObj, i ){
//     pObjNew = Abc_NtkCreatePi( pNtkMiter );
//     // remember this PI in the old PIs
//     pObj->pCopy = pObjNew;
//     // Abc_NtkFindNode(pNtkX, Abc_ObjName(pObj));
//     // cerr << "Find " << Abc_ObjName(pObj) << " node" << endl; // !
//     // add name
//     Abc_ObjAssignName( pObjNew, Abc_ObjName(pObj), NULL );
//     Abc_Obj_t * pObjTmp = pObj;
//     pObj = Abc_NtkFindCi(pNtkX, Abc_ObjName(pObj));;  
//     // cerr << "Abc_ObjIsPi: " << Abc_ObjIsPi(pObj) << endl;
//     if(pObj != NULL && Abc_ObjIsPi(pObj)) // !
//     // if(pObj != NULL )
//       pObj->pCopy = pObjNew;
//     else{
//       cerr << "No " << Abc_ObjName(pObjTmp) << " node" << endl; // !
//     } 
//   }
//   cerr << "finish PI" << endl;
//   // create POs
//   pObjNew = Abc_NtkCreatePo( pNtkMiter );
//   Abc_ObjAssignName( pObjNew, "product_f0", NULL );

//   // create the latches
//   Abc_NtkForEachLatch( pNtkX, pObj, i ){
//     pObjNew = Abc_NtkDupBox( pNtkMiter, pObj, 0 );
//     // add names
//     Abc_ObjAssignName( pObjNew, Abc_ObjName(pObj), "_s" );
//     Abc_ObjAssignName( Abc_ObjFanin0(pObjNew),  Abc_ObjName(Abc_ObjFanin0(pObj)), "_s" );
//     Abc_ObjAssignName( Abc_ObjFanout0(pObjNew), Abc_ObjName(Abc_ObjFanout0(pObj)), "_s" );
//   }
//   Abc_NtkForEachLatch( pNtkB, pObj, i ){
//     pObjNew = Abc_NtkDupBox( pNtkMiter, pObj, 0 );
//     // add name
//     Abc_ObjAssignName( pObjNew, Abc_ObjName(pObj), "_f" );
//     Abc_ObjAssignName( Abc_ObjFanin0(pObjNew),  Abc_ObjName(Abc_ObjFanin0(pObj)), "_f" );
//     Abc_ObjAssignName( Abc_ObjFanout0(pObjNew), Abc_ObjName(Abc_ObjFanout0(pObj)), "_f" );
//   }

//   Abc_Obj_t * pNode;
//   assert( Abc_NtkIsDfsOrdered(pNtkX) );
//   Abc_AigForEachAnd( pNtkX, pNode, i )
//     pNode->pCopy = Abc_AigAnd( (Abc_Aig_t *)pNtkMiter->pManFunc, Abc_ObjChild0Copy(pNode), Abc_ObjChild1Copy(pNode) );
//   assert( Abc_NtkIsDfsOrdered(pNtkB) );
//   Abc_AigForEachAnd( pNtkB, pNode, i )
//     pNode->pCopy = Abc_AigAnd( (Abc_Aig_t *)pNtkMiter->pManFunc, Abc_ObjChild0Copy(pNode), Abc_ObjChild1Copy(pNode) );

//   cerr << "start connect PO" << endl;


//   Vec_Ptr_t * vPairs;

//   // collect the PO pairs from both networks
//   vPairs = Vec_PtrAlloc( 100 );
//   // collect the PO nodes for the miter
//   Abc_NtkForEachPo( pNtkX, pNode, i ) {
//     Vec_PtrPush( vPairs, Abc_ObjChild0Copy(pNode) );
//     pNode = Abc_NtkPo( pNtkB, i );
//     Vec_PtrPush( vPairs, Abc_ObjChild0Copy(pNode) );
//   }
//   // connect new latches
//   Abc_NtkForEachLatch( pNtkX, pNode, i )
//     Abc_ObjAddFanin( Abc_ObjFanin0(pNode)->pCopy, Abc_ObjChild0Copy(Abc_ObjFanin0(pNode)) );
//   Abc_NtkForEachLatch( pNtkB, pNode, i )
//     Abc_ObjAddFanin( Abc_ObjFanin0(pNode)->pCopy, Abc_ObjChild0Copy(Abc_ObjFanin0(pNode)) );

//   // add the miter
//   Abc_ObjAddFanin( Abc_NtkPo(pNtkMiter,0), Abc_AigXor( (Abc_Aig_t *)pNtkMiter->pManFunc, (Abc_Obj_t *)vPairs->pArray[0], (Abc_Obj_t *)vPairs->pArray[1] ));
//   Vec_PtrFree( vPairs );
  
//   return pNtkMiter;
// }