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

#include "AbilitySystem/KaosUtilitiesBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemLog.h"
#include "KaosUtilitiesLogging.h"
#include "GameplayEffect.h"
#include "Logging/StructuredLog.h"


FKaosGameplayAttributeChangedEventWrapperSpec::FKaosGameplayAttributeChangedEventWrapperSpec(UAbilitySystemComponent* AbilitySystemComponent,
	FOnKaosGameplayAttributeChangedEventWrapperSignature InGameplayAttributeChangedEventWrapperDelegate)
	: AbilitySystemComponentWk(AbilitySystemComponent)
	, GameplayAttributeChangedEventWrapperDelegate(InGameplayAttributeChangedEventWrapperDelegate)
{
}

FKaosGameplayAttributeChangedEventWrapperSpec::~FKaosGameplayAttributeChangedEventWrapperSpec()
{
	const int32 RemainingDelegateBindingsCount = DelegateBindings.Num();
	if (RemainingDelegateBindingsCount > 0)
	{
		// We still have delegates bound to the ASC - we need to warn the user!
		// We expect the user to unbind delegates they bound.

		// The exception is if the ASC itself is not valid which indicates things are tearing down - in that case, we'll give them a pass since it's a moot point that
		//  we are still bound if the ASC isn't around anymore.
		UAbilitySystemComponent* AbilitySystemComponent = AbilitySystemComponentWk.Get();
		if (IsValid(AbilitySystemComponent))
		{
			ABILITY_LOG(
				Error, 
				TEXT("~FKaosGameplayAttributeChangedEventWrapperSpec: our bound spec is being destroyed but we still have %d delegate bindings bound to the ASC on '%s'! Please cache off the Bound delegate handle and unbind it when finished."),
				RemainingDelegateBindingsCount,
				*GetNameSafe(AbilitySystemComponent->GetOwner()));
		}
	}
}


FKaosGameplayAttributeChangedEventWrapperSpecHandle::FKaosGameplayAttributeChangedEventWrapperSpecHandle()
	: Data(nullptr)
{
}

FKaosGameplayAttributeChangedEventWrapperSpecHandle::FKaosGameplayAttributeChangedEventWrapperSpecHandle(FKaosGameplayAttributeChangedEventWrapperSpec* DataPtr)
	: Data(DataPtr)
{
}

bool FKaosGameplayAttributeChangedEventWrapperSpecHandle::operator==(FKaosGameplayAttributeChangedEventWrapperSpecHandle const& Other) const
{
	const bool bBothValid = Data.IsValid() && Other.Data.IsValid();
	const bool bBothInvalid = !Data.IsValid() && !Other.Data.IsValid();
	return (bBothInvalid || (bBothValid && (Data.Get() == Other.Data.Get())));
}

bool FKaosGameplayAttributeChangedEventWrapperSpecHandle::operator!=(FKaosGameplayAttributeChangedEventWrapperSpecHandle const& Other) const
{
	return !(operator==(Other));
}


bool UKaosUtilitiesBlueprintLibrary::CanActivateAbilityWithMatchingTags(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayTagContainer& GameplayAbilityTags)
{
	if (AbilitySystemComponent)
	{
		//Get a copy of the ability specs.
		TArray<FGameplayAbilitySpec> Specs = AbilitySystemComponent->GetActivatableAbilities();

		//Get the Actor info as we need it.
		const FGameplayAbilityActorInfo* ActorInfo = AbilitySystemComponent->AbilityActorInfo.Get();

		//Loop through all specs and find if we can activate any ability
		for (const FGameplayAbilitySpec& Spec : Specs)
		{
			//If ability cdo is null, we skip.
			if (Spec.Ability == nullptr)
			{
				continue;
			}

			//If tags match and we can activate, return the call to CanActivateAbility.
			if (Spec.Ability->GetAssetTags().HasAll(GameplayAbilityTags))
			{
				return Spec.Ability->CanActivateAbility(Spec.Handle, ActorInfo);
			}
		}
	}
	return false;
}

