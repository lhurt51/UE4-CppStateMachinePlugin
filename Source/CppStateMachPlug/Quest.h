// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Components/ActorComponent.h"
#include "StateMach_State.h"
#include "Quest.generated.h"

UENUM()
enum class EQuestCompletion : uint8
{
	EQC_NotStarted,
	EQC_Started,
	EQC_Succeeded,
	EQC_Failed
};

/**
 * 
 */
UCLASS()
class CPPSTATEMACHPLUG_API UQuest : public UDataAsset
{
	GENERATED_BODY()
	
public:

	// The name of the quest
	UPROPERTY(EditAnywhere)
		FText QuestName;

	// If the machine accepts our quest activities log, the quest is successful
	UPROPERTY(EditAnywhere)
		UStateMach_State* QuestStateMachine;

	// If true the input list is a blacklist. Otherwise, its a whitelist
	UPROPERTY(EditAnywhere)
		uint32 bInputBlacklist : 1;

	// The blacklist/whitelist (depending on bBlacklist) used to filter imput atoms this Quest recognizes
	UPROPERTY(EditAnywhere)
		TArray<UStateMach_InputAtom*> InputList;

	virtual void OnSucceeded(class UQuestStatus* QuestStatus) const;
	virtual void OnFailed(class UQuestStatus* QuestStatus) const;
	
};

USTRUCT()
struct FQuestInProgress
{
	GENERATED_USTRUCT_BODY()

	// Quest data assets
	UPROPERTY(EditAnywhere)
		const UQuest* Quest;

	// Current progress in the quest
	UPROPERTY(EditAnywhere)
		EQuestCompletion QuestProgress;

protected:
	// All input for this quest. Filtered by the quest blacklist/whitelist
	UPROPERTY(EditAnywhere)
		TArray<UStateMach_InputAtom*> QuestActivities;

public:
	// Returns true if the quest was completed (success or failure) by the new activity
	bool UpdateQuest(const UObject *ObjectRef, UStateMach_InputAtom* QuestActivity)
	{
		// Only log activity to valid, in-progress quests. Check the blacklist/whitelist before logging 
		if (Quest && (QuestProgress == EQuestCompletion::EQC_Started) && (Quest->bInputBlacklist != Quest->InputList.Contains(QuestActivity)))
		{
			FStateMachineResult QuestResult;
			QuestActivities.Add(QuestActivity);
			QuestResult = Quest->QuestStateMachine->RunState(ObjectRef, QuestActivities);
			switch (QuestResult.CompletionType)
			{
			case EStateMachineCompletionType::Accepted:
				QuestProgress = EQuestCompletion::EQC_Succeeded;
				return true;
			case EStateMachineCompletionType::Rejected:
				QuestProgress = EQuestCompletion::EQC_Failed;
				return true;
				// Case not accepted: Still in progress, no update needed
			}
		}
		return false;
	}

	static FQuestInProgress NewQuestInProgress(const UQuest *_Quest)
	{
		FQuestInProgress NewQIP;
		NewQIP.Quest = _Quest;
		NewQIP.QuestProgress = EQuestCompletion::EQC_Started;
		return NewQIP;
	}

};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CPPSTATEMACHPLUG_API UQuestStatus : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this components properties
	UQuestStatus();

	// Called when the game starts
	virtual void BeginPlay() override;

	// Called ever frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);

	// Add to our quest activity log! This also automatically checks to see if any unfinnished quest are now complete
	UFUNCTION(BlueprintCallable, category = "Quests")
		void UpdateQuests(UStateMach_InputAtom* QuestActivity);

	// add a new quest-in-progress entry, or begin the quest provided if it is already on the list and hasnt been started yet
	UFUNCTION(BlueprintCallable, category = "Quests")
		bool BeginQuest(const UQuest *Quest);

protected:
	// The master list of all quest-related things we've done
	UPROPERTY(EditAnywhere)
		TArray<UStateMach_InputAtom*> QuestActivities;

	// The list of quests in our current game or area
	UPROPERTY(EditAnywhere)
		TArray<FQuestInProgress> QuestList;

};

UCLASS()
class CPPSTATEMACHPLUG_API UQuestWithResults : public UQuest
{
	GENERATED_BODY()

public:

	virtual void OnSucceeded(class UQuestStatus *QuestStatus) const override;
	virtual void OnFailed(class UQuestStatus *QuestStatus) const override;

protected:
	// The quests in the list will go from NotStarted to Started if the current quest succeeds
	UPROPERTY(EditAnywhere)
		TArray<UQuest*> SuccessQuests;

	// Input atoms to add if the quest succeeds
	UPROPERTY(EditAnywhere)
		TArray<UStateMach_InputAtom*> SuccessInputs;

	// The quests in this list will go from not started to started if the current quest fails
	UPROPERTY(EditAnywhere)
		TArray<UQuest*> FailureQuests;

	// Input atoms to add if the quest fails
	UPROPERTY(EditAnywhere)
		TArray<UStateMach_InputAtom*> FailureInputs;

};