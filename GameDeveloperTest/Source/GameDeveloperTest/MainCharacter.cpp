// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacter.h"
#include "MainCharacterMovementComponent.h"
#include "DrawDebugHelpers.h"

// Sets default values
AMainCharacter::AMainCharacter(const class FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer.SetDefaultSubobjectClass<UMainCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMainCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (CrosshairTemplate) {
		APlayerController* const PC = CastChecked<APlayerController>(Controller);
		Crosshair = CreateWidget<UCrosshair>(PC, CrosshairTemplate, FName("Crosshair"));
		Crosshair->AddToViewport();
	}
}

// Called every frame
void AMainCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMainCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction(FName("Jump"), EInputEvent::IE_Pressed, this, &AMainCharacter::Jump);
	PlayerInputComponent->BindAction(FName("HookShot"), EInputEvent::IE_Pressed, this, &AMainCharacter::OnHookShotClicked);
	PlayerInputComponent->BindAction(FName("HookShotModeSwitch"), EInputEvent::IE_Pressed, this, &AMainCharacter::OnHookShotModeSwitch);

	PlayerInputComponent->BindAxis(FName("MoveForward"), this, &AMainCharacter::MoveBackandForth);
	PlayerInputComponent->BindAxis(FName("MoveRight"), this, &AMainCharacter::MoveLeftandRight);

	PlayerInputComponent->BindAxis(FName("LookUp"), this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis(FName("LookRight"), this, &APawn::AddControllerYawInput);
}

FVector AMainCharacter::GetHookTargetLocation() const
{
	return HookTargetLocation;
}

// MovementComponent getter
UMainCharacterMovementComponent* AMainCharacter::GetMainCharacterMovementComponent() const
{
	return static_cast<UMainCharacterMovementComponent*>(GetCharacterMovement());
}

void AMainCharacter::SetHookDrag(bool wantsToHookDrag) {
	UMainCharacterMovementComponent* MovementComponent = GetMainCharacterMovementComponent();
	if (MovementComponent)
	{
		if (!HasAuthority())
		{
			MovementComponent->ServerSetHookDragRPC(wantsToHookDrag);
		}
		else
		{
			MovementComponent->SetHookDrag(wantsToHookDrag);
		}
	}
}

void AMainCharacter::Jump()
{
	ACharacter::Jump();
	SetHookDrag(false);
}

void AMainCharacter::OnHookShotClicked()
{
	UWorld* World = GetWorld();
	if (!World) return;

	// get viewport
	FVector CameraLocation;
	FRotator CameraRotation;
	GetController()->GetPlayerViewPoint(CameraLocation, CameraRotation);

	if (ProjectileClass && !bLineTracingMode)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();

		// Spawn the projectile at the muzzle.
		AHookProjectile* Projectile = World->SpawnActor<AHookProjectile>(ProjectileClass, CameraLocation, CameraRotation, SpawnParams);
		if (Projectile)
		{
			Projectile->OwnerCharacter = this;
			// Set the projectile's initial trajectory.
			FVector LaunchDirection = CameraRotation.Vector();
			Projectile->FireInDirection(LaunchDirection);
			Projectile->SetLifeSpan(MaxRange / Projectile->ProjectileMovementComponent->InitialSpeed);
		}
	}
	else // line tracing
	{
		FHitResult HitResult;
		if (GetWorld()->LineTraceSingleByObjectType(HitResult, CameraLocation, CameraLocation + CameraRotation.Vector() * MaxRange, FCollisionObjectQueryParams(ECollisionChannel::ECC_WorldStatic), FCollisionQueryParams::DefaultQueryParam))
		{
			// if hit something
			if (HitResult.GetActor())
			{
				MoveHookDrag(HitResult.Location);
				DrawDebugLine(World, CameraLocation, HookTargetLocation, FColor(255, 0, 0), false, 3.f, 0.f, 10.f);
			}
		}
	}
}

void AMainCharacter::OnHookShotModeSwitch()
{
	bLineTracingMode = !bLineTracingMode;

	if (bLineTracingMode)
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor(255, 255, 255), TEXT("switched to line tracing mode"));
	else
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor(255, 255, 255), TEXT("switched to projectile mode"));
}

void AMainCharacter::MoveHookDrag(FVector location)
{
	HookTargetLocation = location;
	SetHookDrag(true);
}

void AMainCharacter::MoveBackandForth(float AxisValue)
{
	AddMovementInput(GetActorForwardVector() * AxisValue);
}

void AMainCharacter::MoveLeftandRight(float AxisValue)
{
	AddMovementInput(GetActorRightVector() * AxisValue);
}

