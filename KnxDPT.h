//    This file is part of Arduino Knx Bus Device library.

//    The Arduino Knx Bus Device library allows to turn Arduino into "self-made" KNX bus device.
//    Copyright (C) 2014 2015 2016 Franck MARINI (fm@liwan.fr)

//    The Arduino Knx Bus Device library is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.

//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.

//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.


// File : KnxDPT.h
// Author : Franck Marini
// Description : Definition of the KNX Datapoints types as per "knx.org" specification

#ifndef KNXDPT_H
#define KNXDPT_H

#include <avr/pgmspace.h> // DPT arrays are stored in flash using PROG MEMORY

// List of the DPT formats
// A Character
// A[n] String of n characters
// B Boolean / Bit set
// C Control
// E Exponent
// F Floating point value
// M Mantissa
// N eNumeration
// r Reserved bit or field
// S Sign
// U Unsigned value
// V 2's Complement signed value
// Z8 Standardised Status/Command B8. Encoding as in DPT_StatusGen
enum eKnxDPT_Format {
  KNX_DPT_FORMAT_B1 = 0,
  KNX_DPT_FORMAT_B2,
  KNX_DPT_FORMAT_B1U3,
  KNX_DPT_FORMAT_A8,
  KNX_DPT_FORMAT_U8,
  KNX_DPT_FORMAT_V8,
  KNX_DPT_FORMAT_B5N3,
  KNX_DPT_FORMAT_U16,
  KNX_DPT_FORMAT_V16,
  KNX_DPT_FORMAT_F16,
  KNX_DPT_FORMAT_N3N5R2N6R2N6,
  KNX_DPT_FORMAT_R3N5R4N4R1U7,
  KNX_DPT_FORMAT_U32,
  KNX_DPT_FORMAT_V32,
  KNX_DPT_FORMAT_F32,
  KNX_DPT_FORMAT_U4U4U4U4U4U4B4N4,
  KNX_DPT_FORMAT_A112,
  KNX_DPT_FORMAT_R2U6, 
  KNX_DPT_FORMAT_B1R1U6,
  KNX_DPT_FORMAT_U8R4U4R3U5U3U5R2U6B16,
  KNX_DPT_FORMAT_N8,
  KNX_DPT_FORMAT_B8,
  KNX_DPT_FORMAT_B16,
  KNX_DPT_FORMAT_N2,
  KNX_DPT_FORMAT_AN,
  KNX_DPT_FORMAT_U4U4,
  KNX_DPT_FORMAT_R1B1U6,
  KNX_DPT_FORMAT_B32,
  KNX_DPT_FORMAT_V64,
  KNX_DPT_FORMAT_B24,
  KNX_DPT_FORMAT_N3,
  KNX_DPT_FORMAT_B1Z8,
  KNX_DPT_FORMAT_N8Z8,
  KNX_DPT_FORMAT_U8Z8,
  KNX_DPT_FORMAT_U16Z8,
  KNX_DPT_FORMAT_V8Z8,
  KNX_DPT_FORMAT_V16Z8,
  KNX_DPT_FORMAT_U16N8,
  KNX_DPT_FORMAT_U8B8,
  KNX_DPT_FORMAT_V16B8,
  KNX_DPT_FORMAT_V16B16,
  KNX_DPT_FORMAT_U8N8,
  KNX_DPT_FORMAT_V16V16V16,
  KNX_DPT_FORMAT_V16V16V16V16,
  KNX_DPT_FORMAT_V16U8B8,
  KNX_DPT_FORMAT_V16U8B16,
  KNX_DPT_FORMAT_U16U8N8N8P8,
  KNX_DPT_FORMAT_U5U5U16,
  KNX_DPT_FORMAT_V32Z8,
  KNX_DPT_FORMAT_U8N8N8N8B8B8,
  KNX_DPT_FORMAT_U16V16,
  KNX_DPT_FORMAT_N16U32,
  KNX_DPT_FORMAT_F16F16F16,
  KNX_DPT_FORMAT_V8N8N8,
  KNX_DPT_FORMAT_V16V16N8N8,
  KNX_DPT_FORMAT_U16U8,
  KNX_DPT_FORMAT_V32N8Z8,
  KNX_DPT_FORMAT_U16U32U8N8,
  KNX_DPT_FORMAT_A8A8A8A8,
  KNX_DPT_FORMAT_U8U8U8,
  KNX_DPT_FORMAT_A8A8
};

