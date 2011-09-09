/*
 * m68k virtual CPU header
 *
 *  Copyright (c) 2005-2007 CodeSourcery
 *  Written by Paul Brook
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */
#ifndef CPU_M68K_H
#define CPU_M68K_H

#define TARGET_LONG_BITS 32

#define CPUArchState struct CPUM68KState

#include "config.h"
#include "qemu-common.h"
#include "exec/cpu-defs.h"

#include "fpu/softfloat.h"

#define MAX_QREGS 32

#define TARGET_HAS_ICE 1

#define ELF_MACHINE	EM_68K

#define EXCP_ACCESS         2   /* Access (MMU) error.  */
#define EXCP_ADDRESS        3   /* Address error.  */
#define EXCP_ILLEGAL        4   /* Illegal instruction.  */
#define EXCP_DIV0           5   /* Divide by zero */
#define EXCP_CHK            6   /* CHK, CHK2 instruction */
#define EXCP_PRIVILEGE      8   /* Privilege violation.  */
#define EXCP_TRACE          9
#define EXCP_LINEA          10  /* Unimplemented line-A (MAC) opcode.  */
#define EXCP_LINEF          11  /* Unimplemented line-F (FPU) opcode.  */
#define EXCP_DEBUGNBP       12  /* Non-breakpoint debug interrupt.  */
#define EXCP_DEBEGBP        13  /* Breakpoint debug interrupt.  */
#define EXCP_FORMAT         14  /* RTE format error.  */
#define EXCP_UNINITIALIZED  15
#define EXCP_TRAP0          32   /* User trap #0.  */
#define EXCP_TRAP15         47   /* User trap #15.  */
#define EXCP_UNSUPPORTED    61
#define EXCP_ICE            13

#define EXCP_RTE            0x100
#define EXCP_HALT_INSN      0x101
#ifdef CONFIG_EMULOP
#define EXCP_EXEC_RETURN    0x20000
#endif

#define NB_MMU_MODES 2

#define TARGET_PHYS_ADDR_SPACE_BITS 32
#define TARGET_VIRT_ADDR_SPACE_BITS 32

#ifdef CONFIG_USER_ONLY
/* Linux uses 4k pages.  */
#define TARGET_PAGE_BITS 12
#else
/* Smallest TLB entry size is 1k.  */
#define TARGET_PAGE_BITS 10
#endif

enum {
    /* 1 bit to define user level / supervisor access */
    ACCESS_SUPER = 0x01,
    /* 1 bit to indicate direction */
    ACCESS_STORE = 0x02,
    /* Type of instruction that generated the access */
    ACCESS_CODE  = 0x10, /* Code fetch access                */
    ACCESS_INT   = 0x20, /* Integer load/store access        */
    ACCESS_FLOAT = 0x30, /* floating point load/store access */
};

typedef uint32_t CPUM68K_SingleU;
typedef uint64_t CPUM68K_DoubleU;

typedef struct {
    uint32_t high;
    uint64_t low;
} __attribute__((packed)) CPUM68K_XDoubleU;

typedef struct {
   uint8_t high[2];
   uint8_t low[10];
} __attribute__((packed)) CPUM68K_PDoubleU;

typedef CPU_LDoubleU FPReg;
#define PRIxFPH PRIx16
#define PRIxFPL PRIx64

