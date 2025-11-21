// Copyright (C) 2025, Daniel Moss
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

#include "AbilitySystem/AbilityAsync/KaosAbilityAsync_WaitAbilityEnded.h"

#include "AbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(KaosAbilityAsync_WaitAbilityEnded)

UKaosAbilityAsync_WaitAbilityEnded* UKaosAbilityAsync_WaitAbilityEnded::WaitForAbilityEndedWithTags(
	AActor* TargetActor, FGameplayTagContainer AbilityTags, EGameplayContainerMatchType MatchType, bool bOnlyTriggerOnce)
{
	UKaosAbilityAsync_WaitAbilityEnded* Obj = NewObject<UKaosAbilityAsync_WaitAbilityEnded>();
	Obj->SetAbilityActor(TargetActor);
	Obj->FilterMode = EWaitEndFilterMode::ByTags;
	Obj->FilterTags = AbilityTags;
	Obj->FilterMatchType = MatchType;
	Obj->bOnlyTriggerOnce = bOnlyTriggerOnce;
	return Obj;
}

UKaosAbilityAsync_WaitAbilityEnded* UKaosAbilityAsync_WaitAbilityEnded::WaitForAbilityEndedOfClass(
	AActor* TargetActor, TSubclassOf<UGameplayAbility> AbilityClass, bool bOnlyTriggerOnce)
{
	UKaosAbilityAsync_WaitAbilityEnded* Obj = NewObject<UKaosAbilityAsync_WaitAbilityEnded>();
	Obj->SetAbilityActor(TargetActor);
	Obj->FilterMode = EWaitEndFilterMode::ByClass;
	Obj->FilterAbilityClass = AbilityClass;
	Obj->bOnlyTriggerOnce = bOnlyTriggerOnce;
	return Obj;
}

UKaosAbilityAsync_WaitAbilityEnded* UKaosAbilityAsync_WaitAbilityEnded::WaitForAbilityEndedInstance(
	AActor* TargetActor, FGameplayAbilitySpecHandle SpecHandle, bool bOnlyTriggerOnce)
{
	UKaosAbilityAsync_WaitAbilityEnded* Obj = NewObject<UKaosAbilityAsync_WaitAbilityEnded>();
	Obj->SetAbilityActor(TargetActor);
	Obj->FilterMode = EWaitEndFilterMode::BySpecHandle;
	Obj->FilterSpecHandle = SpecHandle;
	Obj->bOnlyTriggerOnce = bOnlyTriggerOnce;
	return Obj;
}

void UKaosAbilityAsync_WaitAbilityEnded::Activate()
{
	Super::Activate();

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		MyHandle = ASC->OnAbilityEnded.AddUObject(this, &ThisClass::OnAbilityEnded);
	}
	else
	{
		EndAction();
	}
}

void UKaosAbilityAsync_WaitAbilityEnded::EndAction()
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		ASC->OnAbilityEnded.Remove(MyHandle);
	}
	Super::EndAction();
}

void UKaosAbilityAsync_WaitAbilityEnded::OnAbilityEnded(const FAbilityEndedData& EndedData)
{
    if (!ShouldBroadcastDelegates())
    {
        EndAction();
        return;
    }

    UGameplayAbility* EndedAbility = EndedData.AbilityThatEnded;
    if (!EndedAbility)
    {
        return;
    }

    bool bMatch = false;

    switch (FilterMode)
    {
    case EWaitEndFilterMode::ByTags:
    {
        const FGameplayTagContainer& Tags = EndedAbility->GetAssetTags();
		switch (FilterMatchType)
		{
		case EGameplayContainerMatchType::Any:
			bMatch = Tags.HasAny(FilterTags);
			break;
		case EGameplayContainerMatchType::All:
			bMatch = Tags.HasAll(FilterTags);
			break;
		}
        break;
    }

    case EWaitEndFilterMode::ByClass:
    {
        bMatch = EndedAbility->IsA(FilterAbilityClass);
        break;
    }

    case EWaitEndFilterMode::BySpecHandle:
    {
        bMatch = (EndedData.AbilitySpecHandle == FilterSpecHandle);
        break;
    }
    }

    if (!bMatch)
    {
        return;
    }

    // Build and broadcast final data
    FKaosAbilityEndedData Out;
    Out.AbilityThatEnded = EndedAbility;
    Out.AbilitySpecHandle = EndedData.AbilitySpecHandle;
    Out.bWasCancelled = EndedData.bWasCancelled;

    Ended.Broadcast(Out);

    if (bOnlyTriggerOnce)
    {
        EndAction();
    }
}
