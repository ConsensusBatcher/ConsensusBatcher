/*
 * Copyright (c) 2012-2020 MIRACL UK Ltd.
 *
 * This file is part of MIRACL Core
 * (see https://github.com/miracl/core).
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* CORE mod p functions */
/* Small Finite Field arithmetic */
/* SU=m, SU is Stack Usage (NOT_SPECIAL Modulus) */

#include "fp_BN462.h"

/* Fast Modular Reduction Methods */

/* r=d mod m */
/* d MUST be normalised */
/* Products must be less than pR in all cases !!! */
/* So when multiplying two numbers, their product *must* be less than MODBITS+BASEBITS*NLEN */
/* Results *may* be one bit bigger than MODBITS */

#if MODTYPE_BN462 == PSEUDO_MERSENNE
/* r=d mod m */

/* Converts from BIG integer to residue form mod Modulus */
void FP_BN462_nres(FP_BN462 *y, BIG_464_28 x)
{
    BIG_464_28 mdls;
    BIG_464_28_rcopy(mdls, Modulus_BN462);
    BIG_464_28_copy(y->g, x);
    BIG_464_28_mod(y->g,mdls);
    y->XES = 1;
}

/* Converts from residue form back to BIG integer form */
void FP_BN462_redc(BIG_464_28 x, FP_BN462 *y)
{
    BIG_464_28_copy(x, y->g);
}

/* reduce a DBIG to a BIG exploiting the special form of the modulus */
void FP_BN462_mod(BIG_464_28 r, DBIG_464_28 d)
{
    BIG_464_28 t, b;
    chunk v, tw;
    BIG_464_28_split(t, b, d, MODBITS_BN462);

    /* Note that all of the excess gets pushed into t. So if squaring a value with a 4-bit excess, this results in
       t getting all 8 bits of the excess product! So products must be less than pR which is Montgomery compatible */

    if (MConst_BN462 < NEXCESS_464_28)
    {
        BIG_464_28_imul(t, t, MConst_BN462);
        BIG_464_28_norm(t);
        BIG_464_28_add(r, t, b);
        BIG_464_28_norm(r);
        tw = r[NLEN_464_28 - 1];
        r[NLEN_464_28 - 1] &= TMASK_BN462;
        r[0] += MConst_BN462 * ((tw >> TBITS_BN462));
    }
    else
    {
        v = BIG_464_28_pmul(t, t, MConst_BN462);
        BIG_464_28_add(r, t, b);
        BIG_464_28_norm(r);
        tw = r[NLEN_464_28 - 1];
        r[NLEN_464_28 - 1] &= TMASK_BN462;
#if CHUNK == 16
        r[1] += muladd_464_28(MConst_BN462, ((tw >> TBITS_BN462) + (v << (BASEBITS_464_28 - TBITS_BN462))), 0, &r[0]);
#else
        r[0] += MConst_BN462 * ((tw >> TBITS_BN462) + (v << (BASEBITS_464_28 - TBITS_BN462)));
#endif
    }
    BIG_464_28_norm(r);
}
#endif

/* This only applies to Curve C448, so specialised (for now) */
#if MODTYPE_BN462 == GENERALISED_MERSENNE

void FP_BN462_nres(FP_BN462 *y, BIG_464_28 x)
{
    BIG_464_28 mdls;
    BIG_464_28_rcopy(mdls, Modulus_BN462);
    BIG_464_28_copy(y->g, x);
    BIG_464_28_mod(y->g,mdls);
    y->XES = 1;
}

/* Converts from residue form back to BIG integer form */
void FP_BN462_redc(BIG_464_28 x, FP_BN462 *y)
{
    BIG_464_28_copy(x, y->g);
}

/* reduce a DBIG to a BIG exploiting the special form of the modulus */
void FP_BN462_mod(BIG_464_28 r, DBIG_464_28 d)
{
    BIG_464_28 t, b;
    chunk carry;
    BIG_464_28_split(t, b, d, MBITS_BN462);

    BIG_464_28_add(r, t, b);

    BIG_464_28_dscopy(d, t);
    BIG_464_28_dshl(d, MBITS_BN462 / 2);

    BIG_464_28_split(t, b, d, MBITS_BN462);

    BIG_464_28_add(r, r, t);
    BIG_464_28_add(r, r, b);
    BIG_464_28_norm(r);
    BIG_464_28_shl(t, MBITS_BN462 / 2);

    BIG_464_28_add(r, r, t);

    carry = r[NLEN_464_28 - 1] >> TBITS_BN462;

    r[NLEN_464_28 - 1] &= TMASK_BN462;
    r[0] += carry;

    r[224 / BASEBITS_464_28] += carry << (224 % BASEBITS_464_28); /* need to check that this falls mid-word */
    BIG_464_28_norm(r);
}

