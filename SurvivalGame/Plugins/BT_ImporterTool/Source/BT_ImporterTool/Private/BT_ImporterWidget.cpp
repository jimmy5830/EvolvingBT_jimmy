// Fill out your copyright notice in the Description page of Project Settings.

#include "BT_ImporterWidget.h"
#include "Json.h"
#include "Serialization/JsonSerializer.h"
#include "AssetRegistryModule.h"
#include "PackageTools.h"
#include "UObject/SavePackage.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BTCompositeNode.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/Composites/BTComposite_Sequence.h"
#include "BehaviorTree/Composites/BTComposite_Selector.h"
// [추가] 언리얼 엔진 기본 태스크 노드 헤더 (Wait, MoveTo)
#include "BehaviorTree/Tasks/BTTask_Wait.h"
#include "BehaviorTree/Tasks/BTTask_MoveTo.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

void UBT_ImporterWidget::NativeConstruct()
{
    Super::NativeConstruct();
    // UI 버튼 이벤트 바인딩
    if (ImportButton)
    {
        ImportButton->OnClicked.AddDynamic(this, &UBT_ImporterWidget::OnImportButtonClicked);
    }
    if (GenerateButton)
    {
        GenerateButton->OnClicked.AddDynamic(this, &UBT_ImporterWidget::OnGenerateButtonClicked);
    }
}

// ----------------------------------------------------------------
// [Part 1] LLM API 통신 로직
// ----------------------------------------------------------------

void UBT_ImporterWidget::OnGenerateButtonClicked()
{
    if (!QueryTextBox || !StatusText) return;

    FString UserQuery = QueryTextBox->GetText().ToString();
    if (UserQuery.IsEmpty())
    {
        StatusText->SetText(FText::FromString(TEXT("오류: 자연어 명령을 입력해주세요.")));
        return;
    }

    StatusText->SetText(FText::FromString(TEXT("API 호출 중... 잠시만 기다려주세요.")));

    // Upstage API 요청 시작
    SendUpstageRequest(UserQuery);
}

void UBT_ImporterWidget::SendUpstageRequest(FString UserQuery)
{
    // 1. RAG 데이터 로드 (Nodes.json)
    FString NodesJsonPath = FPaths::ProjectContentDir() / TEXT("Data/Nodes.json");
    FString NodesContent = LoadFileToString(NodesJsonPath);

    // 2. Few-Shot 예제 로드 (Dataset_sample.json)
    FString SampleJsonPath = FPaths::ProjectContentDir() / TEXT("Data/Dataset_sample.json");
    FString SampleContent = LoadFileToString(SampleJsonPath);

    // 3. 시스템 프롬프트 구성
    // RAG 데이터와 Few-Shot을 포함하여 정확한 JSON 출력을 유도함
    FString SystemPrompt = FString::Printf(TEXT(
        "You are a precise behavior tree generator. \n"
        "Available Nodes Info: %s \n"
        "Here is a Few-shot example: %s \n"
        "Respond ONLY with the raw JSON object. No markdown, no explanations."
    ), *NodesContent, *SampleContent);

    // 4. HTTP 요청 생성 및 설정
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->OnProcessRequestComplete().BindUObject(this, &UBT_ImporterWidget::OnResponseReceived);
    Request->SetURL("https://api.upstage.ai/v1/chat/completions");
    Request->SetVerb("POST");
    Request->SetHeader("Authorization", "Bearer " + UpstageApiKey);
    Request->SetHeader("Content-Type", "application/json");

    // 5. JSON Body 구성 (모델: solar-pro2)
    TSharedPtr<FJsonObject> JsonRequest = MakeShareable(new FJsonObject);
    JsonRequest->SetStringField("model", "solar-pro2");
    JsonRequest->SetBoolField("stream", false);

    TArray<TSharedPtr<FJsonValue>> Messages;

    // 시스템 메시지 추가
    TSharedPtr<FJsonObject> SystemMsg = MakeShareable(new FJsonObject);
    SystemMsg->SetStringField("role", "system");
    SystemMsg->SetStringField("content", SystemPrompt);
    Messages.Add(MakeShareable(new FJsonValueObject(SystemMsg)));

    // 사용자 메시지 추가
    TSharedPtr<FJsonObject> UserMsg = MakeShareable(new FJsonObject);
    UserMsg->SetStringField("role", "user");
    UserMsg->SetStringField("content", UserQuery);
    Messages.Add(MakeShareable(new FJsonValueObject(UserMsg)));

    JsonRequest->SetArrayField("messages", Messages);

    // 요청 전송
    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(JsonRequest.ToSharedRef(), Writer);

    Request->SetContentAsString(RequestBody);
    Request->ProcessRequest();
}

