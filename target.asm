.386
.model flat,stdcall
option casemap:none
Include e:\masm32\include\windows.inc
Include e:\masm32\include\kernel32.inc
Include e:\masm32\include\msvcrt.inc
Includelib e:\masm32\lib\msvcrt.lib
Includelib e:\masm32\lib\kernel32.lib
Include e:\masm32\macros\macros.asm

.data
	_a	equ	1
	_b	dword	?

.CODE
_square	PROC
	push	ebx;saveState
	push	ebp
	mov	ebp,	esp;saveState finish
	mov	eax,	0;LocalVar
	push	eax
	mov	eax,	dword  ptr [ebp+(16)]
	mov	ebx,	eax
	mov	eax,	dword  ptr [ebp+(20)]
	add	eax,	ebx
	mov	dword  ptr [ebp+(-12)],	eax
	mov   ebx,ebp
	add   ebx,-12
	cmp   esp ,ebx
	jle  espp0
	mov esp, ebx
espp0:
	mov	eax,	dword  ptr [ebp+(12)]
	mov	ebx,	eax
	mov	eax,	dword  ptr [ebp+(-12)]
	cdq
	imul	ebx
	mov	dword  ptr [ebp+(-20)],	eax
	mov   ebx,ebp
	add   ebx,-20
	cmp   esp ,ebx
	jle  espp1
	mov esp, ebx
espp1:
	mov	eax,	2
	mov	ebx,	eax
	mov	eax,	dword  ptr [ebp+(-20)]
	cdq
	idiv	ebx
	mov	dword  ptr [ebp+(-28)],	eax
	mov   ebx,ebp
	add   ebx,-28
	cmp   esp ,ebx
	jle  espp2
	mov esp, ebx
espp2:
	mov	eax,	dword  ptr [ebp+(-28)]
	mov	dword  ptr [ebp+(-4)],	eax
	mov	eax,	dword  ptr [ebp+(-4)]
	mov	esp,	ebp
	pop	ebp
	pop	ebx
	ret
	mov	esp,	ebp
	pop	ebp
	pop	ebx
	ret
_square	endp
START:
	push	ebp
	mov	ebp,	esp
	mov	eax,	0;LocalVar
	push	eax
	mov	eax,	0;LocalVar
	push	eax
	mov	eax,	0;LocalVar
	push	eax
	invoke crt_scanf,SADD("%d"),addr dword ptr[ebp+(-4)]
	invoke crt_scanf,SADD("%d"),addr dword ptr[ebp+(-8)]
	invoke crt_scanf,SADD("%d"),addr dword ptr[ebp+(-8)]
	invoke crt_scanf,SADD("%d"),addr dword ptr[ebp+(-12)]
	mov	eax,	dword  ptr [ebp+(-4)]
	push	eax;pass value to function
	mov	eax,	dword  ptr [ebp+(-8)]
	push	eax;pass value to function
	mov	eax,	dword  ptr [ebp+(-12)]
	push	eax;pass value to function
	call	_square
	add	esp,	12
	mov	dword  ptr [ebp+(-20)],	eax
	mov   ebx,ebp
	add   ebx,-20
	cmp   esp ,ebx
	jle  espp3
	mov esp, ebx
espp3:
	push eax;print begin
	mov	eax,	dword  ptr [ebp+(-20)]
	invoke crt_printf,SADD("%d "),eax
invoke crt_printf,SADD("%c"),10
	pop eax;print over
   invoke ExitProcess, 0; exit with code 0
END START
