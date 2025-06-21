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

#include "AbilitySystem/KaosAbilitySystemComponent.h"
#include "KaosUtilitiesLogging.h"
#include "AbilitySystem/KaosAbilityTagRelationships.h"
#include "AbilitySystem/KaosGameplayAbility.h"
#include "GameFramework/Pawn.h"

void UKaosAbilitySystemComponent::ApplyAbilityBlockAndCancelTags(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bEnableBlockTags, const FGameplayTagContainer& BlockTags, bool bExecuteCancelTags,
                                                                 const FGameplayTagContainer& CancelTags)
{
	FGameplayTagContainer AbilityBlockTags = BlockTags;
	FGameplayTagContainer AbilityCancelTags = CancelTags;

	const UKaosAbilityTagRelationships* TagRelationship = GetAbilityTagRelationships();
	if (TagRelationship)
	{
		TagRelationship->GetAbilityTagsToBlockAndCancel(AbilityTags, &AbilityBlockTags, &AbilityCancelTags);
	}

	Super::ApplyAbilityBlockAndCancelTags(AbilityTags, RequestingAbility, bEnableBlockTags, AbilityBlockTags, bExecuteCancelTags, AbilityCancelTags);
}

void UKaosAbilitySystemComponent::K2_UnBlockAbilitiesWithTags(FGameplayTagContainer& Tags)
{
	ApplyAbilityBlockAndCancelTags(FGameplayTagContainer(), nullptr, false, Tags, false, FGameplayTagContainer());
}

void UKaosAbilitySystemComponent::K2_BlockAbilitiesWithTags(FGameplayTagContainer& Tags)
{
	ApplyAbilityBlockAndCancelTags(FGameplayTagContainer(), nullptr, true, Tags, false, FGameplayTagContainer());
}

void UKaosAbilitySystemComponent::GetRelationshipActivationTagRequirements(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer& OutActivationRequired, FGameplayTagContainer& OutActivationBlocked) const
{
	const UKaosAbilityTagRelationships* TagRelationship = GetAbilityTagRelationships();
	if (TagRelationship)
	{
		TagRelationship->GetRequiredAndBlockedActivationTags(AbilityTags, &OutActivationRequired, &OutActivationBlocked);
	}
}

void UKaosAbilitySystemComponent::NotifyAbilityFailed(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason)
{
	if (const APawn* Avatar = Cast<APawn>(GetAvatarActor()))
	{
		if (!Avatar->IsLocallyControlled() && Ability->IsSupportedForNetworking())
		{
			ClientNotifyAbilityFailed(Ability, FailureReason);
			return;
		}
	}

	HandleAbilityFailed(Ability, FailureReason);
}

void UKaosAbilitySystemComponent::NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled)
{
	Super::NotifyAbilityEnded(Handle, Ability, bWasCancelled);

	FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(Handle);
	ENetRole OwnerRole = GetOwnerRole();
	UKaosGameplayAbility* KaosGA = Cast<UKaosGameplayAbility>(Ability);
	if (OwnerRole == ROLE_Authority && !Spec->RemoveAfterActivation)
	{
		if (KaosGA && KaosGA->IsInstantiated() && KaosGA->ShouldRemoveAfterActivation() && !Spec->IsActive())
		{
			ClearAbility(Handle);
		}
	}
}

FActiveGameplayEffect* UKaosAbilitySystemComponent::GetActiveGameplayEffect_Mutable(FActiveGameplayEffectHandle Handle)
{
	return ActiveGameplayEffects.GetActiveGameplayEffect(Handle);
}

TArray<FActiveGameplayEffectHandle> UKaosAbilitySystemComponent::GetAllActiveEffectHandles() const
{
	return ActiveGameplayEffects.GetAllActiveEffectHandles();
}