// Definition of the length in bits according to the format
// NB : table is stored in flash program memory to save RAM
const byte KnxDPTFormatToLengthBit[] PROGMEM = {
  1 , //  KNX_DPT_FORMAT_B1 = 0,
  2 , //  KNX_DPT_FORMAT_B2,
  4 , // KNX_DPT_FORMAT_B1U3
  8 , //  KNX_DPT_FORMAT_A8,
  8 , //  KNX_DPT_FORMAT_U8,
  8 , //  KNX_DPT_FORMAT_V8,
  8 , //  KNX_DPT_FORMAT_B5N3,
  16, //  KNX_DPT_FORMAT_U16,
  16, //  KNX_DPT_FORMAT_V16,
  16, //  KNX_DPT_FORMAT_F16,
  24, //  KNX_DPT_FORMAT_N3N5R2N6N2N6,
  24, //  KNX_DPT_FORMAT_R3N5R4N4R1N7,
  32, //  KNX_DPT_FORMAT_U32,
  32, //  KNX_DPT_FORMAT_V32,
  32, //  KNX_DPT_FORMAT_F32,
  32, //  KNX_DPT_FORMAT_U4U4U4U4U4U4B4N4,
 112, //  KNX_DPT_FORMAT_A112,
  8 , //  KNX_DPT_FORMAT_R2U6, 
  8 , //  KNX_DPT_FORMAT_B1R1U6,
  56, //  KNX_DPT_FORMAT_U8R4U4R3U5U3U5R2U6B16,
  8 , //  KNX_DPT_FORMAT_N8,
  8 , //  KNX_DPT_FORMAT_B8,
  16, //  KNX_DPT_FORMAT_B16,
  2 ,//  KNX_DPT_FORMAT_N2,
 112,//  KNX_DPT_FORMAT_AN,
  8 , //  KNX_DPT_FORMAT_U4U4,
  8 , //  KNX_DPT_FORMAT_R1B1U6,
  32, //  KNX_DPT_FORMAT_B32,
  64, //  KNX_DPT_FORMAT_V64,
  24, //  KNX_DPT_FORMAT_B24,
  3 , //  KNX_DPT_FORMAT_N3,
  2 , //  KNX_DPT_FORMAT_B1Z8,
  16, //  KNX_DPT_FORMAT_N8Z8,
  16, //  KNX_DPT_FORMAT_U8Z8,
  24, //  KNX_DPT_FORMAT_U16Z8,
  16, //  KNX_DPT_FORMAT_V8Z8,
  24, //  KNX_DPT_FORMAT_V16Z8,
  24, //  KNX_DPT_FORMAT_U16N8,
  16, //  KNX_DPT_FORMAT_U8B8,
  24, //  KNX_DPT_FORMAT_V16B8,
  32, //  KNX_DPT_FORMAT_V16B16,
  16, //  KNX_DPT_FORMAT_U8N8,
  48, //  KNX_DPT_FORMAT_V16V16V16,
  64, //  KNX_DPT_FORMAT_V16V16V16V16,
  32, //  KNX_DPT_FORMAT_V16U8B8,
  40, //  KNX_DPT_FORMAT_V16U8B16,
  48, //  KNX_DPT_FORMAT_U16U8N8N8P8,
  16, //  KNX_DPT_FORMAT_U5U5U6,
  40, //  KNX_DPT_FORMAT_V32Z8,
  48, //  KNX_DPT_FORMAT_U8N8N8N8B8B8,
  32, //  KNX_DPT_FORMAT_U16V16,
  48, //  KNX_DPT_FORMAT_N16U32,
  48, //  KNX_DPT_FORMAT_F16F16F16,
  24, //  KNX_DPT_FORMAT_V8N8N8,
  48, //  KNX_DPT_FORMAT_V16V16N8N8,
  24, //  KNX_DPT_FORMAT_U16U8,
  48, //  KNX_DPT_FORMAT_V32N8Z8,
  64, //  KNX_DPT_FORMAT_U16U32U8N8,
  24, //  KNX_DPT_FORMAT_A8A8A8A8,
  24, //  KNX_DPT_FORMAT_U8U8U8,
  16 //  KNX_DPT_FORMAT_A8A8
};

