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
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayEffectTypes.h"
#include "KaosGameplayCueBlueprintLibrary.generated.h"

/**
 * 
 */
UCLASS()
class KAOSGASUTILITIES_API UKaosGameplayCueBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Add a gameplay cue locally on Target. Experimental. Use with caution. */
	UFUNCTION(BlueprintCallable, Category="KaosGAS", Meta = (GameplayTagFilter = "GameplayCue"))
	static void AddGameplayCueLocal(AActor* Target, FGameplayTag GameplayCueTag, const FGameplayCueParameters& CueParameters);

	/** Remove a gameplay cue locally on Target. Experimental. Use with caution.  */
	UFUNCTION(BlueprintCallable, Category="KaosGAS", Meta = (GameplayTagFilter = "GameplayCue"))
	static void RemoveGameplayCueLocal(AActor* Target, FGameplayTag GameplayCueTag, const FGameplayCueParameters& CueParameters);

	/** Execute a gameplay cue locally on Target */
	UFUNCTION(BlueprintCallable, Category="KaosGAS", Meta = (GameplayTagFilter = "GameplayCue"))
	static void ExecuteGameplayCueLocal(AActor* Target, FGameplayTag GameplayCueTag, const FGameplayCueParameters& CueParameters);

	/** Build a gameplay cue params from passed in source actor */
	UFUNCTION(BlueprintCallable, Category="KaosGAS")
	static void BuildCueParametersFromSource(AActor* SourceActor, FGameplayCueParameters& OutCueParameters);

	/** Build a gameplay cue params from passed in hitresult and source actor */
	UFUNCTION(BlueprintCallable, Category="KaosGAS")
	static void BuildCueParametersFromHitResult(AActor* SourceActor, FHitResult& HitResult, FGameplayCueParameters& OutCueParameters);
};
