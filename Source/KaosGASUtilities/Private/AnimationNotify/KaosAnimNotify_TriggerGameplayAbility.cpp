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

#include "AnimationNotify/KaosAnimNotify_TriggerGameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/KaosUtilitiesBlueprintLibrary.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"

UKaosAnimNotify_TriggerGameplayAbility::UKaosAnimNotify_TriggerGameplayAbility(): bSetNextMontageSectionIfAbilityFailed(false)
{
	bIsNativeBranchingPoint = true;
}

void UKaosAnimNotify_TriggerGameplayAbility::BranchingPointNotify(FBranchingPointNotifyPayload& BranchingPointPayload)
{
	Super::BranchingPointNotify(BranchingPointPayload);
	USkeletalMeshComponent* MeshComp = BranchingPointPayload.SkelMeshComponent;
	AActor* Actor = MeshComp ? MeshComp->GetOwner() : nullptr;
	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor);
	if (!ASC)
	{
		HandleJumpSection(MeshComp, nullptr, Cast<UAnimMontage>(BranchingPointPayload.SequenceAsset));
		return;
	}

	if (ASC->IsOwnerActorAuthoritative())
	{
		TArray<FGameplayAbilitySpec*> OutSpecs;
		ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(FGameplayTagContainer(AbilityTriggerTag), OutSpecs);
		for (const FGameplayAbilitySpec* Spec : OutSpecs)
		{
			const bool bValidAbility = Spec->Ability != nullptr;
			const bool bTagsPass = Spec->Ability->GetAssetTags().HasAll(FGameplayTagContainer(AbilityTriggerTag));
			if (bValidAbility && bTagsPass && ASC->TryActivateAbility(Spec->Handle, false))
			{
				//We are done, 
				return;
			}
		}

		HandleJumpSection(MeshComp, ASC, Cast<UAnimMontage>(BranchingPointPayload.SequenceAsset));
	}
}

FString UKaosAnimNotify_TriggerGameplayAbility::GetNotifyName_Implementation() const
{
	return AbilityTriggerTag.ToString();
}

#if WITH_EDITOR
bool UKaosAnimNotify_TriggerGameplayAbility::CanBePlaced(UAnimSequenceBase* Animation) const
{
	return (Animation && Animation->IsA(UAnimMontage::StaticClass()));
}
#endif

void UKaosAnimNotify_TriggerGameplayAbility::HandleJumpSection(USkeletalMeshComponent* MeshComponent, UAbilitySystemComponent* ASC, UAnimMontage* CurrentMontage)
{
	//Should we handle GAS(?)
	if (bSetNextMontageSectionIfAbilityFailed)
	{
		const FName CurrentMontageSectionName = MeshComponent->GetAnimInstance()->Montage_GetCurrentSection(CurrentMontage);
		if (ASC && ASC->GetCurrentMontage() == CurrentMontage)
		{
			ASC->CurrentMontageSetNextSectionName(CurrentMontageSectionName, NextMontageSectionIfAbilityFailed);
		}
		else
		{
			MeshComponent->GetAnimInstance()->Montage_SetNextSection(CurrentMontageSectionName, NextMontageSectionIfAbilityFailed, CurrentMontage);
		}
	}
}
