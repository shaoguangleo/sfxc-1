      DIMENSION IRM(5)
      REAL*8 FJD
      REAL*8 XC, XW
      COMMON/WXCB/XC(2000),YC1(2000),XW(2000),YW1(2000),
     .            YW2(2000),YW3(2000)
      COMMON/TIMJD/FJD
      COMMON/LYUS/ICRT,KBRD,LUP,IDT,IRM,KIN,IODLEN,IODES
      COMMON/INOUT/NCB,NWX,IBFR,IEXP,CSAVE,WSAVE,IDOY,IYR,BATCH,XGO
      COMMON/CALS/NCABL,KWX,ISTN,XMIN,XMAX,LSIGN,IDUM
      COMMON/AUTOP/ycmin,ycmax,ytmin,ytmax,ypmin,ypmax,yhmin,yhmax
      integer*4 ycmin,ycmax,ytmin,ytmax,ypmin,ypmax,yhmin,yhmax
      CHARACTER*40 KWX,NCABL
      CHARACTER*80 IBFR
      CHARACTER*8  ISTN
      CHARACTER*14 IEXP
      CHARACTER*1 CSAVE,WSAVE,LSIGN,IDUM
      LOGICAL BATCH,XGO
      INTEGER*2 IODLEN,IODES(5)