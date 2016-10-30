#include <stdbool.h>
#include <stdio.h>

#define DP_op 0
#define MEM_op 1
#define BR_op 2
#define uncond 14
#define NE 1
#define EQ 0
#define GT 12
#define NREGS 16
#define STACK_SIZE 1024
#define SP 13
#define LR 14
int sum_array_a(unsigned int *a, int n);
#define PC 15

int branch(void);
int cmp(int a, int b);
int add(int a, int b);
int mov(void);
int sub(int a, int b);
int ldr(int *a);

struct arm_state {
    unsigned int regs[NREGS];
    unsigned int eq;
    unsigned int ne;
    unsigned int gt;
    unsigned char stack[STACK_SIZE];
};

void init_arm_state(struct arm_state *as, unsigned int *func,
                   unsigned int arg0, unsigned int arg1, unsigned int arg2, unsigned int arg3)
{
    int i;

    /* zero out all arm state */
    for (i = 0; i < NREGS; i++) {
        as->regs[i] = 0;
    }

    as->eq = 1;
    as->ne = 0;
    as->gt = 4;

    for (i = 0; i < STACK_SIZE; i++) {
        as->stack[i] = 0;
    }

    as->regs[PC] = (unsigned int) func;
    as->regs[SP] = (unsigned int) &as->stack[STACK_SIZE];
    as->regs[LR] = 0;

    as->regs[0] = arg0;
    as->regs[1] = arg1;
    as->regs[2] = arg2;
    as->regs[3] = arg3;
}

unsigned int get_opcode(unsigned int iw){
  return (iw >> 26) & 0b11;
}

bool is_DP_inst(unsigned int iw){
  return get_opcode(iw) == 0;
}

bool is_Mem_inst(unsigned int iw){
  return get_opcode(iw) == 1;
}

bool is_Branch_inst(unsigned int iw){
  return get_opcode(iw) == 2;
}

unsigned int get_cond_field(unsigned int iw){
  return (iw >> 28) & 0xF;
}

unsigned int get_inst(unsigned int iw){
  return (iw >> 21) & 0xF;
}

int get_I_bit(unsigned int iw){
  return (iw >> 25) & 0b1;
}

unsigned int get_Rd(unsigned int iw){
  return (iw >> 12) & 0xF;
}

unsigned int get_Rn(unsigned int iw){
  return (iw >> 16) & 0xF;
}

bool is_mov_inst(unsigned int iw){
  return get_inst(iw) == 0b1101;
}

bool is_sub_inst(unsigned int iw){
  return get_inst(iw) == 0b0010;
}

bool is_cmp_inst(unsigned int iw){
  return get_inst(iw) == 0b1010;
}

bool is_add_inst(unsigned int iw)
{
    return get_inst(iw) == 0b0100;
}

bool is_ldr(unsigned int iw){
  //printf("%d\n", (iw >> 20) & 0b1);
  return (iw >> 20) & 0b1 == 1;
}

bool is_bx_inst(unsigned int iw)
{
    unsigned int bx_code;

    bx_code = (iw >> 4) & 0x00FFFFFF;

    return (bx_code == 0b000100101111111111110001);
}

void clear_cpsr(struct arm_state *state){
    state ->ne = 0;
    state ->eq = 1;
    state ->gt = 4;
}

bool can_proceed(struct arm_state *state, unsigned int iw){
  unsigned int cond_field = get_cond_field(iw);
  //printf("     cond_field: %d\n", cond_field);
  // printf("     state ->ne: %d state->eq: %d state ->gt: %d\n",state->ne,state->eq,state->gt);
  bool can_proceed = false;
  if (cond_field == uncond){
    printf("      unconditional execution\n");
    can_proceed = true;
  } if ((cond_field == NE) && (state->ne == NE)){
    printf("      Not equal execution\n");
    can_proceed = true;
  } if((cond_field == EQ) && (state->eq == EQ)){
    printf("      Equal execution\n");
    can_proceed = true;
  }  if(cond_field == state->gt){
    printf("      Greater than execution\n");
    can_proceed = true;
  }
  return can_proceed;
}

void armemu_cmp(struct arm_state *state){
    unsigned int iw;
    unsigned int rn, rm;
    int i;
    int src2;

    clear_cpsr(state);
    iw = *((unsigned int *) state->regs[PC]);
    i = get_I_bit(iw);
    rn = get_Rn(iw);

    if(i == 0){
         src2 = state->regs[iw & 0xF];
    } else {
      src2 = iw & 0xFF;
    }
     printf("\n Comparing %d and  %d\n",state->regs[rn],src2);
     if(state->regs[rn] == src2){
      state ->eq = 0;		 
    }
    if(state->regs[rn] > src2){
      state ->gt = 12;
    }
    if(state->regs[rn] != src2){
      state ->ne = 1;
    }
    // printf("PC: %d\n", state->regs[PC]);
    state->regs[PC] = state->regs[PC] + 4;
}

