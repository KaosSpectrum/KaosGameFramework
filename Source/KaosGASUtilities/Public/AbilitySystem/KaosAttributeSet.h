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
#include "AttributeSet.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "KaosAttributeSet.generated.h"


class UKaosAbilitySystemComponent;

/**
 * This macro defines a set of helper functions for accessing and initializing attributes.
 *
 * The following example of the macro:
 *		ATTRIBUTE_ACCESSORS(UAresHealthSet, Health)
 * will create the following functions:
 *		static FGameplayAttribute GetHealthAttribute();
 *		float GetHealth() const;
 *		void SetHealth(float NewVal);
 *		void InitHealth(float NewVal);
 */
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

struct KAOSGASUTILITIES_API FKaosAttributeSetInitter : public FAttributeSetInitter
{
	virtual void PreloadAttributeSetData(const TArray<UCurveTable*>& CurveData) override;

	virtual void InitAttributeSetDefaults(UAbilitySystemComponent* AbilitySystemComponent, FName GroupName, int32 Level, bool bInitialInit) const override;
	virtual void ApplyAttributeDefault(UAbilitySystemComponent* AbilitySystemComponent, FGameplayAttribute& InAttribute, FName GroupName, int32 Level) const override;

	virtual TArray<float> GetAttributeSetValues(UClass* AttributeSetClass, FProperty* AttributeProperty, FName GroupName) const override;

private:
	bool IsSupportedProperty(FProperty* Property) const;

	struct FKaosAttributeDefaultValueList
	{
		void AddPair(FProperty* InProperty, float InValue)
		{
			List.Add(FKaosOffsetValuePair(InProperty, InValue));
		}

		struct FKaosOffsetValuePair
		{
			FKaosOffsetValuePair(FProperty* InProperty, float InValue)
				: Property(InProperty), Value(InValue)
			{
			}

			FProperty* Property;
			float Value;
		};

		TArray<FKaosOffsetValuePair> List;
	};

	struct FKaosAttributeSetDefaults
	{
		TMap<TSubclassOf<UAttributeSet>, FKaosAttributeDefaultValueList> DataMap;
	};

	struct FKaosAttributeSetDefaultsCollection
	{
		TArray<FKaosAttributeSetDefaults> LevelData;
	};

	TMap<FName, FKaosAttributeSetDefaultsCollection> Defaults;
};


USTRUCT(BlueprintType)
struct FKaosAttributeBasics
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FGameplayEffectContextHandle Context;

	UPROPERTY()
	TObjectPtr<UKaosAbilitySystemComponent> SourceASC = nullptr;

	UPROPERTY()
	FGameplayTagContainer SourceTags;

	UPROPERTY()
	FGameplayTagContainer SpecAssetTags;

	UPROPERTY()
	TObjectPtr<AActor> TargetAvatarActor = nullptr;

	UPROPERTY()
	FGameplayTagContainer TargetTags;

	UPROPERTY()
	TObjectPtr<AActor> SourceAvatarActor = nullptr;

	UPROPERTY()
	TObjectPtr<UObject> SourceObject = nullptr;
};

/*
*
 */
UCLASS()
class KAOSGASUTILITIES_API UKaosAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
protected:
	virtual UWorld* GetWorld() const override;

	UKaosAbilitySystemComponent* GetKaosAbilitySystemComponent() const;

	virtual void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
	{
	}

	virtual TSharedRef<FKaosAttributeBasics> GetBasicsFromModData(const FGameplayEffectModCallbackData& Data) const;
};