bool UKaosUtilitiesBlueprintLibrary::HasActiveAbilityWithMatchingTags(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayTagContainer& GameplayAbilityTags)
{
	if (AbilitySystemComponent)
	{
		//Get a copy of the ability specs.
		TArray<FGameplayAbilitySpec> Specs = AbilitySystemComponent->GetActivatableAbilities();

		//Get the Actor info as we need it.
		const FGameplayAbilityActorInfo* ActorInfo = AbilitySystemComponent->AbilityActorInfo.Get();

		//Loop through all specs and find if we can activate any ability
		for (const FGameplayAbilitySpec& Spec : Specs)
		{
			//If ability cdo is null, we skip.
			if (Spec.Ability == nullptr)
			{
				continue;
			}

			//If tags match then we have the ability.
			if (Spec.Ability->GetAssetTags().HasAll(GameplayAbilityTags) && Spec.IsActive())
			{
				return Spec.Ability->CanActivateAbility(Spec.Handle, ActorInfo);
			}
		}
	}
	return false;
}

void UKaosUtilitiesBlueprintLibrary::CancelAbilityWithAllTags(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayTagContainer& GameplayAbilityTags)
{
	if (AbilitySystemComponent)
	{
		//Get a copy of the ability specs.
		TArray<FGameplayAbilitySpec> Specs = AbilitySystemComponent->GetActivatableAbilities();

		//Loop through all specs and find if we can activate any ability
		for (const FGameplayAbilitySpec& Spec : Specs)
		{
			//If ability cdo is null, we skip.
			if (Spec.Ability == nullptr)
			{
				continue;
			}

			//If tags match, cancel the ability
			if (Spec.Ability->GetAssetTags().HasAll(GameplayAbilityTags) && Spec.IsActive())
			{
				AbilitySystemComponent->CancelAbilityHandle(Spec.Handle);
			}
		}
	}
}

bool UKaosUtilitiesBlueprintLibrary::HasAbilityWithAllTags(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayTagContainer& GameplayAbilityTags)
{
	if (AbilitySystemComponent)
	{
		//Get a copy of the ability specs.
		TArray<FGameplayAbilitySpec> Specs = AbilitySystemComponent->GetActivatableAbilities();

		//Loop through all specs and find if we can activate any ability
		for (const FGameplayAbilitySpec& Spec : Specs)
		{
			//If ability cdo is null, we skip.
			if (Spec.Ability == nullptr)
			{
				continue;
			}

			//If tags match then we have the ability
			if (Spec.Ability->GetAssetTags().HasAll(GameplayAbilityTags))
			{
				return true;
			}
		}
	}
	return false;
}

bool UKaosUtilitiesBlueprintLibrary::IsAbilityOnCooldownWithAllTags(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayTagContainer& GameplayAbilityTags, float& TimeRemaining, float& CooldownDuration)
{
	if (AbilitySystemComponent)
	{
		//Get a copy of the ability specs.
		TArray<FGameplayAbilitySpec> Specs = AbilitySystemComponent->GetActivatableAbilities();

		//Loop through all specs and find if we can activate any ability
		for (const FGameplayAbilitySpec& Spec : Specs)
		{
			//If ability cdo is null, we skip.
			if (Spec.Ability == nullptr)
			{
				continue;
			}

			//If tags match, check if the cooldown tags are applied to the ASC.
			if (Spec.Ability->GetAssetTags().HasAll(GameplayAbilityTags))
			{
				const FGameplayTagContainer* CooldownTags = Spec.Ability->GetCooldownTags();
				if (CooldownTags && CooldownTags->Num() > 0 && AbilitySystemComponent->HasAnyMatchingGameplayTags(*CooldownTags))
				{
					FGameplayEffectQuery const Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(*CooldownTags);
					TArray<TPair<float, float>> DurationAndTimeRemaining = AbilitySystemComponent->GetActiveEffectsTimeRemainingAndDuration(Query);
					if (DurationAndTimeRemaining.Num() > 0)
					{
						// Iterate over all the effects applying the cooldown (if there are, somehow, multiple) and find the longest
						int32 BestIdx = 0;
						float LongestTime = DurationAndTimeRemaining[0].Key;
						for (int32 Idx = 1; Idx < DurationAndTimeRemaining.Num(); ++Idx)
						{
							if (DurationAndTimeRemaining[Idx].Key > LongestTime)
							{
								LongestTime = DurationAndTimeRemaining[Idx].Key;
								BestIdx = Idx;
							}
						}

						TimeRemaining = DurationAndTimeRemaining[BestIdx].Key;
						CooldownDuration = DurationAndTimeRemaining[BestIdx].Value;

						return true;
					}
				}
			}
		}
	}
	return false;
}

