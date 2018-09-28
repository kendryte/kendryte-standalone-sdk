# Copyright 2018 Canaan Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#include "encoding.h"

# define REGBYTES 8
# define STKSHIFT 17


.section .text.start, "ax", @progbits
.globl _start
_start:
  j 1f
  .word 0xdeadbeef
1:
  csrw mideleg, 0
  csrw medeleg, 0
  csrw mie, 0
  csrw mip, 0
  la t0, trap_entry
  csrw mtvec, t0

  li  x1, 0
  li  x2, 0
  li  x3, 0
  li  x4, 0
  li  x5, 0
  li  x6, 0
  li  x7, 0
  li  x8, 0
  li  x9, 0
  li  x10,0
  li  x11,0
  li  x12,0
  li  x13,0
  li  x14,0
  li  x15,0
  li  x16,0
  li  x17,0
  li  x18,0
  li  x19,0
  li  x20,0
  li  x21,0
  li  x22,0
  li  x23,0
  li  x24,0
  li  x25,0
  li  x26,0
  li  x27,0
  li  x28,0
  li  x29,0
  li  x30,0
  li  x31,0

  li t0, MSTATUS_FS
  csrs mstatus, t0

  fssr    x0
  fmv.d.x f0, x0
  fmv.d.x f1, x0
  fmv.d.x f2, x0
  fmv.d.x f3, x0
  fmv.d.x f4, x0
  fmv.d.x f5, x0
  fmv.d.x f6, x0
  fmv.d.x f7, x0
  fmv.d.x f8, x0
  fmv.d.x f9, x0
  fmv.d.x f10,x0
  fmv.d.x f11,x0
  fmv.d.x f12,x0
  fmv.d.x f13,x0
  fmv.d.x f14,x0
  fmv.d.x f15,x0
  fmv.d.x f16,x0
  fmv.d.x f17,x0
  fmv.d.x f18,x0
  fmv.d.x f19,x0
  fmv.d.x f20,x0
  fmv.d.x f21,x0
  fmv.d.x f22,x0
  fmv.d.x f23,x0
  fmv.d.x f24,x0
  fmv.d.x f25,x0
  fmv.d.x f26,x0
  fmv.d.x f27,x0
  fmv.d.x f28,x0
  fmv.d.x f29,x0
  fmv.d.x f30,x0
  fmv.d.x f31,x0

  .option push
  .option norelax
    la gp, __global_pointer$
  .option pop
  la  tp, _end + 63
  and tp, tp, -64
  csrr a0, mhartid

  sll a2, a0, STKSHIFT
  add tp, tp, a2
  add sp, a0, 1
  sll sp, sp, STKSHIFT
  add sp, sp, tp

  j _init_bsp

  .globl trap_entry
  .type trap_entry, @function
  .align 2
trap_entry:
  addi sp, sp, -REGBYTES
  sd t0, 0x0(sp)
  csrr t0, mcause
  bgez t0, .handle_syscall

  ld t0, 0x0(sp)
  addi sp, sp, REGBYTES
  addi sp, sp, -36*REGBYTES

  sd ra,  0 * REGBYTES(sp)
  sd t0,  1 * REGBYTES(sp)
  sd t1,  2 * REGBYTES(sp)
  sd t2,  3 * REGBYTES(sp)
  sd a0,  4 * REGBYTES(sp)
  sd a1,  5 * REGBYTES(sp)
  sd a2,  6 * REGBYTES(sp)
  sd a3,  7 * REGBYTES(sp)
  sd a4,  8 * REGBYTES(sp)
  sd a5,  9 * REGBYTES(sp)
  sd a6, 10 * REGBYTES(sp)
  sd a7, 11 * REGBYTES(sp)
  sd t3, 12 * REGBYTES(sp)
  sd t4, 13 * REGBYTES(sp)
  sd t5, 14 * REGBYTES(sp)
  sd t6, 15 * REGBYTES(sp)

  fsd ft0,  ( 0 + 16) * REGBYTES(sp)
  fsd ft1,  ( 1 + 16) * REGBYTES(sp)
  fsd ft2,  ( 2 + 16) * REGBYTES(sp)
  fsd ft3,  ( 3 + 16) * REGBYTES(sp)
  fsd ft4,  ( 4 + 16) * REGBYTES(sp)
  fsd ft5,  ( 5 + 16) * REGBYTES(sp)
  fsd ft6,  ( 6 + 16) * REGBYTES(sp)
  fsd ft7,  ( 7 + 16) * REGBYTES(sp)
  fsd fa0,  ( 8 + 16) * REGBYTES(sp)
  fsd fa1,  ( 9 + 16) * REGBYTES(sp)
  fsd fa2,  (10 + 16) * REGBYTES(sp)
  fsd fa3,  (11 + 16) * REGBYTES(sp)
  fsd fa4,  (12 + 16) * REGBYTES(sp)
  fsd fa5,  (13 + 16) * REGBYTES(sp)
  fsd fa6,  (14 + 16) * REGBYTES(sp)
  fsd fa7,  (15 + 16) * REGBYTES(sp)
  fsd ft8,  (16 + 16) * REGBYTES(sp)
  fsd ft9,  (17 + 16) * REGBYTES(sp)
  fsd ft10, (18 + 16) * REGBYTES(sp)
  fsd ft11, (19 + 16) * REGBYTES(sp)

  csrr a0, mcause
  csrr a1, mepc