bool UKaosAbilitySystemComponent::CanApplyAttributeModifiers(FGameplayEffectSpec EffectSpec)
{
	EffectSpec.CalculateModifierMagnitudes();

	for (int32 ModIdx = 0; ModIdx < EffectSpec.Modifiers.Num(); ++ModIdx)
	{
		const FGameplayModifierInfo& ModDef = EffectSpec.Def->Modifiers[ModIdx];
		const FModifierSpec& ModSpec = EffectSpec.Modifiers[ModIdx];

		// It only makes sense to check additive operators
		if (ModDef.ModifierOp == EGameplayModOp::Additive)
		{
			if (!ModDef.Attribute.IsValid())
			{
				continue;
			}
			const UAttributeSet* Set = GetAttributeSubobject(ModDef.Attribute.GetAttributeSetClass());
			const float CurrentValue = ModDef.Attribute.GetNumericValueChecked(Set);
			const float CostValue = ModSpec.GetEvaluatedMagnitude();

			if (CurrentValue + CostValue < 0.f)
			{
				return false;
			}
		}
	}
	return true;
}

void UKaosAbilitySystemComponent::MarkActiveGameplayEffectDirty(FActiveGameplayEffect* ActiveGE)
{
	if (ActiveGE)
	{
		ActiveGameplayEffects.MarkItemDirty(*ActiveGE);
	}
}

void UKaosAbilitySystemComponent::CheckActiveEffectDuration(const FActiveGameplayEffectHandle& Handle)
{
	ActiveGameplayEffects.CheckDuration(Handle);
}

void UKaosAbilitySystemComponent::GetAbilityTargetData(const FGameplayAbilitySpecHandle AbilityHandle, FGameplayAbilityActivationInfo ActivationInfo, FGameplayAbilityTargetDataHandle& OutTargetDataHandle) const
{
	const TSharedPtr<FAbilityReplicatedDataCache> ReplicatedData = AbilityTargetDataMap.Find(FGameplayAbilitySpecHandleAndPredictionKey(AbilityHandle, ActivationInfo.GetActivationPredictionKey()));
	if (ReplicatedData.IsValid())
	{
		OutTargetDataHandle = ReplicatedData->TargetData;
	}
}

const UKaosAbilityTagRelationships* UKaosAbilitySystemComponent::GetAbilityTagRelationships() const
{
	return AbilityTagRelationship;
}

void UKaosAbilitySystemComponent::ClientNotifyAbilityFailed_Implementation(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason)
{
	HandleAbilityFailed(Ability, FailureReason);
}

void UKaosAbilitySystemComponent::HandleAbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason)
{
	if (const UKaosGameplayAbility* KaosAbility = Cast<const UKaosGameplayAbility>(Ability))
	{
		KaosAbility->OnAbilityFailedToActivate(FailureReason);
		if (KaosAbility->IsPassiveAbility())
		{
			KaosAbility->NotifyFailedToActivatePassiveAbility();
		}
	}
}

bool UKaosAbilitySystemComponent::CanActivateAbilityByHandle(const FGameplayAbilitySpecHandle& Handle, FGameplayTagContainer& OutFailureTags)
{
	ABILITYLIST_SCOPE_LOCK();
	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		const FGameplayAbilityActorInfo* ActorInfo = AbilityActorInfo.Get();
		
		if (AbilitySpec.Handle == Handle && AbilitySpec.Ability)
		{
			return AbilitySpec.Ability->CanActivateAbility(AbilitySpec.Handle, ActorInfo, nullptr, nullptr, &OutFailureTags);
		}
	}
	return false;
}

bool UKaosAbilitySystemComponent::CanActivateAbilityByClass(TSubclassOf<UGameplayAbility> AbilityClass, FGameplayTagContainer& OutFailureTags)
{
	ABILITYLIST_SCOPE_LOCK();
	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		const FGameplayAbilityActorInfo* ActorInfo = AbilityActorInfo.Get();
		const UGameplayAbility* Ability = AbilitySpec.GetPrimaryInstance() ? AbilitySpec.GetPrimaryInstance() : AbilitySpec.Ability.Get();
		if (Ability->GetClass() == AbilityClass)
		{
			return Ability->CanActivateAbility(AbilitySpec.Handle, ActorInfo, nullptr, nullptr, &OutFailureTags);
		}
	}
	return false;
}

