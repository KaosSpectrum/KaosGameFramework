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
#include "ActiveGameplayEffectHandle.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "Templates/SubclassOf.h"
#include "UObject/Object.h"
#include "KaosUtilitiesTypes.generated.h"

class UAttributeSet;
class UGameplayAbility;
class UGameplayEffect;
class UAbilitySystemComponent;

/**
 * FKaosAbilitySet_GameplayAbility
 *
 *	Data used by the ability set to grant gameplay abilities.
 */
USTRUCT(BlueprintType)
struct FKaosAbilitySet_GameplayAbility
{
	GENERATED_BODY()

public:
	// Gameplay ability to grant.
	UPROPERTY(EditDefaultsOnly, Category=Ability)
	TSubclassOf<UGameplayAbility> Ability = nullptr;

	// Level of ability to grant.
	UPROPERTY(EditDefaultsOnly, Category=Ability)
	int32 AbilityLevel = 1;

	// Tag used to process input for the ability.
	UPROPERTY(EditDefaultsOnly, Category=Ability, Meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
};


/**
 * FKaosAbilitySet_GameplayEffect
 *
 *	Data used by the ability set to grant gameplay effects.
 */
USTRUCT(BlueprintType)
struct FKaosAbilitySet_GameplayEffect
{
	GENERATED_BODY()

public:
	// Gameplay effect to grant.
	UPROPERTY(EditDefaultsOnly, Category=GameplayEffect)
	TSubclassOf<UGameplayEffect> GameplayEffect = nullptr;

	// Level of gameplay effect to grant.
	UPROPERTY(EditDefaultsOnly, Category=GameplayEffect)
	float EffectLevel = 1.0f;
};

/**
 * FKaosAbilitySet_GameplayEffect
 *
 *	Data used by the ability set to grant gameplay effects.
 */
USTRUCT(BlueprintType)
struct FKaosAbilitySet_AttributeSet
{
	GENERATED_BODY()

public:
	// Attribute set to apply
	UPROPERTY(EditDefaultsOnly, Category=AttributeSet)
	TSubclassOf<UAttributeSet> AttributeSet = nullptr;
};

/**
 * FKaosAbilitySetHandle
 *
 *	Data used to store handles to what has been granted by the ability set.
 */
USTRUCT(BlueprintType)
struct FKaosAbilitySetHandle
{
	GENERATED_BODY()

	bool IsValid() const
	{
		return AbilitySystemComponent.IsValid() && HandleId != 0;
	}

	void RemoveSet();

private:
	friend class UKaosGameplayAbilitySet;
	friend class UKaosUtilitiesBlueprintLibrary;

	void AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle);
	void AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle);

	// Handles to the granted abilities.
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;

	// Handles to the granted gameplay effects.
	UPROPERTY()
	TArray<FActiveGameplayEffectHandle> GameplayEffectHandles;
	
	int32 HandleId = 0;

	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = nullptr;

	void Reset()
	{
		AbilitySpecHandles.Reset();
		GameplayEffectHandles.Reset();
		AbilitySystemComponent.Reset();
		HandleId = 0;
	}
};

USTRUCT(BlueprintType)
struct FKaosAttributeInitializationKey
{
	GENERATED_BODY()

public:
	FName GetAttributeInitCategory() const { return AttributeInitCategory; }
	FName GetAttributeInitSubCategory() const { return AttributeInitSubCategory; }
	bool IsValid() const { return !AttributeInitCategory.IsNone() && !AttributeInitSubCategory.IsNone(); }
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName AttributeInitCategory;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName AttributeInitSubCategory;

#if WITH_EDITOR
	friend class FKaosAttributeInitKeyCustomization;
#endif
};