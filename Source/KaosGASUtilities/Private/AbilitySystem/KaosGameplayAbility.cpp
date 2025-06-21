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

#include "AbilitySystem/KaosGameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemLog.h"
#include "AbilitySystem/KaosAbilityCosts.h"
#include "AbilitySystem/KaosAbilitySystemComponent.h"

#define ENSURE_ABILITY_IS_INSTANTIATED_OR_RETURN(FunctionName, ReturnValue)																				\
{																																						\
if (!ensure(IsInstantiated()))																														\
{																																					\
ABILITY_LOG(Error, TEXT("%s: " #FunctionName " cannot be called on a non-instanced ability. Check the instancing policy."), *GetPathName());	\
return ReturnValue;																																\
}																																					\
}

UKaosGameplayAbility::UKaosGameplayAbility()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateNo;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;
	bRemoveAfterActivation = false;
	bPassiveAbility = false;
	bOnlyApplyCostOnHit = false;
}

bool UKaosGameplayAbility::DoesAbilitySatisfyTagRequirements(const UAbilitySystemComponent& AbilitySystemComponent, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
                                                             FGameplayTagContainer* OptionalRelevantTags) const
{
	// Specialized version to handle death exclusion and AbilityTags expansion via ASC

	bool bBlocked = false;
	bool bMissing = false;

	const UAbilitySystemGlobals& AbilitySystemGlobals = UAbilitySystemGlobals::Get();
	const FGameplayTag& BlockedTag = AbilitySystemGlobals.ActivateFailTagsBlockedTag;
	const FGameplayTag& MissingTag = AbilitySystemGlobals.ActivateFailTagsMissingTag;

	// Check if any of this ability's tags are currently blocked
	if (AbilitySystemComponent.AreAbilityTagsBlocked(GetAssetTags()))
	{
		bBlocked = true;
	}

	/*
	 * Relationship related code
	 */

	const UKaosAbilitySystemComponent* KaosAbilitySystemComponent = Cast<UKaosAbilitySystemComponent>(&AbilitySystemComponent);
	static FGameplayTagContainer AbilityRequiredTags;
	AbilityRequiredTags = ActivationRequiredTags;

	static FGameplayTagContainer AbilityBlockedTags;
	AbilityBlockedTags = ActivationBlockedTags;

	// This gets the additional tags from the ASC's relationship mapping for the abilities tags.
	if (KaosAbilitySystemComponent)
	{
		KaosAbilitySystemComponent->GetRelationshipActivationTagRequirements(GetAssetTags(), AbilityRequiredTags, AbilityBlockedTags);
	}

	/*
	 * End of relationship code
	 */

	// Check to see the required/blocked tags for this ability
	if (AbilityBlockedTags.Num() || AbilityRequiredTags.Num())
	{
		static FGameplayTagContainer AbilitySystemComponentTags;

		AbilitySystemComponentTags.Reset();
		AbilitySystemComponent.GetOwnedGameplayTags(AbilitySystemComponentTags);

		if (AbilitySystemComponentTags.HasAny(AbilityBlockedTags))
		{
			NotifyAbilityBlocked(AbilitySystemComponentTags, OptionalRelevantTags);
			bBlocked = true;
		}

		if (!AbilitySystemComponentTags.HasAll(AbilityRequiredTags))
		{
			bMissing = true;
		}
	}

	if (SourceTags != nullptr)
	{
		if (SourceBlockedTags.Num() || SourceRequiredTags.Num())
		{
			if (SourceTags->HasAny(SourceBlockedTags))
			{
				bBlocked = true;
			}

			if (!SourceTags->HasAll(SourceRequiredTags))
			{
				bMissing = true;
			}
		}
	}

	if (TargetTags != nullptr)
	{
		if (TargetBlockedTags.Num() || TargetRequiredTags.Num())
		{
			if (TargetTags->HasAny(TargetBlockedTags))
			{
				bBlocked = true;
			}

			if (!TargetTags->HasAll(TargetRequiredTags))
			{
				bMissing = true;
			}
		}
	}

	if (bBlocked)
	{
		if (OptionalRelevantTags && BlockedTag.IsValid())
		{
			OptionalRelevantTags->AddTag(BlockedTag);
		}
		return false;
	}
	if (bMissing)
	{
		if (OptionalRelevantTags && MissingTag.IsValid())
		{
			OptionalRelevantTags->AddTag(MissingTag);
		}
		return false;
	}

	return true;
}

bool UKaosGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
                                              FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
	{
		return false;
	}

	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	return true;
}

void UKaosGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (bHasBlueprintActivate)
	{
		// A Blueprinted ActivateAbility function must call CommitAbility somewhere in its execution chain.
		K2_ActivateAbility();
	}
	else if (bHasBlueprintActivateFromEvent)
	{
		if (TriggerEventData)
		{
			// A Blueprinted ActivateAbility function must call CommitAbility somewhere in its execution chain.
			K2_ActivateAbilityFromEvent(*TriggerEventData);
		}
		else
		{
			ABILITY_LOG(Warning, TEXT("Ability %s expects event data but none is being supplied. Use Activate Ability instead of Activate Ability From Event."), *GetName());
			constexpr bool bReplicateEndAbility = false;
			constexpr bool bWasCancelled = true;
			EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
		}
	}
}

void UKaosGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UKaosGameplayAbility::SetCanBeCanceled(bool bCanBeCanceled)
{
	Super::SetCanBeCanceled(bCanBeCanceled);
}

void UKaosGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);
	K2_OnAbilityAdded();
	TryActivatePassiveAbility(ActorInfo, Spec);
}

void UKaosGameplayAbility::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnRemoveAbility(ActorInfo, Spec);
	K2_OnAbilityRemoved();
}

