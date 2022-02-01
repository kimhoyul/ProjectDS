// Fill out your copyright notice in the Description page of Project Settings.


#include "DSCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "LockOnTargetComponent.h"
#include "LockOnArmComponent.h"

//////////////////////////////////////////////////////////////////////////
// ADSCharacter
ADSCharacter::ADSCharacter()
{
	/* Lock On 설정 */
	LockonControlRotationRate = 10.f;	// Lock On 시 대상에게 이동 하는 카메라의 속도를 조절												(Default 10.f)
	//Soft Lock Off
	TargetSwitchMouseDelta = 3.f;		// Lock On 대상을 전환하기 위한 입력으로 간주되는 마우스 움직임에 대한 허용치를 조절				(Default 3.f)
	TargetSwitchMinDelaySeconds = .5f;	// Lock On 대상 전환 delay. Lock On System의 제어성을 높이기 위해 사용								(Default .5f)
	//Soft Lock On
	BreakLockMouseDelta = 10.f;			// Lock On 을 해제하기 위한 입력으로 간주되는 마우스의 움직임에 대한 저항							(Default 10.f)
	BrokeLockAimingCooldown = .5f;		// 마우스 움직임으로 Lock On 대상 전환후 플레이어의 카메라 입력 delay.								(Default .5f)

	// Gameped 아날로그 스틱
	TargetSwitchAnalogValue = .8f;		// Lock On 대상을 전환하기 위한 입력으로 간주되는 아날로그 스틱 움직임에 대한 허용치를 조절			(Default .8f)
	BaseTurnRate = 45.f;				// Gameped 아날로그 스틱 X축 방향의 Base turn rate													(Default 45.f)
	BaseLookUpRate = 45.f;				// Gameped 아날로그 스틱 Y축 방향의 LookUp Rate														(Default 45.f)

	// 캐릭터 캡슐 컴포넌트 사이즈 (Default 34.f, 88.f)
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

	// 컨트롤러의 Pitch, Yaw, Roll 값 사용하지 않기. 카메라에만 영향을 미치게 설정.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// 캐릭터 이동 설정
	GetCharacterMovement()->bOrientRotationToMovement = true; // 캐릭터의 이동방향을 설정한다...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...여기서 설정한 회전 속도로									(Default 0.f,540.f,0.f)
	GetCharacterMovement()->JumpZVelocity = 600.f;  // 캐릭터의 점프 높이																	(Default 600.f)
	GetCharacterMovement()->AirControl = 0.0f; // 캐릭터가 공중에 있을때 움직일수 있는 값 설정												(Default 0.2f)


	// Spring Arm 생성(카메라를 Block 하는 콜리전과 충돌 했을 경우 플레이어 쪽으로 끌어당긴다).
	CameraLockArm = CreateDefaultSubobject<ULockOnArmComponent>(TEXT("CameraLockArm")); // 컴퍼넌트 추가 CameraLockArm 으로 이름 설정
	CameraLockArm->SetupAttachment(RootComponent); // 컴퍼넌트의 위치는 "RootComponent"의 하위로
	CameraLockArm->SetRelativeLocation(FVector(0.f, 0.f, 50.f)); // 이 캐릭터의 컴퍼넌트 트랜스폼 값중 로케이션 값을 설정

	// Follo Camera 생성
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera")); // 컴퍼넌트 추가 FollowCamera 로 이름 설정
	FollowCamera->SetupAttachment(CameraLockArm, USpringArmComponent::SocketName); // 카메라를 Spring Arm 끝에 달아 컨트롤러의 방향에 맞추어 Spring Arm을 조정
	FollowCamera->bUsePawnControlRotation = false; // 카메라가 Spring Arm 을 기준으로 회전하지 않도록 설정

	// target component 생성 (Lock On 을 위한 바운드)
	TargetComponent = CreateDefaultSubobject<ULockOnTargetComponent>(TEXT("TargetComponent"));
	TargetComponent->SetupAttachment(GetRootComponent());

}

