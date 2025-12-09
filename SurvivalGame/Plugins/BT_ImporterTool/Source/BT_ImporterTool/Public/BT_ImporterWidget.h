#pragma once

#include "CoreMinimal.h"
#include "Editor/Blutility/Classes/EditorUtilityWidget.h"
#include "Components/MultiLineEditableText.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "BehaviorTree/BTNode.h"
#include "Http.h" 

class FJsonObject;

#include "BT_ImporterWidget.generated.h"

UCLASS()
class BT_IMPORTERTOOL_API UBT_ImporterWidget : public UEditorUtilityWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;

public:
    // --- 기존 UI ---
    UPROPERTY(meta = (BindWidget))
        UMultiLineEditableText* JsonTextBox;

    UPROPERTY(meta = (BindWidget))
        UButton* ImportButton;

    UPROPERTY(meta = (BindWidget))
        UTextBlock* StatusText;

    // --- [추가] 자연어 입력용 UI ---
    UPROPERTY(meta = (BindWidget))
        UMultiLineEditableText* QueryTextBox;

    UPROPERTY(meta = (BindWidget))
        UButton* GenerateButton;

    // --- 함수들 ---
    UFUNCTION()
        void OnImportButtonClicked();

    UFUNCTION()
        void OnGenerateButtonClicked(); // API 호출 함수

private:
    UBTNode* CreateNodeRecursive(TSharedPtr<FJsonObject> JsonObject, UObject* Outer);
    void CreateBehaviorTreeAsset(TSharedPtr<FJsonObject> RootJsonObject);

    // --- [추가] HTTP 및 헬퍼 함수 ---
    void SendUpstageRequest(FString UserQuery);
    void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
    FString LoadFileToString(FString Filename);
    FString CleanJsonString(FString RawContent);

    // API Key (보안 주의: 실제 배포시엔 암호화 필요)
    const FString UpstageApiKey = TEXT("up_84VIGWnO3gVLFMReOH66MZb2B676v");
};