#endif

#if MODTYPE_BN462 == MONTGOMERY_FRIENDLY

/* convert to Montgomery n-residue form */
void FP_BN462_nres(FP_BN462 *y, BIG_464_28 x)
{
    DBIG_464_28 d;
    BIG_464_28 r;
    BIG_464_28_rcopy(r, R2modp_BN462);
    BIG_464_28_mul(d, x, r);
    FP_BN462_mod(y->g, d);
    y->XES = 2;
}

/* convert back to regular form */
void FP_BN462_redc(BIG_464_28 x, FP_BN462 *y)
{
    DBIG_464_28 d;
    BIG_464_28_dzero(d);
    BIG_464_28_dscopy(d, y->g);
    FP_BN462_mod(x, d);
}

/* fast modular reduction from DBIG to BIG exploiting special form of the modulus */
void FP_BN462_mod(BIG_464_28 a, DBIG_464_28 d)
{
    int i;

    for (i = 0; i < NLEN_464_28; i++)
        d[NLEN_464_28 + i] += muladd_464_28(d[i], MConst_BN462 - 1, d[i], &d[NLEN_464_28 + i - 1]);

    BIG_464_28_sducopy(a, d);
    BIG_464_28_norm(a);
}

#endif

#if MODTYPE_BN462 == NOT_SPECIAL

/* convert to Montgomery n-residue form */
void FP_BN462_nres(FP_BN462 *y, BIG_464_28 x)
{
    DBIG_464_28 d;
    BIG_464_28 r;
    BIG_464_28_rcopy(r, R2modp_BN462);
    BIG_464_28_mul(d, x, r);
    FP_BN462_mod(y->g, d);
    y->XES = 2;
}

/* convert back to regular form */
void FP_BN462_redc(BIG_464_28 x, FP_BN462 *y)
{
    DBIG_464_28 d;
    BIG_464_28_dzero(d);
    BIG_464_28_dscopy(d, y->g);
    FP_BN462_mod(x, d);
}


/* reduce a DBIG to a BIG using Montgomery's no trial division method */
/* d is expected to be dnormed before entry */
/* SU= 112 */
void FP_BN462_mod(BIG_464_28 a, DBIG_464_28 d)
{
    BIG_464_28 mdls;
    BIG_464_28_rcopy(mdls, Modulus_BN462);
    BIG_464_28_monty(a, mdls, MConst_BN462, d);
}

#endif

void FP_BN462_from_int(FP_BN462 *x,int a)
{
    BIG_464_28 w;
    if (a<0) BIG_464_28_rcopy(w, Modulus_BN462);
    else BIG_464_28_zero(w); 
    BIG_464_28_inc(w,a); BIG_464_28_norm(w); 
    FP_BN462_nres(x,w);
}

/* test x==0 ? */
/* SU= 48 */
int FP_BN462_iszilch(FP_BN462 *x)
{
    BIG_464_28 m;
    FP_BN462 y;
    FP_BN462_copy(&y,x);
    FP_BN462_reduce(&y);
    FP_BN462_redc(m,&y);
    return BIG_464_28_iszilch(m);
}

int FP_BN462_isunity(FP_BN462 *x)
{
    BIG_464_28 m;
    FP_BN462 y;
    FP_BN462_copy(&y,x);
    FP_BN462_reduce(&y);
    FP_BN462_redc(m,&y);
    return BIG_464_28_isunity(m);
}


void FP_BN462_copy(FP_BN462 *y, FP_BN462 *x)
{
    BIG_464_28_copy(y->g, x->g);
    y->XES = x->XES;
}

void FP_BN462_rcopy(FP_BN462 *y, const BIG_464_28 c)
{
    BIG_464_28 b;
    BIG_464_28_rcopy(b, c);
    FP_BN462_nres(y, b);
}

