#include "leqSolving/leqSolving.hpp"

/**Function*************************************************************

  Synopsis    [Generate simulation pattern for BA]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Leq_GenSimPattern(Leq_Ba_t * pBa, const unsigned & stateId, const unsigned & piId, const unsigned & nChoiceNum, Vec_Str_t * vSimPattern){

  // int i, j, 
  int k, bufSize = Leq_BaNumInput(pBa)-Leq_BaNumInactInput(pBa);
  char bufferState[Leq_BaNumState(pBa)];
  char bufferNChoice[Leq_BaNumInactInput(pBa)];
  char bufferInput[bufSize];
  // cerr << "state id : " << stateId << endl;
  // cerr << "input id : " << piId << endl;
  // cerr << "nChoiceNum: " << nChoiceNum << endl;
  // cerr << "nChoice: " << Leq_BaNumInactInput(pBa) << endl;
  // getchar();
  // for(i = 0; i < stateNum; ++i){ //each state vector
  Leq_Dec2Bin(stateId, bufferState, Leq_BaNumState(pBa));
  // for(j = 0; j < piNum; ++j){ //each input
  Leq_Dec2Bin(piId, bufferInput, bufSize);
  for(k = 0; k < nChoiceNum; ++k){ //each nondeter choice
    Leq_Dec2Bin(k, bufferNChoice, Leq_BaNumInactInput(pBa));
    // printf ("nChoice: %s  input: %s  state: %s\n",bufferNChoice, bufferInput, bufferState);
    Vec_StrPushBuffer( vSimPattern, bufferNChoice, Leq_BaNumInactInput(pBa));
    Vec_StrPushBuffer( vSimPattern, bufferInput, bufSize);
    Vec_StrPushBuffer( vSimPattern, bufferState, Leq_BaNumState(pBa));
  }
  // printf ("nChoice: %d  input: %d  state: %d\n",k, piId, stateId);
  if(nChoiceNum == 0){
    Vec_StrPushBuffer( vSimPattern, bufferInput, bufSize);
    Vec_StrPushBuffer( vSimPattern, bufferState, Leq_BaNumState(pBa));
  }
  
  // }
  // }
  // Vec_StrIntPrint( vSimPattern );
  #ifdef SHOW
    Vec_StrIntPrint( vSimPattern );
  #endif
  return;
}

/**Function*************************************************************

  Synopsis    [Simulate comb cir]

  Description []
               
  SideEffects []

  SeeAlso     [Fra_SmlSimulateCombGiven()]

***********************************************************************/
Fra_Sml_t * Leq_SimComb( Aig_Man_t * pAig, Vec_Str_t * vSimInfo ){
  Fra_Sml_t * p;
  assert( Aig_ManRegNum(pAig) == 0 );
  if ( vSimInfo == NULL )
    return NULL;
  if ( Vec_StrSize(vSimInfo) % Aig_ManCiNum(pAig) != 0 ){
    printf( "The number of binary digits (%d) is not divisible by the number of primary inputs (%d).\n", 
       Vec_StrSize(vSimInfo), Aig_ManCiNum(pAig) );
    Vec_StrFree( vSimInfo );
    return NULL;
  }
  // cerr << "Abc_BitWordNum " << Abc_BitWordNum(Vec_StrSize(vSimInfo) / Aig_ManCiNum(pAig))  << endl;
  p = Fra_SmlStart( pAig, 0, 1, Abc_BitWordNum(Vec_StrSize(vSimInfo) / Aig_ManCiNum(pAig)) );
  // cerr << "finish sml start" << endl;
  Fra_SmlInitializeGiven( p, vSimInfo );
  Vec_StrFree( vSimInfo );
  Fra_SmlSimulateOne( p );
  return p;
}

