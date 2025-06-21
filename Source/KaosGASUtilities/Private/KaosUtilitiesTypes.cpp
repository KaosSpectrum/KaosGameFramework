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

#include "KaosUtilitiesTypes.h"

#include "AbilitySystemComponent.h"
#include "KaosUtilitiesLogging.h"
#include "Logging/StructuredLog.h"


void FKaosAbilitySetHandle::RemoveSet()
{
	if (!AbilitySystemComponent->IsOwnerActorAuthoritative())
	{
		UE_LOGFMT(LogKaosUtilities, Warning, "Tried to remove set when not authoritive");
		return;
	}

	for (const FGameplayAbilitySpecHandle& Handle : AbilitySpecHandles)
	{
		if (Handle.IsValid())
		{
			AbilitySystemComponent->ClearAbility(Handle);
		}
	}

	for (const FActiveGameplayEffectHandle& Handle : GameplayEffectHandles)
	{
		if (Handle.IsValid())
		{
			AbilitySystemComponent->RemoveActiveGameplayEffect(Handle);
		}
	}

	// We do NOT remove any spawned sets.
	// This is because GameplayEffects from other things MIGHT be interacting with them and we CAN NOT
	// for sure know WHAT and WHY without expensive lookups. Also we might not know in the future.
	
	// for (UAttributeSet* Set : AttributeSets)
	// {
	// 	AbilitySystemComponent->RemoveSpawnedAttribute(Set);
	// }

	UE_LOGFMT(LogKaosUtilities, Log, "Removed ability set with handle {Handle}", HandleId);
	Reset();
}

void FKaosAbilitySetHandle::AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle)
{
	if (Handle.IsValid())
	{
		AbilitySpecHandles.Add(Handle);
	}
}

void FKaosAbilitySetHandle::AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle)
{
	if (Handle.IsValid())
	{
		GameplayEffectHandles.Add(Handle);
	}
}

