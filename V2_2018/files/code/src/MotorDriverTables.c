/*! ----------------------------------------------------------------------------
 * \file		MotorDriver.c
 * \brief		MLX81300 Motor driver tables
 *
 * \note		project MLX81300
 * 
 * \author 		Marcel Braat
 *   
 * \date 		2012-02-11
 *   
 * \version 	1.0 - preliminary
 *
 *
 * MELEXIS Microelectronic Integrated Systems
 * 
 * Copyright (C) 2012-2013 Melexis N.V.
 * The Software is being delivered 'AS IS' and Melexis, whether explicitly or 
 * implicitly, makes no warranty as to its Use or performance. 
 * The user accepts the Melexis Firmware License Agreement.  
 *
 * Melexis confidential & proprietary
 * ****************************************************************************	*
 * Resources:
 *
 * ****************************************************************************	*/

#include "Build.h"
#include "MotorDriver.h"
#include "MotorParams.h"
#include "MotorDriverTables.h"

#define   Q15(A)      (int16) ((A) * 32768)

/* Set-Test - Part A: Single FET ON */
uint8 const c_au8DrvCfgSelfTestA[8] =
{
	(DRV_CFG_T_TRISTATE | DRV_CFG_W_0 | DRV_CFG_V_TRISTATE | DRV_CFG_U_TRISTATE),	/* PhD = Z, PhC = L, PhB = Z, PhA = Z */
	(DRV_CFG_T_TRISTATE | DRV_CFG_W_1 | DRV_CFG_V_TRISTATE | DRV_CFG_U_TRISTATE),	/* PhD = Z, PhC = H, PhB = Z, PhA = Z */
	(DRV_CFG_T_TRISTATE | DRV_CFG_W_TRISTATE | DRV_CFG_V_0 | DRV_CFG_U_TRISTATE),	/* PhD = Z, PhC = Z, PhB = L, PhA = Z */
	(DRV_CFG_T_TRISTATE | DRV_CFG_W_TRISTATE | DRV_CFG_V_1 | DRV_CFG_U_TRISTATE),	/* PhD = Z, PhC = Z, PhB = H, PhA = Z */
	(DRV_CFG_T_TRISTATE | DRV_CFG_W_TRISTATE | DRV_CFG_V_TRISTATE | DRV_CFG_U_0),	/* PhD = Z, PhC = Z, PhB = Z, PhA = L */
	(DRV_CFG_T_TRISTATE | DRV_CFG_W_TRISTATE | DRV_CFG_V_TRISTATE | DRV_CFG_U_1),	/* PhD = Z, PhC = Z, PhB = Z, PhA = H */
	(DRV_CFG_T_0 | DRV_CFG_W_TRISTATE | DRV_CFG_V_TRISTATE | DRV_CFG_U_TRISTATE),	/* PhD = L, PhC = Z, PhB = Z, PhA = Z */
	(DRV_CFG_T_1 | DRV_CFG_W_TRISTATE | DRV_CFG_V_TRISTATE | DRV_CFG_U_TRISTATE)	/* PhD = H, PhC = Z, PhB = Z, PhA = Z */
};

uint16 const c_au16DrvAdcSelfTestA[4][2] =
{
	{(ADC_CH11 | ADC_REF_2_50_V), 0xFFFF},										/* Ch11: Voltage on W Driver output (divided by 14) */
	{(ADC_CH10 | ADC_REF_2_50_V), 0xFFFF},										/* Ch10: Voltage on V Driver output (divided by 14) */
	{(ADC_CH9 | ADC_REF_2_50_V), 0xFFFF},										/* Ch9 : Voltage on U Driver output (divided by 14) */
	{(ADC_CH25 | ADC_REF_2_50_V), 0xFFFF}										/* Ch29: Voltage on T Driver output (divided by 14) */
};

