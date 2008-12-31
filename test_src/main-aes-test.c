/* main-aes-test.c */
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
/*
 * AES test-suit
 * 
*/

#include "config.h"
#include "serial-tools.h"
#include "uart.h"
#include "debug.h"

#include "aes.h"
#include "aes128_enc.h"
#include "aes128_dec.h"
#include "aes192_enc.h"
#include "aes192_dec.h"
#include "aes256_enc.h"
#include "aes256_dec.h"
#include "aes_keyschedule.h"

#include "nessie_bc_test.h"
#include "cli.h"
#include "performance_test.h"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

char* cipher_name = "AES";

/*****************************************************************************
 *  additional validation-functions											 *
 *****************************************************************************/

void testrun_nessie_aes(void){
	nessie_bc_ctx.blocksize_B =  16;
	nessie_bc_ctx.keysize_b   = 128;
	nessie_bc_ctx.name        = cipher_name;
	nessie_bc_ctx.ctx_size_B  = sizeof(aes128_ctx_t);
	nessie_bc_ctx.cipher_enc  = (nessie_bc_enc_fpt)aes128_enc;
	nessie_bc_ctx.cipher_dec  = (nessie_bc_dec_fpt)aes128_dec;
	nessie_bc_ctx.cipher_genctx  = (nessie_bc_gen_fpt)aes_init;
	nessie_bc_run();
	
	nessie_bc_ctx.keysize_b   = 192;
	nessie_bc_ctx.ctx_size_B  = sizeof(aes192_ctx_t);
	nessie_bc_ctx.cipher_enc  = (nessie_bc_enc_fpt)aes192_enc;
	nessie_bc_ctx.cipher_dec  = (nessie_bc_dec_fpt)aes192_dec;
	nessie_bc_run();
	
	nessie_bc_ctx.keysize_b   = 256;
	nessie_bc_ctx.ctx_size_B  = sizeof(aes256_ctx_t);
	nessie_bc_ctx.cipher_enc  = (nessie_bc_enc_fpt)aes256_enc;
	nessie_bc_ctx.cipher_dec  = (nessie_bc_dec_fpt)aes256_dec;
	nessie_bc_run(); 
}

void testrun_test_aes(void){
	uint8_t key[16] = { 0x2b, 0x7e, 0x15, 0x16, 
	                    0x28, 0xae, 0xd2, 0xa6,
	                    0xab, 0xf7, 0x15, 0x88,
	                    0x09, 0xcf, 0x4f, 0x3c };
	uint8_t data[16] = { 0x32, 0x43, 0xf6, 0xa8,
	                     0x88, 0x5a, 0x30, 0x8d, 
	                     0x31, 0x31, 0x98, 0xa2, 
	                     0xe0, 0x37, 0x07, 0x34 };
	aes128_ctx_t ctx;
	aes128_init(key, &ctx);
	uart_putstr_P(PSTR("\r\n\r\n cipher test (FIPS 197):\r\n key:        "));
	uart_hexdump(key, 16);
	uart_putstr_P(PSTR("\r\n plaintext:  "));
	uart_hexdump(data, 16);
	aes128_enc(data, &ctx);
	uart_putstr_P(PSTR("\r\n ciphertext: "));
	uart_hexdump(data, 16);
	
	
}

void testrun_testkey_aes(void){
	uint8_t key[16] = { 0x2b, 0x7e, 0x15, 0x16, 
	                    0x28, 0xae, 0xd2, 0xa6,
	                    0xab, 0xf7, 0x15, 0x88,
	                    0x09, 0xcf, 0x4f, 0x3c};
	aes128_ctx_t ctx;
	uint8_t i;
	aes128_init(key, &ctx);
	uart_putstr_P(PSTR("\r\n\r\n keyschedule test (FIPS 197):\r\n key:   "));
	uart_hexdump(key, 16);
	for(i=0; i<11; ++i){
		uart_putstr_P(PSTR("\r\n index: "));
		uart_putc('0'+i/10);
		uart_putc('0'+i%10);
		uart_putstr_P(PSTR(" roundkey "));
		uart_hexdump(ctx.key[i].ks, 16);
	}
}

void testrun_performance_aes(void){
	uint64_t t;
	char str[16];
	uint8_t key[32], data[16];
	aes128_ctx_t ctx;
	
	calibrateTimer();
	print_overhead();
	
	memset(key,  0, 32);
	memset(data, 0, 16);
	
	startTimer(1);
	aes128_init(key, &ctx);
	t = stopTimer();
	uart_putstr_P(PSTR("\r\n\tctx-gen time: "));
	ultoa((unsigned long)t, str, 10);
	uart_putstr(str);
	
	
	startTimer(1);
	aes128_enc(data, &ctx);
	t = stopTimer();
	uart_putstr_P(PSTR("\r\n\tencrypt time: "));
	ultoa((unsigned long)t, str, 10);
	uart_putstr(str);
	
	
	startTimer(1);
	aes128_dec(data, &ctx);
	t = stopTimer();
	uart_putstr_P(PSTR("\r\n\tdecrypt time: "));
	ultoa((unsigned long)t, str, 10);
	uart_putstr(str);
	
	uart_putstr_P(PSTR("\r\n"));
}
/*****************************************************************************
 *  main																	 *
 *****************************************************************************/

int main (void){
	char  str[20];
	DEBUG_INIT();
	uart_putstr("\r\n");

	uart_putstr_P(PSTR("\r\n\r\nCrypto-VS ("));
	uart_putstr(cipher_name);
	uart_putstr_P(PSTR(")\r\nloaded and running\r\n"));

	PGM_P    u   = PSTR("nessie\0test\0testkey\0performance\0");
	void_fpt v[] = {testrun_nessie_aes, 
	                testrun_test_aes, 
	                testrun_testkey_aes, 
	                testrun_performance_aes};

	while(1){ 
		if (!getnextwordn(str,20)){DEBUG_S("DBG: W1\r\n"); goto error;}
		if(execcommand_d0_P(str, u, v)<0){
			uart_putstr_P(PSTR("\r\nunknown command\r\n"));
		}
		continue;
	error:
		uart_putstr("ERROR\r\n");
	}
	
}

