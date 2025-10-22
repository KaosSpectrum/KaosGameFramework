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

#include "KaosBTDecorator_CanActivateAbility.generated.h"

/**
 * Checks to see if we can activate an ability with the supplied ability tag
 */
UCLASS(MinimalAPI)
class UKaosBTDecorator_CanActivateAbility : public UBTDecorator
{
	GENERATED_BODY()

public:
	UKaosBTDecorator_CanActivateAbility(const FObjectInitializer& ObjectInitializer);
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual FString GetStaticDescription() const override;

protected:
	/** Actor for ability activation (normally self) */
	UPROPERTY(EditAnywhere, Category=CanActivateAbility, Meta=(ToolTips="Which Actor (from the blackboard) should be checked for this gameplay tag query?"))
	FBlackboardKeySelector ActorForAbilityActivation;

	/** Tag for ability to check if we can activate */
	UPROPERTY(EditAnywhere, Category=CanActivateAbility, meta=(Categories="AbilityTagCategory"))
	FGameplayTag AbilityTag;
};
