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

/* CORE Fp^8 functions */

/* FP8 elements are of the form a+ib, where i is sqrt(sqrt(-1+sqrt(-1))) */

#include "fp8_BLS48556.h"


/* test x==0 ? */
int FP8_BLS48556_iszilch(FP8_BLS48556 *x)
{
    return (FP4_BLS48556_iszilch(&(x->a)) & FP4_BLS48556_iszilch(&(x->b))); 
}

/* test x==1 ? */
int FP8_BLS48556_isunity(FP8_BLS48556 *x)
{
    return (FP4_BLS48556_isunity(&(x->a)) & FP4_BLS48556_iszilch(&(x->b)));
}

/* test is w real? That is in a+ib test b is zero */
int FP8_BLS48556_isreal(FP8_BLS48556 *w)
{
    return FP4_BLS48556_iszilch(&(w->b));
}

/* return 1 if x==y, else 0 */
int FP8_BLS48556_equals(FP8_BLS48556 *x, FP8_BLS48556 *y)
{
    return (FP4_BLS48556_equals(&(x->a), &(y->a)) & FP4_BLS48556_equals(&(x->b), &(y->b)));
}

// Is x lexically larger than p-x?
// return -1 for no, 0 if x=0, 1 for yes
int FP8_BLS48556_islarger(FP8_BLS48556 *x)
{
    int cmp;
    if (FP8_BLS48556_iszilch(x)) return 0;
    cmp=FP4_BLS48556_islarger(&(x->b));
    if (cmp!=0) return cmp;
    return FP4_BLS48556_islarger(&(x->a));
}

void FP8_BLS48556_toBytes(char *b,FP8_BLS48556 *x)
{
    FP4_BLS48556_toBytes(b,&(x->b));
    FP4_BLS48556_toBytes(&b[4*MODBYTES_560_29],&(x->a));
}

void FP8_BLS48556_fromBytes(FP8_BLS48556 *x,char *b)
{
    FP4_BLS48556_fromBytes(&(x->b),b);
    FP4_BLS48556_fromBytes(&(x->a),&b[4*MODBYTES_560_29]);
}


/* set FP8 from two FP4s */
void FP8_BLS48556_from_FP4s(FP8_BLS48556 *w, FP4_BLS48556 * x, FP4_BLS48556* y)
{
    FP4_BLS48556_copy(&(w->a), x);
    FP4_BLS48556_copy(&(w->b), y);
}

/* set FP8 from FP4 */
void FP8_BLS48556_from_FP4(FP8_BLS48556 *w, FP4_BLS48556 *x)
{
    FP4_BLS48556_copy(&(w->a), x);
    FP4_BLS48556_zero(&(w->b));
}

/* set high part of FP8 from FP4 */
void FP8_BLS48556_from_FP4H(FP8_BLS48556 *w, FP4_BLS48556 *x)
{
    FP4_BLS48556_copy(&(w->b), x);
    FP4_BLS48556_zero(&(w->a));
}

/* set FP8 from FP */
void FP8_BLS48556_from_FP(FP8_BLS48556 *w, FP_BLS48556 *x)
{
    FP2_BLS48556 t;
    FP4_BLS48556 t4;
    FP2_BLS48556_from_FP(&t, x);
    FP4_BLS48556_from_FP2(&t4, &t);
    FP8_BLS48556_from_FP4(w, &t4);
}

/* FP8 copy w=x */
void FP8_BLS48556_copy(FP8_BLS48556 *w, FP8_BLS48556 *x)
{
    if (w == x) return;
    FP4_BLS48556_copy(&(w->a), &(x->a));
    FP4_BLS48556_copy(&(w->b), &(x->b));
}

/* FP8 w=0 */
void FP8_BLS48556_zero(FP8_BLS48556 *w)
{
    FP4_BLS48556_zero(&(w->a));
    FP4_BLS48556_zero(&(w->b));
}

/* FP8 w=1 */
void FP8_BLS48556_one(FP8_BLS48556 *w)
{
    FP4_BLS48556_one(&(w->a));
    FP4_BLS48556_zero(&(w->b));
}

int FP8_BLS48556_sign(FP8_BLS48556 *w)
{
    int p1,p2;
    p1=FP4_BLS48556_sign(&(w->a));
    p2=FP4_BLS48556_sign(&(w->b));
#ifdef BIG_ENDIAN_SIGN_BLS48556
    p2 ^= (p1 ^ p2)&FP4_BLS48556_iszilch(&(w->b));
    return p2;
#else
    p1 ^= (p1 ^ p2)&FP4_BLS48556_iszilch(&(w->a));
    return p1;
#endif
}

