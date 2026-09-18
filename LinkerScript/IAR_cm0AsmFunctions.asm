  SECTION .data:DATA:ROOT(8)
  thumb 
    
  public SVC_Handler
  public GET_CONTROL
  public SET_CONTROL
  public BACK_TO_START

; Override SCV handler
        ALIGNRAM 4
SVC_Handler:
        PUSH {LR}
        MOVS r0, #4
        MOV  r1, lr
        TST  r0, r1
        BEQ  L_MSP
        MRS  r0, PSP
L_MSP:
        MRS  r0, MSP        
        POP  {PC}       

        ALIGNRAM 4
GET_CONTROL:
        PUSH {LR}
        MRS  r0, CONTROL ; Read CONTROL register into R0
        POP  {PC}       

        ALIGNRAM 4
SET_CONTROL:
        PUSH {LR}
        MSR  CONTROL, r0 ; Write R0 into CONTROL register
        POP  {PC}       

        ALIGNRAM 4
BACK_TO_START:
        DSB     ; Make sure outstanding transfers are done        
        ISB        ; Make sure outstanding transfers are done
        LDR R0, =0x10010000
        LDR R1, [R0] ; Load addr 0x1000 0000 contet to R1
        MSR MSP, R1
        LDR R1, [R0, #4]
        BLX  R1
 END