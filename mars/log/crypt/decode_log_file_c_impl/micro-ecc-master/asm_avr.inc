/* Copyright 2015, Kenneth MacKay. Licensed under the BSD 2-clause license. */

#ifndef _UECC_ASM_AVR_H_
#define _UECC_ASM_AVR_H_

#if (uECC_SUPPORTS_secp256r1 || uECC_SUPPORTS_secp256k1)
    #define uECC_MIN_WORDS 32
#endif
#if uECC_SUPPORTS_secp224r1
    #undef uECC_MIN_WORDS
    #define uECC_MIN_WORDS 28
#endif
#if uECC_SUPPORTS_secp192r1
    #undef uECC_MIN_WORDS
    #define uECC_MIN_WORDS 24
#endif
#if uECC_SUPPORTS_secp160r1
    #undef uECC_MIN_WORDS
    #define uECC_MIN_WORDS 20
#endif

#if __AVR_HAVE_EIJMP_EICALL__
    #define IJMP "eijmp \n\t"
#else
    #define IJMP "ijmp \n\t"
#endif

#if (uECC_OPTIMIZATION_LEVEL >= 2)

uECC_VLI_API void uECC_vli_clear(uECC_word_t *vli, wordcount_t num_words) {
    volatile uECC_word_t *v = vli;
    __asm__ volatile (
    #if (uECC_MAX_WORDS != uECC_MIN_WORDS)
        "ldi r30, pm_lo8(1f) \n\t"
        "ldi r31, pm_hi8(1f) \n\t"
        "sub r30, %[num] \n\t"
        "sbc r31, __zero_reg__ \n\t"
        IJMP
    #endif
        
        REPEAT(uECC_MAX_WORDS, "st x+, __zero_reg__ \n\t")
        "1: \n\t"
        : "+x" (v)
        : [num] "r" (num_words)
        : 
    #if (uECC_MAX_WORDS != uECC_MIN_WORDS)
          "r30", "r31", "cc"
    #endif
    );
}
#define asm_clear 1

uECC_VLI_API void uECC_vli_set(uECC_word_t *dest, const uECC_word_t *src, wordcount_t num_words) {
    volatile uECC_word_t *d = dest;
    __asm__ volatile (
    #if (uECC_MAX_WORDS != uECC_MIN_WORDS)
        "ldi r30, pm_lo8(1f) \n\t"
        "ldi r31, pm_hi8(1f) \n\t"
        "sub r30, %[num] \n\t"
        "sbc r31, __zero_reg__ \n\t"
        IJMP
    #endif
        
        REPEAT(uECC_MAX_WORDS,
            "ld r0, y+ \n\t"
            "st x+, r0 \n\t")
        "1: \n\t"
        : "+x" (d), "+y" (src)
        : [num] "r" ((uint8_t)(num_words * 2))
        : "r0",
    #if (uECC_MAX_WORDS != uECC_MIN_WORDS)
          "r30", "r31", "cc"
    #endif
    );
}
#define asm_set 1

uECC_VLI_API void uECC_vli_rshift1(uECC_word_t *vli, wordcount_t num_words) {
    volatile uECC_word_t *v = vli;
    __asm__ volatile (
    #if (uECC_MAX_WORDS != uECC_MIN_WORDS)
        "ldi r30, pm_lo8(1f) \n\t"
        "ldi r31, pm_hi8(1f) \n\t"
        "sub r30, %[jump] \n\t"
        "sbc r31, __zero_reg__ \n\t"
    #endif
        
        "add r26, %[num] \n\t"
        "adc r27, __zero_reg__ \n\t"
        "ld r0, -x \n\t"
        "lsr r0 \n\t"
        "st x, r0 \n\t"
    #if (uECC_MAX_WORDS != uECC_MIN_WORDS)
        IJMP
    #endif
        
        REPEAT(DEC(uECC_MAX_WORDS),
            "ld r0, -x \n\t"
            "ror r0 \n\t"
            "st x, r0 \n\t")
        "1: \n\t"
        : "+x" (v)
    #if (uECC_MAX_WORDS != uECC_MIN_WORDS)
        : [num] "r" (num_words), [jump] "r" ((uint8_t)(3 * (num_words - 1)))
        : "r0", "r30", "r31", "cc"
    #else
        : [num] "r" (num_words)
        : "r0", "cc"
    #endif
    );
}
#define asm_rshift1 1

#define ADD_RJPM_TABLE(N)       \
    "movw r30, %A[result] \n\t" \
    "rjmp add_%=_" #N " \n\t"

#define ADD_RJPM_DEST(N)     \
    "add_%=_" #N ":"         \
    "ld %[clb], x+ \n\t"     \
    "ld %[rb], y+ \n\t"      \
    "adc %[clb], %[rb] \n\t" \
    "st z+, %[clb] \n\t"