bool UKaosUtilitiesBlueprintLibrary::CanActivateAbilityByClass(UAbilitySystemComponent* AbilitySystemComponent, TSubclassOf<UGameplayAbility> AbilityClass)
{
	if (AbilitySystemComponent)
	{
		//Get a copy of the ability specs.
		TArray<FGameplayAbilitySpec> Specs = AbilitySystemComponent->GetActivatableAbilities();

		for (const FGameplayAbilitySpec& AbilitySpec : Specs)
		{
			const FGameplayAbilityActorInfo* ActorInfo = AbilitySystemComponent->AbilityActorInfo.Get();
			const UGameplayAbility* Ability = AbilitySpec.GetPrimaryInstance() ? AbilitySpec.GetPrimaryInstance() : AbilitySpec.Ability.Get();
			if (Ability->GetClass() == AbilityClass)
			{
				return Ability->CanActivateAbility(AbilitySpec.Handle, ActorInfo, nullptr, nullptr, nullptr);
			}
		}
	}

	return false;
}

FKaosAbilitySetHandle UKaosUtilitiesBlueprintLibrary::GiveAbilitySetToActor(AActor* Actor, UKaosGameplayAbilitySet* Set, UObject* OptionalOverrideSourceObject)
{
	if (!IsValid(Actor))
	{
		UE_LOGFMT(LogKaosUtilities, Warning, "Tried to give ability set to actor which is null");
		return {};
	}

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor);
	if (!IsValid(ASC))
	{
		UE_LOGFMT(LogKaosUtilities, Warning, "Tried to give ability set to actor {Actor} which does not have an Ability System Component", Actor->GetName());
		return {};
	}

	if (!IsValid(Set))
	{
		UE_LOGFMT(LogKaosUtilities, Warning, "Tried to give ability set to actor {Actor} but the Ability Set is null", Actor->GetName());
		return {};
	}

	return Set->GiveAbilitySetTo(ASC, OptionalOverrideSourceObject);
}

FKaosAbilitySetHandle UKaosUtilitiesBlueprintLibrary::GiveAbilitySetToASC(UAbilitySystemComponent* AbilitySystemComponent, UKaosGameplayAbilitySet* Set, UObject* OptionalOverrideSourceObject)
{
	if (!IsValid(Set))
	{
		UE_LOGFMT(LogKaosUtilities, Warning, "Tried to give ability set to AbilitySystemComponent {ASC} but the Ability Set is null", *GetNameSafe(AbilitySystemComponent));
		return {};
	}

	if (!IsValid(AbilitySystemComponent))
	{
		UE_LOGFMT(LogKaosUtilities, Warning, "Tried to give ability set {Set} with a null AbilitySystemComponent", Set->GetName());
		return {};
	}

	return Set->GiveAbilitySetTo(AbilitySystemComponent, OptionalOverrideSourceObject);
}

FKaosAbilitySetHandle UKaosUtilitiesBlueprintLibrary::GiveAbilitySetToInterface(TScriptInterface<IAbilitySystemInterface> AbilitySystemInterface, UKaosGameplayAbilitySet* Set, UObject* OptionalOverrideSourceObject)
{
	if (!IsValid(Set))
	{
		UE_LOGFMT(LogKaosUtilities, Warning, "Tried to give ability set to AbilitySystemInterface {ASC} but the Ability Set is null", *GetNameSafe(AbilitySystemInterface.GetObject()));
		return {};
	}

	if (!IsValid(AbilitySystemInterface.GetObject()))
	{
		UE_LOGFMT(LogKaosUtilities, Warning, "Tried to give ability set {Set} with a invalid interface", Set->GetName());
		return {};
	}

	return Set->GiveAbilitySetToInterface(AbilitySystemInterface, OptionalOverrideSourceObject);
}

void UKaosUtilitiesBlueprintLibrary::TakeAbilitySet(FKaosAbilitySetHandle& AbilitySetHandle)
{
	if (!AbilitySetHandle.IsValid())
	{
		UE_LOGFMT(LogKaosUtilities, Warning, "Tried to remove ability set with an inavalid Ability Set Handle.");
		return;
	}
	AbilitySetHandle.RemoveSet();
}

void UKaosUtilitiesBlueprintLibrary::RemoveAllAbilitySetsFromInterface(TScriptInterface<IAbilitySystemInterface> AbilitySystemInterface)
{
	UAbilitySystemComponent* ASC = Cast<UAbilitySystemComponent>(AbilitySystemInterface.GetObject());
	RemoveAllAbilitySets(ASC);
}

