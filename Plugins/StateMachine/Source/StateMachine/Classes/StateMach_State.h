// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "StateMach_State.generated.h"

class UStateMach_Branch;
class UStateMach_State;

UENUM()
enum class EStateMachineCompletionType : uint8
{
	// Implicit failure - this state is not marked as Accept
	NotAccepted,

	// Success - this state is an Accept state
	Accepted,

	// Explicit failure - this state is specifically marked as failure/not-accept 
	Rejected,

	// Our simulation ran out of steps while the machine was still running
	OutOfSteps UMETA(Hidden)
};

USTRUCT()
struct STATEMACHINE_API FStateMachineResult
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
		EStateMachineCompletionType	CompletionType;

	UPROPERTY()
		UStateMach_State*	FinalState;

	UPROPERTY()
		int32	DataIndex;

};

UCLASS()
class STATEMACHINE_API UStateMach_InputAtom : public UDataAsset
{
	GENERATED_BODY()

public:
	// Display value for this atom, mainly for debugging purposes
	UPROPERTY(EditAnywhere)
		FName Description;

};

UCLASS(EditInlineNew)
class STATEMACHINE_API UStateMach_Branch : public UDataAsset
{
	GENERATED_BODY()

public:
	// Returns DestinationState on success, NULL on failure. OutDataIndex might be something other than 1, if a branch is made to consume multiple inputs.
	virtual UStateMach_State* TryBranch(const UObject* RefObject, const TArray<UStateMach_InputAtom*> &DataSource, int32 DataIndex, int32 &OutDataIndex);

protected:
	// State where we will go next if this branch is taken. If NULL this branch will be ignored
	UPROPERTY(EditAnywhere)
		UStateMach_State* DestinationState;

	// If true , the meaning of AcceptableInputs is reversed
	UPROPERTY(EditAnywhere)
		uint32 bReverseInputTest : 1;

	// Acceptable inputs. The current input atom must be on this list
	UPROPERTY(EditAnywhere)
		TArray<UStateMach_InputAtom*> AcceptableInputs;

};

/**
 * 
 */
UCLASS()
class STATEMACHINE_API UStateMach_State : public UDataAsset
{
	GENERATED_BODY()
	
public:

	UStateMach_State();
	
	/** Attemp to run the state with input from the given DataSourceObject. Will start reading input at DataIndex.
		Will decrement RemainingSteps and automatically fail after it hits 0*/
	UFUNCTION(BlueprintCallable, Category = "State Machine")
		virtual FStateMachineResult RunState(const UObject* RefObject, const TArray<UStateMach_InputAtom*> &DataSource, int32 DataIndex = 0, int32 RemainingSteps = -1);

protected:

	// Loop. Used when input atom being processed isnt recognized
	virtual FStateMachineResult LoopState(const UObject* RefObject, const TArray<UStateMach_InputAtom*> &DataSource, int32 DataIndex, int32 RemainingSteps);

	// If input runs out on this state ( or TerminateImidiately is true), this is how the result will be interpreted
	UPROPERTY(EditAnywhere)
		EStateMachineCompletionType CompletionType;

	// A state machine run that enters this state will terminate imediately, regardless of whether or not there is more input data
	UPROPERTY(EditAnywhere)
		uint32 bTerminateImmediately : 1;

	// If this is set, this state will loop on itself whenever and unhandle input atom is detected
	UPROPERTY(EditAnywhere)
		uint32 bLoopByDefault : 1;

	// Instanced branches to other states. These are in priority order, so the first succesful branch will be taken
	UPROPERTY(EditAnywhere, Instanced)
		TArray<UStateMach_Branch*> InstancedBranches;

	// Branches to other states. These are in priority order, so the first succesful branch will be taken. These run after Instance Branches
	UPROPERTY(EditAnywhere)
		TArray<UStateMach_Branch*> SharedBranches;
	
};
