#include <sys/ipc.h>
#include <stdio.h>
#include <sys/shm.h>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <sstream>
#include <sys/wait.h>
#include <time.h>
#include <chrono>
#include <ctime>
#include <math.h>
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::system_clock;
using namespace std;

int main(){
  bool deadlock;
  int showmode;
  int ID0, ID;
  int Nfilo;
  int ct, ctt, ctu, accu, atte, atted, Nturns;
  int semID, resID;
  int wait_Flag;
  char *resptr;
  int BUFFERSIZE, SEMAPHSIZE;
  char yon_cycldep;
  ////////////////////
  showmode=4;

  std::cout << "Indicare il numero intero di filosofi: ";
  std::cin >> Nfilo;
  std::cout << std::endl;

  std::cout << "Indicare il numero di turni: ";
  std::cin >> Nturns;
  std::cout << std::endl;

  std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  std::cout << "Indicare se si vuole applicare la dipendenza ciclica (Y/N): ";
  yon_cycldep = std::cin.get();
  if (yon_cycldep == 'Y') deadlock=true;
  else deadlock=false;

  int ReadDelay=5e5;	// microseconds
  int ProcDelay=1e4;	// microseconds
  int ForkDelay=1e3;	// cycles
  ////////////////////

  BUFFERSIZE=(Nfilo+2)*2+Nfilo+1;

  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  /////////            RISORSA                  ////////////
  //////////////////////////////////////////////////////////

  // creazione buffer con SHMGET e concatenazione con SHMAT (sh.mem. attach)
  resID=shmget(IPC_PRIVATE,BUFFERSIZE,IPC_CREAT|0600);
  printf("ID del Buffer: %d\n", resID);
  char newreg_buffer[BUFFERSIZE], registro_buffer[BUFFERSIZE];
  resptr = (char*) shmat(resID,0,0);
  for (ct=0; ct<BUFFERSIZE; ct++) resptr[ct]='0';
  resptr[BUFFERSIZE-1]='\0';
  resptr[Nfilo]='_'; resptr[Nfilo+1]='_'; 
  resptr[2*Nfilo+2]='_'; resptr[2*Nfilo+3]='_'; 
  strncpy(registro_buffer,resptr,BUFFERSIZE);
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////


  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  /////////            SEMAFORI                  ///////////
  //////////////////////////////////////////////////////////

  // creazione buffer con SHMGET (e concatenazione con SHMAT (sh.mem. attach)
  SEMAPHSIZE=2*Nfilo+1;
  char* semptr;
  semID = shmget(IPC_PRIVATE, SEMAPHSIZE, IPC_CREAT | 0600);
  printf("ID del \"Semaforo\": %d\n",semID);
  semptr= (char*)shmat(semID, 0, 0);
  for (ct=0; ct<SEMAPHSIZE-1; ct++) semptr[ct]='1'; semptr[SEMAPHSIZE-1]='\0';
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////


  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  ///////////    GENERAZIONE DEI PROCESSI CONCORRENTI    ///////////////////
  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////

  ID0=getpid();
  ct=1; while(fork()) if (++ct>=Nfilo+1) break;
  ID=getpid()-ID0;

  printf( "Processo %d, ID relativo =%d\n", getpid(), ID);
  char* str1 = (char*) calloc(BUFFERSIZE , sizeof(char));   


  sleep(1);
  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////
  ////////////              REGIONE CRITICA               ///////////////////
  ///////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////

  // LETTORE
  if (ID==0) {
    std::cout << "Reader (Proc.ID=" << ID << ") is starting." << std::endl;
    strncpy(registro_buffer,resptr,BUFFERSIZE);
    if (showmode==4){

      atted=0;
      int Nn[3], No[3];
      for (ctt=0; ctt<3; ctt++) {No[ctt]=0; Nn[ctt]=0;}
      FILE *pfw;
      pfw=fopen("outdata.txt","w");

      switch (deadlock) {
        case(true): fprintf(pfw,"1\n"); break;
        case(false): fprintf(pfw,"0\n"); break;
      }

      wait_Flag=1;
      while(wait_Flag) {

        usleep(ReadDelay);
        strncpy(registro_buffer,resptr,BUFFERSIZE);
        std::cout << "Sys_Stat:" << registro_buffer << std::endl;
        for (ctt=1; ctt<=Nfilo; ctt++)
          fprintf(pfw,"%c ",registro_buffer[ctt-1]);
        for (ctt=1; ctt<=Nfilo; ctt++)
          fprintf(pfw,"%c ",registro_buffer[ctt-1+Nfilo+2]);
        for (ctt=1; ctt<Nfilo; ctt++)
          fprintf(pfw,"%c ",registro_buffer[ctt-1+2*(Nfilo+2)]);
        fprintf(pfw,"%c\n",registro_buffer[Nfilo-1+2*(Nfilo+2)]);

        // Controllo di statiticit  (serve per uscire se siamo in DEADLOCK)
        Nn[0]=0; Nn[1]=0; Nn[2]=0;
        for (ctt=0;ctt<Nfilo;ctt++) {
          Nn[0]+=(registro_buffer[ctt]-'0')*pow(2,Nfilo-ctt-1);
          Nn[1]+=(registro_buffer[Nfilo+2+ctt]-'0')*pow(2,Nfilo-ctt-1);
          Nn[2]+=(registro_buffer[2*(Nfilo+2)+ctt]-'0')*pow(2,Nfilo-ctt-1); }
        accu=0;
        for (ctt=0; ctt<3; ctt++) accu+=abs(No[ctt]-Nn[ctt]);
        if (accu==0) atted++;
        else atted=0;
        for (ctt=0; ctt<3; ctt++) No[ctt]=Nn[ctt];
        if (atted==5) wait_Flag=0;

        // Controllo di arrivo (serve per vedere se siamo alla fine in assenza di DEADLOCK)
        atte=0;
        for(ctu=0; ctu<5; ctu++) {
           accu=0;
           for (ctt=0;ctt<Nfilo;ctt++)
             accu+=registro_buffer[ctt]-'0';
           for (ctt=0;ctt<Nfilo;ctt++)
              accu+=registro_buffer[(Nfilo+2)+ctt]-'0';
           for (ctt=0;ctt<Nfilo;ctt++)
             accu+=registro_buffer[2*(Nfilo+2)+ctt]-'0';
           if (accu==0) atte++;
           else atte=0;
         }
        if (atte==5) wait_Flag=0;


      }
      fclose(pfw);
      exit(0);
    }
    else while(1) {
      strncpy(newreg_buffer,resptr,BUFFERSIZE);
      newreg_buffer[BUFFERSIZE-1]='\0';
      if (showmode==2){
        std::cout << "SysStat:" << newreg_buffer << std::endl;
        usleep(3e5);
      }
      if (showmode==3){
        std::cout << "Desinanti:    " << &newreg_buffer[Nfilo*2+4] << std::endl;
        usleep(1e5);

      }
    }
  }

  // FILOSOFI
  if ((ID>=1)&(ID<Nfilo)) {
    for (ctt=1; ctt<=Nturns; ctt++){

      // Posata destra
      wait_Flag=1;
      while(wait_Flag){
        strncpy(str1,semptr,SEMAPHSIZE); //XXX
        --semptr[2*ID];
        if (semptr[2*ID+1]=='0') ++semptr[2*ID];
        else wait_Flag=0; }
      strncpy(str1,resptr,BUFFERSIZE); //XXX
      resptr[(Nfilo+2)+ID-1]='1';
      if (showmode==1) {
        std::cout << "Proc. "<< ID << "-Ciclo "<<ctt<<": ho preso la posata destra" << std::endl;
        strncpy(str1,&resptr[2*(Nfilo+2)],Nfilo); //XXX
        std::cout << "PR."<<ID<<",Cyc."<<ctt<<"-Stato risorse :" << str1 << std::endl;
        std::cout << std::endl;
      }

      usleep(ProcDelay); 

      // Posata sinistra
      wait_Flag=1;
      while(wait_Flag){
        strncpy(str1,semptr,SEMAPHSIZE); //XXX
        --semptr[2*(ID-1)+1];
        for (ctu=0; ctu<ProcDelay; ctu++) {ctu=ctu+3; ctu=ctu-3;}
        if (semptr[2*(ID-1)]=='0') ++semptr[2*(ID-1)+1];
        else wait_Flag=0; }
      strncpy(str1,resptr,BUFFERSIZE); //XXX
      resptr[ID-1]='1';
      if (showmode==1) {
        std::cout << "Proc. "<< ID << "-Ciclo "<<ctt<<": ho preso la posata sinistra" << std::endl;
        std::cout << "PR."<<ID<<",Cyc."<<ctt<<"-Stato risorse :" << str1 << std::endl;
        std::cout << std::endl;
        }

      usleep(ProcDelay); 

      // Utilizzo della risorsa
      if (showmode==1) {
        std::cout << "Proc.ID=" << ID << "-Ciclo "<<ctt<<" - INIZIO utilizzo risorsa." << std::endl; }
      strncpy(str1,resptr,BUFFERSIZE); //XXX
      resptr[2*(Nfilo+2)+ID-1]='1';
      usleep(1e6);
      resptr[2*(Nfilo+2)+ID-1]='0';
      resptr[ID-1]='0';
      resptr[Nfilo+2+ID-1]='0';
      if (showmode==1) {
        std::cout << "Proc.ID=" << ID << "-Ciclo "<<ctt<<" - FINE utilizzo risorsa." << std::endl;
        std::cout << std::endl;}


      ++semptr[2*ID]; ++semptr[2*(ID-1)+1];
      if (showmode==1) {
        strncpy(str1,&resptr[2*(Nfilo+2)],Nfilo); //XXX
        std::cout << "PR."<<ID<<",Cyc."<<ctt<<"-Stato risorse :" << str1 << std::endl;
        std::cout << std::endl;
      }

      usleep(ProcDelay*2);

    }
  }
//////////////////////////////////////////////

  if (ID==Nfilo) {
    for (ctt=1; ctt<=Nturns; ctt++){

      if (!deadlock) {
        if (ctt==1) std::cout << "No DeadLock: il sistema non si bloccherà." << std::endl;
        // Posata sinistra
        wait_Flag=1;
        while(wait_Flag){
          strncpy(str1,semptr,SEMAPHSIZE); //XXX
          --semptr[2*(ID-1)+1];
          for (ctu=0; ctu<ForkDelay; ctu++) {ctu=ctu+3; ctu=ctu-3;}
          if (semptr[2*(ID-1)]=='0') ++semptr[2*(ID-1)+1];
          else wait_Flag=0; }
        strncpy(str1,resptr,BUFFERSIZE); //XXX
        resptr[ID-1]='1';
        if (showmode==1) {
          std::cout << "Proc. "<< ID << "-Ciclo "<<ctt<<": ho preso la posata sinistra" << std::endl;
          std::cout << "PR."<<ID<<",Cyc."<<ctt<<"-Stato risorse :" << str1 << std::endl;
          std::cout << std::endl;
        }

        usleep(ProcDelay); 

        // Posata destra
        wait_Flag=1;
        while(wait_Flag){
          strncpy(str1,semptr,SEMAPHSIZE); //XXX
          --semptr[0];
          if (semptr[1]=='0') ++semptr[0];
          else wait_Flag=0; }
        strncpy(str1,resptr,BUFFERSIZE); //XXX
        resptr[(Nfilo+2)+ID-1]='1';
        if (showmode==1) {
          std::cout << "Proc. "<< ID << "-Ciclo "<<ctt<<": ho preso la posata destra" << std::endl;
          strncpy(str1,&resptr[2*(Nfilo+2)],Nfilo); //XXX
          std::cout << "PR."<<ID<<",Cyc."<<ctt<<"-Stato risorse :" << str1 << std::endl;
          std::cout << std::endl;
        }
      }
      else {
        if (ctt==1) std::cout << "DeadLock attivo: il sistema si bloccherà." << std::endl;
        // Posata destra
        wait_Flag=1;
        while(wait_Flag){
          strncpy(str1,semptr,SEMAPHSIZE); //XXX
          --semptr[0];
          if (semptr[1]=='0') ++semptr[0];
          else wait_Flag=0; }
        strncpy(str1,resptr,BUFFERSIZE); //XXX
        resptr[(Nfilo+2)+ID-1]='1';
        if (showmode==1) {
          std::cout << "Proc. "<< ID << "-Ciclo "<<ctt<<": ho preso la posata destra" << std::endl;
          strncpy(str1,&resptr[2*(Nfilo+2)],Nfilo); //XXX
          std::cout << "PR."<<ID<<",Cyc."<<ctt<<"-Stato risorse :" << str1 << std::endl;
          std::cout << std::endl;
        }

        usleep(ProcDelay); 

        // Posata sinistra
        wait_Flag=1;
        while(wait_Flag){
          strncpy(str1,semptr,SEMAPHSIZE); //XXX
          --semptr[2*(ID-1)+1];
          for (ctu=0; ctu<ForkDelay; ctu++) {ctu=ctu+3; ctu=ctu-3;}
          if (semptr[2*(ID-1)]=='0') ++semptr[2*(ID-1)+1];
          else wait_Flag=0; }
        strncpy(str1,resptr,BUFFERSIZE); //XXX
        resptr[ID-1]='1';
        if (showmode==1) {
          std::cout << "Proc. "<< ID << "-Ciclo "<<ctt<<": ho preso la posata sinistra" << std::endl;
          std::cout << "PR."<<ID<<",Cyc."<<ctt<<"-Stato risorse :" << str1 << std::endl;
          std::cout << std::endl;
        }
      }


  usleep(ProcDelay); 


      // Utilizzo della risorsa
      if (showmode==1) {
        std::cout << "Proc.ID=" << ID << "-Ciclo "<<ctt<<" - INIZIO utilizzo risorsa." << std::endl;
        std::cout << std::endl;
      }
      strncpy(str1,resptr,BUFFERSIZE); //XXX
      resptr[2*(Nfilo+2)+ID-1]='1';
      usleep(1e6);
      strncpy(str1,resptr,BUFFERSIZE); //XXX
      resptr[2*(Nfilo+2)+ID-1]='0';
      resptr[Nfilo+2+ID-1]='0';
      resptr[ID-1]='0';
      if (showmode==1) {
        std::cout << "Proc.ID=" << ID << "-Ciclo "<<ctt<<" - FINE utilizzo risorsa." << std::endl;
        std::cout << std::endl; }


      ++semptr[2*(ID-1)+1]; ++semptr[0];
      if (showmode==1) {
        strncpy(str1,&resptr[2*(Nfilo+2)],Nfilo); //XXX
        std::cout << "PR."<<ID<<",Cyc."<<ctt<<"-Stato risorse :" << str1 << std::endl;
        std::cout << std::endl;
      }

      usleep(ProcDelay*2);
    }
  }

  ///////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////
  ////////////              REGIONE CRITICA               ///////////////////
  ///////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////

  std::cout << "\n********************\nFINE Processo " << ID << "\n********************\n" << std::endl;

  wait(NULL);
  exit(0);

}
