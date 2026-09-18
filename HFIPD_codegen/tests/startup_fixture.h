#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "hfipd_injection.h"
#include "hfipd_vectors.h"
typedef uint8_t uint8;typedef uint16_t uint16;typedef uint32_t uint32;
typedef int16_t Ifx_Math_Fract16;
typedef struct {int16_t real,imag;} Ifx_Math_CmpFract16;
typedef struct {int16_t amplitude;uint32_t angle;} Ifx_Math_PolarFract16;
typedef struct {int16_t dcLinkVoltageQ15;} Ifx_MHA_MeasurementADC_CYT2B7_Output;
typedef int Ifx_MDA_IToFControllerF16_Output;
typedef enum {Ifx_MS_FocSolutionF16_State_off=1,Ifx_MS_FocSolutionF16_State_standBy=2,
 Ifx_MS_FocSolutionF16_State_fault=3,Ifx_MS_FocSolutionF16_State_run=4,
 Ifx_MS_FocSolutionF16_State_rampDown=5} Ifx_MS_FocSolutionF16_State;
enum {Ifx_MS_FocSolutionF16_ControlMode_foc,Ifx_MS_FocSolutionF16_ControlMode_vToF};
typedef struct {uint32_t angle;} IToF;
typedef struct {
 struct {Ifx_MS_FocSolutionF16_State state;int actualControlMode,subState;} p_status;
 IToF iToF;uint32_t angle;
 Ifx_Math_CmpFract16 dqCommand,currentsAlphaBeta,openDq;
 bool openEnabled,p_enablePowerStage,p_enableControl,p_directClosedLoopHandoffActive;
 struct {bool p_forceDutyEnable;} modulator;
 int16_t speed;
} Ifx_MS_FocSolutionF16;
static Ifx_MS_FocSolutionF16 FocDemoClosedLoop;
static uint16 Cal_FocAlignmentDuration_cnt_u16=3,rotorAlignCounter;
static uint8 Cal_FocStartupMode_u8,enableControl,enablePowerStage;
static int16_t Cal_FocAlignmentCurrentD_Q15_s16=0,Cal_FocAlignmentCurrentQ_Q15_s16=100;
static int16_t Cal_FocAlignmentCurrentStartD_Q15_s16=0,Cal_FocAlignmentCurrentStartQ_Q15_s16=0;
static int16_t referenceSpeedQ0;
static bool fault,observerValid;
static unsigned handoffCalls,observerCalls,regulationCalls,seedCalls;
static int inputMode;
static Ifx_Math_PolarFract16 applied;
#define IFX_MS_FOCSOLUTIONF16_CFG_BASE_MECH_SPEED_RPM 10000
#define IFX_MS_FOCSOLUTIONF16_BASE_CURRENT_A 50.0F
#define IFX_MS_FOCSOLUTIONF16_BASE_VOLTAGE_V 1000.0F
#define MCU_FAST_MARK(x) ((void)0)
static int16_t Ifx_Math_DivSat_F16(int a,int b){return (int16_t)(a*32768/b);}
static void Ifx_MS_FocSolutionF16_setIToFCurrentReference(Ifx_MS_FocSolutionF16 *s,Ifx_Math_CmpFract16 d){(void)s;(void)d;}
static void Ifx_MS_FocSolutionF16_setOpenLoopDqReference(Ifx_MS_FocSolutionF16 *s,Ifx_Math_CmpFract16 d){s->openDq=d;}
static void Ifx_MS_FocSolutionF16_enableOpenLoopDqReference(Ifx_MS_FocSolutionF16 *s,bool e){s->openEnabled=e;}
static void Ifx_MDA_IToFControllerF16_setAnglePreviousValue(IToF *s,uint32 a){s->angle=a;seedCalls++;}
static bool Ifx_MS_FocSolutionF16_enterClosedLoopFromAlignment(Ifx_MS_FocSolutionF16 *s,int16_t r)
{(void)r;handoffCalls++;if(!observerValid)return false;s->p_status.subState=1;return true;}
static void Ifx_MS_FocSolutionF16_limitSpeed(Ifx_MS_FocSolutionF16*s,int16_t v){s->speed=v;}
static bool Ifx_MS_FocSolutionF16_stopCurrentExceeded(const Ifx_MS_FocSolutionF16*s){(void)s;return false;}
static void Ifx_MS_FocSolutionF16_subStateMachine(Ifx_MS_FocSolutionF16*s,int o,Ifx_Math_CmpFract16 d)
{(void)o;(void)d;s->dqCommand=s->openEnabled?s->openDq:(Ifx_Math_CmpFract16){0,50};}
static Ifx_MHA_MeasurementADC_CYT2B7_Output Ifx_MS_FocSolutionF16_measureAndReconstruct(Ifx_MS_FocSolutionF16*s)
{(void)s;return (Ifx_MHA_MeasurementADC_CYT2B7_Output){3277};}
static uint32 Ifx_MS_FocSolutionF16_estimatePositionAndSpeed(Ifx_MS_FocSolutionF16*s)
{(void)s;observerCalls++;return 123u;}
static Ifx_Math_PolarFract16 Ifx_MS_FocSolutionF16_regulationLoop(Ifx_MS_FocSolutionF16*s,uint32 a)
{(void)a;regulationCalls++;return (Ifx_Math_PolarFract16){123,s->iToF.angle};}
static Ifx_Math_PolarFract16 Ifx_MS_FocSolutionF16_vToFLoop(Ifx_MS_FocSolutionF16*s)
{(void)s;return (Ifx_Math_PolarFract16){321,0};}
static void Ifx_MS_FocSolutionF16_voltageGeneration(Ifx_MS_FocSolutionF16*s,Ifx_Math_PolarFract16 v,Ifx_MHA_MeasurementADC_CYT2B7_Output m)
{(void)s;(void)m;applied=v;}
