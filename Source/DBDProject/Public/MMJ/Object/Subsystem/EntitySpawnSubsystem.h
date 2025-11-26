// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "EntitySpawnSubsystem.generated.h"

class ADBDObject;
class AObj_Entity;
class AEntitySpawner;
/**
 * 
 */
UCLASS()
class DBDPROJECT_API UEntitySpawnSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	UFUNCTION()
	AEntitySpawner* GetEntitySpawner() const;

	UFUNCTION()
	void SetEntitySpawner(AEntitySpawner* Spawner);

	UFUNCTION()
	AObj_Entity* SpawnEntity(ADBDObject* OwnerObject);
private:
	UPROPERTY()
	AEntitySpawner* EntitySpawner;
	
};
