#pragma once

#include "Core.h"

namespace KBEngine
{
	class KBENGINE_API KBEMath
	{
	public:

		static inline float int82angle(int8 angle, bool half = false)
		{
			return float(angle) * float((PI / (half ? 254.f : 128.f)));
		}

		static inline bool almostEqual(const float f1, const float f2, const float epsilon = 0.0004f)
		{
			return fabsf(f1 - f2) < epsilon;
		}

		static inline FVector Unreal2KBEnginePosition(const FVector& unrealPoint)
		{
			return FVector(unrealPoint[1] * 0.01, unrealPoint[2] * 0.01, unrealPoint[0] * 0.01);
		}

		static inline FVector Unreal2KBEnginePosition(const float* unrealPoint)
		{
			return FVector(unrealPoint[1] * 0.01, unrealPoint[2] * 0.01, unrealPoint[0] * 0.01);
		}

		static inline FVector KBEngine2UnrealPosition(const FVector& kbenginePoint)
		{
			return FVector(kbenginePoint[2] * 100, kbenginePoint[0] * 100, kbenginePoint[1] * 100);
		}

		static inline FVector KBEngine2UnrealPosition(const float* kbenginePoint)
		{
			return FVector(kbenginePoint[2] * 100, kbenginePoint[0] * 100, kbenginePoint[1] * 100);
		}


		/*
		1、KBE和UE4中，描述方向的X,Y,Z两者的对应关系是一致的（数值表述是不一致的，KBE用弧度，UE4用角度）
		2、在游戏的过程中，我们实际所需要同步的只是Z轴（朝向）
		*/
		static inline FVector Unreal2KBEngineDirection(const FVector& unrealDir)
		{
			return unrealDir * 2 * PI / 360.0;
		}

		static inline FVector Unreal2KBEngineDirection(const float* unrealDir)
		{
			return Unreal2KBEngineDirection(FVector(unrealDir[0], unrealDir[1], unrealDir[2]));
		}

		static inline FVector KBEngine2UnrealDirection(const FVector& kbengineDir)
		{
			return kbengineDir * 360.0 / (2 * PI);
		}

		static inline FVector KBEngine2UnrealDirection(const float* kbengineDir)
		{
			return KBEngine2UnrealDirection(FVector(kbengineDir[0], kbengineDir[1], kbengineDir[2]));
		}

		// 四元数逆运算算法
		static FQuat QuatReverse(const FQuat& quat)
		{
			FQuat conjugate = quat.Inverse();
			float magnitude = quat.SizeSquared();
			return conjugate / magnitude;
		}

		// 世界坐标和本地坐标互換
		static FVector PositionLocalToWorld(const FVector& parentPos, const FVector& parentDir, const FVector& localPos)
		{
			FQuat Rotation = FQuat::MakeFromEuler(parentDir);
			return Rotation.RotateVector(localPos) + parentPos;
		}

		static FVector PositionWorldToLocal(const FVector& parentPos, const FVector& parentDir, const FVector& worldPos)
		{
			FQuat Rotation = FQuat::MakeFromEuler(parentDir);
			return Rotation.UnrotateVector(worldPos - parentPos);
		}

		static FVector DirectionLocalToWorld(const FVector& parentDir, const FVector& localDir)
		{
			FQuat pr = FQuat::MakeFromEuler(parentDir);
			FQuat lr = FQuat::MakeFromEuler(localDir);
			FQuat wr = pr*lr;
			return wr.Euler();
		}

		static FVector DirectionWorldToLocal(const FVector& parentDir, const FVector& worldDir)
		{
			FQuat pr = FQuat::MakeFromEuler(parentDir);
			FQuat wr = FQuat::MakeFromEuler(worldDir);
			FQuat pr_r = QuatReverse(pr); //逆运算算法
			FQuat lr = pr_r * wr;
			return lr.Euler();
		}


	};
}
