/*
 * Copyright (C) 2010 Melexis N.V.
 *
 * Software Platform
 *
 */

#ifndef ITC2_H_
#define ITC2_H_


#include <extintlib.h>

/*
 * 1st level (dispatcher) interrupt configuration
 */
#define ITC1_PWM_INT_ENABLE()       EXT5_INT_ENABLE()
#define ITC1_TIMER1_INT_ENABLE()    EXT6_INT_ENABLE()
#define ITC1_TIMER2_INT_ENABLE()    EXT7_INT_ENABLE()
#define ITC1_EXTIO_INT_ENABLE(p)    EXT3_INT_ENABLE(p)
#define ITC1_DIAG_INT_ENABLE(p)     EXT4_INT_ENABLE(p)


/*
 * Helper macros
 */
#define __L2_INT_ENABLE(pend, mask, bitno)  \
    do {                                    \
        (pend)  = (1u << (bitno));          \
        (mask) |= (1u << (bitno));          \
    } while (0)

#define __L2_INT_DISABLE(pend, mask, bitno) \
        (mask) &= ~(1u << (bitno))

#define __L2_INT_CLEAR(pend, mask, bitno)   \
        (pend) = (1u << (bitno))

/*
 * API functions
 */
/* ITC1_PWM_INT dispatcher (EXT5_IT) */
#define ITC2_PWMA_ENABLE()              __L2_INT_ENABLE( PWM_PEND, PWM_MASK, 0)
#define ITC2_PWMA_DISABLE()             __L2_INT_DISABLE(PWM_PEND, PWM_MASK, 0)
#define ITC2_PWMA_ACKNOWLEDGE()         __L2_INT_CLEAR(  PWM_PEND, PWM_MASK, 0)

#define ITC2_PWMI_ENABLE()              __L2_INT_ENABLE( PWM_PEND, PWM_MASK, 1)
#define ITC2_PWMI_DISABLE()             __L2_INT_DISABLE(PWM_PEND, PWM_MASK, 1)
#define ITC2_PWMI_ACKNOWLEDGE()         __L2_INT_CLEAR(  PWM_PEND, PWM_MASK, 1)

/* ITC1_TIMER1_INT dispatcher (EXT6_IT) */
#define ITC2_T1_IRQ1_ENABLE()           __L2_INT_ENABLE( TIMER1_PEND, TIMER1_MASK, 0)
#define ITC2_T1_IRQ1_DISABLE()          __L2_INT_DISABLE(TIMER1_PEND, TIMER1_MASK, 0)
#define ITC2_T1_IRQ1_ACKNOWLEDGE()      __L2_INT_CLEAR(  TIMER1_PEND, TIMER1_MASK, 0)

#define ITC2_T1_IRQ2_ENABLE()           __L2_INT_ENABLE( TIMER1_PEND, TIMER1_MASK, 1)
#define ITC2_T1_IRQ2_DISABLE()          __L2_INT_DISABLE(TIMER1_PEND, TIMER1_MASK, 1)
#define ITC2_T1_IRQ2_ACKNOWLEDGE()      __L2_INT_CLEAR(  TIMER1_PEND, TIMER1_MASK, 1)

#define ITC2_T1_IRQ3_ENABLE()           __L2_INT_ENABLE( TIMER1_PEND, TIMER1_MASK, 2)
#define ITC2_T1_IRQ3_DISABLE()          __L2_INT_DISABLE(TIMER1_PEND, TIMER1_MASK, 2)
#define ITC2_T1_IRQ3_ACKNOWLEDGE()      __L2_INT_CLEAR(  TIMER1_PEND, TIMER1_MASK, 2)

#define ITC2_T1_IRQ4_ENABLE()           __L2_INT_ENABLE( TIMER1_PEND, TIMER1_MASK, 3)
#define ITC2_T1_IRQ4_DISABLE()          __L2_INT_DISABLE(TIMER1_PEND, TIMER1_MASK, 3)
#define ITC2_T1_IRQ4_ACKNOWLEDGE()      __L2_INT_CLEAR(  TIMER1_PEND, TIMER1_MASK, 3)

#define ITC2_T1_IRQ5_ENABLE()           __L2_INT_ENABLE( TIMER1_PEND, TIMER1_MASK, 4)
#define ITC2_T1_IRQ5_DISABLE()          __L2_INT_DISABLE(TIMER1_PEND, TIMER1_MASK, 4)
#define ITC2_T1_IRQ5_ACKNOWLEDGE()      __L2_INT_CLEAR(  TIMER1_PEND, TIMER1_MASK, 4)