/* Set w=-x */
void FP8_BLS48556_neg(FP8_BLS48556 *w, FP8_BLS48556 *x)
{
    /* Just one field neg */
    FP4_BLS48556 m, t;
    FP8_BLS48556_norm(x);
    FP4_BLS48556_add(&m, &(x->a), &(x->b));
    FP4_BLS48556_norm(&m);
    FP4_BLS48556_neg(&m, &m);
    FP4_BLS48556_add(&t, &m, &(x->b));
    FP4_BLS48556_add(&(w->b), &m, &(x->a));
    FP4_BLS48556_copy(&(w->a), &t);
    FP8_BLS48556_norm(w);
}

/* Set w=conj(x) */
void FP8_BLS48556_conj(FP8_BLS48556 *w, FP8_BLS48556 *x)
{
    FP4_BLS48556_copy(&(w->a), &(x->a));
    FP4_BLS48556_neg(&(w->b), &(x->b));
    FP8_BLS48556_norm(w);
}

/* Set w=-conj(x) */
void FP8_BLS48556_nconj(FP8_BLS48556 *w, FP8_BLS48556 *x)
{
    FP4_BLS48556_copy(&(w->b), &(x->b));
    FP4_BLS48556_neg(&(w->a), &(x->a));
    FP8_BLS48556_norm(w);
}

/* Set w=x+y */
void FP8_BLS48556_add(FP8_BLS48556 *w, FP8_BLS48556 *x, FP8_BLS48556 *y)
{
    FP4_BLS48556_add(&(w->a), &(x->a), &(y->a));
    FP4_BLS48556_add(&(w->b), &(x->b), &(y->b));
}

/* Set w=x-y */
/* Input y MUST be normed */
void FP8_BLS48556_sub(FP8_BLS48556 *w, FP8_BLS48556 *x, FP8_BLS48556 *y)
{
    FP8_BLS48556 my;

    FP8_BLS48556_neg(&my, y);
    FP8_BLS48556_add(w, x, &my);

}

/* reduce all components of w mod Modulus */
void FP8_BLS48556_reduce(FP8_BLS48556 *w)
{
    FP4_BLS48556_reduce(&(w->a));
    FP4_BLS48556_reduce(&(w->b));
}

/* normalise all elements of w */
void FP8_BLS48556_norm(FP8_BLS48556 *w)
{
    FP4_BLS48556_norm(&(w->a));
    FP4_BLS48556_norm(&(w->b));
}

/* Set w=s*x, where s is FP4 */
void FP8_BLS48556_pmul(FP8_BLS48556 *w, FP8_BLS48556 *x, FP4_BLS48556 *s)
{
    FP4_BLS48556_mul(&(w->a), &(x->a), s);
    FP4_BLS48556_mul(&(w->b), &(x->b), s);
}

/* Set w=s*x, where s is FP2 */
void FP8_BLS48556_qmul(FP8_BLS48556 *w, FP8_BLS48556 *x, FP2_BLS48556 *s)
{
    FP4_BLS48556_pmul(&(w->a), &(x->a), s);
    FP4_BLS48556_pmul(&(w->b), &(x->b), s);
}

/* Set w=s*x, where s is FP2 */
void FP8_BLS48556_tmul(FP8_BLS48556 *w, FP8_BLS48556 *x, FP_BLS48556 *s)
{
    FP4_BLS48556_qmul(&(w->a), &(x->a), s);
    FP4_BLS48556_qmul(&(w->b), &(x->b), s);
}

/* Set w=s*x, where s is int */
void FP8_BLS48556_imul(FP8_BLS48556 *w, FP8_BLS48556 *x, int s)
{
    FP4_BLS48556_imul(&(w->a), &(x->a), s);
    FP4_BLS48556_imul(&(w->b), &(x->b), s);
}

