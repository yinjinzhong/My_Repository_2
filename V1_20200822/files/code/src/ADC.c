/*! ----------------------------------------------------------------------------
 * \file		ADC.c
 * \brief		MLX81310 ADC handling
 *
 * \note		project MLX81310
 * 
 * \author 		Marcel Braat
 *   
 * \date 		2012-02-11
 *   
 * \version 	1.0 - preliminary
 *
 * \functions	ADC_Init()
 *				ADC_Start()
 *				ADC_SetupShortDetection()
 *				ADC_Stop()
 *				ADC_PowerOff()
 *				ADC_IT()
 *				GetVsupply()
 *				GetVsupplyMotor()
 *				GetPhaseMotor()
 *				GetChipTemperature()
 *				GetRawMotorDriverCurrent()
 *				GetMotorDriverCurrent()
 *				MeasureVsupplyAndTemperature()
 *				MeasurePhaseVoltage()
 *
 * MELEXIS Microelectronic Integrated Systems
 * 
 * Copyright (C) 2012-2014 Melexis N.V.
 * The Software is being delivered 'AS IS' and Melexis, whether explicitly or 
 * implicitly, makes no warranty as to its Use or performance. 
 * The user accepts the Melexis Firmware License Agreement.  
 *
 * Melexis confidential & proprietary
 *
 * ****************************************************************************	*/

#include "Build.h"
#include "ADC.h"
#include "ErrorCodes.h"
#include "main.h"
#include "MotorDriver.h"
#include "private_mathlib.h"
#include <mathlib.h>															/* Use Melexis math-library functions to avoid compiler warnings */

/* ****************************************************************************	*
 *    NORMAL FAR IMPLEMENTATION	(@NEAR Memory Space >= 0x100)					*
 * ****************************************************************************	*/
#pragma space nodp																/* __NEAR_SECTION__ */
uint16 l_u16CurrentZeroOffset = 100U;											/* Zero-current ADC-offset */
uint16 g_u16MCurrgain = 300U;													/* MMP160616-1 */
#if _SUPPORT_LIN_AA
uint8 g_u8AdcIsrMode = C_ADC_ISR_NONE;											/* ADC ISR mode = None */
#endif /* _SUPPORT_LIN_AA */
uint8 l_u8AdcPowerOff = TRUE;

T_ADC_MOTORRUN_STEPPER4 volatile g_AdcMotorRunStepper4;							/* ADC results Stepper mode */
#if ((_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_VSM) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_GND) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRRORSPECIAL))
uint16 g_u16CurrentMotorCoilA = 0U;
uint16 g_u16CurrentMotorCoilB = 0U;
#endif /* ((_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_VSM) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_GND) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRRORSPECIAL)) */

#pragma space none																/* __NEAR_SECTION__ */

/* ****************************************************************************	*
 *	Internal function prototypes												*
 * ****************************************************************************	*/

/* ****************************************************************************	*
 *    ROM-Code tables															*
 * ****************************************************************************	*/
/* ADC Vref disable */															/* Ch  Trigger	Vref	Description */
uint16 const SBASE_VREF_OFF[2] = { (ADC_CH1  | ADC_REF_OFF), ADC_EOT};			/*  1	F/W		0.0V	Internal Temperature Sensor */

/* Current offset measurement */												/* Ch  Trigger	Vref	Description */
uint16 const SBASE_CURROFF[2] = {ADC_MCUR, ADC_EOT};							/* 13	 --%	2.5V	Unfiltered Current */
uint16 const SBASE_VDDA[2] = { ADC_VDDA, ADC_EOT};								/*  3	F/W		2.5V	VDDA voltage (divided by 2) */
uint16 const SBASE_VDDD[2] = { ADC_VDDD, ADC_EOT};								/*  2	F/W		2.5V	VDDD voltage */
uint16 const SBASE_VIO[6][2] =
{
	{ ADC_IO0, ADC_EOT},
	{ ADC_IO1, ADC_EOT},
	{ ADC_IO2, ADC_EOT},
	{ ADC_IO3, ADC_EOT},
	{ ADC_IO4, ADC_EOT},
	{ ADC_IO5, ADC_EOT}
};	/* (MMP170302-1) */

/* Temperature measurement */													/* Ch  Trigger	Vref	Description */
uint16 const SBASE_TEMP[2] = { ADC_TJ, ADC_EOT};								/*  1	F/W		2.5V	Internal Temperature Sensor */

/* Supply voltage measurement */												/* Ch  Trigger	Vref	Description */
uint16 const SBASE_SUPPLYVOLT[2] = { ADC_VS, ADC_EOT};							/*  0	F/W		2.5V	Voltage Vs */
uint16 const SBASE_MOTORVOLT[2] = { ADC_VSM, ADC_EOT};							/* 14	F/W		2.5V	Voltage Vsm */

/* Motor-driver current measurement */											/* Ch  Trigger	Vref	Description */
uint16 const SBASE_CURRENT[2] = { ADC_MCUR, ADC_EOT};							/* 29   F/W		2.5V	Filtered Current */

/* Stepper-mode
 * ADC-duration: 1 x Motor PWM period
 */
