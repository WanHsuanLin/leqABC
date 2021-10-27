/**CFile****************************************************************

  FileName    [leqSolving.h]

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [Language equantion solving package.]

  Synopsis    []

  Author      []
  
  Affiliation [NTU]

  Date        [Ver. 1.0. Started - April 10, 2021.]

  Revision    []

***********************************************************************/

#ifndef LEQSOLVING_HPP
#define LEQSOLVING_HPP

#include "base/abc/abc.h"
#include "base/main/main.h"
#include "base/main/mainInt.h"
#include <vector>
#include <iostream>
#include <map>
#include <unordered_set>
#include "proof/fra/fra.h"
#include <unordered_map>
#include <climits>
#include <math.h>
#include "aig/saig/saig.h"
#include <set>
#include "proof/pdr/pdr.h"
#include "proof/fraig/fraig.h"
#include "proof/int/int.h"


using namespace std;

// #define MINISAT
// #define SHOW
// #define DEBUG
extern "C" {
  Aig_Man_t * Abc_NtkToDar( Abc_Ntk_t * pNtk, int fExors, int fRegisters );
  Abc_Ntk_t * Abc_NtkDarFraig( Abc_Ntk_t * pNtk, int nConfLimit, int fDoSparse, int fProve, int fTransfer, int fSpeculate, int fChoicing, int fVerbose );
  void        Abc_NtkShow( Abc_Ntk_t * pNtk, int fGateNames, int fSeq, int fUseReverse );
  Abc_Ntk_t * Abc_NtkDarSeqSweep( Abc_Ntk_t * pNtk, Fra_Ssw_t * pPars );
  void        Fra_SmlInitializeGiven( Fra_Sml_t * p, Vec_Str_t * vSimInfo );
  void        Fra_SmlSimulateOne( Fra_Sml_t * p );
  void        Fra_SmlPrintOutputs( Fra_Sml_t * p, int nPatterns );
  void        Io_ReadEqnStrCutAt( char * pStr, char * pStop, int fUniqueOnly, Vec_Ptr_t * vTokens );
  Hop_Obj_t * Parse_FormulaParserEqn( FILE * pOutput, char * pFormInit, Vec_Ptr_t * vVarNames, Hop_Man_t * pMan );
  Abc_Ntk_t * Abc_NtkDC2( Abc_Ntk_t * pNtk, int fBalance, int fUpdateLevel, int fFanout, int fPower, int fVerbose );
  void        Abc_NtkMakeSeq( Abc_Ntk_t * pNtk, int nLatchesToAdd );
  Abc_Ntk_t * Abc_NtkDarLcorr( Abc_Ntk_t * pNtk, int nFramesP, int nConfMax, int fVerbose );
  Abc_Ntk_t * Abc_NtkDarLcorrNew( Abc_Ntk_t * pNtk, int nVarsMax, int nConfMax, int fVerbose );
  void        Abc_NtkMiterPrepare( Abc_Ntk_t * pNtk1, Abc_Ntk_t * pNtk2, Abc_Ntk_t * pNtkMiter, int fComb, int nPartSize, int fMulti );
  void        Abc_NtkMiterAddOne( Abc_Ntk_t * pNtk, Abc_Ntk_t * pNtkMiter );
  int         Abc_NtkQuantify( Abc_Ntk_t * pNtk, int fUniv, int iVar, int fVerbose );
  int         Abc_NtkDarCec( Abc_Ntk_t * pNtk1, Abc_Ntk_t * pNtk2, int nConfLimit, int fPartition, int fVerbose );
  Abc_Ntk_t * Abc_NtkMakeOnePo( Abc_Ntk_t * pNtk, int Output, int nRange );
  int         Abc_NtkDarSec( Abc_Ntk_t * pNtk1, Abc_Ntk_t * pNtk2, Fra_Sec_t * p );
  // int         Abc_NtkComparePis( Abc_Ntk_t * pNtk1, Abc_Ntk_t * pNtk2, int fComb );
  void        Io_WriteVerilog( Abc_Ntk_t * pNtk, char * FileName, int fOnlyAnds );
  Abc_Ntk_t * Io_Read( char * pFileName, Io_FileType_t FileType, int fCheck, int fBarBufs );
  Abc_Ntk_t * Abc_NtkMakeOnePo( Abc_Ntk_t * pNtk, int Output, int nRange );
  void        Abc_NtkQbf( Abc_Ntk_t * pNtk, int nPars, int nIters, int fDumpCnf, int fVerbose );
  Abc_Ntk_t * Abc_NtkDarCleanupAig( Abc_Ntk_t * pNtk, int fCleanupPis, int fCleanupPos, int fVerbose );
  void        Fra_SecSetDefaultParams( Fra_Sec_t * p );
  void        Abc_NtkVerifyReportErrorSeq( Abc_Ntk_t * pNtk1, Abc_Ntk_t * pNtk2, int * pModel, int nFrames );
  int         Abc_NtkDarPdr( Abc_Ntk_t * pNtk, Pdr_Par_t * pPars );
  int         Abc_NtkDarBmcInter( Abc_Ntk_t * pNtk, Inter_ManParams_t * pPars, Abc_Ntk_t ** ppNtkRes );
  int         Abc_NtkDarProve( Abc_Ntk_t * pNtk, Fra_Sec_t * pSecPar, int nBmcFramesMax, int nBmcConfMax );
}

