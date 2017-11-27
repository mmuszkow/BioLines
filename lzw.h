/*
* BioLines (https://github.com/mmuszkow/BioLines)
* Detection of filamentous structures in biological microscopic images
* Copyright(C) 2017 Maciek Muszkowski
*
* This program is free software : you can redistribute it and / or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program.If not, see <http://www.gnu.org/licenses/>.
*/

// Based on: https://rosettacode.org/wiki/LZW_compression#C

#ifndef __LZW_H__
#define __LZW_H__

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct lzw_buff {
    size_t item_size;
    size_t length;
    /* this is not a pointer, it is only used to get the data offset inside the struct */
    void*  data;
};

/* Initializes buffer with 0's. */
struct lzw_buff* lzw_buff_alloc(size_t item_size, size_t length) {
    struct lzw_buff *buff = (struct lzw_buff*) calloc(1, sizeof(size_t) * 2 + item_size * length);
    buff->item_size = item_size;
    buff->length = length;
    return buff;
};

/* Extends or trims the buffer, new space is filled with 0's. */
struct lzw_buff* _lzw_buff_resize(struct lzw_buff* buff, size_t new_length) {
    size_t old_length = buff->length;
    buff = (struct lzw_buff*) realloc(buff, sizeof(size_t) * 2 + buff->item_size * new_length);
    if (new_length > old_length) {
        uint8_t* data = (uint8_t*) &buff->data;
        memset(data + buff->item_size * old_length, 0, buff->item_size * (new_length - old_length));
    }
    buff->length = new_length;
    return buff;
};

/* Fills buffer data with 0's. */
#define _lzw_zero(buff) memset(&buff->data, 0, buff->item_size * buff->length)

/* In case someone wonders. */
#define _lzw_free free

#define LZW_M_CLR 256 /* clear table marker */
#define LZW_M_EOD 257 /* end-of-data marker */
#define LZW_M_NEW 258 /* new code index */

struct lzw_enc_t {
    uint16_t next[256];
};

struct lzw_dec_t {
    uint16_t prev, back;
    uint8_t c;
};

/* writes bits to the output buffer, expands it when needed */
#define _lzw_write_bits(x) { \
    tmp = (tmp << bits) | x; \
    o_bits += bits; \
    if (outbuff->length <= out_len) { \
        outbuff = _lzw_buff_resize(outbuff, outbuff->length * 2); \
        outdata = (uint8_t*) &outbuff->data; \
    } \
    while (o_bits >= 8) { \
        o_bits -= 8; \
        outdata[out_len++] = tmp >> o_bits; \
        tmp &= (1 << o_bits) - 1; \
    } \
}

/* inbuff is 8-bit buffer, function can produce 9-15 bits encoded strings (@max_bits) */
struct lzw_buff* lzw_encode(struct lzw_buff* inbuff, int max_bits) {
    size_t len = inbuff->length;
    int bits = 9, next_shift = 512, out_len = 0, o_bits = 0;
    uint32_t tmp = 0;
    uint16_t code, c, nc, next_code = LZW_M_NEW;
    struct lzw_buff* dbuff = lzw_buff_alloc(sizeof(struct lzw_enc_t), 512);
    struct lzw_buff* outbuff = lzw_buff_alloc(2, 4);
    uint8_t* outdata = (uint8_t*) &outbuff->data;
    uint8_t* indata = (uint8_t*) &inbuff->data;
    struct lzw_enc_t* d = (struct lzw_enc_t*) &dbuff->data;
    
    if (max_bits > 15) max_bits = 15;
    if (max_bits < 9 ) max_bits = 12;
    
    for (code = *(indata++); --len; ) {
        c = *(indata++);
        if ((nc = d[code].next[c]))
            code = nc;
        else {
            _lzw_write_bits(code);
            nc = d[code].next[c] = next_code++;
            code = c;
        }
        
        if (next_code == next_shift) {
            if (++bits > max_bits) {
                _lzw_write_bits(LZW_M_CLR);
                bits = 9;
                next_shift = 512;
                next_code = LZW_M_NEW;
                _lzw_zero(dbuff);
            } else {
                dbuff = _lzw_buff_resize(dbuff, next_shift *= 2);
                d = (struct lzw_enc_t*) &dbuff->data;
            }
        }
    }
    
