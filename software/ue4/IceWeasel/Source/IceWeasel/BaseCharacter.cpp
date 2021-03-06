// Fill out your copyright notice in the Description page of Project Settings.

#include "IceWeasel.h"
#include "BaseCharacter.h"

// Sets default values
ABaseCharacter::ABaseCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);
	
	//Create and set SpringArm component as our RootComponent
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f;
	CameraBoom->bUsePawnControlRotation = true;

	//Create a Camera Component and attach it to our SpringArm Component "CameraBoom"
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(GetMesh());

	//Initialize default values
	AimPitch = 0.0f;
	ADSBlendInterpSpeed = 10.0f;
	CameraFOV = 90.0f;
	ADSCameraFOV = 60.0f;
	bAlwaysADS = false;
}

// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	CharacterWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
}

// Called every frame
void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsSprinting)
	{
		GetCharacterMovement()->MaxWalkSpeed = CharacterWalkSpeed * 2.0f;
		GetCharacterMovement()->MaxFlySpeed = CharacterWalkSpeed * 2.0f;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = CharacterWalkSpeed;
		GetCharacterMovement()->MaxFlySpeed = CharacterWalkSpeed;
	}

	//can aim down sight on third person
	bool CanAdsOnTP = !bAlwaysADS && !IsLocallyControlled();

	if (!CanAdsOnTP)
	{
		ADSBlend = 1.0f;
	}

	if (bIsAimingDownSights)
	{
		if (CanAdsOnTP)
			ADSBlend = FMath::FInterpTo(ADSBlend, 1.0f, DeltaTime, ADSBlendInterpSpeed);
		
		Camera->FieldOfView = FMath::FInterpTo(Camera->FieldOfView, ADSCameraFOV, DeltaTime, ADSBlendInterpSpeed);
	}
	else
	{
		if (CanAdsOnTP)
			ADSBlend = FMath::FInterpTo(ADSBlend, 0.0f, DeltaTime, ADSBlendInterpSpeed);
		
		Camera->FieldOfView = FMath::FInterpTo(Camera->FieldOfView, CameraFOV, DeltaTime, ADSBlendInterpSpeed);
	}

}

// Called to bind functionality to input
void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("Sprint", this, &ABaseCharacter::Sprint);
	
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ABaseCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ABaseCharacter::CrouchButtonReleased);

	PlayerInputComponent->BindAction("ADS", IE_Pressed, this, &ABaseCharacter::ADSButtonPressed);
	PlayerInputComponent->BindAction("ADS", IE_Released, this, &ABaseCharacter::ADSButtonReleased);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ABaseCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ABaseCharacter::FireButtonReleased);
}

#pragma region Server RPCs

/*
//RPC that is Run on Server
void ABaseCharacter::Server_CalculatePitch_Implementation()
{
	CalculatePitch();
}


bool ABaseCharacter::Server_CalculatePitch_Validate()
{
	return true;
} */

//RPC that is Run on Server
void ABaseCharacter::ServerSetCrouchButtonDown_Implementation(bool IsDown)
{
	bCrouchButtonDown = IsDown;
}

bool ABaseCharacter::ServerSetCrouchButtonDown_Validate(bool IsDown)
{
	return true;
}

//RPC that is Run on Server
void ABaseCharacter::ServerSetFireButtonDown_Implementation(bool IsDown)
{
	bFireButtonDown = IsDown;
}

bool ABaseCharacter::ServerSetFireButtonDown_Validate(bool IsDown)
{
	return true;
}

//RPC that is Run on Server
void ABaseCharacter::ServerSetIsSprinting_Implementation(bool IsSprinting)
{
	bIsSprinting = IsSprinting;
}

bool ABaseCharacter::ServerSetIsSprinting_Validate(bool IsSprinting)
{
	return true;
}

//RPC that is Run on Server
void ABaseCharacter::ServerSetIsAimingDownSights_Implementation(bool IsADS)
{
	bIsAimingDownSights = IsADS;
}

bool ABaseCharacter::ServerSetIsAimingDownSights_Validate(bool IsADS)
{
	return true;
}
#pragma endregion 


#pragma region Action Mapping
void ABaseCharacter::CrouchButtonPressed()
{
	if (!CanCharacterCrouch())
		return;

	//Set the value locally first
	//If this is server, then the variable will be replicated to everyone else
	bCrouchButtonDown = true;

	Crouch();

	//in case this is not the server, then request the server to replicate the variable to everyone else except us (COND_SkipOwner)
	if (!HasAuthority())
		ServerSetCrouchButtonDown(bCrouchButtonDown);

}