void UKaosUtilitiesBlueprintLibrary::RemoveAllAbilitySets(UAbilitySystemComponent* ASC)
{
	if (!ASC || !IsValidChecked(ASC))
	{
		UE_LOGFMT(LogKaosUtilities, Warning, "Tried to remove all ability sets with a invalid ASC");
		return;
	}
	UKaosGameplayAbilitySet::RemoveAllAbilitySets(ASC);
}

int32 UKaosUtilitiesBlueprintLibrary::GetAbilityLevel(const FGameplayEffectContextHandle& EffectContextHandle)
{
	return EffectContextHandle.GetAbilityLevel();
}

void UKaosUtilitiesBlueprintLibrary::GetAssetTags(const FGameplayEffectSpecHandle& SpecHandle, FGameplayTagContainer& OutAssetTags)
{
	const FGameplayEffectSpec* Spec = SpecHandle.IsValid() ? SpecHandle.Data.Get() : nullptr;
	if (Spec)
	{
		Spec->GetAllAssetTags(OutAssetTags);
	}
}

void UKaosUtilitiesBlueprintLibrary::SetLooseGameplayTagCount(UAbilitySystemComponent* AbilitySystemComponent, FGameplayTag GameplayTag, int32 NewCount)
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetLooseGameplayTagCount(GameplayTag, NewCount);
	}
}

FGameplayAbilitySpec* UKaosUtilitiesBlueprintLibrary::FindAbilitySpecByClass(UAbilitySystemComponent* AbilitySystemComponent, TSubclassOf<UGameplayAbility> AbilityClass, UObject* OptionalSourceObject)
{
	if (AbilitySystemComponent)
	{
		//Get a copy of the ability specs.
		TArray<FGameplayAbilitySpec> Specs = AbilitySystemComponent->GetActivatableAbilities();

		for (FGameplayAbilitySpec& Spec : Specs)
		{
			const bool bMatchesSourceObject = OptionalSourceObject != nullptr ? OptionalSourceObject == Spec.SourceObject.Get() : true;
			if (Spec.Ability->GetClass() == AbilityClass && bMatchesSourceObject)
			{
				return &Spec;
			}
		}
	}
	return nullptr;
}

FGameplayAbilitySpec* UKaosUtilitiesBlueprintLibrary::FindAbilitySpecWithAllAbilityTags(UAbilitySystemComponent* AbilitySystemComponent, FGameplayTagContainer AbilityTags, UObject* OptionalSourceObject)
{
	if (AbilitySystemComponent)
	{
		//Get a copy of the ability specs.
		TArray<FGameplayAbilitySpec> Specs = AbilitySystemComponent->GetActivatableAbilities();

		for (FGameplayAbilitySpec& Spec : Specs)
		{
			const bool bMatchesSourceObject = OptionalSourceObject != nullptr ? OptionalSourceObject == Spec.SourceObject.Get() : true;
			if (Spec.Ability->GetAssetTags().HasAll(AbilityTags) && bMatchesSourceObject)
			{
				return &Spec;
			}
		}
	}
	return nullptr;
}

bool UKaosUtilitiesBlueprintLibrary::HasAttributeSet(UAbilitySystemComponent* AbilitySystemComponent, TSubclassOf<UAttributeSet> AttributeClass)
{
	if (AbilitySystemComponent)
	{
		for (const UAttributeSet* Set : AbilitySystemComponent->GetSpawnedAttributes())
		{
			if (Set && Set->IsA(AttributeClass))
			{
				return true;
			}
		}
	}
	return false;
}

bool UKaosUtilitiesBlueprintLibrary::IsAbilityTagBlocked(UAbilitySystemComponent* AbilitySystemComponent, FGameplayTag AbilityTag)
{
	if (AbilitySystemComponent)
	{
		return AbilitySystemComponent->AreAbilityTagsBlocked(FGameplayTagContainer(AbilityTag));
	}
	return false;
}

bool UKaosUtilitiesBlueprintLibrary::IsAbilityActive(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayAbilitySpecHandle& InHandle)
{
	if (AbilitySystemComponent)
	{
		//Get a copy of the ability specs.
		TArray<FGameplayAbilitySpec> Specs = AbilitySystemComponent->GetActivatableAbilities();

		for (const FGameplayAbilitySpec& Spec : Specs)
		{
			if (Spec.Handle == InHandle)
			{
				return Spec.IsActive();
			}
		}
	}
	return false;
}