// Define all the KNX Datapoint IDs (from 1.001 to 14.007 only)
enum e_KnxDPT_ID { 
  KNX_DPT_1_001 = 0, // 1.001 B1 DPT_Switch
  KNX_DPT_1_002, // 1.002 B1 DPT_Bool
  KNX_DPT_1_003, // 1.003 B1 DPT_Enable
  KNX_DPT_1_004, // 1.004 B1 DPT_Ramp
  KNX_DPT_1_005, // 1.005 B1 DPT_Alarm
  KNX_DPT_1_006, // 1.006 B1 DPT_BinaryValue
  KNX_DPT_1_007, // 1.007 B1 DPT_Step
  KNX_DPT_1_008, // 1.008 B1 DPT_UpDown
  KNX_DPT_1_009, // 1.009 B1 DPT_OpenClose
  KNX_DPT_1_010, // 1.010 B1 DPT_Start
  KNX_DPT_1_011, // 1.011 B1 DPT_State
  KNX_DPT_1_012, // 1.012 B1 DPT_Invert
  KNX_DPT_1_013, // 1.013 B1 DPT_DimSendStyle
  KNX_DPT_1_014, // 1.014 B1 DPT_InputSource
  KNX_DPT_1_015, // 1.015 B1 DPT_Reset
  KNX_DPT_1_016, // 1.016 B1 DPT_Ack
  KNX_DPT_1_017, // 1.017 B1 DPT_Trigger
  KNX_DPT_1_018, // 1.018 B1 DPT_Occupancy
  KNX_DPT_1_019, // 1.019 B1 DPT_Window_Door
  KNX_DPT_1_021, // 1.021 B1 DPT_LogicalFunction
  KNX_DPT_1_022, // 1.022 B1 DPT_Scene_AB
  KNX_DPT_1_023, // 1.023 B1 DPT_ShutterBlinds_Mode
  KNX_DPT_1_100, // 1.100 B1 DPT_Heat/Cool
  KNX_DPT_2_001, // 2.001 B2 DPT_Switch_Control
  KNX_DPT_2_002, // 2.002 B2 DPT_Bool_Control
  KNX_DPT_2_003, // 2.003 B2 DPT_Enable_Control
  KNX_DPT_2_004, // 2.004 B2 DPT_Ramp_Control
  KNX_DPT_2_005, // 2.005 B2 DPT_Alarm_Control
  KNX_DPT_2_006, // 2.006 B2 DPT_BinaryValue_Control
  KNX_DPT_2_007, // 2.007 B2 DPT_Step_Control
  KNX_DPT_2_008, // 2.008 B2 DPT_Direction1_Control
  KNX_DPT_2_009, // 2.009 B2 DPT_Direction2_Control
  KNX_DPT_2_010, // 2.010 B2 DPT_Start_Control
  KNX_DPT_2_011, // 2.011 B2 DPT_State_Control
  KNX_DPT_2_012, // 2.012 B2 DPT_Invert_Control
  KNX_DPT_3_007, // 3.007 B1U3 DPT_Control_Dimming
  KNX_DPT_3_008, // 3.008 B1U3 DPT_Control_Blinds
  KNX_DPT_4_001, // 4.001 A8 DPT_Char_ASCII
  KNX_DPT_4_002, // 4.002 A8 DPT_Char_8859_1
  KNX_DPT_5_001, // 5.001 U8 DPT_Scaling
  KNX_DPT_5_003, // 5.003 U8 DPT_Angle
  KNX_DPT_5_004, // 5.004 U8 DPT_Percent_U8
  KNX_DPT_5_005, // 5.005 U8 DPT_DecimalFactor
  KNX_DPT_5_006, // 5.006 U8 DPT_Tariff
  KNX_DPT_5_010, // 5.010 U8 DPT_Value_1_Ucount
  KNX_DPT_6_001, // 6.001 V8 DPT_Percent_V8
  KNX_DPT_6_010, // 6.010 V8 DPT_Value_1_Count
  KNX_DPT_6_020, // 6.020 B5N3 DPT_Status_Mode3
  KNX_DPT_7_001, // 7.001 U16 DPT_Value_2_Ucount
  KNX_DPT_7_002, // 7.002 U16 DPT_TimePeriodMsec
  KNX_DPT_7_003, // 7.003 U16 DPT_TimePeriod10MSec
  KNX_DPT_7_004, // 7.004 U16 DPT_TimePeriod100MSec
  KNX_DPT_7_005, // 7.005 U16 DPT_TimePeriodSec
  KNX_DPT_7_006, // 7.006 U16 DPT_TimePeriodMin
  KNX_DPT_7_007, // 7.007 U16 DPT_TimePeriodHrs
  KNX_DPT_7_010, // 7.010 U16 DPT_PropDataType
  KNX_DPT_7_011, // 7.011 U16 DPT_Length_mm
  KNX_DPT_7_012, // 7.012 U16 DPT_UElCurrentmA
  KNX_DPT_7_013, // 7.013 U16 DPT_Brightness
  KNX_DPT_8_001, // 8.001 V16 DPT_Value_2_Count
  KNX_DPT_8_002, // 8.002 V16 DPT_DeltaTimeMsec
  KNX_DPT_8_003, // 8.003 V16 DPT_DeltaTime10MSec
  KNX_DPT_8_004, // 8.004 V16 DPT_DeltaTime100MSec
  KNX_DPT_8_005, // 8.005 V16 DPT_DeltaTimeSec
  KNX_DPT_8_006, // 8.006 V16 DPT_DeltaTimeMin
  KNX_DPT_8_007, // 8.007 V16 DPT_DeltaTimeHrs
  KNX_DPT_8_010, // 8.010 V16 DPT_Percent_V16
  KNX_DPT_8_011, // 8.011 V16 DPT_Rotation_Angle
  KNX_DPT_9_001, // 9.001 F16 DPT_Value_Temp
  KNX_DPT_9_002, // 9.002 F16 DPT_Value_Tempd
  KNX_DPT_9_003, // 9.003 F16 DPT_Value_Tempa
  KNX_DPT_9_004, // 9.004 F16 DPT_Value_Lux
  KNX_DPT_9_005, // 9.005 F16 DPT_Value_Wsp
  KNX_DPT_9_006, // 9.006 F16 DPT_Value_Pres
  KNX_DPT_9_007, // 9.007 F16 DPT_Value_Humidity
  KNX_DPT_9_008, // 9.008 F16 DPT_Value_AirQuality
  KNX_DPT_9_010, // 9.010 F16 DPT_Value_Time1
  KNX_DPT_9_011, // 9.011 F16 DPT_Value_Time2
  KNX_DPT_9_020, // 9.020 F16 DPT_Value_Volt
  KNX_DPT_9_021, // 9.021 F16 DPT_Value_Curr
  KNX_DPT_9_022, // 9.022 F16 DPT_PowerDensity
  KNX_DPT_9_023, // 9.023 F16 DPT_KelvinPerPercent
  KNX_DPT_9_024, // 9.024 F16 DPT_Power
  KNX_DPT_9_025, // 9.025 F16 DPT_Value_Volume_Flow
  KNX_DPT_9_026, // 9.026 F16 DPT_Rain_Amount
  KNX_DPT_9_027, // 9.027 F16 DPT_Value_Temp_F
  KNX_DPT_9_028, // 9.028 F16 DPT_Value_Wsp_kmh
  KNX_DPT_10_001, // 10.001 N3N5r2N6r2N6 DPT_TimeOfDay
  KNX_DPT_11_001, // 11.001 r3N5r4N4r1U7 DPT_Date
  KNX_DPT_12_001, // 12.001 U32 DPT_Value_4_Ucount
  KNX_DPT_13_001, // 13.001 V32 DPT_Value_4_Count
  KNX_DPT_13_010, // 13.010 V32 DPT_ActiveEnergy
  KNX_DPT_13_011, // 13.011 V32 DPT_ApparantEnergy
  KNX_DPT_13_012, // 13.012 V32 DPT_ReactiveEnergy
  KNX_DPT_13_013, // 13.013 V32 DPT_ActiveEnergy_kWh
  KNX_DPT_13_014, // 13.014 V32 DPT_ApparantEnergy_kVAh
  KNX_DPT_13_015, // 13.015 V32 DPT_ReactiveEnergy_kVARh
  KNX_DPT_13_100, // 13.100 V32 DPT_LongDeltaTimeSec
  KNX_DPT_14_000, // 14.000 F32 DPT_Value_Acceleration
  KNX_DPT_14_001, // 14.001 F32 DPT_Value_Acceleration_Angular
  KNX_DPT_14_002, // 14.002 F32 DPT_Value_Activation_Energy
  KNX_DPT_14_003, // 14.003 F32 DPT_Value_Activity
  KNX_DPT_14_004, // 14.004 F32 DPT_Value_Mol
  KNX_DPT_14_005, // 14.005 F32 DPT_Value_Amplitude
  KNX_DPT_14_006, // 14.006 F32 DPT_Value_AngleRad
  KNX_DPT_14_007 // 14.007 F32 DPT_Value_AngleDeg
};		 

