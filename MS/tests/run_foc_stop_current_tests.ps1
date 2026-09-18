param([string]$IarRoot = 'D:\APP\iar9401')

$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..\..')).Path
$buildDir = Join-Path $repoRoot 'Build\FocStopCurrentTests'
New-Item -ItemType Directory -Force -Path $buildDir | Out-Null
$source = Get-Content -Raw -LiteralPath (Join-Path $repoRoot 'MS\src\Ifx_MS_FocSolutionF16.c')

# Exercise the production function bodies, with other state-machine operations
# stubbed. PWM/hardware timing and complete startup require bench regression.
function Get-FunctionBody([string]$Name) {
    $match = [regex]::Match($source, '(?m)^static inline [^\r\n]*\b' + $Name + '\([^;{}]*\)\s*\{')
    if (!$match.Success) { throw "Production definition missing: $Name" }
    $depth = 1
    $end = $match.Index + $match.Length
    while (($depth -gt 0) -and ($end -lt $source.Length)) {
        if ($source[$end] -eq '{') { $depth++ }
        if ($source[$end] -eq '}') { $depth-- }
        $end++
    }
    if ($depth -ne 0) { throw "Unbalanced definition: $Name" }
    return $source.Substring($match.Index, $end - $match.Index)
}

$fixture = @'
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <float.h>
#include <math.h>
#include "Ifx_MS_FocSolutionF16_Cfg.h"
typedef int16_t Ifx_Math_Fract16;
typedef struct { int16_t real, imag; } Ifx_Math_CmpFract16;
typedef int Ifx_MDA_IToFControllerF16_Output;
typedef enum { Ifx_MS_FocSolutionF16_State_run,
    Ifx_MS_FocSolutionF16_State_rampDown, Ifx_MS_FocSolutionF16_State_standBy,
    Ifx_MS_FocSolutionF16_State_fault } Ifx_MS_FocSolutionF16_State;
