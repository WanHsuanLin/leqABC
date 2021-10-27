#include "leqSolving/leqSolving.hpp"
/**Function*************************************************************

  Synopsis    [Write the eqn from simulation]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

Abc_Ntk_t * Leq_EqnWriteNetwork( Abc_Ntk_t * pNtkOri, Leq_Ba_t * pBa, vector<vector<set<unsigned> *> > & vNextState, set<unsigned> & sF0, set<unsigned> & sNF0 ){
  Vec_Ptr_t * vVars;
  Abc_Ntk_t * pNtk;
  Abc_Obj_t * pObj, * pNode;
  char * pNodeName = (char*) malloc (sizeof(char) * 20), * pFormula = (char*) malloc (sizeof(char) * 50000000), * pVarName = (char*) malloc (sizeof(char) * 100);
  int i, j;
  int stateNum = vNextState.size(), piIdBegin = Leq_BaNumInactInput(pBa), piIdEnd = Leq_BaNumInput(pBa), piNum = 1 << (Leq_BaNumInput(pBa)-Leq_BaNumInactInput(pBa));
  // allocate the empty network
  pNtk = Abc_NtkAlloc( ABC_NTK_NETLIST, ABC_FUNC_AIG, 1 );
  // set the specs
  pNtk->pName = Leq_BaName(pBa);
  // pNtk->pSpec = Extra_UtilStrsav(Extra_FileReaderGetFileName(p));

  // go through the lines of the file
  vVars  = Vec_PtrAlloc( 10000 );
  bool print = 0;
  // construct pi
  // // TODO
  cerr << "[Info] print PI info of the new ntk " << endl;
  for (i = piIdBegin; (i < piIdEnd) && (((pObj) = Abc_NtkPi(pNtkOri, i)), 1); ++i){
    // cerr << "create " << i << " pi, name: " << Abc_ObjName(pObj) << endl;
    pNode = Io_ReadCreatePi( pNtk, Abc_ObjName(pObj) );
    Abc_ObjAssignName( pNode, Abc_ObjName(pObj), NULL );
  }
  // construct l_pi
  cerr << "[Info] print LO info of the new ntk " << endl;
  for (i = 0; i < stateNum; ++i){
    sprintf( pVarName, "s%d_o", i );
    // cerr << "create " << i << " l_pi, name: " << pVarName << endl;
    pNode = Io_ReadCreatePi( pNtk, pVarName );
    Abc_ObjAssignName( pNode, pVarName, NULL );
  }
  // construct po
  cerr << "[Info] print PO info of the new ntk " << endl;
  sprintf( pVarName, "f0");
  // cerr << "create po, name: " << pVarName << endl;
  pNode = Io_ReadCreatePo( pNtk, pVarName );
  Abc_ObjAssignName( pNode, pVarName, NULL );

  // construct l_po
  cerr << "[Info] print LI info of the new ntk " << endl;
  for (i = 0; i < stateNum; ++i){
    sprintf( pVarName, "s%d_i", i );
    // cerr << "create " << i << " l_po, name: " << pVarName << endl;
    pNode = Io_ReadCreatePo( pNtk, pVarName );
    Abc_ObjAssignName( pNode, pVarName, NULL );
  }

  // construct all required internal node
  //TODO
  cerr << "[Info] print internal nodes info of the new ntk " << endl;
//   Abc_NtkForEachPi(pNtk, pObj, i){
//       cerr << Abc_ObjName(pObj) << endl;
//   }
  int num, t;
  for( i = 0; i < piNum; ++i ){
    sprintf( pNodeName, "in%d_i", i );
    // cerr << "line70: " << pNodeName << endl;
    num = i;
    if (num & 1)
      sprintf( pVarName, "%s" , Abc_ObjName(Abc_NtkPi(pNtk, piIdEnd - piIdBegin - 1)));
    else
      sprintf( pVarName, "!%s" , Abc_ObjName(Abc_NtkPi(pNtk, piIdEnd - piIdBegin -1 )));
    // cerr << "line77: " << pVarName << endl;
    num = num >> 1;
    strcat (pFormula,pVarName);
    // cerr << "line80: " << pFormula << endl;
    
    for (t = piIdEnd - piIdBegin - 2; t >= 0 ; --t){
      if (num & 1)
        sprintf( pVarName, "*%s" , Abc_ObjName(Abc_NtkPi(pNtk, t)));
      else
        sprintf( pVarName, "*!%s" , Abc_ObjName(Abc_NtkPi(pNtk, t)));
    strcat (pFormula,pVarName);
      num = num >> 1;
    //   cerr << "line88: " << pVarName << endl;
    //   cerr << "line89: " << pFormula << endl;
    }
    cerr << "node " << pNodeName <<  " : " << pFormula << endl;
    Leq_EqnWriteNode(pNodeName, pFormula, vVars, pNtk);
    sprintf( pFormula, "" );
  }

  // construct transition function for each state depending on the sim results
  cerr << "[Info] print LI function info of the new ntk " << endl;
  for ( i = 0; i < stateNum; ++i ){
    sprintf( pNodeName, "s%d_i", i );
    sprintf( pFormula, "0" );
    for ( j = 0; j < piNum; ++j){
      if( !(vNextState[i][j] == NULL) )
        if( !vNextState[i][j]->empty()){
          sprintf( pVarName, "+in%d_i*(" , j);
          strcat( pFormula, pVarName );
          for (auto & id : (*vNextState[i][j])){
            sprintf( pVarName, "s%d_o+" , id);
            strcat (pFormula,pVarName);
          }
          *(pFormula+strlen(pFormula)-1) = ')';
        }
      if(print) cerr << "node " << pNodeName <<  " : " << pFormula << endl;
    }
    Leq_EqnWriteNode(pNodeName, pFormula, vVars, pNtk);
    if(print) cerr << "node " << pNodeName <<  " : " << pFormula << endl;
  }
  // construct f^0
  cerr << "[Info] print po function info of the new ntk " << endl;
  sprintf( pNodeName, "f0");
  sprintf( pFormula, "!0" );
  if(!sNF0.empty()){
    for(auto & id : sNF0){
      sprintf( pVarName, "*!s%d_i" , id);
      strcat (pFormula,pVarName);
    }
  }
  if(!sF0.empty()){
    strcat (pFormula,"*(0");
    for(auto & id : sF0){
      sprintf( pVarName, "+s%d_i" , id);
      strcat (pFormula,pVarName);
    }
    strcat (pFormula,")");
  }
  
  // *(pFormula+strlen(pFormula)-1) = '\0';
  Leq_EqnWriteNode(pNodeName, pFormula, vVars, pNtk);
  if(print) cerr << "node " << pNodeName <<  " : " << pFormula << endl;

  Vec_PtrFree( vVars );
  Abc_NtkFinalizeRead( pNtk );
  free(pNodeName);
  free(pFormula);
  free(pVarName);
  Abc_NtkDelete(pNtkOri);
  return pNtk;
}

void Leq_EqnWriteNode(char * pNodeName, char * pFormula, Vec_Ptr_t * vVars, Abc_Ntk_t * pNtk){
  // get the type of the line
  Abc_Obj_t * pNode;
  char * pFormulaCopy;
  //  make a copy of formula for names
  pFormulaCopy = Extra_UtilStrsav( pFormula );
  // find the names of the fanins of this node
  Io_ReadEqnStrCutAt( pFormulaCopy, "!*+()", 1, vVars );
  // create the node
  pNode = Io_ReadCreateNode( pNtk, pNodeName, (char **)Vec_PtrArray(vVars), Vec_PtrSize(vVars) );
  Abc_ObjAssignName( pNode, pNodeName, NULL );
  // derive the function
  pNode->pData =  Parse_FormulaParserEqn( stdout, pFormula, vVars, (Hop_Man_t *)pNtk->pManFunc );
  // remove the cubes
  ABC_FREE( pFormulaCopy );
}
