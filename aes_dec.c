/* aes.c */
/*
    This file is part of the Crypto-avr-lib/microcrypt-lib.
    Copyright (C) 2008  Daniel Otte (daniel.otte@rub.de)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdint.h>
#include <string.h>
#include "gf256mul.h"
#include "aes.h"
#include "aes_invsbox.h"
#include "aes_dec.h"
#include <avr/pgmspace.h>

void aes_invshiftrow(void* data, uint8_t shift){
	uint8_t tmp[4];
	tmp[0] = ((uint8_t*)data)[(4+0-shift)&3];
	tmp[1] = ((uint8_t*)data)[(4+1-shift)&3];
	tmp[2] = ((uint8_t*)data)[(4+2-shift)&3];
	tmp[3] = ((uint8_t*)data)[(4+3-shift)&3];
	memcpy(data, tmp, 4);
}

static
void aes_dec_round(aes_cipher_state_t* state, const aes_roundkey_t* k){
	uint8_t tmp[16];
	uint8_t i;
	/* keyAdd */
	for(i=0; i<16; ++i){
		tmp[i] = state->s[i] ^ k->ks[i];
	}
	/* mixColums */
	for(i=0; i<4; ++i){
		state->s[4*0+i] =
			  gf256mul(0xe, tmp[4*0+i], 0x1b)
			^ gf256mul(0xb, tmp[4*1+i], 0x1b)
			^ gf256mul(0xd, tmp[4*2+i], 0x1b)
			^ gf256mul(0x9, tmp[4*3+i], 0x1b);
		state->s[4*1+i] =
			  gf256mul(0x9, tmp[4*0+i], 0x1b)
			^ gf256mul(0xe, tmp[4*1+i], 0x1b)
			^ gf256mul(0xb, tmp[4*2+i], 0x1b)
			^ gf256mul(0xd, tmp[4*3+i], 0x1b);
		state->s[4*2+i] =
			  gf256mul(0xd, tmp[4*0+i], 0x1b)
			^ gf256mul(0x9, tmp[4*1+i], 0x1b)
			^ gf256mul(0xe, tmp[4*2+i], 0x1b)
			^ gf256mul(0xb, tmp[4*3+i], 0x1b);
		state->s[4*3+i] =
			  gf256mul(0xb, tmp[4*0+i], 0x1b)
			^ gf256mul(0xd, tmp[4*1+i], 0x1b)
			^ gf256mul(0x9, tmp[4*2+i], 0x1b)
			^ gf256mul(0xe, tmp[4*3+i], 0x1b);
	}	
	/* shiftRows */
	aes_invshiftrow(state->s+4, 1);
	aes_invshiftrow(state->s+8, 2);
	aes_invshiftrow(state->s+12, 3);		
	/* subBytes */
	for(i=0; i<16; ++i){
		state->s[i] = pgm_read_byte(aes_invsbox+state->s[i]);
	}
}


static
void aes_dec_firstround(aes_cipher_state_t* state, const aes_roundkey_t* k){
	uint8_t i;
	/* keyAdd */
	for(i=0; i<16; ++i){
		state->s[i] ^= k->ks[i];
	}
	/* shiftRows */
	aes_invshiftrow(state->s+4, 1);
	aes_invshiftrow(state->s+8, 2);
	aes_invshiftrow(state->s+12, 3);		
	/* subBytes */
	for(i=0; i<16; ++i){
		state->s[i] = pgm_read_byte(aes_invsbox+state->s[i]);
	}
}

void aes_decrypt_core(aes_cipher_state_t* state, const aes_genctx_t* ks, uint8_t rounds){
	uint8_t i;
	aes_dec_firstround(state, &(ks->key[i=rounds]));
	for(;rounds>1;--rounds){
		--i;
		aes_dec_round(state, &(ks->key[i]));
	}
	for(i=0; i<16; ++i){
		state->s[i] ^= ks->key[0].ks[i];
	}
}
