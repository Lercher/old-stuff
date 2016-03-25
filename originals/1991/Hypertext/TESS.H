    /*
     * TESS.H -- 'C' INCLUDE file for TesSeRact
     */
/********************************< TESS.H >**********************************
*                                                                           *
*                       C Include File for TesSeRact                        *
*                       ----------------------------                        *
*                                                                           *
*   TesSeRact(TM) -- A Library of Routines for Creating Ram-Resident (TSR)  *
*                    programs for the IBM PC and compatible Personal        *
*                    Computers.                                             *
*                                                                           *
*   The software, documentation and source code are:                        *
*                                                                           *
*       Copyright (C) 1986, 1987, 1988 Tesseract Development Team           *
*       All Rights Reserved                                                 *
*                                                                           *
*************************************************************************CR*/

/**
     This product  supports the  TesSeRact(TM)  Standard  for  Ram-Resident
     Program Communication.   For  information about TesSeRact, contact the
     TesSeRact Development Team at:
     
          TesSeRact Development Team
          c/o Chip Rabinowitz
          2084 Woodlawn Avenue
          Glenside, PA 19038
          1-215-884-3373
     
          Compuserve:    70731,20
          MCIMAIL:       315-5415
     
     This MCIMAIL  Account has  been provided  to the TesSeRact Development
     Team by Borland International, Inc.  The TesSeRact Development Team is
     in no  way associated with Borland International, Inc.  TesSeRact is a
     trademark of the TesSeRact Development Team.

**/

/**
***************************************************************************
                             Copyright Notice
***************************************************************************

This  document,   other  accompanying  written  and  disk-based  notes  and
specifications, and all referenced and related program files, demonstration
code and  object modules  accompanying this document are copyrighted by the
TesSeRact Development  Team.   The copyright  owner hereby licenses you to:
use the  software; make as many copies of the software and documentation as
you wish;  give exact  copies of the original to anyone; and distribute the
software and  documentation in  its unmodified  form via  electronic means.
There is no charge for any of the above.

You are specifically prohibited from charging, or requesting donations, for
any such  copies, however made.  Exceptions may be granted to organizations
which charge  a small  fee for  materials, handling,  postage  and  general
overhead.  NO   ORGANIZATION  IS   AUTHORIZED  TO  CHARGE  ANY  AMOUNT  FOR
DISTRIBUTION OF  THE SOFTWARE  OR DOCUMENTATION UNDER ANY OTHER CONDITIONS.
Organizations which  charge a fee for distribution of any and all TesSeRact
materials, except  as noted  above or  with the express, written consent of
the TesSeRact  Development Team,  will be  considered in  violation of this
copyright, and will be prosecuted to the full extent of the law.

In addition,  you are specifically prohibited from making any modifications
to the  TesSeRact Library  Routines and/or documentation, unless you have a
license for  the use  of the  source code.   Under  no circumstances is the
copyright notice embedded in the TesSeRact code to be modified or removed.

This is  not free  software.   This license allows you to use this software
without charge  for a period of 30 days.  In order to include this software
as part  of any  product, either commercial, shareware, freeware, or public
domain, registration  is required.   TesSeRact  may not  be included in any
product for  any use  without registration.   Any such use of the TesSeRact
product are in violation of federal copyright laws, and will be prosecuted.

No copy  of the  software may  be distributed  or given  away without  this
accompanying documentation; this notice must not be removed.


***************************************************************************
                                 Warranty
***************************************************************************

There is  no warranty  of any  kind associated  with this software, and the
copyright owner  is not  liable for  damages of  any kind.   By  using this
software, you  agree to  this.  Every effort has been made by the TesSeRact
Development Team  to make  this product  bug-free.   However, the nature of
software development  is  that  it  is  impossible  to  guarantee  bug-free
software.

In the event a verifiable bug is found, the TesSeRact Development Team will
make every  attempt to  repair the  bug as  soon as possible.  

***************************************************************************
                             Acknowledgements
***************************************************************************

Grateful appreciation  is expressed  to CompuServe Information Service, the
IBM Software  Forums and  their Sysops, and Computer Language Magazine, for
providing us  with the  means to  accomplish the  development of TesSeRact.
Without their  gracious assistance,  TesSeRact could  never have  come into
existence.

And let's not forget Anne Marie, who suffered through the long evenings and
nights spent getting this product ready.

***************************************************************************
The TesSeRact Development Team:
***************************************************************************
     
     Team Leader:   Chip Rabinowitz
     Developers:    Lane Ferris
                    Kim Kokkonen
                    Jim Kyle
                    Neil J. Rubenking
                    Barry Simon
                    Rick Wilson
     Contributors:  Thomas Brandenborg
                    Chris Dunford
                    John Friend
                    Mark Horvatich
                    John Hensley
     Advisors:      Robert Bierman
                    David Intersimone
                    Rick Kraus
                    David Moskowitz
                    Gary Saxer

***************************************************************************

**/

