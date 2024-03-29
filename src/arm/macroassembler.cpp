/*
	Copyright (c) 1994-2006 Sun Microsystems Inc.
	All Rights Reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions
	are met:

	- Redistributions of source code must retain the above copyright notice,
	this list of conditions and the following disclaimer.

	- Redistribution in binary form must reproduce the above copyright
	notice, this list of conditions and the following disclaimer in the
	documentation and/or other materials provided with the
	distribution.

	- Neither the name of Sun Microsystems or the names of contributors may
	be used to endorse or promote products derived from this software without
	specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
	"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
	FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
	COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
	INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
	SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
	HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
	STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
	OF THE POSSIBILITY OF SUCH DAMAGE.

	The original source code covered by the above license above has been
	modified significantly by Google Inc.
	Copyright 2012 the V8 project authors. All rights reserved.

	The original source code covered by the above license above has been
	modified by elemental in order to fit in EmuMaster
 */

#include "macroassembler.h"

namespace Arm {

MacroAssembler::MacroAssembler(void *buffer, int size) :
	Assembler(buffer, size)
{
}

void MacroAssembler::lowLevelDebug(const char *s,
								   Register ra,
								   Register rb,
								   Register rc)
{
	Q_ASSERT(ra.code() < 13 && rb.code() < 13 && rc.code() < 13);
	int preserved = 0x1fff | lr.bit();
	stm(db_w, sp, preserved);
	add(fp, sp, Operand(12*4));
	mrs(r4, CPSR);

	Label omitString;
	b(&omitString);

	int sPtr = intptr_t(buffer_ + pcOffset());
	do {
		db(*s);
	} while (*(s++));
	while (pcOffset() & 3)
		db('\0');

	bind(&omitString);
	ldr(r3, MemOperand(sp, rc.code()*4));
	ldr(r2, MemOperand(sp, rb.code()*4));
	ldr(r1, MemOperand(sp, ra.code()*4));
	mov(r0, Operand(sPtr));

	void (*qDebugPtr)(const char *,...) = &qDebug;
	mov(ip, Operand(intptr_t(qDebugPtr)));
	blx(ip);

	msr(CPSR_f, Operand(r4));
	ldm(ia_w, sp, preserved);
}

} // namespace Arm
