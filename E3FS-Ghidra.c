#include "out.h"



int _init(EVP_PKEY_CTX *ctx)

{
  int iVar1;
  
  iVar1 = __gmon_start__();
  return iVar1;
}



void FUN_00101020(void)

{
  (*(code *)(undefined *)0x0)();
  return;
}



void FUN_001011a0(void)

{
  __cxa_finalize();
  return;
}



// WARNING: Unknown calling convention -- yet parameter storage is locked

void free(void *__ptr)

{
  free(__ptr);
  return;
}



// WARNING: Unknown calling convention -- yet parameter storage is locked

char * strcpy(char *__dest,char *__src)

{
  char *pcVar1;
  
  pcVar1 = strcpy(__dest,__src);
  return pcVar1;
}



// WARNING: Unknown calling convention -- yet parameter storage is locked

int toupper(int __c)

{
  int iVar1;
  
  iVar1 = toupper(__c);
  return iVar1;
}



void wgetch(void)

{
  wgetch();
  return;
}



void wclear(void)

{
  wclear();
  return;
}



// WARNING: Unknown calling convention -- yet parameter storage is locked

int fclose(FILE *__stream)

{
  int iVar1;
  
  iVar1 = fclose(__stream);
  return iVar1;
}



// WARNING: Unknown calling convention -- yet parameter storage is locked

size_t strlen(char *__s)

{
  size_t sVar1;
  
  sVar1 = strlen(__s);
  return sVar1;
}



void __stack_chk_fail(void)

{
                    // WARNING: Subroutine does not return
  __stack_chk_fail();
}



void initscr(void)

{
  initscr();
  return;
}



void wrefresh(void)

{
  wrefresh();
  return;
}



// WARNING: Unknown calling convention -- yet parameter storage is locked

char * fgets(char *__s,int __n,FILE *__stream)

{
  char *pcVar1;
  
  pcVar1 = fgets(__s,__n,__stream);
  return pcVar1;
}



// WARNING: Unknown calling convention -- yet parameter storage is locked

int strcmp(char *__s1,char *__s2)

{
  int iVar1;
  
  iVar1 = strcmp(__s1,__s2);
  return iVar1;
}



// WARNING: Unknown calling convention -- yet parameter storage is locked

int feof(FILE *__stream)

{
  int iVar1;
  
  iVar1 = feof(__stream);
  return iVar1;
}



void mvprintw(void)

{
  mvprintw();
  return;
}



// WARNING: Unknown calling convention -- yet parameter storage is locked

int select(int __nfds,fd_set *__readfds,fd_set *__writefds,fd_set *__exceptfds,timeval *__timeout)

{
  int iVar1;
  
  iVar1 = select(__nfds,__readfds,__writefds,__exceptfds,__timeout);
  return iVar1;
}



// WARNING: Unknown calling convention -- yet parameter storage is locked

void * malloc(size_t __size)

{
  void *pvVar1;
  
  pvVar1 = malloc(__size);
  return pvVar1;
}



void __isoc99_sscanf(void)

{
  __isoc99_sscanf();
  return;
}



// WARNING: Unknown calling convention -- yet parameter storage is locked

FILE * fopen(char *__filename,char *__modes)

{
  FILE *pFVar1;
  
  pFVar1 = fopen(__filename,__modes);
  return pFVar1;
}



// WARNING: Unknown calling convention -- yet parameter storage is locked

int atoi(char *__nptr)

{
  int iVar1;
  
  iVar1 = atoi(__nptr);
  return iVar1;
}



// WARNING: Unknown calling convention -- yet parameter storage is locked

int sprintf(char *__s,char *__format,...)

{
  int iVar1;
  
  iVar1 = sprintf(__s,__format);
  return iVar1;
}



void endwin(void)

{
  endwin();
  return;
}



// WARNING: Unknown calling convention -- yet parameter storage is locked

uint sleep(uint __seconds)

{
  uint uVar1;
  
  uVar1 = sleep(__seconds);
  return uVar1;
}



// WARNING: Unknown calling convention -- yet parameter storage is locked

ushort ** __ctype_b_loc(void)

{
  ushort **ppuVar1;
  
  ppuVar1 = __ctype_b_loc();
  return ppuVar1;
}



void processEntry _start(undefined8 param_1,undefined8 param_2)

{
  undefined1 auStack_8 [8];
  
  __libc_start_main(main,param_2,&stack0x00000008,__libc_csu_init,__libc_csu_fini,param_1,auStack_8)
  ;
  do {
                    // WARNING: Do nothing block with infinite loop
  } while( true );
}



// WARNING: Removing unreachable block (ram,0x00101363)
// WARNING: Removing unreachable block (ram,0x0010136f)

void deregister_tm_clones(void)

{
  return;
}



// WARNING: Removing unreachable block (ram,0x001013a4)
// WARNING: Removing unreachable block (ram,0x001013b0)

void register_tm_clones(void)

{
  return;
}



void __do_global_dtors_aux(void)

{
  if (completed_8061 != '\0') {
    return;
  }
  FUN_001011a0(__dso_handle);
  deregister_tm_clones();
  completed_8061 = 1;
  return;
}



void frame_dummy(void)

{
  register_tm_clones();
  return;
}



undefined8 kbhit(void)

{
  int iVar1;
  undefined8 uVar2;
  long lVar3;
  __fd_mask *p_Var4;
  long in_FS_OFFSET;
  timeval local_a8;
  fd_set local_98;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  local_a8.tv_sec = 0;
  local_a8.tv_usec = 0;
  lVar3 = 0x10;
  p_Var4 = local_98.fds_bits;
  for (; lVar3 != 0; lVar3 = lVar3 + -1) {
    *p_Var4 = 0;
    p_Var4 = p_Var4 + 1;
  }
  local_98.fds_bits[0] = local_98.fds_bits[0] | 1;
  iVar1 = select(1,&local_98,(fd_set *)0x0,(fd_set *)0x0,&local_a8);
  if (iVar1 == -1) {
    uVar2 = 0;
  }
  else if ((local_98.fds_bits[0] & 1U) == 0) {
    uVar2 = 0;
  }
  else {
    uVar2 = 1;
  }
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    // WARNING: Subroutine does not return
    __stack_chk_fail();
  }
  return uVar2;
}



int * listaCreaNodo(char *param_1,undefined8 param_2,int *param_3,int param_4)

{
  int *piVar1;
  
  piVar1 = (int *)malloc(0x238);
  if (piVar1 == (int *)0x0) {
    piVar1 = (int *)0x0;
  }
  else {
    *(undefined8 *)(piVar1 + 0x8a) = param_2;
    piVar1[1] = param_4;
    piVar1[2] = 0x3c;
    piVar1[3] = 0;
    piVar1[4] = 0;
    *param_3 = *param_3 + 1;
    *piVar1 = *param_3;
    piVar1[5] = 0;
    piVar1[6] = 0;
    piVar1[7] = 0;
    piVar1[8] = 0;
    piVar1[9] = 0;
    *(undefined1 *)(piVar1 + 10) = 0;
    strcpy((char *)(piVar1 + 0x4a),param_1);
  }
  return piVar1;
}