/* Swap a and b if d=1 */
void FP_BN462_cswap(FP_BN462 *a, FP_BN462 *b, int d)
{
    sign32 t, c = d;
    BIG_464_28_cswap(a->g, b->g, d);

    c = ~(c - 1);
    t = c & ((a->XES) ^ (b->XES));
    a->XES ^= t;
    b->XES ^= t;

}

/* Move b to a if d=1 */
void FP_BN462_cmove(FP_BN462 *a, FP_BN462 *b, int d)
{
    sign32 c = -d;

    BIG_464_28_cmove(a->g, b->g, d);
    a->XES ^= (a->XES ^ b->XES)&c;
}

void FP_BN462_zero(FP_BN462 *x)
{
    BIG_464_28_zero(x->g);
    x->XES = 1;
}

int FP_BN462_equals(FP_BN462 *x, FP_BN462 *y)
{
    FP_BN462 xg, yg;
    FP_BN462_copy(&xg, x);
    FP_BN462_copy(&yg, y);
    FP_BN462_reduce(&xg);
    FP_BN462_reduce(&yg);
    if (BIG_464_28_comp(xg.g, yg.g) == 0) return 1;
    return 0;
}

// Is x lexically larger than p-x?
// return -1 for no, 0 if x=0, 1 for yes
int FP_BN462_islarger(FP_BN462 *x)
{
    BIG_464_28 p,fx,sx;
    if (FP_BN462_iszilch(x)) return 0;
    BIG_464_28_rcopy(p,Modulus_BN462);
    FP_BN462_redc(fx,x);
    BIG_464_28_sub(sx,p,fx);  BIG_464_28_norm(sx); 
    return BIG_464_28_comp(fx,sx);
}

void FP_BN462_toBytes(char *b,FP_BN462 *x)
{
    BIG_464_28 t;
    FP_BN462_redc(t, x);
    BIG_464_28_toBytes(b, t);
}

void FP_BN462_fromBytes(FP_BN462 *x,char *b)
{
    BIG_464_28 t;
    BIG_464_28_fromBytes(t, b);
    FP_BN462_nres(x, t);
}

/* output FP */
/* SU= 48 */
void FP_BN462_output(FP_BN462 *r)
{
    BIG_464_28 c;
    FP_BN462_reduce(r);
    FP_BN462_redc(c, r);
    BIG_464_28_output(c);
}

void FP_BN462_rawoutput(FP_BN462 *r)
{
    BIG_464_28_rawoutput(r->g);
}

#ifdef GET_STATS
int tsqr = 0, rsqr = 0, tmul = 0, rmul = 0;
int tadd = 0, radd = 0, tneg = 0, rneg = 0;
int tdadd = 0, rdadd = 0, tdneg = 0, rdneg = 0;
#endif

#ifdef FUSED_MODMUL

/* Insert fastest code here */

#endif

/* r=a*b mod Modulus */
/* product must be less that p.R - and we need to know this in advance! */
/* SU= 88 */
void FP_BN462_mul(FP_BN462 *r, FP_BN462 *a, FP_BN462 *b)
{
    DBIG_464_28 d;

    if ((sign64)a->XES * b->XES > (sign64)FEXCESS_BN462)
    {
#ifdef DEBUG_REDUCE
        printf("Product too large - reducing it\n");
#endif
        FP_BN462_reduce(a);  /* it is sufficient to fully reduce just one of them < p */
    }

#ifdef FUSED_MODMUL
    FP_BN462_modmul(r->g, a->g, b->g);
#else
    BIG_464_28_mul(d, a->g, b->g);
    FP_BN462_mod(r->g, d);
#endif
    r->XES = 2;
}


/* multiplication by an integer, r=a*c */
/* SU= 136 */
void FP_BN462_imul(FP_BN462 *r, FP_BN462 *a, int c)
{
    int s = 0;

    if (c < 0)
    {
        c = -c;
        s = 1;
    }

#if MODTYPE_BN462==PSEUDO_MERSENNE || MODTYPE_BN462==GENERALISED_MERSENNE
    DBIG_464_28 d;
    BIG_464_28_pxmul(d, a->g, c);
    FP_BN462_mod(r->g, d);
    r->XES = 2;

#else
    //Montgomery
    BIG_464_28 k;
    FP_BN462 f;
    if (a->XES * c <= FEXCESS_BN462)
    {
        BIG_464_28_pmul(r->g, a->g, c);
        r->XES = a->XES * c; // careful here - XES jumps!
    }
    else
    {
        // don't want to do this - only a problem for Montgomery modulus and larger constants
        BIG_464_28_zero(k);
        BIG_464_28_inc(k, c);
        BIG_464_28_norm(k);
        FP_BN462_nres(&f, k);
        FP_BN462_mul(r, a, &f);
    }
#endif

    if (s)
    {
        FP_BN462_neg(r, r);
        FP_BN462_norm(r);
    }
}

