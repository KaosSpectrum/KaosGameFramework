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

#include "KaosGameplayTagStackContainer.h"
#include "GameFramework/Actor.h"
#include "KaosGameplayTagStackOwnerInterface.h"
#include "GameplayTagsManager.h"
#include "UObject/Stack.h"

void FKaosGameplayTagStackContainer::AddStack(FGameplayTag Tag, int32 StackCount)
{
	if (!Tag.IsValid())
	{
		FFrame::KismetExecutionMessage(TEXT("An invalid tag was passed to AddStack"), ELogVerbosity::Warning);
		return;
	}

	if (StackCount > 0)
	{
		for (FKaosGameplayTagStack& Stack : Stacks)
		{
			if (Stack.Tag == Tag)
			{
				Stack.PreviousCount = Stack.StackCount;
				const int32 NewCount = Stack.StackCount + StackCount;
				Stack.StackCount = NewCount;
				TagToCountMap[Tag] = NewCount;
				if (IKaosGameplayTagStackOwnerInterface* OwnerInterface = Cast<IKaosGameplayTagStackOwnerInterface>(Owner.Get()))
				{
					OwnerInterface->OnTagStackChanged(Tag, Stack.PreviousCount, Stack.StackCount);
				}
				MarkItemDirty(Stack);
				return;
			}
		}

		FKaosGameplayTagStack& NewStack = Stacks.Emplace_GetRef(Tag, StackCount);
		if (IKaosGameplayTagStackOwnerInterface* OwnerInterface = Cast<IKaosGameplayTagStackOwnerInterface>(Owner.Get()))
		{
			OwnerInterface->OnTagStackAdded(Tag, StackCount);
		}
		MarkItemDirty(NewStack);
		TagToCountMap.Add(Tag, StackCount);
	}
	if (AActor* Actor = Cast<AActor>(Owner.Get()))
	{
		Actor->FlushNetDormancy();
	}
}

void FKaosGameplayTagStackContainer::RemoveStack(FGameplayTag Tag, int32 StackCount)
{
	if (!Tag.IsValid())
	{
		FFrame::KismetExecutionMessage(TEXT("An invalid tag was passed to RemoveStack"), ELogVerbosity::Warning);
		return;
	}

	if (StackCount > 0)
	{
		for (auto It = Stacks.CreateIterator(); It; ++It)
		{
			FKaosGameplayTagStack& Stack = *It;
			if (Stack.Tag == Tag)
			{
				if (Stack.StackCount <= StackCount)
				{
					It.RemoveCurrent();
					TagToCountMap.Remove(Tag);
					if (IKaosGameplayTagStackOwnerInterface* OwnerInterface = Cast<IKaosGameplayTagStackOwnerInterface>(Owner.Get()))
					{
						OwnerInterface->OnTagStackRemoved(Tag, StackCount);
					}
					MarkArrayDirty();
				}
				else
				{
					Stack.PreviousCount = Stack.StackCount;
					const int32 NewCount = Stack.StackCount - StackCount;
					Stack.StackCount = NewCount;
					TagToCountMap[Tag] = NewCount;
					if (IKaosGameplayTagStackOwnerInterface* OwnerInterface = Cast<IKaosGameplayTagStackOwnerInterface>(Owner.Get()))
					{
						OwnerInterface->OnTagStackChanged(Tag, Stack.PreviousCount, NewCount);
					}
					MarkItemDirty(Stack);
				}
				return;
			}
		}
	}

	if (AActor* Actor = Cast<AActor>(Owner.Get()))
	{
		Actor->FlushNetDormancy();
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

TMap<FGameplayTag, int32> FKaosGameplayTagStackContainer::GetStackCountIncludingChildren(FGameplayTag Tag) const
{
	TMap<FGameplayTag, int32> Result;
	UGameplayTagsManager& GameplayTagsManager = UGameplayTagsManager::Get();
	FGameplayTagContainer Children = GameplayTagsManager.RequestGameplayTagChildren(Tag);
	Children.AddTagFast(Tag);
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
		const FGameplayTag Tag = Stacks[Index].Tag;
		TagToCountMap.Remove(Tag);
		if (IKaosGameplayTagStackOwnerInterface* OwnerInterface = Cast<IKaosGameplayTagStackOwnerInterface>(Owner.Get()))
		{
			OwnerInterface->OnTagStackRemoved(Tag, Stacks[Index].StackCount);
		}
	}
}

void FKaosGameplayTagStackContainer::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (int32 Index : AddedIndices)
	{
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
		FKaosGameplayTagStack& Stack = Stacks[Index];
		TagToCountMap[Stack.Tag] = Stack.StackCount;
		Stack.PreviousCount = Stack.StackCount;
		if (IKaosGameplayTagStackOwnerInterface* OwnerInterface = Cast<IKaosGameplayTagStackOwnerInterface>(Owner.Get()))
		{
			OwnerInterface->OnTagStackChanged(Stack.Tag, Stacks[Index].PreviousCount, Stacks[Index].StackCount);
		}
	}
}