void listaInsertarFinal(long param_1,long param_2)

{
  if (*(long *)(param_1 + 0x230) == 0) {
    *(long *)(param_1 + 0x230) = param_2;
    *(undefined8 *)(param_2 + 0x230) = 0;
  }
  else {
    listaInsertarFinal(*(undefined8 *)(param_1 + 0x230),param_2);
  }
  return;
}



long listaBuscaMinP(long param_1)

{
  int local_14;
  long local_10;
  
  local_14 = 0x7fffffff;
  if (*(long *)(param_1 + 0x230) == 0) {
    local_10 = 0;
  }
  else {
    for (local_10 = *(long *)(param_1 + 0x230); local_10 != 0;
        local_10 = *(long *)(local_10 + 0x230)) {
      if (*(int *)(local_10 + 8) < local_14) {
        local_14 = *(int *)(local_10 + 8);
      }
    }
    for (local_10 = *(long *)(param_1 + 0x230);
        (local_10 != 0 && (local_14 != *(int *)(local_10 + 8)));
        local_10 = *(long *)(local_10 + 0x230)) {
    }
  }
  return local_10;
}



long listaExtraeMinP(long param_1)

{
  long lVar1;
  undefined8 local_20;
  undefined8 local_18;
  
  if (*(long *)(param_1 + 0x230) == 0) {
    local_20 = 0;
  }
  else {
    lVar1 = listaBuscaMinP(param_1);
    if (lVar1 == 0) {
      local_20 = 0;
    }
    else {
      local_18 = param_1;
      for (local_20 = *(long *)(param_1 + 0x230); (local_20 != 0 && (local_20 != lVar1));
          local_20 = *(long *)(local_20 + 0x230)) {
        local_18 = local_20;
      }
      *(undefined8 *)(local_18 + 0x230) = *(undefined8 *)(local_20 + 0x230);
      *(undefined8 *)(local_20 + 0x230) = 0;
    }
  }
  return local_20;
}



long listaExtraeInicio(long param_1)

{
  long lVar1;
  
  if (*(long *)(param_1 + 0x230) == 0) {
    lVar1 = 0;
  }
  else {
    lVar1 = *(long *)(param_1 + 0x230);
    *(undefined8 *)(param_1 + 0x230) = *(undefined8 *)(lVar1 + 0x230);
    *(undefined8 *)(lVar1 + 0x230) = 0;
  }
  return lVar1;
}



long listaExtrae(long param_1,int param_2)

{
  long lVar1;
  undefined8 local_18;
  
  for (local_18 = param_1;
      (*(long *)(local_18 + 0x230) != 0 && (param_2 != **(int **)(local_18 + 0x230)));
      local_18 = *(long *)(local_18 + 0x230)) {
  }
  if (*(long *)(local_18 + 0x230) == 0) {
    lVar1 = 0;
  }
  else {
    lVar1 = *(long *)(local_18 + 0x230);
    *(undefined8 *)(local_18 + 0x230) = *(undefined8 *)(lVar1 + 0x230);
    *(undefined8 *)(lVar1 + 0x230) = 0;
  }
  return lVar1;
}



void libera(void *param_1)

{
  if (param_1 != (void *)0x0) {
    libera(*(undefined8 *)((long)param_1 + 0x230));
    if (*(long *)((long)param_1 + 0x228) != 0) {
      fclose(*(FILE **)((long)param_1 + 0x228));
    }
    free(param_1);
  }
  return;
}



void listaLibera(long param_1)

{
  libera(*(undefined8 *)(param_1 + 0x230));
  *(undefined8 *)(param_1 + 0x230) = 0;
  return;
}



undefined8 buscaUsrInCntUsrs(int param_1)

{
  undefined8 uVar1;
  int local_c;
  
  local_c = 0;
  while ((local_c < 0x400 && (*(int *)(cntUsrs + (long)local_c * 4) != -1))) {
    if (param_1 == *(int *)(cntUsrs + (long)local_c * 4)) {
      return 1;
    }
    local_c = local_c + 1;
  }
  if ((*(int *)(cntUsrs + (long)local_c * 4) == -1) && (local_c < 0x400)) {
    uVar1 = 0;
  }
  else {
    uVar1 = 0xffffffff;
  }
  return uVar1;
}



undefined4 addToCntUsrs(undefined4 param_1)

{
  int local_c;
  
  local_c = 0;
  while ((local_c < 0x400 && (*(int *)(cntUsrs + (long)local_c * 4) != -1))) {
    local_c = local_c + 1;
  }
  if ((local_c < 0x400) && (*(int *)(cntUsrs + (long)local_c * 4) == -1)) {
    *(undefined4 *)(cntUsrs + (long)local_c * 4) = param_1;
  }
  else {
    param_1 = 0xffffffff;
  }
  return param_1;
}



int cuentaUsuarios(void)

{
  int iVar1;
  int local_14;
  long local_10;
  
  for (local_14 = 0; local_14 < 0x400; local_14 = local_14 + 1) {
    *(undefined4 *)(cntUsrs + (long)local_14 * 4) = 0xffffffff;
  }
  for (local_10 = Ejecucion._560_8_; local_10 != 0; local_10 = *(long *)(local_10 + 0x230)) {
    iVar1 = buscaUsrInCntUsrs(*(undefined4 *)(local_10 + 4));
    if (iVar1 == 0) {
      addToCntUsrs(*(undefined4 *)(local_10 + 4));
    }
  }
  for (local_10 = Listos._560_8_; local_10 != 0; local_10 = *(long *)(local_10 + 0x230)) {
    iVar1 = buscaUsrInCntUsrs(*(undefined4 *)(local_10 + 4));
    if (iVar1 == 0) {
      addToCntUsrs(*(undefined4 *)(local_10 + 4));
    }
  }
  local_14 = 0;
  while ((local_14 < 0x400 && (*(int *)(cntUsrs + (long)local_14 * 4) != -1))) {
    local_14 = local_14 + 1;
  }
  return local_14;
}



void actualizaW(void)

{
  undefined4 uVar1;
  int iVar2;
  undefined4 uVar3;
  double dVar4;
  
  iVar2 = cuentaUsuarios();
  uVar1 = MinP;
  W = 1.0 / (float)iVar2;
  dVar4 = (double)W;
  uVar3 = cuentaUsuarios();
  mvprintw(dVar4,0,dx + 0x5a,
           "------------------------------ USUARIOS: [%d] ----- MinP: [%d] ------ W: [%.2f] ------------------------------"
           ,uVar3,uVar1);
  return;
}



void listaActualizaInfoPlan(long param_1)