/*#define MSC5 */                       /* If using Microsoft C 5.0        */
/*#define TC */                         /* If using Turbo C 1.5            */


    /*
     * Stores interrupt vector information for TesSeRact use
     */
struct TsrIntTable {
    void far * OldVector;               /* Old Interrupt Vector            */
    unsigned char IntNumber;            /* Interrupt Number                */
    void near * NewVector;              /* offset of new vector            */
    };

    /*
     * User structure for defining additional hotkeys
     */
struct ExtraHot {
    unsigned char Hotkey;               /* hotkey to check for             */
    unsigned char ShiftState;           /* shift state for this hot key    */
    unsigned char FlagByte;             /* flag value to use               */
                                        /*  MAY NOT BE ZERO!!!             */
    };

    /*
     * TesSeRact internal data
     */
struct TsrData {
    unsigned char RevLvl;               /* Revision Level of TESS Lib      */
    unsigned char PopupType;            /* Type of popup in effect         */
    unsigned char WasInt8;              /* An Interrupt 08h occurred       */
    unsigned char WasInt13;             /* An Interrupt 13h occurred       */
    unsigned char IntFlags;             /* Which interrupts are active     */
    unsigned char SoftFlags;            /* Which soft ints are active      */
    unsigned char DosVersion;           /* current major revision of DOS   */
    unsigned char waitcount;            /* Count to wait before popping up */
    void far * InDosFlag;               /* Pointer to DOS INDOS flag       */
    void far * DosCritErr;              /* Pointer to DOS Critical Error   */
    unsigned short UserPSP;             /* PSP segment of user program     */
    unsigned short User28PSP;           /* PSP segment of user program     */
    void far * UserDTA;                 /* DTA of interrupted program      */
    void far * User28DTA;               /* DTA of interrupted program      */
    unsigned short UserSS;              /* Stack segment of user program   */
    unsigned short UserSP;              /* Stack pointer of user program   */
    unsigned short User28SS;            /* Stack segment of user program   */
    unsigned short User28SP;            /* Stack pointer of user program   */
    void far * UserInt24;               /* pointer to use INT 24 handler   */
    unsigned short OldExtErr[3];        /* storage for old DOS 3 extended  */
                                        /*     error information           */
    unsigned char OldBreak;             /* Old Break Setting               */
    unsigned char OldVerify;            /* Old Verify Setting              */
    unsigned char InWord4;              /* flag to indicate in WORD 4.0    */
    unsigned char WasWord4;             /* word 4 special popup flag       */
    unsigned char NewKbdFlag;           /* Enhanced Keyboard Call in use   */
    unsigned char Word4Delay;           /* Delay for Word 4                */

    struct TsrIntTable Int8;            /* Interrupt vector tables         */
    struct TsrIntTable Int9;
    struct TsrIntTable Int13;
    struct TsrIntTable Int16;
    struct TsrIntTable Int1C;
    struct TsrIntTable Int21;
    struct TsrIntTable Int28;
    struct TsrIntTable Int2F;
    struct TsrIntTable Int1B;
    struct TsrIntTable Int23;
    struct TsrIntTable Int24;

    };

    /*
     * TesSeRact data for individual TSR
     */