    _lzw_write_bits(code);
    _lzw_write_bits(LZW_M_EOD);
    if (tmp) _lzw_write_bits(tmp);
    free(dbuff);
    outbuff = _lzw_buff_resize(outbuff, out_len);
    return outbuff;
}

#define _lzw_write_out(c) { \
    if (out_len >= outbuff->length) { \
        outbuff = _lzw_buff_resize(outbuff, outbuff->length * 2); \
        outdata = (uint8_t*) &outbuff->data; \
    } \
    outdata[out_len++] = c; }

/* inbuff is 8-bit buffer */
struct lzw_buff* lzw_decode(struct lzw_buff* inbuff) {
    struct lzw_buff* outbuff = lzw_buff_alloc(1, 4);
    struct lzw_buff* dbuff = lzw_buff_alloc(sizeof(struct lzw_dec_t), 512);
    uint8_t* indata = (uint8_t*) &inbuff->data;
    uint8_t* outdata = (uint8_t*) &outbuff->data;
    struct lzw_dec_t* d = (struct lzw_dec_t*) &dbuff->data;
    int out_len = 0, j, next_shift = 512, bits = 9, n_bits = 0;
    size_t len;
    uint16_t code, c, t, next_code = LZW_M_NEW;
    uint32_t tmp = 0;
    
    for (j = 0; j < 256; j++) d[j].c = j;
    
    for (len = inbuff->length; len;) {
        while(n_bits < bits) {
            if (len > 0) {
                len --;
                tmp = (tmp << 8) | *(indata++);
                n_bits += 8;
            } else {
                tmp = tmp << (bits - n_bits);
                n_bits = bits;
            }
        }
        n_bits -= bits;
        code = tmp >> n_bits;
        tmp &= (1 << n_bits) - 1;
        
        if (code == LZW_M_EOD) break;
        if (code == LZW_M_CLR) {
            _lzw_zero(dbuff);
            for (j = 0; j < 256; j++) d[j].c = j;
            next_code = LZW_M_NEW;
            next_shift = 512;
            bits = 9;
            continue;
        }
        
        if (code >= next_code) {
            free(outbuff);
            outbuff = NULL;
            goto bail;
        }
        
        d[next_code].prev = c = code;
        while (c > 255) {
            t = d[c].prev; d[t].back = c; c = t;
        }
        
        d[next_code - 1].c = c;
        
        while (d[c].back) {
            _lzw_write_out(d[c].c);
            t = d[c].back; d[c].back = 0; c = t;
        }
        _lzw_write_out(d[c].c);
        
        if (++next_code >= next_shift) {
            if (++bits > 16) {
                free(outbuff);
                outbuff = NULL;
                goto bail;
            }
            dbuff = _lzw_buff_resize(dbuff, next_shift *= 2);
            d = (struct lzw_dec_t*) &dbuff->data;
        }
    }
    
    outbuff = _lzw_buff_resize(outbuff, out_len);
bail: free(dbuff);
    return outbuff;
}

void lzw_test(uint8_t* buff, size_t length) {
    float best_ratio = 99999;
    int best_bits = 0;
    struct lzw_buff* inbuff = lzw_buff_alloc(1, length);
    memcpy(&inbuff->data, buff, length);
    for(int max_bits = 9; max_bits <= 15; max_bits++) {
        struct lzw_buff* enc = lzw_encode(inbuff, max_bits);
        float ratio = 100 * (enc->length / (float) length);
        if(ratio < best_ratio) {
            best_ratio = ratio;
            best_bits = max_bits;
        }
        free(enc);
    }
    printf("Best compression (%f%% of the original size) with %d bits\n", best_ratio, best_bits);
}

#endif /* __LZW_H__ */
