#include <float.h>
#include <stdint.h>
#include <stdio.h>

#include "apsfsm_ert_replay_bridge.h"

#define APSFSM_ERT_RECORD_FLOATS (19u)
#define APSFSM_ERT_TOLERANCE     (2.0e-5F)

static int APSFSM_ErtReplay_isFinite(float value)
{
    return (value == value) && (value <= FLT_MAX) && (value >= -FLT_MAX);
}

static float APSFSM_ErtReplay_absolute(float value)
{
    return (value < 0.0F) ? -value : value;
}

int main(int argumentCount, char *arguments[])
{
    FILE *vectorFile;
    uint32_t sampleCount;
    uint32_t sampleIndex;
    float maximumStateError = 0.0F;
    float maximumDiagnosticError = 0.0F;
    unsigned int allFinite = 1u;

    if (argumentCount != 2)
    {
        (void)fprintf(stderr, "usage: apsfsm_ert_replay <vectors.bin>\n");
        return 2;
    }

    vectorFile = fopen(arguments[1], "rb");
    if (vectorFile == NULL)
    {
        (void)fprintf(stderr, "unable to open replay vectors\n");
        return 3;
    }

    if (fread(&sampleCount, sizeof(sampleCount), 1u, vectorFile) != 1u)
    {
        (void)fclose(vectorFile);
        return 4;
    }

    for (sampleIndex = 0u; sampleIndex < sampleCount; sampleIndex++)
    {
        float record[APSFSM_ERT_RECORD_FLOATS];
        float state[4];
        float diagnostics[4];
        uint32_t valueIndex;

        if (fread(record, sizeof(float), APSFSM_ERT_RECORD_FLOATS,
                  vectorFile) != APSFSM_ERT_RECORD_FLOATS)
        {
            (void)fclose(vectorFile);
            return 5;
        }

        APSFSM_ErtReplay_step((uint8_t)record[0], record[1], record[2],
            &record[3], &record[8], state, diagnostics);

        for (valueIndex = 0u; valueIndex < 4u; valueIndex++)
        {
            const float stateError = APSFSM_ErtReplay_absolute(
                state[valueIndex] - record[11u + valueIndex]);
            const float diagnosticError = APSFSM_ErtReplay_absolute(
                diagnostics[valueIndex] - record[15u + valueIndex]);

            if (stateError > maximumStateError)
            {
                maximumStateError = stateError;
            }
            if (diagnosticError > maximumDiagnosticError)
            {
                maximumDiagnosticError = diagnosticError;
            }
            if ((APSFSM_ErtReplay_isFinite(state[valueIndex]) == 0)
                || (APSFSM_ErtReplay_isFinite(diagnostics[valueIndex]) == 0))
            {
                allFinite = 0u;
            }
        }
    }

    (void)fclose(vectorFile);
    (void)printf("ERT replay: samples=%u max_state=%.9g max_diag=%.9g finite=%u\n",
        sampleCount, maximumStateError, maximumDiagnosticError, allFinite);

    if ((maximumStateError > APSFSM_ERT_TOLERANCE)
        || (maximumDiagnosticError > APSFSM_ERT_TOLERANCE)
        || (allFinite == 0u))
    {
        return 1;
    }

    return 0;
}
