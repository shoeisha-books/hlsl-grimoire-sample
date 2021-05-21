#include "stdafx.h"
#include "system/system.h"
#include "sub.h"


struct OutputData
{
    float avarageScore; // 平均点
    float maxScore;     // 最高得点
    float minScore;     // 最小得点

    // step-3 出力構造体にメンバーを追加する
    int totalScore;     // 合計点
};

///////////////////////////////////////////////////////////////////
// ウィンドウプログラムのメイン関数
///////////////////////////////////////////////////////////////////
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    // ゲームの初期化
    InitGame(hInstance, hPrevInstance, lpCmdLine, nCmdShow, TEXT("Game"));

    //////////////////////////////////////
    // ここから初期化を行うコードを記述する
    //////////////////////////////////////

    // コンピュートシェーダのロード
    Shader cs;
    cs.LoadCS("Assets/shader/sample.fx", "CSMain");

    RootSignature rs;
    InitRootSignature(rs, cs);

    PipelineState pipelineState;
    InitPipelineState(rs, pipelineState, cs);

    // 入力データを受け取るバッファーを作成
    int inputData[] = {
        20, 30, 40
    };

    StructuredBuffer inputSB;
    inputSB.Init(sizeof(int), 3, inputData);

    // 出力データを受け取るバッファーを作成
    RWStructuredBuffer outputSb;
    outputSb.Init(sizeof(OutputData), 1, nullptr);

    // 入力データと出力データをディスクリプタヒープに登録する
    DescriptorHeap ds;
    ds.RegistShaderResource(0, inputSB);
    ds.RegistUnorderAccessResource(0, outputSb);
    ds.Commit();
    //////////////////////////////////////
    // 初期化を行うコードを書くのはここまで！！！
    //////////////////////////////////////
    auto& renderContext = g_graphicsEngine->GetRenderContext();

    // ここからゲームループ
    while (DispatchWindowMessage())
    {
        // フレーム開始
        g_engine->BeginFrame();
        //////////////////////////////////////
        // 入力する成績データをランダムに生成する
        //////////////////////////////////////
        for (int i = 0; i < 3; i++)
        {
            inputData[i] = rand() % 101;
        }

        inputSB.Update(inputData);

        //////////////////////////////////////
        // ここからDirectComputeへのディスパッチ命令
        //////////////////////////////////////
        renderContext.SetComputeRootSignature(rs);
        renderContext.SetPipelineState(pipelineState);
        renderContext.SetComputeDescriptorHeap(ds);
        renderContext.Dispatch(1, 1, 1);

        // フレーム終了
        g_engine->EndFrame();

        // 平均点、最高得点、最低得点を表示する
        char text[256];
        OutputData* outputData = (OutputData*)outputSb.GetResourceOnCPU();
        // step-4 合計点を表示する
        sprintf(
            text,
            "１人目 = %d\n" \
            "２人目 = %d\n" \
            "３人目 = %d\n" \
            "平均点 = %0.2f\n" \
            "最高得点=%0.2f\n" \
            "最低得点=%0.2f\n" \
            "合計点=%d\n", // これを追加
            inputData[0],
            inputData[1],
            inputData[2],
            outputData->avarageScore,
            outputData->maxScore,
            outputData->minScore,
            outputData->totalScore // これも追加
        );
        MessageBoxA(nullptr, text, "成績発表", MB_OK);
    }
    return 0;
}

