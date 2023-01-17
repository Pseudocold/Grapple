// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Crosshair.h"
#include "HookProjectile.h"
#include "MainCharacter.generated.h"

UCLASS()
class GAMEDEVELOPERTEST_API AMainCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMainCharacter(const class FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	//functions
	
	//get the end position of hookshot
	UFUNCTION(BlueprintCallable, Category = "Movement")
	FVector GetHookTargetLocation() const;

	// MovementComponent getter
	UFUNCTION(BlueprintCallable, Category = "Movement")
	UMainCharacterMovementComponent* GetMainCharacterMovementComponent() const;

	// HookDrag setter
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SetHookDrag(bool wantsToHookDrag);

	//call the function in movementcomponent
	void MoveHookDrag(FVector location);

	//properties
	
	//max range of the hook can go
	UPROPERTY(EditAnywhere, blueprintReadWrite, Category = "Custom|Hook")
	float MaxRange = 1000;

	//hook shot mode
	UPROPERTY(EditAnywhere, blueprintReadWrite, Category = "Custom|Hook")
	bool bLineTracingMode = false;

	// Projectile class to spawn.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile)
	TSubclassOf<class AHookProjectile> ProjectileClass;

	//a template that can be set in a blueprint, the crosshair widget instance will be instantiated using the chosen template
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom") 
	TSubclassOf<class UUserWidget> CrosshairTemplate;
private:
	//functions
	void Jump();
	void OnHookShotClicked();
	void OnHookShotModeSwitch();

	void MoveBackandForth(float AxisValue);
	void MoveLeftandRight(float AxisValue);

	//properties
	FVector HookTargetLocation;

	//pointer of actual crosshair widget
	UCrosshair* Crosshair;
};