/* ITC1_TIMER2_INT dispatcher (EXT7_IT) */
#define ITC2_T2_IRQ1_ENABLE()           __L2_INT_ENABLE( TIMER2_PEND, TIMER2_MASK, 0)
#define ITC2_T2_IRQ1_DISABLE()          __L2_INT_DISABLE(TIMER2_PEND, TIMER2_MASK, 0)
#define ITC2_T2_IRQ1_ACKNOWLEDGE()      __L2_INT_CLEAR(  TIMER2_PEND, TIMER2_MASK, 0)

#define ITC2_T2_IRQ2_ENABLE()           __L2_INT_ENABLE( TIMER2_PEND, TIMER2_MASK, 1)
#define ITC2_T2_IRQ2_DISABLE()          __L2_INT_DISABLE(TIMER2_PEND, TIMER2_MASK, 1)
#define ITC2_T2_IRQ2_ACKNOWLEDGE()      __L2_INT_CLEAR(  TIMER2_PEND, TIMER2_MASK, 1)

#define ITC2_T2_IRQ3_ENABLE()           __L2_INT_ENABLE( TIMER2_PEND, TIMER2_MASK, 2)
#define ITC2_T2_IRQ3_DISABLE()          __L2_INT_DISABLE(TIMER2_PEND, TIMER2_MASK, 2)
#define ITC2_T2_IRQ3_ACKNOWLEDGE()      __L2_INT_CLEAR(  TIMER2_PEND, TIMER2_MASK, 2)

#define ITC2_T2_IRQ4_ENABLE()           __L2_INT_ENABLE( TIMER2_PEND, TIMER2_MASK, 3)
#define ITC2_T2_IRQ4_DISABLE()          __L2_INT_DISABLE(TIMER2_PEND, TIMER2_MASK, 3)
#define ITC2_T2_IRQ4_ACKNOWLEDGE()      __L2_INT_CLEAR(  TIMER2_PEND, TIMER2_MASK, 3)

#define ITC2_T2_IRQ5_ENABLE()           __L2_INT_ENABLE( TIMER2_PEND, TIMER2_MASK, 4)
#define ITC2_T2_IRQ5_DISABLE()          __L2_INT_DISABLE(TIMER2_PEND, TIMER2_MASK, 4)
#define ITC2_T2_IRQ5_ACKNOWLEDGE()      __L2_INT_CLEAR(  TIMER2_PEND, TIMER2_MASK, 4)

/* ITC1_EXTIO_INT dispatcher (EXT3_IT) */
#define ITC2_CURREG_ENABLE()            __L2_INT_ENABLE( EXTIO_PEND, EXTIO_MASK, 0)
#define ITC2_CURREG_DISABLE()           __L2_INT_DISABLE(EXTIO_PEND, EXTIO_MASK, 0)
#define ITC2_CURREG_ACKNOWLEDGE()       __L2_INT_CLEAR(  EXTIO_PEND, EXTIO_MASK, 0)

#define ITC2_SPI_OVF_ENABLE()           __L2_INT_ENABLE( EXTIO_PEND, EXTIO_MASK, 1)
#define ITC2_SPI_OVF_DISABLE()          __L2_INT_DISABLE(EXTIO_PEND, EXTIO_MASK, 1)
#define ITC2_SPI_OVF_ACKNOWLEDGE()      __L2_INT_CLEAR(  EXTIO_PEND, EXTIO_MASK, 1)

#define ITC2_SPI_REC_WORD_ENABLE()      __L2_INT_ENABLE( EXTIO_PEND, EXTIO_MASK, 2)
#define ITC2_SPI_REC_WORD_DISABLE()     __L2_INT_DISABLE(EXTIO_PEND, EXTIO_MASK, 2)
#define ITC2_SPI_REC_WORD_ACKNOWLEDGE() __L2_INT_CLEAR(  EXTIO_PEND, EXTIO_MASK, 2)

#define ITC2_SPI_TR_WORD_ENABLE()       __L2_INT_ENABLE( EXTIO_PEND, EXTIO_MASK, 3)
#define ITC2_SPI_TR_WORD_DISABLE()      __L2_INT_DISABLE(EXTIO_PEND, EXTIO_MASK, 3)
#define ITC2_SPI_TR_WORD_ACKNOWLEDGE()  __L2_INT_CLEAR(  EXTIO_PEND, EXTIO_MASK, 3)