uECC_VLI_API uECC_word_t uECC_vli_add(uECC_word_t *result,
                                      const uECC_word_t *left,
                                      const uECC_word_t *right,
                                      wordcount_t num_words) {
    volatile uECC_word_t *r = result;
    uint8_t carry;
    uint8_t right_byte;

    __asm__ volatile (
    #if (uECC_MAX_WORDS != uECC_MIN_WORDS)
        "ldi r30, pm_lo8(add_%=_" STR(uECC_MAX_WORDS) ") \n\t"
        "ldi r31, pm_hi8(add_%=_" STR(uECC_MAX_WORDS) ") \n\t"
        "sub r30, %[num] \n\t"
        "sbc r31, __zero_reg__ \n\t"
    #endif
    
        "clc \n\t"
    #if (uECC_MAX_WORDS != uECC_MIN_WORDS)
        IJMP
        REPEATM(uECC_MAX_WORDS, ADD_RJPM_TABLE)
    #endif
    
        REPEATM(uECC_MAX_WORDS, ADD_RJPM_DEST)
        
        "mov %[clb], __zero_reg__ \n\t"
        "adc %[clb], %[clb] \n\t" /* Store carry bit. */

        : "+x" (left), "+y" (right),
          [clb] "=&r" (carry), [rb] "=&r" (right_byte)
        : [result] "r" (r), [num] "r" ((uint8_t)(num_words * 2))
        : "r30", "r31", "cc"
    );
    return carry;
}
#define asm_add 1

#define SUB_RJPM_TABLE(N)       \
    "movw r30, %A[result] \n\t" \
    "rjmp sub_%=_" #N " \n\t"

#define SUB_RJPM_DEST(N)     \
    "sub_%=_" #N ":"         \
    "ld %[clb], x+ \n\t"     \
    "ld %[rb], y+ \n\t"      \
    "sbc %[clb], %[rb] \n\t" \
    "st z+, %[clb] \n\t"

uECC_VLI_API uECC_word_t uECC_vli_sub(uECC_word_t *result,
                                      const uECC_word_t *left,
                                      const uECC_word_t *right,
                                      wordcount_t num_words) {
    volatile uECC_word_t *r = result;
    uint8_t carry;
    uint8_t right_byte;

    __asm__ volatile (
    #if (uECC_MAX_WORDS != uECC_MIN_WORDS)
        "ldi r30, pm_lo8(sub_%=_" STR(uECC_MAX_WORDS) ") \n\t"
        "ldi r31, pm_hi8(sub_%=_" STR(uECC_MAX_WORDS) ") \n\t"
        "sub r30, %[num] \n\t"
        "sbc r31, __zero_reg__ \n\t"
    #endif
    
        "clc \n\t"
    #if (uECC_MAX_WORDS != uECC_MIN_WORDS)
        IJMP
        REPEATM(uECC_MAX_WORDS, SUB_RJPM_TABLE)
    #endif
    
        REPEATM(uECC_MAX_WORDS, SUB_RJPM_DEST)
        
        "mov %[clb], __zero_reg__ \n\t"
        "adc %[clb], %[clb] \n\t" /* Store carry bit. */

        : "+x" (left), "+y" (right),
          [clb] "=&r" (carry), [rb] "=&r" (right_byte)
        : [result] "r" (r), [num] "r" ((uint8_t)(num_words * 2))
        : "r30", "r31", "cc"
    );
    return carry;
}
#define asm_sub 1

#if (uECC_OPTIMIZATION_LEVEL >= 3)

#include "asm_avr_mult_square.inc"

__attribute((noinline))
uECC_VLI_API void uECC_vli_mult(uECC_word_t *result,
                                const uECC_word_t *left,
                                const uECC_word_t *right,
                                wordcount_t num_words) {
    /* num_words should already be in r18. */
    register wordcount_t r18 __asm__("r18") = num_words;
    
    __asm__ volatile (
        "push r18 \n\t"
#if (uECC_MIN_WORDS == 20)
        FAST_MULT_ASM_20
        "pop r18 \n\t"
    #if (uECC_MAX_WORDS > 20)
        FAST_MULT_ASM_20_TO_24
    #endif
    #if (uECC_MAX_WORDS > 24)
        FAST_MULT_ASM_24_TO_28
    #endif
    #if (uECC_MAX_WORDS > 28)
        FAST_MULT_ASM_28_TO_32
    #endif
#elif (uECC_MIN_WORDS == 24)
        FAST_MULT_ASM_24
        "pop r18 \n\t"
    #if (uECC_MAX_WORDS > 24)
        FAST_MULT_ASM_24_TO_28
    #endif
    #if (uECC_MAX_WORDS > 28)
        FAST_MULT_ASM_28_TO_32
    #endif
#elif (uECC_MIN_WORDS == 28)
        FAST_MULT_ASM_28
        "pop r18 \n\t"
    #if (uECC_MAX_WORDS > 28)
        FAST_MULT_ASM_28_TO_32
    #endif
#elif (uECC_MIN_WORDS == 32)
        FAST_MULT_ASM_32
        "pop r18 \n\t"
#endif
        "2: \n\t"
        "eor r1, r1 \n\t"
        : "+x" (left), "+y" (right), "+z" (result)
        : "r" (r18)
        : "r0", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10",
          "r11", "r12", "r13", "r14", "r15", "r16", "r17", "r19", "r20",
          "r21", "r22", "r23", "r24", "r25", "cc"
    );
}
#define asm_mult 1

