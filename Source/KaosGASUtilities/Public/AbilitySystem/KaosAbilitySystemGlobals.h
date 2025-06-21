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
#include "AbilitySystemGlobals.h"
#include "KaosAttributeSet.h"
#include "KaosUtilitiesTypes.h"
#include "UObject/Object.h"
#include "KaosAbilitySystemGlobals.generated.h"

/**
 * 
 */
UCLASS()
class KAOSGASUTILITIES_API UKaosAbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_BODY()

public:
	static UKaosAbilitySystemGlobals& Get() { return *Cast<UKaosAbilitySystemGlobals>(IGameplayAbilitiesModule::Get().GetAbilitySystemGlobals()); }
	
	TArray<FSoftObjectPath> GetCurveInitTables() const { return GetGlobalAttributeSetDefaultsTablePaths(); }

	FKaosAttributeSetInitter* GetKaosAttributeSetInitter() const;

	void InitAttributeSetDefaults(UAbilitySystemComponent* AbilitySystemComponent, const FKaosAttributeInitializationKey& Key, int32 Level, bool bInitialInit);
	void ApplyAttributeSetDefaults(UAbilitySystemComponent* AbilitySystemComponent, FGameplayAttribute& InAttribute, const FKaosAttributeInitializationKey& Key, int32 Level);
	TArray<float> GetAttributeSetValues(UClass* AttributeSetClass, FProperty* AttributeProperty, const FKaosAttributeInitializationKey& Key) const;

	virtual TSharedPtr<FKaosAttributeBasics> AllocKaosAttributeBasics() const;
	
	virtual void ReloadAttributeDefaults() override;

protected:
	virtual void AllocAttributeSetInitter() override;
};
