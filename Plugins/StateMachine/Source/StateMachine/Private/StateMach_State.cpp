// Fill out your copyright notice in the Description page of Project Settings.

#include "StateMach_State.h"

UStateMach_State::UStateMach_State()
{
	bLoopByDefault = true;
}

UStateMach_State* UStateMach_Branch::TryBranch(const UObject* RefObject, const TArray<UStateMach_InputAtom*> &DataSource, int32 DataIndex, int32 &OutDataIndex)
{
	OutDataIndex = DataIndex;
	if (!AcceptableInputs.Num() || (DataSource.IsValidIndex(DataIndex)) && AcceptableInputs.Contains(DataSource[DataIndex]))
	{
		++OutDataIndex;
		return bReverseInputTest ? nullptr : DestinationState;
	}
	return bReverseInputTest ? DestinationState : nullptr;
}

FStateMachineResult UStateMach_State::RunState(const UObject* RefObject, const TArray<UStateMach_InputAtom*> &DataSource, int32 DataIndex /* = 0*/, int32 RemainingSteps /* = -1*/)
{
	bool bMustEndNow = (bTerminateImmediately || !DataSource.IsValidIndex(DataIndex));

	// If we are still running, see where our branches lead
	if (RemainingSteps && !bMustEndNow)
	{
		UStateMach_State* DestinationState = nullptr;
		int32 DestinationDataIndex = DataIndex;
		for (int32 i = 0; i < InstancedBranches.Num(); ++i)
		{
			// This should be a check. There shouldnt be null branches in the list
			if (InstancedBranches[i])
			{
				DestinationState = InstancedBranches[i]->TryBranch(RefObject, DataSource, DataIndex, DestinationDataIndex);
				if (DestinationState)
				{
					return DestinationState->RunState(RefObject, DataSource, DestinationDataIndex, RemainingSteps - 1);
				}
			}
		}
		// We didnt find any acceptable branch, so we must end on this atate unless we are told to loop
		if (bLoopByDefault)
		{
			return LoopState(RefObject, DataSource, DataIndex, RemainingSteps);
		}
		bMustEndNow = true;
	}

	FStateMachineResult SMR;
	SMR.FinalState = this;
	SMR.DataIndex = DataIndex;
	SMR.CompletionType = bMustEndNow ? CompletionType : EStateMachineCompletionType::OutOfSteps;
	return SMR;
}

FStateMachineResult UStateMach_State::LoopState(const UObject* RefObject, const TArray<UStateMach_InputAtom*> &DataSource, int32 DataIndex, int32 RemainingSteps)
{
	// By default, increase data index by 1 and decrease remaining steps by 1
	return RunState(RefObject, DataSource, DataIndex + 1, RemainingSteps - 1);
}