bool UKaosUtilitiesBlueprintLibrary::IsAbilityActiveByClass(UAbilitySystemComponent* AbilitySystemComponent, TSubclassOf<UGameplayAbility> AbilityClass, UObject* OptionalSourceObject)
{
	if (AbilitySystemComponent)
	{
		if (const FGameplayAbilitySpec* Spec = FindAbilitySpecByClass(AbilitySystemComponent, AbilityClass, OptionalSourceObject))
		{
			return Spec->IsActive();
		}
	}
	return false;
}

bool UKaosUtilitiesBlueprintLibrary::CanApplyAttributeModifiers(UAbilitySystemComponent* AbilitySystemComponent, FGameplayEffectSpec EffectSpec)
{
	if (AbilitySystemComponent)
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
				const UAttributeSet* Set = AbilitySystemComponent->GetAttributeSet(ModDef.Attribute.GetAttributeSetClass());
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
	return false;
}

void UKaosUtilitiesBlueprintLibrary::BlockAbilitiesWithTags(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayTagContainer& GameplayAbilityTags)
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->BlockAbilitiesWithTags(GameplayAbilityTags);
	}
}

void UKaosUtilitiesBlueprintLibrary::UnblockAbilitiesWithTags(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayTagContainer& GameplayAbilityTags)
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->UnBlockAbilitiesWithTags(GameplayAbilityTags);
	}
}


FKaosGameplayAttributeChangedEventWrapperSpecHandle UKaosUtilitiesBlueprintLibrary::BindEventWrapperToAttributeChangedKaos(
	UAbilitySystemComponent* AbilitySystemComponent, FGameplayAttribute Attribute,
	FOnKaosGameplayAttributeChangedEventWrapperSignature GameplayAttributeChangedEventWrapperDelegate, bool bExecuteForCurrentValueImmediately)
{
	if (!::IsValid(AbilitySystemComponent))
	{
		return FKaosGameplayAttributeChangedEventWrapperSpecHandle();
	}

	//TODO: Maybe we shouldn't allow binding..
	if (!AbilitySystemComponent->HasAttributeSetForAttribute(Attribute))
	{
		UE_LOG(LogAbilitySystem, Warning, TEXT("Tried to bind to an attribute that the owner does not have. Will still bind."));
	}

	FKaosGameplayAttributeChangedEventWrapperSpec* AttributeBindingSpec = new FKaosGameplayAttributeChangedEventWrapperSpec(AbilitySystemComponent, GameplayAttributeChangedEventWrapperDelegate);
	FKaosGameplayAttributeChangedEventWrapperSpecHandle AttributeBindingHandle(AttributeBindingSpec);

	// Bind to the ASC's attribute change listening delegate (which is not a 'dynamic' delegate and thereby can't be used in BP).
	const FDelegateHandle AttributeChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Attribute).AddLambda(
		[GameplayAttributeChangedEventWrapperDelegate]
		(const FOnAttributeChangeData& ChangeData)
	{
		ProcessGameplayAttributeChangedEventWrapper(ChangeData, GameplayAttributeChangedEventWrapperDelegate);
	});

	AttributeBindingSpec->DelegateBindings.Add(Attribute, AttributeChangedDelegateHandle);

	if (bExecuteForCurrentValueImmediately)
	{
		const float CurrentValue = AbilitySystemComponent->GetNumericAttribute(Attribute);
		GameplayAttributeChangedEventWrapperDelegate.ExecuteIfBound(Attribute, 0.f, CurrentValue);
	}
	
	return AttributeBindingHandle;
}

