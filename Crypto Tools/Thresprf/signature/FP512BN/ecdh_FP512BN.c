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

/* ECDH/ECIES/ECDSA API Functions */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "ecdh_FP512BN.h"


#if CURVETYPE_FP512BN!=WEIERSTRASS
// Process a random BIG r by RFC7748 (for Montgomery & Edwards curves only)
static void RFC7748(BIG_512_29 r)
{
    int c,lg=0;
    BIG_512_29 t;
    c=CURVE_Cof_I_FP512BN;
    while (c!=1)
    {
        lg++;
        c/=2;
    }
    int n=8*EGS_FP512BN-lg+1;
    BIG_512_29_mod2m(r,n);
    BIG_512_29_zero(t); BIG_512_29_inc(t,1); BIG_512_29_shl(t,n);
    BIG_512_29_add(r,r,t);
    c=BIG_512_29_lastbits(r,lg);
    BIG_512_29_dec(r,c);

}
#endif


/* return 1 if S is in ranger 0 < S < order , else return 0 */
int ECP_FP512BN_IN_RANGE(octet* S)
{
    BIG_512_29 r,s;
    BIG_512_29_rcopy(r, CURVE_Order_FP512BN);
    BIG_512_29_fromBytes(s,S->val);
    if (BIG_512_29_iszilch(s)) return 0;
    if (BIG_512_29_comp(s,r)>=0) return 0;
    return 1;
}


/* Calculate a public/private EC GF(p) key pair. W=S.G mod EC(p),
 * where S is the secret key and W is the public key
 * and G is fixed generator.
 * If RNG is NULL then the private key is provided externally in S
 * otherwise it is generated randomly internally */
int ECP_FP512BN_KEY_PAIR_GENERATE(csprng *RNG, octet* S, octet *W)
{
    BIG_512_29 r, s;
    ECP_FP512BN G;
    int res = 0;

    ECP_FP512BN_generator(&G);
    BIG_512_29_rcopy(r, CURVE_Order_FP512BN);

    if (RNG != NULL)
    {
#if CURVETYPE_FP512BN!=WEIERSTRASS
        BIG_512_29_random(s,RNG);          // from random bytes
#else
        BIG_512_29_randomnum(s, r, RNG);   // Removes biases
#endif
    }
    else
    {
        BIG_512_29_fromBytes(s, S->val);
    }

#if CURVETYPE_FP512BN!=WEIERSTRASS
    RFC7748(s);                     // For Montgomery or Edwards, apply RFC7748 transformation
#endif
    S->len = EGS_FP512BN;
    BIG_512_29_toBytes(S->val, s);

    ECP_FP512BN_clmul(&G, s, r);
    ECP_FP512BN_toOctet(W, &G, false); /* To use point compression on public keys, change to true */

    return res;
}

/* Validate public key */
int ECP_FP512BN_PUBLIC_KEY_VALIDATE(octet *W)
{
    BIG_512_29 q, r, k;
    ECP_FP512BN WP;
    int valid, nb;
    int res = 0;

    BIG_512_29_rcopy(q, Modulus_FP512BN);
    BIG_512_29_rcopy(r, CURVE_Order_FP512BN);

    valid = ECP_FP512BN_fromOctet(&WP, W);
    if (!valid) res = ECDH_INVALID_PUBLIC_KEY;

    if (res == 0)
    {   /* Check point is not in wrong group */
        nb = BIG_512_29_nbits(q);
        BIG_512_29_one(k);
        BIG_512_29_shl(k, (nb + 4) / 2);
        BIG_512_29_add(k, q, k);
        BIG_512_29_sdiv(k, r); /* get co-factor */

        while (BIG_512_29_parity(k) == 0)
        {
            ECP_FP512BN_dbl(&WP);
            BIG_512_29_fshr(k, 1);
        }

        if (!BIG_512_29_isunity(k)) ECP_FP512BN_mul(&WP, k);
        if (ECP_FP512BN_isinf(&WP)) res = ECDH_INVALID_PUBLIC_KEY;
    }

    return res;
}

