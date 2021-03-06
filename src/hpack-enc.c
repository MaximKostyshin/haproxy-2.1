/*
 * HPACK decompressor (RFC7541)
 *
 * Copyright (C) 2014-2017 Willy Tarreau <willy@haproxy.org>
 * Copyright (C) 2017 HAProxy Technologies
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <common/hpack-enc.h>
#include <common/http-hdr.h>
#include <common/ist.h>

#include <types/global.h>

/*
 * HPACK encoding: these tables were generated using gen-enc.c
 */

/* encoding of stream of compressed headers. This stream is composed of series
 * of <len:8b> <index:8b> <name:<len>*8b>.
 */
const char hpack_enc_stream[666] = {
         /*    0: */ 0x03, 0x15, 0x61, 0x67, 0x65, 0x03, 0x3c, 0x76,
         /*    8: */ 0x69, 0x61, 0x04, 0x21, 0x64, 0x61, 0x74, 0x65,
         /*   16: */ 0x04, 0x26, 0x68, 0x6f, 0x73, 0x74, 0x04, 0x22,
         /*   24: */ 0x65, 0x74, 0x61, 0x67, 0x04, 0x25, 0x66, 0x72,
         /*   32: */ 0x6f, 0x6d, 0x04, 0x2d, 0x6c, 0x69, 0x6e, 0x6b,
         /*   40: */ 0x04, 0x3b, 0x76, 0x61, 0x72, 0x79, 0x05, 0x04,
         /*   48: */ 0x3a, 0x70, 0x61, 0x74, 0x68, 0x05, 0x16, 0x61,
         /*   56: */ 0x6c, 0x6c, 0x6f, 0x77, 0x05, 0x32, 0x72, 0x61,
         /*   64: */ 0x6e, 0x67, 0x65, 0x06, 0x13, 0x61, 0x63, 0x63,
         /*   72: */ 0x65, 0x70, 0x74, 0x06, 0x36, 0x73, 0x65, 0x72,
         /*   80: */ 0x76, 0x65, 0x72, 0x06, 0x20, 0x63, 0x6f, 0x6f,
         /*   88: */ 0x6b, 0x69, 0x65, 0x06, 0x23, 0x65, 0x78, 0x70,
         /*   96: */ 0x65, 0x63, 0x74, 0x07, 0x33, 0x72, 0x65, 0x66,
         /*  104: */ 0x65, 0x72, 0x65, 0x72, 0x07, 0x24, 0x65, 0x78,
         /*  112: */ 0x70, 0x69, 0x72, 0x65, 0x73, 0x07, 0x02, 0x3a,
         /*  120: */ 0x6d, 0x65, 0x74, 0x68, 0x6f, 0x64, 0x07, 0x06,
         /*  128: */ 0x3a, 0x73, 0x63, 0x68, 0x65, 0x6d, 0x65, 0x07,
         /*  136: */ 0x08, 0x3a, 0x73, 0x74, 0x61, 0x74, 0x75, 0x73,
         /*  144: */ 0x07, 0x34, 0x72, 0x65, 0x66, 0x72, 0x65, 0x73,
         /*  152: */ 0x68, 0x08, 0x2e, 0x6c, 0x6f, 0x63, 0x61, 0x74,
         /*  160: */ 0x69, 0x6f, 0x6e, 0x08, 0x27, 0x69, 0x66, 0x2d,
         /*  168: */ 0x6d, 0x61, 0x74, 0x63, 0x68, 0x08, 0x2a, 0x69,
         /*  176: */ 0x66, 0x2d, 0x72, 0x61, 0x6e, 0x67, 0x65, 0x0a,
         /*  184: */ 0x3a, 0x75, 0x73, 0x65, 0x72, 0x2d, 0x61, 0x67,
         /*  192: */ 0x65, 0x6e, 0x74, 0x0a, 0x37, 0x73, 0x65, 0x74,
         /*  200: */ 0x2d, 0x63, 0x6f, 0x6f, 0x6b, 0x69, 0x65, 0x0a,
         /*  208: */ 0x01, 0x3a, 0x61, 0x75, 0x74, 0x68, 0x6f, 0x72,
         /*  216: */ 0x69, 0x74, 0x79, 0x0b, 0x35, 0x72, 0x65, 0x74,
         /*  224: */ 0x72, 0x79, 0x2d, 0x61, 0x66, 0x74, 0x65, 0x72,
         /*  232: */ 0x0c, 0x1f, 0x63, 0x6f, 0x6e, 0x74, 0x65, 0x6e,
         /*  240: */ 0x74, 0x2d, 0x74, 0x79, 0x70, 0x65, 0x0c, 0x2f,
         /*  248: */ 0x6d, 0x61, 0x78, 0x2d, 0x66, 0x6f, 0x72, 0x77,
         /*  256: */ 0x61, 0x72, 0x64, 0x73, 0x0d, 0x18, 0x63, 0x61,
         /*  264: */ 0x63, 0x68, 0x65, 0x2d, 0x63, 0x6f, 0x6e, 0x74,
         /*  272: */ 0x72, 0x6f, 0x6c, 0x0d, 0x2c, 0x6c, 0x61, 0x73,
         /*  280: */ 0x74, 0x2d, 0x6d, 0x6f, 0x64, 0x69, 0x66, 0x69,
         /*  288: */ 0x65, 0x64, 0x0d, 0x12, 0x61, 0x63, 0x63, 0x65,
         /*  296: */ 0x70, 0x74, 0x2d, 0x72, 0x61, 0x6e, 0x67, 0x65,
         /*  304: */ 0x73, 0x0d, 0x29, 0x69, 0x66, 0x2d, 0x6e, 0x6f,
         /*  312: */ 0x6e, 0x65, 0x2d, 0x6d, 0x61, 0x74, 0x63, 0x68,
         /*  320: */ 0x0d, 0x17, 0x61, 0x75, 0x74, 0x68, 0x6f, 0x72,
         /*  328: */ 0x69, 0x7a, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x0d,
         /*  336: */ 0x1e, 0x63, 0x6f, 0x6e, 0x74, 0x65, 0x6e, 0x74,
         /*  344: */ 0x2d, 0x72, 0x61, 0x6e, 0x67, 0x65, 0x0e, 0x1c,
         /*  352: */ 0x63, 0x6f, 0x6e, 0x74, 0x65, 0x6e, 0x74, 0x2d,
         /*  360: */ 0x6c, 0x65, 0x6e, 0x67, 0x74, 0x68, 0x0e, 0x0f,
         /*  368: */ 0x61, 0x63, 0x63, 0x65, 0x70, 0x74, 0x2d, 0x63,
         /*  376: */ 0x68, 0x61, 0x72, 0x73, 0x65, 0x74, 0x0f, 0x10,
         /*  384: */ 0x61, 0x63, 0x63, 0x65, 0x70, 0x74, 0x2d, 0x65,
         /*  392: */ 0x6e, 0x63, 0x6f, 0x64, 0x69, 0x6e, 0x67, 0x0f,
         /*  400: */ 0x11, 0x61, 0x63, 0x63, 0x65, 0x70, 0x74, 0x2d,
         /*  408: */ 0x6c, 0x61, 0x6e, 0x67, 0x75, 0x61, 0x67, 0x65,
         /*  416: */ 0x10, 0x1a, 0x63, 0x6f, 0x6e, 0x74, 0x65, 0x6e,
         /*  424: */ 0x74, 0x2d, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x69,
         /*  432: */ 0x6e, 0x67, 0x10, 0x1b, 0x63, 0x6f, 0x6e, 0x74,
         /*  440: */ 0x65, 0x6e, 0x74, 0x2d, 0x6c, 0x61, 0x6e, 0x67,
         /*  448: */ 0x75, 0x61, 0x67, 0x65, 0x10, 0x1d, 0x63, 0x6f,
         /*  456: */ 0x6e, 0x74, 0x65, 0x6e, 0x74, 0x2d, 0x6c, 0x6f,
         /*  464: */ 0x63, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x10, 0x3d,
         /*  472: */ 0x77, 0x77, 0x77, 0x2d, 0x61, 0x75, 0x74, 0x68,
         /*  480: */ 0x65, 0x6e, 0x74, 0x69, 0x63, 0x61, 0x74, 0x65,
         /*  488: */ 0x11, 0x39, 0x74, 0x72, 0x61, 0x6e, 0x73, 0x66,
         /*  496: */ 0x65, 0x72, 0x2d, 0x65, 0x6e, 0x63, 0x6f, 0x64,
         /*  504: */ 0x69, 0x6e, 0x67, 0x11, 0x28, 0x69, 0x66, 0x2d,
         /*  512: */ 0x6d, 0x6f, 0x64, 0x69, 0x66, 0x69, 0x65, 0x64,
         /*  520: */ 0x2d, 0x73, 0x69, 0x6e, 0x63, 0x65, 0x12, 0x30,
         /*  528: */ 0x70, 0x72, 0x6f, 0x78, 0x79, 0x2d, 0x61, 0x75,
         /*  536: */ 0x74, 0x68, 0x65, 0x6e, 0x74, 0x69, 0x63, 0x61,
         /*  544: */ 0x74, 0x65, 0x13, 0x19, 0x63, 0x6f, 0x6e, 0x74,
         /*  552: */ 0x65, 0x6e, 0x74, 0x2d, 0x64, 0x69, 0x73, 0x70,
         /*  560: */ 0x6f, 0x73, 0x69, 0x74, 0x69, 0x6f, 0x6e, 0x13,
         /*  568: */ 0x2b, 0x69, 0x66, 0x2d, 0x75, 0x6e, 0x6d, 0x6f,
         /*  576: */ 0x64, 0x69, 0x66, 0x69, 0x65, 0x64, 0x2d, 0x73,
         /*  584: */ 0x69, 0x6e, 0x63, 0x65, 0x13, 0x31, 0x70, 0x72,
         /*  592: */ 0x6f, 0x78, 0x79, 0x2d, 0x61, 0x75, 0x74, 0x68,
         /*  600: */ 0x6f, 0x72, 0x69, 0x7a, 0x61, 0x74, 0x69, 0x6f,
         /*  608: */ 0x6e, 0x19, 0x38, 0x73, 0x74, 0x72, 0x69, 0x63,
         /*  616: */ 0x74, 0x2d, 0x74, 0x72, 0x61, 0x6e, 0x73, 0x70,
         /*  624: */ 0x6f, 0x72, 0x74, 0x2d, 0x73, 0x65, 0x63, 0x75,
         /*  632: */ 0x72, 0x69, 0x74, 0x79, 0x1b, 0x14, 0x61, 0x63,
         /*  640: */ 0x63, 0x65, 0x73, 0x73, 0x2d, 0x63, 0x6f, 0x6e,
         /*  648: */ 0x74, 0x72, 0x6f, 0x6c, 0x2d, 0x61, 0x6c, 0x6c,
         /*  656: */ 0x6f, 0x77, 0x2d, 0x6f, 0x72, 0x69, 0x67, 0x69,
         /*  664: */ 0x6e, 0x00,
};

