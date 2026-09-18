#include "mtpa_project_parameters.h"

static MTPA_ProjectParameters mtpaProjectParameters;

uint8_t MTPA_ProjectParameters_set(float baseCurrentA,
                                   float baseElectricalSpeedRadps,
                                   float baseTorqueNm,
                                   float basePowerW,
                                   float permanentMagnetFluxWb,
                                   float directInductanceH,
                                   float quadratureInductanceH,
                                   uint8_t polePairs)
{
    if ((baseCurrentA <= 0.0F)
        || (baseElectricalSpeedRadps <= 0.0F)
        || (baseTorqueNm <= 0.0F)
        || (basePowerW <= 0.0F)
        || (permanentMagnetFluxWb <= 0.0F)
        || (directInductanceH < 0.0F)
        || (quadratureInductanceH < 0.0F)
        || (polePairs == 0u))
    {
        mtpaProjectParameters.valid = 0u;
        return 0u;
    }

    mtpaProjectParameters.baseCurrentA = baseCurrentA;
    mtpaProjectParameters.baseElectricalSpeedRadps = baseElectricalSpeedRadps;
    mtpaProjectParameters.baseTorqueNm = baseTorqueNm;
    mtpaProjectParameters.basePowerW = basePowerW;
    mtpaProjectParameters.permanentMagnetFluxWb = permanentMagnetFluxWb;
    mtpaProjectParameters.directInductanceH = directInductanceH;
    mtpaProjectParameters.quadratureInductanceH = quadratureInductanceH;
    mtpaProjectParameters.polePairs = polePairs;
    mtpaProjectParameters.valid = 1u;

    return 1u;
}

const MTPA_ProjectParameters *MTPA_ProjectParameters_get(void)
{
    return &mtpaProjectParameters;
}