#if (_SUPPORT_PWM_MODE == BIPOLAR_PWM_DOUBLE_MIRROR)
#if _SUPPORT_PHASE_SHORT_DET
uint16 const SBASE_INIT_4PH_U[6] =												/* ADC Automated measurements */
{																				/* Ch  Trigger	Vref	Description */
	(ADC_MCUR | ADC_HW_TRIGGER_PWM1_CMP),										/* 13	 25%	2.5V	Unfiltered Current */
	(ADC_PHU  | ADC_HW_TRIGGER_PWM2_CMP),										/*  9	 44%	2.5V	Voltage on U Driver output (divided by 14) */
	(ADC_VS   | ADC_HW_TRIGGER_PWM3_CMP),										/*  0	 62%	2.5V	Voltage Vs-filtered (divided by 14) */
	(ADC_TJ   | ADC_HW_TRIGGER_PWM4_CMP),										/*  1	 81%	2.5V	Internal Temperature Sensor */
	(ADC_VSM  | ADC_HW_TRIGGER_PWM1_CNT),										/* 14	100%	2.5V	Voltage Vsm (divided by 14) */
	ADC_EOT																		/* End-of-table marker */
};
uint16 const SBASE_INIT_4PH_V[6] =												/* ADC Automated measurements */
{																				/* Ch  Trigger	Vref	Description */
	(ADC_MCUR | ADC_HW_TRIGGER_PWM1_CMP),										/* 13	 25%	2.5V	Unfiltered Current */
	(ADC_PHV | ADC_HW_TRIGGER_PWM2_CMP),										/* 10	 44%	2.5V	Voltage on V Driver output (divided by 14) */
	(ADC_VS   | ADC_HW_TRIGGER_PWM3_CMP),										/*  0	 62%	2.5V	Voltage Vs-filtered (divided by 14) */
	(ADC_TJ   | ADC_HW_TRIGGER_PWM4_CMP),										/*  1	 81%	2.5V	Internal Temperature Sensor */
	(ADC_VSM  | ADC_HW_TRIGGER_PWM1_CNT),										/* 14	100%	2.5V	Voltage Vsm (divided by 14) */
	ADC_EOT																		/* End-of-table marker */
};
uint16 const SBASE_INIT_4PH_W[6] =												/* ADC Automated measurements */
{																				/* Ch  Trigger	Vref	Description */
	(ADC_MCUR | ADC_HW_TRIGGER_PWM1_CMP),										/* 13	 25%	2.5V	Unfiltered Current */
	(ADC_PHW  | ADC_HW_TRIGGER_PWM2_CMP),										/* 11	 44%	2.5V	Voltage on W Driver output (divided by 14) */
	(ADC_VS   | ADC_HW_TRIGGER_PWM3_CMP),										/*  0	 62%	2.5V	Voltage Vs-filtered (divided by 14) */
	(ADC_TJ   | ADC_HW_TRIGGER_PWM4_CMP),										/*  1	 81%	2.5V	Internal Temperature Sensor */
	(ADC_VSM  | ADC_HW_TRIGGER_PWM1_CNT),										/* 14	100%	2.5V	Voltage Vsm (divided by 14) */
	ADC_EOT																		/* End-of-table marker */
};
uint16 const SBASE_INIT_4PH_T[6] =												/* ADC Automated measurements */
{																				/* Ch  Trigger	Vref	Description */
	(ADC_MCUR | ADC_HW_TRIGGER_PWM1_CMP),										/* 13	 25%	2.5V	Unfiltered Current */
	(ADC_PHT  | ADC_HW_TRIGGER_PWM2_CMP),										/* 25	 44%	2.5V	Voltage on T Driver output (divided by 14) */
	(ADC_VS   | ADC_HW_TRIGGER_PWM3_CMP),										/*  0	 62%	2.5V	Voltage Vs-filtered (divided by 14) */
	(ADC_TJ   | ADC_HW_TRIGGER_PWM4_CMP)										/*  1	 81%	2.5V	Internal Temperature Sensor */
	(ADC_VSM  | ADC_HW_TRIGGER_PWM1_CNT),										/* 14	100%	2.5V	Voltage Vsm (divided by 14) */
	ADC_EOT																		/* End-of-table marker */
};
#else  /* _SUPPORT_PHASE_SHORT_DET */
uint16 const SBASE_INIT_4PH[5] =												/* ADC Automated measurements */
{																				/* Ch  Trigger	Vref	Description */
	(ADC_MCUR | ADC_HW_TRIGGER_PWM1_CMP),										/* 13	 25%	2.5V	Unfiltered Current */
	(ADC_VS   | ADC_HW_TRIGGER_PWM2_CMP),										/*  0	 50%	2.5V	Voltage VS-filtered (divided by 14) */
	(ADC_TJ   | ADC_HW_TRIGGER_PWM3_CMP),										/*  1	 75%	2.5V	Internal Temperature Sensor */
	(ADC_VSM  | ADC_HW_TRIGGER_PWM1_CNT),										/* 14	100%	2.5V	Vsm (divided by 14) */
	ADC_EOT																		/* End-of-table marker */
};
#endif /* _SUPPORT_PHASE_SHORT_DET */
#endif /* (_SUPPORT_PWM_MODE == BIPOLAR_PWM_DOUBLE_MIRROR) */

#if (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRROR_VSM)
#if _SUPPORT_PHASE_SHORT_DET
uint16 const SBASE_INIT_4PH_U[6] =												/* ADC Automated measurements */
{																				/* Ch  Trigger	Vref	Description */
	(ADC_PHU  | ADC_HW_TRIGGER_PWM1_CMP),										/*  9	 20%	2.5V	Voltage on U Driver output (divided by 14) */
	(ADC_VS   | ADC_HW_TRIGGER_PWM2_CMP),										/*  0	 40%	2.5V	Voltage Vs-filtered (divided by 14) */
	(ADC_TJ   | ADC_HW_TRIGGER_PWM3_CMP),										/*  1	 60%	2.5V	Internal Temperature Sensor */
	(ADC_VSM  | ADC_HW_TRIGGER_PWM4_CMP),										/* 14	 80%	2.5V	Voltage Vsm (divided by 14) */
	(ADC_MCUR | ADC_HW_TRIGGER_PWM1_CNT),										/* 13	100%	2.5V	Unfiltered Current */
	ADC_EOT																		/* End-of-table marker */
};
uint16 const SBASE_INIT_4PH_V[6] =												/* ADC Automated measurements */
{																				/* Ch  Trigger	Vref	Description */
	(ADC_PHV  | ADC_HW_TRIGGER_PWM1_CMP),										/* 10	 20%	2.5V	Voltage on V Driver output (divided by 14) */
	(ADC_VS   | ADC_HW_TRIGGER_PWM2_CMP),										/*  0	 40%	2.5V	Voltage Vs-filtered (divided by 14) */
	(ADC_TJ   | ADC_HW_TRIGGER_PWM3_CMP),										/*  1	 60%	2.5V	Internal Temperature Sensor */
	(ADC_VSM  | ADC_HW_TRIGGER_PWM4_CMP),										/* 14	 80%	2.5V	Voltage Vsm (divided by 14) */
	(ADC_MCUR | ADC_HW_TRIGGER_PWM1_CNT),										/* 13	 100%	2.5V	Unfiltered Current */
	ADC_EOT																		/* End-of-table marker */
};
uint16 const SBASE_INIT_4PH_W[6] =												/* ADC Automated measurements */
{																				/* Ch  Trigger	Vref	Description */
	(ADC_PHW  | ADC_HW_TRIGGER_PWM1_CMP),										/* 11	 20%	2.5V	Voltage on W Driver output (divided by 14) */
	(ADC_VS   | ADC_HW_TRIGGER_PWM2_CMP),										/*  0	 40%	2.5V	Voltage Vs-filtered (divided by 14) */
	(ADC_TJ   | ADC_HW_TRIGGER_PWM3_CMP),										/*  1	 60%	2.5V	Internal Temperature Sensor */
	(ADC_VSM  | ADC_HW_TRIGGER_PWM4_CMP),										/* 14	 80%	2.5V	Voltage Vsm (divided by 14) */
	(ADC_MCUR | ADC_HW_TRIGGER_PWM1_CNT),										/* 13	 100%	2.5V	Unfiltered Current */
	ADC_EOT																		/* End-of-table marker */
};
uint16 const SBASE_INIT_4PH_T[6] =												/* ADC Automated measurements */
{																				/* Ch  Trigger	Vref	Description */
	(ADC_PHT  | ADC_HW_TRIGGER_PWM1_CMP),										/* 25	 20%	2.5V	Voltage on T Driver output (divided by 14) */
	(ADC_VS   | ADC_HW_TRIGGER_PWM2_CMP),										/*  0	 40%	2.5V	Voltage Vs-filtered (divided by 14) */
	(ADC_TJ   | ADC_HW_TRIGGER_PWM3_CMP),										/*  1	 60%	2.5V	Internal Temperature Sensor */
	(ADC_VSM  | ADC_HW_TRIGGER_PWM4_CMP),										/* 14	 80%	2.5V	Voltage Vsm (divided by 14) */
	(ADC_MCUR | ADC_HW_TRIGGER_PWM1_CNT),										/* 13	 100%	2.5V	Unfiltered Current */
	ADC_EOT																		/* End-of-table marker */
};
#else  /* _SUPPORT_PHASE_SHORT_DET */
uint16 const SBASE_INIT_4PH[5] =												/* ADC Automated measurements */
{																				/* Ch  Trigger	Vref	Description */
	(ADC_VS   | ADC_HW_TRIGGER_PWM1_CMP),										/*  0	 25%	2.5V	Voltage Vs-filtered (divided by 14) */
	(ADC_TJ   | ADC_HW_TRIGGER_PWM2_CMP),										/*  1	 50%	2.5V	Internal Temperature Sensor */
	(ADC_VSM  | ADC_HW_TRIGGER_PWM3_CMP),										/* 14	 75%	2.5V	Voltage Vsm (divided by 14) */
	(ADC_MCUR | ADC_HW_TRIGGER_PWM1_CNT),										/* 13	100%	2.5V	Unfiltered Current */
	ADC_EOT																		/* End-of-table marker */
};
#endif /* _SUPPORT_PHASE_SHORT_DET */
#endif /* (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRROR_VSM) */

