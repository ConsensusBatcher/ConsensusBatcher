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

/* FP16 elements are of the form a+ib, where i is sqrt(sqrt(-1+sqrt(-1))) */

#include "fp16_BLS48556.h"


/* test x==0 ? */
int FP16_BLS48556_iszilch(FP16_BLS48556 *x)
{
    if (FP8_BLS48556_iszilch(&(x->a)) && FP8_BLS48556_iszilch(&(x->b))) return 1;
    return 0;
}

/* test x==1 ? */
int FP16_BLS48556_isunity(FP16_BLS48556 *x)
{
    if (FP8_BLS48556_isunity(&(x->a)) && FP8_BLS48556_iszilch(&(x->b))) return 1;
    return 0;
}

/* test is w real? That is in a+ib test b is zero */
int FP16_BLS48556_isreal(FP16_BLS48556 *w)
{
    return FP8_BLS48556_iszilch(&(w->b));
}

/* return 1 if x==y, else 0 */
int FP16_BLS48556_equals(FP16_BLS48556 *x, FP16_BLS48556 *y)
{
    if (FP8_BLS48556_equals(&(x->a), &(y->a)) && FP8_BLS48556_equals(&(x->b), &(y->b)))
        return 1;
    return 0;
}

void FP16_BLS48556_toBytes(char *b,FP16_BLS48556 *x)
{
    FP8_BLS48556_toBytes(b,&(x->b));
    FP8_BLS48556_toBytes(&b[8*MODBYTES_560_29],&(x->a));
}

void FP16_BLS48556_fromBytes(FP16_BLS48556 *x,char *b)
{
    FP8_BLS48556_fromBytes(&(x->b),b);
    FP8_BLS48556_fromBytes(&(x->a),&b[8*MODBYTES_560_29]);
}

/* set FP16 from two FP8s */
void FP16_BLS48556_from_FP8s(FP16_BLS48556 *w, FP8_BLS48556 * x, FP8_BLS48556* y)
{
    FP8_BLS48556_copy(&(w->a), x);
    FP8_BLS48556_copy(&(w->b), y);
}

/* set FP16 from FP8 */
void FP16_BLS48556_from_FP8(FP16_BLS48556 *w, FP8_BLS48556 *x)
{
    FP8_BLS48556_copy(&(w->a), x);
    FP8_BLS48556_zero(&(w->b));
}

/* set high part of FP16 from FP8 */
void FP16_BLS48556_from_FP8H(FP16_BLS48556 *w, FP8_BLS48556 *x)
{
    FP8_BLS48556_copy(&(w->b), x);
    FP8_BLS48556_zero(&(w->a));
}

/* FP16 copy w=x */
void FP16_BLS48556_copy(FP16_BLS48556 *w, FP16_BLS48556 *x)
{
    if (w == x) return;
    FP8_BLS48556_copy(&(w->a), &(x->a));
    FP8_BLS48556_copy(&(w->b), &(x->b));
}

/* FP16 w=0 */
void FP16_BLS48556_zero(FP16_BLS48556 *w)
{
    FP8_BLS48556_zero(&(w->a));
    FP8_BLS48556_zero(&(w->b));
}

/* FP16 w=1 */
void FP16_BLS48556_one(FP16_BLS48556 *w)
{
    FP8_BLS48556_one(&(w->a));
    FP8_BLS48556_zero(&(w->b));
}

/* Set w=-x */
void FP16_BLS48556_neg(FP16_BLS48556 *w, FP16_BLS48556 *x)
{
    /* Just one field neg */
    FP8_BLS48556 m, t;
    FP16_BLS48556_norm(x);
    FP8_BLS48556_add(&m, &(x->a), &(x->b));
    FP8_BLS48556_norm(&m);
    FP8_BLS48556_neg(&m, &m);
    FP8_BLS48556_add(&t, &m, &(x->b));
    FP8_BLS48556_add(&(w->b), &m, &(x->a));
    FP8_BLS48556_copy(&(w->a), &t);
    FP16_BLS48556_norm(w);
}

/* Set w=conj(x) */
void FP16_BLS48556_conj(FP16_BLS48556 *w, FP16_BLS48556 *x)
{
    FP8_BLS48556_copy(&(w->a), &(x->a));
    FP8_BLS48556_neg(&(w->b), &(x->b));
    FP16_BLS48556_norm(w);
}

/* Set w=-conj(x) */
void FP16_BLS48556_nconj(FP16_BLS48556 *w, FP16_BLS48556 *x)
{
    FP8_BLS48556_copy(&(w->b), &(x->b));
    FP8_BLS48556_neg(&(w->a), &(x->a));
    FP16_BLS48556_norm(w);
}