#if uECC_SQUARE_FUNC
__attribute((noinline))
uECC_VLI_API void uECC_vli_square(uECC_word_t *result,
                                  const uECC_word_t *left,
                                  wordcount_t num_words) {
    /* num_words should already be in r20. */
    register wordcount_t r20 __asm__("r20") = num_words;
    
    __asm__ volatile (
        "push r20 \n\t"
#if (uECC_MIN_WORDS == 20)
        FAST_SQUARE_ASM_20
        "pop r20 \n\t"
    #if (uECC_MAX_WORDS > 20)
        FAST_SQUARE_ASM_20_TO_24
    #endif
    #if (uECC_MAX_WORDS > 24)
        FAST_SQUARE_ASM_24_TO_28
    #endif
    #if (uECC_MAX_WORDS > 28)
        FAST_SQUARE_ASM_28_TO_32
    #endif
#elif (uECC_MIN_WORDS == 24)
        FAST_SQUARE_ASM_24
        "pop r20 \n\t"
    #if (uECC_MAX_WORDS > 24)
        FAST_SQUARE_ASM_24_TO_28
    #endif
    #if (uECC_MAX_WORDS > 28)
        FAST_SQUARE_ASM_28_TO_32
    #endif
#elif (uECC_MIN_WORDS == 28)
        FAST_SQUARE_ASM_28
        "pop r20 \n\t"
    #if (uECC_MAX_WORDS > 28)
        FAST_SQUARE_ASM_28_TO_32
    #endif
#elif (uECC_MIN_WORDS == 32)
        FAST_SQUARE_ASM_32
        "pop r20 \n\t"
#endif
        "2: \n\t"
        "eor r1, r1 \n\t"
        : "+x" (left), "+z" (result)
        : "r" (r20)
        : "r0", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10",
          "r11", "r12", "r13", "r14", "r15", "r16", "r17", "r18", "r19",
          "r21", "r22", "r23", "r24", "r25", "r28", "r29", "cc"
    );
}
#define asm_square 1
#endif /* uECC_SQUARE_FUNC */

#endif /* (uECC_OPTIMIZATION_LEVEL >= 3) */

