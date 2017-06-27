// Copyright Teruyo Ltd.

#pragma once

#include "AIController.h"
#include "TankAIController.generated.h"


UCLASS()
class BATTLETANK_API ATankAIController : public AAIController
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Setup") // Consider EditDefaultsOnly
	float AcceptanceRadius = 8000;

private:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

};