typedef struct CPUM68KState {
    uint32_t dregs[8];
    uint32_t aregs[8];
    uint32_t pc;
    uint32_t sr;

    /* SSP and USP.  The current_sp is stored in aregs[7], the other here.  */
    int current_sp;
    uint32_t sp[3];

    /* Condition flags.  */
    uint32_t cc_op;
    uint32_t cc_dest;
    uint32_t cc_src;
    uint32_t cc_x;

    /* Control Registers */

    uint32_t sfc;
    uint32_t dfc;

    /* FPU Registers */

    FPReg fregs[8];
    uint32_t fpcr;
    uint32_t fpsr;
    float_status fp_status;

    uint64_t mactmp;
    /* EMAC Hardware deals with 48-bit values composed of one 32-bit and
       two 8-bit parts.  We store a single 64-bit value and
       rearrange/extend this when changing modes.  */
    uint64_t macc[4];
    uint32_t macsr;
    uint32_t mac_mask;

    /* Temporary storage for FPU */

    uint32_t fp0h;
    uint64_t fp0l;
    uint32_t fp1h;
    uint64_t fp1l;

    /* Temporary storage for DIV helpers.  */
    uint32_t div1;
    uint32_t div2;

    /* Upper 32 bits of a 64bit operand for quad MUL/DIV.  */
    uint32_t quadh;

    /* MMU status.  */
    struct {
        uint32_t ar;
        uint32_t ssw;
        /* 68040 */
        uint16_t tcr;
        uint32_t urp;
        uint32_t srp;
        uint32_t dttr0;
        uint32_t dttr1;
        uint32_t ittr0;
        uint32_t ittr1;
        uint32_t mmusr;
    } mmu;

    /* Control registers.  */
    uint32_t vbr;
    uint32_t mbar;
    uint32_t rambar0;
    uint32_t cacr;

    int pending_vector;
    int pending_level;

    uint32_t qregs[MAX_QREGS];

    CPU_COMMON

    uint32_t features;
} CPUM68KState;

#include "cpu-qom.h"

void m68k_tcg_init(void);
void m68k_cpu_init_gdb(M68kCPU *cpu);
M68kCPU *cpu_m68k_init(const char *cpu_model);
int cpu_m68k_exec(CPUM68KState *s);
void do_interrupt_m68k_hardirq(CPUM68KState *env1);
/* you can call this signal handler from your SIGBUS and SIGSEGV
   signal handlers to inform the virtual CPU of exceptions. non zero
   is returned if the signal was handled by the virtual CPU.  */
int cpu_m68k_signal_handler(int host_signum, void *pinfo,
                           void *puc);
void cpu_m68k_flush_flags(CPUM68KState *, int);

enum {
    CC_OP_DYNAMIC, /* Use env->cc_op  */
    CC_OP_FLAGS, /* CC_DEST = CVZN, CC_SRC = unused */
    CC_OP_LOGICB, /* CC_DEST = result, CC_SRC = unused */
    CC_OP_LOGICW, /* CC_DEST = result, CC_SRC = unused */
    CC_OP_LOGIC, /* CC_DEST = result, CC_SRC = unused */
    CC_OP_ADDB,   /* CC_DEST = result, CC_SRC = source */
    CC_OP_ADDW,   /* CC_DEST = result, CC_SRC = source */
    CC_OP_ADD,   /* CC_DEST = result, CC_SRC = source */
    CC_OP_SUBB,   /* CC_DEST = result, CC_SRC = source */
    CC_OP_SUBW,   /* CC_DEST = result, CC_SRC = source */
    CC_OP_SUB,   /* CC_DEST = result, CC_SRC = source */
    CC_OP_ADDXB,  /* CC_DEST = result, CC_SRC = source */
    CC_OP_ADDXW,  /* CC_DEST = result, CC_SRC = source */
    CC_OP_ADDX,  /* CC_DEST = result, CC_SRC = source */
    CC_OP_SUBXB,  /* CC_DEST = result, CC_SRC = source */
    CC_OP_SUBXW,  /* CC_DEST = result, CC_SRC = source */
    CC_OP_SUBX,  /* CC_DEST = result, CC_SRC = source */
    CC_OP_SHIFTB, /* CC_DEST = result, CC_SRC = carry */
    CC_OP_SHIFTW, /* CC_DEST = result, CC_SRC = carry */
    CC_OP_SHIFT, /* CC_DEST = result, CC_SRC = carry */
};

#define CCF_C 0x01
#define CCF_V 0x02
#define CCF_Z 0x04
#define CCF_N 0x08
#define CCF_X 0x10

#define SR_I_SHIFT 8
#define SR_I  0x0700
#define SR_M  0x1000
#define SR_S  0x2000
#define SR_T  0x8000

#define M68K_SSP    0
#define M68K_USP    1
#define M68K_ISP    2

