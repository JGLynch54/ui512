;
;			ui512 Legal Notes
;
;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;
;			File:			ui512_legalnotes.inc
;			Author:			John G. Lynch
;			Legal:			Copyright 2025, per MIT License included
;			Date:			Octoberr 24, 2025
;
IFNDEF							ui512_legalnotes_INC
ui512_legalnotes_INC			EQU		1

;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;
;			Notes:
;				ui512 is a small project to provide basic operations for a variable type of unsigned 512 bit integer.
;
;				ui512a provides basic operations: zero, copy, compare, add, subtract.
;				ui512b provides basic bit-oriented operations: shift left, shift right, and, or, not, least significant bit and most significant bit.
;               ui512md provides multiply and divide.
;
;				It is written in assembly language, using the MASM (ml64) assembler provided as an option within Visual Studio.
;				(currently using VS Community 2022 17.14.10)
;
;				It provides external signatures that allow linkage to C and C++ programs,
;				where a shell/wrapper could encapsulate the methods as part of an object.
;
;				Note: X64 ZMM regs are "little-endian". The 8 QWORD C array declarion is "big-endian".
;				In memory the most significant word is first: var[0], the least last var[7]. Normally not an issue,
;				but when that var is laoded in a Z-reg, the least significant is first, and the most last, which is
;				the way z-regs are loaded, but convention has the z-reg first word commonly termed the most significant, 
;				which is not the way our c arrays are defined. Not an issue programmatically, but often a qword shift left, or right
;				or a carry or borrow to the next most significant word is not the same when the data is in a z-reg.
;
;				It has assembly time options directing the use of Intel processor extensions: AVX4, AVX2, SIMD, or none:
;				(Z (512), Y (256), or X (128) registers, or regular Q (64bit)).
;
;				Note: The file "compile_time_options.inc" contains the options that can be configured for extension usage, and other options.
;
;				If processor extensions are used, the caller must align the variables declared and passed
;				on the appropriate byte boundary (e.g. alignas 64 for 512)
;
;				The modules (in total) are very light-weight (less than 10K bytes) and relatively fast,
;				but is not intended for all processor types or all environments. 
;
;				Intended use cases:
;					1.) a "sum of primes" for primes up to 2^48. 
;					2.) elliptical curve cryptography (ECC)
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