.handle_irq:
  jal handle_irq
  csrw mepc, a0

  ld ra,  0 * REGBYTES(sp)
  ld t0,  1 * REGBYTES(sp)
  ld t1,  2 * REGBYTES(sp)
  ld t2,  3 * REGBYTES(sp)
  ld a0,  4 * REGBYTES(sp)
  ld a1,  5 * REGBYTES(sp)
  ld a2,  6 * REGBYTES(sp)
  ld a3,  7 * REGBYTES(sp)
  ld a4,  8 * REGBYTES(sp)
  ld a5,  9 * REGBYTES(sp)
  ld a6, 10 * REGBYTES(sp)
  ld a7, 11 * REGBYTES(sp)
  ld t3, 12 * REGBYTES(sp)
  ld t4, 13 * REGBYTES(sp)
  ld t5, 14 * REGBYTES(sp)
  ld t6, 15 * REGBYTES(sp)

  fld ft0,  ( 0 + 16) * REGBYTES(sp)
  fld ft1,  ( 1 + 16) * REGBYTES(sp)
  fld ft2,  ( 2 + 16) * REGBYTES(sp)
  fld ft3,  ( 3 + 16) * REGBYTES(sp)
  fld ft4,  ( 4 + 16) * REGBYTES(sp)
  fld ft5,  ( 5 + 16) * REGBYTES(sp)
  fld ft6,  ( 6 + 16) * REGBYTES(sp)
  fld ft7,  ( 7 + 16) * REGBYTES(sp)
  fld fa0,  ( 8 + 16) * REGBYTES(sp)
  fld fa1,  ( 9 + 16) * REGBYTES(sp)
  fld fa2,  (10 + 16) * REGBYTES(sp)
  fld fa3,  (11 + 16) * REGBYTES(sp)
  fld fa4,  (12 + 16) * REGBYTES(sp)
  fld fa5,  (13 + 16) * REGBYTES(sp)
  fld fa6,  (14 + 16) * REGBYTES(sp)
  fld fa7,  (15 + 16) * REGBYTES(sp)
  fld ft8,  (16 + 16) * REGBYTES(sp)
  fld ft9,  (17 + 16) * REGBYTES(sp)
  fld ft10, (18 + 16) * REGBYTES(sp)
  fld ft11, (19 + 16) * REGBYTES(sp)

  addi sp, sp, 36*REGBYTES
  mret

  csrr t0, mcause
  addi t0, t0, -CAUSE_USER_ECALL
  bltz t0, .handle_except
  ld t0, 0x0(sp)
  addi sp, sp, REGBYTES

.handle_syscall:
  ld t0, 0x0(sp)
  addi sp, sp, REGBYTES
  addi sp, sp, -64*REGBYTES
  sd x1, 1*REGBYTES(sp)
  sd x2, 2*REGBYTES(sp)
  sd x3, 3*REGBYTES(sp)
  sd x4, 4*REGBYTES(sp)
  sd x5, 5*REGBYTES(sp)
  sd x6, 6*REGBYTES(sp)
  sd x7, 7*REGBYTES(sp)
  sd x8, 8*REGBYTES(sp)
  sd x9, 9*REGBYTES(sp)
  sd x10, 10*REGBYTES(sp)
  sd x11, 11*REGBYTES(sp)
  sd x12, 12*REGBYTES(sp)
  sd x13, 13*REGBYTES(sp)
  sd x14, 14*REGBYTES(sp)
  sd x15, 15*REGBYTES(sp)
  sd x16, 16*REGBYTES(sp)
  sd x17, 17*REGBYTES(sp)

  csrr a0, mcause
  csrr a1, mepc
  mv a2, sp
  add a3, sp, 32*REGBYTES

  jal handle_syscall

  csrw mepc, a0
  ld x1, 1*REGBYTES(sp)
  ld x2, 2*REGBYTES(sp)
  ld x3, 3*REGBYTES(sp)
  ld x4, 4*REGBYTES(sp)
  ld x5, 5*REGBYTES(sp)
  ld x6, 6*REGBYTES(sp)
  ld x7, 7*REGBYTES(sp)
  ld x8, 8*REGBYTES(sp)
  ld x9, 9*REGBYTES(sp)
  ld x10, 10*REGBYTES(sp)
  ld x11, 11*REGBYTES(sp)
  ld x12, 12*REGBYTES(sp)
  ld x13, 13*REGBYTES(sp)
  ld x14, 14*REGBYTES(sp)
  ld x15, 15*REGBYTES(sp)
  ld x16, 16*REGBYTES(sp)
  ld x17, 17*REGBYTES(sp)

  addi sp, sp, 64*REGBYTES
  mret

