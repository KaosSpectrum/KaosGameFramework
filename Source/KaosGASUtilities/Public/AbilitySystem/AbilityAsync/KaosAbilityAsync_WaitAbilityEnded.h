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
#include "Abilities/Async/AbilityAsync.h"
#include "UObject/Object.h"
#include "AbilityAsync_WaitAbilityEnded.generated.h"

class UGameplayAbility;

/** Wrapper around non bp exposed ended data. */
USTRUCT(BlueprintType)
struct FKaosAbilityEndedData
{
	GENERATED_BODY()

	/** Ability that ended, normally instance but could be CDO */
	UPROPERTY(BlueprintReadOnly, Category = "Ended Data")
	TObjectPtr<UGameplayAbility> AbilityThatEnded;

	/** Specific ability spec that ended */
	UPROPERTY(BlueprintReadOnly, Category = "Ended Data")
	FGameplayAbilitySpecHandle AbilitySpecHandle;

	/** True if this was cancelled deliberately, false if it ended normally */
	UPROPERTY(BlueprintReadOnly, Category = "Ended Data")
	bool bWasCancelled;
};

/**
 * 
 */
UCLASS()
class KAOSGASUTILITIES_API UKaosAbilityAsync_WaitAbilityEnded : public UAbilityAsync
{

		GENERATED_BODY()

public:

	/**
	 * Wait until the specified gameplay ability with tags is ended 
	 * It will keep listening as long as OnlyTriggerOnce = false
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks",
		meta = (DefaultToSelf = "TargetActor", BlueprintInternalUseOnly = "true"))
	static UKaosAbilityAsync_WaitAbilityEnded* WaitForAbilityEndedWithTags(
		AActor* TargetActor,
		FGameplayTagContainer AbilityTags,
		EGameplayContainerMatchType MatchType = EGameplayContainerMatchType::Any,
		bool bOnlyTriggerOnce = false);

	/**
	 * Wait until the specified gameplay ability of class is ended
	 * It will keep listening as long as OnlyTriggerOnce = false
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks",
		meta = (DefaultToSelf = "TargetActor", BlueprintInternalUseOnly = "true"))
	static UKaosAbilityAsync_WaitAbilityEnded* WaitForAbilityEndedOfClass(
		AActor* TargetActor,
		TSubclassOf<UGameplayAbility> AbilityClass,
		bool bOnlyTriggerOnce = false);

	/**
	 * Wait until the specified gameplay ability spec handle is ended
	 * It will keep listening as long as OnlyTriggerOnce = false
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks",
		meta = (DefaultToSelf = "TargetActor", BlueprintInternalUseOnly = "true"))
	static UKaosAbilityAsync_WaitAbilityEnded* WaitForAbilityEndedInstance(
		AActor* TargetActor,
		FGameplayAbilitySpecHandle SpecHandle, bool bOnlyTriggerOnce = false);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAsyncWaitAbilityEndedDelegate, FKaosAbilityEndedData, EndedData);

	UPROPERTY(BlueprintAssignable)
	FAsyncWaitAbilityEndedDelegate Ended;

protected:

	enum class EWaitEndFilterMode : uint8
	{
		ByTags,
		ByClass,
		BySpecHandle
	};

	virtual void Activate() override;
	virtual void EndAction() override;

	void OnAbilityEnded(const FAbilityEndedData& EndedData);

	UPROPERTY(Transient)
	FGameplayTagContainer FilterTags;

	UPROPERTY(Transient)
	TSubclassOf<UGameplayAbility> FilterAbilityClass;

	UPROPERTY(Transient)
	FGameplayAbilitySpecHandle FilterSpecHandle;

	EGameplayContainerMatchType FilterMatchType;

	EWaitEndFilterMode FilterMode;

	bool bOnlyTriggerOnce = false;

	FDelegateHandle MyHandle;
};