#if (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRROR_GND)
#if _SUPPORT_PHASE_SHORT_DET
uint16 const SBASE_INIT_4PH_U[6] =												/* ADC Automated measurements */
{																				/* Ch  Trigger	Vref	Description */
	(ADC_VS   | ADC_HW_TRIGGER_PWM1_CMP),										/*  0	 17%	2.5V	Voltage Vs-filtered (divided by 14) */
	(ADC_PHU  | ADC_HW_TRIGGER_PWM2_CMP),										/*  9	 33%	2.5V	Voltage on U Driver output (divided by 14) */
	(ADC_MCUR | ADC_HW_TRIGGER_PWM3_CMP),										/* 13	 50%	2.5V	Unfiltered Current */
	(ADC_VSM  | ADC_HW_TRIGGER_PWM4_CMP),										/* 14	 67%	2.5V	Voltage Vsm (divided by 14) */
	(ADC_TJ   | ADC_HW_TRIGGER_PWM1_CNT),										/*  1	100%	2.5V	Internal Temperature Sensor */
	ADC_EOT																		/* End-of-table marker */
};
uint16 const SBASE_INIT_4PH_V[6] =												/* ADC Automated measurements */
{																				/* Ch  Trigger	Vref	Description */
	(ADC_VS   | ADC_HW_TRIGGER_PWM1_CMP),										/*  0	 17%	2.5V	Voltage Vs-filtered (divided by 14) */
	(ADC_PHV  | ADC_HW_TRIGGER_PWM2_CMP),										/* 10	 33%	2.5V	Voltage on V Driver output (divided by 14) */
	(ADC_MCUR | ADC_HW_TRIGGER_PWM3_CMP),										/* 13	 50%	2.5V	Unfiltered Current */
	(ADC_VSM  | ADC_HW_TRIGGER_PWM4_CMP),										/* 14	 67%	2.5V	Voltage Vsm (divided by 14) */
	(ADC_TJ   | ADC_HW_TRIGGER_PWM1_CNT),										/*  1	100%	2.5V	Internal Temperature Sensor */
	ADC_EOT																		/* End-of-table marker */
};
uint16 const SBASE_INIT_4PH_W[6] =												/* ADC Automated measurements */
{																				/* Ch  Trigger	Vref	Description */
	(ADC_VS   | ADC_HW_TRIGGER_PWM1_CMP),										/*  0	 17%	2.5V	Voltage Vs-filtered (divided by 14) */
	(ADC_PHW  | ADC_HW_TRIGGER_PWM2_CMP),										/* 11	 33%	2.5V	Voltage on W Driver output (divided by 14) */
	(ADC_MCUR | ADC_HW_TRIGGER_PWM3_CMP),										/* 13	 50%	2.5V	Unfiltered Current */
	(ADC_VSM  | ADC_HW_TRIGGER_PWM4_CMP),										/* 14	 67%	2.5V	Voltage Vsm (divided by 14) */
	(ADC_TJ   | ADC_HW_TRIGGER_PWM1_CNT),										/*  1	100%	2.5V	Internal Temperature Sensor */
	ADC_EOT																		/* End-of-table marker */
};
uint16 const SBASE_INIT_4PH_T[6] =												/* ADC Automated measurements */
{																				/* Ch  Trigger	Vref	Description */
	(ADC_VS   | ADC_HW_TRIGGER_PWM1_CMP),										/*  0	 17%	2.5V	Voltage Vs-filtered (divided by 14) */
	(ADC_PHT  | ADC_HW_TRIGGER_PWM2_CMP),										/* 25	 33%	2.5V	Voltage on T Driver output (divided by 14) */
	(ADC_CH13 | ADC_HW_TRIGGER_PWM3_CMP),										/* 13	 50%	2.5V	Unfiltered Current */
	(ADC_VSM  | ADC_HW_TRIGGER_PWM4_CMP),										/* 14	 67%	2.5V	Voltage Vsm (divided by 14) */
	(ADC_TJ   | ADC_HW_TRIGGER_PWM1_CNT),										/*  1	100%	2.5V	Internal Temperature Sensor */
	ADC_EOT																		/* End-of-table marker */
};
#else  /* _SUPPORT_PHASE_SHORT_DET */
uint16 const SBASE_INIT_4PH[5] =												/* ADC Automated measurements */
{																				/* Ch  Trigger	Vref	Description */
	(ADC_VS   | ADC_HW_TRIGGER_PWM1_CMP),										/*  0	 25%	2.5V	Voltage Vs-filtered (divided by 14) */
	(ADC_MCUR | ADC_HW_TRIGGER_PWM2_CMP),										/* 13	 50%	2.5V	Unfiltered Current */
	(ADC_VSM  | ADC_HW_TRIGGER_PWM3_CMP),										/* 14	 75%	2.5V	Voltage Vsm (divided by 14) */
	(ADC_TJ   | ADC_HW_TRIGGER_PWM1_CNT),										/*  1	100%	2.5V	Internal Temperature Sensor */
	ADC_EOT																		/* End-of-table marker */
};
#endif /* _SUPPORT_PHASE_SHORT_DET */
#endif /* (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRROR_GND) */