struct TsrParms {
    char IdCode[8];                     /* Unique TSR Identification String*/
                                        /* NOTE -- NOT NULL-TERMINATED     */
    unsigned short IdNum;               /* Unique TSR Identification Number*/
    unsigned long FuncFlags;            /* Bit map of supported functions  */
    unsigned char HotKey;               /* Hotkey used by TSR for popup    */
    unsigned char ShiftState;           /* ShiftState used by this TSR pop */
    unsigned char HotKeyFlag;           /* which hotkey is in use          */
    unsigned char ExtraHotCount;        /* Number of Extra Hotkeys to use  */
    void far * ExtraHotKeys;            /* Pointer to extra hotkeys        */
    unsigned short TsrStatus;           /* Current TSR Status Flags        */
    unsigned short TsrPSP;              /* TSR's PSP Segment               */
    void far * TsrDTA;                  /* Pointer to TSR's DTA region     */
    unsigned short TsrDSeg;             /* TSR's Default Data Segment      */
    void far * PopupStack;              /* Pointer to Popup Stack Area     */
    void far * BackStack;               /* Pointer to Background Stack     */
    };

/*-------------------------------------------------------------------------*/

        /************************************************************
        *   Other EQUates                                           *
        *********************************************************CR*/

    /*
     * #defines for HotKey
     */
#define TSRHOT_A        0x1e
#define TSRHOT_B        0x30
#define TSRHOT_C        0x2e
#define TSRHOT_D        0x20
#define TSRHOT_E        0x12
#define TSRHOT_F        0x21
#define TSRHOT_G        0x22
#define TSRHOT_H        0x23
#define TSRHOT_I        0x17
#define TSRHOT_J        0x24
#define TSRHOT_K        0x25
#define TSRHOT_L        0x26
#define TSRHOT_M        0x32
#define TSRHOT_N        0x31
#define TSRHOT_O        0x18
#define TSRHOT_P        0x19
#define TSRHOT_Q        0x10
#define TSRHOT_R        0x13
#define TSRHOT_S        0x1f
#define TSRHOT_T        0x14
#define TSRHOT_U        0x16
#define TSRHOT_V        0x2f
#define TSRHOT_W        0x11
#define TSRHOT_X        0x2d
#define TSRHOT_Y        0x15
#define TSRHOT_Z        0x2c
#define TSRHOT_0        0x0b
#define TSRHOT_1        0x02
#define TSRHOT_2        0x03
#define TSRHOT_3        0x04
#define TSRHOT_4        0x05
#define TSRHOT_5        0x06
#define TSRHOT_6        0x07
#define TSRHOT_7        0x08
#define TSRHOT_8        0x09
#define TSRHOT_9        0x0a
#define TSRHOT_F1       0x3b
#define TSRHOT_F2       0x3c
#define TSRHOT_F3       0x3d
#define TSRHOT_F4       0x3e
#define TSRHOT_F5       0x3f
#define TSRHOT_F6       0x40
#define TSRHOT_F7       0x41
#define TSRHOT_F8       0x42
#define TSRHOT_F9       0x43
#define TSRHOT_F10      0x44
    
    /*
     * Enhanced Keyboards only
     * 
     * May not work with all computers, keyboards
     * 
     */
#define TSRHOT_F11      0x57
#define TSRHOT_F12      0x58

    /*
     * #defines for ShiftState
     */
#define TSRPOPRSHIFT    0x01
#define TSRPOPLSHIFT    0x02
#define TSRPOPCTRL      0x04
#define TSRPOPALT       0x08

    /*
     * #defines for TSRFLAGS
     */
#define TSRUSEPOPUP     0x0010          /* Use TsrMain()                   */
#define TSRUSEBACK      0x0020          /* Use TsrBackProc()               */
#define TSRUSETIMER     0x0040          /* Use TsrTimer()                  */
#define TSRUSEUSER      0x0100          /* Use TsrUserProc()               */
#define NOPOPGRAPH      0x1000          /* Don't pop over graphics         */
#define NOPOPCOMMAND    0x2000          /* Don't pop over command-line     */

    /*
     * #defines for TsrStatus flags
     */

#define HOTKEYON        0x0001          /* hot key pressed                 */
#define SHIFTSON        0x0002          /* shift states match              */
#define TSRACTIVE       0x0004          /* tsr is running in foreground    */
#define INT28ACTIVE     0x0008          /* INT28 routine is running        */

#define POPUPSET        0x0010          /* popup resident routine installed*/
#define BACKSET         0x0020          /* background routine installed    */
#define TIMERSET        0x0040          /* timer procedure installed       */
#define EXTRAHOTSET     0x0080          /* extra hot keys installed        */