typedef struct Leq_Ba_t_       Leq_Ba_t;

struct Leq_Ba_t_
{
  char *                                  _pName;
  unsigned                                _nStates;        //number of states(Latches)
  unsigned                                _nInput;         //input alphabet of the pNtk // use to remember the origin input. It won't be modified once the ntk is given
  unsigned                                _bNondeterType;  // 0 for existential, 1 for universal
  Abc_Ntk_t *                             _pNtk;
  vector<bool>                            _vIniVal;        //final states
  unordered_map<string, Abc_Obj_t *>      _mActInput;         //current support (Name to ID)
  unordered_set<string>                   _sDcInput;         //current support (Name to ID)
  unordered_set<Abc_Obj_t *>              _sDcObj;         //current support (Name to ID)
  unordered_map<string, Abc_Obj_t *>      _mInactInput;       //removed support
};

/****************************************/
// function related to boolean automata //
/****************************************/
void                      Leq_BaInit(Leq_Ba_t* pBa, Abc_Ntk_t * pNtk, bool bS, bool bConvert);
static inline void        Leq_BaInitTranCir(Leq_Ba_t * pBa, Abc_Ntk_t * pNtkOri);
static inline char *      Leq_BaName(Leq_Ba_t * pBa) {return pBa->_pName;};
static inline void        Leq_BaSetName(Leq_Ba_t * pBa, char * pName) {pBa->_pName = pName;};
static inline unsigned    Leq_BaNumState(Leq_Ba_t * pBa) {return pBa->_nStates;};
static inline void        Leq_BaSetNumState(Leq_Ba_t * pBa, unsigned n) {pBa->_nStates = n;};
static inline void        Leq_BaIncNumState(Leq_Ba_t * pBa) {++pBa->_nStates;};
static inline unsigned    Leq_BaNumInput(Leq_Ba_t * pBa) {return pBa->_nInput;}; 
static inline void        Leq_BaSetNumInput(Leq_Ba_t * pBa, unsigned n) { pBa->_nInput = n;}; 
static inline bool        Leq_BaNondeterType(Leq_Ba_t * pBa) {return pBa->_bNondeterType;}; 
static inline void        Leq_BaSetNondeterType(Leq_Ba_t * pBa, bool b) {pBa->_bNondeterType = b;}; 
static inline Abc_Ntk_t * Leq_BaTransCir(Leq_Ba_t * pBa) {return pBa->_pNtk;};
static inline unsigned    Leq_BaNumActInput(Leq_Ba_t * pBa) {return pBa->_mActInput.size();};
static inline unsigned    Leq_BaNumInactInput(Leq_Ba_t * pBa) {return pBa->_mInactInput.size();};
void                      Leq_BaSetTransCir(Leq_Ba_t * pBa, Abc_Ntk_t * pNtk);
void                      Leq_BaPrintInitialState(Leq_Ba_t * pBa);
void                      Leq_BaPrintSupport(Leq_Ba_t * pBa);
void                      Leq_Aba2Ba(Leq_Ba_t * pBa); // ! scability problem
void                      Leq_Aba2Ba2(Leq_Ba_t * pBa); // ! scability problem
void                      Leq_BaGetNextState(const int & stateNum, const int & piNum, const int & nChoiceNum, vector<vector<set<unsigned>* > > & vSimResult, vector<vector<set<unsigned>* > > & vNextState);
void                      Leq_BaResetByTranCir( Leq_Ba_t * pBa);
void                      Leq_BaModifyCirPiOrder(Leq_Ba_t * pBa);
void                      Leq_BaModifyCirPiOrderU(Leq_Ba_t * pBa, unordered_set<string> & sU);
Leq_Ba_t *                Leq_BaDuplicate(Leq_Ba_t * pBa);
Abc_Ntk_t *               Leq_XorBaTransCir(Abc_Ntk_t * pNtkS, Abc_Ntk_t * pNtkF, bool bUseInDirectlyVerify);