#define ITC2_EXTIO_IRQ0_ENABLE()        __L2_INT_ENABLE( EXTIO_PEND, EXTIO_MASK, 4)
#define ITC2_EXTIO_IRQ0_DISABLE()       __L2_INT_DISABLE(EXTIO_PEND, EXTIO_MASK, 4)
#define ITC2_EXTIO_IRQ0_ACKNOWLEDGE()   __L2_INT_CLEAR(  EXTIO_PEND, EXTIO_MASK, 4)

#define ITC2_EXTIO_IRQ1_ENABLE()        __L2_INT_ENABLE( EXTIO_PEND, EXTIO_MASK, 5)
#define ITC2_EXTIO_IRQ1_DISABLE()       __L2_INT_DISABLE(EXTIO_PEND, EXTIO_MASK, 5)
#define ITC2_EXTIO_IRQ1_ACKNOWLEDGE()   __L2_INT_CLEAR(  EXTIO_PEND, EXTIO_MASK, 5)

#define ITC2_EXTIO_IRQ2_ENABLE()        __L2_INT_ENABLE( EXTIO_PEND, EXTIO_MASK, 6)
#define ITC2_EXTIO_IRQ2_DISABLE()       __L2_INT_DISABLE(EXTIO_PEND, EXTIO_MASK, 6)
#define ITC2_EXTIO_IRQ2_ACKNOWLEDGE()   __L2_INT_CLEAR(  EXTIO_PEND, EXTIO_MASK, 6)

#define ITC2_EXTIO_IRQ3_ENABLE()        __L2_INT_ENABLE( EXTIO_PEND, EXTIO_MASK, 7)
#define ITC2_EXTIO_IRQ3_DISABLE()       __L2_INT_DISABLE(EXTIO_PEND, EXTIO_MASK, 7)
#define ITC2_EXTIO_IRQ3_ACKNOWLEDGE()   __L2_INT_CLEAR(  EXTIO_PEND, EXTIO_MASK, 7)

#define ITC2_EXTIO_IRQ4_ENABLE()        __L2_INT_ENABLE( EXTIO_PEND, EXTIO_MASK, 8)
#define ITC2_EXTIO_IRQ4_DISABLE()       __L2_INT_DISABLE(EXTIO_PEND, EXTIO_MASK, 8)
#define ITC2_EXTIO_IRQ4_ACKNOWLEDGE()   __L2_INT_CLEAR(  EXTIO_PEND, EXTIO_MASK, 8)

#define ITC2_EXTIO_IRQ5_ENABLE()        __L2_INT_ENABLE( EXTIO_PEND, EXTIO_MASK, 9)
#define ITC2_EXTIO_IRQ5_DISABLE()       __L2_INT_DISABLE(EXTIO_PEND, EXTIO_MASK, 9)
#define ITC2_EXTIO_IRQ5_ACKNOWLEDGE()   __L2_INT_CLEAR(  EXTIO_PEND, EXTIO_MASK, 9)

#define ITC2_EXTIO_IRQ6_ENABLE()        __L2_INT_ENABLE( EXTIO_PEND, EXTIO_MASK, 10)
#define ITC2_EXTIO_IRQ6_DISABLE()       __L2_INT_DISABLE(EXTIO_PEND, EXTIO_MASK, 10)
#define ITC2_EXTIO_IRQ6_ACKNOWLEDGE()   __L2_INT_CLEAR(  EXTIO_PEND, EXTIO_MASK, 10)


/* ITC1_DIAG_INT dispatcher (EXT4_IT) */
#define ITC2_VSUP_OV_ENABLE()           __L2_INT_ENABLE( DIAGNOSTIC_PEND, DIAGNOSTIC_MASK, 0)
#define ITC2_VSUP_OV_DISABLE()          __L2_INT_DISABLE(DIAGNOSTIC_PEND, DIAGNOSTIC_MASK, 0)
#define ITC2_VSUP_OV_ACKNOWLEDGE()      __L2_INT_CLEAR(  DIAGNOSTIC_PEND, DIAGNOSTIC_MASK, 0)