{
  undefined8 local_10;
  
  if (*(long *)(param_1 + 0x230) != 0) {
    for (local_10 = *(long *)(param_1 + 0x230); local_10 != 0;
        local_10 = *(long *)(local_10 + 0x230)) {
      *(int *)(local_10 + 0xc) = *(int *)(local_10 + 0xc) / 2;
      *(int *)(local_10 + 0x10) = *(int *)(local_10 + 0x10) / 2;
      *(int *)(local_10 + 8) =
           (int)((float)*(int *)(local_10 + 0x10) / (W * 4.0) +
                (float)(*(int *)(local_10 + 0xc) / 2 + 0x3c));
    }
  }
  return;
}



void incCPUKnts(long param_1)

{
  undefined8 local_10;
  
  *(int *)(param_1 + 0xc) = *(int *)(param_1 + 0xc) + INCCPU;
  *(int *)(param_1 + 0x10) = *(int *)(param_1 + 0x10) + INCCPU;
  local_10 = Listos._560_8_;
  if (Listos._560_8_ != 0) {
    for (; local_10 != 0; local_10 = *(long *)(local_10 + 0x230)) {
      if (*(int *)(local_10 + 4) == *(int *)(param_1 + 4)) {
        *(undefined4 *)(local_10 + 0x10) = *(undefined4 *)(param_1 + 0x10);
      }
    }
  }
  return;
}



void imprimeListas(void)

{
  int iVar1;
  int local_44;
  undefined4 *local_40;
  
  for (local_44 = 0; local_44 < 0x28; local_44 = local_44 + 1) {
    mvprintw(local_44 + 2,dx + 0x5a,
             "                                                                                                                  "
            );
  }
  local_44 = 0;
  mvprintw(2,dx + 0x5a,
           "------------------------------------------------[EJECUCION]--------------------------------------------------"
          );
  for (local_40 = (undefined4 *)Ejecucion._560_8_; local_40 != (undefined4 *)0x0;
      local_40 = *(undefined4 **)(local_40 + 0x8c)) {
    mvprintw(local_44 + 3,dx + 0x5a,
             "PID:[%d] UID:[%d] P:[%d] KCPU:[%d] KCPUxU:[%d] FILE:[%s] AX:[%d] BX:[%d] CX:[%d] DX:[%d] PC:[%d] IR:[%s]"
             ,*local_40,local_40[1],local_40[2],local_40[3],local_40[4],local_40 + 0x4a,local_40[5],
             local_40[6],local_40[7],local_40[8],local_40[9],local_40 + 10);
    local_44 = local_44 + 1;
  }
  mvprintw(local_44 + 3,dx + 0x5a,
           "-------------------------------------------------[LISTOS]----------------------------------------------------"
          );
  iVar1 = local_44;
  for (local_40 = (undefined4 *)Listos._560_8_; local_44 = iVar1 + 1, local_40 != (undefined4 *)0x0;
      local_40 = *(undefined4 **)(local_40 + 0x8c)) {
    mvprintw(iVar1 + 4,dx + 0x5a,
             "PID:[%d] UID:[%d] P:[%d] KCPU:[%d] KCPUxU:[%d] FILE:[%s] AX:[%d] BX:[%d] CX:[%d] DX:[%d] PC:[%d] IR:[%s]"
             ,*local_40,local_40[1],local_40[2],local_40[3],local_40[4],local_40 + 0x4a,local_40[5],
             local_40[6],local_40[7],local_40[8],local_40[9],local_40 + 10);
    iVar1 = local_44;
  }
  mvprintw(iVar1 + 4,dx + 0x5a,
           "-----------------------------------------------[TERMINADOS]--------------------------------------------------"
          );
  local_44 = iVar1 + 2;
  for (local_40 = (undefined4 *)Terminados._560_8_; local_40 != (undefined4 *)0x0;
      local_40 = *(undefined4 **)(local_40 + 0x8c)) {
    mvprintw(local_44 + 3,dx + 0x5a,
             "PID:[%d] UID:[%d] P:[%d] KCPU:[%d] KCPUxU:[%d] FILE:[%s] AX:[%d] BX:[%d] CX:[%d] DX:[%d] PC:[%d] IR:[%s]"
             ,*local_40,local_40[1],local_40[2],local_40[3],local_40[4],local_40 + 0x4a,local_40[5],
             local_40[6],local_40[7],local_40[8],local_40[9],local_40 + 10);
    local_44 = local_44 + 1;
  }
  mvprintw(local_44 + 3,dx + 0x5a,
           "                                                                                                                  "
          );
  wrefresh(stdscr);
  return;
}



bool mataProceso(uint param_1)

{
  long in_FS_OFFSET;
  long local_120;
  char local_118 [264];
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  local_118[0] = '\0';
  local_120 = listaExtrae(Ejecucion,param_1);
  if (local_120 == 0) {
    local_120 = listaExtrae(Listos,param_1);
  }
  if (local_120 != 0) {
    listaInsertarFinal(Terminados,local_120);
    actualizaW();
  }
  else {
    sprintf(local_118,"Error: No se pudo encontrar el proceso con PID: [%d].",(ulong)param_1);
    display_msg(local_118,&DAT_001052e6,&DAT_001052e6);
  }
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    // WARNING: Subroutine does not return
    __stack_chk_fail();
  }
  return local_120 != 0;
}



undefined8 crearProceso(char *param_1,undefined4 param_2)

{
  FILE *pFVar1;
  undefined8 uVar2;
  uint *puVar3;
  long in_FS_OFFSET;
  char local_218 [520];
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  pFVar1 = fopen(param_1,"r");
  if (pFVar1 == (FILE *)0x0) {
    sprintf(local_218,"Error: No se pudo abrir el archivo: [%s].",param_1);
    display_msg(local_218,&DAT_001052e6,&DAT_001052e6);
    uVar2 = 0;
  }
  else {
    puVar3 = (uint *)listaCreaNodo(param_1,pFVar1,&PID,param_2);
    if (puVar3 == (uint *)0x0) {
      sprintf(local_218,"Error: No se pudo asignar memoria para el proceso: [%s].",param_1);
      display_msg(local_218,&DAT_001052e6,&DAT_001052e6);
      uVar2 = 0;
    }
    else {
      listaInsertarFinal(Listos,puVar3);
      actualizaW();
      sprintf(local_218,"Proceso: %d [%s] Creado correctamente.",(ulong)*puVar3,puVar3 + 0x4a);
      display_msg(local_218,&DAT_001052e6,&DAT_001052e6);
      imprimeListas();
      uVar2 = 1;
    }
  }
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    // WARNING: Subroutine does not return
    __stack_chk_fail();
  }
  return uVar2;
}



undefined8 toUpper(long param_1)

{
  int iVar1;
  undefined4 local_c;
  
  for (local_c = 0; *(char *)(param_1 + local_c) != '\0'; local_c = local_c + 1) {
    iVar1 = toupper((int)*(char *)(param_1 + local_c));
    *(char *)(param_1 + local_c) = (char)iVar1;
  }
  return 1;
}



