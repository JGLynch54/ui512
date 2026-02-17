;
;			ui512 Legal Notes
;
;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;
;			File:			ui512_legalnotes.inc
;			Author:			John G. Lynch
;			Legal:			Copyright 2026, per MIT License included
;			Date:			October 24, 2025
;
IFNDEF							ui512_legalnotes_INC
ui512_legalnotes_INC			EQU		1

;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;
;	Notes:
;		ui512 is a small project to provide basic operations for a variable type of unsigned 512 bit integer.
;
;		It is written in assembly language, using the MASM (ml64) assembler provided as an option within Visual Studio.	(currently using VS Community 2026 18.3.0)
;		It provides external signatures that allow linkage to C and C++ programs, where a shell/wrapper could encapsulate the methods as part of an object.
;		It has assembly time options directing the use of Intel processor extensions: AVX4, AVX2, SIMD, or none:
;		(Z (512), Y (256), or X (128) registers, or regular Q (64bit)).
;		If processor extensions are used, the caller must align the variables declared and passed on the appropriate byte boundary (e.g. alignas 64 for 512)
;		This module is very light-weight (less than 20K bytes) and relatively fast,	but is not intended for all processor types or all environments. 
;
;		Intended use cases:
;			1.) a "sum of primes" for primes up to 2^48. 
;			2.) elliptical curve cryptography (ECC)
;
;		It provides external signatures that allow linkage to C and C++ programs,
;		where a shell/wrapper could encapsulate the methods as part of an object.
;
;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;
;			MIT License
;
;			Copyright (c) 2024 John G. Lynch
;
;				Permission is hereby granted, free of charge, to any person obtaining a copy
;				of this software and associated documentation files (the "Software"), to deal
;				in the Software without restriction, including without limitation the rights
;				to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
;				copies of the Software, and to permit persons to whom the Software is
;				furnished to do so, subject to the following conditions:
;
;				The above copyright notice and this permission notice shall be included in all
;				copies or substantial portions of the Software.
;
;				THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
;				IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
;				FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
;				AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
;				LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
;				OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
;				SOFTWARE.
;

ENDIF			; ui512_legalnotes_INC
