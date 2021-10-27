#include "leqSolving/leqSolving.hpp"

// void Leq_PrintAigCo(Aig_Man_t * pAig){
//   Aig_Obj_t * pObj;
//   int i;
//   Abc_Print(1, "Print CO INFO: Name/ID\n");
//   Aig_ManForEachCo(pAig, pObj, i){
//     cerr << Abc_ObjName(pObj) << ": " << Abc_ObjId(pObj) << endl;
//   }  
// }

// void Leq_PrintAigCi(Aig_Man_t * pNtk){
//   Aig_Obj_t * pObj;
//   int i;
//   Abc_Print(1, "Print CI INFO: Name/ID\n");
//   Aig_ManForEachCi(pNtk, pObj, i){
//     cerr << Abc_ObjName(pObj) << ": " << Abc_ObjId(pObj) << endl;
//   }  
// }

void Leq_PrintNtkLatchValue(Abc_Ntk_t * pNtk){
  cerr << "show miter" << endl;  
  cerr << " - " << endl;
  Abc_Obj_t * pLatch;
  int i;
  Abc_NtkForEachLatch( pNtk, pLatch, i ){
    if ( Abc_LatchIsInit0(pLatch) )
      cerr << Abc_ObjName(pLatch) << " = " << 0 << endl;
    else if ( Abc_LatchIsInit1(pLatch) )
      cerr << Abc_ObjName(pLatch) << " = " << 1 << endl;
    else{
      cerr << Abc_ObjName(pLatch) << " = " << -1 << endl;
    }
  } 
}

void Leq_PrintNtkPo(Abc_Ntk_t * pNtk){
  Abc_Obj_t * pObj;
  int i;
  cerr << "Print PO INFO: Name/ID" << endl;
  Abc_NtkForEachPo(pNtk, pObj, i){
    cerr << Abc_ObjName(pObj) << ": " << Abc_ObjId(pObj) << endl;
  }  
}

void Leq_PrintNtkPi(Abc_Ntk_t * pNtk){
  Abc_Obj_t * pObj;
  int i;
  cerr << "Print PI INFO: Name/ID" << endl;
  Abc_NtkForEachPi(pNtk, pObj, i){
    cerr << Abc_ObjName(pObj) << ": " << Abc_ObjId(pObj) << endl;
  }  
}

void Leq_WriteBA(Leq_Ba_t * pBa){ 
  char filename[sizeof(Leq_BaName(pBa)) + 3];
  strcpy (filename, Leq_BaName(pBa)) ;
  strcat (filename, ".mv") ;
  Io_Write( Leq_BaTransCir(pBa), filename, IO_FILE_BLIFMV ); 
}

void Leq_Dec2Bin(int n, char * buffer, int size){
  int t = size - 1;
  for (t = size - 1; t >= 0 ; --t){
    if (n & 1)
      *(buffer+t) = (char)(1);
    else
      *(buffer+t) = (char)(0);
    n = n >> 1;
  }
  return ;
}

void Leq_Dec2BinChar(int n, char * buffer, int size){
  int c, t = size - 1;
  for (c = 0 ; c < size ; ++c){
    if (n & 1)
      *(buffer+t) = '1';
    else
      *(buffer+t) = '0';
    --t;
    n = n >> 1;
  }
  return ;
}

void Leq_Dec2BinInt(int n, vector<bool> & vResult){
  int c, t = vResult.size() - 1;
  for (c = 0 ; c < vResult.size() ; ++c){
    if (n & 1)
      vResult[t] = 1;
    else
      vResult[t] = 0;
    --t;
    n = n >> 1;
  }
  return ;
}

unsigned Leq_Bin2Dec(vector<bool> & vResult){
  unsigned value = 0, base = 1;
  for(int i = vResult.size()-1; i >= 0; --i){
    // cerr << i << endl;
    // cerr << vResult[i] << endl;
    // cerr << base << endl;
    if(vResult[i]){
      value += base;
    }
    base = base << 1;
    // cerr << value << endl;
  }
  // cerr << "in Leq_Bin2Dec: value = " << value << endl; 
  // getchar();
  return value;
}

