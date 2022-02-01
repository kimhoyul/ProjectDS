// Fill out your copyright notice in the Description page of Project Settings.


#include "DSCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Kismet/KismetSystemLibrary.h"
#include "LockOnTargetComponent.h"
#include "LockOnArmComponent.h"

//////////////////////////////////////////////////////////////////////////
// ADSCharacter
ADSCharacter::ADSCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);
	
	/* Lock On 설정 */
	LockOnControlRotationRate = 10.f;	
	//Soft Lock Off
	TargetSwitchMouseDelta = 3.f;		
	TargetSwitchMinDelaySeconds = .5f;	
	//Soft Lock On
	BreakLockMouseDelta = 10.f;			
	BrokeLockAimingCooldown = .5f;	
	
	// 컨트롤러의 Pitch, Yaw, Roll 값 사용하지 않기. 카메라에만 영향을 미치게 설정.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// 캐릭터 이동 설정
	GetCharacterMovement()->bOrientRotationToMovement = true; 	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); 
	GetCharacterMovement()->JumpZVelocity = 600.f;  
	GetCharacterMovement()->AirControl = 0.0f; 
	
	// Spring Arm 생성
	CameraLockArm = CreateDefaultSubobject<ULockOnArmComponent>(TEXT("CameraLockArm")); 
	CameraLockArm->SetupAttachment(RootComponent);
	CameraLockArm->SetRelativeLocation(FVector(0.f, 0.f, 50.f));

	// Follo Camera 생성
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraLockArm, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false; 

	// target component 생성
	TargetComponent = CreateDefaultSubobject<ULockOnTargetComponent>(TEXT("TargetComponent"));
	TargetComponent->SetupAttachment(GetRootComponent());
}
// Called to bind functionality to input
void ADSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	PlayerInputComponent->BindAxis("MoveForward", this, &ADSCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ADSCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ADSCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ADSCharacter::LookUp);
	PlayerInputComponent->BindAction("ChangeLeft",IE_Pressed,this,&ADSCharacter::ChangeL);
	PlayerInputComponent->BindAction("ChangeLeft",IE_Released,this,&ADSCharacter::UChangeL);
	PlayerInputComponent->BindAction("ToggleCameraLock", IE_Pressed, CameraLockArm, &ULockOnArmComponent::ToggleCameraLock);
	PlayerInputComponent->BindAction("ToggleSoftLock", IE_Pressed, CameraLockArm, &ULockOnArmComponent::ToggleSoftLock);
}	

void ADSCharacter::ChangeL()
{
	IsChangeTarget = true;
	float TimeSinceLastTargetSwitch = GetWorld()->GetRealTimeSeconds() - LastTargetSwitchTime;
	//Lock On 시
	if (CameraLockArm->IsCameraLockedToTarget())
	{
		// Soft Lock On 상태일때 타깃 변경
		if (CameraLockArm->bUseSoftLock && IsChangeTarget)
		{
			CameraLockArm->BreakTargetLock();
			BrokeLockTime = GetWorld()->GetRealTimeSeconds(); // 쿨다운을 위한 시간 저장
			CameraLockArm->bSoftlockRequiresReset = true; // 소프트락 재설정 여부 true 
		}
		// Soft Lock Off 상태일때
		else if (IsChangeTarget && TimeSinceLastTargetSwitch > TargetSwitchMinDelaySeconds)
		{
			CameraLockArm->SwitchTarget(EDirection::Left);
		}
			LastTargetSwitchTime = GetWorld()->GetRealTimeSeconds(); // 스위칭 쿨타임 위한 시간 저장
	}
	else
	{
		// If camera lock was recently broken by a large mouse delta, allow a cooldown time to prevent erratic camera movement
		bool bRecentlyBrokeLock = (GetWorld()->GetRealTimeSeconds() - BrokeLockTime) < BrokeLockAimingCooldown;
		if (!bRecentlyBrokeLock)
		{
			
		}
	}
}

void ADSCharacter::UChangeL()
{
	IsChangeTarget = false;
}

