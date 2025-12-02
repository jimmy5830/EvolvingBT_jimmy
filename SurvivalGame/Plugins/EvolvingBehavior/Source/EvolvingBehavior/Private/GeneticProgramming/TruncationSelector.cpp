#include "TruncationSelector.h"

#include "EvolvingBehavior.h"
#include "ParentSelector.h"
#include "RandomGen.h"
#include "Containers/Array.h"

TArray<FParentFitnessInfo> UTruncationSelector::SelectParents(
    TArray<FParentFitnessInfo> parentInfo,
    URandomGen* randomGen)
{
    TArray<FParentFitnessInfo> SelectedParents;

    if (parentInfo.Num() == 0 || randomGen == nullptr)
    {
        return SelectedParents;
    }

    // 1) 적합도 기준으로 정렬 (오름차순 후 Pop으로 상위부터 꺼냄: TopNFitnessSelector와 동일 패턴)
    TArray<FParentFitnessInfo> SortedParents(parentInfo);

    SortedParents.Sort([](const FParentFitnessInfo& A, const FParentFitnessInfo& B)
        {
            return A.fitness < B.fitness;
        });

    // 2) TruncationFraction 기반으로 상위 집단 크기 계산
    float ClampedFrac = FMath::Clamp(TruncationFraction, 0.0f, 1.0f);
    int32 TruncSize = FMath::FloorToInt(SortedParents.Num() * ClampedFrac);

    // 최소 1, 최대 전체 개체
    TruncSize = FMath::Clamp(TruncSize, 1, SortedParents.Num());

    // 상위 TruncSize개를 따로 배열에 보관 (fitness 높은 순서로)
    TArray<FParentFitnessInfo> TruncatedPool;
    TruncatedPool.Reserve(TruncSize);

    for (int32 i = 0; i < TruncSize; ++i)
    {
        // SortedParents는 오름차순이므로 Pop()을 쓰면 가장 fitness 높은 개체부터 나온다
        TruncatedPool.Add(SortedParents.Pop());
    }

    // 3) TruncatedPool에서 균등 랜덤으로 NumParentsToSelect개 뽑기 (중복 허용/비허용은 선택)
    int32 NumToSelect = FMath::Clamp(NumParentsToSelect, 1, TruncSize);

    for (int32 i = 0; i < NumToSelect; ++i)
    {
        int32 Index = randomGen->UniformIntInRange(0, TruncatedPool.Num() - 1);
        SelectedParents.Add(TruncatedPool[Index]);

        // 중복을 허용하지 않으려면 RemoveAt 사용
        TruncatedPool.RemoveAt(Index);

        // 더 이상 뽑을 대상이 없으면 종료
        if (TruncatedPool.Num() == 0)
        {
            break;
        }
    }

    return SelectedParents;
}