typedef struct {
    Ifx_Math_CmpFract16 currentsAlphaBeta;
    int16_t rateLimitInSpeedQ15;
    bool p_enableControl, p_enablePowerStage, p_directClosedLoopHandoffActive;
} Ifx_MS_FocSolutionF16;
static unsigned substateCalls;
/* OFF-path regression: HFIPD startup cancellation is covered separately. */
static uint8_t HFIPDInjection_isActive(void) { return 0u; }
static void Ifx_MS_FocSolutionF16_limitSpeed(Ifx_MS_FocSolutionF16 *s, int16_t speed)
{ (void)s; (void)speed; } /* Keep the injected ramp output for boundary tests. */
static void Ifx_MS_FocSolutionF16_subStateMachine(Ifx_MS_FocSolutionF16 *s,
    int output, Ifx_Math_CmpFract16 dq)
{ (void)s; (void)output; (void)dq; ++substateCalls; }
static int32_t Ifx_Math_Abs_F16(int16_t x) { return x < 0 ? -(int32_t)x : x; }
'@
$cal = [regex]::Match($source, 'volatile float Cal_FOC_StopIsHi_A_f32 = [^;]+;').Value
$minSpeed = [regex]::Match($source, '(?m)^#define IFX_MS_FOCSOLUTIONF16_MIN_SPEED[^\r\n]+').Value
if (!$cal -or !$minSpeed) { throw 'Production calibration or speed threshold missing' }
$fixture += "`n$cal`n$minSpeed`n"
foreach ($name in @('Ifx_MS_FocSolutionF16_stopCurrentExceeded',
    'Ifx_MS_FocSolutionF16_stateRun', 'Ifx_MS_FocSolutionF16_stateRampDown')) {
    $fixture += (Get-FunctionBody $name) + "`n"
}
$fixture += @'
static unsigned checks;
#define CHECK(x) do { ++checks; if (!(x)) { printf("FAIL line %d\n", __LINE__); return 1; } } while (0)
int main(void) {
    Ifx_MS_FocSolutionF16 s = {0};
    Ifx_Math_CmpFract16 dq = {0};
    int sign;
    CHECK(Cal_FOC_StopIsHi_A_f32 == 25.0F);
    CHECK(IFX_MS_FOCSOLUTIONF16_CFG_BASE_CURRENT_A == 50.0);
    for (sign = -1; sign <= 1; sign += 2) {
        s.currentsAlphaBeta.real = sign * 16383;
        CHECK(!Ifx_MS_FocSolutionF16_stopCurrentExceeded(&s));
        s.currentsAlphaBeta.real = sign * 16384; /* Exactly 25 A: strict >. */
        CHECK(!Ifx_MS_FocSolutionF16_stopCurrentExceeded(&s));
        s.currentsAlphaBeta.real = sign * 16385;
        CHECK(Ifx_MS_FocSolutionF16_stopCurrentExceeded(&s));
    }
    s.currentsAlphaBeta.real = 13107; s.currentsAlphaBeta.imag = -13107;
    CHECK(Ifx_MS_FocSolutionF16_stopCurrentExceeded(&s)); /* Combined >25 A. */
    s.p_enableControl = true; s.p_enablePowerStage = true;
    s.p_directClosedLoopHandoffActive = true; s.rateLimitInSpeedQ15 = 25000;
    CHECK(Ifx_MS_FocSolutionF16_stateRun(&s, 25000, false, dq, 0) == Ifx_MS_FocSolutionF16_State_run);
    CHECK(substateCalls == 1 && s.p_directClosedLoopHandoffActive);
    s.p_enableControl = false;
    CHECK(Ifx_MS_FocSolutionF16_stateRun(&s, 25000, false, dq, 0) == Ifx_MS_FocSolutionF16_State_standBy);
    CHECK(substateCalls == 1 && !s.p_directClosedLoopHandoffActive);
    s.p_enableControl = true; s.p_enablePowerStage = false;
    CHECK(Ifx_MS_FocSolutionF16_stateRun(&s, 25000, false, dq, 0) == Ifx_MS_FocSolutionF16_State_standBy);
    CHECK(Ifx_MS_FocSolutionF16_stateRun(&s, 25000, true, dq, 0) == Ifx_MS_FocSolutionF16_State_fault);
    Cal_FOC_StopIsHi_A_f32 = 30.0F;
    CHECK(Ifx_MS_FocSolutionF16_stateRun(&s, 25000, false, dq, 0) == Ifx_MS_FocSolutionF16_State_rampDown);
    CHECK(Ifx_MS_FocSolutionF16_stateRampDown(&s, false, dq, 0) == Ifx_MS_FocSolutionF16_State_rampDown);
    CHECK(substateCalls == 2);
    Cal_FOC_StopIsHi_A_f32 = 25.0F;
    CHECK(Ifx_MS_FocSolutionF16_stateRampDown(&s, false, dq, 0) == Ifx_MS_FocSolutionF16_State_standBy);
    CHECK(Ifx_MS_FocSolutionF16_stateRampDown(&s, true, dq, 0) == Ifx_MS_FocSolutionF16_State_fault);
    s.currentsAlphaBeta.real = 0; s.currentsAlphaBeta.imag = 0;
    for (sign = -1; sign <= 1; sign += 2) {
        s.rateLimitInSpeedQ15 = sign * (IFX_MS_FOCSOLUTIONF16_MIN_SPEED + 1);
        CHECK(Ifx_MS_FocSolutionF16_stateRampDown(&s, false, dq, 0) == Ifx_MS_FocSolutionF16_State_rampDown);
        s.rateLimitInSpeedQ15 = sign * IFX_MS_FOCSOLUTIONF16_MIN_SPEED;
        CHECK(Ifx_MS_FocSolutionF16_stateRampDown(&s, false, dq, 0) == Ifx_MS_FocSolutionF16_State_standBy);
    }
    s.currentsAlphaBeta.real = -32768; s.currentsAlphaBeta.imag = -32768;
    CHECK(Ifx_MS_FocSolutionF16_stopCurrentExceeded(&s));
    Cal_FOC_StopIsHi_A_f32 = 0.0F;
    CHECK(!Ifx_MS_FocSolutionF16_stopCurrentExceeded(&s));
    Cal_FOC_StopIsHi_A_f32 = -1.0F;
    CHECK(!Ifx_MS_FocSolutionF16_stopCurrentExceeded(&s));
    Cal_FOC_StopIsHi_A_f32 = NAN;
    CHECK(!Ifx_MS_FocSolutionF16_stopCurrentExceeded(&s));
    Cal_FOC_StopIsHi_A_f32 = INFINITY;
    CHECK(!Ifx_MS_FocSolutionF16_stopCurrentExceeded(&s));
    Cal_FOC_StopIsHi_A_f32 = FLT_MAX;
    CHECK(!Ifx_MS_FocSolutionF16_stopCurrentExceeded(&s));
    printf("FOC stop current tests passed: %u checks\n", checks);
    return 0;
}
'@
$testSource = Join-Path $buildDir 'foc_stop_current_test.c'
$testObject = Join-Path $buildDir 'foc_stop_current_test.o'
$testElf = Join-Path $buildDir 'foc_stop_current_test.elf'
Set-Content -LiteralPath $testSource -Value $fixture -Encoding ascii
& "$IarRoot\arm\bin\iccarm.exe" $testSource -o $testObject --debug --endian=little `
    --cpu=Cortex-M4 -e --fpu=VFPv4_sp --dlib_config "$IarRoot\arm\inc\c\DLib_Config_Normal.h" `
    -I (Join-Path $repoRoot 'ConfigWizard') -Oh
if ($LASTEXITCODE -ne 0) { throw 'FOC stop test compile failed' }
& "$IarRoot\arm\bin\ilinkarm.exe" $testObject --no_out_extension -o $testElf `
    --config_def _CORE_cm4_=0 --config_def _LINK_flash_=0 `
    --config (Join-Path $repoRoot 'LinkerScript\linker_directives_tviibe512k.icf') `
    --semihosting --entry __iar_program_start --cpu=Cortex-M4 --fpu=VFPv4_sp
if ($LASTEXITCODE -ne 0) { throw 'FOC stop test link failed' }
$output = & "$IarRoot\common\bin\CSpyBat.exe" "$IarRoot\arm\bin\armproc.dll" `
    "$IarRoot\arm\bin\armsim2.dll" $testElf --plugin "$IarRoot\arm\bin\armbat.dll" `
    --timeout 10000 --backend --cpu Cortex-M4 --fpu VFPv4_sp 2>&1 | Out-String
Set-Content -LiteralPath (Join-Path $buildDir 'result.txt') -Value $output
if (($LASTEXITCODE -ne 0) -or ($output -notmatch 'FOC stop current tests passed')) {
    throw "FOC stop tests failed: $output"
}
Write-Output $output
