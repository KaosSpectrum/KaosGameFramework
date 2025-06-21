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
#include "AbilitySystemInterface.h"
#include "GameplayAbilitySpec.h"
#include "GameplayTagContainer.h"
#include "KaosGameplayAbilitySet.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UObject/Object.h"
#include "KaosUtilitiesBlueprintLibrary.generated.h"

class UKaosGameplayAbilitySet;
class UGameplayAbility;
class UAbilitySystemComponent;

/** Called when a gameplay attribute bound to an event wrapper via one of the BindEventWrapper<Attribute> methods on the AbilitySystemLibrary changes. */
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnKaosGameplayAttributeChangedEventWrapperSignature, const FGameplayAttribute&, Attribute, float, OldValue, float, NewValue);

/** Holds tracking data for gameplay attribute changed event wrappers that have been bound. */
struct FKaosGameplayAttributeChangedEventWrapperSpec
{
	FKaosGameplayAttributeChangedEventWrapperSpec(
		UAbilitySystemComponent* AbilitySystemComponent,
		FOnKaosGameplayAttributeChangedEventWrapperSignature InGameplayAttributeChangedEventWrapperDelegate);
	~FKaosGameplayAttributeChangedEventWrapperSpec();

	/** The AbilitySystemComponent this spec is bound to. */
	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponentWk;

	/** The event wrapper delegate cached off, to be executed when the gameplay attribute we care about changes. */
	FOnKaosGameplayAttributeChangedEventWrapperSignature GameplayAttributeChangedEventWrapperDelegate;

	/** Map of the respective gameplay attribute to the delegate handle the ASC gave us to use for unbinding later. */
	TMap<FGameplayAttribute, FDelegateHandle> DelegateBindings;
};

/** Handle to a event wrapper listening for gameplay attribute change(s) via one of the BindEventWrapper<to attribute(s)> methods on the AbilitySystemLibrary */
USTRUCT(BlueprintType)
struct FKaosGameplayAttributeChangedEventWrapperSpecHandle
{
	GENERATED_BODY()

	FKaosGameplayAttributeChangedEventWrapperSpecHandle();
	FKaosGameplayAttributeChangedEventWrapperSpecHandle(FKaosGameplayAttributeChangedEventWrapperSpec* DataPtr);

	/** Internal pointer to binding spec */
	TSharedPtr<FKaosGameplayAttributeChangedEventWrapperSpec>	Data;

	bool operator==(FKaosGameplayAttributeChangedEventWrapperSpecHandle const& Other) const;
	bool operator!=(FKaosGameplayAttributeChangedEventWrapperSpecHandle const& Other) const;
};

/**
 * Collection of helper functions for Gameplay Ability System.
 */