/* Set r=a^2 mod m */
/* SU= 88 */
void FP_BN462_sqr(FP_BN462 *r, FP_BN462 *a)
{
    DBIG_464_28 d;

    if ((sign64)a->XES * a->XES > (sign64)FEXCESS_BN462)
    {
#ifdef DEBUG_REDUCE
        printf("Product too large - reducing it\n");
#endif
        FP_BN462_reduce(a);
    }

    BIG_464_28_sqr(d, a->g);
    FP_BN462_mod(r->g, d);
    r->XES = 2;
}

/* SU= 16 */
/* Set r=a+b */
void FP_BN462_add(FP_BN462 *r, FP_BN462 *a, FP_BN462 *b)
{
    BIG_464_28_add(r->g, a->g, b->g);
    r->XES = a->XES + b->XES;
    if (r->XES > FEXCESS_BN462)
    {
#ifdef DEBUG_REDUCE
        printf("Sum too large - reducing it \n");
#endif
        FP_BN462_reduce(r);
    }
}

/* Set r=a-b mod m */
/* SU= 56 */
void FP_BN462_sub(FP_BN462 *r, FP_BN462 *a, FP_BN462 *b)
{
    FP_BN462 n;
    FP_BN462_neg(&n, b);
    FP_BN462_add(r, a, &n);
}

// https://graphics.stanford.edu/~seander/bithacks.html
// constant time log to base 2 (or number of bits in)

static int logb2(unsign32 v)
{
    int r;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;

    v = v - ((v >> 1) & 0x55555555);
    v = (v & 0x33333333) + ((v >> 2) & 0x33333333);
    r = (((v + (v >> 4)) & 0xF0F0F0F) * 0x1010101) >> 24;
    return r;
}

// find appoximation to quotient of a/m
// Out by at most 2.
// Note that MAXXES is bounded to be 2-bits less than half a word
static int quo(BIG_464_28 n, BIG_464_28 m)
{
    int sh;
    chunk num, den;
    int hb = CHUNK / 2;
    if (TBITS_BN462 < hb)
    {
        sh = hb - TBITS_BN462;
        num = (n[NLEN_464_28 - 1] << sh) | (n[NLEN_464_28 - 2] >> (BASEBITS_464_28 - sh));
        den = (m[NLEN_464_28 - 1] << sh) | (m[NLEN_464_28 - 2] >> (BASEBITS_464_28 - sh));
    }
    else
    {
        num = n[NLEN_464_28 - 1];
        den = m[NLEN_464_28 - 1];
    }
    return (int)(num / (den + 1));
}

/* SU= 48 */
/* Fully reduce a mod Modulus */
void FP_BN462_reduce(FP_BN462 *a)
{
    BIG_464_28 m, r;
    int sr, sb, q;
    chunk carry;

    BIG_464_28_rcopy(m, Modulus_BN462);

    BIG_464_28_norm(a->g);

    if (a->XES > 16)
    {
        q = quo(a->g, m);
        carry = BIG_464_28_pmul(r, m, q);
        r[NLEN_464_28 - 1] += (carry << BASEBITS_464_28); // correction - put any carry out back in again
        BIG_464_28_sub(a->g, a->g, r);
        BIG_464_28_norm(a->g);
        sb = 2;
    }
    else sb = logb2(a->XES - 1); // sb does not depend on the actual data

    BIG_464_28_fshl(m, sb);

    while (sb > 0)
    {
// constant time...
        sr = BIG_464_28_ssn(r, a->g, m); // optimized combined shift, subtract and norm
        BIG_464_28_cmove(a->g, r, 1 - sr);
        sb--;
    }

    //BIG_464_28_mod(a->g,m);
    a->XES = 1;
}

void FP_BN462_norm(FP_BN462 *x)
{
    BIG_464_28_norm(x->g);
}

