// Fill out your copyright notice in the Description page of Project Settings.
#include "ShooterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystemComponent.h"

// Sets default values
AShooterCharacter::AShooterCharacter() :
    //Base rates for turning/looking up
	BaseTurnRate(45.f),
	BaseLookUpRate(45.f),
    //Turn rates for aiming/not aiming
    HipTurnRate(90.f),
    HipLookUpRate(90.f),
    AimingTurnRate(20.f),
    AimingLookUpRate(20.f),
    //Mouses Look sensitivity scale factors
MouseHipTurnRate(1.0F),
MouseHipLookUpRate(1.0F),
MouseAimingTurnRate(0.2F),
MouseAimingLookUpRate(0.2F),
    bAiming(false),
    //Camera field of view values
    CameraDefaultFOV(0.F), //Set in BeginPlay
    CameraZoomedFOV(35.F),
    CameraCurrentFOV(0.f),
    ZoomInterpSpeed(20.f),

  //Crosshair spread factors
  CrosshairSpreadMultiplier(0.f),
  CrosshairVelocityFactor(0.f),
  CrosshairInAirFactor(0.f),
  CrosshairAimFactor(0.f),
  CrosshairShootingFactor(0.f),

//Automatic gun fire rate
  AutomaticFireRate(0.1f),
  bShouldFire(true),
  bFireButtonPressed(false),
  //Bullet fire timer variables
  ShootTimeDuration(0.05f),
  bFiringBullet(false)
    
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create a camera boom (pulls in towards the character if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.f; // The camera follows at this distance behind the character
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
    CameraBoom->SocketOffset=FVector(0.f,50.f,70.f);


	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach camera to end of boom
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Don't rotate when the controller rotates. Let the controller only affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false; // Character moves in the direction of input...
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f); // ... at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (FollowCamera)
	{
		CameraDefaultFOV=GetFollowCamera()->FieldOfView;
		CameraCurrentFOV=CameraDefaultFOV;
	}
}

void AShooterCharacter::MoveFoward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation{ Controller->GetControlRotation() };
		const FRotator YawRotation{ 0, Rotation.Yaw, 0 };

		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::X) };
		AddMovementInput(Direction, Value);
	}
}

void AShooterCharacter::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation{ Controller->GetControlRotation() };
		const FRotator YawRotation{ 0, Rotation.Yaw, 0 };

		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::Y) };
		AddMovementInput(Direction, Value);
	}
}

void AShooterCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds()); // deg/sec * sec/frame
}

void AShooterCharacter::LookAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds()); // deg/sec * sec/frame
}

void AShooterCharacter::Turn(float value)
{
	float TurnScaleFactor{};
	if (bAiming)
	{
		TurnScaleFactor=MouseAimingTurnRate;
	}
	else
	{
		TurnScaleFactor=MouseHipTurnRate;
	}
	AddControllerYawInput(value * TurnScaleFactor);
}

void AShooterCharacter::Lookup(float value)
{
	float LookUpScaleFactor{};
	if (bAiming)
	{
		LookUpScaleFactor=MouseAimingTurnRate;
	}
	else
	{
		LookUpScaleFactor=MouseHipTurnRate;
	}
	AddControllerPitchInput(value * LookUpScaleFactor);
}


void AShooterCharacter::FireWeapon()
{
	if (FireSound)
	{
		UGameplayStatics::PlaySound2D(this, FireSound);
	}
	const USkeletalMeshSocket* BarrelSocket = GetMesh()->GetSocketByName("Barrel_Socket");
	if (BarrelSocket)
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(GetMesh());

		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
		}

		FVector BeamEnd;
		bool bBeamEnd = GetBeamEndLocation(
			SocketTransform.GetLocation(), BeamEnd);
		if (bBeamEnd)
		{
			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(),
					ImpactParticles,
					BeamEnd);
			}

			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				BeamParticles,
				SocketTransform);
			if (Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
	}
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}
	
	// Start bullet fire timer for crosshairs
	StartCrosshairBulletFire();
}
	if (FireSound)
	{
		UGameplayStatics::PlaySound2D(this, FireSound);
	}
	const USkeletalMeshSocket* BarrelSocket = GetMesh()->GetSocketByName("Barrel_Socket");
	if (BarrelSocket)
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(GetMesh());

		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
		}

		FVector BeamEnd;
		bool bBeamEnd=GetBeamEndLocation(SocketTransform.GetLocation(), BeamEnd);

		if (bBeamEnd)
		{
			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, BeamEnd);
			}
			UParticleSystemComponent* Beam= UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransform);
			if (Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}

		//Start bullet fire timer for crosshair
		StartCrosshairBulletFire();
	}
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}
}


