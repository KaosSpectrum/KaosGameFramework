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
#include "BehaviorTree/BTDecorator.h"
#include "GameplayTagContainer.h"
#include "KaosBTDecorator_HasGameplayAbility.generated.h"

/**
 * Check if we have an ability with the supplied AbilityTag
 */
UCLASS(MinimalAPI)
class UKaosBTDecorator_HasGameplayAbility : public UBTDecorator
{
	GENERATED_BODY()

public:
	UKaosBTDecorator_HasGameplayAbility();
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual FString GetStaticDescription() const override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

protected:
	/** Target we want to check if we have ability on (normally self) */
	UPROPERTY(EditAnywhere, Category=HasGameplayAbility)
	FBlackboardKeySelector TargetActorKey;

	/** Tag of the ability we want to see if they have */
	UPROPERTY(EditAnywhere, Category=HasGameplayAbility, meta=(Categories="AbilityTagCategory"))
	FGameplayTag AbilityTag;
};