#if uECC_SUPPORTS_secp160r1
static const struct uECC_Curve_t curve_secp160r1;
static void vli_mmod_fast_secp160r1(uECC_word_t *result, uECC_word_t *product) {
    uint8_t carry = 0;
    __asm__ volatile (
        "in r30, __SP_L__ \n\t"
    	"in r31, __SP_H__ \n\t"
    	"sbiw r30, 24 \n\t"
    	"in r0, __SREG__ \n\t"
    	"cli \n\t"
    	"out __SP_H__, r31 \n\t"
    	"out __SREG__, r0 \n\t"
    	"out __SP_L__, r30 \n\t"
    	
    	"adiw r30, 25 \n\t" /* we are shifting by 31 bits, so shift over 4 bytes
    	                       (+ 1 since z initially points below the stack) */
        "adiw r26, 40 \n\t" /* end of product */
        "ld r18, -x \n\t"   /* Load word. */
        "lsr r18 \n\t"      /* Shift. */
        "st -z, r18 \n\t"   /* Store the first result word. */

        /* Now we just do the remaining words with the carry bit (using ROR) */
        REPEAT(19,
            "ld r18, -x \n\t"
            "ror r18 \n\t"
            "st -z, r18 \n\t")

        "eor r18, r18 \n\t" /* r18 = 0 */
        "ror r18 \n\t"      /* get last bit */
        "st -z, r18 \n\t"   /* store it */

        "sbiw r30, 3 \n\t" /* move z back to point at tmp */
        /* now we add right */
        "ld r18, x+ \n\t"
        "st z+, r18 \n\t" /* the first 3 bytes do not need to be added */
        "ld r18, x+ \n\t"
        "st z+, r18 \n\t"
        "ld r18, x+ \n\t"
        "st z+, r18 \n\t"

        "ld r18, x+ \n\t"
        "ld r19, z \n\t"
        "add r18, r19 \n\t"
        "st z+, r18 \n\t"

        /* Now we just do the remaining words with the carry bit (using ADC) */
        REPEAT(16,
            "ld r18, x+ \n\t"
            "ld r19, z \n\t"
            "adc r18, r19 \n\t"
            "st z+, r18 \n\t")

        /* Propagate over the remaining bytes of result */
        "ld r18, z \n\t"
        "adc r18, r1 \n\t"
        "st z+, r18 \n\t"

        "ld r18, z \n\t"
        "adc r18, r1 \n\t"
        "st z+, r18 \n\t"

        "ld r18, z \n\t"
        "adc r18, r1 \n\t"
        "st z+, r18 \n\t"

        "ld r18, z \n\t"
        "adc r18, r1 \n\t"
        "st z+, r18 \n\t"
        
        "sbiw r30, 24 \n\t" /* move z back to point at tmp */
        "sbiw r26, 40 \n\t" /* move x back to point at product */
        
        /* add low bytes of tmp to product, storing in result */
        "ld r18, z+ \n\t"
        "ld r19, x+ \n\t"
        "add r18, r19 \n\t"
        "st y+, r18 \n\t"
        REPEAT(19,
            "ld r18, z+ \n\t"
            "ld r19, x+ \n\t"
            "adc r18, r19 \n\t"
            "st y+, r18 \n\t")
        "adc %[carry], __zero_reg__ \n\t" /* Store carry bit (carry flag is cleared). */
        /* at this point x is at the end of product, y is at the end of result,
           z is 20 bytes into tmp */
        "sbiw r28, 20 \n\t" /* move y back to point at result */
        "adiw r30, 4 \n\t"  /* move z to point to the end of tmp */
        
        /* do omega_mult again with the 4 relevant bytes */
        /* z points to the end of tmp, x points to the end of product */
        "ld r18, -z \n\t" /* Load word. */
        "lsr r18 \n\t"    /* Shift. */
        "st -x, r18 \n\t" /* Store the first result word. */
        
        "ld r18, -z \n\t"
        "ror r18 \n\t"
        "st -x, r18 \n\t"
        "ld r18, -z \n\t"
        "ror r18 \n\t"
        "st -x, r18 \n\t"
        "ld r18, -z \n\t"
        "ror r18 \n\t"
        "st -x, r18 \n\t"
        
        "eor r18, r18 \n\t" /* r18 = 0 */
        "ror r18 \n\t"      /* get last bit */
        "st -x, r18 \n\t"   /* store it */
        
        "sbiw r26, 3 \n\t" /* move x back to point at beginning */
        /* now we add a copy of the 4 bytes */
        "ld r18, z+ \n\t"
        "st x+, r18 \n\t" /* the first 3 bytes do not need to be added */
        "ld r18, z+ \n\t"
        "st x+, r18 \n\t"
        "ld r18, z+ \n\t"
        "st x+, r18 \n\t"
        
        "ld r18, z+ \n\t"
        "ld r19, x \n\t"
        "add r18, r19 \n\t"
        "st x+, r18 \n\t"
        
        /* Propagate over the remaining bytes */
        "ld r18, x \n\t"
        "adc r18, r1 \n\t"
        "st x+, r18 \n\t"
        
        "ld r18, x \n\t"
        "adc r18, r1 \n\t"
        "st x+, r18 \n\t"
        
        "ld r18, x \n\t"
        "adc r18, r1 \n\t"
        "st x+, r18 \n\t"
        
        "ld r18, x \n\t"
        "adc r18, r1 \n\t"
        "st x+, r18 \n\t"
        
        /* now z points to the end of tmp, x points to the end of product
           (y still points at result) */
        "sbiw r26, 8 \n\t" /* move x back to point at beginning of actual data */
        /* add into result */
        "ld r18, x+ \n\t"
        "ld r19, y \n\t"
        "add r18, r19 \n\t"
        "st y+, r18 \n\t"
        REPEAT(7,
            "ld r18, x+ \n\t"
            "ld r19, y \n\t"
            "adc r18, r19 \n\t"
            "st y+, r18 \n\t")
        
        /* Done adding, now propagate carry bit */
        REPEAT(12,
            "ld r18, y \n\t"
            "adc r18, __zero_reg__ \n\t"
            "st y+, r18 \n\t")
        
        "adc %[carry], __zero_reg__ \n\t" /* Store carry bit (carry flag is cleared). */
        "sbiw r28, 20 \n\t" /* move y back to point at result */
        
        "sbiw r30, 1 \n\t" /* fix stack pointer */
    	"in r0, __SREG__ \n\t"
    	"cli \n\t"
    	"out __SP_H__, r31 \n\t"
    	"out __SREG__, r0 \n\t"
    	"out __SP_L__, r30 \n\t"
        
        : "+x" (product), [carry] "+r" (carry)
        : "y" (result)
        : "r0", "r18", "r19", "r30", "r31", "cc"
    );

    if (carry > 0) {
        --carry;
        uECC_vli_sub(result, result, curve_secp160r1.p, 20);
    }
    if (carry > 0) {
        uECC_vli_sub(result, result, curve_secp160r1.p, 20);
    }
    if (uECC_vli_cmp_unsafe(result, curve_secp160r1.p, 20) > 0) {
        uECC_vli_sub(result, result, curve_secp160r1.p, 20);
    }
}
#define asm_mmod_fast_secp160r1 1
#endif /* uECC_SUPPORTS_secp160r1 */