/* Set r=-a mod Modulus */
/* SU= 64 */
void FP_BN462_neg(FP_BN462 *r, FP_BN462 *a)
{
    int sb;
    BIG_464_28 m;

    BIG_464_28_rcopy(m, Modulus_BN462);

    sb = logb2(a->XES - 1);
    BIG_464_28_fshl(m, sb);
    BIG_464_28_sub(r->g, m, a->g);
    r->XES = ((sign32)1 << sb) + 1;

    if (r->XES > FEXCESS_BN462)
    {
#ifdef DEBUG_REDUCE
        printf("Negation too large -  reducing it \n");
#endif
        FP_BN462_reduce(r);
    }

}

/* Set r=a/2. */
/* SU= 56 */
void FP_BN462_div2(FP_BN462 *r, FP_BN462 *a)
{
    BIG_464_28 m;
    BIG_464_28 w;
    BIG_464_28_rcopy(m, Modulus_BN462);
    int pr=BIG_464_28_parity(a->g);

    FP_BN462_copy(r, a);
    BIG_464_28_copy(w,r->g);
    BIG_464_28_fshr(r->g,1);
    BIG_464_28_add(w, w, m);
    BIG_464_28_norm(w);
    BIG_464_28_fshr(w, 1);   
    
    BIG_464_28_cmove(r->g,w,pr);

}

// Could leak size of b
// but not used here with secret exponent b
void FP_BN462_pow(FP_BN462 *r, FP_BN462 *a, BIG_464_28 b)
{
    sign8 w[1 + (NLEN_464_28 * BASEBITS_464_28 + 3) / 4];
    FP_BN462 tb[16];
    BIG_464_28 t;
    int i, nb;

    FP_BN462_copy(r,a);
    FP_BN462_norm(r);
    BIG_464_28_copy(t, b);
    BIG_464_28_norm(t);
    nb = 1 + (BIG_464_28_nbits(t) + 3) / 4;
    /* convert exponent to 4-bit window */
    for (i = 0; i < nb; i++)
    {
        w[i] = BIG_464_28_lastbits(t, 4);
        BIG_464_28_dec(t, w[i]);
        BIG_464_28_norm(t);
        BIG_464_28_fshr(t, 4);
    }

    FP_BN462_one(&tb[0]);
    FP_BN462_copy(&tb[1], r);
    for (i = 2; i < 16; i++)
        FP_BN462_mul(&tb[i], &tb[i - 1], r);

    FP_BN462_copy(r, &tb[w[nb - 1]]);
    for (i = nb - 2; i >= 0; i--)
    {
        FP_BN462_sqr(r, r);
        FP_BN462_sqr(r, r);
        FP_BN462_sqr(r, r);
        FP_BN462_sqr(r, r);
        FP_BN462_mul(r, r, &tb[w[i]]);
    }
    FP_BN462_reduce(r);
}


#if MODTYPE_BN462 == PSEUDO_MERSENNE || MODTYPE_BN462==GENERALISED_MERSENNE

// See eprint paper https://eprint.iacr.org/2018/1038
// If p=3 mod 4 r= x^{(p-3)/4}, if p=5 mod 8 r=x^{(p-5)/8}