undefined8 isNumeric(char *param_1)

{
  ushort **ppuVar1;
  int local_c;
  
  local_c = 0;
  while( true ) {
    if (param_1[local_c] == '\0') {
      return 1;
    }
    ppuVar1 = __ctype_b_loc();
    if ((((*ppuVar1)[param_1[local_c]] & 0x800) == 0) &&
       ((local_c != 0 || ((*param_1 != '+' && (*param_1 != '-')))))) break;
    local_c = local_c + 1;
  }
  return 0;
}



char * ltrim(char *param_1)

{
  ushort **ppuVar1;
  char *local_10;
  
  local_10 = param_1;
  while( true ) {
    ppuVar1 = __ctype_b_loc();
    if (((*ppuVar1)[*local_10] & 0x2000) == 0) break;
    local_10 = local_10 + 1;
  }
  return local_10;
}



char * rtrim(char *param_1)

{
  char *pcVar1;
  size_t sVar2;
  ushort **ppuVar3;
  char *local_10;
  
  sVar2 = strlen(param_1);
  if ((int)sVar2 != 0) {
    pcVar1 = param_1 + (int)sVar2;
    do {
      local_10 = pcVar1;
      ppuVar3 = __ctype_b_loc();
      pcVar1 = local_10 + -1;
    } while (((*ppuVar3)[local_10[-1]] & 0x2000) != 0);
    *local_10 = '\0';
  }
  return param_1;
}



void trim(undefined8 param_1)

{
  undefined8 uVar1;
  
  uVar1 = ltrim(param_1);
  rtrim(uVar1);
  return;
}



void evaluaComando(undefined4 *param_1)

{
  int iVar1;
  size_t sVar2;
  long in_FS_OFFSET;
  int local_18c;
  char local_188 [31];
  char acStack_169 [32];
  char acStack_149 [33];
  char local_128 [264];
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_188[0] = '\0';
  acStack_169[1] = '\0';
  acStack_149[1] = '\0';
  local_128[0] = '\0';
  __isoc99_sscanf(param_1,"%s %s %s",local_188,acStack_169 + 1,acStack_149 + 1);
  toUpper(local_188);
  local_18c = 0;
  while( true ) {
    sVar2 = strlen(local_188);
    if (sVar2 <= (ulong)(long)local_18c) break;
    *(char *)((long)local_18c + (long)param_1) = local_188[local_18c];
    local_18c = local_18c + 1;
  }
  if (acStack_169[1] != '\0') {
    sVar2 = strlen(acStack_169 + 1);
    if (acStack_169[sVar2] == '\n') {
      sVar2 = strlen(acStack_169 + 1);
      acStack_169[sVar2] = '\0';
    }
  }
  if (acStack_149[1] != '\0') {
    sVar2 = strlen(acStack_149 + 1);
    if (acStack_149[sVar2] == '\n') {
      sVar2 = strlen(acStack_149 + 1);
      acStack_149[sVar2] = '\0';
    }
  }
  iVar1 = strcmp(local_188,"LOAD");
  if (iVar1 == 0) {
    if (acStack_169[1] == '\0') {
      sprintf(local_128,"Error: Especifique el nombre de archivo a cargar...");
      display_msg(local_128,&DAT_001052e6,&DAT_001052e6);
    }
    else {
      if (acStack_149[1] != '\0') {
        iVar1 = isNumeric(acStack_149 + 1);
        if (iVar1 != 0) {
          iVar1 = isNumeric(acStack_149 + 1);
          if (iVar1 == 0) {
            sprintf(local_128,&DAT_00105398);
            display_msg(local_128,0,0);
          }
          else {
            iVar1 = atoi(acStack_149 + 1);
            crearProceso(acStack_169 + 1,iVar1);
          }
          goto LAB_001029d7;
        }
      }
      sprintf(local_128,&DAT_001053f8);
      display_msg(local_128,&DAT_001052e6,&DAT_001052e6);
    }
  }
  else {
    iVar1 = strcmp(local_188,"EXIT");
    if (iVar1 != 0) {
      iVar1 = strcmp(local_188,"EXIT\n");
      if (iVar1 != 0) {
        iVar1 = strcmp(local_188,"KILL");
        if (iVar1 == 0) {
          if (acStack_169[1] != '\0') {
            iVar1 = isNumeric(acStack_169 + 1);
            if (iVar1 != 0) {
              iVar1 = atoi(acStack_169 + 1);
              iVar1 = mataProceso(iVar1);
              if (iVar1 != 0) {
                display_proc(Ejecucion._560_8_);
                imprimeListas();
                sprintf(local_128,"Aviso: El nodo con PID[%s], ha sido terminado",acStack_169 + 1);
                display_msg(local_128,&DAT_001052e6,&DAT_001052e6);
              }
              goto LAB_001029d7;
            }
          }
          sprintf(local_128,&DAT_001054f6,acStack_169 + 1);
          display_msg(local_128,"USO: KILL <pid>",&DAT_001052e6);
        }
        else {
          sprintf(local_128,&DAT_00105528,local_188);
          display_msg(local_128,&DAT_001052e6,&DAT_001052e6);
        }
        goto LAB_001029d7;
      }
    }
    sprintf(local_128,&DAT_00105488);
    display_msg(local_128,&DAT_001052e6,&DAT_001052e6);
    iVar1 = wgetch(stdscr);
    iVar1 = toupper(iVar1);
    if ((char)iVar1 == 'S') {
      *param_1 = 0x54495845;
      *(undefined2 *)(param_1 + 1) = 10;
    }
    else {
      *(undefined1 *)param_1 = 0;
    }
  }
LAB_001029d7:
  display_prompt(param_1);
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    // WARNING: Subroutine does not return
    __stack_chk_fail();
  }
  return;
}



void add_history(char *param_1)

{
  int local_c;
  
  for (local_c = 0x14; 0 < local_c; local_c = local_c + -1) {
    strcpy(history + (long)local_c * 0x100,history + (long)(local_c + -1) * 0x100);
  }
  strcpy(history,param_1);
  return;
}



void shell_attention(char *param_1)