bool UKaosAbilitySystemComponent::HasAttributeSet(TSubclassOf<UAttributeSet> AttributeClass) const
{
	for (const UAttributeSet* Set : GetSpawnedAttributes())
	{
		if (Set && Set->IsA(AttributeClass))
		{
			return true;
		}
	}
	return false;
}

bool UKaosAbilitySystemComponent::IsAbilityTagBlocked(FGameplayTag AbilityTag)
{
	return AreAbilityTagsBlocked(FGameplayTagContainer(AbilityTag));
}


void UKaosAbilitySystemComponent::CancelAbilityWithAllTags(const FGameplayTagContainer GameplayAbilityTags)
{
	ABILITYLIST_SCOPE_LOCK();
	
	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		//If ability cdo is null, we skip.
		if (AbilitySpec.Ability == nullptr)
		{
			continue;
		}

		//If tags match, cancel the ability
		if (AbilitySpec.Ability->GetAssetTags().HasAll(GameplayAbilityTags) && AbilitySpec.IsActive())
		{
			CancelAbilityHandle(AbilitySpec.Handle);
		}
	}
}

bool UKaosAbilitySystemComponent::IsAbilityOnCooldownWithAllTags(const FGameplayTagContainer GameplayAbilityTags)
{
	ABILITYLIST_SCOPE_LOCK();
	
	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		//If ability cdo is null, we skip.
		if (AbilitySpec.Ability == nullptr)
		{
			continue;
		}

		//If tags match, check if the cooldown tags are applied to the ASC.
		if (AbilitySpec.Ability->GetAssetTags().HasAll(GameplayAbilityTags))
		{
			const FGameplayTagContainer* CooldownTags = AbilitySpec.Ability->GetCooldownTags();
			if (CooldownTags && CooldownTags->Num() > 0 && HasAnyMatchingGameplayTags(*CooldownTags))
			{
				return true;
			}
		}
	}
	return false;
}

bool UKaosAbilitySystemComponent::HasAbilityWithAllTags(const FGameplayTagContainer GameplayAbilityTags)
{
	ABILITYLIST_SCOPE_LOCK();
	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		//If ability cdo is null, we skip.
		if (AbilitySpec.Ability == nullptr)
		{
			continue;
		}

		//If tags match then we have the ability
		if (AbilitySpec.Ability->GetAssetTags().HasAll(GameplayAbilityTags))
		{
			return true;
		}
	}
	return false;
}

bool UKaosAbilitySystemComponent::CanActivateAbilityWithAllMatchingTags(const FGameplayTagContainer GameplayAbilityTags, FGameplayTagContainer& OutFailureTags)
{
	ABILITYLIST_SCOPE_LOCK();
	const FGameplayAbilityActorInfo* ActorInfo = AbilityActorInfo.Get();
	//Loop through all specs and find if we can activate any ability
	for (const FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		//If ability cdo is null, we skip.
		if (Spec.Ability == nullptr)
		{
			continue;
		}

		//If tags match and we can activate, return the call to CanActivateAbility.
		if (Spec.Ability->GetAssetTags().HasAll(GameplayAbilityTags))
		{
			return Spec.Ability->CanActivateAbility(Spec.Handle, ActorInfo, nullptr, nullptr, &OutFailureTags);
		}
	}
	return false;
}



bool UKaosAbilitySystemComponent::IsAbilityActive(const FGameplayAbilitySpecHandle& InHandle)
{
	ABILITYLIST_SCOPE_LOCK();
	const FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(InHandle);
	return Spec ? Spec->IsActive() : false;
}

FGameplayAbilitySpec* UKaosAbilitySystemComponent::FindAbilitySpecFromTag(FGameplayTag Tag)
{
	for (FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		if (Spec.Ability->GetAssetTags().HasTagExact(Tag))
		{
			return &Spec;
		}
	}

	return nullptr;
}