/* IEEE-1363 Diffie-Hellman online calculation Z=S.WD */
// type = 0 is just x coordinate output
// type = 1 for standard compressed output
// type = 2 for standard uncompress output 04|x|y
int ECP_FP512BN_SVDP_DH(octet *S, octet *WD, octet *Z, int type)
{
    BIG_512_29 r, s, wx;
    int valid;
    ECP_FP512BN W;
    int res = 0;

    BIG_512_29_fromBytes(s, S->val);
    valid = ECP_FP512BN_fromOctet(&W, WD);

    if (!valid) res = ECDH_ERROR;
    if (res == 0)
    {
        BIG_512_29_rcopy(r, CURVE_Order_FP512BN);
        ECP_FP512BN_clmul(&W, s, r);
        if (ECP_FP512BN_isinf(&W)) res = ECDH_ERROR;
        else
        {
#if CURVETYPE_FP512BN!=MONTGOMERY
            if (type>0)
            {
                if (type==1) ECP_FP512BN_toOctet(Z,&W,true);
                else ECP_FP512BN_toOctet(Z,&W,false);
                return res;
            }
            else
                ECP_FP512BN_get(wx, wx, &W);
#else
            ECP_FP512BN_get(wx, &W);
#endif
            Z->len = EFS_FP512BN;
            BIG_512_29_toBytes(Z->val, wx);
        }
    }
    return res;
}

#if CURVETYPE_FP512BN!=MONTGOMERY

/* IEEE ECDSA Signature, C and D are signature on F using private key S */
int ECP_FP512BN_SP_DSA(int hlen, csprng *RNG, octet *K, octet *S, octet *F, octet *C, octet *D)
{
    char h[128];
    octet H = {0, sizeof(h), h};

    BIG_512_29 r, s, f, c, d, u, vx, w;
    ECP_FP512BN G, V;

    SPhash(MC_SHA2, hlen, &H, F);

    ECP_FP512BN_generator(&G);

    BIG_512_29_rcopy(r, CURVE_Order_FP512BN);
    BIG_512_29_fromBytes(s, S->val);

    int blen = H.len;
    if (H.len > EGS_FP512BN) blen = EGS_FP512BN;
    BIG_512_29_fromBytesLen(f, H.val, blen);

    if (RNG != NULL)
    {
        do
        {
            BIG_512_29_randomnum(u, r, RNG);
            BIG_512_29_randomnum(w, r, RNG); /* IMPORTANT - side channel masking to protect invmodp() */

            ECP_FP512BN_copy(&V, &G);
            ECP_FP512BN_clmul(&V, u, r);

            ECP_FP512BN_get(vx, vx, &V);

            BIG_512_29_copy(c, vx);
            BIG_512_29_mod(c, r);
            if (BIG_512_29_iszilch(c)) continue;

            BIG_512_29_modmul(u, u, w, r);

            BIG_512_29_invmodp(u, u, r);
            BIG_512_29_modmul(d, s, c, r);

            BIG_512_29_modadd(d, f, d, r);
            BIG_512_29_modmul(d, d, w, r);
            BIG_512_29_modmul(d, u, d, r);
        } while (BIG_512_29_iszilch(d));
    }
    else
    {
        BIG_512_29_fromBytes(u, K->val);

        ECP_FP512BN_copy(&V, &G);
        ECP_FP512BN_clmul(&V, u, r);

        ECP_FP512BN_get(vx, vx, &V);

        BIG_512_29_copy(c, vx);
        BIG_512_29_mod(c, r);
        if (BIG_512_29_iszilch(c)) return ECDH_ERROR;


        BIG_512_29_invmodp(u, u, r);
        BIG_512_29_modmul(d, s, c, r);

        BIG_512_29_modadd(d, f, d, r);
        BIG_512_29_modmul(d, u, d, r);
        if (BIG_512_29_iszilch(d)) return ECDH_ERROR;
    }

    C->len = D->len = EGS_FP512BN;

    BIG_512_29_toBytes(C->val, c);
    BIG_512_29_toBytes(D->val, d);

    return 0;
}

/* IEEE1363 ECDSA Signature Verification. Signature C and D on F is verified using public key W */
int ECP_FP512BN_VP_DSA(int hlen, octet *W, octet *F, octet *C, octet *D)
{
    char h[128];
    octet H = {0, sizeof(h), h};

    BIG_512_29 r, f, c, d, h2;
    int res = 0;
    ECP_FP512BN G, WP;
    int valid;

    SPhash(MC_SHA2, hlen, &H, F);

    ECP_FP512BN_generator(&G);

    BIG_512_29_rcopy(r, CURVE_Order_FP512BN);

    BIG_512_29_fromBytes(c, C->val);
    BIG_512_29_fromBytes(d, D->val);

    int blen = H.len;
    if (blen > EGS_FP512BN) blen = EGS_FP512BN;

    BIG_512_29_fromBytesLen(f, H.val, blen);

    if (BIG_512_29_iszilch(c) || BIG_512_29_comp(c, r) >= 0 || BIG_512_29_iszilch(d) || BIG_512_29_comp(d, r) >= 0)
        res = ECDH_ERROR;

    if (res == 0)
    {
        BIG_512_29_invmodp(d, d, r);
        BIG_512_29_modmul(f, f, d, r);
        BIG_512_29_modmul(h2, c, d, r);

        valid = ECP_FP512BN_fromOctet(&WP, W);

        if (!valid) res = ECDH_ERROR;
        else
        {
            ECP_FP512BN_mul2(&WP, &G, h2, f);
            if (ECP_FP512BN_isinf(&WP)) res = ECDH_ERROR;
            else
            {
                ECP_FP512BN_get(d, d, &WP);
                BIG_512_29_mod(d, r);
                if (BIG_512_29_comp(d, c) != 0) res = ECDH_ERROR;
            }
        }
    }

    return res;
}