void armemu_mov(struct arm_state *state){
    unsigned int iw;
    unsigned int rd, rn, rm;
    int i;
    iw = *((unsigned int *) state->regs[PC]);
    if(can_proceed(state,iw)){
    printf("\n    MOVE INSTRUCTION");
    i = get_I_bit(iw);
    rd = get_Rd(iw);
    rn = get_Rn(iw);

    if(i == 0){
        rm = iw & 0xF;
	    if(get_cond_field(iw) == 15){
	      state ->regs[rm] = ~state->regs[rm];
	    }
	printf("    moving %d into r%d\n",state->regs[rm],rd);
	state->regs[rd] = state->regs[rm];
    } else {
      rm = iw & 0xFF;
          if(get_cond_field(iw) == 15){
	    rm = ~rm;
	  }
	printf("    moving %d into r%d\n",rm,rd);
      state->regs[rd] = rm;
    }
    }

    if (rd != PC) {
        state->regs[PC] = state->regs[PC] + 4;
    }
}

void armemu_sub(struct arm_state *state){
    unsigned int iw;
    unsigned int rd, rn, rm;
    int i;
    iw = *((unsigned int *) state->regs[PC]);
    if(can_proceed(state,iw)){
    i = get_I_bit(iw);
    //printf("I: %d\n", i);
    rd = get_Rd(iw);
    rn = get_Rn(iw);
    if(i == 0){
        rm = iw & 0xF;
        state->regs[rd] = state->regs[rn] - state->regs[rm];
    } else {
      int imm = iw & 0xFF;
      state->regs[rd] = state ->regs[rn] - imm;
    }
    }
    if (rd != PC) {
        state->regs[PC] = state->regs[PC] + 4;
    }
}

void armemu_add(struct arm_state *state)
{
    unsigned int iw;
    unsigned int rd, rn, rm;
    int i;
    iw = *((unsigned int *) state->regs[PC]);
    if(can_proceed(state,iw)){
    i = get_I_bit(iw);
    printf("adding\n");
    rd = get_Rd(iw);
    rn = get_Rn(iw);
    if(i == 0){
        rm = iw & 0xF;
        state->regs[rd] = state->regs[rn] + state->regs[rm];
	
    } else {
      int imm = iw & 0xFF;
      state->regs[rd] = state ->regs[rn] + imm;
    }
    }
    if (rd != PC) {
        state->regs[PC] = state->regs[PC] + 4;
    }
}

void armemu_bx(struct arm_state *state)
{
    unsigned int iw;
    unsigned int rn;

    iw = *((unsigned int *) state->regs[PC]);
    rn = iw & 0b1111;

    state->regs[PC] = state->regs[rn];
}
bool b_Bit_set(unsigned int iw){
  return (iw >> 22) & 0b1 == 1;
}

void armemu_ldr(struct arm_state *state){
  unsigned int iw;
  unsigned int i,rd, rn, src2;

  iw = *((unsigned int *) state->regs[PC]);
  rn = (iw >> 16) & 0xF;
  rd = (iw >> 12) & 0xF;
  i = (iw >> 25) & 0b1;

  if(i == 0){
    src2 = iw & 0xFFF;
    printf("     loading rn (r%d): %d + imm: %d into rd (r%d)\n",rn,*(unsigned int *) state->regs[rn], src2,rd);
    state->regs[rd] = *(unsigned int *) state ->regs[rn] + src2;
  } else {
    src2 = iw & 0xF;
    printf("     loading rn (r%d): %d + r%d: %d into rd (r%d)\n",rn,    *(unsigned int *) state->regs[rn], src2,     state->regs[src2],rd);
    state->regs[rd] = *(unsigned int*)state->regs[rn] + state->regs[src2];
  }
  printf("r%d (rd) = %d)\n", rd,     state->regs[rd]);
   if (rd != PC) {
        state->regs[PC] = state->regs[PC] + 4;
    }
  
}  

void check_Mem_inst(struct arm_state *state, unsigned int iw){
  if(is_ldr(iw)){
    if(b_Bit_set(iw)){
      printf("\n          ======ldrb======\n");
      printf("       ldrb instruction\n");
      // armemu_ldrb(state);
      printf("          ================\n");
    }else {
      printf("\n          =======ldr=======\n");
      printf("       is ldr instruction\n");
      armemu_ldr(state);
      printf("\n          =================\n");
    }
    //state ->regs[PC] = state->regs[PC]+4;
  }
  else if(is_bx_inst(iw)) {
        armemu_bx(state);
  }
  else{
    printf("      is str instruction\n");
    // if(check_B_bit(iw)){
    // armemu_strb(state);
    //} else {
    //armemu_str(state);
    //}
    state ->regs[PC] = state->regs[PC]+4;
  }
    
}

bool is_mvn(unsigned int iw){
  return  get_cond_field(iw) == 15;
}