/* Set w=x+y */
void FP16_BLS48556_add(FP16_BLS48556 *w, FP16_BLS48556 *x, FP16_BLS48556 *y)
{
    FP8_BLS48556_add(&(w->a), &(x->a), &(y->a));
    FP8_BLS48556_add(&(w->b), &(x->b), &(y->b));
}

/* Set w=x-y */
/* Input y MUST be normed */
void FP16_BLS48556_sub(FP16_BLS48556 *w, FP16_BLS48556 *x, FP16_BLS48556 *y)
{
    FP16_BLS48556 my;

    FP16_BLS48556_neg(&my, y);
    FP16_BLS48556_add(w, x, &my);

}

/* reduce all components of w mod Modulus */
void FP16_BLS48556_reduce(FP16_BLS48556 *w)
{
    FP8_BLS48556_reduce(&(w->a));
    FP8_BLS48556_reduce(&(w->b));
}

/* normalise all elements of w */
void FP16_BLS48556_norm(FP16_BLS48556 *w)
{
    FP8_BLS48556_norm(&(w->a));
    FP8_BLS48556_norm(&(w->b));
}

/* Set w=s*x, where s is FP8 */
void FP16_BLS48556_pmul(FP16_BLS48556 *w, FP16_BLS48556 *x, FP8_BLS48556 *s)
{
    FP8_BLS48556_mul(&(w->a), &(x->a), s);
    FP8_BLS48556_mul(&(w->b), &(x->b), s);
}

/* Set w=s*x, where s is FP2 */
void FP16_BLS48556_qmul(FP16_BLS48556 *w, FP16_BLS48556 *x, FP2_BLS48556 *s)
{
    FP8_BLS48556_qmul(&(w->a), &(x->a), s);
    FP8_BLS48556_qmul(&(w->b), &(x->b), s);
}

/* Set w=s*x, where s is FP */
void FP16_BLS48556_tmul(FP16_BLS48556 *w, FP16_BLS48556 *x, FP_BLS48556 *s)
{
    FP8_BLS48556_tmul(&(w->a), &(x->a), s);
    FP8_BLS48556_tmul(&(w->b), &(x->b), s);
}

/* Set w=s*x, where s is int */
void FP16_BLS48556_imul(FP16_BLS48556 *w, FP16_BLS48556 *x, int s)
{
    FP8_BLS48556_imul(&(w->a), &(x->a), s);
    FP8_BLS48556_imul(&(w->b), &(x->b), s);
}

/* Set w=x^2 */
/* Input MUST be normed  */
void FP16_BLS48556_sqr(FP16_BLS48556 *w, FP16_BLS48556 *x)
{
    FP8_BLS48556 t1, t2, t3;

    FP8_BLS48556_mul(&t3, &(x->a), &(x->b)); /* norms x */
    FP8_BLS48556_copy(&t2, &(x->b));
    FP8_BLS48556_add(&t1, &(x->a), &(x->b));
    FP8_BLS48556_times_i(&t2);

    FP8_BLS48556_add(&t2, &(x->a), &t2);

    FP8_BLS48556_norm(&t1);  // 2
    FP8_BLS48556_norm(&t2);  // 2

    FP8_BLS48556_mul(&(w->a), &t1, &t2);

    FP8_BLS48556_copy(&t2, &t3);
    FP8_BLS48556_times_i(&t2);

    FP8_BLS48556_add(&t2, &t2, &t3);

    FP8_BLS48556_norm(&t2);  // 2
    FP8_BLS48556_neg(&t2, &t2);
    FP8_BLS48556_add(&(w->a), &(w->a), &t2); /* a=(a+b)(a+i^2.b)-i^2.ab-ab = a*a+ib*ib */
    FP8_BLS48556_add(&(w->b), &t3, &t3); /* b=2ab */

    FP16_BLS48556_norm(w);
}