/* bits for 68040 special status word */
#define M68K_CP_040  (0x8000)
#define M68K_CU_040  (0x4000)
#define M68K_CT_040  (0x2000)
#define M68K_CM_040  (0x1000)
#define M68K_MA_040  (0x0800)
#define M68K_ATC_040 (0x0400)
#define M68K_LK_040  (0x0200)
#define M68K_RW_040  (0x0100)
#define M68K_SIZ_040 (0x0060)
#define M68K_TT_040  (0x0018)
#define M68K_TM_040  (0x0007)

/* bits for 68040 write back status word */
#define M68K_WBV_040   (0x80)
#define M68K_WBSIZ_040 (0x60)
#define M68K_WBBYT_040 (0x20)
#define M68K_WBWRD_040 (0x40)
#define M68K_WBLNG_040 (0x00)
#define M68K_WBTT_040  (0x18)
#define M68K_WBTM_040  (0x07)

/* bus access size codes */
#define M68K_BA_SIZE_BYTE    (0x20)
#define M68K_BA_SIZE_WORD    (0x40)
#define M68K_BA_SIZE_LONG    (0x00)
#define M68K_BA_SIZE_LINE    (0x60)

/* bus access transfer type codes */
#define M68K_BA_TT_MOVE16    (0x08)

/* bits for 68040 MMU status register (mmusr) */
#define M68K_MMU_B_040   (0x0800)
#define M68K_MMU_G_040   (0x0400)
#define M68K_MMU_S_040   (0x0080)
#define M68K_MMU_CM_040  (0x0060)
#define M68K_MMU_M_040   (0x0010)
#define M68K_MMU_WP_040  (0x0004)
#define M68K_MMU_T_040   (0x0002)
#define M68K_MMU_R_040   (0x0001)

/* m68k Control Registers */

/* MC680[1234]0/CPU32 */

#define M68K_CR_SFC   0x000
#define M68K_CR_DFC   0x001
#define M68K_CR_USP   0x800
#define M68K_CR_VBR   0x801

/* MC680[234]0 */

#define M68K_CR_CACR  0x002
#define M68K_CR_CAAR  0x802 /* MC68020 and MC68030 only */
#define M68K_CR_MSP   0x803
#define M68K_CR_ISP   0x804

/* MC68040/MC68LC040 */

#define M68K_CR_TC    0x003
#define M68K_CR_ITT0  0x004
#define M68K_CR_ITT1  0x005
#define M68K_CR_DTT0  0x006
#define M68K_CR_DTT1  0x007
#define M68K_CR_MMUSR 0x805
#define M68K_CR_URP   0x806
#define M68K_CR_SRP   0x807

/* MC68EC040 */

#define M68K_CR_IACR0 0x004
#define M68K_CR_IACR1 0x005
#define M68K_CR_DACR0 0x006
#define M68K_CR_DACR1 0x007

#define FCCF_SHIFT 24
#define FCCF_MASK  (0xff << FCCF_SHIFT)
#define FCCF_A     (0x01 << FCCF_SHIFT) /* Not-A-Number */
#define FCCF_I     (0x02 << FCCF_SHIFT) /* Infinity */
#define FCCF_Z     (0x04 << FCCF_SHIFT) /* Zero */
#define FCCF_N     (0x08 << FCCF_SHIFT) /* Negative */

/* CACR fields are implementation defined, but some bits are common.  */
#define M68K_CACR_EUSP  0x10

#define MACSR_PAV0  0x100
#define MACSR_OMC   0x080
#define MACSR_SU    0x040
#define MACSR_FI    0x020
#define MACSR_RT    0x010
#define MACSR_N     0x008
#define MACSR_Z     0x004
#define MACSR_V     0x002
#define MACSR_EV    0x001

void m68k_set_irq_level(M68kCPU *cpu, int level, uint8_t vector);
void m68k_set_macsr(CPUM68KState *env, uint32_t val);
void m68k_switch_sp(CPUM68KState *env);

#define M68K_FPCR_PREC (1 << 6)