/**********************/
// utilility function //
/**********************/
void        Leq_WriteBA(Leq_Ba_t * pBa);
void        Leq_Dec2Bin(int n, char * buffer, int size);
void        Leq_Dec2BinChar(int n, char * buffer, int size);
void        Leq_Dec2BinInt(int n, vector<bool> & vResult);
unsigned    Leq_Bin2Dec(vector<bool> & vResult);
void        Leq_PrintNtkPo(Abc_Ntk_t * pNtk);
void        Leq_PrintNtkPi(Abc_Ntk_t * pNtk);
void        Leq_PrintNtkLatchValue(Abc_Ntk_t * pNtk);
// void        Leq_NtkMakeComb( Abc_Ntk_t * pNtk, bool fRemovef0 );
void        Leq_NtkMakeSeq( Abc_Ntk_t * pNtk, vector<bool> vVal);
void        Leq_OptimizeLatch(Abc_Ntk_t * pNtk, vector<bool> & vVal );
Abc_Ntk_t * Leq_Const0Cir(Leq_Ba_t * pBa);
Abc_Obj_t * Leq_SetVectorOr( Abc_Aig_t * pMan, Abc_Obj_t ** ppObjs, int nObjs );
Abc_Obj_t * Leq_SetVectorAnd( Abc_Aig_t * pMan, Abc_Obj_t ** ppObjs, int nObjs );
Aig_Obj_t * Leq_SetVectorAigAnd( Aig_Man_t * pMan, Aig_Obj_t ** ppObjs, int nObjs );
// write verilog
void        Leq_writeVerilog(Abc_Ntk_t * pNtk, char * pFileName, bool complement);


/****************/
// eqn function //
/****************/
void        Leq_EqnWriteNode(char * pNodeName, char * pFormula, Vec_Ptr_t * vVars, Abc_Ntk_t * pNtk);
Abc_Ntk_t * Leq_EqnWriteNetwork( Abc_Ntk_t * pNtkOri, Leq_Ba_t * pBa, vector<vector<set<unsigned> *> > & vNextState, set<unsigned> & sF0, set<unsigned> & sNF0 );