/* Set w=x*y */
/* Inputs MUST be normed  */
void FP16_BLS48556_mul(FP16_BLS48556 *w, FP16_BLS48556 *x, FP16_BLS48556 *y)
{

    FP8_BLS48556 t1, t2, t3, t4;
    FP8_BLS48556_mul(&t1, &(x->a), &(y->a));
    FP8_BLS48556_mul(&t2, &(x->b), &(y->b));

    FP8_BLS48556_add(&t3, &(y->b), &(y->a));
    FP8_BLS48556_add(&t4, &(x->b), &(x->a));

    FP8_BLS48556_norm(&t4); // 2
    FP8_BLS48556_norm(&t3); // 2

    FP8_BLS48556_mul(&t4, &t4, &t3); /* (xa+xb)(ya+yb) */

    FP8_BLS48556_neg(&t3, &t1); // 1
    FP8_BLS48556_add(&t4, &t4, &t3); //t4E=3
    FP8_BLS48556_norm(&t4);

    FP8_BLS48556_neg(&t3, &t2); // 1
    FP8_BLS48556_add(&(w->b), &t4, &t3); //wbE=3

    FP8_BLS48556_times_i(&t2);
    FP8_BLS48556_add(&(w->a), &t2, &t1);

    FP16_BLS48556_norm(w);
}

/* output FP16 in format [a,b] */
void FP16_BLS48556_output(FP16_BLS48556 *w)
{
    printf("[");
    FP8_BLS48556_output(&(w->a));
    printf(",");
    FP8_BLS48556_output(&(w->b));
    printf("]");
}

void FP16_BLS48556_rawoutput(FP16_BLS48556 *w)
{
    printf("[");
    FP8_BLS48556_rawoutput(&(w->a));
    printf(",");
    FP8_BLS48556_rawoutput(&(w->b));
    printf("]");
}

/* Set w=1/x */
void FP16_BLS48556_inv(FP16_BLS48556 *w, FP16_BLS48556 *x)
{
    FP8_BLS48556 t1, t2;
    FP8_BLS48556_sqr(&t1, &(x->a));
    FP8_BLS48556_sqr(&t2, &(x->b));
    FP8_BLS48556_times_i(&t2);
    FP8_BLS48556_norm(&t2);

    FP8_BLS48556_sub(&t1, &t1, &t2);
    FP8_BLS48556_norm(&t1);

    FP8_BLS48556_inv(&t1, &t1, NULL);

    FP8_BLS48556_mul(&(w->a), &t1, &(x->a));
    FP8_BLS48556_neg(&t1, &t1);
    FP8_BLS48556_norm(&t1);
    FP8_BLS48556_mul(&(w->b), &t1, &(x->b));
}

/* w*=i where i = sqrt(sqrt(-1+sqrt(-1))) */
void FP16_BLS48556_times_i(FP16_BLS48556 *w)
{
    FP8_BLS48556 s, t;
    FP8_BLS48556_copy(&s, &(w->b));
    FP8_BLS48556_copy(&t, &(w->a));
    FP8_BLS48556_times_i(&s);
    FP8_BLS48556_copy(&(w->a), &s);
    FP8_BLS48556_copy(&(w->b), &t);
    FP16_BLS48556_norm(w);
}

void FP16_BLS48556_times_i2(FP16_BLS48556 *w)
{
    FP8_BLS48556_times_i(&(w->a));
    FP8_BLS48556_times_i(&(w->b));
}

void FP16_BLS48556_times_i4(FP16_BLS48556 *w)
{
    FP8_BLS48556_times_i2(&(w->a));
    FP8_BLS48556_times_i2(&(w->b));
}

/* Set w=w^p using Frobenius */
void FP16_BLS48556_frob(FP16_BLS48556 *w, FP2_BLS48556 *f)
{   // f=(i+1)^(p-3)/8
    FP2_BLS48556 ff;

    FP2_BLS48556_sqr(&ff, f); // (i+1)^(p-3)/4
    FP2_BLS48556_norm(&ff);

    FP8_BLS48556_frob(&(w->a), &ff);
    FP8_BLS48556_frob(&(w->b), &ff);

    FP8_BLS48556_qmul(&(w->b), &(w->b), f); // times (1+i)^(p-3)/8
    FP8_BLS48556_times_i(&(w->b));       // (i+1)^(p-1)/8
}

/* Set r=a^b mod m */
void FP16_BLS48556_pow(FP16_BLS48556 *r, FP16_BLS48556 * a, BIG_560_29 b)
{
    FP16_BLS48556 w;
    BIG_560_29 z, zilch;
    int bt;

    BIG_560_29_zero(zilch);

    BIG_560_29_copy(z, b);
    FP16_BLS48556_copy(&w, a);
    FP16_BLS48556_one(r);
    BIG_560_29_norm(z);
    while (1)
    {
        bt = BIG_560_29_parity(z);
        BIG_560_29_shr(z, 1);
        if (bt) FP16_BLS48556_mul(r, r, &w);
        if (BIG_560_29_comp(z, zilch) == 0) break;
        FP16_BLS48556_sqr(&w, &w);
    }
    FP16_BLS48556_reduce(r);
}

