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
#include "BehaviorTree/BTService.h"
#include "GameplayTagContainer.h"
#include "KaosBTService_ActivateAbilityByTag.generated.h"

/*
 * Behaviour to do when the Behaviour Service ceases relevance
 */
UENUM(BlueprintType)
enum class EKaosActivateAbilityByTagOnCeaseRelevanceBehaviour : uint8
{
	CancelAbility UMETA(DisplayName = "Cancel ability when service not relevant"),
	Nothing UMETA(DisplayName = "Do Nothing when service not relevant")
};

/**
 * Try to always run and activate ability with the supplied GameplayTag.
 */
UCLASS(MinimalAPI)
class UKaosBTService_ActivateAbilityByTag : public UBTService
{
	GENERATED_BODY()

public:
	UKaosBTService_ActivateAbilityByTag();
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

protected:
	/** Ability to activate */
	UPROPERTY(EditAnywhere, Category=ActivateAbilityByTag, meta=(Categories="AbilityTagCategory"))
	FGameplayTag AbilityToActivate;

	/** Target to activate ability on (normally self) */
	UPROPERTY(EditAnywhere, Category=ActivateAbilityByTag)
	FBlackboardKeySelector TargetBlackboardKey;

	/** What to do when service becomes non relevant */
	UPROPERTY(EditAnywhere, Category=ActivateAbilityByTag)
	EKaosActivateAbilityByTagOnCeaseRelevanceBehaviour NodeCeaseRelevanceBehaviour = EKaosActivateAbilityByTagOnCeaseRelevanceBehaviour::Nothing;
};