/***********************/
// simulation function //
/***********************/
void        Leq_GenSimPattern(Leq_Ba_t * pBa, const unsigned & stateId, const unsigned & piNum, const unsigned & nChoiceNum, Vec_Str_t * vSimPattern);
Fra_Sml_t * Leq_SimComb( Aig_Man_t * pAig, Vec_Str_t * vSimInfo );
bool        Leq_InproGetSimOutputs( Fra_Sml_t * p, const int & nChoiceNum, unordered_set<unsigned> & sSimResult);
void        Leq_GetSimOutputs( Fra_Sml_t * p, const int & nChoiceNum, unordered_set<unsigned> & sSimResult, set<unsigned> & sF0, set<unsigned> &sNF0);
void        Leq_SimCombNew(Vec_Str_t * vSimInfo, Fra_Sml_t * p);
Fra_Sml_t * Leq_SimStart( Aig_Man_t * pAig, int nPref, int nFrames, int nWordsFrame );

/***********/
// command //
/***********/
int Leq_CommandSc2Ba(Abc_Frame_t* pAbc, int argc, char** argv);
int Leq_CommandSolve(Abc_Frame_t* pAbc, int argc, char** argv);
int Leq_CommandLatchSplit(Abc_Frame_t* pAbc, int argc, char** argv);
int Leq_CommandSupport(Abc_Frame_t* pAbc, int argc, char** argv);
int Leq_CommandInputProgress(Abc_Frame_t* pAbc, int argc, char** argv);
int Leq_CommandLatchExpose(Abc_Frame_t* pAbc, int argc, char** argv);


/*******************************************/
// function related to language operatrion //
/*******************************************/
Leq_Ba_t *  Leq_Solve(Leq_Ba_t* pBaS, Leq_Ba_t* pBaF, unordered_set<string> &sU, unordered_set<string> &sAllInput, bool bDeterminize, bool bIsDeter, int qSolver);
Abc_Ntk_t * Leq_ProductBaTransCir(Abc_Ntk_t * pNtkS, Abc_Ntk_t * pNtkF, bool bUseInDirectlyVerify = 0);
void        Leq_Support(Leq_Ba_t* pBa, unordered_set<string> sSupport);
void        Leq_Complement(Leq_Ba_t * pBa, bool bIsDeter);
void        Leq_PrefixClose(Leq_Ba_t * pBa);
// int Leq_ConvertBa2Nfa(Leq_Ba_t & baX);


/*******************************************/
//            Input Progressive            //
/*******************************************/
void        Leq_InputProgressive(Leq_Ba_t* pBa, unordered_set<string> & sU, bool bDeterminize);
void        Leq_InProModifyNtkPO(const unsigned & cs, Abc_Ntk_t * pNtk);
void        Leq_InProModifyAigPO(const unsigned & cs, Aig_Man_t * pMan);
// bool        Leq_InProDeletState(const unsigned & initS, const unsigned & delS, unordered_map<unsigned, vector<unordered_set<unsigned> *> > & sRS, Abc_Ntk_t * pNtk, Aig_Man_t * pMan);
bool        Leq_InProDeletState(const unsigned & initS, const unsigned & delS, unordered_map<unsigned, vector<unordered_map<unsigned, unordered_set<unsigned> *> *> > & sRS, Abc_Ntk_t * pNtk, Aig_Man_t * pMan);
void        Leq_InproDeterminize(Leq_Ba_t * pBa,map<unsigned,unsigned> & mOldState2NewState);

