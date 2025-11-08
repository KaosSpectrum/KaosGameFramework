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

#include "GameplayTags/KaosGameplayTagStackContainer.h"
#include "GameFramework/Actor.h"
#include "GameplayTags/KaosGameplayTagStackOwnerInterface.h"
#include "KaosUtilitiesLogging.h"
#include "GameplayTagsManager.h"
#include "UObject/Stack.h"

void FKaosGameplayTagStackContainer::AddStackCount(FGameplayTag Tag, int32 StackCount)
{
	if (!Tag.IsValid())
	{
		FFrame::KismetExecutionMessage(TEXT("An invalid tag was passed to AddStack"), ELogVerbosity::Warning);
		return;
	}
	bool bMarkedAnyDirty = false;
	
	if (StackCount > 0)
	{
		if (int32* FoundIndex = TagToIndexMap.Find(Tag))
		{
			FKaosGameplayTagStack& Stack = Stacks[*FoundIndex];
			Stack.PreviousCount = Stack.StackCount;
			const int32 NewCount = Stack.StackCount + StackCount;
			Stack.StackCount = NewCount;
			TagToCountMap[Tag] = NewCount;
			if (IKaosGameplayTagStackOwnerInterface* OwnerInterface = Cast<IKaosGameplayTagStackOwnerInterface>(Owner.Get()))
			{
				OwnerInterface->OnTagStackChanged(Tag, Stack.PreviousCount, Stack.StackCount);
			}
			bMarkedAnyDirty = true;
			MarkItemDirty(Stack);
		}
		else
		{
			int32 NewStackIndex = Stacks.Emplace(Tag, StackCount);
			if (IKaosGameplayTagStackOwnerInterface* OwnerInterface = Cast<IKaosGameplayTagStackOwnerInterface>(Owner.Get()))
			{
				OwnerInterface->OnTagStackAdded(Tag, StackCount);
			}
			bMarkedAnyDirty = true;
			MarkItemDirty(Stacks[NewStackIndex]);
			TagToCountMap.Add(Tag, StackCount);
			TagToIndexMap.Add(Tag, NewStackIndex);
		}
	}

	if (bMarkedAnyDirty)
	{
		if (IKaosGameplayTagStackOwnerInterface* OwnerInterface = Cast<IKaosGameplayTagStackOwnerInterface>(Owner.Get()))
		{
			OwnerInterface->ForceReplication();
		}
	}
}

void FKaosGameplayTagStackContainer::RemoveStackCount(FGameplayTag Tag, int32 StackCount)
{
	if (!Tag.IsValid())
	{
		FFrame::KismetExecutionMessage(TEXT("An invalid tag was passed to RemoveStackCount"), ELogVerbosity::Warning);
		return;
	}

	if (StackCount <= 0)
	{
		return;
	}

	bool bMarkedAnyDirty = false;

	if (int32* FoundIndexPtr = TagToIndexMap.Find(Tag))
	{
		const int32 FoundIndex = *FoundIndexPtr;

		if (!Stacks.IsValidIndex(FoundIndex))
		{
			UE_LOG(LogKaosUtilities, Warning, TEXT("Tag %s index was invalid during RemoveStackCount; map may be stale."), *Tag.ToString());
			TagToIndexMap.Remove(Tag);
			return;
		}

		FKaosGameplayTagStack& Stack = Stacks[FoundIndex];

		if (Stack.StackCount <= StackCount)
		{
			const int32 PreviousCount = Stack.StackCount;

			Stacks.RemoveAtSwap(FoundIndex);

			if (Stacks.IsValidIndex(FoundIndex))
			{
				TagToIndexMap[Stacks[FoundIndex].Tag] = FoundIndex;
			}

			TagToCountMap.Remove(Tag);
			TagToIndexMap.Remove(Tag);

			if (IKaosGameplayTagStackOwnerInterface* OwnerInterface = Cast<IKaosGameplayTagStackOwnerInterface>(Owner.Get()))
			{
				OwnerInterface->OnTagStackRemoved(Tag, PreviousCount, 0);
			}

			bMarkedAnyDirty = true;
			MarkArrayDirty();
		}
		else
		{
			Stack.PreviousCount = Stack.StackCount;
			Stack.StackCount -= StackCount;
			TagToCountMap[Tag] = Stack.StackCount;

			if (IKaosGameplayTagStackOwnerInterface* OwnerInterface = Cast<IKaosGameplayTagStackOwnerInterface>(Owner.Get()))
			{
				OwnerInterface->OnTagStackChanged(Tag, Stack.PreviousCount, Stack.StackCount);
			}

			bMarkedAnyDirty = true;
			MarkItemDirty(Stack);
		}
	}

	if (bMarkedAnyDirty)
	{
		if (IKaosGameplayTagStackOwnerInterface* OwnerInterface = Cast<IKaosGameplayTagStackOwnerInterface>(Owner.Get()))
		{
			OwnerInterface->ForceReplication();
		}
	}
}