static void FP_BN462_fpow(FP_BN462 *r, FP_BN462 *x)
{
    int i, j, k, bw, w, c, nw, lo, m, n, nd, e=PM1D2_BN462;
    FP_BN462 xp[11], t, key;
    const int ac[] = {1, 2, 3, 6, 12, 15, 30, 60, 120, 240, 255};
// phase 1
    FP_BN462_copy(&xp[0], x); // 1
    FP_BN462_sqr(&xp[1], x); // 2
    FP_BN462_mul(&xp[2], &xp[1], x); //3
    FP_BN462_sqr(&xp[3], &xp[2]); // 6
    FP_BN462_sqr(&xp[4], &xp[3]); // 12
    FP_BN462_mul(&xp[5], &xp[4], &xp[2]); // 15
    FP_BN462_sqr(&xp[6], &xp[5]); // 30
    FP_BN462_sqr(&xp[7], &xp[6]); // 60
    FP_BN462_sqr(&xp[8], &xp[7]); // 120
    FP_BN462_sqr(&xp[9], &xp[8]); // 240
    FP_BN462_mul(&xp[10], &xp[9], &xp[5]); // 255

#if MODTYPE_BN462==PSEUDO_MERSENNE
    n = MODBITS_BN462;
#endif
#if MODTYPE_BN462==GENERALISED_MERSENNE  // Ed448 ONLY
    n = MODBITS_BN462 / 2;
#endif

    n-=(e+1);
    c=(MConst_BN462+(1<<e)+1)/(1<<(e+1));

// need c to be odd
    nd=0;
    while (c%2==0)
    {
        c/=2;
        n-=1;
        nd++;
    }

    bw = 0; w = 1; while (w < c) {w *= 2; bw += 1;}
    k = w - c;

    if (k != 0)
    {
        i = 10; while (ac[i] > k) i--;
        FP_BN462_copy(&key, &xp[i]);
        k -= ac[i];
    }
    while (k != 0)
    {
        i--;
        if (ac[i] > k) continue;
        FP_BN462_mul(&key, &key, &xp[i]);
        k -= ac[i];
    }

// phase 2
    FP_BN462_copy(&xp[1], &xp[2]);
    FP_BN462_copy(&xp[2], &xp[5]);
    FP_BN462_copy(&xp[3], &xp[10]);

    j = 3; m = 8;
    nw = n - bw;
    while (2 * m < nw)
    {
        FP_BN462_copy(&t, &xp[j++]);
        for (i = 0; i < m; i++)
            FP_BN462_sqr(&t, &t);
        FP_BN462_mul(&xp[j], &xp[j - 1], &t);
        m *= 2;
    }

    lo = nw - m;
    FP_BN462_copy(r, &xp[j]);

    while (lo != 0)
    {
        m /= 2; j--;
        if (lo < m) continue;
        lo -= m;
        FP_BN462_copy(&t, r);
        for (i = 0; i < m; i++)
            FP_BN462_sqr(&t, &t);
        FP_BN462_mul(r, &t, &xp[j]);
    }
// phase 3

    if (bw != 0)
    {
        for (i = 0; i < bw; i++ )
            FP_BN462_sqr(r, r);
        FP_BN462_mul(r, r, &key);
    }
#if MODTYPE_BN462==GENERALISED_MERSENNE  // Ed448 ONLY
    FP_BN462_copy(&key, r);
    FP_BN462_sqr(&t, &key);
    FP_BN462_mul(r, &t, &xp[0]);
    for (i = 0; i < n + 1; i++)
        FP_BN462_sqr(r, r);
    FP_BN462_mul(r, r, &key);
#endif

    for (i=0;i<nd;i++)
        FP_BN462_sqr(r,r);
}

#endif


// calculates r=x^(p-1-2^e)/2^{e+1) where 2^e|p-1
void FP_BN462_progen(FP_BN462 *r,FP_BN462 *x)
{
#if MODTYPE_BN462==PSEUDO_MERSENNE  || MODTYPE_BN462==GENERALISED_MERSENNE
    FP_BN462_fpow(r, x);  
#else
    int e=PM1D2_BN462;
    BIG_464_28 m;
    BIG_464_28_rcopy(m, Modulus_BN462);
    BIG_464_28_dec(m,1);
    BIG_464_28_shr(m,e);
    BIG_464_28_dec(m,1);
    BIG_464_28_fshr(m,1);
    FP_BN462_pow(r,x,m);
#endif
}

/* Is x a QR? return optional hint for fast follow-up square root */
int FP_BN462_qr(FP_BN462 *x,FP_BN462 *h)
{
    FP_BN462 r;
    int i,e=PM1D2_BN462;
    FP_BN462_progen(&r,x);
    if (h!=NULL)
        FP_BN462_copy(h,&r);

    FP_BN462_sqr(&r,&r);
    FP_BN462_mul(&r,x,&r);
    for (i=0;i<e-1;i++ )
        FP_BN462_sqr(&r,&r);


//    for (i=0;i<e;i++)
//        FP_BN462_sqr(&r,&r);
//    FP_BN462_copy(&s,x);
//    for (i=0;i<e-1;i++ )
//        FP_BN462_sqr(&s,&s);
//    FP_BN462_mul(&r,&r,&s);
    
    return FP_BN462_isunity(&r);
}

