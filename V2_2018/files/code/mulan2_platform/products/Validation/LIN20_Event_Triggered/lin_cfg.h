/*
 * LIN Configuration API
 *
 * This file is application specific and depends on LDF/NCF files
 * Generated by configuration tool C:\EVWS\_LIBRARY_PLATFORMS\library_platform_mulan2\bin\ldf_nodegen.exe (version 1.4.0)
 *
 * Copyright (C) 2007-2015 Melexis N.V.
 */

#ifndef LIN_CFG_H_
#define LIN_CFG_H_


/* 
 * ----------------------------------------------------------------------------
 *              LIN configuration
 * ----------------------------------------------------------------------------
 */
/* Baudrate */
#define LDF_BAUDRATE	9600

#if !defined (HAS_LIN_AUTOBAUDRATE)

#if (LDF_BAUDRATE != ML_BAUDRATE)
#error "Baud rate setting in application LDF file doesn't match setting in Chip.mk of the SW platform"
#endif

#endif /* HAS_LIN_AUTOBAUDRATE */

#if LIN_VERSION >= 21
#define LDF_N_AS_TIMEOUT	1000U /* ms */
#define LDF_N_CR_TIMEOUT	1000U /* ms */
#endif /* LIN_VERSION >= 21 */

/* Number of dynamic messages */
#define ML_NUMBER_OF_DYNAMIC_MESSAGES	4U

/*
 * Initializer for use with static (pre-)configuration
 * Contains configured_nad + protected id for each index
 */
#define ML_NODE_CONFIGURATION_INITIALIZER { 0x31U /* configured NAD */,  0xc1U,  0x61U,  0x20U,  0x03U}

/* --- LIN 2.x ------------------------------------------------------------- */
#if LIN_VERSION >= 20
/* Node Address for/by Diagnostics */
extern const ml_uint8 ml_InitialNAD;
extern ml_uint8 ml_ConfiguredNAD;

/* LIN Product Identification */
extern const ml_uint8 ml_ProductID[5];
#endif /* LIN_VERSION >= 20 */

#if defined(HAS_SAVE_CONFIGURATION_SERVICE)

/* return values for ld_read_configuration */
enum {
    LD_READ_OK,
    LD_LENGTH_TOO_SHORT
};

/* return values for ld_set_configuration */
enum {
    LD_SET_OK,
    LD_LENGTH_NOT_CORRECT,
    LD_DATA_ERROR
};

extern l_u8 node_configuration[ML_NUMBER_OF_DYNAMIC_MESSAGES];

/* Ignore interface parameter ifc since only one LIN interface is available */
#define ld_read_configuration(ifc, data, length) ld_read_configuration_i1((data), (length))
#define ld_set_configuration(ifc, data, length) ld_set_configuration_i1((data), (length))
extern l_u8 ld_read_configuration_i1 (l_u8* const data, l_u8* const length);
extern l_u8 ld_set_configuration_i1  (const l_u8* const data, l_u16 length); 

#endif /* HAS_SAVE_CONFIGURATION_SERVICE */


#endif /* LIN_CFG_H_ */
