/*
 * Copyright � 2021 Infineon Technologies AG. All Rights Reserved.
 *
 * Use of this file is subject to the terms of use of the Evaluation Software License Agreement
 * distributed along with this file within the software delivery package.
 *
 */

#include "Ifx_Math_CurrentReconstruction.h"
#include "Ifx_Math_SubSat.h"
#include "Ifx_Math_NegSat.h"
#include "Ifx_Math_AddSat.h"

/* polyspace-begin CODE-METRIC:VOCF [Justified:Low] "For readability and performance, switch case implementation is
 * justified"
 * */
//Ifx_Math_3PhaseFract16 Ifx_Math_CurrentReconstruction_F16(Ifx_Math_CurrentReconstruction_info info,
//                                                          Ifx_Math_Fract16                  * currentMeasurements)
//{
//    /* Local variable for storing the return value */
//    Ifx_Math_3PhaseFract16 threePhaseCurr;
//
//    /* Local variables for storing the phase current values */
//    Ifx_Math_Fract16       firCurr;
//    Ifx_Math_Fract16       secCurr;
//    Ifx_Math_Fract16       thrCurr;
//
//    /* polyspace +2 MISRA2012:D4.14 [Justified:Low] "The caller function has to guarantee that NULL is not passed as
//     * argument." */
//    firCurr = currentMeasurements[0];
//
//    /* Negation of the second measured current */
//    secCurr = currentMeasurements[1];
//
//    /* Subtraction of the measured current to calculate third phase current */
//    /*
//    thrCurr = Ifx_Math_AddSat_F16(firCurr, secCurr);
//    thrCurr = Ifx_Math_NegSat_F16(thrCurr);
//    */
//    thrCurr =  Ifx_Math_SubSat_F16(0, Ifx_Math_AddSat_F16(firCurr, secCurr));
//
//    threePhaseCurr.u = firCurr;
//
//
//    threePhaseCurr.v = secCurr;
//
//
//    threePhaseCurr.w = thrCurr;
//
//
//    return threePhaseCurr;
//}

//Ifx_Math_3PhaseFract16 Ifx_Math_CurrentReconstruction_F16(Ifx_Math_CurrentReconstruction_info info,
//                                                          Ifx_Math_Fract16                  * currentMeasurements)
//{
//    Ifx_Math_3PhaseFract16 threePhaseCurr;
//    Ifx_Math_Fract16       firCurr;
//    Ifx_Math_Fract16       secCurr;
//    Ifx_Math_Fract16       thrCurr;
//
//    firCurr = Ifx_Math_NegSat_F16(currentMeasurements[0]); //【关键】根据硬件极性取舍Neg
//    secCurr = Ifx_Math_NegSat_F16(currentMeasurements[1]);
//
//    thrCurr =  Ifx_Math_SubSat_F16(0, Ifx_Math_AddSat_F16(firCurr, secCurr));
//
//    threePhaseCurr.u = firCurr;
//    threePhaseCurr.v = secCurr;
//    threePhaseCurr.w = thrCurr;
//
//    return threePhaseCurr;
//}
//
//Ifx_Math_3PhaseFract16 Ifx_Math_CurrentReconstruction_F16(Ifx_Math_CurrentReconstruction_info info,
//                                                          Ifx_Math_Fract16                  * currentMeasurements)
//{
//    Ifx_Math_3PhaseFract16 threePhaseCurr;
//    Ifx_Math_Fract16       firCurr;
//    Ifx_Math_Fract16       secCurr;
//    Ifx_Math_Fract16       thrCurr;
//
//    //方案1：全部取反（优先试）
//    firCurr = Ifx_Math_NegSat_F16(currentMeasurements[0]);
//    secCurr = Ifx_Math_NegSat_F16(currentMeasurements[1]);
//    
//    thrCurr = Ifx_Math_SubSat_F16(0, Ifx_Math_AddSat_F16(firCurr,secCurr));
//    
//    
//    threePhaseCurr.u = firCurr;
//    threePhaseCurr.v = secCurr;
//    threePhaseCurr.w = thrCurr;
//
//    return threePhaseCurr;
//}

//Ifx_Math_3PhaseFract16 Ifx_Math_CurrentReconstruction_F16(Ifx_Math_CurrentReconstruction_info info,
//                                                          Ifx_Math_Fract16                  * currentMeasurements)//这个方案是闭环开环来回转动
//{
//    Ifx_Math_3PhaseFract16 threePhaseCurr;
//    Ifx_Math_Fract16       firCurr;
//    Ifx_Math_Fract16       secCurr;
//    Ifx_Math_Fract16       thrCurr;
//
//    // 固定取反不能删
//    firCurr = Ifx_Math_NegSat_F16(currentMeasurements[0]);
//    secCurr = Ifx_Math_NegSat_F16(currentMeasurements[1]);
//
//    // Q15右移1位 = *0.5，防止Iu+Iv溢出饱和
//    firCurr >>= 1;
//    secCurr >>= 1;
//
//    // Iw = -(Iu+Iv) 不变
//    thrCurr = Ifx_Math_SubSat_F16(0, Ifx_Math_AddSat_F16(firCurr, secCurr));
//
//    threePhaseCurr.u = firCurr;
//    threePhaseCurr.v = secCurr;
//    threePhaseCurr.w = thrCurr;
//
//    return threePhaseCurr;
//}

Ifx_Math_3PhaseFract16 Ifx_Math_CurrentReconstruction_F16(Ifx_Math_CurrentReconstruction_info info,
                                                          Ifx_Math_Fract16                  * currentMeasurements)//这个方案是闭环开环来回转动
{
    Ifx_Math_3PhaseFract16 threePhaseCurr;
    Ifx_Math_Fract16       firCurr;
    Ifx_Math_Fract16       secCurr;
    Ifx_Math_Fract16       thrCurr;

    // 固定取反不能删
    firCurr = Ifx_Math_NegSat_F16(currentMeasurements[0]);
    secCurr = Ifx_Math_NegSat_F16(currentMeasurements[1]);

    // Q15右移1位 = *0.5，防止Iu+Iv溢出饱和
//    firCurr >>= 1;
//    secCurr >>= 1;
//    

    // Iw = -(Iu+Iv) 不变
    thrCurr = Ifx_Math_SubSat_F16(0, Ifx_Math_AddSat_F16(firCurr, secCurr));

    threePhaseCurr.u = firCurr;
    threePhaseCurr.v = secCurr;
    threePhaseCurr.w = thrCurr;

    return threePhaseCurr;
}
/* polyspace-end CODE-METRIC:VOCF [Justified:Low] "For readability and performance, switch case implementation is
 * justified"
 * */