/* Modular inversion */
void FP_BN462_inv(FP_BN462 *r,FP_BN462 *x,FP_BN462 *h)
{
    int i,e=PM1D2_BN462;
    FP_BN462 s,t;
    FP_BN462_norm(x);
    FP_BN462_copy(&s,x);

    if (h==NULL)
        FP_BN462_progen(&t,x);
    else
        FP_BN462_copy(&t,h);

    for (i=0;i<e-1;i++)
    {  
        FP_BN462_sqr(&s,&s);
        FP_BN462_mul(&s,&s,x);
    }
  
    for (i=0;i<=e;i++)
        FP_BN462_sqr(&t,&t);
    
    FP_BN462_mul(r,&t,&s);
    FP_BN462_reduce(r);
}

// Tonelli-Shanks in constant time
void FP_BN462_sqrt(FP_BN462 *r, FP_BN462 *a, FP_BN462 *h)
{
    int i,j,k,u,e=PM1D2_BN462;
    FP_BN462 v,g,t,b;
    BIG_464_28 m;

    if (h==NULL)
        FP_BN462_progen(&g,a);
    else
        FP_BN462_copy(&g,h);

    BIG_464_28_rcopy(m,ROI_BN462);
    FP_BN462_nres(&v,m);

    FP_BN462_sqr(&t,&g);
    FP_BN462_mul(&t,&t,a);
   
    FP_BN462_mul(r,&g,a);
    FP_BN462_copy(&b,&t);
    for (k=e;k>1;k--)
    {
        for (j=1;j<k-1;j++)
            FP_BN462_sqr(&b,&b);
        u=1-FP_BN462_isunity(&b);
        FP_BN462_mul(&g,r,&v);
        FP_BN462_cmove(r,&g,u);
        FP_BN462_sqr(&v,&v);
        FP_BN462_mul(&g,&t,&v);
        FP_BN462_cmove(&t,&g,u);
        FP_BN462_copy(&b,&t);
    }
// always return +ve square root
    k=FP_BN462_sign(r);
    FP_BN462_neg(&v,r); FP_BN462_norm(&v);
    FP_BN462_cmove(r,&v,k);
}

// Calculate both inverse and square root of x, return QR
int FP_BN462_invsqrt(FP_BN462 *i, FP_BN462 *s, FP_BN462 *x)
{
    FP_BN462 h;
    int qr=FP_BN462_qr(x,&h);
    FP_BN462_sqrt(s,x,&h);
    FP_BN462_inv(i,x,&h);
    return qr;
}

// Two for Price of One - See Hamburg https://eprint.iacr.org/2012/309.pdf
// Calculate inverse of i and square root of s, return QR
int FP_BN462_tpo(FP_BN462 *i, FP_BN462 *s)
{
    int qr;
    FP_BN462 w,t;
    FP_BN462_mul(&w,s,i);
    FP_BN462_mul(&t,&w,i);
    qr=FP_BN462_invsqrt(i,s,&t);
    FP_BN462_mul(i,i,&w);
    FP_BN462_mul(s,s,i);
    return qr;
}

/* SU=8 */
/* set n=1 */
void FP_BN462_one(FP_BN462 *n)
{
    BIG_464_28 b;
    BIG_464_28_one(b);
    FP_BN462_nres(n, b);
}

int FP_BN462_sign(FP_BN462 *x)
{
#ifdef BIG_ENDIAN_SIGN_BN462
    int cp;
    BIG_464_28 m,pm1d2;
    FP_BN462 y;
    BIG_464_28_rcopy(pm1d2, Modulus_BN462);
    BIG_464_28_dec(pm1d2,1);
    BIG_464_28_fshr(pm1d2,1); //(p-1)/2
     
    FP_BN462_copy(&y,x);
    FP_BN462_reduce(&y);
    FP_BN462_redc(m,&y);
    cp=BIG_464_28_comp(m,pm1d2);
    return ((cp+1)&2)>>1;

#else
    BIG_464_28 m;
    FP_BN462 y;
    FP_BN462_copy(&y,x);
    FP_BN462_reduce(&y);
    FP_BN462_redc(m,&y);
    return BIG_464_28_parity(m);
#endif
}

void FP_BN462_rand(FP_BN462 *x,csprng *rng)
{
    BIG_464_28 w,m;
    BIG_464_28_rcopy(m,Modulus_BN462);
    BIG_464_28_randomnum(w,m,rng);
    FP_BN462_nres(x,w);
}

