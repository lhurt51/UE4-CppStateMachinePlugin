// Fill out your copyright notice in the Description page of Project Settings.

#include "Quest.h"

// Sets default values for this components properties
UQuestStatus::UQuestStatus()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame
	bWantsBeginPlay = true;
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UQuestStatus::BeginPlay()
{
	Super::BeginPlay();
}

// called every frame
void UQuestStatus::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UQuestStatus::UpdateQuests(UStateMach_InputAtom* QuestActivity)
{
	// Update the master list of everything we've ever done
	QuestActivities.Add(QuestActivity);

	// Update individual quests (if they care about this activity) and see if they are complete
	for (int32 i = QuestList.Num() - 1; i >= 0; --i)
	{
		if (QuestList[i].UpdateQuest(this, QuestActivity))
		{
			UE_LOG(LogTemp, Warning, TEXT("Quest \"%s\" completed: %s"), *QuestList[i].Quest->QuestName.ToString(), (QuestList[i].QuestProgress == EQuestCompletion::EQC_Succeeded) ? TEXT("Success") : TEXT("Failure"));
		}
	}
}


