#include "leqSolving/leqSolving.hpp"
// #include "leqSolving/ba/ba.hpp"

void init(Abc_Frame_t* pAbc) {
  Cmd_CommandAdd(pAbc, "LEQSolving", "ba", Leq_CommandSc2Ba, 0);
  Cmd_CommandAdd(pAbc, "LEQSolving", "solve", Leq_CommandSolve, 0);
  Cmd_CommandAdd(pAbc, "LEQSolving", "spt", Leq_CommandSupport, 0);
  Cmd_CommandAdd(pAbc, "LEQSolving", "inpro", Leq_CommandInputProgress, 0);
}

void destroy(Abc_Frame_t* pAbc) {};

Abc_FrameInitializer_t frame_initializer = {init, destroy};

struct PackageRegistrationManager {
  PackageRegistrationManager() { Abc_FrameAddInitializer(&frame_initializer); }
} lsvPackageRegistrationManager;

int Leq_CommandSc2Ba(Abc_Frame_t* pAbc, int argc, char** argv) {
  Abc_Ntk_t* pNtk = Abc_FrameReadNtk(pAbc);
  int c;

  //convert sequential circuit to ba
  Leq_Ba_t * pBa1 = new Leq_Ba_t();

  Extra_UtilGetoptReset();
  while ((c = Extra_UtilGetopt(argc, argv, "h")) != EOF) {
    switch (c) {
      case 'h':
        goto usage;
      default:
        goto usage;
    }
  }
  if (!pNtk) {
    Abc_Print(-1, "Empty network.\n");
    return 1;
  }
  Leq_BaInit(pBa1, pNtk, 0 ,1);
  
  return 0;

usage:
  Abc_Print(-2, "usage: leq_sc2ba [-h]\n");
  Abc_Print(-2, "\t        converts sequential circuit to Boolean automata\n");
  Abc_Print(-2, "\t-h    : print the command usage\n");
  return 1;
}