#if uECC_SUPPORTS_secp256k1
static const struct uECC_Curve_t curve_secp256k1;
static void vli_mmod_fast_secp256k1(uECC_word_t *result, uECC_word_t *product) {
    uint8_t carry = 0;
    __asm__ volatile (
        "in r30, __SP_L__ \n\t"
    	"in r31, __SP_H__ \n\t"
    	"sbiw r30, 37 \n\t"
    	"in r0, __SREG__ \n\t"
    	"cli \n\t"
    	"out __SP_H__, r31 \n\t"
    	"out __SREG__, r0 \n\t"
    	"out __SP_L__, r30 \n\t"
    	
    	"adiw r30, 1 \n\t"  /* add 1 since z initially points below the stack */
        "adiw r26, 32 \n\t" /* product + uECC_WORDS */
        "ldi r25, 0x03 \n\t"
        "ldi r24, 0xD1 \n\t"
        "ld r18, x+ \n\t"
        "ld r19, x+ \n\t"
        "ld r20, x+ \n\t"
        "ld r21, x+ \n\t"
        
        "mul r24, r18 \n\t"
        "st z+, r0 \n\t"
        "mov r22, r1 \n\t"
        "ldi r23, 0 \n\t"
        
        "mul r24, r19 \n\t"
        "add r22, r0 \n\t"
        "adc r23, r1 \n\t" /* can't overflow */
        "mul r25, r18 \n\t"
        "add r22, r0 \n\t"
        "adc r23, r1 \n\t" /* can't overflow */
        "st z+, r22 \n\t"
        "ldi r22, 0 \n\t"
        
        "mul r24, r20 \n\t"
        "add r23, r0 \n\t"
        "adc r22, r1 \n\t"
        "mul r25, r19 \n\t"
        "add r23, r0 \n\t"
        "adc r22, r1 \n\t"
        "st z+, r23 \n\t"
        "ldi r23, 0 \n\t"
        
        "mul r24, r21 \n\t"
        "add r22, r0 \n\t"
        "adc r23, r1 \n\t"
        "mul r25, r20 \n\t"
        "add r22, r0 \n\t"
        "adc r23, r1 \n\t"
        "st z+, r22 \n\t"
        "ldi r22, 0 \n\t"
        
        /* now we start adding the 2^32 part as well */
        "add r23, r18 \n\t" // 28
        "adc r22, r22 \n\t"
        "ld r18, x+ \n\t"
        "mul r24, r18 \n\t"
        "add r23, r0 \n\t"
        "adc r22, r1 \n\t"
        "mul r25, r21 \n\t"
        "add r23, r0 \n\t"
        "adc r22, r1 \n\t"
        "st z+, r23 \n\t"
        "ldi r23, 0 \n\t"
        
        "add r22, r19 \n\t" // 27
        "adc r23, r23 \n\t"
        "ld r19, x+ \n\t"
        "mul r24, r19 \n\t"
        "add r22, r0 \n\t"
        "adc r23, r1 \n\t"
        "mul r25, r18 \n\t"
        "add r22, r0 \n\t"
        "adc r23, r1 \n\t"
        "st z+, r22 \n\t"
        "ldi r22, 0 \n\t"
        
        REPEAT(6, // 26 - 3
            "add r23, r20 \n\t"
            "adc r22, r22 \n\t"
            "ld r20, x+ \n\t"
            "mul r24, r20 \n\t"
            "add r23, r0 \n\t"
            "adc r22, r1 \n\t"
            "mul r25, r19 \n\t"
            "add r23, r0 \n\t"
            "adc r22, r1 \n\t"
            "st z+, r23 \n\t"
            "ldi r23, 0 \n\t"
            
            "add r22, r21 \n\t"
            "adc r23, r23 \n\t"
            "ld r21, x+ \n\t"
            "mul r24, r21 \n\t"
            "add r22, r0 \n\t"
            "adc r23, r1 \n\t"
            "mul r25, r20 \n\t"
            "add r22, r0 \n\t"
            "adc r23, r1 \n\t"
            "st z+, r22 \n\t"
            "ldi r22, 0 \n\t"
            
            "add r23, r18 \n\t"
            "adc r22, r22 \n\t"
            "ld r18, x+ \n\t"
            "mul r24, r18 \n\t"
            "add r23, r0 \n\t"
            "adc r22, r1 \n\t"
            "mul r25, r21 \n\t"
            "add r23, r0 \n\t"
            "adc r22, r1 \n\t"
            "st z+, r23 \n\t"
            "ldi r23, 0 \n\t"
            
            "add r22, r19 \n\t"
            "adc r23, r23 \n\t"
            "ld r19, x+ \n\t"
            "mul r24, r19 \n\t"
            "add r22, r0 \n\t"
            "adc r23, r1 \n\t"
            "mul r25, r18 \n\t"
            "add r22, r0 \n\t"
            "adc r23, r1 \n\t"
            "st z+, r22 \n\t"
            "ldi r22, 0 \n\t")

        "add r23, r20 \n\t" // 2
        "adc r22, r22 \n\t"
        "ld r20, x+ \n\t"
        "mul r24, r20 \n\t"
        "add r23, r0 \n\t"
        "adc r22, r1 \n\t"
        "mul r25, r19 \n\t"
        "add r23, r0 \n\t"
        "adc r22, r1 \n\t"
        "st z+, r23 \n\t"
        "ldi r23, 0 \n\t"
        
        "add r22, r21 \n\t" // 1
        "adc r23, r23 \n\t"
        "ld r21, x+ \n\t"
        "mul r24, r21 \n\t"
        "add r22, r0 \n\t"
        "adc r23, r1 \n\t"
        "mul r25, r20 \n\t"
        "add r22, r0 \n\t"
        "adc r23, r1 \n\t"
        "st z+, r22 \n\t"
        "ldi r22, 0 \n\t"
        
        /* Now finish the carries etc */
        "add r23, r18 \n\t"
        "adc r22, r22 \n\t"
        "mul r25, r21 \n\t"
        "add r23, r0 \n\t"
        "adc r22, r1 \n\t"
        "st z+, r23 \n\t"
        "ldi r23, 0 \n\t"
        
        "add r22, r19 \n\t"
        "adc r23, r23 \n\t"
        "st z+, r22 \n\t"
        "ldi r22, 0 \n\t"
        
        "add r23, r20 \n\t"
        "adc r22, r22 \n\t"
        "st z+, r23 \n\t"
        "ldi r23, 0 \n\t"
        
        "add r22, r21 \n\t"
        "adc r23, r23 \n\t"
        "st z+, r22 \n\t"
        "st z+, r23 \n\t"
        "eor r1, r1 \n\t" /* make r1 be 0 again */
        
        "sbiw r30, 37 \n\t" /* move z back to point at tmp */
        "subi r26, 64 \n\t" /* move x back to point at product */
        "sbc r27, __zero_reg__ \n\t"
        
        /* add low bytes of tmp to product, storing in result */
        "ld r18, z+ \n\t"
        "ld r19, x+ \n\t"
        "add r18, r19 \n\t"
        "st y+, r18 \n\t"
        REPEAT(31,
            "ld r18, z+ \n\t"
            "ld r19, x+ \n\t"
            "adc r18, r19 \n\t"
            "st y+, r18 \n\t")
        
        "adc %[carry], __zero_reg__ \n\t" /* Store carry bit (carry flag is cleared). */
        /* at this point x is at the end of product, y is at the end of result,
           z is 32 bytes into tmp */
        "sbiw r28, 32 \n\t" /* move y back to point at result */

        /* do omega_mult again with the 5 relevant bytes */
        /* z points to tmp + uECC_WORDS, x points to the end of product */
        "sbiw r26, 32 \n\t" /* shift x back to point into the product buffer
                               (we can overwrite it now) */
        "ld r18, z+ \n\t"
        "ld r19, z+ \n\t"
        "ld r20, z+ \n\t"
        "ld r21, z+ \n\t"
        
        "mul r24, r18 \n\t"
        "st x+, r0 \n\t"
        "mov r22, r1 \n\t"
        "ldi r23, 0 \n\t"
        
        "mul r24, r19 \n\t"
        "add r22, r0 \n\t"
        "adc r23, r1 \n\t" /* can't overflow */
        "mul r25, r18 \n\t"
        "add r22, r0 \n\t"
        "adc r23, r1 \n\t" /* can't overflow */
        "st x+, r22 \n\t"
        "ldi r22, 0 \n\t"
        
        "mul r24, r20 \n\t"
        "add r23, r0 \n\t"
        "adc r22, r1 \n\t"
        "mul r25, r19 \n\t"
        "add r23, r0 \n\t"
        "adc r22, r1 \n\t"
        "st x+, r23 \n\t"
        "ldi r23, 0 \n\t"
        
        "mul r24, r21 \n\t"
        "add r22, r0 \n\t"
        "adc r23, r1 \n\t"
        "mul r25, r20 \n\t"
        "add r22, r0 \n\t"
        "adc r23, r1 \n\t"
        "st x+, r22 \n\t"
        "ldi r22, 0 \n\t"
        
        "add r23, r18 \n\t"
        "adc r22, r22 \n\t"
        "ld r18, z+ \n\t"
        "mul r24, r18 \n\t"
        "add r23, r0 \n\t"
        "adc r22, r1 \n\t"
        "mul r25, r21 \n\t"
        "add r23, r0 \n\t"
        "adc r22, r1 \n\t"
        "st x+, r23 \n\t"
        "ldi r23, 0 \n\t"
        
        /* Now finish the carries etc */
        "add r22, r19 \n\t"
        "adc r23, r23 \n\t"
        "mul r25, r18 \n\t"
        "add r22, r0 \n\t"
        "adc r23, r1 \n\t"
        "st x+, r22 \n\t"
        "ldi r22, 0 \n\t"
        
        "add r23, r20 \n\t"
        "adc r22, r22 \n\t"
        "st x+, r23 \n\t"
        "ldi r23, 0 \n\t"
        
        "add r22, r21 \n\t"
        "adc r23, r23 \n\t"
        "st x+, r22 \n\t"
        "ldi r22, 0 \n\t"
        
        "add r23, r18 \n\t"
        "adc r22, r22 \n\t"
        "st x+, r23 \n\t"
        "st x+, r22 \n\t"
        "eor r1, r1 \n\t" /* make r1 be 0 again */
        
        /* now z points to the end of tmp, x points to the end of product
           (y still points at result) */
        "sbiw r26, 10 \n\t" /* move x back to point at beginning of actual data */
        /* add into result */
        "ld r18, x+ \n\t"
        "ld r19, y \n\t"
        "add r18, r19 \n\t"
        "st y+, r18 \n\t"
        REPEAT(9,
            "ld r18, x+ \n\t"
            "ld r19, y \n\t"
            "adc r18, r19 \n\t"
            "st y+, r18 \n\t")
        
        /* Done adding, now propagate carry bit */
        REPEAT(22,
            "ld r18, y \n\t"
            "adc r18, __zero_reg__ \n\t"
            "st y+, r18 \n\t")
        
        "adc %[carry], __zero_reg__ \n\t"    /* Store carry bit (carry flag is cleared). */
        "sbiw r28, 32 \n\t" /* move y back to point at result */
        
        "sbiw r30, 1 \n\t" /* fix stack pointer */
    	"in r0, __SREG__ \n\t"
    	"cli \n\t"
    	"out __SP_H__, r31 \n\t"
    	"out __SREG__, r0 \n\t"
    	"out __SP_L__, r30 \n\t"
        
        : "+x" (product), [carry] "+r" (carry)
        : "y" (result)
        : "r0", "r18", "r19", "r20", "r21", "r22", "r23", "r24", "r25", "r30", "r31", "cc"
    );
    
    if (carry > 0) {
        --carry;
        uECC_vli_sub(result, result, curve_secp256k1.p, 32);
    }
    if (carry > 0) {
        uECC_vli_sub(result, result, curve_secp256k1.p, 32);
    }
    if (uECC_vli_cmp_unsafe(result, curve_secp256k1.p, 32) > 0) {
        uECC_vli_sub(result, result, curve_secp256k1.p, 32);
    }
}
#define asm_mmod_fast_secp256k1 1
#endif /* uECC_SUPPORTS_secp256k1 */

