!@This is the start of file &DCALI
!
!  THIS COMMON BLOCK USES A SIMULATED LIST STRUCTURE TO KEEP TRACK OF
!  CALIBARITION CONTROL INFORMATION
!
!  TERMINOLOGY:
!
!    ITEM: FOUR WORDS OF HOLLERITH DATA
!    FRAME: AN ARBITRARY NUMBER OF 'ITEMS' PRECEDED BY A WORD CONTAINING
!           AN INTEGER WHOSE ABSOLUTE VALUE IS THE NUMBER OF ITEMS IN THE
!           FRAME
!    COUNT: IS THE NUMBER OF ITEMS IN THE FRAME
!    SECTION: AT LEAST TWO FRAMES, THE LAST ONE OF WHICH HAS A NEGATIVE
!             ITEM COUNT
!
! e.g., a section containing 2 frames the first of which has COUNTx items
!       the second of which has COUNTy items, for a total of
!        2+4*(COUNTx+COUNTy) words:
!
!      COUNTx  ITEMx(1,1),...,ITEMx(4,1),...ITEMx(4,COUNTx)
!    (-COUNTy) ITEMy(1,1),...,ITEMy(4,1),...ITEMx(4,COUNTy)
!
!   THE LIST IS STORED IN IACLAI, NXTCAL IS THE INDEX OF THE ITEM COUNT
!   FOR THE CURRENT FRAME. AN EMPTY FRAME (COUNT=0) TERMINATES THE LIST
!   THE LIST IS NOT USABLE UNTIL IT IS BUILT BY GCALIB et al.
!
      INTEGER*2 MAX_CALI_WORDS
      PARAMETER (MAX_CALI_WORDS=256)
!
      INTEGER*2 IACALI(MAX_CALI_WORDS)
      CHARACTER KEECAL(MAX_CAL)*8, DISCAL(MAX_CAL)*8, ENBCAL(MAX_CAL)*8
      INTEGER*4 L_KEE, L_DIS, L_ENB
      LOGICAL*4 FL_RESET
!
      COMMON / DCALI / L_KEE, L_DIS, L_ENB,
     .                 KEECAL, DISCAL, ENBCAL,
     .                 FL_RESET,
     .                 IACALI