void check_DP_inst(struct arm_state *state, unsigned int iw){
  if (is_cmp_inst(iw)) {
    printf("====Compare====\n");
      armemu_cmp(state);
      printf(" gt: %d\n ne: %d\n eq: %d\n",state->gt,state->ne,state->eq);
      printf("===============\n");
  }
  else if (is_bx_inst(iw)) {
    printf("\n          ======Bx======\n");
    armemu_bx(state);
    printf("          ==============\n");
  } else if (is_add_inst(iw)) {
       printf("\n          ====Add====\n");
        armemu_add(state);
	printf("           ===========\n");
    } else if (is_sub_inst(iw)) {
       printf("\n          ====Sub====\n");
      armemu_sub(state);
      printf("          ===========\n");
  } else if (is_mov_inst(iw) || is_mvn(iw)) {
       printf("\n          ====Mov====\n");
      armemu_mov(state);
      printf("          ===========\n");
    }
  
}

void armemu_branch(struct arm_state *state){
  unsigned int iw;
  signed int imm;
  unsigned int new_BTA;

  iw = *((unsigned int *) state->regs[PC]);
  if(can_proceed(state,iw)){
  imm = iw & 0xFFFFFF;

  printf("imm: %d\n", imm);
  if(imm  & 0x800000){
     new_BTA = 0xFF000000 + imm;
  } else{
    new_BTA = imm;
  }

  new_BTA = new_BTA << 2;
  printf("new_BTA = %d\n",new_BTA);
  state->regs[PC] = state ->regs[PC] + 8;

  int offset = state ->regs[PC] + new_BTA;
  printf("offset = %d\n", offset);
 
  printf("PC:%d\n", state->regs[PC]);

  if(new_BTA > state->regs[PC]){
    printf("branch backwards\n");
    state->regs[PC] = offset;
  }else{
    printf("branch forward\n");
    state->regs[PC] = offset;
  }
  printf("branch to %d",state->regs[PC]);
  }else{
  state->regs[PC] = state->regs[PC] + 4;
  
  }

}
void armemu_bl(struct arm_state *state){
  unsigned int iw;
  iw = *((unsigned int *) state ->regs[PC]);
  if(can_proceed(state,iw)){
      printf("doing bl stuff\n");
  }
  else {
      state->regs[PC] = state->regs[PC]+4;
  }	   
	 }

void check_BR_inst(struct arm_state *state, unsigned int iw){
  unsigned int l_bit;
  l_bit = (iw >> 24) & 0b1;
  //if(can_proceed(state, iw)){
    //printf("canProceed\n");
    if(l_bit == 0){
    printf("      branch instruction\n");
        printf("l_bit: %d\n",l_bit);
    armemu_branch(state);
  } else {
    printf("l_bit: %d\n",l_bit);
    printf("      branch and link\n");
    //}
  }
  // state->regs[PC] = state->regs[PC] + 4;
  }


void armemu_one(struct arm_state *state)
{
     unsigned int iw;
    
     iw = *((unsigned int *) state->regs[PC]);

     //if(is_DP_inst(iw)){
       // printf("\n----DATA PROCESSING---\n");
       printf("cond_field: %d\n", get_cond_field(iw));
       printf("function: %d\n", get_inst(iw));
	// check_DP_inst(state, iw);
        if (is_cmp_inst(iw)) {
            printf("====Compare====\n");
            armemu_cmp(state);
            printf(" gt: %d\n ne: %d\n eq: %d\n",state->gt,state->ne,state->eq);
            printf("===============\n");
        } else if (is_add_inst(iw)) {
            printf("\n          ====Add====\n");
            armemu_add(state);
	    printf("           ===========\n");
        } else if (is_sub_inst(iw)) {
            printf("\n          ====Sub====\n");
            armemu_sub(state);
            printf("          ===========\n");
        } else if (is_mov_inst(iw) || is_mvn(iw)) {
            printf("\n          ====Mov====\n");
            armemu_mov(state);
            printf("          ===========\n");
        
	
	//printf("\n----------------------\n");
      } else if(is_Branch_inst(iw)){
      printf("\n-------BRANCH--------\n");
      check_BR_inst(state,iw);
      printf("\n----------------------\n");
      } else if(is_Mem_inst(iw)){
      //printf("\n--------MEMORY-------\n");
      check_Mem_inst(state,iw);
      //printf("\n----------------------\n");	   
 }
if (is_bx_inst(iw)) {
        armemu_bx(state);
    }
     // state->regs[PC] = state->regs[PC]+4;
}


unsigned int armemu(struct arm_state *state)
{

    while (state->regs[PC] != 0) {
        armemu_one(state);
    }

    return state->regs[0];
}
                  
    
int main(int argc, char **argv)
{
    struct arm_state state;
    unsigned int r;
    unsigned int a[5] = {1,2,3,4,5};
    //init_arm_state(&state, (unsigned int *) add, 1,2,0,0);
    // init_arm_state(&state, (unsigned int *) sub, 1, 2, 0, 0);
    //  init_arm_state(&state, (unsigned int *) mov, 0,0,0,0);
    // init_arm_state(&state, (unsigned int *) cmp,3,1,0,0);
    init_arm_state(&state, (unsigned int *)sum_array_a,(unsigned int) a,5,0,0);
    //init_arm_state(&state, (unsigned int *)ldr,(unsigned int)a,5,0,0);
    // init_arm_state(&state, (unsigned int *)branch,0,0,0,0);
    r = armemu(&state);
    printf("r = %d\n", r);
  
    return 0;
}
