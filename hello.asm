includelib ucrt.lib
includelib legacy_stdio_definitions.lib
includelib msvcrt.lib

option casemap:none

.data
; Format: String to print, 10 for newline, 0 for null terminator
; c++: std::format("{}{}{}", String, '\n' /* EndLn */, '\0' /* NULL */);
fmtStr byte 'Гало дуниа😋🙀! This is a simple program in asm using ML64!', 10, 0

.code
externdef printf:proc
externdef _CRT_INIT:proc
externdef exit:proc

main proc
    call _CRT_INIT  ; Initialize C runtime (important for printf)

    ; Setup stack frame (typical for Windows x64 calling convention)
    push rbp
    mov rbp, rsp
    sub rsp, 32     ; Allocate shadow space for function calls

    ; Load the address of the format string into RCX (first argument for printf)
    lea rcx, fmtStr 

    ; Call printf
    call printf

    ; Exit the program
    xor ecx, ecx    ; Set return code to 0
    call exit       ; exit program
main endp

end