void FKaosGameplayTagStackContainer::RemoveStack(FGameplayTag Tag)
{
	if (!Tag.IsValid())
	{
		FFrame::KismetExecutionMessage(TEXT("An invalid tag was passed to RemoveStack"), ELogVerbosity::Warning);
		return;
	}

	if (int32* FoundIndexPtr = TagToIndexMap.Find(Tag))
	{
		const int32 FoundIndex = *FoundIndexPtr;

		if (!Stacks.IsValidIndex(FoundIndex))
		{
			UE_LOG(LogKaosUtilities, Warning, TEXT("Tag %s was not found in the stack container, but was being removed. This is a bug."), *Tag.ToString());
			TagToIndexMap.Remove(Tag);
			return;
		}

		FKaosGameplayTagStack& Stack = Stacks[FoundIndex];
		const int32 PreviousCount = Stack.StackCount;

		Stacks.RemoveAtSwap(FoundIndex);

		if (Stacks.IsValidIndex(FoundIndex))
		{
			TagToIndexMap[Stacks[FoundIndex].Tag] = FoundIndex;
		}

		TagToCountMap.Remove(Tag);
		TagToIndexMap.Remove(Tag);

		if (IKaosGameplayTagStackOwnerInterface* OwnerInterface = Cast<IKaosGameplayTagStackOwnerInterface>(Owner.Get()))
		{
			OwnerInterface->OnTagStackRemoved(Tag, PreviousCount, 0);
		}

		MarkArrayDirty();

		if (IKaosGameplayTagStackOwnerInterface* OwnerInterface = Cast<IKaosGameplayTagStackOwnerInterface>(Owner.Get()))
		{
			OwnerInterface->ForceReplication();
		}
	}
}

bool FKaosGameplayTagStackContainer::ContainsTagChildren(FGameplayTag Tag) const
{
	UGameplayTagsManager& GameplayTagsManager = UGameplayTagsManager::Get();
	FGameplayTagContainer Children = GameplayTagsManager.RequestGameplayTagChildren(Tag);
	Children.AddTagFast(Tag);
	for (const FGameplayTag& Child : Children)
	{
		//If we have at least on these tags, then we can return data.
		if (ContainsTag(Child))
		{
			return true;
		}
	}
	return false;
}

TMap<FGameplayTag, int32> FKaosGameplayTagStackContainer::GetStackCountIncludingChildren(FGameplayTag Tag, bool bExcludeParent) const
{
	TMap<FGameplayTag, int32> Result;
	UGameplayTagsManager& GameplayTagsManager = UGameplayTagsManager::Get();
	FGameplayTagContainer Children = GameplayTagsManager.RequestGameplayTagChildren(Tag);
	if (!bExcludeParent)
	{
		Children.AddTagFast(Tag);
	}
	for (const FGameplayTag& Child : Children)
	{
		if (const int32* Count = TagToCountMap.Find(Child))
		{
			Result.Add(Child, *Count);
		}
		else
		{
			//Always return 0, because we need this to return the complete child count.
			Result.Add(Child, 0);
		}
	}
	return Result;
}

void FKaosGameplayTagStackContainer::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	for (int32 Index : RemovedIndices)
	{
		if (!Stacks.IsValidIndex(Index))
		{
			continue;
		}
		FKaosGameplayTagStack& Stack = Stacks[Index];

		const FGameplayTag Tag = Stack.Tag;
		const int32 PreviousCount = Stack.StackCount;
		constexpr int32 NewCount = 0;

		TagToCountMap.Remove(Tag);

		if (IKaosGameplayTagStackOwnerInterface* OwnerInterface = Cast<IKaosGameplayTagStackOwnerInterface>(Owner.Get()))
		{
			OwnerInterface->OnTagStackRemoved(Tag, PreviousCount, NewCount);
		}
	}
}

void FKaosGameplayTagStackContainer::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (int32 Index : AddedIndices)
	{
		if (!Stacks.IsValidIndex(Index))
		{
			continue;
		}
		
		FKaosGameplayTagStack& Stack = Stacks[Index];
		Stack.PreviousCount = Stack.StackCount;
		
		TagToCountMap.Add(Stack.Tag, Stack.StackCount);
		
		if (IKaosGameplayTagStackOwnerInterface* OwnerInterface = Cast<IKaosGameplayTagStackOwnerInterface>(Owner.Get()))
		{
			OwnerInterface->OnTagStackAdded(Stack.Tag, Stacks[Index].StackCount);
		}
	}
}

void FKaosGameplayTagStackContainer::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (int32 Index : ChangedIndices)
	{
		if (!Stacks.IsValidIndex(Index))
		{
			continue;
		}
		
		FKaosGameplayTagStack& Stack = Stacks[Index];
		TagToCountMap[Stack.Tag] = Stack.StackCount;
		Stack.PreviousCount = Stack.StackCount;
		
		if (IKaosGameplayTagStackOwnerInterface* OwnerInterface = Cast<IKaosGameplayTagStackOwnerInterface>(Owner.Get()))
		{
			OwnerInterface->OnTagStackChanged(Stack.Tag, Stacks[Index].PreviousCount, Stacks[Index].StackCount);
		}
	}
}
