// Copyright Teruyo Ltd.

#include "BattleTank.h"
#include "TankBarrel.h"
#include "TankTurret.h"
#include "Projectile.h"
#include "TankAimingComponent.h"


// Sets default values for this component's properties
UTankAimingComponent::UTankAimingComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	bWantsBeginPlay = true;
	PrimaryComponentTick.bCanEverTick = true;
	//PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void UTankAimingComponent::Initialise(UTankBarrel* BarrelToSet, UTankTurret* TurretToSet)
{
	Barrel = BarrelToSet;
	Turret = TurretToSet;
}

// Called when the game starts
void UTankAimingComponent::BeginPlay()
{
	Super::BeginPlay();
	LastFireTime = FPlatformTime::Seconds();
}

// Called every frame
void UTankAimingComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	//UE_LOG(LogTemp, Warning, TEXT("KAGUYA <3"));
	//UE_LOG(LogTemp, Warning, TEXT("((%f - %f) > %f) == %s"), FPlatformTime::Seconds(), LastFireTime, ReloadTimeInSeconds, ((FPlatformTime::Seconds() - LastFireTime) > ReloadTimeInSeconds) ? "True" : "False");
	if ((FPlatformTime::Seconds() - LastFireTime) < ReloadTimeInSeconds)
		FiringState = EFiringState::Reloading;
	else if (IsBarrelMoving())
		FiringState = EFiringState::Aiming;
	else
		FiringState = EFiringState::Locked;
}

bool UTankAimingComponent::IsBarrelMoving()
{
	if (!ensure(Barrel)) return false;

	auto BarrelForward = Barrel->GetForwardVector();
	return !BarrelForward.Equals(AimDirection, 0.01f);
}

void UTankAimingComponent::AimAt(FVector HitLocation) 
{
	if (!ensure(Barrel)) return;
	
	FVector OutLaunchVelocity;
	FVector StartLocation = Barrel->GetSocketLocation(FName("Projectile"));
	//UE_LOG(LogTemp, Warning, TEXT("Barrel Location: %s"), *StartLocation.ToString());
	//UE_LOG(LogTemp, Warning, TEXT("HitLocation: %s"), *HitLocation.ToString());
	
	bool bHaveAimSolution = UGameplayStatics::SuggestProjectileVelocity
	(
		this,
		OutLaunchVelocity,
		StartLocation,
		HitLocation,
		LaunchSpeed,
		false,
		0,
		0,
		ESuggestProjVelocityTraceOption::DoNotTrace
	);

	if (bHaveAimSolution)
	{
		AimDirection = OutLaunchVelocity.GetSafeNormal();
		MoveBarrelTowards();
		auto Time = GetWorld()->GetTimeSeconds();
	}
	else 
	{
		auto Time = GetWorld()->GetTimeSeconds();
	}
}

void UTankAimingComponent::MoveBarrelTowards()
{
	if (!ensure(Barrel) || !ensure(Turret)) return;
	auto BarrelRotator = Barrel->GetForwardVector().Rotation();
	auto AimAsRotator = AimDirection.Rotation();
	auto DeltaRotator = AimAsRotator - BarrelRotator;
	//UE_LOG(LogTemp, Warning, TEXT("AimAsRotator: %s"), *AimAsRotator.ToString());

	Barrel->Elevate(DeltaRotator.Pitch);
	Turret->Rotate(DeltaRotator.Yaw);
}

void UTankAimingComponent::Fire()
{
	if (FiringState == EFiringState::Reloading) return;

	if (!ensure(Barrel)) return;
	if (!ensure(ProjectileBlueprint)) return;

	/*bool isReloaded = (FPlatformTime::Seconds() - LastFireTime) > ReloadTimeInSeconds;
	if (isReloaded)
	{*/
	auto Projectile = GetWorld()->SpawnActor<AProjectile>(
		ProjectileBlueprint,
		Barrel->GetSocketLocation(FName("Projectile")),
		Barrel->GetSocketRotation(FName("Projectile"))
	);

	Projectile->LaunchProjectile(LaunchSpeed);
	LastFireTime = FPlatformTime::Seconds();
}