#if (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_VSM) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_GND) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRRORSPECIAL)
#if _SUPPORT_PHASE_SHORT_DET
uint16 const SBASE_INIT_4PH_U[7] =												/* ADC Automated measurements */
{																				/* Ch  Trigger	Vref	Description */
	(ADC_TJ   | ADC_HW_TRIGGER_PWM1_CMP),										/*  1	 17%	2.5V	Internal Temperature Sensor */
	(ADC_VS   | ADC_HW_TRIGGER_PWM2_CMP),										/*  0	 33%	2.5V	Voltage Vs-filtered (divided by 14) */
	(ADC_MCUR | ADC_HW_TRIGGER_PWM3_CMP),										/* 13	 50%	2.5V	Unfiltered Current */
	(ADC_PHU  | ADC_HW_TRIGGER_PWM4_CMP),										/*  9	 67%	2.5V	Voltage on U Driver output (divided by 14) */
	(ADC_VSM  | ADC_HW_TRIGGER_PWM5_CMP),										/* 14	 83%	2.5V	Voltage Vsm (divided by 14) */
	(ADC_MCUR | ADC_HW_TRIGGER_PWM1_CNT),										/* 13	100%	2.5V	Unfiltered Current */
	ADC_EOT																		/* End-of-table marker */
};
uint16 const SBASE_INIT_4PH_V[7] =												/* ADC Automated measurements */
{																				/* Ch  Trigger	Vref	Description */
	(ADC_TJ   | ADC_HW_TRIGGER_PWM1_CMP),										/*  1	 17%	2.5V	Internal Temperature Sensor */
	(ADC_VS   | ADC_HW_TRIGGER_PWM2_CMP),										/*  0	 33%	2.5V	Voltage Vs-filtered (divided by 14) */
	(ADC_MCUR | ADC_HW_TRIGGER_PWM3_CMP),										/* 13	 50%	2.5V	Unfiltered Current */
	(ADC_PHV  | ADC_HW_TRIGGER_PWM4_CMP),										/* 10	 67%	2.5V	Voltage on V Driver output (divided by 14) */
	(ADC_VSM  | ADC_HW_TRIGGER_PWM5_CMP),										/* 14	 83%	2.5V	Voltage Vsm (divided by 14) */
	(ADC_MCUR | ADC_HW_TRIGGER_PWM1_CNT),										/* 13	100%	2.5V	Unfiltered Current */
	ADC_EOT																		/* End-of-table marker */
};
uint16 const SBASE_INIT_4PH_W[7] =												/* ADC Automated measurements */
{																				/* Ch  Trigger	Vref	Description */
	(ADC_TJ   | ADC_HW_TRIGGER_PWM1_CMP),										/*  1	 17%	2.5V	Internal Temperature Sensor */
	(ADC_VS   | ADC_HW_TRIGGER_PWM2_CMP),										/*  0	 33%	2.5V	Voltage Vs-filtered (divided by 14) */
	(ADC_MCUR | ADC_HW_TRIGGER_PWM3_CMP),										/* 13	 50%	2.5V	Unfiltered Current */
	(ADC_PHW  | ADC_HW_TRIGGER_PWM4_CMP),										/* 11	 67%	2.5V	Voltage on W Driver output (divided by 14) */
	(ADC_VSM  | ADC_HW_TRIGGER_PWM5_CMP),										/* 14	 83%	2.5V	Voltage Vsm (divided by 14) */
	(ADC_MCUR | ADC_HW_TRIGGER_PWM1_CNT),										/* 13	100%	2.5V	Unfiltered Current */
	ADC_EOT																		/* End-of-table marker */
};
uint16 const SBASE_INIT_4PH_T[7] =												/* ADC Automated measurements */
{																				/* Ch  Trigger	Vref	Description */
	(ADC_TJ   | ADC_HW_TRIGGER_PWM1_CMP),										/*  1	 17%	2.5V	Internal Temperature Sensor */
	(ADC_VS   | ADC_HW_TRIGGER_PWM2_CMP),										/*  0	 33%	2.5V	Voltage Vs-filtered (divided by 14) */
	(ADC_MCUR | ADC_HW_TRIGGER_PWM3_CMP),										/* 13	 50%	2.5V	Unfiltered Current */
	(ADC_PHT  | ADC_HW_TRIGGER_PWM4_CMP),										/* 25	 67%	2.5V	Voltage on T Driver output (divided by 14) */
	(ADC_VSM  | ADC_HW_TRIGGER_PWM5_CMP),										/* 14	 83%	2.5V	Voltage Vsm (divided by 14) */
	(ADC_MCUR | ADC_HW_TRIGGER_PWM1_CNT),										/* 13	100%	2.5V	Unfiltered Current */
	ADC_EOT																		/* End-of-table marker */
};
#else  /* _SUPPORT_PHASE_SHORT_DET */
uint16 const SBASE_INIT_4PH[6] =												/* ADC Automated measurements */
{																				/* Ch  Trigger	Vref	Description */
	(ADC_TJ   | ADC_HW_TRIGGER_PWM1_CMP),										/*  1	 17%	2.5V	Internal Temperature Sensor */
	(ADC_VS   | ADC_HW_TRIGGER_PWM2_CMP),										/*  0	 33%	2.5V	Voltage Vs-filtered (divided by 14) */
	(ADC_MCUR | ADC_HW_TRIGGER_PWM3_CMP),										/* 13	 50%	2.5V	Unfiltered Current */
	(ADC_VSM  | ADC_HW_TRIGGER_PWM4_CMP),										/* 14	 75%	2.5V	Voltage Vsm (divided by 14) */
	(ADC_MCUR | ADC_HW_TRIGGER_PWM1_CNT),										/* 13	100%	2.5V	Unfiltered Current */
	ADC_EOT																		/* End-of-table marker */
};
#endif /* _SUPPORT_PHASE_SHORT_DET */
#endif /* (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_VSM) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_GND) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRRORSPECIAL) */

#if (_SUPPORT_MOTOR_SELFTEST != FALSE)
uint16 const tAdcSelfTest4A[12] =
{
	(ADC_TJ   | ADC_HW_TRIGGER_PWM1_CNT),										/*  1   100%	2.5V	Temperature */
	(ADC_VPHU | ADC_HW_TRIGGER_PWM2_CMP),										/*  9	 50%	2.5V	Phase-U voltage (divided by 14) */
	(ADC_VPHU | ADC_HW_TRIGGER_PWM1_CNT),										/*  9	100%	2.5V	Phase-U voltage (divided by 14) */
	(ADC_VPHV | ADC_HW_TRIGGER_PWM2_CMP),										/* 10	 50%	2.5V	Phase-V voltage (divided by 14) */
	(ADC_VPHV | ADC_HW_TRIGGER_PWM1_CNT),										/* 10	100%	2.5V	Phase-V voltage (divided by 14) */
	(ADC_VPHW | ADC_HW_TRIGGER_PWM2_CMP),										/* 11	 50%	2.5V	Phase-W voltage (divided by 14) */
	(ADC_VPHW | ADC_HW_TRIGGER_PWM1_CNT),										/* 11	100%	2.5V	Phase-W voltage (divided by 14) */
	(ADC_VPHT | ADC_HW_TRIGGER_PWM2_CMP),										/* 25	 50%	2.5V	Phase-T voltage (divided by 14) */
	(ADC_VPHT | ADC_HW_TRIGGER_PWM1_CNT),										/* 25	100%	2.5V	Phase-T voltage (divided by 14) */
	(ADC_VS   | ADC_HW_TRIGGER_PWM2_CMP),										/*  0	 50%	2.5V	Voltage VS (divided by 14) */
	(ADC_MCUR | ADC_HW_TRIGGER_PWM1_CNT),										/* 13	100%	2.5V	Unfiltered Current */
	ADC_EOT																		/* End-of-table marker */
};
uint16 const tAdcSelfTest4B[12] =
{
	(ADC_TJ   | ADC_HW_TRIGGER_PWM1_CNT),										/*  1   100%	2.5V	Temperature */
	(ADC_VPHU | ADC_HW_TRIGGER_PWM2_CMP),										/*  9	 50%	2.5V	Phase-U voltage (divided by 14) */
	(ADC_VPHU | ADC_HW_TRIGGER_PWM1_CNT),										/*  9	100%	2.5V	Phase-U voltage (divided by 14) */
	(ADC_VPHV | ADC_HW_TRIGGER_PWM2_CMP),										/* 10	 50%	2.5V	Phase-V voltage (divided by 14) */
	(ADC_VPHV | ADC_HW_TRIGGER_PWM1_CNT),										/* 10	100%	2.5V	Phase-V voltage (divided by 14) */
	(ADC_VPHW | ADC_HW_TRIGGER_PWM2_CMP),										/* 11	 50%	2.5V	Phase-W voltage (divided by 14) */
	(ADC_VPHW | ADC_HW_TRIGGER_PWM1_CNT),										/* 11	100%	2.5V	Phase-W voltage (divided by 14) */
	(ADC_VPHT | ADC_HW_TRIGGER_PWM2_CMP),										/* 25	 50%	2.5V	Phase-T voltage (divided by 14) */
	(ADC_VPHT | ADC_HW_TRIGGER_PWM1_CNT),										/* 25	100%	2.5V	Phase-T voltage (divided by 14) */
	(ADC_MCUR | ADC_HW_TRIGGER_PWM2_CMP),										/* 13	 50%	2.5V	Unfiltered Current */
	(ADC_VS   | ADC_HW_TRIGGER_PWM1_CNT),										/*  0	100%	2.5V	Voltage VS (divided by 14) */
	ADC_EOT																		/* End-of-table marker */
};
#endif /* (_SUPPORT_MOTOR_SELFTEST != FALSE) */

