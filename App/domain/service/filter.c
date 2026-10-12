#include "filter.h"

// Exponential Moving Average (EMA) filter
// α = dt / (τ + dt)
// dt : sampling time (s)
// τ : desired time constant (s, filter response speed)

float Filter_Ema(float previous_data, float new_data, float alpha) {
    return alpha * new_data + (1 - alpha) * previous_data;
}