uint8 const c_au8DrvCfgSelfTestB4[10] =
{
	(DRV_CFG_T_TRISTATE | DRV_CFG_W_1 | DRV_CFG_V_TRISTATE | DRV_CFG_U_PWM),	/* PhD = Z, PhC = H, PhB = Z, PhA = PWM */
	(DRV_CFG_T_1 | DRV_CFG_W_TRISTATE | DRV_CFG_V_PWM | DRV_CFG_U_TRISTATE),	/* PhD = H, PhC = Z, PhB = PWM, PhA = Z */
	(DRV_CFG_T_TRISTATE | DRV_CFG_W_0 | DRV_CFG_V_TRISTATE | DRV_CFG_U_PWM),	/* PhD = Z, PhC = L, PhB = Z, PhA = PWM */
	(DRV_CFG_T_0 | DRV_CFG_W_TRISTATE | DRV_CFG_V_PWM | DRV_CFG_U_TRISTATE),	/* PhD = L, PhC = Z, PhB = PWM, PhA = Z */
	(DRV_CFG_T_TRISTATE | DRV_CFG_W_PWM | DRV_CFG_V_TRISTATE | DRV_CFG_U_1),	/* PhD = Z, PhC = PWM, PhB = Z, PhA = H */
	(DRV_CFG_T_PWM | DRV_CFG_W_TRISTATE | DRV_CFG_V_1 | DRV_CFG_U_TRISTATE),	/* PhD = PWM, PhC = Z, PhB = H, PhA = Z */
	(DRV_CFG_T_TRISTATE | DRV_CFG_W_PWM | DRV_CFG_V_TRISTATE | DRV_CFG_U_0),	/* PhD = Z, PhC = PWM, PhB = Z, PhA = L */
	(DRV_CFG_T_PWM | DRV_CFG_W_TRISTATE | DRV_CFG_V_0 | DRV_CFG_U_TRISTATE),	/* PhD = PWM, PhC = Z, PhB = L, PhA = Z */
	(DRV_CFG_T_1 | DRV_CFG_W_1 | DRV_CFG_V_PWM | DRV_CFG_U_PWM),
	(DRV_CFG_T_0 | DRV_CFG_W_1 | DRV_CFG_V_PWM | DRV_CFG_U_PWM)
};
																				/* 7.0Vsm */
