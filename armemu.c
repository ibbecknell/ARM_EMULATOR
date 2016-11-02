#include <stdbool.h>
#include <stdio.h>

#define uncond 14
#define NREGS 16
#define STACK_SIZE 1024
#define SP 13
#define LR 14
#define PC 15

int ldrb(char *string, char* sub);
int ldr_str(void);
int bl(void);
int find_str_a(char*str,char*sub);
int fib_rec_a(int n);
int sum_array_a(unsigned int *a, int n);
int find_max_a(unsigned int *a,int n);
int fib_iter_a(int n);
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
    int num_instructions;
    int num_DP;
    int num_Mem;
    int num_Br;
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

    as->num_instructions = 0;
    as->num_DP = 0;
    as->num_Mem = 0;
    as->num_Br = 0;
}

void print_metrics(struct arm_state *state){
  printf("-------------METRICS---------------\n");
  printf("Number of:\n");
  printf("  Branch instructions:  %d\n", state->num_Br);
  printf("  Memory instructions:  %d\n", state->num_Mem);
  printf("  Data Processing Instructions:  %d\n",state->num_DP);
  printf("TOTAL INSTRUCTIONS------------%d\n",state->num_instructions);
}

void print_regs(struct arm_state *state){
  int i;
  for(i = 0; i<NREGS; i++){
    printf("reg[%d]: %d\n", i, state->regs[i]);
  }
}