UCLASS()
class KAOSGASUTILITIES_API UKaosUtilitiesBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Check to see if we can activate an ability with ALL passed in Tags.
	 * Example: Ability has tags: A.1 and B.1, and GameplayAbilityTags has A.1, it will return true. But if GameplayAbilityTags
	 * has A.1 and C.1, it will return false.
	 */
	UFUNCTION(BlueprintCallable, Category="KaosGAS")
	static bool CanActivateAbilityWithMatchingTags(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayTagContainer& GameplayAbilityTags);

	/**
	 * Check to see if an ability is active with matching tags.
	 * Example: Ability has tags: A.1 and B.1, and GameplayAbilityTags has A.1, it will return true. But if GameplayAbilityTags
	 * has A.1 and C.1, it will return false.
	 */
	UFUNCTION(BlueprintCallable, Category="KaosGAS")
	static bool HasActiveAbilityWithMatchingTags(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayTagContainer& GameplayAbilityTags);

	/**
	 * Cancels ability with all tags
	 * Example: Ability has tags: A.1 and B.1, and GameplayAbilityTags has A.1, it will return true. But if GameplayAbilityTags
	 * has A.1 and C.1, it will return false.
	 */
	UFUNCTION(BlueprintCallable, Category="KaosGAS")
	static void CancelAbilityWithAllTags(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayTagContainer& GameplayAbilityTags);

	/**
	 * Checks to see if we have an ability with all tags.
	 * Example: Ability has tags: A.1 and B.1, and GameplayAbilityTags has A.1, it will return true. But if GameplayAbilityTags
	 * has A.1 and C.1, it will return false.
	 */
	UFUNCTION(BlueprintCallable, Category="KaosGAS")
	static bool HasAbilityWithAllTags(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayTagContainer& GameplayAbilityTags);

	/**
	 * Checks to see if ability is on cooldown with all tags.
	 * Example: Ability has tags: A.1 and B.1, and GameplayAbilityTags has A.1, it will return true. But if GameplayAbilityTags
	 * has A.1 and C.1, it will return false.
	 */
	UFUNCTION(BlueprintCallable, Category="KaosGAS")
	static bool IsAbilityOnCooldownWithAllTags(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayTagContainer& GameplayAbilityTags, float& TimeRemaining, float& Duration);

	/**
	 * Returns true if we can activate ability by the supplied class.
	 */
	UFUNCTION(BlueprintCallable, Category="KaosGAS")
	static bool CanActivateAbilityByClass(UAbilitySystemComponent* AbilitySystemComponent, TSubclassOf<UGameplayAbility> AbilityClass);

	/**
	 * Returns true if we have an attribute set of class type.
	 */
	UFUNCTION(BlueprintCallable, Category="KaosGAS")
	static bool HasAttributeSet(UAbilitySystemComponent* AbilitySystemComponent, TSubclassOf<UAttributeSet> AttributeClass);

	/**
	 * Returns true if a specific ability tag is blocked by the ASC.
	 */
	UFUNCTION(BlueprintCallable, Category="KaosGAS")
	static bool IsAbilityTagBlocked(UAbilitySystemComponent* AbilitySystemComponent, FGameplayTag AbilityTag);

	/**
	 * Returns true if an ability is active by the passed in handle
	 */
	UFUNCTION(BlueprintCallable, Category="KaosGAS")
	static bool IsAbilityActive(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayAbilitySpecHandle& InHandle);

	/**
	 * Returns true if we can activate ability by the supplied class and optional source object.
	 */
	UFUNCTION(BlueprintCallable, Category="KaosGAS")
	static bool IsAbilityActiveByClass(UAbilitySystemComponent* AbilitySystemComponent, TSubclassOf<UGameplayAbility> AbilityClass, UObject* OptionalSourceObject);

	/**
	 * Returns true if we can apply attribute modifies for a specific Effect Spec.
	 */
	UFUNCTION(BlueprintCallable, Category="KaosGAS")
	static bool CanApplyAttributeModifiers(UAbilitySystemComponent* AbilitySystemComponent, FGameplayEffectSpec EffectSpec);

	/**
	 * Will block abilities with the supplied tags
	 */
	UFUNCTION(BlueprintCallable, Category="KaosGAS")
	static void BlockAbilitiesWithTags(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayTagContainer& GameplayAbilityTags);

	/**
	 * Will unblock abilities with the supplied tags. (will affect Gameplay Ability blocking tags!)
	 */
	UFUNCTION(BlueprintCallable, Category="KaosGAS")
	static void UnblockAbilitiesWithTags(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayTagContainer& GameplayAbilityTags);

	/**
	 * Tries to give an ability set to Actor
	 */
	UFUNCTION(BlueprintCallable, Category="KaosGAS")
	static FKaosAbilitySetHandle GiveAbilitySetToActor(AActor* Actor, UKaosGameplayAbilitySet* Set, UObject* OptionalOverrideSourceObject = nullptr);

	/**
	 * Tries to give ability set to ASC
	 */
	UFUNCTION(BlueprintCallable, Category="KaosGAS")
	static FKaosAbilitySetHandle GiveAbilitySetToASC(UAbilitySystemComponent* AbilitySystemComponent, UKaosGameplayAbilitySet* Set, UObject* OptionalOverrideSourceObject = nullptr);

	/**
	 * Tries to give AbilitySet to interface owner.
	 */
	UFUNCTION(BlueprintCallable, Category="KaosGAS")
	static FKaosAbilitySetHandle GiveAbilitySetToInterface(TScriptInterface<IAbilitySystemInterface> AbilitySystemInterface, UKaosGameplayAbilitySet* Set, UObject* OptionalOverrideSourceObject = nullptr);

	/**
	 * Will unblock abilities with the supplied tags. (will affect Gameplay Ability blocking tags!)
	 */
	UFUNCTION(BlueprintCallable, Category="KaosGAS")
	static void TakeAbilitySet(UPARAM(ref) FKaosAbilitySetHandle& AbilitySetHandle);

	/**
	 * Returns the level of the ability the context was applied with
	 */
	UFUNCTION(BlueprintCallable, Category="KaosGAS")
	static int32 GetAbilityLevel(const FGameplayEffectContextHandle& EffectContextHandle);

	/**
	 * Returns asset tags for a GameplayEffectSpecHandle
	 */
	UFUNCTION(BlueprintCallable, Category="KaosGAS")
	static void GetAssetTags(const FGameplayEffectSpecHandle& SpecHandle, FGameplayTagContainer& OutAssetTags);

	/**
	 * Sets the loose tag count directly for a specific tag.
	 */
	UFUNCTION(BlueprintCallable, Category="KaosGAS")
	static void SetLooseGameplayTagCount(UAbilitySystemComponent* AbilitySystemComponent, FGameplayTag GameplayTag, int32 NewCount);

	/*
	 * These are C++ helper functions below here.
	 *
	 * 
	 */

	/*
	 * Find's an ability spec for a specific class with OptionalSourceObject
	 */
	static FGameplayAbilitySpec* FindAbilitySpecByClass(UAbilitySystemComponent* AbilitySystemComponent, TSubclassOf<UGameplayAbility> AbilityClass, UObject* OptionalSourceObject = nullptr);

	/*
	 * Find's an ability spec for an ability with all tags with OptionalSourceObject
	 * Example: Ability has tags: A.1 and B.1, and GameplayAbilityTags has A.1, it will return true. But if GameplayAbilityTags
	 * has A.1 and C.1, it will return false.
	 */
	static FGameplayAbilitySpec* FindAbilitySpecWithAllAbilityTags(UAbilitySystemComponent* AbilitySystemComponent, FGameplayTagContainer GameplayAbilityTags, UObject* OptionalSourceObject = nullptr);

	public:
	// -------------------------------------------------------------------------------
	//		Attribute BP change helpers
	// -------------------------------------------------------------------------------

	/**
	 * Binds to changes in the given Tag on the given ASC's owned tags.
	 * Cache off the returned handle and call one of the 'Unbind'...'EventWrapper' fns and pass in the handle when you are finished with the binding.
	 * @param Attribute                               Attribute to listen for changes on
	 * @param AbilitySystemComponent                     AbilitySystemComponent owning the Event
	 * @param GameplayAttributeChangedEventWrapperDelegate Attribute Changed Event to trigger
	 * @param bExecuteForCurrentValueImmediately   If true, the bound event will immediately execute if we already have the tag.
	 * @return                                  FKaosGameplayAttributeChangedEventWrapperSpecHandle Handle by which this binding request can be unbound.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Attribute")
	static FKaosGameplayAttributeChangedEventWrapperSpecHandle BindEventWrapperToAttributeChangedKaos(
		UAbilitySystemComponent* AbilitySystemComponent,
		FGameplayAttribute Attribute, 
		FOnKaosGameplayAttributeChangedEventWrapperSignature GameplayAttributeChangedEventWrapperDelegate, 
		bool bExecuteForCurrentValueImmediately = true);

	/**
	 * Binds to changes in the given Attributes on the given ASC's attribute sets.
	 * Cache off the returned handle and call one of the 'Unbind'...'EventWrapper' fns and pass in the handle when you are finished with the binding.
	 * @param Attributes                              TArray of Attributes to listen for changes on
	 * @param AbilitySystemComponent                     AbilitySystemComponent owning the Event
	 * @param GameplayAttributeChangedEventWrapperDelegate Attribute Changed Event to trigger
	 * @param bExecuteForCurrentValueImmediately   If true, we fire the delegate immediately with the current value of the attribute.
	 * @return                                  FKaosGameplayAttributeChangedEventWrapperSpecHandle Handle by which this binding request can be unbound.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Attribute")
	static FKaosGameplayAttributeChangedEventWrapperSpecHandle BindEventWrapperToAnyOfGameplayAttributesChangedKaos(
		UAbilitySystemComponent* AbilitySystemComponent,
		const TArray<FGameplayAttribute>& Attributes,
		FOnKaosGameplayAttributeChangedEventWrapperSignature GameplayAttributeChangedEventWrapperDelegate, 
		bool bExecuteForCurrentValueImmediately = true);

	/**
	 * Unbinds the event wrapper attribute change event bound via a BindEventWrapper<to gameplay attribute(s)> method that is tied to the given Handle.
	 * (expected to unbind 1 or none)
	 * @param Handle     FKaosGameplayAttributeChangedEventWrapperSpecHandle Handle provided when binding to a delegate
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Attribute")
	static void UnbindAllGameplayAttributeChangedEventWrappersForHandleKaos(FKaosGameplayAttributeChangedEventWrapperSpecHandle Handle);

	/**
	 * Unbinds the event wrapper attribute change event bound via a BindEventWrapper<to gameplay attribute(s)> method that is tied to the given Handle, for the specific attribute.
	 *  and were bound to the given Attribute.
	 * (expected to unbind 1 or none, only makes sense to call if the original binding was for listening to multiple attributes.)
	 * @param Attribute        Gameplay Attribute to unbind from
	 * @param Handle     	Handle provided when binding to a delegate
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Attribute")
	static void UnbindGameplayAttributeChangedEventWrapperForHandleKaos(FGameplayAttribute Attribute, FKaosGameplayAttributeChangedEventWrapperSpecHandle Handle);

protected:

	// Helper fn to process gameplay attribute changed event wrappers.
	static void ProcessGameplayAttributeChangedEventWrapper(const FOnAttributeChangeData& Attribute, FOnKaosGameplayAttributeChangedEventWrapperSignature GameplayAttributeChangedEventWrapperDelegate);

};