int Leq_CommandSolve(Abc_Frame_t* pAbc, int argc, char** argv) {
  abctime clk;
  Abc_Ntk_t* pNtkS, * pNtkSTmp, * pNtkF, * pNtkFTmp, * pNtkXv, * pNtkXd;
  Abc_Ntk_t* pNtkTemp;
  int fDelete1, fDelete2;
  int c;
  Leq_Ba_t* pBaX = new Leq_Ba_t();
  Leq_Ba_t* pBaXv = new Leq_Ba_t();
  Leq_Ba_t* pBaS = new Leq_Ba_t();
  Leq_Ba_t* pBaSv = new Leq_Ba_t();
  Leq_Ba_t* pBaF = new Leq_Ba_t();
  Leq_Ba_t* pBaFv = new Leq_Ba_t();

  char * pNamesLatch;
  char * pTemp;
  int nParts;

  char ** pArgvNew;
  int nArgcNew;
  bool bVerify = 0;
  bool bDirectVerify = 0;
  bool bDirectVerifyDeter = 0;
  int qSolver = 0;
  bool fFileUV = 0;
  bool bOutput = 0;
  bool bOpt = 0;
  int fUseCommand = 0;

  // nInput = Abc_NtkPiNum(pNtkF)+Abc_NtkPoNum(pNtkF);
  unordered_set<string> sU;
  unordered_set<string> sAllInput;
  unordered_set<string> sProject;
  unordered_set<string> sAll;
  bool bIsDeter = 1;
    
  Fra_Ssw_t Pars, * pPars = &Pars;
  pPars->nPartSize  = 0;
  pPars->nOverSize  = 0;
  pPars->nFramesP   = 0;
  pPars->nFramesK   = 1;
  pPars->nMaxImps   = 5000;
  pPars->nMaxLevs   = 0;
  pPars->fUseImps   = 0;
  pPars->fRewrite   = 0;
  pPars->fFraiging  = 0;
  pPars->fLatchCorr = 0;
  pPars->fWriteImps = 0;
  pPars->fUse1Hot   = 0;
  pPars->fVerbose   = 0;
  pPars->TimeLimit  = 0;

  Extra_UtilGetoptReset();
  while ((c = Extra_UtilGetopt(argc, argv, "fhvcidzow")) != EOF) {
    switch (c) {
      case 'h':
        goto usage;
      case 'f':
        fFileUV ^= 1;
        break;
      case 'o':
        bOutput ^= 1;
        break;
      case 'w':
        bOpt ^= 1;
        break;
      case 'z':
        bDirectVerifyDeter ^= 1;
        pNtkXd = Io_Read( argv[globalUtilOptind], Io_ReadFileType(argv[globalUtilOptind]), 1, 0 );
        ++globalUtilOptind;
        break;
      case 'c':
        qSolver = atoi(argv[globalUtilOptind]);
        globalUtilOptind++;
        break;
      case 'i':
        fUseCommand = atoi(argv[globalUtilOptind]);
        globalUtilOptind++;
        break;
      case 'v':
        bVerify ^= 1;
        pNtkXv = Io_Read( argv[globalUtilOptind], Io_ReadFileType(argv[globalUtilOptind]), 1, 0 );
        ++globalUtilOptind;
        break;
      case 'd':
        bDirectVerify ^= 1;
        pNtkXd = Io_Read( argv[globalUtilOptind], Io_ReadFileType(argv[globalUtilOptind]), 1, 0 );
        ++globalUtilOptind;
        break;
      default:
        goto usage;
    }
  }
  // cerr << "read U" << endl;

  pArgvNew = argv + globalUtilOptind;
  nArgcNew = argc - globalUtilOptind;

  pNamesLatch = pArgvNew[0];

  // read U
  if(fFileUV){
    FILE *fp;
    char pStringU[2000];
    printf( "reading variables %s.\n", pNamesLatch );
    // getchar();
    fp = fopen(pNamesLatch, "r");  
    if (fp == NULL)  {  
      printf("File does not exists \n");  
      return 0;  
    }  
    fscanf(fp, "%s", pStringU);
    printf("%s ", pStringU );  
    // getchar();
    fclose(fp);  
    pNamesLatch = &pStringU[0];
  }
  pTemp  = strtok( pNamesLatch, "," );
  for ( nParts = 0; pTemp; nParts++ ){
    // cerr << "pTemp = " << pTemp << endl;
    string str(pTemp);
    sU.emplace(string(str));
    sAllInput.emplace(string(str));
    pTemp = strtok( NULL, "," );
  }
  
  globalUtilOptind++;
  pArgvNew = argv + globalUtilOptind;
  nArgcNew = argc - globalUtilOptind;

  pNamesLatch = pArgvNew[0];

  
  // read V
  if(fFileUV){
    FILE *fp;
    char pStringV[2000];
    printf( "reading variables %s.\n", pNamesLatch );
    // getchar();
    fp = fopen(pNamesLatch, "r");  
    if (fp == NULL)  {  
      printf("File does not exists \n");  
      return 0;  
    }  
    fscanf(fp, "%s", pStringV);
    printf("%s ", pStringV );  
    // getchar();
    fclose(fp);  
    pNamesLatch = &pStringV[0];
  }
  // cerr << "read v" << endl;
  pTemp  = strtok( pNamesLatch, "," );
  for ( nParts = 0; pTemp; nParts++ ){
    // cerr << "pTemp = " << pTemp << endl;
    string str(pTemp);
    sAllInput.emplace(string(str));
    pTemp = strtok( NULL, "," );
    assert(sU.count(string(str))==0);
  }
  // cerr << "nParts = " << nParts << endl;
  // cerr << "sAllInput.size() = " << sAllInput.size() << endl;

  // read two circuit S and F
  globalUtilOptind++;
  pArgvNew = argv + globalUtilOptind;
  nArgcNew = argc - globalUtilOptind;
  // cerr << "nArgcNew : " << nArgcNew << endl;
  if(bOpt){
    Abc_NtkPrepareTwoNtks( stdout, NULL, pArgvNew, nArgcNew, &pNtkSTmp, &pNtkFTmp, &fDelete1, &fDelete2, 1 );
    pNtkSTmp = Abc_NtkDarLatchSweep( pNtkSTmp, 1, 1, 1, 0, 1, 512, 1, 1 );
    pNtkFTmp = Abc_NtkDarLatchSweep( pNtkFTmp, 1, 1, 1, 0, 1, 512, 1, 1 );
    pNtkF = Abc_NtkDarSeqSweep( pNtkFTmp, pPars );
    pNtkS = Abc_NtkDarSeqSweep( pNtkSTmp, pPars );
  }
  else
    Abc_NtkPrepareTwoNtks( stdout, NULL, pArgvNew, nArgcNew, &pNtkS, &pNtkF, &fDelete1, &fDelete2, 1 );
  // cerr << "pi num of pNtkF" << Abc_NtkPiNum(pNtkF) << endl; 
  Abc_NtkDelete(pNtkFTmp);
  Abc_NtkDelete(pNtkSTmp);
  
  
  if(bVerify){
    Abc_Obj_t * pObj;
    int i;

    Abc_NtkForEachPi(pNtkS, pObj, i){
      sProject.emplace(Abc_ObjName(pObj));
      if(sU.count(string(Abc_ObjName(pObj))) == 0){
        bIsDeter = 0;
      }
    }
    Abc_NtkForEachPo(pNtkS, pObj, i){
      sProject.emplace(Abc_ObjName(pObj));
    }
    Abc_NtkForEachPi(pNtkF, pObj, i){
      sAll.emplace(Abc_ObjName(pObj));
    }
    Abc_NtkForEachPo(pNtkF, pObj, i){
      sAll.emplace(Abc_ObjName(pObj));
    }
  }

  // pNtkS = Abc_NtkDarLcorrNew( pNtkS, 1000, 1000, 1 );  
  // Abc_NtkShow( pNtkS, 0, 0, 1 );
  // pNtkF = Abc_NtkDarLcorrNew( pNtkF, 1000, 1000, 1 );  
  // Abc_NtkShow( pNtkF, 0, 0, 1 );
  // getchar();
  cerr << "---------start baS construction---------" << endl;
  Leq_BaInit(pBaS, pNtkS, 1, 1);
  cerr << "---------end baS construction---------" << endl;
  cerr << "---------start baF construction---------" << endl;
  // Abc_NtkShow( Leq_BaTransCir(pBaS), 0, 0, 1 );
  // getchar();
  Leq_BaInit(pBaF, pNtkF, 0, 1);
  cerr << "---------end baF construction---------" << endl;

  cerr << "---------start solving---------" << endl;
  pBaX = Leq_Solve( pBaS, pBaF, sU, sAllInput, bVerify, bIsDeter, qSolver);
  // Leq_Aba2Ba(pBaX);
  // pNtkTemp = Abc_NtkToNetlist( Leq_BaTransCir(pBaX) );
  // Abc_NtkToAig( pNtkTemp );
  // Io_WriteBenchLut( pNtkTemp, "inpro2.blif" );
  // Leq_Aba2Ba(pBaX);
  // Leq_Aba2Ba(pBaX);
  // Io_Write( Leq_BaTransCir(pBaX), "baX.blif", IO_FILE_BLIF );
  // return 0;
  delete(pBaS);
  delete(pBaF);
  // !
  if(bDirectVerifyDeter){
    Leq_Aba2Ba2(pBaX);
    if ( !Abc_NtkIsStrash(pNtkXd) ){
      pNtkXd = Abc_NtkStrash( pNtkXd, 0, 1, 0 );
    }
    Abc_NtkPrintStats( pNtkXd, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
    if(Leq_VerifyEqX(pNtkXd, pBaX, fUseCommand)){
      cerr << "equivalence to the solution in BALMII" << endl;
    }
  }
  // !
  cerr << "---------end solving---------" << endl;
  if(bOutput) Io_Write( Leq_BaTransCir(pBaX), "rABAX.blif", IO_FILE_BLIF );
  clk = Abc_Clock();
  if(bDirectVerify){
    if ( !Abc_NtkIsStrash(pNtkXd) ){
      pNtkXd = Abc_NtkStrash( pNtkXd, 0, 1, 0 );
    }
    Abc_NtkPrintStats( pNtkXd, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
    if(Leq_VerifyX(pNtkXd, pBaX, fUseCommand)){
      cerr << "equivalence to the solution in BALMII" << endl;
    }
  }
  if(bVerify){
    // rename nchoice
    // Leq_BaRenameInActiveInput(pBaX);
    Leq_BaInit(pBaSv, pNtkS, 1, 1);
    Leq_BaInit(pBaFv, pNtkF, 0, 1);
    Leq_BaInit(pBaXv, pNtkXv, 0, 1);
    cerr << "---------start verify---------" << endl;
    if(bIsDeter){
      cerr << "I is in U. pBaX is deterministic" << endl;
      // Leq_RenameNondeterVariable(pBaX);
    }
    else{
      Leq_Aba2Ba2(pBaX);
    }
    // cerr << "show pBaXv" << endl;
    // Leq_BaPrintInitialState(pBaXv);
    // Leq_PrintNtkLatchValue(pBaXv -> _pNtk);
    // Abc_NtkShow( Leq_BaTransCir(pBaXv), 0, 0, 1 );
    // getchar();
    // cerr << "show pBaX" << endl;
    // Leq_BaPrintInitialState(pBaX);
    // Leq_PrintNtkLatchValue(pBaX -> _pNtk);
    // Abc_NtkShow( Leq_BaTransCir(pBaX), 0, 0, 1 );
    // getchar();
    clk = Abc_Clock();
    if(Leq_VerifyX(Leq_BaTransCir(pBaXv), pBaX, fUseCommand) && Leq_VerifyFXS(pBaSv, pBaFv, pBaX, sProject, sAll))
    // if(Leq_VerifyFXS(pBaSv, pBaFv, pBaX, sProject, sAll))
    {
      cerr << "X is a valid solution" << endl;
    }
    else{
      cerr << "X is NOT a valid solution" <<  endl;
    }
    cerr << "---------end solving---------" << endl;
  }
  cerr << "Runtime for verification progress : " << (double)(Abc_Clock() - clk )/(double)CLOCKS_PER_SEC << endl;
    // Abc_Print(-2, "fail to solve X\n");
   
  
  // write ba
  // if(bWriteNfa)
    // baX.writeBA();
    // Leq_ConvertBa2Nfa(baX);
  
  showMemoryUsage();

  return 0;

usage:
  Abc_Print(-2, "usage: [-i num] [-d file] [-v file] [-z file] [-fhocw] <u> <v> <baS> <baF>\n");
  Abc_Print(-2, "\t        solves langage equation X given F and S. \n");
  Abc_Print(-2, "\t        enter u, v with each input separated by ','\n");
  Abc_Print(-2, "\t-o     : write the resulted sequential circuit representing the solution rABA is written in rABAX.blif\n");
  Abc_Print(-2, "\t         with the pi not in <u> <v> being the psuedo inputs for rABA\n");
  Abc_Print(-2, "\t-v file: verify \n");
  Abc_Print(-2, "\t-w     : perform optimization for S and F \n");
  Abc_Print(-2, "\t-f     : U V are given in files  \n");
  // Abc_Print(-2, "\t-c num : quantifier elimination. 0 for qvar, 1 for cadet  \n");
  Abc_Print(-2, "\t-i num : verify command for R \subset X. 0 for dsec, 1 for pdr, 2 for dprove, 3 for int\n");
  Abc_Print(-2, "\t-d file: directly verify our solution with the solution of balmII\n");
  Abc_Print(-2, "\t-z file: directly verify our deterministic solution with the solution of balmII\n");
  Abc_Print(-2, "\t-h     : print the command usage\n");
  return 1;
}


int Leq_CommandSupport(Abc_Frame_t* pAbc, int argc, char** argv) {
  Abc_Ntk_t* pNtk = Abc_FrameReadNtk(pAbc);
  
  int c;
  char * pNamesSupport;
  char * pTemp;
  int nParts;
  bool bDeterminize = 0;
  char ** pArgvNew;
  int nArgcNew;
  unordered_set<string> sSupport;

  Leq_Ba_t * pBa = new Leq_Ba_t();;

  Extra_UtilGetoptReset();

  while ((c = Extra_UtilGetopt(argc, argv, "hd")) != EOF) {
    switch (c) {
      case 'h':
        goto usage;
        break;
      case 'd':
        bDeterminize ^= 1;
        // globalUtilOptind++;
        break;
      default:
        goto usage;
    }
  }
  pArgvNew = argv + globalUtilOptind;
  nArgcNew = argc - globalUtilOptind;

  pNamesSupport = pArgvNew[0];

  if (!pNtk) {
    Abc_Print(-1, "Empty network.\n");
    return 1;
  }
  Leq_BaInit(pBa, pNtk, 0 ,1);

  // read Support
  pTemp  = strtok( pNamesSupport, "," );
  for ( nParts = 0; pTemp; nParts++ ){
    // cerr << "=======";
    // cerr << pTemp ;
    // cerr << "+++++++ ";
    string str(pTemp);
    cerr << str << endl;
    sSupport.emplace(str);  
    pTemp = strtok( NULL, "," );
  }
  Leq_Support(pBa, sSupport);
  if(bDeterminize){
    Leq_Aba2Ba(pBa);
  }
  
  return 0;

usage:
  Abc_Print(-2, "usage: convert the cuurent sequential circuit to BA and modify support of ba.\n");
  Abc_Print(-2, "\t     Please first read a network. This command will covert network to BA[-h]\n");
  Abc_Print(-2, "\t     prints the nodes in the network\n");
  Abc_Print(-2, "\t-h   : print the command usage\n");
  Abc_Print(-2, "\t-d   : determinize the ba after support operation [default = %s]\n", bDeterminize? "yes": "no");
  return 1;
}

int Leq_CommandInputProgress(Abc_Frame_t* pAbc, int argc, char** argv) {
  Abc_Ntk_t* pNtk = Abc_FrameReadNtk(pAbc);
  
  int c;
  char * pNamesU;
  char * pTemp;
  int nParts;
  char ** pArgvNew;
  int nArgcNew;
  bool bDeterminize = 0;
  unordered_set<string> sU;
  unordered_set<string>::iterator itDc;
  Leq_Ba_t * pBa = new Leq_Ba_t();

  Extra_UtilGetoptReset();

  while ((c = Extra_UtilGetopt(argc, argv, "hd")) != EOF) {
    switch (c) {
      case 'h':
        goto usage;
        break;
      case 'd':
        bDeterminize ^= 1;
        // globalUtilOptind++;
        break;
      default:
        goto usage;
    }
  }
  pArgvNew = argv + globalUtilOptind;
  nArgcNew = argc - globalUtilOptind;

  pNamesU = pArgvNew[0];

  if (!pNtk) {
    Abc_Print(-1, "Empty network.\n");
    return 1;
  }
  Leq_BaInit(pBa, pNtk, 0 ,1);

  // read u
  pTemp  = strtok( pNamesU, "," );
  for ( nParts = 0; pTemp; nParts++ ){
    itDc = pBa->_sDcInput.find(string(pTemp));
    if(itDc == pBa->_sDcInput.end()){
      string str(pTemp);
      sU.emplace(str);  
    }
    pTemp = strtok( NULL, "," );
  }

  Leq_InputProgressive(pBa, sU, bDeterminize);
  
  
  return 0;

usage:
  Abc_Print(-2, "usage: input progressive for ba.\n");
  Abc_Print(-2, "\t     Please first read a network. This command will covert network to BA[-h]\n");
  Abc_Print(-2, "\t     prints the nodes in the network\n");
  Abc_Print(-2, "\t-h   : print the command usage\n");
  Abc_Print(-2, "\t-d   : determinize the ba after operation [default = %s]\n", bDeterminize? "yes": "no");
  return 1;
}