bool AShooterCharacter::GetBeamEndLocation(
	const FVector& MuzzleSocketLocation,
	FVector& OutBeamLocation)
{
	// Get current size of the viewport
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	// Get screen space location of crosshairs
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	// Get world position and direction of crosshairs
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection);

	if (bScreenToWorld) // was deprojection successful?
	{
		FHitResult ScreenTraceHit;
		const FVector Start{ CrosshairWorldPosition };
		const FVector End{ CrosshairWorldPosition + CrosshairWorldDirection * 50'000.f };

		// Set beam end point to line trace end point
		OutBeamLocation = End;

		// Trace outward from crosshairs world location
		GetWorld()->LineTraceSingleByChannel(
			ScreenTraceHit,
			Start,
			End,
			ECollisionChannel::ECC_Visibility);
		if (ScreenTraceHit.bBlockingHit) // was there a trace hit?
		{
			// Beam end point is now trace hit location
			OutBeamLocation = ScreenTraceHit.Location;
		}

		// Perform a second trace, this time from the gun barrel
		FHitResult WeaponTraceHit;
		const FVector WeaponTraceStart{ MuzzleSocketLocation };
		const FVector WeaponTraceEnd{ OutBeamLocation };
		GetWorld()->LineTraceSingleByChannel(
			WeaponTraceHit,
			WeaponTraceStart,
			WeaponTraceEnd,
			ECollisionChannel::ECC_Visibility);
		if (WeaponTraceHit.bBlockingHit) // object between barrel and BeamEndPoint?
		{
			OutBeamLocation = WeaponTraceHit.Location;
		}
		return true;
	}
	return false;
}
	//Get current size of the viewport
	FVector2D ViewPortSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewPortSize);
	}

	// Get screen pace location of crosshairs
	FVector2D CrosshairLocation(ViewPortSize.X / 2.f, ViewPortSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	// Get world position and direction of crosshairs.
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this,0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection);
	if (bScreenToWorld)
	{
		FHitResult ScreenTraceHit;
		const FVector Start{CrosshairWorldPosition};
		const FVector End{CrosshairWorldPosition + CrosshairWorldDirection * 50'000.f};

		// Set beam end point to line trace end point
		OutBeamLocation=End;
		//Trace outward from crosshairs world location
		GetWorld()->LineTraceSingleByChannel(ScreenTraceHit, Start, End, ECollisionChannel::ECC_Visibility);
		if (ScreenTraceHit.bBlockingHit)
		{
			//Beam end point is now trace hit location
			OutBeamLocation=ScreenTraceHit.Location;
				
		}
		//Perform a second trace, this time from the gun barrel
		FHitResult WeaponTraceHit;
		const FVector WeaponTraceStart{MuzzleSocketLocation};
		const FVector WeaponTraceEnd{OutBeamLocation};
		GetWorld()->LineTraceSingleByChannel(WeaponTraceHit,WeaponTraceStart,WeaponTraceEnd,ECC_Visibility);

		if (WeaponTraceHit.bBlockingHit) //object  between barrel and BeamEndPoint
			{
			//Beam end point is now trace hit location
			OutBeamLocation=WeaponTraceHit.Location;
			}
		return true;
	}
	return false;
}

void AShooterCharacter::AimingButtonPressed()
{
	bAiming = true;
}

void AShooterCharacter::AimingButtonReleased()
{
	bAiming = false;
}


void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Handle interpolation for zoom when aiming
	CameraInterpZoom(DeltaTime);
	// Change look sensitivity based on aiming
	SetLookRates();
	// Calculate crosshair spread multiplier
	CalculateCrosshairSpread(DeltaTime);

	FHitResult ItemTraceResult;
	TraceUnderCrosshairs(ItemTraceResult);
	if (ItemTraceResult.bBlockingHit)
	{
		AItem* HitItem = Cast<AItem>(ItemTraceResult.Actor);
		if (HitItem && HitItem->GetPickupWidget())
		{
			// Show Item's Pickup Widget
			HitItem->GetPickupWidget()->SetVisibility(true);
		}
	}
}
// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveFoward",this,&AShooterCharacter::MoveFoward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShooterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("LookAtRate",this,&AShooterCharacter::LookAtRate);
	PlayerInputComponent->BindAxis("TurnAtRate", this, &AShooterCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("Turn",this,&AShooterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AShooterCharacter::Lookup);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this,&ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump",IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("FireButton", IE_Pressed, this,&AShooterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("FireButton", IE_Released, this,&AShooterCharacter::FireButtonReleased);
	PlayerInputComponent->BindAction("Aimingbutton", IE_Pressed, this,&AShooterCharacter::AimingButtonPressed);
	PlayerInputComponent->BindAction("Aimingbutton",IE_Released, this, &AShooterCharacter::AimingButtonReleased);

}