{
  char cVar1;
  int iVar2;
  size_t sVar3;
  long lVar4;
  char local_d;
  
  iVar2 = kbhit();
  if (iVar2 != 0) {
    cVar1 = wgetch(stdscr);
    if (cVar1 == '\x7f') {
      sVar3 = strlen(param_1);
      if ((int)sVar3 < 2) {
        lVar4 = 0;
      }
      else {
        lVar4 = (long)((int)sVar3 + -1);
      }
      param_1[lVar4] = '\0';
    }
    else if (cVar1 == '\n') {
      add_history(param_1);
      hisNav = 0;
      evaluaComando(param_1);
      iVar2 = strcmp(param_1,"EXIT\n");
      if (iVar2 != 0) {
        *param_1 = '\0';
      }
    }
    else if (cVar1 == '\x1b') {
      iVar2 = kbhit();
      if (iVar2 == 0) {
        builtin_strncpy(param_1,"EXIT\n",6);
        evaluaComando(param_1);
        local_d = '\x1b';
      }
      else {
        local_d = wgetch(stdscr);
      }
      if ((local_d == '[') && (iVar2 = kbhit(), iVar2 != 0)) {
        cVar1 = wgetch(stdscr);
        if (cVar1 == 'D') {
          if (cntMultiplo < cntMultiplo * 2) {
            cntMultiplo = (int)((double)cntMultiplo + (double)cntMultiplo);
          }
        }
        else if (cVar1 < 'E') {
          if (cVar1 == 'C') {
            if ((cntMultiplo / 2 < cntMultiplo) && (1 < cntMultiplo)) {
              cntMultiplo = (int)((double)cntMultiplo / 2.0);
            }
          }
          else if (cVar1 < 'D') {
            if (cVar1 == 'A') {
              strcpy(param_1,history + (long)hisNav * 0x100);
              if ((hisNav < 0x14) && (history[(long)(hisNav + 1) * 0x100] != '\0')) {
                hisNav = hisNav + 1;
              }
            }
            else if (cVar1 == 'B') {
              if (0 < hisNav) {
                hisNav = hisNav + -1;
              }
              strcpy(param_1,history + (long)hisNav * 0x100);
            }
          }
        }
      }
    }
    else if (('\x1f' < cVar1) && (cVar1 != '\x7f')) {
      sprintf(param_1,"%s%c",param_1,(ulong)(uint)(int)cVar1);
    }
  }
  return;
}



size_t sigLineaArch(long param_1)

{
  int iVar1;
  char *pcVar2;
  size_t sVar3;
  long in_FS_OFFSET;
  char local_218 [520];
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  if (*(long *)(param_1 + 0x228) == 0) {
    display_msg("Error: No hay archivo abierto.",&DAT_001052e6,&DAT_001052e6);
  }
  else {
    iVar1 = feof(*(FILE **)(param_1 + 0x228));
    if (iVar1 == 0) {
      pcVar2 = fgets((char *)(param_1 + 0x28),0x100,*(FILE **)(param_1 + 0x228));
      if (pcVar2 == (char *)0x0) {
        sprintf(local_218,&DAT_00105578,param_1 + 0x128);
        display_msg(local_218,&DAT_001055b0,&DAT_001052e6);
        fclose(*(FILE **)(param_1 + 0x228));
        *(undefined8 *)(param_1 + 0x228) = 0;
        sVar3 = 0xffffffff;
      }
      else {
        sVar3 = strlen((char *)(param_1 + 0x28));
      }
      goto LAB_00102ef5;
    }
    sprintf(local_218,"Error: Fin de Archivo [%s] encontrado.",param_1 + 0x128);
    display_msg(local_218,&DAT_001052e6,&DAT_001052e6);
    fclose(*(FILE **)(param_1 + 0x228));
    *(undefined8 *)(param_1 + 0x228) = 0;
  }
  sVar3 = 0;
LAB_00102ef5:
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    // WARNING: Subroutine does not return
    __stack_chk_fail();
  }
  return sVar3;
}



void errorInst(undefined8 param_1)

{
  long in_FS_OFFSET;
  char local_118 [264];
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  sprintf(local_118,&DAT_00105600,param_1);
  display_msg(local_118,&DAT_00105628,&DAT_001052e6);
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    // WARNING: Subroutine does not return
    __stack_chk_fail();
  }
  return;
}



void errorReg(undefined8 param_1)

{
  long in_FS_OFFSET;
  char local_118 [264];
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  sprintf(local_118,&DAT_00105650,param_1);
  display_msg(local_118,&DAT_00105628,&DAT_001052e6);
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    // WARNING: Subroutine does not return
    __stack_chk_fail();
  }
  return;
}



void errorDivCero(void)

{
  display_msg("Error division entre cero.",&DAT_00105628,&DAT_001052e6);
  return;
}



undefined8 ejecuta_inst(long param_1,char *param_2)