/* Move b to a if d=1 */
void FP16_BLS48556_cmove(FP16_BLS48556 *f, FP16_BLS48556 *g, int d)
{
    FP8_BLS48556_cmove(&(f->a), &(g->a), d);
    FP8_BLS48556_cmove(&(f->b), &(g->b), d);
}

//#if CURVE_SECURITY_BLS48556 == 256

/* XTR xtr_a function */
/*
void FP16_BLS48556_xtr_A(FP16_BLS48556 *r, FP16_BLS48556 *w, FP16_BLS48556 *x, FP16_BLS48556 *y, FP16_BLS48556 *z)
{
    FP16_BLS48556 t1, t2;

    FP16_BLS48556_copy(r, x);
    FP16_BLS48556_sub(&t1, w, y);
    FP16_BLS48556_norm(&t1);
    FP16_BLS48556_pmul(&t1, &t1, &(r->a));
    FP16_BLS48556_add(&t2, w, y);
    FP16_BLS48556_norm(&t2);
    FP16_BLS48556_pmul(&t2, &t2, &(r->b));
    FP16_BLS48556_times_i(&t2);

    FP16_BLS48556_add(r, &t1, &t2);
    FP16_BLS48556_add(r, r, z);

    FP16_BLS48556_reduce(r);
}
*/
/* XTR xtr_d function */
/*
void FP16_BLS48556_xtr_D(FP16_BLS48556 *r, FP16_BLS48556 *x)
{
    FP16_BLS48556 w;
    FP16_BLS48556_copy(r, x);
    FP16_BLS48556_conj(&w, r);
    FP16_BLS48556_add(&w, &w, &w);
    FP16_BLS48556_sqr(r, r);
    FP16_BLS48556_norm(&w);
    FP16_BLS48556_sub(r, r, &w);
    FP16_BLS48556_reduce(r);    // reduce here as multiple calls trigger automatic reductions
}
*/

