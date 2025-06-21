// (C) Interkaos. All Rights Reserved..


#include "TargetingSystemTasks/KaosTargetingFilterTask_GameplayTags.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayTagAssetInterface.h"

bool UKaosTargetingFilterTask_GameplayTags::ShouldFilterTarget(const FTargetingRequestHandle& TargetingHandle,
                                                               const FTargetingDefaultResultData& TargetData) const
{
	FGameplayTagContainer OwnedTags;
	
	// Try to get from the ASC first
	if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetData.HitResult.GetActor()))
	{
		ASC->GetOwnedGameplayTags(OwnedTags);
	}
	// Try to get from the Tag Asset interface next.
	else if (const IGameplayTagAssetInterface* TagAssetInterface = Cast<IGameplayTagAssetInterface>(TargetData.HitResult.GetActor()))
	{
		TagAssetInterface->GetOwnedGameplayTags(OwnedTags);
	}

	// Only if we have owned tags do we actually do any logic.
	if (OwnedTags.Num() > 0 && !GameplayTagRequirements.IsEmpty())
	{
		const bool RequirementsMet = GameplayTagRequirements.RequirementsMet(OwnedTags);
		return !RequirementsMet;
	}

	// No tags found, do not filter.
	return false;
}
