#include "leqSolving/leqSolving.hpp"
#include "ext-Synthesis/cadet.h"
#include <fstream>

void Leq_EliminateVarManthan(Abc_Ntk_t * &pNtk, unsigned uBegin, Leq_Ba_t * pBa){ //type 0 -> exist, 1-> universal
  // Q(U)E(V)Q(N) F(L,U,V,N)
  // => E(U)Q(V)E(N) \neq F(L,U,V,N)
  int origLatchNum = Abc_NtkLatchNum(pNtk);
  Abc_Ntk_t * pNtkComb = Abc_NtkDup(pNtk);
  Abc_NtkPrintStats( pNtk, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
  Abc_NtkMakeComb(pNtkComb, 0);
  Abc_NtkPrintStats( pNtkComb, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
  int i;
  Vec_Int_t* pScopeU = Vec_IntAlloc( Abc_NtkPiNum(pNtk)-1 - uBegin );
  Vec_Int_t* pScopeV = Vec_IntAlloc( uBegin - Leq_BaNumInactInput(pBa) +1 );
  Vec_Int_t* pScopeN = Vec_IntAlloc( Leq_BaNumInactInput(pBa) );

  Abc_Ntk_t * pNtkOnePo = Abc_NtkMakeOnePo(pNtkComb, 0, 1);
  Abc_NtkDelete(pNtkComb);
  Abc_NtkPrintStats( pNtkOnePo, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
  // add not gate to PO
  cerr << "begin with " << Abc_NtkPiNum(pNtkOnePo)-1-origLatchNum << " , end with " << uBegin << endl;
  cerr << "U variable : " ;
  for(i = Abc_NtkPiNum(pNtkOnePo)-1-origLatchNum; i > uBegin; --i){
    cerr << Abc_ObjName(Abc_NtkPi(pNtkOnePo, i)) << "(" << Abc_ObjFanoutNum(Abc_NtkPi(pNtkOnePo, i)) << ") ";
    if(Abc_ObjFanoutNum(Abc_NtkPi(pNtkOnePo, i)))
      Vec_IntPush( pScopeU, Abc_NtkPi(pNtkOnePo, i)->Id );
  }
  cerr << endl;

  
  // ! run cadat
  Abc_Obj_t * pF0 = Abc_NtkPo(pNtkOnePo, 0);
  Abc_Obj_t * pNewPoIn = Abc_ObjNot(Abc_ObjChild0(pF0));
  Abc_ObjRemoveFanins(pF0);
  Abc_ObjAddFanin( pF0, pNewPoIn);
  Abc_Ntk_t* pNtkU = pNtkOnePo;
  if(Vec_IntSize(pScopeU)){
    pNtkU = manthanExistsElim(pNtkOnePo, pScopeU);
    Abc_NtkDelete(pNtkOnePo);
  }
  Abc_NtkPrintStats( pNtkU, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
  cerr << "end run manthan " << endl;
  // Abc_Ntk_t* pNtkU = pNtkOnePo;
  cerr << endl << "V variable : ";
  for(i = uBegin; i >= Leq_BaNumInactInput(pBa); --i){
    cerr << Abc_ObjName(Abc_NtkPi(pNtkU, i)) << "(" << Abc_ObjFanoutNum(Abc_NtkPi(pNtkU, i)) << ") ";
    if(Abc_ObjFanoutNum(Abc_NtkPi(pNtkU, i))){
      Vec_IntPush( pScopeV, Abc_NtkPi(pNtkU, i)->Id );
    }
  }
  cerr << endl;
  // Q(V)E(N) \neq F(L,V,N)
  // => E(V)Q(N) \neq\neq F(L,V,N)
  Abc_Ntk_t * pNtkTmp = pNtkU;
  pNtkU = Abc_NtkDC2( pNtkTmp, 0, 0, 1, 0, 0 );
  Abc_NtkDelete(pNtkTmp);
  // ! run cadat
  pF0 = Abc_NtkPo(pNtkU, 0);
  pNewPoIn = Abc_ObjNot(Abc_ObjChild0(pF0));
  Abc_ObjRemoveFanins(pF0);
  Abc_ObjAddFanin( pF0, pNewPoIn);
  Abc_Ntk_t* pNtkV = pNtkU;
  if(Vec_IntSize(pScopeV)){
    pNtkV = manthanExistsElim(pNtkU, pScopeV);
    Abc_NtkDelete(pNtkU);
  }
  Abc_NtkPrintStats( pNtkV, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
  cerr << "end run manthan " << endl;

  cerr << endl << "N variable : ";
  for(i = Leq_BaNumInactInput(pBa)-1; i >= 0; --i){
    cerr << Abc_ObjName(Abc_NtkPi(pNtkV, i)) << "(" << Abc_ObjFanoutNum(Abc_NtkPi(pNtkV, i)) << ") ";
    if(Abc_ObjFanoutNum(Abc_NtkPi(pNtkV, i)))
      Vec_IntPush( pScopeN, Abc_NtkPi(pNtkV, i)->Id );
  }
  cerr << endl;
  // Q(N) \neq\neq F(L,N)
  // => E(N) \neq F(L,N)
  // ! run cadat
  pNtkTmp = pNtkV;
  pNtkV = Abc_NtkDC2( pNtkTmp, 0, 0, 1, 0, 0 );
  Abc_NtkDelete(pNtkTmp);

  pF0 = Abc_NtkPo(pNtkV, 0);
  pNewPoIn = Abc_ObjNot(Abc_ObjChild0(pF0));
  Abc_ObjRemoveFanins(pF0);
  Abc_ObjAddFanin( pF0, pNewPoIn);
  Abc_Ntk_t* pNtkN = pNtkV;
  if(Vec_IntSize(pScopeN)){
    pNtkN = manthanExistsElim(pNtkV, pScopeN);
    Abc_NtkDelete(pNtkV);
  }
  pNtk = pNtkN;
  // pNtk = Abc_NtkDarCleanupAig( pNtkN, 0, 0, 0 );
  pNtk = Abc_NtkStrash( pNtkN, 0, 1, 0 );
  // Abc_AigCleanup((Abc_Aig_t *)pNtkN->pManFunc);
  // pNtk = Abc_NtkDup(pNtkN);
  pF0 = Abc_NtkPo(pNtk, 0);
  pNewPoIn = Abc_ObjNot(Abc_ObjChild0(pF0));
  Abc_ObjRemoveFanins(pF0);
  Abc_ObjAddFanin( pF0, pNewPoIn);
  Abc_NtkDelete(pNtkN);
  Abc_NtkPrintStats( pNtk, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
  cerr << "end run manthan " << endl;
  // Abc_NtkShow( pNtk, 0, 0, 1 );
  // getchar();
  return;
}

// Abc_Ntk_t * Leq_makeOnePo(Abc_Ntk_t * pNtk){
//   Abc_Obj_t * pNodeCo = Abc_NtkCo( pNtk, 0 );
//   Abc_Ntk_t * pNtkRes = Abc_NtkCreateCone( pNtk, Abc_ObjFanin0(pNodeCo), Abc_ObjName(pNodeCo), 1 );
//   if ( pNodeCo && Abc_ObjFaninC0(pNodeCo) ){
//     Abc_NtkPo(pNtkRes, 0)->fCompl0  ^= 1;
//   }
//   return pNtkRes;
// }