/* Set w=x^2 */
/* Input MUST be normed  */
void FP8_BLS48556_sqr(FP8_BLS48556 *w, FP8_BLS48556 *x)
{
    FP4_BLS48556 t1, t2, t3;

    FP4_BLS48556_mul(&t3, &(x->a), &(x->b)); /* norms x */
    FP4_BLS48556_copy(&t2, &(x->b));
    FP4_BLS48556_add(&t1, &(x->a), &(x->b));
    FP4_BLS48556_times_i(&t2);

    FP4_BLS48556_add(&t2, &(x->a), &t2);

    FP4_BLS48556_norm(&t1);  // 2
    FP4_BLS48556_norm(&t2);  // 2

    FP4_BLS48556_mul(&(w->a), &t1, &t2);

    FP4_BLS48556_copy(&t2, &t3);
    FP4_BLS48556_times_i(&t2);

    FP4_BLS48556_add(&t2, &t2, &t3);

    FP4_BLS48556_norm(&t2);  // 2
    FP4_BLS48556_neg(&t2, &t2);
    FP4_BLS48556_add(&(w->a), &(w->a), &t2); /* a=(a+b)(a+i^2.b)-i^2.ab-ab = a*a+ib*ib */
    FP4_BLS48556_add(&(w->b), &t3, &t3); /* b=2ab */

    FP8_BLS48556_norm(w);
}

/* Set w=x*y */
/* Inputs MUST be normed  */
void FP8_BLS48556_mul(FP8_BLS48556 *w, FP8_BLS48556 *x, FP8_BLS48556 *y)
{

    FP4_BLS48556 t1, t2, t3, t4;
    FP4_BLS48556_mul(&t1, &(x->a), &(y->a));
    FP4_BLS48556_mul(&t2, &(x->b), &(y->b));

    FP4_BLS48556_add(&t3, &(y->b), &(y->a));
    FP4_BLS48556_add(&t4, &(x->b), &(x->a));

    FP4_BLS48556_norm(&t4); // 2
    FP4_BLS48556_norm(&t3); // 2

    FP4_BLS48556_mul(&t4, &t4, &t3); /* (xa+xb)(ya+yb) */

    FP4_BLS48556_neg(&t3, &t1); // 1
    FP4_BLS48556_add(&t4, &t4, &t3); //t4E=3
    FP4_BLS48556_norm(&t4);

    FP4_BLS48556_neg(&t3, &t2); // 1
    FP4_BLS48556_add(&(w->b), &t4, &t3); //wbE=3

    FP4_BLS48556_times_i(&t2);
    FP4_BLS48556_add(&(w->a), &t2, &t1);

    FP8_BLS48556_norm(w);
}

/* output FP8 in format [a,b] */
void FP8_BLS48556_output(FP8_BLS48556 *w)
{
    printf("[");
    FP4_BLS48556_output(&(w->a));
    printf(",");
    FP4_BLS48556_output(&(w->b));
    printf("]");
}

void FP8_BLS48556_rawoutput(FP8_BLS48556 *w)
{
    printf("[");
    FP4_BLS48556_rawoutput(&(w->a));
    printf(",");
    FP4_BLS48556_rawoutput(&(w->b));
    printf("]");
}

/* Set w=1/x */
void FP8_BLS48556_inv(FP8_BLS48556 *w, FP8_BLS48556 *x, FP_BLS48556 *h)
{
    FP4_BLS48556 t1, t2;
    FP4_BLS48556_sqr(&t1, &(x->a));
    FP4_BLS48556_sqr(&t2, &(x->b));
    FP4_BLS48556_times_i(&t2);
    FP4_BLS48556_norm(&t2);

    FP4_BLS48556_sub(&t1, &t1, &t2);
    FP4_BLS48556_norm(&t1);
    FP4_BLS48556_inv(&t1, &t1,h);

    FP4_BLS48556_mul(&(w->a), &t1, &(x->a));
    FP4_BLS48556_neg(&t1, &t1);
    FP4_BLS48556_norm(&t1);
    FP4_BLS48556_mul(&(w->b), &t1, &(x->b));
}

/* w*=i where i = sqrt(sqrt(-1+sqrt(-1))) */
void FP8_BLS48556_times_i(FP8_BLS48556 *w)
{
    FP4_BLS48556 s, t;
    FP4_BLS48556_copy(&s, &(w->b));
    FP4_BLS48556_copy(&t, &(w->a));
    FP4_BLS48556_times_i(&s);
    FP4_BLS48556_copy(&(w->a), &s);
    FP4_BLS48556_copy(&(w->b), &t);
    FP8_BLS48556_norm(w);
#if TOWER_BLS48556 == POSITOWER
    FP8_BLS48556_neg(w, w);  // ***
    FP8_BLS48556_norm(w);
#endif
}