/* ****************************************************************************	*
 * ADC_StartSoftTrig()
 *
 * Start ADC measurement using Software trigger.
 * ****************************************************************************	*/
void ADC_StartSoftTrig( void)
{
	PEND = CLR_ADC_IT;
	ADC_CTRL = ADC_START;
	if ( l_u8AdcPowerOff != FALSE )
	{
		NopDelay( DELAY_50us); /*lint !e522 */
	}
	l_u8AdcPowerOff = FALSE;
	NopDelay( ADC_SETTING); /*lint !e522 */
	ADC_CTRL = (ADC_START | ADC_SOFT_TRIG);										/* Single shot */
	while ( (ADC_CTRL & ADC_START) != 0U ) {}									/* Wait for ADC result */
} /* End of ADC_StartSoftTrig() */

/* ****************************************************************************	*
 * ADC_Init()
 *
 * Measure Zero-current offset
 * ADC ISR priority: 3
 * ****************************************************************************	*/
void ADC_Init( void)
{
	uint16 volatile u16VDDA;
	uint16 volatile u16ZCO;

	ADC_Stop();																	/* Clear the ADC control register */

	/* VDDA and VDDD check (MMP170222-4) */
	ADC_SBASE = (uint16) SBASE_VDDA;
	ADC_DBASE = (uint16) ((uint16 *) &u16VDDA);
	ADC_StartSoftTrig();
	if ( (u16VDDA < C_MIN_VDDA) || (u16VDDA > C_MAX_VDDA) )
	{
		/* TODO[MMP]: What to do? E.g. enter SLEEP (below error is lost) */
		g_e8ErrorElectric = (uint8) C_ERR_ELECTRIC_PERM;
#if _SUPPORT_LOG_ERRORS
		SetLastError( (uint8) C_ERR_VDDA);
#endif /* _SUPPORT_LOG_ERRORS */
	}

	ADC_SBASE = (uint16) SBASE_VDDD;
	ADC_DBASE = (uint16) ((uint16 *) &u16ZCO);
	ADC_StartSoftTrig();
	if ( (u16ZCO < C_MIN_VDDD) || (u16ZCO > C_MAX_VDDD) )
	{
		/* TODO[MMP]: What to do? E.g. enter SLEEP (below error is lost) */
		g_e8ErrorElectric = (uint8) C_ERR_ELECTRIC_PERM;
#if _SUPPORT_LOG_ERRORS
		SetLastError( (uint8) C_ERR_VDDD);
#endif /* _SUPPORT_LOG_ERRORS */
	}

	/* I/O Output check (MMP170302-1) */
	if ( (ANA_OUTF & (IO5_ENA | IO4_ENA | IO3_ENA | IO2_ENA | IO1_ENA | IO0_ENA)) != 0U )
	{
		/* At least one or more I/O's are defined as output */
		uint16 u16Idx;
		for ( u16Idx = 0U; u16Idx < 6; u16Idx++ )
		{
			uint16 u16Mask = (IO0_ENA << u16Idx);
			if ( (ANA_OUTF & u16Mask) != 0U )
			{
				uint16 u16OrgOutState = ANA_OUTN;

				/* Check high level; May cause VDDA overload */
				ANA_OUTN = u16OrgOutState | u16Mask;
				ADC_SBASE = (uint16) &SBASE_VIO[u16Idx][0];
				ADC_DBASE = (uint16) ((uint16 *) &u16ZCO);
				ADC_StartSoftTrig();
				ANA_OUTN = u16OrgOutState;
				if ( u16ZCO < C_ADC_MAX )
				{
					/* ADC Vref=2.5V, I/O-output voltage is 3.3V */
					g_e8ErrorElectric = (uint8) C_ERR_ELECTRIC_PERM;
					ANA_OUTF &= ~u16Mask;										/* Disable I/O Output-mode */
#if _SUPPORT_LOG_ERRORS
					SetLastError( (uint8) (C_ERR_VIO_0 + u16Idx));
#endif /* _SUPPORT_LOG_ERRORS */
				}
				else
				{
					/* Check low level */
					ANA_OUTN = u16OrgOutState & ~u16Mask;
					ADC_SBASE = (uint16) &SBASE_VIO[u16Idx][0];
					ADC_DBASE = (uint16) ((uint16 *) &u16ZCO);
					ADC_StartSoftTrig();
					ANA_OUTN = u16OrgOutState;
					if ( (u16ZCO > C_MAX_IO_LOW) )
					{
						g_e8ErrorElectric = (uint8) C_ERR_ELECTRIC_PERM;
						ANA_OUTF &= ~u16Mask;										/* Disable I/O Output-mode */
#if _SUPPORT_LOG_ERRORS
						SetLastError( (uint8) (C_ERR_VIO_0 + u16Idx));
#endif /* _SUPPORT_LOG_ERRORS */
					}
				}
			}
		}
	}

	{
		uint16 u16DrvCfg = DRVCFG;												/* Save driver state (MMP170405-1 - Begin) */
		DRVCFG &= ~DIS_DRV;														/* Enable driver to allow current measurement */
		ADC_SBASE = (uint16) SBASE_CURROFF;
		ADC_DBASE = (uint16) &u16ZCO;
		ADC_StartSoftTrig();
		l_u16CurrentZeroOffset = /* lint -e{530} */ u16ZCO;
		ADC_CTRL = (ADC_START | ADC_SOFT_TRIG);									/* Single shot */
		while ( (ADC_CTRL & ADC_START) != 0U) {}								/* Wait for ADC result */
		l_u16CurrentZeroOffset = (l_u16CurrentZeroOffset + u16ZCO) >> 1;
		DRVCFG = u16DrvCfg;														/* Restore Driver state (MMP170405-1 - End) */
	}

	g_u16MCurrgain = EE_GMCURR_NEW;												/* MMP160616-1 */

#ifdef SUPPORT_CALIBRATED_ZERO_CURRENT
	/* Validate zero-offset current measurement against calibration */
	uint16 u16Delta;
	if ( l_u16CurrentZeroOffset > EE_OMCURR )
	{
		u16Delta = l_u16CurrentZeroOffset - EE_OMCURR;
	}
	else
	{
		u16Delta = EE_OMCURR - l_u16CurrentZeroOffset;
	}
	if ( u16Delta > 6U )														/* Need to define this value: 6 LSB's is approximate 5mA */
	{
		l_u16CurrentZeroOffset = EE_OMCURR;										/* Take calibrated current offset, instead of measured offset */
	}
#endif /* SUPPORT_CALIBRATED_ZERO_CURRENT */

	PRIO = (PRIO & ~(3U << 2)) | ((5U - 3U) << 2);								/* ADC IRQ Priority: 5 (3..6) */

} /* End of ADC_Init() */