/* IEEE1363 ECIES encryption. Encryption of plaintext M uses public key W and produces ciphertext V,C,T */
void ECP_FP512BN_ECIES_ENCRYPT(int hlen, octet *P1, octet *P2, csprng *RNG, octet *W, octet *M, int tlen, octet *V, octet *C, octet *T)
{

    int i, len;
    char z[EFS_FP512BN], vz[3 * EFS_FP512BN + 1], k[2 * AESKEY_FP512BN], k1[AESKEY_FP512BN], k2[AESKEY_FP512BN], l2[8], u[EFS_FP512BN];
    octet Z = {0, sizeof(z), z};
    octet VZ = {0, sizeof(vz), vz};
    octet K = {0, sizeof(k), k};
    octet K1 = {0, sizeof(k1), k1};
    octet K2 = {0, sizeof(k2), k2};
    octet L2 = {0, sizeof(l2), l2};
    octet U = {0, sizeof(u), u};

    if (ECP_FP512BN_KEY_PAIR_GENERATE(RNG, &U, V) != 0) return;
    if (ECP_FP512BN_SVDP_DH(&U, W, &Z, 0) != 0) return;

    OCT_copy(&VZ, V);
    OCT_joctet(&VZ, &Z);

    KDF2(MC_SHA2, hlen, &K, 2 * AESKEY_FP512BN, &VZ, P1);

    K1.len = K2.len = AESKEY_FP512BN;
    for (i = 0; i < AESKEY_FP512BN; i++)
    {
        K1.val[i] = K.val[i];
        K2.val[i] = K.val[AESKEY_FP512BN + i];
    }

    AES_CBC_IV0_ENCRYPT(&K1, M, C);

    OCT_jint(&L2, P2->len, 8);

    len = C->len;
    OCT_joctet(C, P2);
    OCT_joctet(C, &L2);
    HMAC(MC_SHA2, hlen, T, tlen, &K2, C);
    C->len = len;
}

/* IEEE1363 ECIES decryption. Decryption of ciphertext V,C,T using private key U outputs plaintext M */
int ECP_FP512BN_ECIES_DECRYPT(int hlen, octet *P1, octet *P2, octet *V, octet *C, octet *T, octet *U, octet *M)
{

    int i, len;
    char z[EFS_FP512BN], vz[3 * EFS_FP512BN + 1], k[2 * AESKEY_FP512BN], k1[AESKEY_FP512BN], k2[AESKEY_FP512BN], l2[8], tag[32];
    octet Z = {0, sizeof(z), z};
    octet VZ = {0, sizeof(vz), vz};
    octet K = {0, sizeof(k), k};
    octet K1 = {0, sizeof(k1), k1};
    octet K2 = {0, sizeof(k2), k2};
    octet L2 = {0, sizeof(l2), l2};
    octet TAG = {0, sizeof(tag), tag};

    if (ECP_FP512BN_SVDP_DH(U, V, &Z, 0) != 0) return 0;

    OCT_copy(&VZ, V);
    OCT_joctet(&VZ, &Z);

    KDF2(MC_SHA2, hlen, &K, 2 * AESKEY_FP512BN, &VZ, P1);

    K1.len = K2.len = AESKEY_FP512BN;
    for (i = 0; i < AESKEY_FP512BN; i++)
    {
        K1.val[i] = K.val[i];
        K2.val[i] = K.val[AESKEY_FP512BN + i];
    }

    if (!AES_CBC_IV0_DECRYPT(&K1, C, M)) return 0;

    OCT_jint(&L2, P2->len, 8);

    len = C->len;
    OCT_joctet(C, P2);
    OCT_joctet(C, &L2);
    HMAC(MC_SHA2, hlen, &TAG, T->len, &K2, C);
    C->len = len;

    if (!OCT_ncomp(T, &TAG, T->len)) return 0;

    return 1;

}

#endif