void FP8_BLS48556_times_i2(FP8_BLS48556 *w)
{
    FP4_BLS48556_times_i(&(w->a));
    FP4_BLS48556_times_i(&(w->b));
}

/* Set w=w^p using Frobenius */
void FP8_BLS48556_frob(FP8_BLS48556 *w, FP2_BLS48556 *f)
{   // f=(i+1)^(p-3)/4
    FP2_BLS48556 ff;
    FP2_BLS48556_sqr(&ff, f); // (i+1)^(p-3)/2
    FP2_BLS48556_mul_ip(&ff); // (i+1)^(p-1)/2
    FP2_BLS48556_norm(&ff);
    FP4_BLS48556_frob(&(w->a), &ff);
    FP4_BLS48556_frob(&(w->b), &ff);
    FP4_BLS48556_pmul(&(w->b), &(w->b), f); // times (1+i)^(p-3)/4
    FP4_BLS48556_times_i(&(w->b));       // (i+1)^(p-1)/4
}

/* Set r=a^b mod m */
/*
void FP8_BLS48556_pow(FP8_BLS48556 *r, FP8_BLS48556* a, BIG_560_29 b)
{
    FP8_BLS48556 w;
    BIG_560_29 z, zilch;
    int bt;

    BIG_560_29_zero(zilch);

    BIG_560_29_copy(z, b);
    FP8_BLS48556_copy(&w, a);
    FP8_BLS48556_norm(&w);
    FP8_BLS48556_one(r);
    BIG_560_29_norm(z);
    while (1)
    {
        bt = BIG_560_29_parity(z);
        BIG_560_29_shr(z, 1);
        if (bt) FP8_BLS48556_mul(r, r, &w);
        if (BIG_560_29_comp(z, zilch) == 0) break;
        FP8_BLS48556_sqr(&w, &w);
    }
    FP8_BLS48556_reduce(r);
}*/

//#if CURVE_SECURITY_BLS48556 == 192

/* XTR xtr_a function */
/*
void FP8_BLS48556_xtr_A(FP8_BLS48556 *r, FP8_BLS48556 *w, FP8_BLS48556 *x, FP8_BLS48556 *y, FP8_BLS48556 *z)
{
    FP8_BLS48556 t1, t2;

    FP8_BLS48556_copy(r, x);
    FP8_BLS48556_sub(&t1, w, y);
    FP8_BLS48556_norm(&t1);
    FP8_BLS48556_pmul(&t1, &t1, &(r->a));
    FP8_BLS48556_add(&t2, w, y);
    FP8_BLS48556_norm(&t2);
    FP8_BLS48556_pmul(&t2, &t2, &(r->b));
    FP8_BLS48556_times_i(&t2);

    FP8_BLS48556_add(r, &t1, &t2);
    FP8_BLS48556_add(r, r, z);

    FP8_BLS48556_reduce(r);
}
*/
/* XTR xtr_d function */
/*
void FP8_BLS48556_xtr_D(FP8_BLS48556 *r, FP8_BLS48556 *x)
{
    FP8_BLS48556 w;
    FP8_BLS48556_copy(r, x);
    FP8_BLS48556_conj(&w, r);
    FP8_BLS48556_add(&w, &w, &w);
    FP8_BLS48556_sqr(r, r);
    FP8_BLS48556_norm(&w);
    FP8_BLS48556_sub(r, r, &w);
    FP8_BLS48556_reduce(r);    // reduce here as multiple calls trigger automatic reductions
}
*/