#define ITC2_VSUP_UV_ENABLE()           __L2_INT_ENABLE( DIAGNOSTIC_PEND, DIAGNOSTIC_MASK, 1)
#define ITC2_VSUP_UV_DISABLE()          __L2_INT_DISABLE(DIAGNOSTIC_PEND, DIAGNOSTIC_MASK, 1)
#define ITC2_VSUP_UV_ACKNOWLEDGE()      __L2_INT_CLEAR(  DIAGNOSTIC_PEND, DIAGNOSTIC_MASK, 1)

#define ITC2_VREG_UV_ENABLE()           __L2_INT_ENABLE( DIAGNOSTIC_PEND, DIAGNOSTIC_MASK, 2)
#define ITC2_VREG_UV_DISABLE()          __L2_INT_DISABLE(DIAGNOSTIC_PEND, DIAGNOSTIC_MASK, 2)
#define ITC2_VREG_UV_ACKNOWLEDGE()      __L2_INT_CLEAR(  DIAGNOSTIC_PEND, DIAGNOSTIC_MASK, 2)

#define ITC2_VBOOST_UV_ENABLE()         __L2_INT_ENABLE( DIAGNOSTIC_PEND, DIAGNOSTIC_MASK, 3)
#define ITC2_VBOOST_UV_DISABLE()        __L2_INT_DISABLE(DIAGNOSTIC_PEND, DIAGNOSTIC_MASK, 3)
#define ITC2_VBOOST_UV_ACKNOWLEDGE()    __L2_INT_CLEAR(  DIAGNOSTIC_PEND, DIAGNOSTIC_MASK, 3)

#define ITC2_OVER_TEMP_ENABLE()         __L2_INT_ENABLE( DIAGNOSTIC_PEND, DIAGNOSTIC_MASK, 4)
#define ITC2_OVER_TEMP_DISABLE()        __L2_INT_DISABLE(DIAGNOSTIC_PEND, DIAGNOSTIC_MASK, 4)
#define ITC2_OVER_TEMP_ACKNOWLEDGE()    __L2_INT_CLEAR(  DIAGNOSTIC_PEND, DIAGNOSTIC_MASK, 4)

#define ITC2_VDS_ERROR_ENABLE()         __L2_INT_ENABLE( DIAGNOSTIC_PEND, DIAGNOSTIC_MASK, 5)
#define ITC2_VDS_ERROR_DISABLE()        __L2_INT_DISABLE(DIAGNOSTIC_PEND, DIAGNOSTIC_MASK, 5)
#define ITC2_VDS_ERROR_ACKNOWLEDGE()    __L2_INT_CLEAR(  DIAGNOSTIC_PEND, DIAGNOSTIC_MASK, 5)

#define ITC2_VGS_ERROR_ENABLE()         __L2_INT_ENABLE( DIAGNOSTIC_PEND, DIAGNOSTIC_MASK, 6)
#define ITC2_VGS_ERROR_DISABLE()        __L2_INT_DISABLE(DIAGNOSTIC_PEND, DIAGNOSTIC_MASK, 6)
#define ITC2_VGS_ERROR_ACKNOWLEDGE()    __L2_INT_CLEAR(  DIAGNOSTIC_PEND, DIAGNOSTIC_MASK, 6)

#define ITC2_OVER_CURRENT_ENABLE()      __L2_INT_ENABLE( DIAGNOSTIC_PEND, DIAGNOSTIC_MASK, 7)
#define ITC2_OVER_CURRENT_DISABLE()     __L2_INT_DISABLE(DIAGNOSTIC_PEND, DIAGNOSTIC_MASK, 7)
#define ITC2_OVER_CURRENT_ACKNOWLEDGE() __L2_INT_CLEAR(  DIAGNOSTIC_PEND, DIAGNOSTIC_MASK, 7)

#define ITC2_HVIO0_SHORT_DET_ENABLE()       __L2_INT_ENABLE( DIAGNOSTIC_PEND, DIAGNOSTIC_MASK, 8)
#define ITC2_HVIO0_SHORT_DET_DISABLE()      __L2_INT_DISABLE(DIAGNOSTIC_PEND, DIAGNOSTIC_MASK, 8)
#define ITC2_HVIO0_SHORT_DET_ACKNOWLEDGE()  __L2_INT_CLEAR(  DIAGNOSTIC_PEND, DIAGNOSTIC_MASK, 8)