FKaosGameplayAttributeChangedEventWrapperSpecHandle UKaosUtilitiesBlueprintLibrary::BindEventWrapperToAnyOfGameplayAttributesChangedKaos(
	UAbilitySystemComponent* AbilitySystemComponent, const TArray<FGameplayAttribute>& Attributes,
	FOnKaosGameplayAttributeChangedEventWrapperSignature GameplayAttributeChangedEventWrapperDelegate, bool bExecuteForCurrentValueImmediately)
{
	if (!::IsValid(AbilitySystemComponent))
	{
		return FKaosGameplayAttributeChangedEventWrapperSpecHandle();
	}

	FKaosGameplayAttributeChangedEventWrapperSpec* AttributeBindingSpec = new FKaosGameplayAttributeChangedEventWrapperSpec(AbilitySystemComponent, GameplayAttributeChangedEventWrapperDelegate);
	FKaosGameplayAttributeChangedEventWrapperSpecHandle AttributeBindingHandle(AttributeBindingSpec);

	AttributeBindingSpec->DelegateBindings.Reserve(Attributes.Num());

	// Bind each attribute and add to the DelegateBindings container.
	for (const FGameplayAttribute& Attribute : Attributes)
	{
		//TODO: Maybe we shouldn't allow binding..
		if (!AbilitySystemComponent->HasAttributeSetForAttribute(Attribute))
		{
			UE_LOG(LogAbilitySystem, Warning, TEXT("Tried to bind to an attribute that the owner does not have. Will still bind."));
		}
		
		// Bind to the ASC's attribute change listening delegate (which is not a 'dynamic' delegate and thereby can't be used in BP).
		const FDelegateHandle AttributeChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Attribute).AddLambda(
			[GameplayAttributeChangedEventWrapperDelegate]
			(const FOnAttributeChangeData& ChangeData)
		{
			ProcessGameplayAttributeChangedEventWrapper(ChangeData, GameplayAttributeChangedEventWrapperDelegate);
		});

		AttributeBindingSpec->DelegateBindings.Add(Attribute, AttributeChangedDelegateHandle);
	}

	if (bExecuteForCurrentValueImmediately)
	{
		for (const FGameplayAttribute& Attribute : Attributes)
		{
			const float CurrentValue = AbilitySystemComponent->GetNumericAttribute(Attribute);
			GameplayAttributeChangedEventWrapperDelegate.ExecuteIfBound(Attribute, 0.f, CurrentValue);
		}
	}

	return AttributeBindingHandle;
}

void UKaosUtilitiesBlueprintLibrary::UnbindAllGameplayAttributeChangedEventWrappersForHandleKaos(FKaosGameplayAttributeChangedEventWrapperSpecHandle Handle)
{
	FKaosGameplayAttributeChangedEventWrapperSpec* GameplayAttributeChangedEventDataPtr = Handle.Data.Get();
	if (GameplayAttributeChangedEventDataPtr == nullptr)
	{
		return;
	}

	UAbilitySystemComponent* AbilitySystemComponent = GameplayAttributeChangedEventDataPtr->AbilitySystemComponentWk.Get();
	if (AbilitySystemComponent == nullptr)
	{
		return;
	}

	for (const auto& [Attribute, BoundHandle] : GameplayAttributeChangedEventDataPtr->DelegateBindings)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Attribute).Remove(BoundHandle);
	}

	GameplayAttributeChangedEventDataPtr->DelegateBindings.Reset();
}

void UKaosUtilitiesBlueprintLibrary::UnbindGameplayAttributeChangedEventWrapperForHandleKaos(FGameplayAttribute Attribute,
	FKaosGameplayAttributeChangedEventWrapperSpecHandle Handle)
{
	FKaosGameplayAttributeChangedEventWrapperSpec* GameplayAttributeChangedEventDataPtr = Handle.Data.Get();
	if (GameplayAttributeChangedEventDataPtr == nullptr)
	{
		return;
	}

	UAbilitySystemComponent* AbilitySystemComponent = GameplayAttributeChangedEventDataPtr->AbilitySystemComponentWk.Get();
	if (AbilitySystemComponent == nullptr)
	{
		return;
	}

	for (auto DelegateBindingIterator = GameplayAttributeChangedEventDataPtr->DelegateBindings.CreateIterator(); DelegateBindingIterator; ++DelegateBindingIterator)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(DelegateBindingIterator->Key).Remove(DelegateBindingIterator->Value);
		DelegateBindingIterator.RemoveCurrent();
	}
}

void UKaosUtilitiesBlueprintLibrary::ProcessGameplayAttributeChangedEventWrapper(const FOnAttributeChangeData& Attribute,
	FOnKaosGameplayAttributeChangedEventWrapperSignature GameplayAttributeChangedEventWrapperDelegate)
{
	GameplayAttributeChangedEventWrapperDelegate.ExecuteIfBound(Attribute.Attribute, Attribute.OldValue, Attribute.NewValue);
}