// Definition of the format according to the ID
// NB : table is stored in flash program memory to save RAM
const byte KnxDPTIdToFormat[] PROGMEM = {
  KNX_DPT_FORMAT_B1, //  KNX_DPT_1_001, // 1.001 B1 DPT_Switch
  KNX_DPT_FORMAT_B1, //  KNX_DPT_1_002, // 1.002 B1 DPT_Bool
  KNX_DPT_FORMAT_B1, //  KNX_DPT_1_003, // 1.003 B1 DPT_Enable
  KNX_DPT_FORMAT_B1, //  KNX_DPT_1_004, // 1.004 B1 DPT_Ramp
  KNX_DPT_FORMAT_B1, //  KNX_DPT_1_005, // 1.005 B1 DPT_Alarm
  KNX_DPT_FORMAT_B1, //  KNX_DPT_1_006, // 1.006 B1 DPT_BinaryValue
  KNX_DPT_FORMAT_B1, //  KNX_DPT_1_007, // 1.007 B1 DPT_Step
  KNX_DPT_FORMAT_B1, //  KNX_DPT_1_008, // 1.008 B1 DPT_UpDown
  KNX_DPT_FORMAT_B1, //  KNX_DPT_1_009, // 1.009 B1 DPT_OpenClose
  KNX_DPT_FORMAT_B1, //  KNX_DPT_1_010, // 1.010 B1 DPT_Start
  KNX_DPT_FORMAT_B1, //  KNX_DPT_1_011, // 1.011 B1 DPT_State
  KNX_DPT_FORMAT_B1, //  KNX_DPT_1_012, // 1.012 B1 DPT_Invert
  KNX_DPT_FORMAT_B1, //  KNX_DPT_1_013, // 1.013 B1 DPT_DimSendStyle
  KNX_DPT_FORMAT_B1, //  KNX_DPT_1_014, // 1.014 B1 DPT_InputSource
  KNX_DPT_FORMAT_B1, //  KNX_DPT_1_015, // 1.015 B1 DPT_Reset
  KNX_DPT_FORMAT_B1, //  KNX_DPT_1_016, // 1.016 B1 DPT_Ack
  KNX_DPT_FORMAT_B1, //  KNX_DPT_1_017, // 1.017 B1 DPT_Trigger
  KNX_DPT_FORMAT_B1, //  KNX_DPT_1_018, // 1.018 B1 DPT_Occupancy
  KNX_DPT_FORMAT_B1, //  KNX_DPT_1_019, // 1.019 B1 DPT_Window_Door
  KNX_DPT_FORMAT_B1, //  KNX_DPT_1_021, // 1.021 B1 DPT_LogicalFunction
  KNX_DPT_FORMAT_B1, //  KNX_DPT_1_022, // 1.022 B1 DPT_Scene_AB
  KNX_DPT_FORMAT_B1, //  KNX_DPT_1_023, // 1.023 B1 DPT_ShutterBlinds_Mode
  KNX_DPT_FORMAT_B1, //  KNX_DPT_1_100, // 1.100 B1 DPT_Heat/Cool
  KNX_DPT_FORMAT_B2, //  KNX_DPT_2_001, // 2.001 B2 DPT_Switch_Control
  KNX_DPT_FORMAT_B2, //  KNX_DPT_2_002, // 2.002 B2 DPT_Bool_Control
  KNX_DPT_FORMAT_B2, //  KNX_DPT_2_003, // 2.003 B2 DPT_Enable_Control
  KNX_DPT_FORMAT_B2, //  KNX_DPT_2_004, // 2.004 B2 DPT_Ramp_Control
  KNX_DPT_FORMAT_B2, //  KNX_DPT_2_005, // 2.005 B2 DPT_Alarm_Control
  KNX_DPT_FORMAT_B2, //  KNX_DPT_2_006, // 2.006 B2 DPT_BinaryValue_Control
  KNX_DPT_FORMAT_B2, //  KNX_DPT_2_007, // 2.007 B2 DPT_Step_Control
  KNX_DPT_FORMAT_B2, //  KNX_DPT_2_008, // 2.008 B2 DPT_Direction1_Control
  KNX_DPT_FORMAT_B2, //  KNX_DPT_2_009, // 2.009 B2 DPT_Direction2_Control
  KNX_DPT_FORMAT_B2, //  KNX_DPT_2_010, // 2.010 B2 DPT_Start_Control
  KNX_DPT_FORMAT_B2, //  KNX_DPT_2_011, // 2.011 B2 DPT_State_Control
  KNX_DPT_FORMAT_B2, //  KNX_DPT_2_012, // 2.012 B2 DPT_Invert_Control
  KNX_DPT_FORMAT_B1U3, //  KNX_DPT_3_007, // 3.007 B1U3 DPT_Control_Dimming
  KNX_DPT_FORMAT_B1U3, //  KNX_DPT_3_008, // 3.008 B1U3 DPT_Control_Blinds
  KNX_DPT_FORMAT_A8, //  KNX_DPT_4_001, // 4.001 A8 DPT_Char_ASCII
  KNX_DPT_FORMAT_A8, //  KNX_DPT_4_002, // 4.002 A8 DPT_Char_8859_1
  KNX_DPT_FORMAT_U8, //  KNX_DPT_5_001, // 5.001 U8 DPT_Scaling
  KNX_DPT_FORMAT_U8, //  KNX_DPT_5_003, // 5.003 U8 DPT_Angle
  KNX_DPT_FORMAT_U8, //  KNX_DPT_5_004, // 5.004 U8 DPT_Percent_U8
  KNX_DPT_FORMAT_U8, //  KNX_DPT_5_005, // 5.005 U8 DPT_DecimalFactor
  KNX_DPT_FORMAT_U8, //  KNX_DPT_5_006, // 5.006 U8 DPT_Tariff
  KNX_DPT_FORMAT_U8, //  KNX_DPT_5_010, // 5.010 U8 DPT_Value_1_Ucount
  KNX_DPT_FORMAT_V8, //  KNX_DPT_6_001, // 6.001 V8 DPT_Percent_V8
  KNX_DPT_FORMAT_V8, //  KNX_DPT_6_010, // 6.010 V8 DPT_Value_1_Count
  KNX_DPT_FORMAT_B5N3, //  KNX_DPT_6_020, // 6.020 B5N3 DPT_Status_Mode3
  KNX_DPT_FORMAT_U16, //  KNX_DPT_7_001, // 7.001 U16 DPT_Value_2_Ucount
  KNX_DPT_FORMAT_U16, //  KNX_DPT_7_002, // 7.002 U16 DPT_TimePeriodMsec
  KNX_DPT_FORMAT_U16, //  KNX_DPT_7_003, // 7.003 U16 DPT_TimePeriod10MSec
  KNX_DPT_FORMAT_U16, //  KNX_DPT_7_004, // 7.004 U16 DPT_TimePeriod100MSec
  KNX_DPT_FORMAT_U16, //  KNX_DPT_7_005, // 7.005 U16 DPT_TimePeriodSec
  KNX_DPT_FORMAT_U16, //  KNX_DPT_7_006, // 7.006 U16 DPT_TimePeriodMin
  KNX_DPT_FORMAT_U16, //  KNX_DPT_7_007, // 7.007 U16 DPT_TimePeriodHrs
  KNX_DPT_FORMAT_U16, //  KNX_DPT_7_010, // 7.010 U16 DPT_PropDataType
  KNX_DPT_FORMAT_U16, //  KNX_DPT_7_011, // 7.011 U16 DPT_Length_mm
  KNX_DPT_FORMAT_U16, //  KNX_DPT_7_012, // 7.012 U16 DPT_UElCurrentmA
  KNX_DPT_FORMAT_U16, //  KNX_DPT_7_013, // 7.013 U16 DPT_Brightness
  KNX_DPT_FORMAT_V16, //  KNX_DPT_8_001, // 8.001 V16 DPT_Value_2_Count
  KNX_DPT_FORMAT_V16, //  KNX_DPT_8_002, // 8.002 V16 DPT_DeltaTimeMsec
  KNX_DPT_FORMAT_V16, //  KNX_DPT_8_003, // 8.003 V16 DPT_DeltaTime10MSec
  KNX_DPT_FORMAT_V16, //  KNX_DPT_8_004, // 8.004 V16 DPT_DeltaTime100MSec
  KNX_DPT_FORMAT_V16, //  KNX_DPT_8_005, // 8.005 V16 DPT_DeltaTimeSec
  KNX_DPT_FORMAT_V16, //  KNX_DPT_8_006, // 8.006 V16 DPT_DeltaTimeMin
  KNX_DPT_FORMAT_V16, //  KNX_DPT_8_007, // 8.007 V16 DPT_DeltaTimeHrs
  KNX_DPT_FORMAT_V16, //  KNX_DPT_8_010, // 8.010 V16 DPT_Percent_V16
  KNX_DPT_FORMAT_V16, //  KNX_DPT_8_011, // 8.011 V16 DPT_Rotation_Angle
  KNX_DPT_FORMAT_F16, //  KNX_DPT_9_001, // 9.001 F16 DPT_Value_Temp
  KNX_DPT_FORMAT_F16, //  KNX_DPT_9_002, // 9.002 F16 DPT_Value_Tempd
  KNX_DPT_FORMAT_F16, //  KNX_DPT_9_003, // 9.003 F16 DPT_Value_Tempa
  KNX_DPT_FORMAT_F16, //  KNX_DPT_9_004, // 9.004 F16 DPT_Value_Lux
  KNX_DPT_FORMAT_F16, //  KNX_DPT_9_005, // 9.005 F16 DPT_Value_Wsp
  KNX_DPT_FORMAT_F16, //  KNX_DPT_9_006, // 9.006 F16 DPT_Value_Pres
  KNX_DPT_FORMAT_F16, //  KNX_DPT_9_007, // 9.007 F16 DPT_Value_Humidity
  KNX_DPT_FORMAT_F16, //  KNX_DPT_9_008, // 9.008 F16 DPT_Value_AirQuality
  KNX_DPT_FORMAT_F16, //  KNX_DPT_9_010, // 9.010 F16 DPT_Value_Time1
  KNX_DPT_FORMAT_F16, //  KNX_DPT_9_011, // 9.011 F16 DPT_Value_Time2
  KNX_DPT_FORMAT_F16, //  KNX_DPT_9_020, // 9.020 F16 DPT_Value_Volt
  KNX_DPT_FORMAT_F16, //  KNX_DPT_9_021, // 9.021 F16 DPT_Value_Curr
  KNX_DPT_FORMAT_F16, //  KNX_DPT_9_022, // 9.022 F16 DPT_PowerDensity
  KNX_DPT_FORMAT_F16, //  KNX_DPT_9_023, // 9.023 F16 DPT_KelvinPerPercent
  KNX_DPT_FORMAT_F16, //  KNX_DPT_9_024, // 9.024 F16 DPT_Power
  KNX_DPT_FORMAT_F16, //  KNX_DPT_9_025, // 9.025 F16 DPT_Value_Volume_Flow
  KNX_DPT_FORMAT_F16, //  KNX_DPT_9_026, // 9.026 F16 DPT_Rain_Amount
  KNX_DPT_FORMAT_F16, //  KNX_DPT_9_027, // 9.027 F16 DPT_Value_Temp_F
  KNX_DPT_FORMAT_F16, //  KNX_DPT_9_028, // 9.028 F16 DPT_Value_Wsp_kmh
  KNX_DPT_FORMAT_N3N5R2N6R2N6, //  KNX_DPT_10_001, // 10.001 N3N5r2N6r2N6 DPT_TimeOfDay
  KNX_DPT_FORMAT_R3N5R4N4R1U7, //  KNX_DPT_11_001, // 11.001 r3N5r4N4r1U7 DPT_Date
  KNX_DPT_FORMAT_U32, //  KNX_DPT_12_001, // 12.001 U32 DPT_Value_4_Ucount
  KNX_DPT_FORMAT_V32, //  KNX_DPT_13_001, // 13.001 V32 DPT_Value_4_Count
  KNX_DPT_FORMAT_V32, //  KNX_DPT_13_010, // 13.010 V32 DPT_ActiveEnergy
  KNX_DPT_FORMAT_V32, //  KNX_DPT_13_011, // 13.011 V32 DPT_ApparantEnergy
  KNX_DPT_FORMAT_V32, //  KNX_DPT_13_012, // 13.012 V32 DPT_ReactiveEnergy
  KNX_DPT_FORMAT_V32, //  KNX_DPT_13_013, // 13.013 V32 DPT_ActiveEnergy_kWh
  KNX_DPT_FORMAT_V32, //  KNX_DPT_13_014, // 13.014 V32 DPT_ApparantEnergy_kVAh
  KNX_DPT_FORMAT_V32, //  KNX_DPT_13_015, // 13.015 V32 DPT_ReactiveEnergy_kVARh
  KNX_DPT_FORMAT_V32, //  KNX_DPT_13_100, // 13.100 V32 DPT_LongDeltaTimeSec
  KNX_DPT_FORMAT_F32, //  KNX_DPT_14_000, // 14.000 F32 DPT_Value_Acceleration
  KNX_DPT_FORMAT_F32, //  KNX_DPT_14_001, // 14.001 F32 DPT_Value_Acceleration_Angular
  KNX_DPT_FORMAT_F32, //  KNX_DPT_14_002, // 14.002 F32 DPT_Value_Activation_Energy
  KNX_DPT_FORMAT_F32, //  KNX_DPT_14_003, // 14.003 F32 DPT_Value_Activity
  KNX_DPT_FORMAT_F32, //  KNX_DPT_14_004, // 14.004 F32 DPT_Value_Mol
  KNX_DPT_FORMAT_F32, //  KNX_DPT_14_005, // 14.005 F32 DPT_Value_Amplitude
  KNX_DPT_FORMAT_F32, //  KNX_DPT_14_006, // 14.006 F32 DPT_Value_AngleRad
  KNX_DPT_FORMAT_F32 //  KNX_DPT_14_007, // 14.007 F32 DPT_Value_AngleDeg
};

#endif // KNXDPT_H
