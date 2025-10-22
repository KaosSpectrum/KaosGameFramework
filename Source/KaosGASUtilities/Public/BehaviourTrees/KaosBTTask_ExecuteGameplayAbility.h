// Copyright (C) 2024, Daniel Moss
// 
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "BehaviorTree/BTTaskNode.h"
#include "KaosBTTask_ExecuteGameplayAbility.generated.h"

/*
 * Behaviour to do when the Behaviour Tree task is aborted
 */
UENUM(BlueprintType)
enum class EKaosExecuteGameplayAbilityAbortBehaviour : uint8
{
	CancelAbility UMETA(DisplayName = "Cancel Ability on Abort"),
	Nothing UMETA(DisplayName = "Do Nothing on Abort")
};

/**
 * Executes an ability with the supplied GameplayTag
 */
UCLASS(MinimalAPI)
class UKaosBTTask_ExecuteGameplayAbility : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UKaosBTTask_ExecuteGameplayAbility();

	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
	virtual uint16 GetInstanceMemorySize() const override;
	virtual FString GetStaticDescription() const override;

protected:
	void HandleAbilityEnded(const FAbilityEndedData& AbilityEndedData, UBehaviorTreeComponent* BehaviorTreeComponent, uint8* NodeMemory);

	/** Target we want to activate ability on (normally self) */
	UPROPERTY(EditAnywhere, Category=ExecuteGameplayAbility)
	FBlackboardKeySelector TargetActorKey;

	/** Tag of the ability we want to activate */
	UPROPERTY(EditAnywhere, Category=ExecuteGameplayAbility, meta=(Categories="AbilityTagCategory"))
	FGameplayTag AbilityToActivate;

	/** What do we do if this task is aborted */
	UPROPERTY(EditAnywhere, Category=ExecuteGameplayAbility)
	EKaosExecuteGameplayAbilityAbortBehaviour AbortBehaviour;
};