/* This points to the first position in table hpack_enc_stream[] of a header
 * of the same length.
 */
const signed short hpack_pos_len[32] = {
         /*    0: */   -1,   -1,   -1,    0,   10,   46,   67,   99,
         /*    8: */  153,   -1,  183,  219,  232,  260,  350,  382,
         /*   16: */  416,  488,  526,  546,   -1,   -1,   -1,   -1,
         /*   24: */   -1,  609,   -1,  636,   -1,   -1,   -1,   -1,
};

/* Tries to encode header whose name is <n> and value <v> into the chunk <out>.
 * Returns non-zero on success, 0 on failure (buffer full).
 */
int hpack_encode_header(struct buffer *out, const struct ist n,
			const struct ist v)
{
	int len = out->data;
	int size = out->size;
	int pos;

	if (len >= size)
		return 0;

	/* look for the header field <n> in the static table */
	if (n.len >= sizeof(hpack_pos_len) / sizeof(hpack_pos_len[0]))
		goto make_literal;

	pos = hpack_pos_len[n.len];
	if (pos >= 0) {
		/* At least one header field of this length exist */
		do {
			char idx;

			pos++;
			idx = hpack_enc_stream[pos++];
			pos += n.len;
			if (isteq(ist2(&hpack_enc_stream[pos - n.len], n.len), n)) {
				/* emit literal with indexing (7541#6.2.1) :
				 * [ 0 | 1 | Index (6+) ]
				 */
				out->area[len++] = idx | 0x40;
				goto emit_value;
			}
		} while ((unsigned char)hpack_enc_stream[pos] == n.len);
	}

 make_literal:
	if (likely(n.len < 127 && len + 2 + n.len <= size)) {
		out->area[len++] = 0x00;      /* literal without indexing -- new name */
		out->area[len++] = n.len;     /* single-byte length encoding */
		ist2bin(out->area + len, n);
		len += n.len;
	}
	else if (hpack_len_to_bytes(n.len) &&
		 len + 1 + hpack_len_to_bytes(n.len) + n.len <= size) {
		out->area[len++] = 0x00;      /* literal without indexing -- new name */
		len = hpack_encode_len(out->area, len, n.len);
		ist2bin(out->area + len, n);
		len += n.len;
	}
	else {
		/* header field name too large for the buffer */
		return 0;
	}

 emit_value:
	/* copy literal header field value */
	if (!hpack_len_to_bytes(v.len) ||
	    len + hpack_len_to_bytes(v.len) + v.len > size) {
		/* header value too large for the buffer */
		return 0;
	}

	len = hpack_encode_len(out->area, len, v.len);
	memcpy(out->area + len, v.ptr, v.len);
	len += v.len;

	out->data = len;
	return 1;
}