#define ITC2_HVIO1_SHORT_DET_ENABLE()       __L2_INT_ENABLE( DIAGNOSTIC_PEND, DIAGNOSTIC_MASK, 9)
#define ITC2_HVIO1_SHORT_DET_DISABLE()      __L2_INT_DISABLE(DIAGNOSTIC_PEND, DIAGNOSTIC_MASK, 9)
#define ITC2_HVIO1_SHORT_DET_ACKNOWLEDGE()  __L2_INT_CLEAR(  DIAGNOSTIC_PEND, DIAGNOSTIC_MASK, 9)

#define ITC2_HVIO2_SHORT_DET_ENABLE()       __L2_INT_ENABLE( DIAGNOSTIC_PEND, DIAGNOSTIC_MASK, 10)
#define ITC2_HVIO2_SHORT_DET_DISABLE()      __L2_INT_DISABLE(DIAGNOSTIC_PEND, DIAGNOSTIC_MASK, 10)
#define ITC2_HVIO2_SHORT_DET_ACKNOWLEDGE()  __L2_INT_CLEAR(  DIAGNOSTIC_PEND, DIAGNOSTIC_MASK, 10)

/*
 * Second level handlers
 */
/* connected to ITC1_PWM_INT dispatcher (EXT5_IT) */
extern void ITC2_PWMA (void);
extern void ITC2_PWMI (void);

/* connected to ITC1_TIMER1_INT dispatcher (EXT6_IT) */
extern void ITC2_T1_IRQ1 (void);
extern void ITC2_T1_IRQ2 (void);
extern void ITC2_T1_IRQ3 (void);
extern void ITC2_T1_IRQ4 (void);
extern void ITC2_T1_IRQ5 (void);

/* connected to ITC1_TIMER2_INT dispatcher (EXT7_IT) */
extern void ITC2_T2_IRQ1 (void);
extern void ITC2_T2_IRQ2 (void);
extern void ITC2_T2_IRQ3 (void);
extern void ITC2_T2_IRQ4 (void);
extern void ITC2_T2_IRQ5 (void);

/* connected to ITC1_EXTIO_INT dispatcher (EXT3_IT) */
extern void ITC2_CURREG (void);
extern void ITC2_SPI_OVF (void);
extern void ITC2_SPI_REC_WORD (void);
extern void ITC2_SPI_TR_WORD (void);
extern void ITC2_EXTIO_IRQ0 (void);
extern void ITC2_EXTIO_IRQ1 (void);
extern void ITC2_EXTIO_IRQ2 (void);
extern void ITC2_EXTIO_IRQ3 (void);
extern void ITC2_EXTIO_IRQ4 (void);
extern void ITC2_EXTIO_IRQ5 (void);
extern void ITC2_EXTIO_IRQ6 (void);

/* connected to ITC1_DIAG_INT dispatcher (EXT4_IT) */
extern void ITC2_VSUP_OV (void);
extern void ITC2_VSUP_UV (void);
extern void ITC2_VREG_UV (void);
extern void ITC2_VBOOST_UV (void);
extern void ITC2_OVER_TEMP (void);
extern void ITC2_VDS_ERROR (void);
extern void ITC2_VGS_ERROR (void);
extern void ITC2_OVER_CURRENT (void);
extern void ITC2_HVIO0_SHORT (void);
extern void ITC2_HVIO1_SHORT (void);
extern void ITC2_HVIO2_SHORT (void);

/*
 * Second level handler type
 */
typedef void (* L2_HandlerType)(void);


extern const L2_HandlerType L2_vector_table[];

/*-----------------------------------------------------------------------------
 * Initializes second level interrupt controller
 *
 * Arguments:   none
 *
 * Returns:     none
 *
 */
inline __attribute__((always_inline))
static void ITC2_Init (void)
{
    /*
     * Second level interrupts pending flags and masks are undefined
     * after reset. Mask (disable) all 2nd level interrupts and reset any
     * pending flags (by writing 1s).
     */
    DIAGNOSTIC_MASK = 0;
    EXTIO_MASK      = 0;
    TIMER1_MASK     = 0;
    TIMER2_MASK     = 0;
    PWM_MASK        = 0;

    DIAGNOSTIC_PEND = 0xFFFF;
    EXTIO_PEND      = 0xFFFF;
    TIMER1_PEND     = 0xFFFF;
    TIMER2_PEND     = 0xFFFF;
    PWM_PEND        = 0xFFFF;
}


#endif /* ITC2_H_ */