/* r=x^n using XTR method on traces of FP24s */
/*
void FP8_BLS48556_xtr_pow(FP8_BLS48556 *r, FP8_BLS48556 *x, BIG_560_29 n)
{
    int i, par, nb;
    BIG_560_29 v;
    FP2_BLS48556 w2;
    FP4_BLS48556 w4;
    FP8_BLS48556 t, a, b, c, sf;

    BIG_560_29_zero(v);
    BIG_560_29_inc(v, 3);
    BIG_560_29_norm(v);
    FP2_BLS48556_from_BIG(&w2, v);
    FP4_BLS48556_from_FP2(&w4, &w2);
    FP8_BLS48556_from_FP4(&a, &w4);
    FP8_BLS48556_copy(&sf, x);
    FP8_BLS48556_norm(&sf);
    FP8_BLS48556_copy(&b, &sf);
    FP8_BLS48556_xtr_D(&c, &sf);

    par = BIG_560_29_parity(n);
    BIG_560_29_copy(v, n);
    BIG_560_29_norm(v);
    BIG_560_29_shr(v, 1);
    if (par == 0)
    {
        BIG_560_29_dec(v, 1);
        BIG_560_29_norm(v);
    }

    nb = BIG_560_29_nbits(v);
    for (i = nb - 1; i >= 0; i--)
    {
        if (!BIG_560_29_bit(v, i))
        {
            FP8_BLS48556_copy(&t, &b);
            FP8_BLS48556_conj(&sf, &sf);
            FP8_BLS48556_conj(&c, &c);
            FP8_BLS48556_xtr_A(&b, &a, &b, &sf, &c);
            FP8_BLS48556_conj(&sf, &sf);
            FP8_BLS48556_xtr_D(&c, &t);
            FP8_BLS48556_xtr_D(&a, &a);
        }
        else
        {
            FP8_BLS48556_conj(&t, &a);
            FP8_BLS48556_xtr_D(&a, &b);
            FP8_BLS48556_xtr_A(&b, &c, &b, &sf, &t);
            FP8_BLS48556_xtr_D(&c, &c);
        }
    }

    if (par == 0) FP8_BLS48556_copy(r, &c);
    else FP8_BLS48556_copy(r, &b);
    FP8_BLS48556_reduce(r);
}
*/
/* r=ck^a.cl^n using XTR double exponentiation method on traces of FP24s. See Stam thesis. */
/*
void FP8_BLS48556_xtr_pow2(FP8_BLS48556 *r, FP8_BLS48556 *ck, FP8_BLS48556 *cl, FP8_BLS48556 *ckml, FP8_BLS48556 *ckm2l, BIG_560_29 a, BIG_560_29 b)
{
    int i, f2;
    BIG_560_29 d, e, w;
    FP8_BLS48556 t, cu, cv, cumv, cum2v;


    BIG_560_29_copy(e, a);
    BIG_560_29_copy(d, b);
    BIG_560_29_norm(e);
    BIG_560_29_norm(d);
    FP8_BLS48556_copy(&cu, ck);
    FP8_BLS48556_copy(&cv, cl);
    FP8_BLS48556_copy(&cumv, ckml);
    FP8_BLS48556_copy(&cum2v, ckm2l);

    f2 = 0;
    while (BIG_560_29_parity(d) == 0 && BIG_560_29_parity(e) == 0)
    {
        BIG_560_29_shr(d, 1);
        BIG_560_29_shr(e, 1);
        f2++;
    }
    while (BIG_560_29_comp(d, e) != 0)
    {
        if (BIG_560_29_comp(d, e) > 0)
        {
            BIG_560_29_imul(w, e, 4);
            BIG_560_29_norm(w);
            if (BIG_560_29_comp(d, w) <= 0)
            {
                BIG_560_29_copy(w, d);
                BIG_560_29_copy(d, e);
                BIG_560_29_sub(e, w, e);
                BIG_560_29_norm(e);
                FP8_BLS48556_xtr_A(&t, &cu, &cv, &cumv, &cum2v);
                FP8_BLS48556_conj(&cum2v, &cumv);
                FP8_BLS48556_copy(&cumv, &cv);
                FP8_BLS48556_copy(&cv, &cu);
                FP8_BLS48556_copy(&cu, &t);
            }
            else if (BIG_560_29_parity(d) == 0)
            {
                BIG_560_29_shr(d, 1);
                FP8_BLS48556_conj(r, &cum2v);
                FP8_BLS48556_xtr_A(&t, &cu, &cumv, &cv, r);
                FP8_BLS48556_xtr_D(&cum2v, &cumv);
                FP8_BLS48556_copy(&cumv, &t);
                FP8_BLS48556_xtr_D(&cu, &cu);
            }
            else if (BIG_560_29_parity(e) == 1)
            {
                BIG_560_29_sub(d, d, e);
                BIG_560_29_norm(d);
                BIG_560_29_shr(d, 1);
                FP8_BLS48556_xtr_A(&t, &cu, &cv, &cumv, &cum2v);
                FP8_BLS48556_xtr_D(&cu, &cu);
                FP8_BLS48556_xtr_D(&cum2v, &cv);
                FP8_BLS48556_conj(&cum2v, &cum2v);
                FP8_BLS48556_copy(&cv, &t);
            }
            else
            {
                BIG_560_29_copy(w, d);
                BIG_560_29_copy(d, e);
                BIG_560_29_shr(d, 1);
                BIG_560_29_copy(e, w);
                FP8_BLS48556_xtr_D(&t, &cumv);
                FP8_BLS48556_conj(&cumv, &cum2v);
                FP8_BLS48556_conj(&cum2v, &t);
                FP8_BLS48556_xtr_D(&t, &cv);
                FP8_BLS48556_copy(&cv, &cu);
                FP8_BLS48556_copy(&cu, &t);
            }
        }
        if (BIG_560_29_comp(d, e) < 0)
        {
            BIG_560_29_imul(w, d, 4);
            BIG_560_29_norm(w);
            if (BIG_560_29_comp(e, w) <= 0)
            {
                BIG_560_29_sub(e, e, d);
                BIG_560_29_norm(e);
                FP8_BLS48556_xtr_A(&t, &cu, &cv, &cumv, &cum2v);
                FP8_BLS48556_copy(&cum2v, &cumv);
                FP8_BLS48556_copy(&cumv, &cu);
                FP8_BLS48556_copy(&cu, &t);
            }
            else if (BIG_560_29_parity(e) == 0)
            {
                BIG_560_29_copy(w, d);
                BIG_560_29_copy(d, e);
                BIG_560_29_shr(d, 1);
                BIG_560_29_copy(e, w);
                FP8_BLS48556_xtr_D(&t, &cumv);
                FP8_BLS48556_conj(&cumv, &cum2v);
                FP8_BLS48556_conj(&cum2v, &t);
                FP8_BLS48556_xtr_D(&t, &cv);
                FP8_BLS48556_copy(&cv, &cu);
                FP8_BLS48556_copy(&cu, &t);
            }
            else if (BIG_560_29_parity(d) == 1)
            {
                BIG_560_29_copy(w, e);
                BIG_560_29_copy(e, d);
                BIG_560_29_sub(w, w, d);
                BIG_560_29_norm(w);
                BIG_560_29_copy(d, w);
                BIG_560_29_shr(d, 1);
                FP8_BLS48556_xtr_A(&t, &cu, &cv, &cumv, &cum2v);
                FP8_BLS48556_conj(&cumv, &cumv);
                FP8_BLS48556_xtr_D(&cum2v, &cu);
                FP8_BLS48556_conj(&cum2v, &cum2v);
                FP8_BLS48556_xtr_D(&cu, &cv);
                FP8_BLS48556_copy(&cv, &t);
            }
            else
            {
                BIG_560_29_shr(d, 1);
                FP8_BLS48556_conj(r, &cum2v);
                FP8_BLS48556_xtr_A(&t, &cu, &cumv, &cv, r);
                FP8_BLS48556_xtr_D(&cum2v, &cumv);
                FP8_BLS48556_copy(&cumv, &t);
                FP8_BLS48556_xtr_D(&cu, &cu);
            }
        }
    }
    FP8_BLS48556_xtr_A(r, &cu, &cv, &cumv, &cum2v);
    for (i = 0; i < f2; i++)    FP8_BLS48556_xtr_D(r, r);
    FP8_BLS48556_xtr_pow(r, r, d);
}

#endif
*/

