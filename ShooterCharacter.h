// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ShooterCharacter.generated.h"

UCLASS()
class SHOOTERZX_API AShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AShooterCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//Called for fowards/backwards input
	void MoveFoward(float Value);

	//Called for side to side input
	void MoveRight(float Value);

	/*Called via input at the give rate
	 *@param Rate this is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 **/
	void TurnAtRate(float Rate);

	/*Called via input to look up/down at a given rate.
	 *@param Rate this is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 **/
	void LookAtRate(float Rate);

	/*Rotate Controller based on mouse X movement
	 *@param Rate The input value from mouse movement
	 **/
	void Turn(float value);

	/*Rotate Controller based on mouse Y movement
	 *@param Rate The input value from mouse movement
	 **/
	void Lookup(float value);
	
	/** Called when the Fire Button is pressed */
	void FireWeapon();

	bool GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation);

	//** Set bAiming to true or false with button press */
	void AimingButtonPressed();
	void AimingButtonReleased();

	void CameraInterpZoom(float DeltaTime);
	//Set BaseTurnRate and BaseLookUpRate based on Aiming
	
	// SetBaseTurnRate and BaseLookUpRate
	void SetLookRates();
	
	void CalculateCrosshairSpread(float Deltatime);

	//Functions when fire the weapon
    void FireButtonPressed();
	void FireButtonReleased();

	void StartFireTimer();

	UFUNCTION()
    void AutoFireReset();

    /** Line trace for items under the crosshairs */
	bool TraceUnderCrosshairs(FHitResult& OutHitResult);
	
	void StartCrosshairBulletFire();

	UFUNCTION()
	void FinishCrosshairBulletFire();
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


private:
	// Camera boom positioning the camera behind the character
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,Category=Camera, meta=(AllowPrivateAccess= "true" ));
	class USpringArmComponent* CameraBoom; // Componente agregar camara
	


	//Camera that follows the character
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,Category=Camera, meta=(AllowPrivateAccess= "true" ));
	class UCameraComponent* FollowCamera; 

	//Base turn rate, in deg/sec. Other scaling may affect final turn rate.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= Camera,meta=(AllowPrivateAccess= "true"));
	float BaseTurnRate;

	//Base Look up/down rate, in deg/sec. Other scaling may affect final turn rate.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= Camera,meta=(AllowPrivateAccess= "true"));
	float BaseLookUpRate;

	// Turn rate while not aiming
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= Camera,meta=(AllowPrivateAccess= "true"));
	float HipTurnRate;

	// Look up rate when not aiming
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= Camera,meta=(AllowPrivateAccess= "true"));
	float HipLookUpRate;

    //Turn rate when aiming
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= Camera,meta=(AllowPrivateAccess= "true"));
	float AimingTurnRate;

	//Look up rate when aiming
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= Camera,meta=(AllowPrivateAccess= "true"));
	float AimingLookUpRate;

	//Scale factor for mouse look sensitivity, Turn rate when not aiming.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= Camera,meta=(AllowPrivateAccess= "true"), meta=(ClampMin="0.0", ClampMax="1.0",UIMin="0.0",UIMax="1.0"))
	float MouseHipTurnRate;
	//Scale factor for mouse look sensitivity, LookUp rate when not aiming.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= Camera,meta=(AllowPrivateAccess= "true"), meta=(ClampMin="0.0", ClampMax="1.0",UIMin="0.0",UIMax="1.0"))
	float MouseHipLookUpRate;

	//Scale factor for mouse look sensitivity, Turn rate when aiming.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= Camera,meta=(AllowPrivateAccess= "true"), meta=(ClampMin="0.0", ClampMax="1.0",UIMin="0.0",UIMax="1.0"))
	float MouseAimingTurnRate;

	//Scale factor for mouse look sensitivity, Look Up rate when  aiming.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= Camera,meta=(AllowPrivateAccess= "true"), meta=(ClampMin="0.0", ClampMax="1.0",UIMin="0.0",UIMax="1.0"))
	float MouseAimingLookUpRate;
	//Sound particles 
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category= "Combat", meta=(AllowPrivateAccess="true"))
	class USoundCue* FireSound;
	//Particles system sound
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category= "Combat", meta=(AllowPrivateAccess="true"))
	class UParticleSystem* MuzzleFlash;
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category= "Combat", meta=(AllowPrivateAccess="true"))
	class UParticleSystem* MuzzleFlash1;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category= "Combat", meta=(AllowPrivateAccess="true"))
	class UAnimMontage* HipFireMontage;

	// Particles show when impact.
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category= "Combat", meta=(AllowPrivateAccess="true"))
	UParticleSystem* ImpactParticles;

	//Smoke trail for bullets
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category= "Combat", meta=(AllowPrivateAccess="true"))
	UParticleSystem* BeamParticles;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= "Combat",meta=(AllowPrivateAccess="true"))
	bool bAiming;

	//Default camera field of view value
	float CameraDefaultFOV;

	//flield of view value for when zoomed
	float CameraZoomedFOV;

	// Current field of view this frame
	float CameraCurrentFOV;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category ="Combat", meta=(AllowPrivateAccess="true"))
	float ZoomInterpSpeed;

   //Determiner the spread of crosshairs
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Crosshairs, meta=(AllowPrivateAccess="true"))
	float CrosshairSpreadMultiplier;

	//Velocity component from crosshairs spread
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Crosshairs, meta=(AllowPrivateAccess="true"))
	float CrosshairVelocityFactor;

	//In air component for crosshairs spread
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Crosshairs, meta=(AllowPrivateAccess="true"))
	float CrosshairInAirFactor;

	//Aim component for crosshairs spread
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Crosshairs, meta=(AllowPrivateAccess="true"))
	float CrosshairAimFactor;

	//Shooting component for crosshairs spread
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Crosshairs, meta=(AllowPrivateAccess="true"))
	float CrosshairShootingFactor;

	// left mouse or right trigger
	bool bFireButtonPressed;
	//true when we can fire. false when stopped
	bool bShouldFire;

	//Rate of automatic gun fire
	float AutomaticFireRate;
	//Sets a timer between  gunshots
	FTimerHandle AutoFireTimer;
	
	float ShootTimeDuration;
	bool bFiringBullet;
	FTimerHandle CrosshairShootTimer;
 
	
public:
	FORCEINLINE USpringArmComponent* GetCameraBoom(){return CameraBoom;}
	FORCEINLINE UCameraComponent* GetFollowCamera(){return FollowCamera;}
	FORCEINLINE bool GetAiming() const{return bAiming;}

	UFUNCTION(BlueprintCallable)
    float GetCrosshairSpreadMultiplier() const;
};