void Leq_SimCombNew(Vec_Str_t * vSimInfo, Fra_Sml_t * p){
  Fra_SmlInitializeGiven( p, vSimInfo );
  Vec_StrFree( vSimInfo );
  Fra_SmlSimulateOne( p );
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     [Fra_SmlStart]

***********************************************************************/
Fra_Sml_t * Leq_SimStart( Aig_Man_t * pAig, int nPref, int nFrames, int nWordsFrame )
{
    Fra_Sml_t * p;
    p = (Fra_Sml_t *)ABC_ALLOC( char, sizeof(Fra_Sml_t) + sizeof(unsigned) * (Aig_ManObjNumMax(pAig)+1000) * (nPref + nFrames) * nWordsFrame );
    memset( p, 0, sizeof(Fra_Sml_t) + sizeof(unsigned) * (nPref + nFrames) * nWordsFrame );
    p->pAig        = pAig;
    p->nPref       = nPref;
    p->nFrames     = nPref + nFrames;
    p->nWordsFrame = nWordsFrame;
    p->nWordsTotal = (nPref + nFrames) * nWordsFrame;
    p->nWordsPref  = nPref * nWordsFrame;
    // constant 1 is initialized to 0 because we store values modulus phase (pObj->fPhase)
    return p;
}

/**Function*************************************************************

  Synopsis    [Get simulation outputs]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Leq_GetSimOutputs( Fra_Sml_t * p, const int & nChoiceNum, unordered_set<unsigned> & sSimResult, set<unsigned> & sF0, set<unsigned> &sNF0){
  Aig_Obj_t * pObj;
  unsigned * pSims;
  int k, t = 0, m, n = nChoiceNum? nChoiceNum : 1;
  bool fail = 0;
  int state;
  vector<bool> vResult(Aig_ManCoNum(p->pAig)-1,0);
  for(k = 0; k < n; ++k){ //each nondeter choice
    fail = 0;
    Aig_ManForEachCo( p->pAig, pObj, m ){
      pSims = Fra_ObjSim( p, pObj->Id );
      // printf( "%d", Abc_InfoHasBit( pSims, t ) );
      if( m == 0 && !(bool)Abc_InfoHasBit( pSims, t )){
        // printf( "\n");
        fail = 1;
        //! tmp 
        //return 0;
        //!
      }
      else if(m) vResult[m-1] = (bool)Abc_InfoHasBit( pSims, t );
    }
    // printf( "\n");
    ++t;
    state = Leq_Bin2Dec(vResult);
    // cerr << "pattern " << bufferNChoice << bufferInput << bufferState << " : " << bufferOut << endl;
    sSimResult.emplace(state);
    if(fail)
      sNF0.emplace(state);
    else 
      sF0.emplace(state);
    // printf( "\n" );
  }
  // for(auto s : sSimResult){
  //   cerr << " " << s << " ";
  // }
  // cerr << endl;
  return;
  // if(fail) return 0;
  // return 1;
}

bool Leq_InproGetSimOutputs( Fra_Sml_t * p, const int & nChoiceNum, unordered_set<unsigned> & sSimResult){
  Aig_Obj_t * pObj;
  unsigned * pSims;
  int k, t = 0, m, n = nChoiceNum? nChoiceNum : 1;
  bool fail = 0;
  vector<bool> vResult(Aig_ManCoNum(p->pAig)-1,0);
  for(k = 0; k < n; ++k){ //each nondeter choice
    Aig_ManForEachCo( p->pAig, pObj, m ){
      pSims = Fra_ObjSim( p, pObj->Id );
      // printf( "%d", Abc_InfoHasBit( pSims, t ) );
      if( m == 0 && !(bool)Abc_InfoHasBit( pSims, t )){
        // printf( "\n");
        fail = 1;
        //! tmp 
        //return 0;
        //!
      }
      else if(m) vResult[m-1] = (bool)Abc_InfoHasBit( pSims, t );
    }
    // printf( "\n");
    ++t;
    // cerr << "pattern " << bufferNChoice << bufferInput << bufferState << " : " << bufferOut << endl;
    sSimResult.emplace(Leq_Bin2Dec(vResult));
    // printf( "\n" );
  }
  // for(auto s : sSimResult){
  //   cerr << " " << s << " ";
  // }
  // cerr << endl;
  if(fail) return 0;
  return 1;
}