/* New stuff for ECp8 support */

/* Set w=x/2 */
void FP8_BLS48556_div2(FP8_BLS48556 *w, FP8_BLS48556 *x)
{
    FP4_BLS48556_div2(&(w->a), &(x->a));
    FP4_BLS48556_div2(&(w->b), &(x->b));
}

/* Move b to a if d=1 */
void FP8_BLS48556_cmove(FP8_BLS48556 *f, FP8_BLS48556 *g, int d)
{
    FP4_BLS48556_cmove(&(f->a), &(g->a), d);
    FP4_BLS48556_cmove(&(f->b), &(g->b), d);
}

void FP8_BLS48556_rand(FP8_BLS48556 *x,csprng *rng)
{
    FP4_BLS48556_rand(&(x->a),rng);
    FP4_BLS48556_rand(&(x->b),rng);
}

#if PAIRING_FRIENDLY_BLS48556 == BLS48_CURVE

/* test for x a QR */
int FP8_BLS48556_qr(FP8_BLS48556 *x, FP_BLS48556 *h)
{ /* test x^(p^4-1)/2 = 1 */

    FP8_BLS48556 c;
    FP8_BLS48556_conj(&c,x);
    FP8_BLS48556_mul(&c,&c,x);

    return FP4_BLS48556_qr(&(c.a),h);
}