/* ****************************************************************************	*
 * ADC_Start()
 *
 * ADC ISR priority: 3
 * ****************************************************************************	*/
#if _SUPPORT_PHASE_SHORT_DET
void ADC_Start( uint16 u16Mode)
#else  /* _SUPPORT_PHASE_SHORT_DET */
void ADC_Start( void)
#endif /* _SUPPORT_PHASE_SHORT_DET */
{
	ADC_Stop();																	/* clear the ADC control register */
#if _SUPPORT_PHASE_SHORT_DET
	ADC_SetupShortDetection( u16Mode);
#else  /* _SUPPORT_PHASE_SHORT_DET */
	ADC_SBASE = (uint16) SBASE_INIT_4PH;
#endif /* _SUPPORT_PHASE_SHORT_DET */
	ADC_DBASE = (uint16) &g_AdcMotorRunStepper4;
	ADC_CTRL  = (ADC_LOOP | ADC_TRIG_SRC | ADC_SYNC_SOC);						/* Loop cycle of conversion is done */
	ADC_CTRL |= ADC_START;														/* Start ADC */
	if ( l_u8AdcPowerOff != FALSE )												/* Add delay */
	{
		NopDelay( DELAY_mPWM); /*lint !e522 */
	}
	l_u8AdcPowerOff = FALSE;
} /* End of ADC_Start() */

#if _SUPPORT_PHASE_SHORT_DET
/* ****************************************************************************	*
 * ADC_SetupShortDetection()
 *
 * Setup ADC for correct motor phase voltage measurement. The phase connected to Vsm.
 * ****************************************************************************	*/
void ADC_SetupShortDetection( uint16 u16Mode)
{
	if ( u16Mode == 0U )
	{
		/* Phase based on micro-step index */
		u16Mode = g_u16MicroStepIdx;
		if ( g_e8MotorDirectionCCW != (uint8) C_MOTOR_DIR_CW )
		{
			u16Mode--;
		}
#if _SUPPORT_DOUBLE_USTEP
		u16Mode = ((u16Mode + (1U << NVRAM_MICRO_STEPS)) >> (NVRAM_MICRO_STEPS + 1U));
#else  /* _SUPPORT_DOUBLE_USTEP */
		u16Mode = ((u16Mode + ((1U << NVRAM_MICRO_STEPS) >> 1)) >> NVRAM_MICRO_STEPS);
#endif /* _SUPPORT_DOUBLE_USTEP */
	}
	/* Phase based on u16Mode */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT)
	if ( u16Mode == 1U )
	{
		ADC_SBASE = (uint16) SBASE_INIT_4PH_U;									/* Check U-phase voltage */
	}
	else if ( u16Mode == 2U )
	{
#if (_SUPPORT_PWM_MODE != BIPOLAR_PWM_SINGLE_MIRRORSPECIAL)
		ADC_SBASE = (uint16) SBASE_INIT_4PH_T;									/* Check T-phase voltage */
#else  /* (_SUPPORT_PWM_MODE != BIPOLAR_PWM_SINGLE_MIRRORSPECIAL) */
		ADC_SBASE = (uint16) SBASE_INIT_4PH_V;									/* Check V-phase voltage */
#endif /* (_SUPPORT_PWM_MODE != BIPOLAR_PWM_SINGLE_MIRRORSPECIAL) */
	}
	else if ( u16Mode == 3U )
	{
		ADC_SBASE = (uint16) SBASE_INIT_4PH_W;									/* Check W-phase voltage */
	}
	else
	{
#if (_SUPPORT_PWM_MODE != BIPOLAR_PWM_SINGLE_MIRRORSPECIAL)
		ADC_SBASE = (uint16) SBASE_INIT_4PH_V;									/* Check V-phase voltage */
#else  /* (_SUPPORT_PWM_MODE != BIPOLAR_PWM_SINGLE_MIRRORSPECIAL) */
		ADC_SBASE = (uint16) SBASE_INIT_4PH_T;									/* Check T-phase voltage */
#endif /* (_SUPPORT_PWM_MODE != BIPOLAR_PWM_SINGLE_MIRRORSPECIAL) */
	}
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UW_VT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT)
	if ( u16Mode == 1U )
	{
#if (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRROR_GND)
		ADC_SBASE = (uint16) SBASE_INIT_4PH_W;									/* Check W-phase voltage */
#else  /* (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRROR_GND) */
		ADC_SBASE = (uint16) SBASE_INIT_4PH_T;									/* Check T-phase voltage */
#endif /* (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRROR_GND) */
	}
	else if ( u16Mode == 2U )
	{
#if (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRROR_GND) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRRORSPECIAL)
		ADC_SBASE = (uint16) SBASE_INIT_4PH_U;									/* Check U-phase voltage */
#else  /* (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRROR_GND) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRRORSPECIAL) */
		ADC_SBASE = (uint16) SBASE_INIT_4PH_V;									/* Check V-phase voltage */
#endif /* (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRROR_GND) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRRORSPECIAL) */
	}
	else if ( u16Mode == 3U )
	{
#if (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRROR_GND)
		ADC_SBASE = (uint16) SBASE_INIT_4PH_T;									/* Check T-phase voltage */
#else  /* (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRROR_GND) */
		ADC_SBASE = (uint16) SBASE_INIT_4PH_W;									/* Check W-phase voltage */
#endif /* (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRROR_GND) */
	}
	else
	{
#if (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRROR_GND) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRRORSPECIAL)
		ADC_SBASE = (uint16) SBASE_INIT_4PH_V;									/* Check V-phase voltage */
#else  /* (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRROR_GND) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRRORSPECIAL) */
		ADC_SBASE = (uint16) SBASE_INIT_4PH_U;									/* Check U-phase voltage */
#endif /* (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRROR_GND) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRRORSPECIAL) */
	}
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UV_WT) */
#if (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW)
	if ( u16Mode == 1U )
	{
		ADC_SBASE = (uint16) SBASE_INIT_4PH_V;									/* Check V-phase voltage */
	}
	else if ( u16Mode == 2U )
	{
#if (_SUPPORT_PWM_MODE != BIPOLAR_PWM_SINGLE_MIRRORSPECIAL)
		ADC_SBASE = (uint16) SBASE_INIT_4PH_T;									/* Check T-phase voltage */
#else  /* (_SUPPORT_PWM_MODE != BIPOLAR_PWM_SINGLE_MIRRORSPECIAL) */
		ADC_SBASE = (uint16) SBASE_INIT_4PH_U;									/* Check U-phase voltage */
#endif /* (_SUPPORT_PWM_MODE != BIPOLAR_PWM_SINGLE_MIRRORSPECIAL) */
	}
	else if ( u16Mode == 3U )
	{
		ADC_SBASE = (uint16) SBASE_INIT_4PH_W;									/* Check W-phase voltage */
	}
	else
	{
#if (_SUPPORT_PWM_MODE != BIPOLAR_PWM_SINGLE_MIRRORSPECIAL)
		ADC_SBASE = (uint16) SBASE_INIT_4PH_U;									/* Check U-phase voltage */
#else  /* (_SUPPORT_PWM_MODE != BIPOLAR_PWM_SINGLE_MIRRORSPECIAL) */
		ADC_SBASE = (uint16) SBASE_INIT_4PH_T;									/* Check T-phase voltage */
#endif /* (_SUPPORT_PWM_MODE != BIPOLAR_PWM_SINGLE_MIRRORSPECIAL) */
	}