void UBT_ImporterWidget::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    // 네트워크 오류 처리
    if (!bWasSuccessful || !Response.IsValid())
    {
        StatusText->SetText(FText::FromString(TEXT("오류: API 요청 실패")));
        return;
    }

    // HTTP 상태 코드 확인
    if (Response->GetResponseCode() != 200)
    {
        StatusText->SetText(FText::FromString(FString::Printf(TEXT("오류: API 에러 코드 %d"), Response->GetResponseCode())));
        return;
    }

    // 응답 본문 파싱
    TSharedPtr<FJsonObject> ResponseObj;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

    if (FJsonSerializer::Deserialize(Reader, ResponseObj) && ResponseObj.IsValid())
    {
        const TArray<TSharedPtr<FJsonValue>>* Choices;
        if (ResponseObj->TryGetArrayField("choices", Choices) && Choices->Num() > 0)
        {
            // LLM 응답 텍스트 추출
            TSharedPtr<FJsonObject> FirstChoice = (*Choices)[0]->AsObject();
            TSharedPtr<FJsonObject> Message = FirstChoice->GetObjectField("message");
            FString Content = Message->GetStringField("content");

            // 마크다운 제거 및 순수 JSON 추출
            FString FinalJson = CleanJsonString(Content);

            // UI 결과창 업데이트
            if (JsonTextBox)
            {
                JsonTextBox->SetText(FText::FromString(FinalJson));
            }
            StatusText->SetText(FText::FromString(TEXT("성공: BT JSON 생성 완료! Import 버튼을 누르세요.")));
        }
    }
}

// 파일 읽기 유틸리티
FString UBT_ImporterWidget::LoadFileToString(FString Filename)
{
    FString Result;
    FFileHelper::LoadFileToString(Result, *Filename);
    return Result;
}

// JSON 문자열 정제 (Markdown 코드 블록 제거)
FString UBT_ImporterWidget::CleanJsonString(FString RawContent)
{
    FString Result = RawContent;
    int32 StartIdx = Result.Find("```json");
    if (StartIdx != -1)
    {
        Result = Result.RightChop(StartIdx + 7);
        int32 EndIdx = Result.Find("```", ESearchCase::IgnoreCase, ESearchDir::FromEnd);
        if (EndIdx != -1) Result = Result.Left(EndIdx);
    }
    else
    {
        StartIdx = Result.Find("```");
        if (StartIdx != -1)
        {
            Result = Result.RightChop(StartIdx + 3);
            int32 EndIdx = Result.Find("```", ESearchCase::IgnoreCase, ESearchDir::FromEnd);
            if (EndIdx != -1) Result = Result.Left(EndIdx);
        }
    }
    return Result.TrimStartAndEnd();
}


// ----------------------------------------------------------------
// [Part 2] Import 및 Asset 생성 로직
// ----------------------------------------------------------------

void UBT_ImporterWidget::OnImportButtonClicked()
{
    if (!JsonTextBox || !StatusText) return;

    // UI에 있는 JSON 텍스트 파싱
    FString JsonString = JsonTextBox->GetText().ToString();
    TSharedPtr<FJsonObject> RootJsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

    if (FJsonSerializer::Deserialize(Reader, RootJsonObject) && RootJsonObject.IsValid())
    {
        // 파싱 성공 시 에셋 생성 시작
        CreateBehaviorTreeAsset(RootJsonObject);
    }
    else
    {
        StatusText->SetText(FText::FromString(TEXT("오류: JSON 형식이 잘못되었습니다.")));
    }
}