// Called to bind functionality to input
void ADSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// 게임플레이 키 바인딩

	/* PlayerInputComponent 가 있는지 Assert 진행 */
	check(PlayerInputComponent);

	// 키보드 입력
	PlayerInputComponent->BindAxis("MoveForward", this, &ADSCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ADSCharacter::MoveRight);

	// 마우스 입력
	PlayerInputComponent->BindAxis("Turn", this, &ADSCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ADSCharacter::LookUp);

	// Gameped 아날로그 스틱 입력
	PlayerInputComponent->BindAxis("TurnRate", this, &ADSCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ADSCharacter::LookUpAtRate);

	// Lock On 입력
	PlayerInputComponent->BindAction("ToggleCameraLock", IE_Pressed, CameraLockArm, &ULockOnArmComponent::ToggleCameraLock);
	PlayerInputComponent->BindAction("ToggleSoftLock", IE_Pressed, CameraLockArm, &ULockOnArmComponent::ToggleSoftLock);
}

// 전후 입력
void ADSCharacter::MoveForward(float Val)
{
	if ((Controller != NULL) && (Val != .0f))
	{
		// 앞뒤 어느쪽 으로 이동하는지 알아내기(Lock On 한 Target이 없으면 Controller의 로테이션값 반환하고, 타깃이 있으면 (타깃 로케이션 - 나의 로케이션)A의 제곱근의 합을 구한후 A를 역제곱근한 값을 1로 나누어 나온값을 다시 A에 곱한후 로테이션값으로 변환 
		// 블루프린트 Get Unit Direction (vector) 에 해당
		const FRotator Rotation = CameraLockArm->CameraTarget == nullptr ? Controller->GetControlRotation() : (CameraLockArm->CameraTarget->GetOwner()->GetActorLocation() - GetActorLocation()).GetSafeNormal().Rotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// 월드 또는 타깃으로부터의 forward vector 얻어서 인풋
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Val);
	}
}

// 좌우 입력
void ADSCharacter::MoveRight(float Val)
{
	if ((Controller != NULL) && (Val != 0.0f))
	{
		// 좌우 어느쪽 으로 이동하는지 알아내기
		const FRotator Rotation = CameraLockArm->CameraTarget == nullptr ? Controller->GetControlRotation() : (CameraLockArm->CameraTarget->GetOwner()->GetActorLocation() - GetActorLocation()).GetSafeNormal().Rotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// 월드 또는 타깃으로부터의 right vector 얻어서 인풋
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

//Gameped X축 입력
void ADSCharacter::TurnAtRate(float Val)
{
	// 마지막 타깃 스위치 시도 이후 아날로그 스틱이 중립으로 돌아 갔는지 확인필요
	if (FMath::Abs(Val) < .1f)
		bAnalogSettledSinceLastTargetSwitch = true;

	if (CameraLockArm->IsCameraLockedToTarget() && (FMath::Abs(Val) > TargetSwitchAnalogValue) && bAnalogSettledSinceLastTargetSwitch)
	{
		if (Val < 0)
			CameraLockArm->SwitchTarget(EDirection::Left);
		else
			CameraLockArm->SwitchTarget(EDirection::Right);

		bAnalogSettledSinceLastTargetSwitch = false;
	}
	else
	{
		// calculate delta for this frame from the rate information
		AddControllerYawInput(Val * BaseTurnRate * GetWorld()->GetDeltaSeconds());
	}
}

// Gameped Y축 입력
void ADSCharacter::LookUpAtRate(float Val)
{
	// calculate delta for this frame from the rate information
	if (!CameraLockArm->IsCameraLockedToTarget())
		AddControllerPitchInput(Val * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
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
		FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, LockonControlRotationRate);

		//... 하고 카메라 벡터를 틱으로 갱신
		GetController()->SetControlRotation(NewRot);
	}
}


