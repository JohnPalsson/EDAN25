	.file	"d.cc"
	.section	".toc","aw"
	.section	".text"
	.section	.rodata
	.type	_ZStL19piecewise_construct, @object
	.size	_ZStL19piecewise_construct, 1
_ZStL19piecewise_construct:
	.zero	1
	.type	_ZStL13allocator_arg, @object
	.size	_ZStL13allocator_arg, 1
_ZStL13allocator_arg:
	.zero	1
	.type	_ZStL6ignore, @object
	.size	_ZStL6ignore, 1
_ZStL6ignore:
	.zero	1
	.type	_ZStL10defer_lock, @object
	.size	_ZStL10defer_lock, 1
_ZStL10defer_lock:
	.zero	1
	.type	_ZStL11try_to_lock, @object
	.size	_ZStL11try_to_lock, 1
_ZStL11try_to_lock:
	.zero	1
	.type	_ZStL10adopt_lock, @object
	.size	_ZStL10adopt_lock, 1
_ZStL10adopt_lock:
	.zero	1
	.section	.text._ZNSt13__atomic_baseIyEC2Ev,"axG",@progbits,_ZNSt13__atomic_baseIyEC2Ev,comdat
	.align 2
	.weak	_ZNSt13__atomic_baseIyEC2Ev
	.section	".opd","aw"
	.align 3
_ZNSt13__atomic_baseIyEC2Ev:
	.quad	.L._ZNSt13__atomic_baseIyEC2Ev,.TOC.@tocbase
	.previous
	.type	_ZNSt13__atomic_baseIyEC2Ev, @function
.L._ZNSt13__atomic_baseIyEC2Ev:
.LFB2344:
	.cfi_startproc
	std 31,-8(1)
	stdu 1,-64(1)
	.cfi_def_cfa_offset 64
	.cfi_offset 31, -8
	mr 31,1
	.cfi_def_cfa_register 31
	std 3,112(31)
	addi 1,31,64
	.cfi_def_cfa 1, 0
	ld 31,-8(1)
	blr
	.long 0
	.byte 0,9,0,0,128,1,0,1
	.cfi_endproc
.LFE2344:
	.size	_ZNSt13__atomic_baseIyEC2Ev,.-.L._ZNSt13__atomic_baseIyEC2Ev
	.section	.text._ZNSt6atomicIyEC2Ev,"axG",@progbits,_ZNSt6atomicIyEC5Ev,comdat
	.align 2
	.weak	_ZNSt6atomicIyEC2Ev
	.section	".opd","aw"
	.align 3
_ZNSt6atomicIyEC2Ev:
	.quad	.L._ZNSt6atomicIyEC2Ev,.TOC.@tocbase
	.previous
	.type	_ZNSt6atomicIyEC2Ev, @function
.L._ZNSt6atomicIyEC2Ev:
.LFB2346:
	.cfi_startproc
	mflr 0
	std 0,16(1)
	std 31,-8(1)
	stdu 1,-128(1)
	.cfi_def_cfa_offset 128
	.cfi_offset 65, 16
	.cfi_offset 31, -8
	mr 31,1
	.cfi_def_cfa_register 31
	std 3,176(31)
	ld 9,176(31)
	mr 3,9
	bl _ZNSt13__atomic_baseIyEC2Ev
	nop
	addi 1,31,128
	.cfi_def_cfa 1, 0
	ld 0,16(1)
	mtlr 0
	ld 31,-8(1)
	blr
	.long 0
	.byte 0,9,0,1,128,1,0,1
	.cfi_endproc
.LFE2346:
	.size	_ZNSt6atomicIyEC2Ev,.-.L._ZNSt6atomicIyEC2Ev
	.weak	_ZNSt6atomicIyEC1Ev
	.set	_ZNSt6atomicIyEC1Ev,_ZNSt6atomicIyEC2Ev
	.lcomm	_ZL3sum,8,8
	.type	_ZL3sum, @object
	.section	".text"
	.align 2
	.section	".opd","aw"
	.align 3
_ZL7consumev:
	.quad	.L._ZL7consumev,.TOC.@tocbase
	.previous
	.type	_ZL7consumev, @function
.L._ZL7consumev:
.LFB2348:
	.cfi_startproc
	mflr 0
	std 0,16(1)
	std 31,-8(1)
	stdu 1,-144(1)
	.cfi_def_cfa_offset 144
	.cfi_offset 65, 16
	.cfi_offset 31, -8
	mr 31,1
	.cfi_def_cfa_register 31
.L4:
	li 9,42
	std 9,112(31)
	addis 3,2,_ZL3sum@toc@ha
	addi 3,3,_ZL3sum@toc@l
	ld 4,112(31)
	bl _ZNSt13__atomic_baseIyEpLEy
	nop
	b .L4
	.long 0
	.byte 0,9,0,1,128,1,0,1
	.cfi_endproc