void print_stack(struct arm_state *state){
  int i;
  printf("STACK\n");
  for(i = 0; i<STACK_SIZE; i++){
    printf("stack[%d] = %d\n",i,state->stack[i]);
  }
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

bool is_add_inst(unsigned int iw){
    return get_inst(iw) == 0b0100;
}

bool is_ldr(unsigned int iw){
  return (iw >> 20) & 0b1 == 1;
}

bool is_bx_inst(unsigned int iw){
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
    bool can_proceed = false;
  
    if (cond_field == uncond){
        can_proceed = true;
    }
    if (cond_field == state->ne){
        can_proceed = true;
    }
    if(cond_field == state->eq){
        can_proceed = true;
    }
    if(cond_field == state->gt){
        can_proceed = true;
    }
    return can_proceed;
}

void armemu_cmp(struct arm_state *state){
    unsigned int iw;
    unsigned int rn, rm;
    int i;
    int src2;

    state->num_DP = state->num_DP + 1;
    
    clear_cpsr(state);
    iw = *((unsigned int *) state->regs[PC]);
    i = get_I_bit(iw);
    rn = get_Rn(iw);


    if(i == 0){
      src2 = state->regs[iw & 0xF];
    } else {
      src2 = iw & 0xFF;
    }
    
    if(state->regs[rn] == src2){
      state ->eq = 0;		 
    }
    if(state->regs[rn] > src2){
      state ->gt = 12;
    }
    if(state->regs[rn] != src2){
      state ->ne = 1;
    }

    state->regs[PC] = state->regs[PC] + 4;
}

void armemu_mov(struct arm_state *state){
    unsigned int iw;
    unsigned int rd, rn, rm;
    int i;
    iw = *((unsigned int *) state->regs[PC]);

    if(can_proceed(state,iw)){
        i = get_I_bit(iw);
        rd = get_Rd(iw);
        rn = get_Rn(iw);

        state->num_DP = state->num_DP;
    
        if(i == 0){
            rm = iw & 0xF;
	    if(get_inst(iw) == 15){
	        state ->regs[rm] = ~state->regs[rm];
            }
	    state->regs[rd] = state->regs[rm];
        } else {
            rm = iw & 0xFF;
            if(get_inst(iw) == 15){
                rm = ~rm;
            }
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
        state->num_DP = state->num_DP + 1;
    
        i = get_I_bit(iw);
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

void armemu_add(struct arm_state *state){
    unsigned int iw;
    unsigned int rd, rn, rm;
    int i;
    iw = *((unsigned int *) state->regs[PC]);
    if(can_proceed(state,iw)){
        i = get_I_bit(iw);
        rd = get_Rd(iw);
        rn = get_Rn(iw);

        state->num_DP = state->num_DP + 1;
    
        if(i == 0){
            rm = iw & 0xF;
            state->regs[rd] = state->regs[rn] + state->regs[rm];
        } else {
            int imm = iw & 0xFF;
            state->regs[rd] = state ->regs[rn] + imm;
        } 
    }
    state->regs[PC] = state->regs[PC] + 4;
}

void armemu_bx(struct arm_state *state){
    unsigned int iw;
    unsigned int rn;
    
    iw = *((unsigned int *) state->regs[PC]);
    rn = iw & 0b1111;

    state->num_Br = state->num_Br + 1;
    
    state->regs[PC] = state->regs[rn];
}

bool b_Bit_set(unsigned int iw){
    return (iw >> 22) & 0b1 == 1;
}

void armemu_ldrb(struct arm_state *state){
    unsigned int iw;
    unsigned int i,u, rd, rn, src2;
    unsigned int imm;
  
    iw = *((unsigned int *)state->regs[PC]);
    rn = (iw >> 16) & 0xF;
    rd = (iw >> 12) & 0xF;
    i = (iw >> 25) & 0b1;
    u = (iw >> 23) & 0b1;

    state->num_Mem = state->num_Mem + 1;
    
  
    if(i == 0){
        //immediate value
        src2 = (iw & 0xFF);
        //zero extend value
        imm = src2;
        unsigned int * tmp = (unsigned int *)(state->regs[rn] + imm);
        state->regs[rd] = (unsigned char)*tmp;
    } else {
        //register value
        src2 = (iw & 0xF);
        //add offset to base
        unsigned int * tmp = (unsigned int *)(state->regs[rn] + state->regs[src2]);
        state->regs[rd] = (unsigned char) *tmp;
    }
    state->regs[PC] = state->regs[PC]+4;
}

void armemu_ldr(struct arm_state *state){
    unsigned int iw;
    unsigned int i,rd, rn, src2;

    iw = *((unsigned int *) state->regs[PC]);
    rn = (iw >> 16) & 0xF;
    rd = (iw >> 12) & 0xF;
    i = (iw >> 25) & 0b1;

    state->num_Mem = state->num_Mem + 1;
    
    unsigned int *temp = (unsigned int *) state->regs[rn];
    state->regs[rd] =*temp;
  
    if (rd != PC) {
        state->regs[PC] = state->regs[PC] + 4;
    }
}  

void armemu_str(struct arm_state *state){
    unsigned int iw;
    unsigned int i,rd, rn, src2;

    state->num_Mem = state->num_Mem + 1;

    iw = *((unsigned int *) state->regs[PC]);
    rn = (iw >> 16) & 0xF;
    rd = (iw >> 12) & 0xF;
 
    unsigned int *temp = (unsigned int *) state->regs[rn];

    *temp = state->regs[rd];

    state->regs[PC] = state->regs[PC] + 4;
}

void check_Mem_inst(struct arm_state *state, unsigned int iw){
    if(is_ldr(iw)){
        if(b_Bit_set(iw)){
            armemu_ldrb(state);
        } else {
            armemu_ldr(state);
        }
    } else {
        armemu_str(state);
    }
}

bool is_mvn(unsigned int iw){
  return  get_inst(iw) == 0b1111;
}

void check_DP_inst(struct arm_state *state, unsigned int iw){
    if (is_cmp_inst(iw)) {
        armemu_cmp(state);
    } else if (is_add_inst(iw)) {
        armemu_add(state);
    } else if (is_sub_inst(iw)) {
        armemu_sub(state);
    } else if (is_mov_inst(iw) || is_mvn(iw)) {
        armemu_mov(state);
    }
}

void armemu_branch(struct arm_state *state){
    unsigned int iw;
    signed int imm;
    unsigned int new_BTA;
  
    iw = *((unsigned int *) state->regs[PC]);
    if(can_proceed(state,iw)){
        state->num_Br = state->num_Br + 1;

	imm = iw & 0xFFFFFF;
        unsigned int l_bit = (iw >> 24) & 0b1;
  
        if(imm  & 0x800000){
            new_BTA = 0xFF000000 + imm;
        } else {
           new_BTA = imm;
        }

        new_BTA = new_BTA << 2;
        state->regs[PC] = state ->regs[PC] + 8;
  
        if(l_bit == 1){
            state->regs[LR] = (state->regs[PC])-4;
        }
  
        int offset = state ->regs[PC] + new_BTA;
        state->regs[PC] = offset;
	
    } else {
        state->regs[PC] = state->regs[PC] + 4;
    }

}

void armemu_one(struct arm_state *state){
    unsigned int iw;
    iw = *((unsigned int *) state->regs[PC]);

    if(is_DP_inst(iw)){
      check_DP_inst(state, iw);
    } else if(is_Branch_inst(iw)){
        armemu_branch(state);
    } else if(is_Mem_inst(iw)){
       check_Mem_inst(state,iw);
    }
    if (is_bx_inst(iw)) {
       armemu_bx(state);
    }
}


unsigned int armemu(struct arm_state *state){
    while (state->regs[PC] != 0) {
        armemu_one(state);
    }
    state->num_instructions = state->num_DP + state->num_Mem + state->num_Br;
    return state->regs[0];
}


void test_sum_array(void){
    struct arm_state state;
    unsigned int r;
    int a[5] = {-1, -4,-6,-3,-9};
    int len = 5;
    int j;
    int thousand[1000];
    int b[5] = {1,2,5,4,5};
    int zero[5] = {0,0,0,0,0};

    for(j=0;j<1000;j++){
        thousand[j] = 2;
    }

    init_arm_state(&state, (unsigned int *)sum_array_a,(unsigned int) thousand,1000,0,0);
    r = armemu(&state);
  
    printf("\n");
    printf("\n==============Testing 1000 element array\n");
    printf("         Array with 1000 two's\n");
    printf("\nEmulator result = %d\n",r);
    printf("Assembly result = %d\n",sum_array_a(thousand,1000));
    printf("\n");
    print_metrics(&state);
  
    init_arm_state(&state, (unsigned int *)sum_array_a,(unsigned int) a,len,0,0);
    r = armemu(&state);
    printf("==============================================\n");
    printf("\n==============Testing negative-element array\n");
    printf("         Input: {-1,-4,-6,-3,-9}, length: %d\n",len);
    printf("\nEmulator result = %d\n",r);
    printf("Assembly result = %d\n", sum_array_a(a,len));
    printf("\n");
    print_metrics(&state);
    printf("==============================================\n");
    init_arm_state(&state, (unsigned int *)sum_array_a,(unsigned int) b,len,0,0);
    r = armemu(&state);
  
    printf("\n==============Testing positive-element array\n");
    printf("          Input {1,2,3,4,5}, length: %d\n",len);
    printf("\nEmulator result = %d\n", r);
    printf("Assembly result = %d\n", sum_array_a(b,len));
    printf("\n");
    print_metrics(&state);
    printf("==============================================\n");

    init_arm_state(&state, (unsigned int *)sum_array_a,(unsigned int) zero,len,0,0);
    r = armemu(&state);
    printf("\n");
    printf("\n==============Testing Zero element array\n");
    printf("          Input {0,0,0,0,0} length 5\n");
    printf("\nEmulator result = %d\n", r);
    printf("Assembly result = %d\n", sum_array_a(zero,len));
    printf("\n");
    print_metrics(&state);
    printf("==============================================\n");
}


void test_find_max(void){
    struct arm_state state;
    unsigned int r;
    int a[5] = {1,2,4,76,3};
    int len = 5;
    int max = 76;
    int b[5] = {-1, -4,-6,-3,-9};
    int j;
    int thousand[1000];
    int zero[5] = {0,0,0,0,0};
    for(j=0;j<1000;j++){
        thousand[j] = j;
    }

    init_arm_state(&state, (unsigned int *)find_max_a,(unsigned int) a,len,0,0);
    r = armemu(&state);

  
    printf("===============Testing positive-element array\n");
    printf("       Input: {1,2,4,76,3}\n");
    printf("Emulator result = %d\n",r);
    printf("Assembly result = %d\n",find_max_a(a,len));
    printf("\n");
    print_metrics(&state);
    printf("\n==============================================\n");

    init_arm_state(&state, (unsigned int *)find_max_a,(unsigned int) b,len,0,0);
    r = armemu(&state);
  
    printf("===============Testing negative-element array\n");
    printf("       Input: {-1,-4,-6,-3,-9}\n");
    printf("Emulator result = %d\n",r);
    printf("Assembly = %d\n", len, find_max_a(b,len));
    printf("\n");
    print_metrics(&state);
    printf("\n==============================================\n");

    len = 1000;
    init_arm_state(&state, (unsigned int *)find_max_a,(unsigned int) thousand,len,0,0);
    r = armemu(&state);
  
    printf("===============Testing 1000-element array of 0-999\n");
    printf("        Input: 1000 element array 0-999\n");
    printf("Emulator result = %d\n",r);
    printf("Assembly result = %d\n", find_max_a(thousand,len));
    printf("\n");
    print_metrics(&state);
    printf("\n==============================================\n");

    len = 5;
    init_arm_state(&state, (unsigned int *)find_max_a,(unsigned int) zero,len,0,0);
    r = armemu(&state);
  
    printf("===============Testing array of zeros\n");
    printf("       Input: {0,0,0,0,0}\n");
    printf("Emulator result = %d\n", r);
    printf("Assembly = %d\n", find_max_a(zero,len));
    printf("\n");
    print_metrics(&state);
    printf("\n==============================================\n");
}

void test_fib_rec(void){
    struct arm_state state;
    unsigned int r;
    int n = 3;
    int i;
    printf("Emulator: ");
    for(i=0;i<20; i++){
        init_arm_state(&state, (unsigned int *)fib_rec_a,i,0,0,0);
        r = armemu(&state);
        printf("%d", r);
        if(i<19){
            printf(", ");
        }
    }
    printf("\n");
    printf("Assembly: ");
    for(i=0;i<20;i++){
        printf("%d",fib_rec_a(i));
        if(i<19){
            printf(", ");
        }
    }
    printf("\n");
}

void test_fib_iter(void){
    struct arm_state state;
    unsigned int r;
    int i;
    int n = 3;

    printf("Emulator: ");
    for(i=0;i<20; i++){
        init_arm_state(&state, (unsigned int *)fib_iter_a,i,0,0,0);
        r = armemu(&state);
        printf("%d", r);
        if(i<19){
            printf(", ");
        }
    }
    printf("\n");
    printf("Assembly: ");
    for(i=0;i<20;i++){
        printf("%d",fib_iter_a(i));
        if(i<19){
            printf(", ");
        }
    }
    printf("\n");
}

void test_find_str(void){
    struct arm_state state;
    unsigned int r;
    char *string = "horsemouth";
    char *sub = "th";
    char *st = "charlie";
    char *su = "li";
    int in = find_str_a(string,sub);

    init_arm_state(&state, (unsigned int *)find_str_a,(unsigned int) string,(unsigned int)sub,0,0);
    r = armemu(&state);

    printf("\n\n==========Test 1:\n");
    printf("Emulator: first found %s in %s at index %d\n",sub,string,r);
    printf("Assembly: first found %s in %s at index %d\n",sub, string, in);
    print_metrics(&state);
    printf("\n=====================================");

    printf("\n\n==========Test 2:\n");
    init_arm_state(&state, (unsigned int *)find_str_a,(unsigned int) st,(unsigned int)su,0,0);
    r = armemu(&state);
    printf("Emulator: first found %s in %s at index %d\n",su, st, r);
    printf("Assembly: first found %s in %s at index %d\n",su, st, find_str_a(st,su));
    print_metrics(&state);
    printf("\n=====================================");

    printf("\n\n==========Test 3:\n");
    string = "cheese";
    sub = "ese";
    init_arm_state(&state, (unsigned int *)find_str_a,(unsigned int) string,(unsigned int)sub,0,0);
    r = armemu(&state);
    printf("Emulator: first found %s in %s at index %d\n","ese", "cheese", r);
    printf("Assembly: first found %s in %s at index %d\n","ese", "cheese", find_str_a("cheese","ese"));
    print_metrics(&state);
    printf("\n=====================================");


    printf("\n\n==========Test 4:\n");
    string = "dog";
    sub = "ese";
    init_arm_state(&state, (unsigned int *)find_str_a,(unsigned int) string,(unsigned int)sub,0,0);
    r = armemu(&state);
    printf("Emulator: first found %s in %s at index %d\n","ese", "dog", r);
    printf("Assembly: first found %s in %s at index %d\n","ese", "dog", find_str_a("dog","ese"));
    print_metrics(&state);
    printf("\n=====================================");

  
    printf("\n\n==========Test 5:\n");
    string = "cheese";
    sub = "es";
    init_arm_state(&state, (unsigned int *)find_str_a,(unsigned int) string,(unsigned int)sub,0,0);
    r = armemu(&state);
    printf("Emulator: first found %s in %s at index %d\n","es", "cheese",r);
    printf("Assembly: first found %s in %s at index %d\n","es", "cheese", find_str_a("cheese","es"));
    print_metrics(&state);
    printf("\n=====================================");

  
    printf("\n\n==========Test 6:\n");
    string = "dog";
    sub = "og";
    init_arm_state(&state, (unsigned int *)find_str_a,(unsigned int) string,(unsigned int)sub,0,0);
    r = armemu(&state);
    printf("Emulator: first found %s in %s at index %d\n","og", "dog",r);
    printf("Assembly: first found %s in %s at index %d\n","og", "dog", find_str_a("dog","og"));
    print_metrics(&state);
    printf("\n=====================================");

    printf("\n\n==========Test 7:\n");
    string = "doog";
    sub = "og";
    init_arm_state(&state, (unsigned int *)find_str_a,(unsigned int) string,(unsigned int)sub,0,0);
    r = armemu(&state);
    printf("Emulator: first found %s in %s at index %d\n","og", "doog", r);
    printf("Assembly: first found %s in %s at index %d\n","og", "doog", find_str_a("doog","og"));
    print_metrics(&state);
    printf("\n=====================================");

    printf("\n\n==========Test 8:\n");
    string = "doog";
    sub = "";
    init_arm_state(&state, (unsigned int *)find_str_a,(unsigned int) string,(unsigned int)sub,0,0);
    r = armemu(&state);
    printf("Emulator: first found %s in %s at index %d\n","", "doog", r);
    printf("Assembly: first found %s in %s at index %d\n","", "doog", find_str_a("doog",""));
    print_metrics(&state);
    printf("\n=====================================");
  
    printf("\n\n==========Test 9:\n");
    string = "ssdfsddogpoopsjfs";
    sub = "dogpoop";
    init_arm_state(&state, (unsigned int *)find_str_a,(unsigned int) string,(unsigned int)sub,0,0);
    r = armemu(&state);
    printf("Emulator: first found %s in %s at index %d\n","dogpoop", "ssdfsddogpoopsjfsd", r);
    printf("Assembly: first found %s in %s at index %d\n","dogpoop", "ssdfsddogpoopsjfsd", find_str_a("ssdfsddogpoopsjfs","dogpoop"));
    printf("\n");

    print_metrics(&state);
    printf("\n=====================================\n");
}
    
int main(int argc, char **argv){
    printf("------------Test sum_array------------\n");
    test_sum_array();
    printf("------------Test find_max------------\n");
    test_find_max();
    printf("------------Test fib_rec------------\n");
    test_fib_rec();
    printf("\n------------Test fib_iter------------\n");
    test_fib_iter();
    printf("\n------------Test find_str------------\n");
    test_find_str();

    return 0;
}
