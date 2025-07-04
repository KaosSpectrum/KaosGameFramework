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

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "KaosGameplayTagStackContainer.generated.h"

class APlayerState;

struct FKaosGameplayTagStackContainer;
struct FNetDeltaSerializeInfo;

/*
 * How to Use FKaosGameplayTagStackContainer with Replication
 * -----------------------------------------------------------
 * This container is a fast-replicating struct that tracks gameplay tag stacks.
 * Use it in any actor or component where replicated gameplay tag stacks are needed.
 *
 * 1. Add the container as a replicated property:
 *
 *     UPROPERTY(Replicated)
 *     FKaosGameplayTagStackContainer Container;
 *
 * 2. Register it for replication in GetLifetimeReplicatedProps:
 *
 *     void UMyComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
 *     {
 *         Super::GetLifetimeReplicatedProps(OutLifetimeProps);
 *
 *         FDoRepLifetimeParams Params;
 *         Params.bIsPushBased = true; // Enable push-based replication for efficiency
 *         DOREPLIFETIME_WITH_PARAMS_FAST(UMyComponent, Container, Params);
 *     }
 *
 * 3. Set the container's owner in PostInitComponents or BeginPlay:
 *
 *     if (!IsTemplate())
 *     {
 *         Container.SetOwner(this);
 *     }
 *
 *     // The owner must implement the IDominanceGameplayTagStackOwnerInterface
 *
 * 4. Reacting to changes:
 *     The container automatically notifies its owner (on both server and client)
 *     when tag stacks are added, removed, or changed. Use this to trigger logic
 *     like UI updates, gameplay effects, or analytics.
 *
 * Notes:
 * - The FKaosGameplayTagStackContainer uses FFastArraySerializer to replicate efficiently.
 * - Tags and their stack counts are stored internally in both a replicated array and a local accelerated map.
 * - Modifying stacks should always be done through AddStack() and RemoveStack() to ensure proper replication.
 */


/**
 * Represents one stack of a gameplay tag (tag + count)
 */
USTRUCT(BlueprintType)
struct FKaosGameplayTagStack : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FKaosGameplayTagStack()
	{}

	FKaosGameplayTagStack(FGameplayTag InTag, int32 InStackCount)
		: Tag(InTag)
		, StackCount(InStackCount)
	{
	}

	FString GetDebugString() const
	{
		return FString::Printf(TEXT("%sx%d"), *Tag.ToString(), StackCount);
	}

private:
	friend FKaosGameplayTagStackContainer;

	UPROPERTY()
	FGameplayTag Tag;
	
	UPROPERTY()
	int32 StackCount = 0;
	
	UPROPERTY()
	int32 PreviousCount = 0;
};

/** Container of gameplay tag stacks */
USTRUCT(BlueprintType)
struct FKaosGameplayTagStackContainer : public FFastArraySerializer
{
	GENERATED_BODY()

	FKaosGameplayTagStackContainer()
	{
		Owner = nullptr;
	}

public:
	// Adds a specified number of stacks to the tag (does nothing if StackCount is below 1)
	void AddStack(FGameplayTag Tag, int32 StackCount);

	// Removes a specified number of stacks from the tag (does nothing if StackCount is below 1)
	void RemoveStack(FGameplayTag Tag, int32 StackCount);

	// Returns the stack count of the specified tag (or 0 if the tag is not present)
	int32 GetStackCount(FGameplayTag Tag) const
	{
		return TagToCountMap.FindRef(Tag);
	}

	// Returns true if there is at least one stack of the specified tag
	bool ContainsTag(FGameplayTag Tag) const
	{
		return TagToCountMap.Contains(Tag);
	}

    //Checks to see if a stack count contains child tags
	bool ContainsTagChildren(FGameplayTag Tag) const;

    //Returns the stack count including child tags
	TMap<FGameplayTag, int32> GetStackCountIncludingChildren(FGameplayTag Tag) const;

    //Returns the entire accelerated map.
	const TMap<FGameplayTag, int32>& GetAllStacks() const
	{
		return TagToCountMap;
	}

	//~FFastArraySerializer contract
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);

	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);

	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);

	//~End of FFastArraySerializer contract

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FKaosGameplayTagStack, FKaosGameplayTagStackContainer>(Stacks, DeltaParms, *this);
	}

	void SetOwner(UObject* InOwner)
	{
		Owner = InOwner;
	}


private:
	// Replicated list of gameplay tag stacks
	UPROPERTY()
	TArray<FKaosGameplayTagStack> Stacks;
	
	// Accelerated list of tag int count for queries
	TMap<FGameplayTag, int32> TagToCountMap;

  //Owner which has the interface IKaosGameplayTagStackOwnerInterface
	TWeakObjectPtr<UObject> Owner;
};

template<>
struct TStructOpsTypeTraits<FKaosGameplayTagStackContainer> : public TStructOpsTypeTraitsBase2<FKaosGameplayTagStackContainer>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};