// 전후 입력 LockOn / LockOff 구분
void ADSCharacter::MoveForward(float Val)
{
	if ((Controller != NULL) && (Val != .0f))
	{
		// Lock Off = Controller의 로테이션값 반환 / Lock On = (타깃 로케이션 - 나의 로케이션)A의 제곱근의 합을 구한후 A를 역제곱근한 값을 1로 나누어 나온값을 다시 A에 곱한후 로테이션값으로 변환 
		// 블루프린트 Get Unit Direction (vector) 에 해당
		const FRotator Rotation = CameraLockArm->CameraTarget == nullptr ? Controller->GetControlRotation() : (CameraLockArm->CameraTarget->GetOwner()->GetActorLocation() - GetActorLocation()).GetSafeNormal().Rotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Val);
	}
}

// 좌우 입력
void ADSCharacter::MoveRight(float Val)
{
	if ((Controller != NULL) && (Val != 0.0f))
	{
		// Lock Off = Controller의 로테이션값 반환 / Lock On = (타깃 로케이션 - 나의 로케이션)A의 제곱근의 합을 구한후 A를 역제곱근한 값을 1로 나누어 나온값을 다시 A에 곱한후 로테이션값으로 변환 
		const FRotator Rotation = CameraLockArm->CameraTarget == nullptr ? Controller->GetControlRotation() : (CameraLockArm->CameraTarget->GetOwner()->GetActorLocation() - GetActorLocation()).GetSafeNormal().Rotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Val);
	}
}

// 마우스 X축 입력
void ADSCharacter::Turn(float Val)
{
	float TimeSinceLastTargetSwitch = GetWorld()->GetRealTimeSeconds() - LastTargetSwitchTime;
	//Lock On 시
	if (CameraLockArm->IsCameraLockedToTarget())
	{
		// Soft Lock On 상태일때 타깃 변경
		if (CameraLockArm->bUseSoftLock && FMath::Abs(Val) > BreakLockMouseDelta) //Soft Lock On 상태이고 마우스 움직임이 BreakLockMouseDelta 이상으로 움직이면...
		{
			CameraLockArm->BreakTargetLock(); // ...BreakTargetLock 함수를 실행해라
			BrokeLockTime = GetWorld()->GetRealTimeSeconds(); // 쿨다운을 위한 시간 저장
			CameraLockArm->bSoftlockRequiresReset = true; // 소프트락 재설정 여부 true 
		}
		// Soft Lock Off 상태일때
		else if (FMath::Abs(Val) > TargetSwitchMouseDelta // Soft Lock Off 상태이고 마우스 움직임이 TargetSwitchMouseDelta 이상으로 움직이고...
			&& TimeSinceLastTargetSwitch > TargetSwitchMinDelaySeconds)	// ...스위칭 쿨타임이 지낫으면 실행해라
		{
			if (Val < 0)
				CameraLockArm->SwitchTarget(EDirection::Left);
			else
				CameraLockArm->SwitchTarget(EDirection::Right);

			LastTargetSwitchTime = GetWorld()->GetRealTimeSeconds(); // 스위칭 쿨타임 위한 시간 저장
		}
	}
	else
	{
		// If camera lock was recently broken by a large mouse delta, allow a cooldown time to prevent erratic camera movement
		bool bRecentlyBrokeLock = (GetWorld()->GetRealTimeSeconds() - BrokeLockTime) < BrokeLockAimingCooldown;
		if (!bRecentlyBrokeLock)
			AddControllerYawInput(Val);
	}
}

//마우스 Y축 입력
void ADSCharacter::LookUp(float Val)
{
	if (!CameraLockArm->IsCameraLockedToTarget())
		AddControllerPitchInput(Val);
}



// Lock On 시 카메라 제어
void ADSCharacter::TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);

	if (CameraLockArm->IsCameraLockedToTarget())
	{
		// 플레이어에서 타겟으로 카메라 벡터 전환후 InterpTo 통하여 easein으로 이동... 
		FVector TargetVect = CameraLockArm->CameraTarget->GetComponentLocation() - CameraLockArm->GetComponentLocation();
		FRotator TargetRot = TargetVect.GetSafeNormal().Rotation();
		FRotator CurrentRot = GetControlRotation();
		FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, LockOnControlRotationRate);

		//... 하고 카메라 벡터를 틱으로 갱신
		GetController()->SetControlRotation(NewRot);
	}
}


