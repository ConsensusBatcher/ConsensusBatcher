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
#include "arch.h"
#include "fp_BLS48581.h"

/* Curve BLS48581 - Pairing friendly BLS48 curve */

#if CHUNK==16

#error Not supported

#endif

#if CHUNK==32

// Base Bits= 29
const BIG_584_29 Modulus_BLS48581= {0x565912B,0x16E0AA73,0x12922B0F,0x1FBEE434,0xEE0A578,0x12A898B8,0xBDA0D9E,0x9E8E6DB,0x19CD3039,0x17041566,0x2B90EBD,0xEA88949,0xC0F1F39,0x18DD9DF3,0x1E344884,0xADD09,0x1D47012A,0x9C12718,0x1CD1DBCC,0x501EE7F,0x1};
const BIG_584_29 ROI_BLS48581= {0x565912A,0x16E0AA73,0x12922B0F,0x1FBEE434,0xEE0A578,0x12A898B8,0xBDA0D9E,0x9E8E6DB,0x19CD3039,0x17041566,0x2B90EBD,0xEA88949,0xC0F1F39,0x18DD9DF3,0x1E344884,0xADD09,0x1D47012A,0x9C12718,0x1CD1DBCC,0x501EE7F,0x1};
const BIG_584_29 R2modp_BLS48581= {0x19BCC5B0,0x126A343E,0xA46F6C0,0x29B7799,0xB758510,0x1357043D,0x1DC2D482,0x115A8C75,0x1F6CA5F6,0x9B40365,0x16E1C7DB,0x1A304684,0xA85F60F,0x16E21141,0x1D5AE21,0xD9EA745,0x1641FB69,0x249C4AC,0xA2B23E1,0x14FCE472,0x0};
const chunk MConst_BLS48581= 0x39D5A7D;
const BIG_584_29 CRu_BLS48581= {0x1C79B90,0x6F4D62F,0x3977A09,0x19B1C7F1,0x1421A205,0x190FBA7B,0x16D758AA,0x129736C1,0x2F43166,0x115AFDC,0x1E9AA9CD,0xD508D6B,0xBED6FF4,0x1360F2FE,0x95960F5,0x709E55F,0x1F4E2096,0x2AEEAE8,0xB1A2DD4,0x0,0x0};
const BIG_584_29 Fra_BLS48581= {0x42AEB25,0x1175B67F,0x182B4FE5,0x1683284E,0x115ECE4D,0x1EF92B83,0x22B03E9,0x6EF6A24,0x8C41890,0x4249F0E,0x61F9A1,0xC60FC53,0x16B62F49,0x71E57E,0xF9E161D,0xFA08F30,0x1E55F606,0x1AE6BF39,0x140527E9,0x1FA61B6,0x0};
const BIG_584_29 Frb_BLS48581= {0x42AEB25,0x1175B67F,0x182B4FE5,0x1683284E,0x115ECE4D,0x1EF92B83,0x22B03E9,0x6EF6A24,0x8C41890,0x4249F0E,0x61F9A1,0xC60FC53,0x16B62F49,0x71E57E,0xF9E161D,0xFA08F30,0x1E55F606,0x1AE6BF39,0x140527E9,0x1FA61B6,0x0};
const BIG_584_29 SQRTm3_BLS48581= {0x1D65A0A,0x8F6FE15,0xB6336FD,0xC5B5452,0x69D616D,0x8923C1,0x1E2B5C49,0x4BA7957,0x13E4CD6C,0x14D8B5AE,0x583BB23,0x14076E71,0x14343F50,0x121BB7F6,0xB818699,0x11F7124B,0x1EAABFFD,0x4635146,0x69D8024,0x501EE7F,0x1};
const BIG_584_29 TWK_BLS48581= {0x1F426953,0x1DA19E92,0xEE618F5,0x159C8B24,0xD37E160,0x145DEFD1,0xD2F0630,0x1FF9915F,0x2C4F62,0xDA1C978,0x51E0598,0x4FD2A34,0x132462ED,0x13341DC2,0x81D59C2,0x19374271,0x1D9EAEB8,0x1ABB4E6E,0xDEF0401,0x74CB4A,0x0};
#endif

#if CHUNK==64

// Base Bits= 60
const BIG_584_60 Modulus_BLS48581= {0xEDC154E6565912BL,0x8FDF721A4A48AC3L,0x7A5513170EE0A57L,0x394F4736DAF6836L,0xAF6E082ACD9CD30L,0xF3975444A48AE43L,0x22131BB3BE6C0F1L,0x12A0056E84F8D1L,0x76F313824E31D47L,0x1280F73FF34L};
const BIG_584_60 ROI_BLS48581= {0xEDC154E6565912AL,0x8FDF721A4A48AC3L,0x7A5513170EE0A57L,0x394F4736DAF6836L,0xAF6E082ACD9CD30L,0xF3975444A48AE43L,0x22131BB3BE6C0F1L,0x12A0056E84F8D1L,0x76F313824E31D47L,0x1280F73FF34L};
const BIG_584_60 R2modp_BLS48581= {0x79868479F1B5833L,0xFB6EBA8FCB82D07L,0x9CC8A7F1FD84C7FL,0x402C51CF5CC3CBBL,0x3F3114F078502CL,0xFC90829BDC8336EL,0xC7BE91DE9CA8EEDL,0xD4D273BB17BFADBL,0x6EC7C9A81E792CAL,0x1DC317A6E4L};
const chunk MConst_BLS48581= 0x148B81FC39D5A7DL;
const BIG_584_60 CRu_BLS48581= {0x4DE9AC5E1C79B90L,0x5CD8E3F88E5DE82L,0xAB21F74F7421A20L,0x6694B9B60DB5D62L,0x73422B5FB82F431L,0xFF46A846B5FA6AAL,0x83D66C1E5FCBED6L,0x2096384F2AFA565L,0x8B75055DD5D1F4EL,0x2C6L};
const BIG_584_60 Fra_BLS48581= {0x62EB6CFE42AEB25L,0xDB41942760AD3F9L,0xA7DF2570715ECE4L,0x90377B51208AC0FL,0x6848493E1C8C418L,0xF496307E298187EL,0x58740E3CAFD6B62L,0xF6067D047983E78L,0x49FA75CD7E73E55L,0xFD30DB501L};
const BIG_584_60 Frb_BLS48581= {0x62EB6CFE42AEB25L,0xDB41942760AD3F9L,0xA7DF2570715ECE4L,0x90377B51208AC0FL,0x6848493E1C8C418L,0xF496307E298187EL,0x58740E3CAFD6B62L,0xF6067D047983E78L,0x49FA75CD7E73E55L,0xFD30DB501L};
const BIG_584_60 SQRTm3_BLS48581= {0x51EDFC2A1D65A0AL,0xD62DAA292D8CDBFL,0x24112478269D616L,0x6C25D3CABF8AD71L,0xC8E9B16B5D3E4CDL,0xF50A03B738960EEL,0x1A664376FED4343L,0xBFFD8FB8925AE06L,0x600908C6A28DEAAL,0x1280F73F9A7L};
const BIG_584_60 TWK_BLS48581= {0x7B433D25F426953L,0xACE45923B9863DL,0xC28BBDFA2D37E16L,0x62FFCC8AFB4BC18L,0x661B4392F002C4FL,0x2ED27E951A14781L,0x670A6683B853246L,0xAEB8C9BA138A075L,0xC10075769CDDD9EL,0x3A65A537BL};
#endif