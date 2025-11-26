// Fill out your copyright notice in the Description page of Project Settings.


#include "MMJ/Object/Subsystem/EntitySpawnSubsystem.h"

#include "MMJ/Object/Interactable/DBDObject.h"
#include "MMJ/Object/Spawner/EntitySpawner.h"
#include "Shared/Component/InteractableComponent.h"

AEntitySpawner* UEntitySpawnSubsystem::GetEntitySpawner() const
{
	return EntitySpawner;
}

void UEntitySpawnSubsystem::SetEntitySpawner(AEntitySpawner* Spawner)
{
	if (!Spawner) return;
	EntitySpawner = Spawner;
}

AObj_Entity* UEntitySpawnSubsystem::SpawnEntity(ADBDObject* OwnerObject)
{
	if (!GetEntitySpawner() || !OwnerObject) return nullptr;

	return GetEntitySpawner()->SpawnEntity(
		OwnerObject,
		OwnerObject->GetInteractableComponent()->InteractableTag
		);
}