FGameplayAbilitySpec* UKaosAbilitySystemComponent::FindAbilitySpecByClassAndSource(TSubclassOf<UGameplayAbility> AbilityClass, UObject* SourceObject)
{
	for (FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		if (Spec.Ability->GetClass() == AbilityClass && Spec.SourceObject == SourceObject)
		{
			return &Spec;
		}
	}
	return nullptr;
}


bool UKaosAbilitySystemComponent::IsAbilityActiveByClass(TSubclassOf<UGameplayAbility> AbilityClass, UObject* SourceObject)
{
	ABILITYLIST_SCOPE_LOCK();

	FGameplayAbilitySpec* Spec;

	if (SourceObject)
	{
		Spec = FindAbilitySpecByClassAndSource(AbilityClass, SourceObject);
	}
	else
	{
		Spec = FindAbilitySpecFromClass(AbilityClass);
	}

	if (Spec)
	{
		return Spec->IsActive();
	}
	return false;
}

bool UKaosAbilitySystemComponent::IsAbilityActiveByTags(const FGameplayTagContainer* WithTags, const FGameplayTagContainer* WithoutTags, UGameplayAbility* Ignore)
{
	ABILITYLIST_SCOPE_LOCK();

	for (FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		if (!Spec.IsActive() || Spec.Ability == nullptr || Spec.Ability == Ignore)
		{
			continue;
		}

		const bool WithTagPass = (!WithTags || Spec.Ability->GetAssetTags().HasAny(*WithTags));
		const bool WithoutTagPass = (!WithoutTags || !Spec.Ability->GetAssetTags().HasAny(*WithoutTags));

		if (WithTagPass && WithoutTagPass)
		{
			return true;
		}
	}
	return false;
}

bool UKaosAbilitySystemComponent::HasActiveAbilityWithAnyMatchingTag(const FGameplayTagContainer Tags)
{
	TArray<FGameplayAbilitySpec> Specs = GetActivatableAbilities();
	for (const FGameplayAbilitySpec& Spec : Specs)
	{
		if (Spec.Ability == nullptr)
		{
			continue;
		}

		if (Spec.Ability->GetAssetTags().HasAny(Tags) && Spec.IsActive())
		{
			return true;
		}
	}
	return false;
}

bool UKaosAbilitySystemComponent::HasActiveAbilityWithAllMatchingTag(const FGameplayTagContainer Tags)
{
	TArray<FGameplayAbilitySpec> Specs = GetActivatableAbilities();
	for (const FGameplayAbilitySpec& Spec : Specs)
	{
		if (Spec.Ability == nullptr)
		{
			continue;
		}

		if (Spec.Ability->GetAssetTags().HasAll(Tags) && Spec.IsActive())
		{
			return true;
		}
	}
	return false;
}

bool UKaosAbilitySystemComponent::CanActivateAbilityWithAnyMatchingTag(const FGameplayTagContainer GameplayAbilityTags)
{
	TArray<FGameplayAbilitySpec> Specs = GetActivatableAbilities();
	const FGameplayAbilityActorInfo* ActorInfo = AbilityActorInfo.Get();
	for (const FGameplayAbilitySpec& Spec : Specs)
	{
		if (Spec.Ability == nullptr)
		{
			continue;
		}

		if (Spec.Ability->GetAssetTags().HasAny(GameplayAbilityTags) && Spec.Ability->CanActivateAbility(Spec.Handle, ActorInfo))
		{
			return true;
		}
	}
	return false;
}

bool UKaosAbilitySystemComponent::CanActivateAbilityWithAllMatchingTag(const FGameplayTagContainer GameplayAbilityTags)
{
	TArray<FGameplayAbilitySpec> Specs = GetActivatableAbilities();
	const FGameplayAbilityActorInfo* ActorInfo = AbilityActorInfo.Get();
	for (const FGameplayAbilitySpec& Spec : Specs)
	{
		if (Spec.Ability == nullptr)
		{
			continue;
		}

		if (Spec.Ability->GetAssetTags().HasAll(GameplayAbilityTags) && Spec.Ability->CanActivateAbility(Spec.Handle, ActorInfo))
		{
			return true;
		}
	}
	return false;
}
