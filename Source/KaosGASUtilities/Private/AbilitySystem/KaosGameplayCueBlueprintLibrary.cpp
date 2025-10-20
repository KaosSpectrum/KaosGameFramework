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

#include "AbilitySystem/KaosGameplayCueBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayCueManager.h"
#include "AbilitySystemGlobals.h"

void UKaosGameplayCueBlueprintLibrary::AddGameplayCueLocal(AActor* Target, FGameplayTag GameplayCueTag, const FGameplayCueParameters& CueParameters)
{
	UGameplayCueManager::AddGameplayCue_NonReplicated(Target, GameplayCueTag, CueParameters);
}

void UKaosGameplayCueBlueprintLibrary::RemoveGameplayCueLocal(AActor* Target, FGameplayTag GameplayCueTag, const FGameplayCueParameters& CueParameters)
{
	UGameplayCueManager::RemoveGameplayCue_NonReplicated(Target, GameplayCueTag, CueParameters);
}

void UKaosGameplayCueBlueprintLibrary::ExecuteGameplayCueLocal(AActor* Target, FGameplayTag GameplayCueTag, const FGameplayCueParameters& CueParameters)
{
	UGameplayCueManager::ExecuteGameplayCue_NonReplicated(Target, GameplayCueTag, CueParameters);
}

void UKaosGameplayCueBlueprintLibrary::BuildCueParametersFromSource(AActor* SourceActor, FGameplayCueParameters& OutCueParameters)
{
	if (!SourceActor)
	{
		return;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(SourceActor);
	IGameplayTagAssetInterface* GameplayTagAssetInterface = Cast<IGameplayTagAssetInterface>(SourceActor);
	FGameplayTagContainer SourceTags;
	if (ASC)
	{
		ASC->GetOwnedGameplayTags(SourceTags);
	}
	else if (GameplayTagAssetInterface)
	{
		GameplayTagAssetInterface->GetOwnedGameplayTags(SourceTags);
	}
	OutCueParameters.AggregatedSourceTags.AppendTags(SourceTags);
	OutCueParameters.Location = SourceActor->GetActorLocation();
	OutCueParameters.Instigator = SourceActor;
}

void UKaosGameplayCueBlueprintLibrary::BuildCueParametersFromHitResult(AActor* SourceActor, FHitResult& HitResult, FGameplayCueParameters& OutCueParameters)
{
	if (!SourceActor)
	{
		return;
	}

	const UAbilitySystemComponent* SourceASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(SourceActor);
	const IGameplayTagAssetInterface* SourceGameplayTagAssetInterface = Cast<IGameplayTagAssetInterface>(SourceActor);
	FGameplayTagContainer SourceTags;

	if (SourceASC)
	{
		SourceASC->GetOwnedGameplayTags(SourceTags);
	}
	else if (SourceGameplayTagAssetInterface)
	{
		SourceGameplayTagAssetInterface->GetOwnedGameplayTags(SourceTags);
	}

	const UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(HitResult.GetActor());
	const IGameplayTagAssetInterface* TargetGameplayTagAssetInterface = Cast<IGameplayTagAssetInterface>(HitResult.GetActor());
	FGameplayTagContainer TargetTags;

	if (TargetASC)
	{
		TargetASC->GetOwnedGameplayTags(TargetTags);
	}
	else if (TargetGameplayTagAssetInterface)
	{
		TargetGameplayTagAssetInterface->GetOwnedGameplayTags(TargetTags);
	}

	OutCueParameters.AggregatedSourceTags.AppendTags(SourceTags);
	OutCueParameters.AggregatedTargetTags.AppendTags(TargetTags);
	OutCueParameters.Location = HitResult.Location;
	OutCueParameters.Normal = HitResult.Normal;
	OutCueParameters.Instigator = SourceActor;
	OutCueParameters.PhysicalMaterial = HitResult.PhysMaterial;
}
