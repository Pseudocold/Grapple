// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacterMovementComponent.h"
#include "MainCharacter.h"

UMainCharacterMovementComponent::UMainCharacterMovementComponent()
{
	bwantsToHookDrag = false;
}

void UMainCharacterMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{

	if (CustomMovementMode == ECustomMovementMode::CMOVE_HookDrag)
	{
		PhysHookDrag(deltaTime, Iterations);
	}
	Super::PhysCustom(deltaTime, Iterations);
}

// hookshot dragging implementation
void UMainCharacterMovementComponent::PhysHookDrag(float deltaTime, int32 Iterations) 
{
	if (!IsCustomMovementMode(ECustomMovementMode::CMOVE_HookDrag))
	{
		StartNewPhysics(deltaTime, Iterations);
		return;
	}

	// stop dragging by clicking stop button
	if (!bwantsToHookDrag)
	{
		SetHookDrag(false);
		SetMovementMode(EMovementMode::MOVE_Falling);
		StartNewPhysics(deltaTime, Iterations);
		return;
	}

	FHitResult Hit;
	FVector MoveDelta = HookDragDirectionVector * HookDragSpeed * deltaTime;
	SafeMoveUpdatedComponent(MoveDelta, FRotator::ZeroRotator, true, Hit);

	if (Hit.IsValidBlockingHit())
	{
		SlideAlongSurface(MoveDelta, 1.f - Hit.Time, Hit.Normal, Hit, false);
	}
}

void UMainCharacterMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//calculate velocity normalized direction vector
	if (IsHookDragging())
	{
		HookDragDirectionVector = (HookTargetLocation - GetOwner()->GetActorLocation()).GetSafeNormal();
	}
}

void UMainCharacterMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	if (bwantsToHookDrag)
	{
		SetMovementMode(EMovementMode::MOVE_Custom, ECustomMovementMode::CMOVE_HookDrag);
	}

	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);
}

void UMainCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	if (PreviousMovementMode == MovementMode && PreviousCustomMode == CustomMovementMode)
	{
		return;
	}

	// stop hook drag
	if (PreviousMovementMode == EMovementMode::MOVE_Custom && PreviousCustomMode == ECustomMovementMode::CMOVE_HookDrag)
	{
		SetHookDrag(false);
	}
	
	//start hook drag
	if (IsCustomMovementMode(ECustomMovementMode::CMOVE_HookDrag))
	{
		AMainCharacter* MainCharacter = Cast<AMainCharacter>(GetCharacterOwner());
		HookTargetLocation = MainCharacter->GetHookTargetLocation();
		HookDragDirectionVector = (HookTargetLocation - GetOwner()->GetActorLocation()).GetSafeNormal();
	}
}

bool UMainCharacterMovementComponent::IsCustomMovementMode(uint8 cm) const
{
	return (MovementMode == EMovementMode::MOVE_Custom && CustomMovementMode == cm);
}

bool UMainCharacterMovementComponent::IsHookDragging() {
	return IsCustomMovementMode(ECustomMovementMode::CMOVE_HookDrag);
}

void UMainCharacterMovementComponent::SetHookDrag(bool wantsToHookDrag)
{

	if (bwantsToHookDrag != wantsToHookDrag)
	{
		execSetHookDrag(wantsToHookDrag);

		if (!GetOwner() || !GetPawnOwner())
			return;

		if (!GetOwner()->HasAuthority() && GetPawnOwner()->IsLocallyControlled())
		{
			ServerSetHookDragRPC(wantsToHookDrag);
		}
		else if (GetOwner()->HasAuthority() && !GetPawnOwner()->IsLocallyControlled())
		{
			ClientSetHookDragRPC(wantsToHookDrag);
		}

	}
}

void UMainCharacterMovementComponent::execSetHookDrag(bool wantsToHookDrag)
{
	bwantsToHookDrag = wantsToHookDrag;
}

//replication
void UMainCharacterMovementComponent::ClientSetHookDragRPC_Implementation(bool wantsToHookDrag)
{
	execSetHookDrag(wantsToHookDrag);
}

bool UMainCharacterMovementComponent::ServerSetHookDragRPC_Validate(bool wantsToHookDrag)
{
	return true;
}

void UMainCharacterMovementComponent::ServerSetHookDragRPC_Implementation(bool wantsToHookDrag)
{
	execSetHookDrag(wantsToHookDrag);
}

//network sync implementation
FNetworkPredictionData_Client*
UMainCharacterMovementComponent::GetPredictionData_Client() const
{
	check(PawnOwner != NULL);

	if (!ClientPredictionData)
	{
		UMainCharacterMovementComponent* MutableThis = const_cast<UMainCharacterMovementComponent*>(this);

		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_MainCharacterMovement(*this);
	}
	return ClientPredictionData;
}

FNetworkPredictionData_Client_MainCharacterMovement::FNetworkPredictionData_Client_MainCharacterMovement(const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{

}

FSavedMovePtr FNetworkPredictionData_Client_MainCharacterMovement::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_SavedCharacterMovement());
}

void FSavedMove_SavedCharacterMovement::Clear()
{
	Super::Clear();

	savedwantsToHookDrag = false;
}

uint8 FSavedMove_SavedCharacterMovement::GetCompressedFlags() const
{
	return Super::GetCompressedFlags();
}

bool FSavedMove_SavedCharacterMovement::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const
{
	if (savedwantsToHookDrag != ((FSavedMove_SavedCharacterMovement*)&NewMove)->savedwantsToHookDrag)
		return false;
	if (savedHookDragDirectionVector != ((FSavedMove_SavedCharacterMovement*)&NewMove)->savedHookDragDirectionVector)
		return false;
	if (savedHookTargetLocation != ((FSavedMove_SavedCharacterMovement*)&NewMove)->savedHookTargetLocation)
		return false;

	return Super::CanCombineWith(NewMove, Character, MaxDelta);
}

void FSavedMove_SavedCharacterMovement::SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);
	UMainCharacterMovementComponent* CharMov = Cast<UMainCharacterMovementComponent>(Character->GetCharacterMovement());
	if (CharMov)
	{
		savedHookTargetLocation = CharMov->HookTargetLocation;
		savedwantsToHookDrag = CharMov->bwantsToHookDrag;
		savedHookDragDirectionVector = CharMov->HookDragDirectionVector;
	}
}

void FSavedMove_SavedCharacterMovement::PrepMoveFor(ACharacter* Character)
{
	Super::PrepMoveFor(Character);
	UMainCharacterMovementComponent* CharMov = Cast<UMainCharacterMovementComponent>(Character->GetCharacterMovement());
	if (CharMov)
	{
		CharMov->HookTargetLocation = savedHookTargetLocation;
		CharMov->bwantsToHookDrag = savedwantsToHookDrag;
		CharMov->HookDragDirectionVector = savedHookDragDirectionVector;
	}
}