// 재귀적 노드 생성 함수 (Factory Method)
UBTNode* UBT_ImporterWidget::CreateNodeRecursive(TSharedPtr<FJsonObject> JsonObject, UObject* Outer)
{
    FString NodeName = JsonObject->GetStringField("name");
    UBTNode* NewNode = nullptr;

    // 1. Composite Nodes (기본 컴포지트 생성)
    if (NodeName == "Sequence")
    {
        NewNode = NewObject<UBTComposite_Sequence>(Outer);
    }
    else if (NodeName == "Selector")
    {
        NewNode = NewObject<UBTComposite_Selector>(Outer);
    }
    // 2. Task Nodes (기본 제공 노드 매핑)
    else if (NodeName == "Wait")
    {
        UBTTask_Wait* WaitNode = NewObject<UBTTask_Wait>(Outer);
        // "time" 파라미터 적용
        TSharedPtr<FJsonObject> Params = JsonObject->GetObjectField("parameters");
        if (Params.IsValid() && Params->HasField("time"))
        {
            WaitNode->WaitTime = Params->GetNumberField("time");
        }
        NewNode = WaitNode;
    }
    else if (NodeName == "MoveTo" || NodeName == "MoveToNextWaypoint")
    {
        // MoveTo 계열은 언리얼 기본 MoveToTask로 매핑
        NewNode = NewObject<UBTTask_MoveTo>(Outer);
    }
    // 3. Unknown Nodes (미구현 노드에 대한 방어 로직)
    else
    {
        // LLM이 생성했으나 C++ 클래스가 없는 경우, 트리가 끊기지 않도록 Wait 노드로 대체 생성
        UBTTask_Wait* Placeholder = NewObject<UBTTask_Wait>(Outer);
        // 에디터에서 식별 가능하도록 이름에 (Unknown) 태그 추가
        Placeholder->NodeName = NodeName + TEXT(" (Unknown)");
        Placeholder->WaitTime = 0.5f;
        NewNode = Placeholder;

        // 로그 출력으로 알림
        UE_LOG(LogTemp, Warning, TEXT("Unknown Node '%s' detected. Replaced with Wait node."), *NodeName);
    }

    if (NewNode == nullptr) return nullptr;

    // --- 자식(Children) 노드 재귀 처리 ---
    // Composite 노드일 경우에만 자식을 가질 수 있음
    if (UBTCompositeNode* CompositeParent = Cast<UBTCompositeNode>(NewNode))
    {
        const TSharedPtr<FJsonObject>* ParamsObject;
        if (JsonObject->TryGetObjectField("parameters", ParamsObject))
        {
            const TArray<TSharedPtr<FJsonValue>>* ChildrenArray;
            if ((*ParamsObject)->TryGetArrayField("children", ChildrenArray))
            {
                // 자식 노드 순회 및 재귀 호출
                for (TSharedPtr<FJsonValue> ChildValue : *ChildrenArray)
                {
                    TSharedPtr<FJsonObject> ChildObject = ChildValue->AsObject();
                    if (ChildObject.IsValid())
                    {
                        UBTNode* NewChildNode = CreateNodeRecursive(ChildObject, Outer);
                        if (NewChildNode)
                        {
                            CompositeParent->Children.Add(FBTCompositeChild());
                            FBTCompositeChild& ChildInfo = CompositeParent->Children.Last();

                            // 자식 타입에 따라 적절한 슬롯에 할당
                            if (UBTCompositeNode* ChildAsComposite = Cast<UBTCompositeNode>(NewChildNode))
                            {
                                ChildInfo.ChildComposite = ChildAsComposite;
                            }
                            else if (UBTTaskNode* ChildAsTask = Cast<UBTTaskNode>(NewChildNode))
                            {
                                ChildInfo.ChildTask = ChildAsTask;
                            }
                        }
                    }
                }
            }
        }
    }

    return NewNode;
}

// 최종 BT 에셋(.uasset) 생성 및 저장 함수
void UBT_ImporterWidget::CreateBehaviorTreeAsset(TSharedPtr<FJsonObject> RootJsonObject)
{
    // 저장 경로 및 패키지 설정
    FString AssetName = "BT_ImportedFromJSON";
    FString PackagePath = "/Game/BehaviorTrees/Imports/" + AssetName;

    UPackage* Package = CreatePackage(*PackagePath);
    Package->SetDirtyFlag(true);

    // BehaviorTree 객체 생성
    UBehaviorTree* NewBehaviorTree = NewObject<UBehaviorTree>(Package, *AssetName, RF_Public | RF_Standalone);

    // 루트 노드부터 재귀적으로 트리 생성
    UBTCompositeNode* NewRootNode = Cast<UBTCompositeNode>(
        CreateNodeRecursive(RootJsonObject, NewBehaviorTree)
        );

    // 생성 결과 확인 및 저장
    if (NewRootNode)
    {
        NewBehaviorTree->RootNode = NewRootNode;
        FAssetRegistryModule::AssetCreated(NewBehaviorTree);
        FString PackageFileName = FPackageName::LongPackageNameToFilename(PackagePath, FPackageName::GetAssetPackageExtension());

        if (UPackage::SavePackage(Package, NewBehaviorTree, RF_Public | RF_Standalone, *PackageFileName))
        {
            StatusText->SetText(FText::FromString(FString::Printf(TEXT("성공: %s.uasset 생성!"), *AssetName)));
        }
        else
        {
            StatusText->SetText(FText::FromString(FString::Printf(TEXT("오류: %s.uasset 저장 실패."), *AssetName)));
        }
    }
    else
    {
        StatusText->SetText(FText::FromString(TEXT("오류: RootNode 생성 실패.")));
    }
}