/* r=x^n using XTR method on traces of FP48s */
/*
void FP16_BLS48556_xtr_pow(FP16_BLS48556 *r, FP16_BLS48556 *x, BIG_560_29 n)
{
    int i, par, nb;
    BIG_560_29 v;
    FP2_BLS48556 w2;
    FP4_BLS48556 w4;
    FP8_BLS48556 w8;
    FP16_BLS48556 t, a, b, c, sf;

    BIG_560_29_zero(v);
    BIG_560_29_inc(v, 3);
    BIG_560_29_norm(v);
    FP2_BLS48556_from_BIG(&w2, v);
    FP4_BLS48556_from_FP2(&w4, &w2);
    FP8_BLS48556_from_FP4(&w8, &w4);
    FP16_BLS48556_from_FP8(&a, &w8);
    FP16_BLS48556_copy(&sf, x);
    FP16_BLS48556_norm(&sf);
    FP16_BLS48556_copy(&b, &sf);
    FP16_BLS48556_xtr_D(&c, &sf);


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
            FP16_BLS48556_copy(&t, &b);
            FP16_BLS48556_conj(&sf, &sf);
            FP16_BLS48556_conj(&c, &c);
            FP16_BLS48556_xtr_A(&b, &a, &b, &sf, &c);
            FP16_BLS48556_conj(&sf, &sf);
            FP16_BLS48556_xtr_D(&c, &t);
            FP16_BLS48556_xtr_D(&a, &a);
        }
        else
        {
            FP16_BLS48556_conj(&t, &a);
            FP16_BLS48556_xtr_D(&a, &b);
            FP16_BLS48556_xtr_A(&b, &c, &b, &sf, &t);
            FP16_BLS48556_xtr_D(&c, &c);
        }
    }

    if (par == 0) FP16_BLS48556_copy(r, &c);
    else FP16_BLS48556_copy(r, &b);
    FP16_BLS48556_reduce(r);
}
*/
/* r=ck^a.cl^n using XTR double exponentiation method on traces of FP48s. See Stam thesis. */
/*
void FP16_BLS48556_xtr_pow2(FP16_BLS48556 *r, FP16_BLS48556 *ck, FP16_BLS48556 *cl, FP16_BLS48556 *ckml, FP16_BLS48556 *ckm2l, BIG_560_29 a, BIG_560_29 b)
{
    int i, f2;
    BIG_560_29 d, e, w;
    FP16_BLS48556 t, cu, cv, cumv, cum2v;


    BIG_560_29_copy(e, a);
    BIG_560_29_copy(d, b);
    BIG_560_29_norm(d);
    BIG_560_29_norm(e);
    FP16_BLS48556_copy(&cu, ck);
    FP16_BLS48556_copy(&cv, cl);
    FP16_BLS48556_copy(&cumv, ckml);
    FP16_BLS48556_copy(&cum2v, ckm2l);

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
                FP16_BLS48556_xtr_A(&t, &cu, &cv, &cumv, &cum2v);
                FP16_BLS48556_conj(&cum2v, &cumv);
                FP16_BLS48556_copy(&cumv, &cv);
                FP16_BLS48556_copy(&cv, &cu);
                FP16_BLS48556_copy(&cu, &t);
            }
            else if (BIG_560_29_parity(d) == 0)
            {
                BIG_560_29_shr(d, 1);
                FP16_BLS48556_conj(r, &cum2v);
                FP16_BLS48556_xtr_A(&t, &cu, &cumv, &cv, r);
                FP16_BLS48556_xtr_D(&cum2v, &cumv);
                FP16_BLS48556_copy(&cumv, &t);
                FP16_BLS48556_xtr_D(&cu, &cu);
            }
            else if (BIG_560_29_parity(e) == 1)
            {
                BIG_560_29_sub(d, d, e);
                BIG_560_29_norm(d);
                BIG_560_29_shr(d, 1);
                FP16_BLS48556_xtr_A(&t, &cu, &cv, &cumv, &cum2v);
                FP16_BLS48556_xtr_D(&cu, &cu);
                FP16_BLS48556_xtr_D(&cum2v, &cv);
                FP16_BLS48556_conj(&cum2v, &cum2v);
                FP16_BLS48556_copy(&cv, &t);
            }
            else
            {
                BIG_560_29_copy(w, d);
                BIG_560_29_copy(d, e);
                BIG_560_29_shr(d, 1);
                BIG_560_29_copy(e, w);
                FP16_BLS48556_xtr_D(&t, &cumv);
                FP16_BLS48556_conj(&cumv, &cum2v);
                FP16_BLS48556_conj(&cum2v, &t);
                FP16_BLS48556_xtr_D(&t, &cv);
                FP16_BLS48556_copy(&cv, &cu);
                FP16_BLS48556_copy(&cu, &t);
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
                FP16_BLS48556_xtr_A(&t, &cu, &cv, &cumv, &cum2v);
                FP16_BLS48556_copy(&cum2v, &cumv);
                FP16_BLS48556_copy(&cumv, &cu);
                FP16_BLS48556_copy(&cu, &t);
            }
            else if (BIG_560_29_parity(e) == 0)
            {
                BIG_560_29_copy(w, d);
                BIG_560_29_copy(d, e);
                BIG_560_29_shr(d, 1);
                BIG_560_29_copy(e, w);
                FP16_BLS48556_xtr_D(&t, &cumv);
                FP16_BLS48556_conj(&cumv, &cum2v);
                FP16_BLS48556_conj(&cum2v, &t);
                FP16_BLS48556_xtr_D(&t, &cv);
                FP16_BLS48556_copy(&cv, &cu);
                FP16_BLS48556_copy(&cu, &t);
            }
            else if (BIG_560_29_parity(d) == 1)
            {
                BIG_560_29_copy(w, e);
                BIG_560_29_copy(e, d);
                BIG_560_29_sub(w, w, d);
                BIG_560_29_norm(w);
                BIG_560_29_copy(d, w);
                BIG_560_29_shr(d, 1);
                FP16_BLS48556_xtr_A(&t, &cu, &cv, &cumv, &cum2v);
                FP16_BLS48556_conj(&cumv, &cumv);
                FP16_BLS48556_xtr_D(&cum2v, &cu);
                FP16_BLS48556_conj(&cum2v, &cum2v);
                FP16_BLS48556_xtr_D(&cu, &cv);
                FP16_BLS48556_copy(&cv, &t);
            }
            else
            {
                BIG_560_29_shr(d, 1);
                FP16_BLS48556_conj(r, &cum2v);
                FP16_BLS48556_xtr_A(&t, &cu, &cumv, &cv, r);
                FP16_BLS48556_xtr_D(&cum2v, &cumv);
                FP16_BLS48556_copy(&cumv, &t);
                FP16_BLS48556_xtr_D(&cu, &cu);
            }
        }
    }
    FP16_BLS48556_xtr_A(r, &cu, &cv, &cumv, &cum2v);
    for (i = 0; i < f2; i++)    FP16_BLS48556_xtr_D(r, r);
    FP16_BLS48556_xtr_pow(r, r, d);
}

#endif
*/
