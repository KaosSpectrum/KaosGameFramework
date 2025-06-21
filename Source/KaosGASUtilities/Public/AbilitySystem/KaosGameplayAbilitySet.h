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
#include "GameplayTagContainer.h"
#include "KaosUtilitiesTypes.h"
#include "Engine/DataAsset.h"
#include "UObject/Object.h"
#include "KaosGameplayAbilitySet.generated.h"

class UAbilitySystemComponent;
class UGameplayAbility;
class UGameplayEffect;


/**
 * 
 */
UCLASS(MinimalAPI)
class UKaosGameplayAbilitySet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UKaosGameplayAbilitySet(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	KAOSGASUTILITIES_API const TArray<FKaosAbilitySet_GameplayAbility>& GetGrantedGameplayAbilities() const { return GrantedGameplayAbilities; }
	KAOSGASUTILITIES_API const TArray<FKaosAbilitySet_GameplayEffect>& GetGrantedGameplayEffects() const { return GrantedGameplayEffects; }

protected:
	KAOSGASUTILITIES_API virtual FKaosAbilitySetHandle GiveAbilitySetTo(UAbilitySystemComponent* ASC, UObject* OverrideSourceObject = nullptr) const;
	KAOSGASUTILITIES_API virtual FKaosAbilitySetHandle GiveAbilitySetToInterface(TScriptInterface<IAbilitySystemInterface> AbilitySystemInterface, UObject* OverrideSourceObject = nullptr) const;
	KAOSGASUTILITIES_API static void RemoveAllAbilitySets(UAbilitySystemComponent* ASC);

	// Gameplay abilities to grant when this ability set is granted.
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities", meta=(TitleProperty=Ability))
	TArray<FKaosAbilitySet_GameplayAbility> GrantedGameplayAbilities;

	// Gameplay effects to grant when this ability set is granted.
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects", meta=(TitleProperty=GameplayEffect))
	TArray<FKaosAbilitySet_GameplayEffect> GrantedGameplayEffects;

	// Gameplay effects to grant when this ability set is granted.
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects", meta=(TitleProperty=AttributeSet))
	TArray<FKaosAbilitySet_AttributeSet> GrantedAttributeSets;

	friend class UKaosUtilitiesBlueprintLibrary;
};
