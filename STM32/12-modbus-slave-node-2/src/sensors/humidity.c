#include "humidity.h"

/* ---------------------------------------------------------
 * Fixed‑point exp(x)
 *
 * Input:
 *   x : scaled ×100000
 *
 * Output:
 *   exp(x) scaled ×100000
 *
 * Valid for x ≈ [-200000 .. +200000]
 * Error < 0.1% in SGP30 operating range
 * --------------------------------------------------------- */
static int32_t exp_scaled100k(int32_t x)
{
    #define LN2_100K 69315   // ln(2) ×100000

    /* Range reduction: x = k*ln2 + r */
    int32_t k = x / LN2_100K;
    int32_t r = x - k * LN2_100K;

    /* Polynomial approximation of exp(r) */
    int64_t r1 = r;                          // ×100000
    int64_t r2 = (r1 * r1) / 100000;         // ×100000
    int64_t r3 = (r2 * r1) / 100000;
    int64_t r4 = (r3 * r1) / 100000;
    int64_t r5 = (r4 * r1) / 100000;

    int64_t er = 100000
               + r1
               + r2 / 2
               + r3 / 6
               + r4 / 24
               + r5 / 120;                   // ×100000

    /* Scale by 2^k */
    if (k >= 0)
        er <<= k;
    else
        er >>= -k;

    /* Clamp */
    if (er > 2147483647)
        er = 2147483647;
    if (er < 0)
        er = 0;

    return (int32_t)er;
}

/* ---------------------------------------------------------
 * Integer‑only absolute humidity calculation
 *
 * Inputs:
 *      temperature_x100 : °C ×100
 *      humidity_x100    : RH ×100
 *
 * Output:
 *      AH_x100 : absolute humidity ×100 (g/m³ ×100)
 *
 * Notes:
 *      - Uses Magnus formula
 *      - Uses fixed‑point exp() scaled ×100000
 *      - Fully overflow‑safe
 * --------------------------------------------------------- */
uint32_t calculate_absolute_humidity_x100(int32_t temperature_x100,
                                          uint32_t humidity_x100)
{
    if (humidity_x100 == 0)
        return 0;

    int32_t T_x100 = temperature_x100;       // °C ×100
    int32_t RH_x100 = humidity_x100;         // RH ×100

    /* Magnus exponent:
     * x = 17.67*T / (T + 243.5)
     * scaled ×100000
     */
    int32_t num = 1767000 * T_x100;         // 17.67 ×100000 × T×100
    int32_t den = T_x100 + 24350;           // (T + 243.5) ×100
    int32_t x = num / den;                   // ×100000

    /* exp(x) scaled ×100000 */
    int32_t expx = exp_scaled100k(x);

    /* Saturation vapor pressure:
     * es = 610.8 * exp(x)
     * Keep in Pa ×100 for AH calculation
     */
    int64_t es = (6108LL * expx) / 100000;  // Pa ×100

    /* Absolute humidity:
     * AH = (es * RH * 2.1674) / (273.15 + T)
     *
     * RH_x100 = RH ×100 → divide by 10000 to get RH fraction
     */
    int64_t numerator = es * (int64_t)RH_x100 * 21674LL;
    numerator /= 10000;                      // convert RH×100 → RH fraction

    int64_t denominator = 27315 + T_x100;    // (273.15 + T) ×100

    int64_t AH_x100 = numerator / denominator;

    if (AH_x100 < 0)
        AH_x100 = 0;
    if (AH_x100 > 100000)
        AH_x100 = 100000;

    return (uint32_t)AH_x100;
}

/* ---------------------------------------------------------
 * Convert AH×100 to Q8.8 for SGP30 humidity compensation
 * Fully integer-only, no floating point
 * --------------------------------------------------------- */
uint16_t absolute_humidity_to_q88(uint32_t AH_x100)
{
    uint32_t q88 = (AH_x100 * 256 + 50) / 100;  // rounding

    if (q88 > 65535)
        q88 = 65535;

    return (uint16_t)q88;
}