#endif /* (_SUPPORT_BIPOLAR_MODE == BIPOLAR_MODE_UT_VW) */
} /* End of ADC_SetupShortDetection() */
#endif /* _SUPPORT_PHASE_SHORT_DET */

/* ****************************************************************************	*
 * ADC_Stop()
 *
 * Stop ADC (with waiting for pending ADC conversions to be finished)
 * ****************************************************************************	*/
void ADC_Stop(void)
{	
	if ( (ADC_CTRL & ADC_START) != 0U )											/* In case ADC is active, wait to finish it */
	{
		ADC_CTRL &= ~(ADC_LOOP | ADC_TRIG_SRC | ADC_SYNC_SOC);					/* Stop looping and HW-triggers */
		while ( (ADC_CTRL & ADC_START) != 0U )									/* As long as the ADC is active ... */
		{
			ADC_CTRL |= ADC_SOFT_TRIG;											/* ... Set S/W trigger */
			NopDelay( DELAY_7us); /*lint !e522 */
		}
	}
	ADC_CTRL = 0U;																/* Clear the ADC control register */
	BEGIN_CRITICAL_SECTION();
	MASK &= ~EN_ADC_IT;															/* Disable ADC Interrupt */
	END_CRITICAL_SECTION();
#if _SUPPORT_LIN_AA
	g_u8AdcIsrMode = C_ADC_ISR_NONE;
#endif /* _SUPPORT_LIN_AA */
} /* End of ADC_Stop() */


/* ****************************************************************************	*
 * ADC_PowerOff()
 *
 * Power-off ADC, by disabling reference voltage
 * ****************************************************************************	*/
void ADC_PowerOff( void)
{
	int16 u16Scratch;															/* MMP170404-2 */

	ADC_Stop();																	/* Stop ADC conversion, and disable ADC-IRQ */
	ADC_SBASE = (uint16) SBASE_VREF_OFF;
	ADC_DBASE = (uint16) &u16Scratch;											/* MMP170404-2 */
	PEND = CLR_ADC_IT;
	ADC_CTRL = ADC_START;
	ADC_Stop();
	l_u8AdcPowerOff = TRUE;
} /* End of ADC_PowerOff() */

/* ****************************************************************************	*
 * ADC_IT()
 *
 * ADC Interrupt Service Routine
 * In case no ADC_ISR action required, this ISR has 4us overhead (is approx: 9.5% at 24kHz PWM)
 * (push/pop + check for LIN-AA + Check BEMF ZC)
 * ****************************************************************************	*/
__interrupt__ void ADC_IT(void) 
{
#if _SUPPORT_LIN_AA
	if ( g_u8AdcIsrMode == C_ADC_ISR_LIN_AA ) 									/* LIN-AutoAddressing sequence */
	{
		AutoAddressingReadADCResult();											/* See MELEXIS doc */
	}
#endif /* _SUPPORT_LIN_AA */
} /* End of ADC_IT() */

/* ****************************************************************************	*
 * GetVsupply()
 *
 * Get Supply-voltage [10mV]
 * ****************************************************************************	*/
void GetVsupply( void)
{
	uint16 u16FilteredSupplyVoltage = g_AdcMotorRunStepper4.FilteredSupplyVoltage;
	g_i16SupplyVoltage = (int16) ((mulI32_I16byI16( (int16)(u16FilteredSupplyVoltage - EE_OADC), EE_GADC) + (C_GVOLTAGE_DIV/2)) / C_GVOLTAGE_DIV);
} /* End of GetVsupply() */

#if _SUPPORT_MLX_DEBUG_MODE
uint16 GetRawChipSupply( void)
{
	uint16 u16FilteredSupplyVoltage = g_AdcMotorRunStepper4.FilteredSupplyVoltage;
	return ( u16FilteredSupplyVoltage );
} /* End of GetRawChipSupply() */
#endif /* _SUPPORT_MLX_DEBUG_MODE */

/* ****************************************************************************	*
 * GetVsupplyMotor()
 *
 * Get Supply-voltage [10mV]
 * ****************************************************************************	*/
void GetVsupplyMotor( void)
{
	uint16 u16FilteredDriverVoltage = g_AdcMotorRunStepper4.FilteredDriverVoltage;
#if _SUPPORT_VSMFILTERED
	g_i16MotorVoltage = (int16) ((mulI32_I16byU16( (int16)(u16FilteredDriverVoltage - EE_OVOLTAGE), EE_GVOLTAGE) + (C_GVOLTAGE_DIV/2)) / C_GVOLTAGE_DIV);
#else  /* _SUPPORT_VSMFILTERED */
	g_i16MotorVoltage = (int16) ((mulI32_I16byU16( (int16)(u16FilteredDriverVoltage - EE_OADC), EE_GADC) + (C_GVOLTAGE_DIV/2)) / C_GVOLTAGE_DIV);
#endif /* _SUPPORT_VSMFILTERED */
} /* End of GetVsupplyMotor() */

#if _SUPPORT_PHASE_SHORT_DET
/* ****************************************************************************	*
 * GetPhaseMotor()
 *
 * Get Phase-voltage [10mV]
 * ****************************************************************************	*/
void GetPhaseMotor( void)
{
	uint16 u16UnfilteredPhaseVoltage = g_AdcMotorRunStepper4.FilteredPhaseVoltage;
	g_i16PhaseVoltage = mulI16_I16byI16RndDiv64( (int16) (u16UnfilteredPhaseVoltage - EE_OADC), EE_GADC);
} /* End of GetPhaseMotor() */
#endif /* _SUPPORT_PHASE_SHORT_DET */

/* ****************************************************************************	*
 * ResetChipTemperature()
 *
 * Reset Chip temperature
 * ****************************************************************************	*/
void ResetChipTemperature( void)
{
	g_AdcMotorRunStepper4.IntTemperatureSensor = 0U;

} /* End of ResetChipTemperature() */

/* ****************************************************************************	*
 * GetChipTemperature()
 *
 * Get Chip temperature [C]
 * ****************************************************************************	*/