#endif /* (uECC_OPTIMIZATION_LEVEL >= 2) */

/* ---- "Small" implementations ---- */

#if !asm_add
uECC_VLI_API uECC_word_t uECC_vli_add(uECC_word_t *result,
                                      const uECC_word_t *left,
                                      const uECC_word_t *right,
                                      wordcount_t num_words) {
    volatile uECC_word_t *r = result;
    uint8_t carry = 0;
    uint8_t left_byte;
    uint8_t right_byte;

    __asm__ volatile (
        "clc \n\t"
        
        "1: \n\t"
        "ld %[left], x+ \n\t"  /* Load left byte. */
        "ld %[right], y+ \n\t" /* Load right byte. */
        "adc %[left], %[right] \n\t" /* Add. */
        "st z+, %[left] \n\t"  /* Store the result. */
        "dec %[i] \n\t"
        "brne 1b \n\t"
        
        "adc %[carry], %[carry] \n\t" /* Store carry bit. */

        : "+z" (r), "+x" (left), "+y" (right), [i] "+r" (num_words),
            [carry] "+r" (carry), [left] "=&r" (left_byte), [right] "=&r" (right_byte)
        : 
        : "cc"
    );
    return carry;
}
#define asm_add 1
#endif

#if !asm_sub
uECC_VLI_API uECC_word_t uECC_vli_sub(uECC_word_t *result,
                                      const uECC_word_t *left,
                                      const uECC_word_t *right,
                                      wordcount_t num_words) {
    volatile uECC_word_t *r = result;
    uint8_t borrow = 0;
    uint8_t left_byte;
    uint8_t right_byte;

    __asm__ volatile (
        "clc \n\t"
        
        "1: \n\t"
        "ld %[left], x+ \n\t"  /* Load left byte. */
        "ld %[right], y+ \n\t" /* Load right byte. */
        "sbc %[left], %[right] \n\t" /* Subtract. */
        "st z+, %[left] \n\t"  /* Store the result. */
        "dec %[i] \n\t"
        "brne 1b \n\t"
        
        "adc %[borrow], %[borrow] \n\t" /* Store carry bit in borrow. */

        : "+z" (r), "+x" (left), "+y" (right), [i] "+r" (i),
            [borrow] "+r" (borrow), [left] "=&r" (left_byte), [right] "=&r" (right_byte)
        :
        : "cc"
    );
    return borrow;
}
#define asm_sub 1
#endif