.LFE2348:
	.size	_ZL7consumev,.-.L._ZL7consumev
	.section	.text._ZNSt13__atomic_baseIyEpLEy,"axG",@progbits,_ZNSt13__atomic_baseIyEpLEy,comdat
	.align 2
	.weak	_ZNSt13__atomic_baseIyEpLEy
	.section	".opd","aw"
	.align 3
_ZNSt13__atomic_baseIyEpLEy:
	.quad	.L._ZNSt13__atomic_baseIyEpLEy,.TOC.@tocbase
	.previous
	.type	_ZNSt13__atomic_baseIyEpLEy, @function
.L._ZNSt13__atomic_baseIyEpLEy:
.LFB2439:
	.cfi_startproc
	std 31,-8(1)
	stdu 1,-64(1)
	.cfi_def_cfa_offset 64
	.cfi_offset 31, -8
	mr 31,1
	.cfi_def_cfa_register 31
	std 3,112(31)
	std 4,120(31)
	ld 10,112(31)
	ld 8,120(31)
	sync
.L6:
	ldarx 9,0,10
	add 9,9,8
	stdcx. 9,0,10
	mcrf 7,0
	bne 7,.L6
	isync
	mr 3,9
	addi 1,31,64
	.cfi_def_cfa 1, 0
	ld 31,-8(1)
	blr
	.long 0
	.byte 0,9,0,0,128,1,0,1
	.cfi_endproc
.LFE2439:
	.size	_ZNSt13__atomic_baseIyEpLEy,.-.L._ZNSt13__atomic_baseIyEpLEy
	.section	".text"
	.align 2
	.section	".opd","aw"
	.align 3
_Z41__static_initialization_and_destruction_0ii:
	.quad	.L._Z41__static_initialization_and_destruction_0ii,.TOC.@tocbase
	.previous
	.type	_Z41__static_initialization_and_destruction_0ii, @function
.L._Z41__static_initialization_and_destruction_0ii:
.LFB2561:
	.cfi_startproc
	mflr 0
	std 0,16(1)
	std 31,-8(1)
	stdu 1,-128(1)
	.cfi_def_cfa_offset 128
	.cfi_offset 65, 16
	.cfi_offset 31, -8
	mr 31,1
	.cfi_def_cfa_register 31
	mr 10,3
	mr 9,4
	stw 10,176(31)
	stw 9,184(31)
	lwz 9,176(31)
	cmpwi 7,9,1
	bne 7,.L8
	lwz 10,184(31)
	li 9,0
	ori 9,9,65535
	cmpw 7,10,9
	bne 7,.L8
	addis 3,2,_ZL3sum@toc@ha
	addi 3,3,_ZL3sum@toc@l
	bl _ZNSt6atomicIyEC1Ev
	nop
.L8:
	addi 1,31,128
	.cfi_def_cfa 1, 0
	ld 0,16(1)
	mtlr 0
	ld 31,-8(1)
	blr
	.long 0
	.byte 0,9,0,1,128,1,0,1
	.cfi_endproc
.LFE2561:
	.size	_Z41__static_initialization_and_destruction_0ii,.-.L._Z41__static_initialization_and_destruction_0ii
	.section	.rodata
	.align 2
	.type	_ZN9__gnu_cxxL21__default_lock_policyE, @object
	.size	_ZN9__gnu_cxxL21__default_lock_policyE, 4
_ZN9__gnu_cxxL21__default_lock_policyE:
	.long	2
	.section	".text"
	.align 2
	.section	".opd","aw"
	.align 3
_GLOBAL__sub_I_d.cc:
	.quad	.L._GLOBAL__sub_I_d.cc,.TOC.@tocbase
	.previous
	.type	_GLOBAL__sub_I_d.cc, @function
.L._GLOBAL__sub_I_d.cc:
.LFB2562:
	.cfi_startproc
	mflr 0
	std 0,16(1)
	std 31,-8(1)
	stdu 1,-128(1)
	.cfi_def_cfa_offset 128
	.cfi_offset 65, 16
	.cfi_offset 31, -8
	mr 31,1
	.cfi_def_cfa_register 31
	li 3,1
	li 4,-1
	rldicl 4,4,0,48
	bl _Z41__static_initialization_and_destruction_0ii
	addi 1,31,128
	.cfi_def_cfa 1, 0
	ld 0,16(1)
	mtlr 0
	ld 31,-8(1)
	blr
	.long 0
	.byte 0,9,0,1,128,1,0,1
	.cfi_endproc
.LFE2562:
	.size	_GLOBAL__sub_I_d.cc,.-.L._GLOBAL__sub_I_d.cc
	.section	.init_array,"aw"
	.align 3
	.quad	_GLOBAL__sub_I_d.cc
	.ident	"GCC: (GNU) 4.8.2 20131212 (Red Hat 4.8.2-7)"
	.section	.note.GNU-stack,"",@progbits