void GetChipTemperature( uint16 u16Init)
{
	uint16 u16ChipTemperatureSensor = g_AdcMotorRunStepper4.IntTemperatureSensor;
	int16 i16ChipTemperature, i16ChipTempDelta;

#if _SUPPORT_TWO_LINE_TEMP_INTERPOLATION
	if ( u16ChipTemperatureSensor < EE_OTEMP )
	{
		/* Temperature above 35C */
		i16ChipTemperature = (int16) muldivU16_U16byU16byU16( (EE_OTEMP - u16ChipTemperatureSensor), (125U - 35U), (EE_OTEMP - EE_HIGHTEMP)) + EE_MIDTEMP;
	}
	else
	{
		/* Temperature below 35C */
		i16ChipTemperature = EE_MIDTEMP - (int16) muldivU16_U16byU16byU16( (u16ChipTemperatureSensor - EE_OTEMP), (uint16) (35 - (-40)), (EE_LOWTEMP - EE_OTEMP));
	}
#else  /* _SUPPORT_TWO_LINE_TEMP_INTERPOLATION */
	i16ChipTemperature = (mulI32_I16byI16( (EE_OTEMP - u16ChipTemperatureSensor), EE_GTEMP) / C_GTEMP_DIV) + EE_MIDTEMP;
#endif /* _SUPPORT_TWO_LINE_TEMP_INTERPOLATION */

	if ( u16Init == FALSE )
	{
		i16ChipTempDelta = i16ChipTemperature - g_i16ChipTemperature;			/* Delta-temperature = new-temperature - previous-temperature */
		if ( i16ChipTempDelta < 0 )
		{
			i16ChipTempDelta = -i16ChipTempDelta;								/* Absolute temperature change */
		}
		if ( (uint16) i16ChipTempDelta > C_TEMPERATURE_JUMP )					/* Temperature change small, then accept new temperature */
		{
			if ( i16ChipTemperature > g_i16ChipTemperature )					/* To great temperature change; Check temperature change "direction" */
			{
				i16ChipTemperature = g_i16ChipTemperature + 1;					/* Increase by one degree */
			}
			else
			{
				i16ChipTemperature = g_i16ChipTemperature - 1;					/* Decrease by one degree */
			}
		}
	}
	g_i16ChipTemperature = i16ChipTemperature;
} /* End of GetChipTemperature() */

#if _SUPPORT_MLX_DEBUG_MODE
uint16 GetRawTemperature( void)
{
	uint16 u16ChipTemperatureSensor = g_AdcMotorRunStepper4.IntTemperatureSensor;
	return ( u16ChipTemperatureSensor );
} /* End of GetRawTemperature() */
#endif /* _SUPPORT_MLX_DEBUG_MODE */

/* ****************************************************************************	*
 * GetRawMotorDriverCurrent()
 *
 * Get (raw) Motor Driver Current [ADC-LSB]
 * ****************************************************************************	*/
uint16 GetRawMotorDriverCurrent( void)
{
	uint16 u16Current = 0U;
#if ((_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_VSM) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_GND) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRRORSPECIAL))
	g_u16CurrentMotorCoilA = g_AdcMotorRunStepper4.UnfilteredDriverCurrent;
	if ( g_u16CurrentMotorCoilA > l_u16CurrentZeroOffset )
	{
		u16Current = g_u16CurrentMotorCoilA - (int16) l_u16CurrentZeroOffset;
	}
	g_u16CurrentMotorCoilB = g_AdcMotorRunStepper4.UnfilteredDriverCurrent2;
	if ( g_u16CurrentMotorCoilB > l_u16CurrentZeroOffset )
	{
		u16Current = u16Current + (g_u16CurrentMotorCoilB - l_u16CurrentZeroOffset);
	}
#else  /* ((_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_VSM) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_GND) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRRORSPECIAL)) */
	u16Current = (uint16) g_AdcMotorRunStepper4.UnfilteredDriverCurrent;
	if ( u16Current > l_u16CurrentZeroOffset )
	{
		u16Current = u16Current - (int16) l_u16CurrentZeroOffset;
	}
#endif /* ((_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_VSM) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_INDEPENDED_GND) || (_SUPPORT_PWM_MODE == BIPOLAR_PWM_SINGLE_MIRRORSPECIAL)) */
	return ( u16Current );
} /* End of GetRawMotorDriverCurrent() */

/* ****************************************************************************	*
 * GetMotorDriverCurrent()
 *
 * Get Motor Driver Current [mA]
 * ****************************************************************************	*/
int16 GetMotorDriverCurrent( void)
{
	uint16 u16Current = GetRawMotorDriverCurrent();
	u16Current = (uint16) ((mulU32_U16byU16( u16Current, g_u16MCurrgain) + (C_GMCURR_DIV / 2U)) / C_GMCURR_DIV);
	return ( (int16) u16Current);
} /* End of GetMotorDriverCurrent() */

/* ****************************************************************************	*
 * MeasureVsupplyAndTemperature()
 *
 * Measure Vbat and Temperature (single-shot, software triggered)
 * ****************************************************************************	*/
void MeasureVsupplyAndTemperature( void)
{	
	ADC_Stop();
	ADC_SBASE = (uint16) SBASE_MOTORVOLT;										/* switch ADC input source to Voltage */
	ADC_DBASE = (uint16) &g_AdcMotorRunStepper4.FilteredDriverVoltage;
	ADC_StartSoftTrig();

	ADC_SBASE = (uint16) SBASE_SUPPLYVOLT;										/* switch ADC input source to Voltage */
	ADC_DBASE = (uint16) &g_AdcMotorRunStepper4.FilteredSupplyVoltage;
	ADC_StartSoftTrig();

	ADC_SBASE = (uint16) SBASE_TEMP;											/* switch ADC input source to Temperature */
	ADC_DBASE = (uint16) &g_AdcMotorRunStepper4.IntTemperatureSensor;
	ADC_StartSoftTrig();
} /* End of MeasureVsupplyAndTemperature() */

/* ****************************************************************************	*
 * MeasureMotorCurrent()
 *
 * Measure Motor-current (filtered) (single-shot, software triggered)
 * ****************************************************************************	*/
void MeasureMotorCurrent( void)
{
	ADC_Stop();
	uint16 u16DrvCfg = DRVCFG;													/* MMP170405-1 - Begin */
	DRVCFG &= ~DIS_DRV;															/* Enable driver to allow current measurement */
	g_AdcMotorRunStepper4.UnfilteredDriverCurrent = l_u16CurrentZeroOffset;
	g_AdcMotorRunStepper4.UnfilteredDriverCurrent2 = l_u16CurrentZeroOffset;
	ADC_SBASE = (uint16) SBASE_CURRENT;											/* Switch ADC input source to Motor-driver current */
	ADC_DBASE = (uint16) &g_AdcMotorRunStepper4.UnfilteredDriverCurrent;
	ADC_StartSoftTrig();
	DRVCFG = u16DrvCfg;															/* MMP170405-1 - End */

} /* End of MeasureMotorCurrent() */

#if _SUPPORT_PHASE_SHORT_DET || (_SUPPORT_MOTOR_SELFTEST != FALSE)
/* ****************************************************************************	*
 * MeasurePhaseVoltage()
 *
 * Measure Phase voltage (single-shot, software triggered)
 * ****************************************************************************	*/
void MeasurePhaseVoltage( uint16 u16AdcSbase)
{
	uint16 u16PhaseVoltage;
	ADC_Stop();
	ADC_SBASE = (uint16) u16AdcSbase;											/* switch ADC input source to Motor-driver current */
	ADC_DBASE = (uint16) &u16PhaseVoltage;
	ADC_StartSoftTrig();
	g_i16PhaseVoltage = (int16) mulI16_I16byI16RndDiv64((int16)(u16PhaseVoltage - EE_OADC), EE_GADC);

} /* End of MeasurePhaseVoltage() */
#endif /* _SUPPORT_PHASE_SHORT_DET || (_SUPPORT_MOTOR_SELFTEST != FALSE) */

/* EOF */