float AShooterCharacter::GetCrosshairSpreadMultiplier() const
{
	return CrosshairSpreadMultiplier;
}

void AShooterCharacter::CameraInterpZoom(float DeltaTime)
{
	//Set current Camera field of view
	if (bAiming)
	{
		//Interpolate to zoomed FOV
		CameraCurrentFOV=FMath::FInterpTo(CameraCurrentFOV, CameraZoomedFOV, DeltaTime,ZoomInterpSpeed);
		GetFollowCamera()->SetFieldOfView(CameraCurrentFOV);
	}
	else
	{
		//Interpolate to default FOV
		CameraCurrentFOV=FMath::FInterpTo(CameraCurrentFOV, CameraDefaultFOV, DeltaTime,ZoomInterpSpeed);
		GetFollowCamera()->SetFieldOfView(CameraCurrentFOV);
	}
}

void AShooterCharacter::SetLookRates()
{
	if (bAiming)
	{
		BaseTurnRate=AimingTurnRate;
		BaseLookUpRate=AimingLookUpRate;
	}
	else
	{
		BaseTurnRate=HipTurnRate;
		BaseLookUpRate=HipLookUpRate;
	}
}

void AShooterCharacter::CalculateCrosshairSpread(float Deltatime)
{
	FVector2D WalkSpeedRange{0.f, 600.f};
    FVector2D VelocityMultiplierRange{0.f, 1.0f};
	FVector Velocity{GetVelocity()};
	Velocity.Z=0.f;

	
	if (GetCharacterMovement()->IsFalling())//is in air
		{
		//Spread the crosshair slowly while in air
			CrosshairInAirFactor=FMath::FInterpTo(CrosshairInAirFactor, 2.25f, Deltatime, 2.25f);
		}
	else //Character is on the ground
		
		{
		//Shrink the crosshairs rapidly while on the ground
		CrosshairInAirFactor=FMath::FInterpTo(CrosshairInAirFactor, 0.f, Deltatime, 30.f);
		}
	if (bAiming)
	{
		//Shrink crosshairs a small amount very quickly
		CrosshairAimFactor=FMath::FInterpTo(CrosshairAimFactor,0.6f,Deltatime,30.f);
	}
	else
	{
		CrosshairAimFactor=FMath::FInterpTo(CrosshairAimFactor,0.f,Deltatime,30.f);
	}
	if (bFiringBullet)
	{
		CrosshairShootingFactor=FMath::FInterpTo(CrosshairShootingFactor, 0.3f, Deltatime, 60.f);
	}
	else
	{
		CrosshairShootingFactor=FMath::FInterpTo(CrosshairShootingFactor, 0.3f, Deltatime, 60.f);
	}
	CrosshairSpreadMultiplier=0.5f+CrosshairVelocityFactor + CrosshairInAirFactor-CrosshairAimFactor + CrosshairShootingFactor;
	CrosshairVelocityFactor=FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

	
	

	
}

void AShooterCharacter::FireButtonPressed()
{
	bFireButtonPressed=true;
	StartFireTimer();
}

void AShooterCharacter::FireButtonReleased()
{
	bFireButtonPressed=false;
}

void AShooterCharacter::StartFireTimer()
{
	if (bShouldFire)
	{
		FireWeapon();
		bShouldFire=false;
		GetWorldTimerManager().SetTimer(AutoFireTimer, this, &AShooterCharacter::AutoFireReset, AutomaticFireRate );
	}
}

void AShooterCharacter::AutoFireReset()
{
	bShouldFire=true;
	if (bFireButtonPressed)
	{
		StartFireTimer();
	}
}

bool AShooterCharacter::TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation)
{
	//Get Viewport Size
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

    //Get screen space location of crosshairs
	FVector2D CrosshairLocation
	return false;
	
}


void AShooterCharacter::StartCrosshairBulletFire()
{
	bFiringBullet = true;

	GetWorldTimerManager().SetTimer(
		CrosshairShootTimer, 
		this, 
		&AShooterCharacter::FinishCrosshairBulletFire, 
		ShootTimeDuration);
}

void AShooterCharacter::FinishCrosshairBulletFire()
{
	bFiringBullet = false;
}
