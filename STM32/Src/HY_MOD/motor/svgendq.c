#include "HY_MOD/motor/svgendq.h"
#ifdef HY_MOD_STM32_MOTOR

#include "HY_MOD/main/variable_cal.h"

// Sector 0: this is special case for (Ualpha,Ubeta) = (0,0)
// inline void SVGEN_run(SVGENDQ *svgq)
// {
//     float32_t t1 = svgq->Ualpha * 0.5f;
//     float32_t t2 = svgq->Ubeta * SQRT3_DIV_2;
//     // Inverse clarke transformation
//     svgq->Va = svgq->Ualpha;
//     svgq->Vb = -t1 + t2;
//     svgq->Vc = -t1 - t2;
//     // 60 degree Sector determination
//     svgq->Sector = 0;
//     if (svgq->Va > 0.0f) svgq->Sector += 4;
//     if (svgq->Vb > 0.0f) svgq->Sector += 2;
//     if (svgq->Vc > 0.0f) svgq->Sector += 1;
// }

inline void SVGEN_run(SVGENDQ *svgq)
{
    // 構造三個輔助變數，用於精確將扇區切分在 0, 60, 120, 180, 240, 300 度
    float32_t temp = SQRT3 * svgq->Ualpha;

    // 扇區判斷專用向量
    svgq->Va = temp - svgq->Ubeta;
    svgq->Vb = svgq->Ubeta;
    svgq->Vc = -temp - svgq->Ubeta;

    // 60度扇區判定 (Sector determination)
    svgq->Sector = 0;
    if (svgq->Va > 0.0f) svgq->Sector += 4;
    if (svgq->Vb > 0.0f) svgq->Sector += 2;
    if (svgq->Vc > 0.0f) svgq->Sector += 1;
}

#endif