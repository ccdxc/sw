PUBLIC add32_with_carry
PUBLIC csum_fold
PUBLIC csum_tcpudp_nofold
PUBLIC csum_ipv6_magic

.code _text

add32_with_carry PROC PUBLIC
    mov eax, ecx
    add eax, edx
    adc eax, 0
    ret
add32_with_carry ENDP

csum_fold PROC PUBLIC
    mov eax, ecx
    sal eax, 16
    mov edx, eax
    mov eax, ecx
    mov ax, 0
    add eax, edx
    adc eax, 0ffffh
    not eax
    shr eax, 16
    ret
csum_fold ENDP


; stack:
;    [rsp +  8] = storage for rcx (not filled)
;    [rsp + 16] = storage for rdx (not filled)
;    [rsp + 24] = storage for r8 (not filled)
;    [rsp + 32] = storage for r9 (not filled)
;    [rsp + 40] = arg 5 (filled)

; args: 
;    u32 saddr
;    u32 daddr
;    u16 len
;    u16 proto
;    u32 sum

csum_tcpudp_nofold PROC PUBLIC
    ; save first two args
    mov [rsp + 8], rcx
    mov [rsp + 16], rdx

    movzx edx, r8w
    movzx eax, r9w
    add eax, edx
    sal eax, 8
    mov edx, eax
    mov eax, [rsp + 40]
    add eax, [rsp + 16]
    adc eax, [rsp + 8]
    adc eax, edx
    adc eax, 0    
    ret
csum_tcpudp_nofold ENDP

; stack:
;    [rsp +  8] = storage for rcx (not filled)
;    [rsp + 16] = storage for rdx (not filled)
;    [rsp + 24] = storage for r8 (not filled)
;    [rsp + 32] = storage for r9 (not filled)
;    [rsp + 40] = arg 5 (filled)

; args: 
;    ptr saddr
;    ptr daddr
;    u32 len
;    u16 proto
;    u32 sum

csum_ipv6_magic PROC PUBLIC
    ; save first two args
    mov   [rsp + 8], rcx
    mov   [rsp + 16], rdx

    bswap r8d
    mov   edx, r8d ; zero extends to high 32-bits
    movzx eax, r9w ; zero extends high 32-bits
    rol   ax, 8
    add   rdx, rax
    mov   eax, [rsp + 40] ; zero extends high 32-bits
    add   rax, rdx
    mov   rdx, [rsp + 8] ; saddr *
    mov   rcx, [rsp + 16] ; daddr *

    add   rax, [rdx]
    adc   rax, [rdx + 8]
    adc   rax, [rcx]
    adc   rax, [rcx + 8]
    adc   rax, 0

    mov   ecx, eax ; first param to add32_with_carry
    shr   rax, 32    
    mov   edx, eax ; second param to add32_with_carry
    call  add32_with_carry
    mov   ecx, eax
    call  csum_fold
    ret
csum_ipv6_magic ENDP

END