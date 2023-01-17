// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MainCharacterMovementComponent.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum ECustomMovementMode
{
	CMOVE_HookDrag		UMETA(DisplayName = "HookDrag"),
	CMOVE_MAX			UMETA(Hidden),
};

UCLASS()
class GAMEDEVELOPERTEST_API UMainCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	UMainCharacterMovementComponent();

	friend class FSavedMove_SavedCharacterMovement;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;

	// hookshot dragging implementation
	void PhysHookDrag(float deltaTime, int32 Iterations);

	bool IsCustomMovementMode(uint8 cm) const;
protected:
	//inner execute funcs
	void execSetHookDrag(bool wantsToHookDrag);

public:

	UFUNCTION(BlueprintCallable)
	bool IsHookDragging();

	//setters
	UFUNCTION(BlueprintCallable)
	void SetHookDrag(bool wantsToHookDrag);

	//rpcs
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void ServerSetHookDragRPC(bool wantsToHookDrag);
	UFUNCTION(Client, Reliable, BlueprintCallable)
	void ClientSetHookDragRPC(bool wantsToHookDrag);

	//properties
	UPROPERTY(EditAnywhere, blueprintReadWrite, Category = "Custom|Hook")
	float HookDragSpeed = 1000;

	bool bwantsToHookDrag : 1;
	FVector HookDragDirectionVector;
	FVector HookTargetLocation;
};

/** FSavedMove_Character represents a saved move on the client that has been sent to the server and might need to be played back. */
class FSavedMove_SavedCharacterMovement : public FSavedMove_Character
{

	friend class UMainCharacterMovementComponent;

public:
	typedef FSavedMove_Character Super;
	virtual void Clear() override;
	virtual uint8 GetCompressedFlags() const override;
	virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const override;
	virtual void SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character& ClientData) override;
	virtual void PrepMoveFor(ACharacter* Character) override;

	FVector savedHookTargetLocation;
	FVector savedHookDragDirectionVector;
	bool savedwantsToHookDrag : 1;
};

/** Get prediction data for a client game. Should not be used if not running as a client. Allocates the data on demand and can be overridden to allocate a custom override if desired. Result must be a FNetworkPredictionData_Client_Character. */
class FNetworkPredictionData_Client_MainCharacterMovement : public FNetworkPredictionData_Client_Character
{
public:
	FNetworkPredictionData_Client_MainCharacterMovement(const UCharacterMovementComponent& ClientMovement);
	typedef FNetworkPredictionData_Client_Character Super;
	virtual FSavedMovePtr AllocateNewMove() override;
};