/*******************************************/
//            Input Progressive2           //
/*******************************************/
void        Leq_InputProgressive2(Leq_Ba_t* pBa, unordered_set<string> & sU, int qSolver);
void        Leq_EliminateVar(Abc_Ntk_t * &pNtk, unsigned uBegin, Leq_Ba_t * pBa);
void        Leq_EliminateVar1(Abc_Ntk_t * &pNtk, unsigned uBegin, Leq_Ba_t * pBa);
Abc_Ntk_t * Leq_NtkQuantify( Abc_Ntk_t * pNtk, bool type, int pi, bool verbose );
void        Leq_AddQuantifyCirPrepare( Abc_Ntk_t * pNtk, Abc_Ntk_t * pNtkQ, Abc_Ntk_t * pNtkMiter, int qPi, bool type );
void        Leq_AddQuantifyCirFinalize( Abc_Ntk_t * pNtk, Abc_Ntk_t * pNtkQ, Abc_Ntk_t * pNtkNew, int qPi);
Abc_Ntk_t * Leq_AddCheckCir( Abc_Ntk_t * pNtk, Abc_Ntk_t * pNtkCheck );
void        Leq_AddCheckCirFinalize( Abc_Ntk_t * pNtk1, Abc_Ntk_t * pNtk2, Abc_Ntk_t * pNtkAnd);
void        Leq_AddCheckCirPrepare( Abc_Ntk_t * pNtk1, Abc_Ntk_t * pNtkMiter);
Abc_Ntk_t * Leq_AddCheckCirInt( Abc_Ntk_t * pNtk1, Abc_Ntk_t * pNtk2);
int         Leq_CheckEquivalent(Abc_Ntk_t * pNtk1, Abc_Ntk_t * & pNtk2);

/*******************************************/
//     Eliminate Quantifier by Manthan     //
/*******************************************/
// void        Leq_EliminateVarManthan(Abc_Ntk_t * &pNtk, unsigned uBegin, Leq_Ba_t * pBa);
// Abc_Ntk_t * Leq_NtkPutOnTop( Abc_Ntk_t * pNtk, Abc_Ntk_t * pNtk2, int latchBegin );
// void        Leq_RunManthan(string vFileName, string varFileName);
// void        Leq_RunManthan(char* vFileName, char* varFileName);
// void        Leq_EliminateVarCadet(Abc_Ntk_t * &pNtk, unsigned uBegin, Leq_Ba_t * pBa);
// Abc_Ntk_t * Leq_makeOnePo(Abc_Ntk_t * pNtk);

/*******************************************/
//                verify                   //
/*******************************************/
int         Leq_VerifyFXS(Leq_Ba_t* pBaS, Leq_Ba_t* pBaF, Leq_Ba_t* pBaX, unordered_set<string> &sU, unordered_set<string> &sAllInput);
int         Leq_VerifyX(Abc_Ntk_t* pNtkXv, Leq_Ba_t* pBaX, int fUseCommand);
int         Leq_LanguageContain(Abc_Ntk_t * pBa1, Abc_Ntk_t * pBa2);
int         Leq_NtkDarSec( Abc_Ntk_t * pMiter, Fra_Sec_t * pSecPar );
Abc_Ntk_t * Leq_NtkMiter( Abc_Ntk_t * pNtk1, Abc_Ntk_t * pNtk2 );
Abc_Ntk_t * Leq_NtkMiterInt( Abc_Ntk_t * pNtk1, Abc_Ntk_t * pNtk2 );
void        Leq_NtkMiterFinalize( Abc_Ntk_t * pNtk1, Abc_Ntk_t * pNtk2, Abc_Ntk_t * pNtkMiter);
Abc_Obj_t * Leq_AigMiter( Abc_Aig_t * pMan, Vec_Ptr_t * vPairs, int fImplic );
int         Leq_VerifyEqX(Abc_Ntk_t* pNtkXv, Leq_Ba_t* pBaX, int fUseCommand);
// int         Leq_XBalm( Abc_Ntk_t * pNtkX, Abc_Ntk_t * pNtkB, int fUseCommand );
// Abc_Ntk_t * Leq_XorTransCir(Abc_Ntk_t * pNtkX, Abc_Ntk_t * pNtkB);


/*******************************************/
//                util                   //
/*******************************************/
// memory
extern double     getPeakMemoryUsage();
extern double     getCurrMemoryUsage();
extern void       showMemoryUsage(bool flag = true);


#endif