void UKaosGameplayAbility::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	check(ActorInfo);

	bool bHasDeterminedIfAbilityHitTarget = false;
	bool bAbilityHitTarget = false;

	const UGameplayEffect* CostGE = GetCostGameplayEffect();

	if (CostGE)
	{
		if (bOnlyApplyCostOnHit)
		{
			bAbilityHitTarget = DetermineIfAbilityHitTarget(Handle, ActorInfo, ActivationInfo);
			bHasDeterminedIfAbilityHitTarget = true;
		}

		if (!bOnlyApplyCostOnHit || (bOnlyApplyCostOnHit && bAbilityHitTarget))
		{
			ApplyGameplayEffectToOwner(Handle, ActorInfo, ActivationInfo, CostGE, GetAbilityLevel(Handle, ActorInfo));
		}
	}

	// Pay any additional costs
	for (TObjectPtr<UKaosAbilityCosts> AdditionalCost : AdditionalCosts)
	{
		if (AdditionalCost != nullptr)
		{
			if (AdditionalCost->ShouldOnlyApplyCostOnHit())
			{
				if (!bHasDeterminedIfAbilityHitTarget)
				{
					bAbilityHitTarget = DetermineIfAbilityHitTarget(Handle, ActorInfo, ActivationInfo);
					bHasDeterminedIfAbilityHitTarget = true;
				}

				if (!bAbilityHitTarget)
				{
					continue;
				}
			}

			AdditionalCost->ApplyCost(this, Handle, ActorInfo, ActivationInfo);
		}
	}
}

bool UKaosGameplayAbility::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags) || !ActorInfo)
	{
		return false;
	}
	//TODO: Energy/Stamina cost
	// Verify we can afford any additional costs
	for (TObjectPtr<UKaosAbilityCosts> AdditionalCost : AdditionalCosts)
	{
		if (AdditionalCost != nullptr)
		{
			if (!AdditionalCost->CheckCost(this, Handle, ActorInfo, /*inout*/ OptionalRelevantTags))
			{
				return false;
			}
		}
	}

	return true;
}

FGameplayTagContainer UKaosGameplayAbility::K2_GetCooldownTags() const
{
	//TODO:
	return {};
}

FGameplayTagContainer UKaosGameplayAbility::GetAbilityTags() const
{
	return GetAssetTags();
}

bool UKaosGameplayAbility::DoesTargetHaveTags(const AActor* Target, const FGameplayTagContainer& Tags) const
{
	if (const UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Target))
	{
		return ASC->HasAnyMatchingGameplayTags(Tags);
	}

	if (const IGameplayTagAssetInterface* TagAssetInterface = Cast<IGameplayTagAssetInterface>(Target))
	{
		return TagAssetInterface->HasAnyMatchingGameplayTags(Tags);
	}

	return false;
}

void UKaosGameplayAbility::OnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const
{
	NativeOnAbilityFailedToActivate(FailedReason);
	K2_OnAbilityFailedToActivate(FailedReason);
}

void UKaosGameplayAbility::NotifyFailedToActivatePassiveAbility() const
{
	K2_FailedToActivatePassiveAbility();
}

bool UKaosGameplayAbility::IsAbilityBlockedByTags(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const
{
	return ActorInfo ? ActorInfo->AbilitySystemComponent->AreAbilityTagsBlocked(GetAssetTags()) : false;
}

void UKaosGameplayAbility::TryActivatePassiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) const
{
	// Try to activate if activation type is Passive. Passive abilities are auto activated when given.
	if (ActorInfo && !Spec.IsActive() && IsPassiveAbility())
	{
		UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
		const AActor* AvatarActor = ActorInfo->AvatarActor.Get();

		// If avatar actor is torn off or about to die, don't try to activate until we get the new one.
		if (ASC && AvatarActor && !AvatarActor->GetTearOff() && (AvatarActor->GetLifeSpan() <= 0.0f))
		{
			const bool bIsLocalExecution = (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::LocalPredicted) || (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::LocalOnly);
			const bool bIsServerExecution = (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::ServerOnly) || (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::ServerInitiated);

			const bool bClientShouldActivate = ActorInfo->IsLocallyControlled() && bIsLocalExecution;
			const bool bServerShouldActivate = ActorInfo->IsNetAuthority() && bIsServerExecution;

			if (bClientShouldActivate || bServerShouldActivate)
			{
				if (!ASC->TryActivateAbility(Spec.Handle))
				{
					NotifyFailedToActivatePassiveAbility();
				}
			}
		}
	}
}

void UKaosGameplayAbility::NativeOnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const
{
}

void UKaosGameplayAbility::OnPawnAvatarSet()
{
	K2_OnPawnAvatarSet();
}

bool UKaosGameplayAbility::DetermineIfAbilityHitTarget(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (ActorInfo->IsNetAuthority())
	{
		if (UKaosAbilitySystemComponent* ASC = Cast<UKaosAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get()))
		{
			FGameplayAbilityTargetDataHandle TargetData;
			ASC->GetAbilityTargetData(Handle, ActivationInfo, TargetData);
			for (int32 TargetDataIdx = 0; TargetDataIdx < TargetData.Data.Num(); ++TargetDataIdx)
			{
				if (UAbilitySystemBlueprintLibrary::TargetDataHasHitResult(TargetData, TargetDataIdx))
				{
					return true;
				}
			}
		}
	}

	return false;
}

bool UKaosGameplayAbility::K2_IsAbilityBlockedByTags() const
{
	ENSURE_ABILITY_IS_INSTANTIATED_OR_RETURN(K2_IsAbilityBlockedByTags(), false);
	return IsAbilityBlockedByTags(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo());
}