#if !asm_mult
__attribute((noinline))
uECC_VLI_API void uECC_vli_mult(uECC_word_t *result,
                                const uECC_word_t *left,
                                const uECC_word_t *right,
                                wordcount_t num_words) {
    volatile uECC_word_t *r = result;
    uint8_t r0 = 0;
    uint8_t r1 = 0;
    uint8_t r2 = 0;
    uint8_t zero = 0;
    uint8_t k, i;
    
    __asm__ volatile (
        "ldi %[k], 1 \n\t" /* k = 1; k < num_words; ++k */
        
        "1: \n\t"
        "ldi %[i], 0 \n\t"  /* i = 0; i < k; ++i */
        
        "add r28, %[k] \n\t" /* pre-add right ptr */
        "adc r29, %[zero] \n\t"
        
        "2: \n\t"
        "ld r0, x+ \n\t"
        "ld r1, -y \n\t"
        "mul r0, r1 \n\t"
        
        "add %[r0], r0 \n\t"
        "adc %[r1], r1 \n\t"
        "adc %[r2], %[zero] \n\t"
        
        "inc %[i] \n\t"
        "cp %[i], %[k] \n\t"
        "brlo 2b \n\t" /* loop if i < k */
        
        "sub r26, %[k] \n\t" /* fix up left ptr */
        "sbc r27, %[zero] \n\t"
        
        "st z+, %[r0] \n\t"  /* Store the result. */
        "mov %[r0], %[r1] \n\t"
        "mov %[r1], %[r2] \n\t"
        "mov %[r2], %[zero] \n\t"
        
        "inc %[k] \n\t"
        "cp %[k], %[num] \n\t"
        "brlo 1b \n\t" /* loop if k < num_words */
        
        /* second half */
        "mov %[k], %[num] \n\t" /* k = num_words; k > 0; --k */
        "add r28, %[num] \n\t" /* move right ptr to point at the end of right */
        "adc r29, %[zero] \n\t"
        
        "1: \n\t"
        "ldi %[i], 0 \n\t" /* i = 0; i < k; ++i */
        
        "2: \n\t"
        "ld r0, x+ \n\t"
        "ld r1, -y \n\t"
        "mul r0, r1 \n\t"
        
        "add %[r0], r0 \n\t"
        "adc %[r1], r1 \n\t"
        "adc %[r2], %[zero] \n\t"
        
        "inc %[i] \n\t"
        "cp %[i], %[k] \n\t"
        "brlo 2b \n\t" /* loop if i < k */
        
        "add r28, %[k] \n\t" /* fix up right ptr */
        "adc r29, %[zero] \n\t"
        
        "st z+, %[r0] \n\t"  /* Store the result. */
        "mov %[r0], %[r1] \n\t"
        "mov %[r1], %[r2] \n\t"
        "mov %[r2], %[zero] \n\t"
        
        "dec %[k] \n\t"
        "sub r26, %[k] \n\t" /* fix up left ptr (after k is decremented, so next time
                                we start 1 higher) */
        "sbc r27, %[zero] \n\t"
        
        "cp %[k], %[zero] \n\t"
        "brne 1b \n\t" /* loop if k > 0 */
        
        "st z+, %[r0] \n\t"  /* Store last result byte. */
        "eor r1, r1 \n\t" /* fix r1 to be 0 again */
    
        : "+z" (result), "+x" (left), "+y" (right),
          [r0] "+r" (r0), [r1] "+r" (r1), [r2] "+r" (r2),
          [zero] "+r" (zero), [num] "+r" (num_words),
          [k] "=&r" (k), [i] "=&r" (i)
        : 
        : "r0", "cc"
    );
}
#define asm_mult 1
#endif