#define SPACE_VECTOR_SINUS				TRUE									/* 27mA  */
#define SPACE_VECTOR_DOUBLE_SINUS		FALSE									/* 35mA  */
#define SPACE_VECTOR_TRIPLE_SINUS		FALSE									/* 41mA   21% extra vs. SPACE_VECTOR_SINUS */
#define SPACE_VECTOR_FIFTH_SINUS		FALSE									/* 43mA  4-5% extra vs. SPACE_VECTOR_TRIPPLE_SINUS */
#define SPACE_VECTOR_TRAPIZE			FALSE									/* 48mA  */
#define SPACE_VECTOR_BLOCK				FALSE									/* 58mA  */
/* *** 4-phase, 32-steps per rotation, sinus-waveform *** */
int16 const c_ai16MicroStepVector4PH[SZ_MICRO_VECTOR_TABLE_4PH] =
{
#if (_SUPPORT_DOUBLE_USTEP == FALSE)
#if SPACE_VECTOR_SINUS
	Q15(0.098017),	Q15(0.290285),	Q15(0.471397),	Q15(0.634393),				/* A[ 0: 3] */
	Q15(0.773010),	Q15(0.881921),	Q15(0.956940),	Q15(0.995185),				/* A[ 4: 7] */
	Q15(0.995185),	Q15(0.956940),	Q15(0.881921),	Q15(0.773010),				/* A[ 8:11], B[ 0: 3] */
	Q15(0.634393),	Q15(0.471397),	Q15(0.290285),	Q15(0.098017),				/* A[12:15], B[ 4: 7] */
	Q15(-0.098017),	Q15(-0.290285),	Q15(-0.471397),	Q15(-0.634393),				/* A[16:19], B[ 8:11] */
	Q15(-0.773010),	Q15(-0.881921),	Q15(-0.95694),	Q15(-0.995185),				/* A[20:23], B[12:15] */
	Q15(-0.995185),	Q15(-0.956940),	Q15(-0.881921),	Q15(-0.773010),				/* A[24:27], B[16:19] */
	Q15(-0.634393),	Q15(-0.471397),	Q15(-0.290285),	Q15(-0.098017),				/* A[28:31], B[20:23] */
	Q15(0.098017),	Q15(0.290285),	Q15(0.471397),	Q15(0.634393),				/*           B[24:27] */
	Q15(0.773010),	Q15(0.881921),	Q15(0.956940),	Q15(0.995185)				/*           B[28:31] */
#endif /* SPACE_VECTOR_SINUS */

#if SPACE_VECTOR_DOUBLE_SINUS
	Q15(0.139653),	Q15(0.407425),	Q15(0.641981),	Q15(0.825585),				/* A[ 0: 3] */
	Q15(0.946463),	Q15(0.999969),	Q15(0.988767),	Q15(0.922007),				/* A[ 4: 7] */
	Q15(0.922007),	Q15(0.988767),	Q15(0.999969),	Q15(0.946463),				/* A[ 8:11], B[ 0: 3] */
	Q15(0.825585),	Q15(0.641981),	Q15(0.407425),	Q15(0.139653),				/* A[12:15], B[ 4: 7] */
	Q15(-0.139653),	Q15(-0.407425),	Q15(-0.641981),	Q15(-0.825585),				/* A[16:19], B[ 8:11] */
	Q15(-0.946463),	Q15(-0.999969),	Q15(-0.988767),	Q15(-0.922007),				/* A[20:23], B[12:15] */
	Q15(-0.922007),	Q15(-0.988767),	Q15(-0.999969),	Q15(-0.946463),				/* A[24:27], B[16:19] */
	Q15(-0.825585),	Q15(-0.641981),	Q15(-0.407425),	Q15(-0.139653),				/* A[28:31], B[20:23] */
	Q15(0.139653),	Q15(0.407425),	Q15(0.641981),	Q15(0.825585),				/*           B[24:27] */
	Q15(0.946463),	Q15(0.999969),	Q15(0.988767),	Q15(0.922007)				/*           B[28:31] */
#endif /* SPACE_VECTOR_DOUBLE_SINUS */

#if SPACE_VECTOR_TRIPLE_SINUS
	Q15(0.179956),	Q15(0.512962),	Q15(0.773021),	Q15(0.934840),
	Q15(0.999969),	Q15(0.994267),	Q15(0.957075),	Q15(0.926791),
	Q15(0.926791),	Q15(0.957075),	Q15(0.994267),	Q15(0.999969),
	Q15(0.934840),	Q15(0.773021),	Q15(0.512962),	Q15(0.179956),
	Q15(-0.179956),	Q15(-0.512962),	Q15(-0.773021),	Q15(-0.934840),
	Q15(-1.000000),	Q15(-0.994267),	Q15(-0.957075),	Q15(-0.926791),
	Q15(-0.926791),	Q15(-0.957075),	Q15(-0.994267),	Q15(-1.000000),
	Q15(-0.934840),	Q15(-0.773021),	Q15(-0.512962),	Q15(-0.179956),
	Q15(0.179956),	Q15(0.512962),	Q15(0.773021),	Q15(0.934840),
	Q15(0.999969),	Q15(0.994267),	Q15(0.957075),	Q15(0.926791)
#endif /* SPACE_VECTOR_TRIPPLE_SINUS */

#if SPACE_VECTOR_FIFTH_SINUS
	Q15(0.206002),  Q15(0.572805),  Q15(0.827490),  Q15(0.957199),
	Q15(0.997635),  Q15(0.999969),  Q15(0.998126),  Q15(0.999592),
	Q15(0.999592),  Q15(0.998126),  Q15(0.999969),  Q15(0.997635),
	Q15(0.957199),  Q15(0.827490),  Q15(0.572805),  Q15(0.206002),
	Q15(-0.206002), Q15(-0.572805), Q15(-0.827490), Q15(-0.957199),
	Q15(-0.997635), Q15(-0.999969), Q15(-0.998126), Q15(-0.999592),
	Q15(-0.999592), Q15(-0.998126), Q15(-0.999969), Q15(-0.997635),
	Q15(-0.957199), Q15(-0.827490), Q15(-0.572805), Q15(-0.206002),
	Q15(0.206002),  Q15(0.572805),  Q15(0.827490),  Q15(0.957199),
	Q15(0.997635),  Q15(0.999969),  Q15(0.998126),  Q15(0.999592)
#endif /* SPACE_VECTOR_FIFTH_SINUS */

#if SPACE_VECTOR_TRAPIZE
	Q15(0.333333),  Q15(0.666667),  Q15(0.999969),  Q15(0.999969),
	Q15(0.999969),  Q15(0.999969),  Q15(0.999969),  Q15(0.999969),
	Q15(0.999969),  Q15(0.999969),  Q15(0.999969),  Q15(0.999969),
	Q15(0.999969),  Q15(0.999969),  Q15(0.666667),  Q15(0.333333),
	Q15(-0.333333), Q15(-0.666667), Q15(-0.999969), Q15(-0.999969),
	Q15(-0.999969), Q15(-0.999969), Q15(-0.999969), Q15(-0.999969),
	Q15(-0.999969), Q15(-0.999969), Q15(-0.999969), Q15(-0.999969),
	Q15(-0.999969), Q15(-0.999969), Q15(-0.666667), Q15(-0.333333),
	Q15(0.333333),  Q15(0.666667),  Q15(0.999969),  Q15(0.999969),
	Q15(0.999969),  Q15(0.999969),  Q15(0.999969),  Q15(0.999969)
#endif /* SPACE_VECTOR_TRAPIZE */

#if SPACE_VECTOR_BLOCK
	Q15(0.999969),  Q15(0.999969),  Q15(0.999969),  Q15(0.999969),
	Q15(0.999969),  Q15(0.999969),  Q15(0.999969),  Q15(0.999969),
	Q15(0.999969),  Q15(0.999969),  Q15(0.999969),  Q15(0.999969),
	Q15(0.999969),  Q15(0.999969),  Q15(0.999969),  Q15(0.999969),
	Q15(-0.999969), Q15(-0.999969), Q15(-0.999969), Q15(-0.999969),
	Q15(-0.999969), Q15(-0.999969), Q15(-0.999969), Q15(-0.999969),
	Q15(-0.999969), Q15(-0.999969), Q15(-0.999969), Q15(-0.999969),
	Q15(-0.999969), Q15(-0.999969), Q15(-0.999969), Q15(-0.999969),
	Q15(0.999969),  Q15(0.999969),  Q15(0.999969),  Q15(0.999969),
	Q15(0.999969),  Q15(0.999969),  Q15(0.999969),  Q15(0.999969)
#endif /* SPACE_VECTOR_BLOCK */
#else  /* (_SUPPORT_DOUBLE_USTEP == FALSE) */
#if SPACE_VECTOR_SINUS
	Q15(0.049125),	Q15(0.146903),	Q15(0.243266),	Q15(0.337286),
	Q15(0.428057),	Q15(0.514707),	Q15(0.596399),	Q15(0.672348),
	Q15(0.741822),	Q15(0.804151),	Q15(0.858736),	Q15(0.905051),
	Q15(0.942650),	Q15(0.971171),	Q15(0.990339),	Q15(0.999969),
	Q15(0.999969),	Q15(0.990339),	Q15(0.971171),	Q15(0.942650),
	Q15(0.905051),	Q15(0.858736),	Q15(0.804151),	Q15(0.741822),
	Q15(0.672348),	Q15(0.596399),	Q15(0.514707),	Q15(0.428057),
	Q15(0.337286),	Q15(0.243266),	Q15(0.146903),	Q15(0.049125),
	Q15(-0.049125),	Q15(-0.146903),	Q15(-0.243266),	Q15(-0.337286),
	Q15(-0.428057),	Q15(-0.514707),	Q15(-0.596399),	Q15(-0.672348),
	Q15(-0.741822),	Q15(-0.804151),	Q15(-0.858736),	Q15(-0.905051),
	Q15(-0.942650),	Q15(-0.971171),	Q15(-0.990339),	Q15(-0.999969),
	Q15(-0.999969),	Q15(-0.990339),	Q15(-0.971171),	Q15(-0.942650),
	Q15(-0.905051),	Q15(-0.858736),	Q15(-0.804151),	Q15(-0.741822),
	Q15(-0.672348),	Q15(-0.596399),	Q15(-0.514707),	Q15(-0.428057),
	Q15(-0.337286),	Q15(-0.243266),	Q15(-0.146903),	Q15(-0.049125),
	Q15(0.049125),	Q15(0.146903),	Q15(0.243266),	Q15(0.337286),
	Q15(0.428057),	Q15(0.514707),	Q15(0.596399),	Q15(0.672348),
	Q15(0.741822),	Q15(0.804151),	Q15(0.858736),	Q15(0.905051),
	Q15(0.942650),	Q15(0.971171),	Q15(0.990339),	Q15(0.999969)
#endif /* SPACE_VECTOR_SINUS */

#if SPACE_VECTOR_DOUBLE_SINUS
	Q15(0.077767),	Q15(0.231332),	Q15(0.379056),	Q15(0.517260),
	Q15(0.642584),	Q15(0.752097),	Q15(0.843386),	Q15(0.914640),
	Q15(0.964696),	Q15(0.993071),	Q15(0.999969),	Q15(0.986257),
	Q15(0.953423),	Q15(0.903509),	Q15(0.839025),	Q15(0.762846),
	Q15(0.762846),	Q15(0.839025),	Q15(0.903509),	Q15(0.953423),
	Q15(0.986257),	Q15(0.999969),	Q15(0.993071),	Q15(0.964696),
	Q15(0.914640),	Q15(0.843386),	Q15(0.752097),	Q15(0.642584),
	Q15(0.517260),	Q15(0.379056),	Q15(0.231332),	Q15(0.077767),
	Q15(-0.077767),	Q15(-0.231332),	Q15(-0.379056),	Q15(-0.517260),
	Q15(-0.642584),	Q15(-0.752097),	Q15(-0.843386),	Q15(-0.914640),
	Q15(-0.964696),	Q15(-0.993071),	Q15(-0.999969),	Q15(-0.986257),
	Q15(-0.953423),	Q15(-0.903509),	Q15(-0.839025),	Q15(-0.762846),
	Q15(-0.762846),	Q15(-0.839025),	Q15(-0.903509),	Q15(-0.953423),
	Q15(-0.986257),	Q15(-0.999969),	Q15(-0.993071),	Q15(-0.964696),
	Q15(-0.914640),	Q15(-0.843386),	Q15(-0.752097),	Q15(-0.642584),
	Q15(-0.517260),	Q15(-0.379056),	Q15(-0.231332),	Q15(-0.077767),
	Q15(0.077767),	Q15(0.231332),	Q15(0.379056),	Q15(0.517260),
	Q15(0.642584),	Q15(0.752097),	Q15(0.843386),	Q15(0.914640),
	Q15(0.964696),	Q15(0.993071),	Q15(0.999969),	Q15(0.986257),
	Q15(0.953423),	Q15(0.903509),	Q15(0.839025),	Q15(0.762846)
#endif /* SPACE_VECTOR_DOUBLE_SINUS */

#if SPACE_VECTOR_TRIPLE_SINUS
	Q15(0.090295),	Q15(0.267438),	Q15(0.434496),	Q15(0.585575),
	Q15(0.715951),	Q15(0.822395),	Q15(0.903378),	Q15(0.959124),
	Q15(0.991527),	Q15(1.003919),	Q15(1.000726),	Q15(0.987034),
	Q15(0.968103),	Q15(0.948878),	Q15(0.933528),	Q15(0.925065),
	Q15(0.925065),	Q15(0.933528),	Q15(0.948878),	Q15(0.968103),
	Q15(0.987034),	Q15(1.000726),	Q15(1.003919),	Q15(0.991527),
	Q15(0.959124),	Q15(0.903378),	Q15(0.822395),	Q15(0.715951),
	Q15(0.585575),	Q15(0.434496),	Q15(0.267438),	Q15(0.090295),
	Q15(-0.090295),	Q15(-0.267438),	Q15(-0.434496),	Q15(-0.585575),
	Q15(-0.715951),	Q15(-0.822395),	Q15(-0.903378),	Q15(-0.959124),
	Q15(-0.991527),	Q15(-1.003919),	Q15(-1.000726),	Q15(-0.987034),
	Q15(-0.968103),	Q15(-0.948878),	Q15(-0.933528),	Q15(-0.925065),
	Q15(-0.925065),	Q15(-0.933528),	Q15(-0.948878),	Q15(-0.968103),
	Q15(-0.987034),	Q15(-1.000726),	Q15(-1.003919),	Q15(-0.991527),
	Q15(-0.959124),	Q15(-0.903378),	Q15(-0.822395),	Q15(-0.715951),
	Q15(-0.585575),	Q15(-0.434496),	Q15(-0.267438),	Q15(-0.090295),
	Q15(0.090295),	Q15(0.267438),	Q15(0.434496),	Q15(0.585575),
	Q15(0.715951),	Q15(0.822395),	Q15(0.903378),	Q15(0.959124),
	Q15(0.991527),	Q15(1.003919),	Q15(1.000726),	Q15(0.987034),
	Q15(0.968103),	Q15(0.948878),	Q15(0.933528),	Q15(0.925065)
#endif /* SPACE_VECTOR_TRIPPLE_SINUS */

#if SPACE_VECTOR_FIFTH_SINUS
	Q15(0.103752),	Q15(0.305299),	Q15(0.489804),	Q15(0.648507),
	Q15(0.776039),	Q15(0.870874),	Q15(0.935106),	Q15(0.973639),
	Q15(0.992994),	Q15(0.999996),	Q15(1.000618),	Q15(0.999206),
	Q15(0.998198),	Q15(0.998322),	Q15(0.999165),	Q15(0.999890),
	Q15(0.999890),	Q15(0.999165),	Q15(0.998322),	Q15(0.998198),
	Q15(0.999206),	Q15(1.000618),	Q15(0.999996),	Q15(0.992994),
	Q15(0.973639),	Q15(0.935106),	Q15(0.870874),	Q15(0.776039),
	Q15(0.648507),	Q15(0.489804),	Q15(0.305299),	Q15(0.103752),
	Q15(-0.103752),	Q15(-0.305299),	Q15(-0.489804),	Q15(-0.648507),
	Q15(-0.776039),	Q15(-0.870874),	Q15(-0.935106),	Q15(-0.973639),
	Q15(-0.992994),	Q15(-0.999996),	Q15(-1.000618),	Q15(-0.999206),
	Q15(-0.998198),	Q15(-0.998322),	Q15(-0.999165),	Q15(-0.999890),
	Q15(-0.999890),	Q15(-0.999165),	Q15(-0.998322),	Q15(-0.998198),
	Q15(-0.999206),	Q15(-1.000618),	Q15(-0.999996),	Q15(-0.992994),
	Q15(-0.973639),	Q15(-0.935106),	Q15(-0.870874),	Q15(-0.776039),
	Q15(-0.648507),	Q15(-0.489804),	Q15(-0.305299),	Q15(-0.103752),
	Q15(0.103752),	Q15(0.305299),	Q15(0.489804),	Q15(0.648507),
	Q15(0.776039),	Q15(0.870874),	Q15(0.935106),	Q15(0.973639),
	Q15(0.992994),	Q15(0.999996),	Q15(1.000618),	Q15(0.999206),
	Q15(0.998198),	Q15(0.998322),	Q15(0.999165),	Q15(0.999890)
#endif /* SPACE_VECTOR_FIFTH_SINUS */

#if SPACE_VECTOR_TRAPIZE
	Q15(0.166667),  Q15(0.333333),  Q15(0.500000),  Q15(0.666667),
	Q15(0.833333),  Q15(0.999969),  Q15(0.999969),  Q15(0.999969),
	Q15(0.999969),  Q15(0.999969),  Q15(0.999969),  Q15(0.999969),
	Q15(0.999969),  Q15(0.999969),  Q15(0.999969),  Q15(0.999969),
	Q15(0.999969),  Q15(0.999969),  Q15(0.999969),  Q15(0.999969),
	Q15(0.999969),  Q15(0.999969),  Q15(0.999969),  Q15(0.999969),
	Q15(0.999969),  Q15(0.999969),  Q15(0.999969),  Q15(0.833333),
	Q15(0.666667),  Q15(0.500000),  Q15(0.333333),  Q15(0.166667),
	Q15(-0.166667), Q15(-0.333333), Q15(-0.500000), Q15(-0.666667),
	Q15(-0.833333), Q15(-0.999969), Q15(-0.999969), Q15(-0.999969),
	Q15(-0.999969), Q15(-0.999969), Q15(-0.999969), Q15(-0.999969),
	Q15(-0.999969), Q15(-0.999969), Q15(-0.999969), Q15(-0.999969),
	Q15(-0.999969), Q15(-0.999969), Q15(-0.999969), Q15(-0.999969),
	Q15(-0.999969), Q15(-0.999969), Q15(-0.999969), Q15(-0.999969),
	Q15(-0.999969), Q15(-0.999969), Q15(-0.999969), Q15(-0.833333),
	Q15(-0.666667), Q15(-0.500000), Q15(-0.333333), Q15(-0.166667),
	Q15(0.166667),  Q15(0.333333),  Q15(0.500000),  Q15(0.666667),
	Q15(0.833333),  Q15(0.999969),  Q15(0.999969),  Q15(0.999969),
	Q15(0.999969),  Q15(0.999969),  Q15(0.999969),  Q15(0.999969),
	Q15(0.999969),  Q15(0.999969),  Q15(0.999969),  Q15(0.999969)
#endif /* SPACE_VECTOR_TRAPIZE */

#if SPACE_VECTOR_BLOCK
	Q15(0.999969),  Q15(0.999969),  Q15(0.999969),  Q15(0.999969),
	Q15(0.999969),  Q15(0.999969),  Q15(0.999969),  Q15(0.999969),
	Q15(0.999969),  Q15(0.999969),  Q15(0.999969),  Q15(0.999969),
	Q15(0.999969),  Q15(0.999969),  Q15(0.999969),  Q15(0.999969),
	Q15(0.999969),  Q15(0.999969),  Q15(0.999969),  Q15(0.999969),
	Q15(0.999969),  Q15(0.999969),  Q15(0.999969),  Q15(0.999969),
	Q15(0.999969),  Q15(0.999969),  Q15(0.999969),  Q15(0.999969),
	Q15(0.999969),  Q15(0.999969),  Q15(0.999969),  Q15(0.999969),
	Q15(-0.999969), Q15(-0.999969), Q15(-0.999969), Q15(-0.999969),
	Q15(-0.999969), Q15(-0.999969), Q15(-0.999969), Q15(-0.999969),
	Q15(-0.999969), Q15(-0.999969), Q15(-0.999969), Q15(-0.999969),
	Q15(-0.999969), Q15(-0.999969), Q15(-0.999969), Q15(-0.999969),
	Q15(-0.999969), Q15(-0.999969), Q15(-0.999969), Q15(-0.999969),
	Q15(-0.999969), Q15(-0.999969), Q15(-0.999969), Q15(-0.999969),
	Q15(-0.999969), Q15(-0.999969), Q15(-0.999969), Q15(-0.999969),
	Q15(-0.999969), Q15(-0.999969), Q15(-0.999969), Q15(-0.999969),
	Q15(0.999969),  Q15(0.999969),  Q15(0.999969),  Q15(0.999969),
	Q15(0.999969),  Q15(0.999969),  Q15(0.999969),  Q15(0.999969),
	Q15(0.999969),  Q15(0.999969),  Q15(0.999969),  Q15(0.999969),
	Q15(0.999969),  Q15(0.999969),  Q15(0.999969),  Q15(0.999969)
#endif /* SPACE_VECTOR_BLOCK */
#endif /* (_SUPPORT_DOUBLE_USTEP == FALSE) */
};

/* EOF */