void do_m68k_semihosting(CPUM68KState *env, int nr);

/* There are 4 ColdFire core ISA revisions: A, A+, B and C.
   Each feature covers the subset of instructions common to the
   ISA revisions mentioned.  */

enum m68k_features {
    M68K_FEATURE_M68000,
    M68K_FEATURE_CF_ISA_A,
    M68K_FEATURE_CF_ISA_B, /* (ISA B or C).  */
    M68K_FEATURE_CF_ISA_APLUSC, /* BIT/BITREV, FF1, STRLDSR (ISA A+ or C).  */
    M68K_FEATURE_BRAL, /* Long unconditional branch.  (ISA A+ or B).  */
    M68K_FEATURE_CF_FPU,
    M68K_FEATURE_CF_MAC,
    M68K_FEATURE_CF_EMAC,
    M68K_FEATURE_CF_EMAC_B, /* Revision B EMAC (dual accumulate).  */
    M68K_FEATURE_USP, /* User Stack Pointer.  (ISA A+, B or C).  */
    M68K_FEATURE_EXT_FULL, /* 68020+ full extension word.  */
    M68K_FEATURE_WORD_INDEX, /* word sized address index registers.  */
    M68K_FEATURE_SCALED_INDEX, /* scaled address index registers.  */
    M68K_FEATURE_LONG_MULDIV,	/* 32 bit multiply/divide. */
    M68K_FEATURE_QUAD_MULDIV,	/* 64 bit multiply/divide. */
    M68K_FEATURE_BCCL,		/* Long conditional branches.  */
    M68K_FEATURE_BITFIELD,	/* Bit field insns.  */
    M68K_FEATURE_FPU,
    M68K_FEATURE_CAS
};

static inline int m68k_feature(CPUM68KState *env, int feature)
{
    return (env->features & (1u << feature)) != 0;
}

void m68k_cpu_list(FILE *f, fprintf_function cpu_fprintf);

void register_m68k_insns (CPUM68KState *env);

#ifdef CONFIG_USER_ONLY
/* Linux uses 4k pages.  */
#define TARGET_PAGE_BITS 12
#else
/* Smallest TLB entry size is 1k.  */
#define TARGET_PAGE_BITS 10
#endif

#define TARGET_PHYS_ADDR_SPACE_BITS 32
#define TARGET_VIRT_ADDR_SPACE_BITS 32

static inline CPUM68KState *cpu_init(const char *cpu_model)
{
    M68kCPU *cpu = cpu_m68k_init(cpu_model);
    if (cpu == NULL) {
        return NULL;
    }
    return &cpu->env;
}

#define cpu_exec cpu_m68k_exec
#define cpu_gen_code cpu_m68k_gen_code
#define cpu_signal_handler cpu_m68k_signal_handler
#define cpu_list m68k_cpu_list

/* MMU modes definitions */
#define MMU_MODE0_SUFFIX _kernel
#define MMU_MODE1_SUFFIX _user
#define MMU_KERNEL_IDX 0
#define MMU_USER_IDX 1
static inline int cpu_mmu_index (CPUM68KState *env)
{
    return (env->sr & SR_S) == 0 ? 1 : 0;
}

int cpu_m68k_handle_mmu_fault(CPUM68KState *env, target_ulong address, int rw,
                              int mmu_idx);
#define cpu_handle_mmu_fault cpu_m68k_handle_mmu_fault

#include "exec/cpu-all.h"

static inline void cpu_get_tb_cpu_state(CPUM68KState *env, target_ulong *pc,
                                        target_ulong *cs_base, int *flags)
{
    *pc = env->pc;
    *cs_base = 0;
    *flags = (env->fpcr & M68K_FPCR_PREC)       /* Bit  6 */
            | (env->sr & SR_S)                  /* Bit  13 */
            | ((env->macsr >> 4) & 0xf);        /* Bits 0-3 */
}

static inline bool cpu_has_work(CPUState *cpu)
{
    return cpu->interrupt_request & CPU_INTERRUPT_HARD;
}

#include "exec/exec-all.h"

#endif