#define USERPROCON      0x0100          /* User-defined procedure installed*/
#define TSRENABLED      0x0200          /* TSR currently enabled           */
#define TSRRELEASED     0x0400          /* TSR has been released           */
#define EXTRAINT24      0x0800          /* User installed replacement INT24*/


    /*
     * EQUates for IntFlags and SoftFlags
     */

#define ININT13         0x04            /* Interrupt 13 is currently active*/
#define ININT21         0x08            /* Interrupt 21 is currently active*/
#define ININT16         0x10            /* Interrupt 16 is currently active*/
#define ININT28         0x20            /* Interrupt 28 is currently active*/
#define ININT08         0x40            /* Interrupt 08 is currently active*/
#define ININT1C         0x80            /* Interrupt 1C is currently active*/

    /*
     * Keyboard Stuffing speeds
     */
#define STUFF_SLOW      0               /* Stuff key when buffer is empty  */
#define STUFF_MED       1               /* Stuff up to four keys per tick  */
#define STUFF_FAST      2               /* Full buffer on every tick       */

/*-------------------------------------------------------------------------*/

        /************************************************************
        *   Prototypes for routines in TESS.LIB                     *
        *********************************************************CR*/

extern unsigned far pascal TsDoInit(
    unsigned char HotKey,
    unsigned char ShiftState,
    unsigned short TSRFlags,
    unsigned short MemoryTop
    );


extern void far pascal TsSetStack(
    void far *PopUpStack, 
    void far *BackGroundStack
    );

extern void far pascal TessBeep(
    void
    );

/*-------------------------------------------------------------------------*/
        /************************************************************
        *  TesSeRact Multiplex Interrupt (2Fh) functions            *
        *    These functions are provided for your convenience in   *
        *    TESS.LIB                                               *
        *********************************************************CR*/

extern unsigned far pascal TsCheckResident(
    char far *IDStr,
    unsigned far *IDNum
    );

extern struct TsrParms far * far pascal TsGetParms(
    unsigned short TsrIdNum
    );

extern unsigned far pascal TsCheckHotkey(
    unsigned char HotKey
    );

extern unsigned far pascal TsSetUser24(
    unsigned short TsrIdNum,
    void (far *UserCritErrProc) (void)
    );

extern struct TsrData far * far pascal TsGetData(
    unsigned short TsrIdNum
    );

extern unsigned far pascal TsSetExtraHot(
    unsigned short TsrIdNum,
    unsigned char count, 
    struct ExtraHot far *ExtraHotKeys
    );

extern unsigned far pascal TsEnable(
    unsigned short TsrIdNum
    );

extern unsigned far pascal TsDisable(
    unsigned short TsrIdNum
    );

extern unsigned far pascal TsRelease(
    unsigned short TsrIdNum
    );

extern unsigned far pascal TsRestart(
    unsigned short TsrIdNum
    );

extern unsigned far pascal TsGetStat(
    unsigned short TsrIdNum
    );

extern unsigned far pascal TsSetStat(
    unsigned short TsrIdNum,
    unsigned short NewStatus
    );

extern unsigned far pascal TsGetPopType(
    unsigned short TsrIdNum
    );

extern unsigned far pascal TsCallUserProc(
    unsigned short TsrIdNum,
    void far *UserPtr
    );

extern unsigned far pascal TsStuffKeyboard(
    unsigned short TsrIdNum,
    void far *KbdChars,
    unsigned short CharCount,
    unsigned short Speed
    );

void far pascal TsVerify2f(void);

/*-------------------------------------------------------------------------*/
        /************************************************************
        *   USER-DEFINED routines                                   *
        *     If no entry point for these routines is defined in    *
        *     your code, a dummy procedure from TESS.LIB will be    *
        *     used to satisfy the external references.              *
        *       These are the prototypes for the functions.         *
        *     Note that beginning with Version 0.74, it is no       *
        *       longer necessary for all these procedures to be     *
        *       defined -- TESS.LIB has dummy procedures to resolve *
        *       the external references.                            *
        *********************************************************CR*/

void far pascal TsrMain(
    void
    );

unsigned far pascal TsrBackCheck(
    void
    );

void far pascal TsrBackProc(
    void
    );

void far pascal TsrTimerProc(
    void
    );

void far pascal TsrUserProc(
    void far *UserPtr
    );

void far pascal TsrCleanUp(
    unsigned InitOrShutdown
    );