{
  int iVar1;
  char *pcVar2;
  undefined8 uVar3;
  long in_FS_OFFSET;
  int local_8c;
  int local_88;
  int local_84;
  int local_80;
  int local_7c;
  char local_78 [32];
  char local_58 [32];
  char local_38 [40];
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  __isoc99_sscanf(param_2,"%s %s %s",local_78,local_58,local_38);
  pcVar2 = (char *)trim(local_78);
  strcpy(local_78,pcVar2);
  pcVar2 = (char *)trim(local_58);
  strcpy(local_58,pcVar2);
  pcVar2 = (char *)trim(local_38);
  strcpy(local_38,pcVar2);
  toUpper(local_78);
  toUpper(local_58);
  toUpper(local_38);
  strcpy((char *)(param_1 + 0x28),param_2);
  incCPUKnts(param_1);
  display_proc(param_1);
  iVar1 = strcmp(local_78,"END");
  if (iVar1 == 0) {
    display_msg(&DAT_001056b8,"Programa Finalizado Correctamente.",&DAT_001052e6);
    uVar3 = 0xffffffff;
    goto LAB_00103cfb;
  }
  iVar1 = strcmp(local_78,"INC");
  if (iVar1 == 0) {
    iVar1 = strcmp("AX",local_58);
    if (iVar1 == 0) {
      *(int *)(param_1 + 0x14) = *(int *)(param_1 + 0x14) + 1;
    }
    else {
      iVar1 = strcmp("BX",local_58);
      if (iVar1 == 0) {
        *(int *)(param_1 + 0x18) = *(int *)(param_1 + 0x18) + 1;
      }
      else {
        iVar1 = strcmp("CX",local_58);
        if (iVar1 == 0) {
          *(int *)(param_1 + 0x1c) = *(int *)(param_1 + 0x1c) + 1;
        }
        else {
          iVar1 = strcmp("DX",local_58);
          if (iVar1 == 0) {
            *(int *)(param_1 + 0x20) = *(int *)(param_1 + 0x20) + 1;
          }
          else {
            errorReg(local_58);
          }
        }
      }
    }
  }
  else {
    iVar1 = strcmp(local_78,"DEC");
    if (iVar1 == 0) {
      iVar1 = strcmp("AX",local_58);
      if (iVar1 == 0) {
        *(int *)(param_1 + 0x14) = *(int *)(param_1 + 0x14) + -1;
      }
      else {
        iVar1 = strcmp("BX",local_58);
        if (iVar1 == 0) {
          *(int *)(param_1 + 0x18) = *(int *)(param_1 + 0x18) + -1;
        }
        else {
          iVar1 = strcmp("CX",local_58);
          if (iVar1 == 0) {
            *(int *)(param_1 + 0x1c) = *(int *)(param_1 + 0x1c) + -1;
          }
          else {
            iVar1 = strcmp("DX",local_58);
            if (iVar1 == 0) {
              *(int *)(param_1 + 0x20) = *(int *)(param_1 + 0x20) + -1;
            }
            else {
              errorReg(local_58);
            }
          }
        }
      }
    }
    else {
      iVar1 = strcmp("MOV",local_78);
      if (iVar1 == 0) {
        iVar1 = strcmp("AX",local_38);
        if (iVar1 == 0) {
          local_8c = *(int *)(param_1 + 0x14);
        }
        else {
          iVar1 = strcmp("BX",local_38);
          if (iVar1 == 0) {
            local_8c = *(int *)(param_1 + 0x18);
          }
          else {
            iVar1 = strcmp("CX",local_38);
            if (iVar1 == 0) {
              local_8c = *(int *)(param_1 + 0x1c);
            }
            else {
              iVar1 = strcmp("DX",local_38);
              if (iVar1 == 0) {
                local_8c = *(int *)(param_1 + 0x20);
              }
              else {
                iVar1 = isNumeric(local_38);
                if (iVar1 == 0) {
                  errorReg(local_38);
                  uVar3 = 0;
                  goto LAB_00103cfb;
                }
                local_8c = atoi(local_38);
              }
            }
          }
        }
        iVar1 = strcmp("AX",local_58);
        if (iVar1 == 0) {
          *(int *)(param_1 + 0x14) = local_8c;
        }
        else {
          iVar1 = strcmp("BX",local_58);
          if (iVar1 == 0) {
            *(int *)(param_1 + 0x18) = local_8c;
          }
          else {
            iVar1 = strcmp("CX",local_58);
            if (iVar1 == 0) {
              *(int *)(param_1 + 0x1c) = local_8c;
            }
            else {
              iVar1 = strcmp("DX",local_58);
              if (iVar1 != 0) {
                errorReg(local_58);
                uVar3 = 0;
                goto LAB_00103cfb;
              }
              *(int *)(param_1 + 0x20) = local_8c;
            }
          }
        }
      }
      else {
        iVar1 = strcmp("ADD",local_78);
        if (iVar1 == 0) {
          iVar1 = strcmp("AX",local_38);
          if (iVar1 == 0) {
            local_88 = *(int *)(param_1 + 0x14);
          }
          else {
            iVar1 = strcmp("BX",local_38);
            if (iVar1 == 0) {
              local_88 = *(int *)(param_1 + 0x18);
            }
            else {
              iVar1 = strcmp("CX",local_38);
              if (iVar1 == 0) {
                local_88 = *(int *)(param_1 + 0x1c);
              }
              else {
                iVar1 = strcmp("DX",local_38);
                if (iVar1 == 0) {
                  local_88 = *(int *)(param_1 + 0x20);
                }
                else {
                  iVar1 = isNumeric(local_38);
                  if (iVar1 == 0) {
                    errorReg(local_38);
                    uVar3 = 0;
                    goto LAB_00103cfb;
                  }
                  local_88 = atoi(local_38);
                }
              }
            }
          }
          iVar1 = strcmp("AX",local_58);
          if (iVar1 == 0) {
            *(int *)(param_1 + 0x14) = *(int *)(param_1 + 0x14) + local_88;
          }
          else {
            iVar1 = strcmp("BX",local_58);
            if (iVar1 == 0) {
              *(int *)(param_1 + 0x18) = *(int *)(param_1 + 0x18) + local_88;
            }
            else {
              iVar1 = strcmp("CX",local_58);
              if (iVar1 == 0) {
                *(int *)(param_1 + 0x1c) = *(int *)(param_1 + 0x1c) + local_88;
              }
              else {
                iVar1 = strcmp("DX",local_58);
                if (iVar1 != 0) {
                  errorReg(local_58);
                  uVar3 = 0;
                  goto LAB_00103cfb;
                }
                *(int *)(param_1 + 0x20) = *(int *)(param_1 + 0x20) + local_88;
              }
            }
          }
        }
        else {
          iVar1 = strcmp("SUB",local_78);
          if (iVar1 == 0) {
            iVar1 = strcmp("AX",local_38);
            if (iVar1 == 0) {
              local_84 = *(int *)(param_1 + 0x14);
            }
            else {
              iVar1 = strcmp("BX",local_38);
              if (iVar1 == 0) {
                local_84 = *(int *)(param_1 + 0x18);
              }
              else {
                iVar1 = strcmp("CX",local_38);
                if (iVar1 == 0) {
                  local_84 = *(int *)(param_1 + 0x1c);
                }
                else {
                  iVar1 = strcmp("DX",local_38);
                  if (iVar1 == 0) {
                    local_84 = *(int *)(param_1 + 0x20);
                  }
                  else {
                    iVar1 = isNumeric(local_38);
                    if (iVar1 == 0) {
                      errorReg(local_38);
                      uVar3 = 0;
                      goto LAB_00103cfb;
                    }
                    local_84 = atoi(local_38);
                  }
                }
              }
            }
            iVar1 = strcmp("AX",local_58);
            if (iVar1 == 0) {
              *(int *)(param_1 + 0x14) = *(int *)(param_1 + 0x14) - local_84;
            }
            else {
              iVar1 = strcmp("BX",local_58);
              if (iVar1 == 0) {
                *(int *)(param_1 + 0x18) = *(int *)(param_1 + 0x18) - local_84;
              }
              else {
                iVar1 = strcmp("CX",local_58);
                if (iVar1 == 0) {
                  *(int *)(param_1 + 0x1c) = *(int *)(param_1 + 0x1c) - local_84;
                }
                else {
                  iVar1 = strcmp("DX",local_58);
                  if (iVar1 != 0) {
                    errorReg(local_58);
                    uVar3 = 0;
                    goto LAB_00103cfb;
                  }
                  *(int *)(param_1 + 0x20) = *(int *)(param_1 + 0x20) - local_84;
                }
              }
            }
          }
          else {
            iVar1 = strcmp("MUL",local_78);
            if (iVar1 == 0) {
              iVar1 = strcmp("AX",local_38);
              if (iVar1 == 0) {
                local_80 = *(int *)(param_1 + 0x14);
              }
              else {
                iVar1 = strcmp("BX",local_38);
                if (iVar1 == 0) {
                  local_80 = *(int *)(param_1 + 0x18);
                }
                else {
                  iVar1 = strcmp("CX",local_38);
                  if (iVar1 == 0) {
                    local_80 = *(int *)(param_1 + 0x1c);
                  }
                  else {
                    iVar1 = strcmp("DX",local_38);
                    if (iVar1 == 0) {
                      local_80 = *(int *)(param_1 + 0x20);
                    }
                    else {
                      iVar1 = isNumeric(local_38);
                      if (iVar1 == 0) {
                        errorReg(local_38);
                        uVar3 = 0;
                        goto LAB_00103cfb;
                      }
                      local_80 = atoi(local_38);
                    }
                  }
                }
              }
              iVar1 = strcmp("AX",local_58);
              if (iVar1 == 0) {
                *(int *)(param_1 + 0x14) = *(int *)(param_1 + 0x14) * local_80;
              }
              else {
                iVar1 = strcmp("BX",local_58);
                if (iVar1 == 0) {
                  *(int *)(param_1 + 0x18) = *(int *)(param_1 + 0x18) * local_80;
                }
                else {
                  iVar1 = strcmp("CX",local_58);
                  if (iVar1 == 0) {
                    *(int *)(param_1 + 0x1c) = *(int *)(param_1 + 0x1c) * local_80;
                  }
                  else {
                    iVar1 = strcmp("DX",local_58);
                    if (iVar1 != 0) {
                      errorReg(local_58);
                      uVar3 = 0;
                      goto LAB_00103cfb;
                    }
                    *(int *)(param_1 + 0x20) = *(int *)(param_1 + 0x20) * local_80;
                  }
                }
              }
            }
            else {
              iVar1 = strcmp("DIV",local_78);
              if (iVar1 != 0) {
                errorInst(local_78);
                uVar3 = 0;
                goto LAB_00103cfb;
              }
              iVar1 = strcmp("AX",local_38);
              if (iVar1 == 0) {
                local_7c = *(int *)(param_1 + 0x14);
              }
              else {
                iVar1 = strcmp("BX",local_38);
                if (iVar1 == 0) {
                  local_7c = *(int *)(param_1 + 0x18);
                }
                else {
                  iVar1 = strcmp("CX",local_38);
                  if (iVar1 == 0) {
                    local_7c = *(int *)(param_1 + 0x1c);
                  }
                  else {
                    iVar1 = strcmp("DX",local_38);
                    if (iVar1 == 0) {
                      local_7c = *(int *)(param_1 + 0x20);
                    }
                    else {
                      iVar1 = isNumeric(local_38);
                      if (iVar1 == 0) {
                        errorReg(local_38);
                        uVar3 = 0;
                        goto LAB_00103cfb;
                      }
                      local_7c = atoi(local_38);
                    }
                  }
                }
              }
              if (local_7c == 0) {
                errorDivCero();
                uVar3 = 0;
                goto LAB_00103cfb;
              }
              iVar1 = strcmp("AX",local_58);
              if (iVar1 == 0) {
                *(int *)(param_1 + 0x14) = *(int *)(param_1 + 0x14) / local_7c;
              }
              else {
                iVar1 = strcmp("BX",local_58);
                if (iVar1 == 0) {
                  *(int *)(param_1 + 0x18) = *(int *)(param_1 + 0x18) / local_7c;
                }
                else {
                  iVar1 = strcmp("CX",local_58);
                  if (iVar1 == 0) {
                    *(int *)(param_1 + 0x1c) = *(int *)(param_1 + 0x1c) / local_7c;
                  }
                  else {
                    iVar1 = strcmp("DX",local_58);
                    if (iVar1 != 0) {
                      errorReg(local_58);
                      uVar3 = 0;
                      goto LAB_00103cfb;
                    }
                    *(int *)(param_1 + 0x20) = *(int *)(param_1 + 0x20) / local_7c;
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  *(int *)(param_1 + 0x24) = *(int *)(param_1 + 0x24) + 1;
  uVar3 = 1;
LAB_00103cfb:
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    // WARNING: Subroutine does not return
    __stack_chk_fail();
  }
  return uVar3;
}



void display_msg(long param_1,long param_2,long param_3)

{
  mvprintw(0xd,2,"-------------------------------MENSAJES----------------------------------");
  if (param_1 != 0) {
    mvprintw(0xe,2,"-                                                                       -");
  }
  mvprintw(0xf,2,"-                                                                       -");
  if (param_2 != 0) {
    mvprintw(0x10,2,"-                                                                       -");
  }
  mvprintw(0x11,2,"-                                                                       -");
  if (param_3 != 0) {
    mvprintw(0x12,2,"-                                                                       -");
  }
  mvprintw(0x13,2,"-------------------------------------------------------------------------");
  if (param_1 != 0) {
    mvprintw(0xe,4,&DAT_001057fa,param_1);
  }
  if (param_2 != 0) {
    mvprintw(0x10,4,&DAT_001057fa,param_2);
  }
  if (param_3 != 0) {
    mvprintw(0x12,4,&DAT_001057fa,param_3);
  }
  wrefresh(stdscr);
  return;
}



void display_prompt(undefined8 param_1)

{
  int local_c;
  
  for (local_c = 0; local_c < 2; local_c = local_c + 1) {
    if (history[(long)local_c * 0x100] != '\0') {
      mvprintw(local_c + 1,1,
               "                                                                        ");
      mvprintw(local_c + 1,1,"Prompt>%s",history + (long)local_c * 0x100);
    }
  }
  mvprintw(0,1,"                                                                        ");
  mvprintw(0,1,"Prompt>%s",param_1);
  wrefresh(stdscr);
  return;
}



void display_proc(undefined4 *param_1)

{
  size_t sVar1;
  
  mvprintw(4,2,"------------------------------PROCESADOR---------------------------------");
  mvprintw(5,2,"-                                                                       -");
  mvprintw(6,2,"-                                                                       -");
  mvprintw(7,2,"-                                                                       -");
  mvprintw(8,2,"-                                                                       -");
  mvprintw(9,2,"-                                                                       -");
  mvprintw(10,2,"-                                                                       -");
  mvprintw(0xb,2,"-------------------------------------------------------------------------");
  if (param_1 == (undefined4 *)0x0) {
    mvprintw(5,4,"AX:[--]          ");
    mvprintw(5,0x2a,"PC:[--]           ");
    mvprintw(6,4,"BX:[--]          ");
    mvprintw(6,0x2a,"IR:[--]           ");
    mvprintw(7,4,"CX:[--]          ");
    mvprintw(7,0x2a,"PID:[--]          ");
    mvprintw(8,4,"DX:[--]          ");
    mvprintw(8,0x2a,"NAME:[--]          ");
    mvprintw(9,4,"P:[--]            ");
    mvprintw(9,0x2a,"UID:[--]           ");
    mvprintw(10,4,"KCPU:[--]          ");
    mvprintw(10,0x2a,"KCPUxU:[--]          ");
  }
  else {
    if (*(char *)(param_1 + 10) != '\0') {
      sVar1 = strlen((char *)(param_1 + 10));
      if (*(char *)((long)param_1 + sVar1 + 0x27) == '\n') {
        sVar1 = strlen((char *)(param_1 + 10));
        *(undefined1 *)((long)param_1 + sVar1 + 0x27) = 0;
      }
    }
    mvprintw(5,4,"AX:[%d]",param_1[5]);
    mvprintw(5,0x2a,"PC:[%d]",param_1[9]);
    mvprintw(6,4,"BX:[%d]",param_1[6]);
    mvprintw(6,0x2a,"IR:[%s]",param_1 + 10);
    mvprintw(7,4,"CX:[%d]",param_1[7]);
    mvprintw(7,0x2a,"PID:[%d]",*param_1);
    mvprintw(8,4,"DX:[%d]",param_1[8]);
    mvprintw(8,0x2a,"NAME:[%s]",param_1 + 0x4a);
    mvprintw(9,4,"P:[%d]",param_1[2]);
    mvprintw(9,0x2a,"UID:[%d]",param_1[1]);
    mvprintw(10,4,"KCPU:[%d]",param_1[3]);
    mvprintw(10,0x2a,"KCPUxU:[%d]",param_1[4]);
  }
  wrefresh(stdscr);
  return;
}



undefined8 main(void)

{
  bool bVar1;
  int iVar2;
  char *__src;
  uint *puVar3;
  undefined8 uVar4;
  long in_FS_OFFSET;
  int local_530;
  char local_518 [256];
  char local_418 [256];
  char local_318 [256];
  char local_218 [520];
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  local_518[0] = ' ';
  local_518[1] = ' ';
  local_518[2] = 0;
  local_418[0] = '\0';
  Listos._0_4_ = 0;
  Listos._4_4_ = 0;
  Listos._8_4_ = 0;
  Listos._12_4_ = 0;
  Listos._16_4_ = 0;
  Listos._36_4_ = 0;
  Listos._20_4_ = 0;
  Listos._24_4_ = 0;
  Listos._28_4_ = 0;
  Listos._32_4_ = 0;
  Listos[0x28] = 0;
  Listos[0x128] = 0;
  Listos._552_8_ = 0;
  Listos._560_8_ = 0;
  Ejecucion._0_4_ = 0;
  Ejecucion._4_4_ = 0;
  Ejecucion._8_4_ = 0;
  Ejecucion._12_4_ = 0;
  Ejecucion._16_4_ = 0;
  Ejecucion._36_4_ = 0;
  Ejecucion._20_4_ = 0;
  Ejecucion._24_4_ = 0;
  Ejecucion._28_4_ = 0;
  Ejecucion._32_4_ = 0;
  Ejecucion[0x28] = 0;
  Ejecucion[0x128] = 0;
  Ejecucion._552_8_ = 0;
  Ejecucion._560_8_ = 0;
  Terminados._0_4_ = 0;
  Terminados._4_4_ = 0;
  Terminados._8_4_ = 0;
  Terminados._12_4_ = 0;
  Terminados._16_4_ = 0;
  Terminados._36_4_ = 0;
  Terminados._20_4_ = 0;
  Terminados._24_4_ = 0;
  Terminados._28_4_ = 0;
  Terminados._32_4_ = 0;
  Terminados[0x28] = 0;
  Terminados[0x128] = 0;
  Terminados._552_8_ = 0;
  Terminados._560_8_ = 0;
  local_530 = 0;
  bVar1 = false;
  initscr();
  wclear(stdscr);
  if (stdscr == 0) {
    MAXY = -1;
    MAXX = -1;
  }
  else {
    MAXY = *(short *)(stdscr + 4) + 1;
    MAXX = *(short *)(stdscr + 6) + 1;
  }
  display_msg(&DAT_001052e6,&DAT_001052e6,&DAT_001052e6);
  display_proc(0);
  do {
    shell_attention(local_418);
    display_prompt(local_418);
    imprimeListas();
    mvprintw(0x14,2,"                                                ");
    wrefresh(stdscr);
    mvprintw(0x14,2,"cnt:[%d] % Delay:[%d] = [%d]",cntDelay,cntMultiplo,
             (long)cntDelay % (long)cntMultiplo & 0xffffffff);
    wrefresh(stdscr);
    if (bVar1) {
      sleep(2);
      bVar1 = false;
    }
    if ((Ejecucion._560_8_ == 0) || (4 < local_530)) {
      if (Ejecucion._560_8_ == 0) {
        if (Listos._560_8_ != 0) {
          Ejecucion._560_8_ = listaExtraeMinP(Listos);
          MinP = *(undefined4 *)(Ejecucion._560_8_ + 8);
          actualizaW();
          if (Ejecucion._560_8_ != 0) {
            local_530 = 0;
          }
        }
      }
      else if (local_530 == 5) {
        uVar4 = listaExtraeInicio(Ejecucion);
        listaInsertarFinal(Listos,uVar4);
        listaActualizaInfoPlan(Listos);
        bVar1 = true;
      }
    }
    else if (cntDelay % cntMultiplo == 0) {
      do {
        iVar2 = sigLineaArch(Ejecucion._560_8_);
        strcpy(local_318,(char *)(Ejecucion._560_8_ + 0x28));
        __src = (char *)trim(local_318);
        strcpy(local_318,__src);
        strcpy((char *)(Ejecucion._560_8_ + 0x28),local_318);
        if (local_318[0] != '\0') break;
      } while (0 < iVar2);
      if (iVar2 < 1) {
        puVar3 = (uint *)listaExtraeInicio(Ejecucion);
        listaInsertarFinal(Terminados,puVar3);
        actualizaW();
        sprintf(local_218,&DAT_00105a48,(ulong)*puVar3,puVar3 + 0x4a);
        display_msg(0,0,local_218);
      }
      else {
        iVar2 = ejecuta_inst(Ejecucion._560_8_,Ejecucion._560_8_ + 0x28);
        if (iVar2 < 1) {
          uVar4 = listaExtraeInicio(Ejecucion);
          listaInsertarFinal(Terminados,uVar4);
          actualizaW();
        }
        local_530 = local_530 + 1;
      }
    }
    display_proc(Ejecucion._560_8_);
    iVar2 = strcmp(local_418,local_518);
    if (iVar2 != 0) {
      strcpy(local_518,local_418);
    }
    cntDelay = cntDelay + 1;
    iVar2 = strcmp(local_418,"EXIT\n");
    if (iVar2 == 0) {
      listaLibera(Listos);
      listaLibera(Ejecucion);
      listaLibera(Terminados);
      endwin();
      if (local_10 == *(long *)(in_FS_OFFSET + 0x28)) {
        return 1;
      }
                    // WARNING: Subroutine does not return
      __stack_chk_fail();
    }
  } while( true );
}



void __libc_csu_init(EVP_PKEY_CTX *param_1,undefined8 param_2,undefined8 param_3)

{
  long lVar1;
  
  _init(param_1);
  lVar1 = 0;
  do {
    (*(code *)(&__frame_dummy_init_array_entry)[lVar1])((ulong)param_1 & 0xffffffff,param_2,param_3)
    ;
    lVar1 = lVar1 + 1;
  } while (lVar1 != 1);
  return;
}



void __libc_csu_fini(void)

{
  return;
}



void _fini(void)

{
  return;
}