#if (uECC_SQUARE_FUNC && !asm_square)
uECC_VLI_API void uECC_vli_square(uECC_word_t *result,
                                  const uECC_word_t *left,
                                  wordcount_t num_words) {
    volatile uECC_word_t *r = result;
    uint8_t r0 = 0;
    uint8_t r1 = 0;
    uint8_t r2 = 0;
    uint8_t zero = 0;
    uint8_t k;
    
    __asm__ volatile (
        "ldi %[k], 1 \n\t" /* k = 1; k < num_words * 2; ++k */
        
        "1: \n\t"
        
        "movw r26, %[orig] \n\t"  /* copy orig ptr to 'left' ptr */
        "movw r30, %[orig] \n\t"  /* copy orig ptr to 'right' ptr */
        "cp %[k], %[num] \n\t"
        "brlo 2f \n\t"
        "breq 2f \n\t"
        
        /* when k > num_words, we start from (k - num_words) on the 'left' ptr */
        "add r26, %[k] \n\t"
        "adc r27, %[zero] \n\t"
        "sub r26, %[num] \n\t"
        "sbc r27, %[zero] \n\t"
        "add r30, %[num] \n\t" /* move right ptr to point at the end */
        "adc r31, %[zero] \n\t"
        "rjmp 3f \n\t"
        
        "2: \n\t" /* when k <= num_words, we add k to the 'right' ptr */
        "add r30, %[k] \n\t" /* pre-add 'right' ptr */
        "adc r31, %[zero] \n\t"
        
        "3: \n\t"
        "ld r0, x+ \n\t"
        "cp r26, r30 \n\t" /* if left == right here, then we are done after this mult
                              (and we don't need to double) */
        "breq 4f \n\t"
        "ld r1, -z \n\t"
        "mul r0, r1 \n\t"
        
        /* add twice since it costs the same as doubling */
        "add %[r0], r0 \n\t"
        "adc %[r1], r1 \n\t"
        "adc %[r2], %[zero] \n\t"
        "add %[r0], r0 \n\t"
        "adc %[r1], r1 \n\t"
        "adc %[r2], %[zero] \n\t"
        
        "cpse r26, r30 \n\t" /* if left == right here, then we are done */
        "rjmp 3b \n\t"
        "rjmp 5f \n\t" /* skip code for non-doubled mult */
        
        "4: \n\t"
        "ld r1, -z \n\t"
        "mul r0, r1 \n\t"
        "add %[r0], r0 \n\t"
        "adc %[r1], r1 \n\t"
        "adc %[r2], %[zero] \n\t"
        
        "5: \n\t"
        "movw r30, %[result] \n\t" /* make z point to result */
        "st z+, %[r0] \n\t"        /* Store the result. */
        "movw %[result], r30 \n\t" /* update result ptr*/
        "mov %[r0], %[r1] \n\t"
        "mov %[r1], %[r2] \n\t"
        "mov %[r2], %[zero] \n\t"
        
        "inc %[k] \n\t"
        "cp %[k], %[max] \n\t"
        "brlo 1b \n\t" /* loop if k < num_words * 2 */
        
        "movw r30, %[result] \n\t"  /* make z point to result */
        "st z+, %[r0] \n\t"  /* Store last result byte. */
        "eor r1, r1 \n\t" /* fix r1 to be 0 again */
    
        : [result] "+r" (r),
          [r0] "+r" (r0), [r1] "+r" (r1), [r2] "+r" (r2), [zero] "+r" (zero),
          [k] "=&a" (k)
        : [orig] "r" (left), [max] "r" ((uint8_t)(2 * num_words)),
          [num] "r" (num_words)
        : "r0", "r26", "r27", "r30", "r31", "cc"
    );
}
#define asm_square 1
#endif /* uECC_SQUARE_FUNC && !asm_square */

#endif /* _UECC_ASM_AVR_H_ */
