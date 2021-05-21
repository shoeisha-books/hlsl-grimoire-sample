// 平均点、最高得点、最低得点を計算するコンピュートシェーダー
#define NUM_STUDENT 3.0f // 生徒の数

// 出力データ構造体
struct OutputData
{
    float avarageScore; // 平均点
    float maxScore;     // 最高点
    float minScore;     // 最小点
    // step-1 出力構造体にメンバーを追加する
    int totalScore;     // 合計点
    float deviation;    // 標準偏差
};

// 入力データにアクセスするための変数
StructuredBuffer<int> g_scores : register(t0);

// 出力先にアクセスするための変数
RWStructuredBuffer<OutputData> g_outputData : register(u0);

// これがエントリーポイント
[numthreads(1, 1, 1)] // この行は今は気にしなくてよい
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    g_outputData[0].maxScore = 0;
    g_outputData[0].minScore = 100;

    int totalScore = 0;
    for(int i = 0; i < NUM_STUDENT; i++)
    {
        totalScore += g_scores[i];
        g_outputData[0].maxScore = max(g_outputData[0].maxScore, g_scores[i]);
        g_outputData[0].minScore = min(g_outputData[0].minScore, g_scores[i]);
    }
    g_outputData[0].avarageScore = totalScore / NUM_STUDENT;

    // step-2 合計点を出力する
    g_outputData[0].totalScore = totalScore;
    float dev = 0.0f;
    for(int i = 0; i < NUM_STUDENT; i++)
    {
        // 平均点からの差をtに記憶する
        float t = g_scores[i] - g_outputData[0].avarageScore;

        // 平均点からの差のt^2の合計を求めていく
        dev += t * t;
    }

    // 標準偏差を求める
    g_outputData[0].deviation = sqrt(dev / NUM_STUDENT);
}
