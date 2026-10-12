#ifndef SOC_NCM_H
#define SOC_NCM_H

#include "../model/app_model.h"

/**
 * @brief Calculate the State of Charge (SOC) based on temperature and open circuit voltage (OCV)
 *
 * @param temperature temperature in 0.1°C
 * @param ocv open circuit voltage in mV
 * @return SOC (State Of Charge) in %
 */
float SocNcm_CalculateSoc(int16_t temperature, uint16_t ocv);

/**
 * @brief Calculate the battery capacity based on temperature
 *
 * @param temperature temperature in 0.1°C
 * @return battery capacity ratio of nominal capacity
 */
float SocNcm_CalculateCapacityRatioByTemperature(int16_t temperature);

/**
 * @brief Calculate the battery capacity based on cycle count
 *
 * @param cycle cycle count
 * @return battery capacity ratio of nominal capacity
 */
float SocNcm_CalculateCapacityByCycle(float cycle);

#endif