void ABaseCharacter::CrouchButtonReleased()
{
	if (!CanCharacterCrouch())
		return;

	bCrouchButtonDown = false;

	UnCrouch();

	if (!HasAuthority())
		ServerSetCrouchButtonDown(bCrouchButtonDown);

}

void ABaseCharacter::ADSButtonPressed()
{
	bIsAimingDownSights = true;

	if (!HasAuthority())
		ServerSetIsAimingDownSights(bIsAimingDownSights);
}

void ABaseCharacter::ADSButtonReleased()
{
	bIsAimingDownSights = false;

	if (!HasAuthority())
		ServerSetIsAimingDownSights(bIsAimingDownSights);
}

void ABaseCharacter::FireButtonPressed()
{
	bFireButtonDown = true;

	if (!HasAuthority())
		ServerSetFireButtonDown(bFireButtonDown);
}

void ABaseCharacter::FireButtonReleased()
{
	bFireButtonDown = false;

	if (!HasAuthority())
		ServerSetFireButtonDown(bFireButtonDown);
}

#pragma endregion


bool ABaseCharacter::CanCharacterCrouch()const
{
	return !GetCharacterMovement()->IsFalling();
}

bool ABaseCharacter::CanCharacterJump()const
{
	return CanJump() && !bCrouchButtonDown;
}

bool ABaseCharacter::CanCharacterSprint()const
{
	FVector ForwardVector = GetActorForwardVector(); //normalized forward direction of the player
	FVector VelocityVector = GetCharacterMovement()->Velocity.GetSafeNormal(); //the normalized direction in which the player is moving
	
	bool IsMovingForward = false;
	bool IsMovingOnRightVector = false;

	float p = FVector::DotProduct(ForwardVector, VelocityVector); //cosine of angle between forward vector and velocity vector

	//p = 1 if player is moving forward
	//p = -1 if player is moving backward
	//p = 0 if player is moving right or left

	//we don't get exact values due to limited precision so check if p is nearly equal to 1, -1 or 0

	if (p > 0.7f) //check if dot product is nearly one
		IsMovingForward = true;

	if (p < 0.1f) //check if dot product is nearly zero
		IsMovingOnRightVector = true;
	

	return !bCrouchButtonDown && //Is not crouching
		!GetCharacterMovement()->IsFalling() && //Is not Falling
		(GetCharacterMovement()->Velocity.SizeSquared() != 0.0f) && //Is not sationary
		IsMovingForward && //Is moving forward and not backward
		!IsMovingOnRightVector; //Is NOT moving right or left
	
}

void ABaseCharacter::Sprint(float AxisValue)
{

	if (AxisValue != 0.0f && //is sprint button down
		 CanCharacterSprint() //make sure it's in valid state to sprint. Example - you shouldn't be able to sprint while crouching
		)
	{
		bIsSprinting = true;

		if (!HasAuthority())
			ServerSetIsSprinting(true);
		
	}
	else
	{
		bIsSprinting = false;

		if (!HasAuthority())
			ServerSetIsSprinting(false);

	}
}

/*
//Calculate AimPitch to be used inside animation blueprint for aimoffsets
void ABaseCharacter::CalculatePitch()
{
	FRotator ControlRotation = Controller->GetControlRotation();
	FRotator ActorRotation = GetActorRotation();

	FRotator Delta = ControlRotation - ActorRotation;

	FRotator Pitch(AimPitch, 0.0f, 0.0f);

	FRotator Final = FMath::RInterpTo(Pitch, Delta, GetWorld()->DeltaTimeSeconds, 0.0f);

	float OldAimPitch = AimPitch;
	float NewAimPitch = FMath::ClampAngle(Final.Pitch, -90.0f, 90.0f);
	
	AimPitch = NewAimPitch;

}*/

//Replicate variables
void ABaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABaseCharacter, bCrouchButtonDown, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ABaseCharacter, bFireButtonDown, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ABaseCharacter, bIsSprinting, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ABaseCharacter, bIsAimingDownSights, COND_SkipOwner);

	DOREPLIFETIME_CONDITION(ABaseCharacter, AimPitch, COND_SkipOwner);
}