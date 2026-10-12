#ifndef SOC_LFP_H
#define SOC_LFP_H

#include <stdint.h>

#include "../model/app_model.h"

/**
 * @brief Calculate the State of Charge (SOC) based on temperature and open circuit voltage (OCV)
 *
 * @param temperature temperature in 0.1°C
 * @param ocv open circuit voltage in mV
 * @return SOC (State Of Charge) in %
 */
float SocLfp_CalculateSoc(int16_t temperature, uint16_t ocv);

/**
 * @brief Calculate the battery capacity based on temperature
 *
 * @param temperature temperature in 0.1°C
 * @return battery capacity ratio of nominal capacity
 */
float SocLfp_CalculateCapacityRatioByTemperature(int16_t temperature);

/**
 * @brief Calculate the battery capacity based on cycle count
 *
 * @param cycle cycle count
 * @return battery capacity ratio of nominal capacity
 */
float SocLfp_CalculateCapacityByCycle(float cycle);

/**
 * @brief Check if the current conditions are in the target region for SOC estimation
 *
 * @param temperature temperature in 0.1°C
 * @param ocv open circuit voltage in mV
 * @return true if in target region, false otherwise
 */
bool SocLfp_IsCalibrationRegion(int16_t temperature, uint16_t ocv);

/**
 * @brief Check if the current conditions are in the full charge region
 *        ! must call this function in charging state
 *
 * @param temperature temperature in 0.1°C
 * @param voltage open circuit voltage in mV
 * @return true if in full charge region, false otherwise
 */

bool SocLfp_IsFullChargeRegion(int16_t temperature, uint16_t voltage);

#endif