.handle_except:
  ld t0, 0x0(sp)
  addi sp, sp, REGBYTES
  addi sp, sp, -64*REGBYTES
  sd x1, 1*REGBYTES(sp)
  sd x2, 2*REGBYTES(sp)
  sd x3, 3*REGBYTES(sp)
  sd x4, 4*REGBYTES(sp)
  sd x5, 5*REGBYTES(sp)
  sd x6, 6*REGBYTES(sp)
  sd x7, 7*REGBYTES(sp)
  sd x8, 8*REGBYTES(sp)
  sd x9, 9*REGBYTES(sp)
  sd x10, 10*REGBYTES(sp)
  sd x11, 11*REGBYTES(sp)
  sd x12, 12*REGBYTES(sp)
  sd x13, 13*REGBYTES(sp)
  sd x14, 14*REGBYTES(sp)
  sd x15, 15*REGBYTES(sp)
  sd x16, 16*REGBYTES(sp)
  sd x17, 17*REGBYTES(sp)
  sd x18, 18*REGBYTES(sp)
  sd x19, 19*REGBYTES(sp)
  sd x20, 20*REGBYTES(sp)
  sd x21, 21*REGBYTES(sp)
  sd x22, 22*REGBYTES(sp)
  sd x23, 23*REGBYTES(sp)
  sd x24, 24*REGBYTES(sp)
  sd x25, 25*REGBYTES(sp)
  sd x26, 26*REGBYTES(sp)
  sd x27, 27*REGBYTES(sp)
  sd x28, 28*REGBYTES(sp)
  sd x29, 29*REGBYTES(sp)
  sd x30, 30*REGBYTES(sp)
  sd x31, 31*REGBYTES(sp)

  fsd f0, ( 0  + 32)*REGBYTES(sp)
  fsd f1, ( 1  + 32)*REGBYTES(sp)
  fsd f2, ( 2  + 32)*REGBYTES(sp)
  fsd f3, ( 3  + 32)*REGBYTES(sp)
  fsd f4, ( 4  + 32)*REGBYTES(sp)
  fsd f5, ( 5  + 32)*REGBYTES(sp)
  fsd f6, ( 6  + 32)*REGBYTES(sp)
  fsd f7, ( 7  + 32)*REGBYTES(sp)
  fsd f8, ( 8  + 32)*REGBYTES(sp)
  fsd f9, ( 9  + 32)*REGBYTES(sp)
  fsd f10,( 10 + 32)*REGBYTES(sp)
  fsd f11,( 11 + 32)*REGBYTES(sp)
  fsd f12,( 12 + 32)*REGBYTES(sp)
  fsd f13,( 13 + 32)*REGBYTES(sp)
  fsd f14,( 14 + 32)*REGBYTES(sp)
  fsd f15,( 15 + 32)*REGBYTES(sp)
  fsd f16,( 16 + 32)*REGBYTES(sp)
  fsd f17,( 17 + 32)*REGBYTES(sp)
  fsd f18,( 18 + 32)*REGBYTES(sp)
  fsd f19,( 19 + 32)*REGBYTES(sp)
  fsd f20,( 20 + 32)*REGBYTES(sp)
  fsd f21,( 21 + 32)*REGBYTES(sp)
  fsd f22,( 22 + 32)*REGBYTES(sp)
  fsd f23,( 23 + 32)*REGBYTES(sp)
  fsd f24,( 24 + 32)*REGBYTES(sp)
  fsd f25,( 25 + 32)*REGBYTES(sp)
  fsd f26,( 26 + 32)*REGBYTES(sp)
  fsd f27,( 27 + 32)*REGBYTES(sp)
  fsd f28,( 28 + 32)*REGBYTES(sp)
  fsd f29,( 29 + 32)*REGBYTES(sp)
  fsd f30,( 30 + 32)*REGBYTES(sp)
  fsd f31,( 31 + 32)*REGBYTES(sp)

  csrr a0, mcause
  csrr a1, mepc
  mv a2, sp
  add a3, sp, 32*REGBYTES

  jal handle_except

  csrw mepc, a0
  ld x1, 1*REGBYTES(sp)
  ld x2, 2*REGBYTES(sp)
  ld x3, 3*REGBYTES(sp)
  ld x4, 4*REGBYTES(sp)
  ld x5, 5*REGBYTES(sp)
  ld x6, 6*REGBYTES(sp)
  ld x7, 7*REGBYTES(sp)
  ld x8, 8*REGBYTES(sp)
  ld x9, 9*REGBYTES(sp)
  ld x10, 10*REGBYTES(sp)
  ld x11, 11*REGBYTES(sp)
  ld x12, 12*REGBYTES(sp)
  ld x13, 13*REGBYTES(sp)
  ld x14, 14*REGBYTES(sp)
  ld x15, 15*REGBYTES(sp)
  ld x16, 16*REGBYTES(sp)
  ld x17, 17*REGBYTES(sp)
  ld x18, 18*REGBYTES(sp)
  ld x19, 19*REGBYTES(sp)
  ld x20, 20*REGBYTES(sp)
  ld x21, 21*REGBYTES(sp)
  ld x22, 22*REGBYTES(sp)
  ld x23, 23*REGBYTES(sp)
  ld x24, 24*REGBYTES(sp)
  ld x25, 25*REGBYTES(sp)
  ld x26, 26*REGBYTES(sp)
  ld x27, 27*REGBYTES(sp)
  ld x28, 28*REGBYTES(sp)
  ld x29, 29*REGBYTES(sp)
  ld x30, 30*REGBYTES(sp)
  ld x31, 31*REGBYTES(sp)

  fld f0, ( 0  + 32)*REGBYTES(sp)
  fld f1, ( 1  + 32)*REGBYTES(sp)
  fld f2, ( 2  + 32)*REGBYTES(sp)
  fld f3, ( 3  + 32)*REGBYTES(sp)
  fld f4, ( 4  + 32)*REGBYTES(sp)
  fld f5, ( 5  + 32)*REGBYTES(sp)
  fld f6, ( 6  + 32)*REGBYTES(sp)
  fld f7, ( 7  + 32)*REGBYTES(sp)
  fld f8, ( 8  + 32)*REGBYTES(sp)
  fld f9, ( 9  + 32)*REGBYTES(sp)
  fld f10,( 10 + 32)*REGBYTES(sp)
  fld f11,( 11 + 32)*REGBYTES(sp)
  fld f12,( 12 + 32)*REGBYTES(sp)
  fld f13,( 13 + 32)*REGBYTES(sp)
  fld f14,( 14 + 32)*REGBYTES(sp)
  fld f15,( 15 + 32)*REGBYTES(sp)
  fld f16,( 16 + 32)*REGBYTES(sp)
  fld f17,( 17 + 32)*REGBYTES(sp)
  fld f18,( 18 + 32)*REGBYTES(sp)
  fld f19,( 19 + 32)*REGBYTES(sp)
  fld f20,( 20 + 32)*REGBYTES(sp)
  fld f21,( 21 + 32)*REGBYTES(sp)
  fld f22,( 22 + 32)*REGBYTES(sp)
  fld f23,( 23 + 32)*REGBYTES(sp)
  fld f24,( 24 + 32)*REGBYTES(sp)
  fld f25,( 25 + 32)*REGBYTES(sp)
  fld f26,( 26 + 32)*REGBYTES(sp)
  fld f27,( 27 + 32)*REGBYTES(sp)
  fld f28,( 28 + 32)*REGBYTES(sp)
  fld f29,( 29 + 32)*REGBYTES(sp)
  fld f30,( 30 + 32)*REGBYTES(sp)
  fld f31,( 31 + 32)*REGBYTES(sp)

  addi sp, sp, 64*REGBYTES
  mret

.section ".tdata.begin"
.globl _tdata_begin
_tdata_begin:

.section ".tdata.end"
.globl _tdata_end
_tdata_end:

.section ".tbss.end"
.globl _tbss_end
_tbss_end:

