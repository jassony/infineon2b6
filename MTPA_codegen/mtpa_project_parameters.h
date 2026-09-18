#ifndef MTPA_PROJECT_PARAMETERS_H
#define MTPA_PROJECT_PARAMETERS_H

#include <stdint.h>

typedef struct
{
    float baseCurrentA;
    float baseElectricalSpeedRadps;
    float baseTorqueNm;
    float basePowerW;
    float permanentMagnetFluxWb;
    float directInductanceH;
    float quadratureInductanceH;
    uint8_t polePairs;
    uint8_t valid;
} MTPA_ProjectParameters;

uint8_t MTPA_ProjectParameters_set(float baseCurrentA,
                                   float baseElectricalSpeedRadps,
                                   float baseTorqueNm,
                                   float basePowerW,
                                   float permanentMagnetFluxWb,
                                   float directInductanceH,
                                   float quadratureInductanceH,
                                   uint8_t polePairs);

const MTPA_ProjectParameters *MTPA_ProjectParameters_get(void);

#endif /* MTPA_PROJECT_PARAMETERS_H */