void Leq_NtkMakeSeq( Abc_Ntk_t * pNtk, vector<bool> vVal){
  Abc_Obj_t * pObjLi, * pObjLo, * pObj;
  int i, nLatchesToAdd = vVal.size();
  assert( Abc_NtkBoxNum(pNtk) == 0 );
  if ( !Abc_NtkIsComb(pNtk) ){
    printf( "The network is a not a combinational one.\n" );
    return;
  }
  if ( nLatchesToAdd >= Abc_NtkPiNum(pNtk) ){
    printf( "The number of latches is more or equal than the number of PIs.\n" );
    return;
  }
  if ( nLatchesToAdd >= Abc_NtkPoNum(pNtk) ){
      printf( "The number of latches is more or equal than the number of POs.\n" );
      return;
  }
   
  // move the last POs to become COs
  // int delNum = 0;
  vector<int> vDelPoId(nLatchesToAdd, 0);
  Vec_PtrClear( pNtk->vPos );
  Abc_NtkForEachCo( pNtk, pObj, i ){
    if ( i < Abc_NtkCoNum(pNtk) - nLatchesToAdd ){
      Vec_PtrPush( pNtk->vPos, pObj );
      continue;
    }
    pObj->Type = ABC_OBJ_BI;
    pNtk->nObjCounts[ABC_OBJ_BI]++;
    pNtk->nObjCounts[ABC_OBJ_PO]--;
  }

  // move the last PIs to become CIs
  // vector<bool>::iterator iterV = vVal.begin();
  Vec_PtrClear( pNtk->vPis );
  Abc_NtkForEachCi( pNtk, pObj, i ){
    if ( i < Abc_NtkCiNum(pNtk) - nLatchesToAdd ){
      Vec_PtrPush( pNtk->vPis, pObj );
      continue;
    }
    pObj->Type = ABC_OBJ_BO;
    pNtk->nObjCounts[ABC_OBJ_PI]--;
    pNtk->nObjCounts[ABC_OBJ_BO]++;
  }

  // create latches
  //TODO delete const la and replace pi by const
  char * pName = (char*) malloc (sizeof(char) * 10);
  for ( i = 0; i < nLatchesToAdd; i++ ){
    pObjLo = Abc_NtkCi( pNtk, Abc_NtkCiNum(pNtk) - nLatchesToAdd + i );
    // sprintf( pName, "s%d_o", i );
    // Abc_ObjAssignName(pObjLo, pName, NULL);
    pObjLi = Abc_NtkCo( pNtk, Abc_NtkCoNum(pNtk) - nLatchesToAdd + i );
    // sprintf( pName, "s%d_i", i );
    // Abc_ObjAssignName(pObjLi, pName, NULL);
    pObj = Abc_NtkCreateLatch( pNtk );
    sprintf( pName, "s%d", i );
    Abc_ObjAssignName(pObj, pName, NULL);
    Abc_ObjAddFanin( pObj, pObjLi);
    Abc_ObjAddFanin( pObjLo, pObj);
    if(vVal[i])
      Abc_LatchSetInit1( pObj );
    else
      Abc_LatchSetInit0( pObj );
  }

  if ( !Abc_NtkCheck( pNtk ) )
    fprintf( stdout, "Abc_NtkMakeSeq(): Network check has failed.\n" );
}

Abc_Ntk_t * Leq_Const0Cir(Leq_Ba_t * pBa){
  Abc_Ntk_t * pNtk;
  Abc_Obj_t * pObj, * pObjNew, * pConst1;
  int i;
  pNtk = Abc_NtkAlloc( ABC_NTK_STRASH, ABC_FUNC_AIG, 1 );
  pConst1 = Abc_AigConst1(pNtk);
  Abc_NtkForEachPi( pBa->_pNtk, pObj, i ){
    pObjNew = Abc_NtkCreatePi( pNtk );
    Abc_ObjAssignName( pObjNew, Abc_ObjName(pObj), NULL );
  }
  pObj = Abc_NtkCreatePo( pNtk );  
  Abc_ObjAssignName( pObj, "f0", NULL );
  Abc_ObjAddFanin(pObj,Abc_ObjNot(pConst1));
  
  if ( !Abc_NtkCheck( pNtk ) )
  {
    printf( "pNtk: The network check has failed.\n" );
    Abc_NtkDelete( pNtk );
    return nullptr;
  }
  return pNtk;
}

/**Function*************************************************************

  Synopsis    [Set the output of BA]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
Abc_Obj_t * Leq_SetVectorOr( Abc_Aig_t * pMan, Abc_Obj_t ** ppObjs, int nObjs ){
  Abc_Obj_t * pObj1, * pObj2;
  if ( nObjs == 1 )
    return ppObjs[0];
  pObj1 = Leq_SetVectorOr( pMan, ppObjs,           nObjs/2 );
  pObj2 = Leq_SetVectorOr( pMan, ppObjs + nObjs/2, nObjs - nObjs/2 );
  return Abc_AigOr( pMan, pObj1, pObj2 );
}

Abc_Obj_t * Leq_SetVectorAnd( Abc_Aig_t * pMan, Abc_Obj_t ** ppObjs, int nObjs ){
  Abc_Obj_t * pObj1, * pObj2;
  if ( nObjs == 1 )
    return ppObjs[0];
  pObj1 = Leq_SetVectorAnd( pMan, ppObjs,           nObjs/2 );
  pObj2 = Leq_SetVectorAnd( pMan, ppObjs + nObjs/2, nObjs - nObjs/2 );
  return Abc_AigAnd( pMan, pObj1, pObj2 );
}

Aig_Obj_t * Leq_SetVectorAigAnd( Aig_Man_t * pMan, Aig_Obj_t ** ppObjs, int nObjs ){
  Aig_Obj_t * pObj1, * pObj2;
  if ( nObjs == 1 )
    return ppObjs[0];
  pObj1 = Leq_SetVectorAigAnd( pMan, ppObjs,           nObjs/2 );
  pObj2 = Leq_SetVectorAigAnd( pMan, ppObjs + nObjs/2, nObjs - nObjs/2 );
  return Aig_And( pMan, pObj1, pObj2 );
} 

void Leq_writeVerilog(Abc_Ntk_t * pNtk, char * pFileName, bool complement){
  // Abc_Ntk_t * pNtkTemp = Abc_NtkToNetlist( pNtk );
  Abc_Ntk_t * pNtkTemp = Abc_NtkDup( pNtk );
  if(complement){
    Abc_Obj_t * pF0 = Abc_NtkPo(pNtkTemp, 0);
    Abc_Obj_t * pNewPoIn = Abc_ObjNot(Abc_ObjChild0(pF0));
    Abc_ObjRemoveFanins(pF0);
    Abc_ObjAddFanin( pF0, pNewPoIn);
  }
  // if ( !Abc_NtkHasAig(pNtkTemp) && !Abc_NtkHasMapping(pNtkTemp) )
  //   Abc_NtkToAig( pNtkTemp );
  cerr << "print cir converted to verilog" << endl;
  Abc_NtkShow( pNtkTemp, 0, 0, 1 );
  getchar();
  Io_Write(pNtkTemp, pFileName, IO_FILE_VERILOG);
  // Io_WriteVerilog( pNtkTemp, pFileName, 1 );
  Abc_NtkDelete( pNtkTemp );
}