/* sqrt(a+xb) = sqrt((a+sqrt(a*a-n*b*b))/2)+x.b/(2*sqrt((a+sqrt(a*a-n*b*b))/2)) */

void FP8_BLS48556_sqrt(FP8_BLS48556 *r, FP8_BLS48556* x, FP_BLS48556 *h)
{
    FP4_BLS48556 a, b, s, t;
    FP8_BLS48556 nr;
    FP_BLS48556 hint,twk;
    int sgn,qr;

    FP8_BLS48556_copy(r, x);
    if (FP8_BLS48556_iszilch(x)) return;

    FP4_BLS48556_copy(&a, &(x->a));
    FP4_BLS48556_copy(&s, &(x->b));

    FP4_BLS48556_sqr(&s, &s); // s*=s
    FP4_BLS48556_sqr(&a, &a); // a*=a
    FP4_BLS48556_times_i(&s);
    FP4_BLS48556_norm(&s);
    FP4_BLS48556_sub(&a, &a, &s); // a-=txx(s)
    FP4_BLS48556_norm(&a);
    FP4_BLS48556_sqrt(&s, &a, h);
   
    FP4_BLS48556_copy(&t, &(x->a));
    FP4_BLS48556_add(&a, &t, &s);
    FP4_BLS48556_norm(&a);
    FP4_BLS48556_div2(&a, &a);

    FP4_BLS48556_div2(&b,&(r->b));                   // w1=b/2
    qr=FP4_BLS48556_qr(&a,&hint);                    // only exp! Cost=+1

// tweak hint - multiply old hint by Norm(1/Beta)^e where Beta is irreducible polynomial
    FP4_BLS48556_copy(&s,&a);
    FP_BLS48556_rcopy(&twk,TWK_BLS48556);
    FP_BLS48556_mul(&twk,&twk,&hint);
    FP4_BLS48556_div_i(&s); FP4_BLS48556_norm(&s); // switch to other candidate

    FP4_BLS48556_cmove(&a,&s,1-qr);
    FP_BLS48556_cmove(&hint,&twk,1-qr);

    FP4_BLS48556_sqrt(&(r->a),&a,&hint);             // a=sqrt(w2)  Cost=+1
    FP4_BLS48556_inv(&s,&a,&hint);                  // w3=1/w2
    FP4_BLS48556_mul(&s,&s,&(r->a));                // w3=1/sqrt(w2)
    FP4_BLS48556_mul(&(r->b),&s,&b);                // b=(b/2)*1/sqrt(w2)
    FP4_BLS48556_copy(&t,&(r->a));

    FP4_BLS48556_cmove(&(r->a),&(r->b),1-qr);
    FP4_BLS48556_cmove(&(r->b),&t,1-qr);


    sgn=FP8_BLS48556_sign(r);
    FP8_BLS48556_neg(&nr,r); FP8_BLS48556_norm(&nr);
    FP8_BLS48556_cmove(r,&nr,sgn);
}


void FP8_BLS48556_div_i(FP8_BLS48556 *f)
{
    FP4_BLS48556 u, v;
    FP4_BLS48556_copy(&u, &(f->a));
    FP4_BLS48556_copy(&v, &(f->b));
    FP4_BLS48556_div_i(&u);
    FP4_BLS48556_copy(&(f->a), &v);
    FP4_BLS48556_copy(&(f->b), &u);
#if TOWER_BLS48556 == POSITOWER
    FP8_BLS48556_neg(f, f);  // ***
    FP8_BLS48556_norm(f);
#endif
}
/*
void FP8_BLS48556_div_i2(FP8_BLS48556 *f)
{
    FP4_BLS48556_div_i(&(f->a));
    FP4_BLS48556_div_i(&(f->b));
}


void FP8_BLS48556_div_2i(FP8_BLS48556 *f)
{
    FP4_BLS48556 u,v;
    FP4_BLS48556_copy(&u,&(f->a));
    FP4_BLS48556_copy(&v,&(f->b));
    FP4_BLS48556_div_2i(&u);
    FP4_BLS48556_add(&v,&v,&v);
    FP4_BLS48556_norm(&v);
    FP4_BLS48556_copy(&(f->a),&v);
    FP4_BLS48556_copy(&(f->b),&u);
}
*/
#endif
