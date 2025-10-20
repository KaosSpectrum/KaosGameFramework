// Copyright (C) 2024, Daniel Moss
// 
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#include "KaosMathStatics.h"

#include "GameFramework/Actor.h"

float UKaosMathStatics::TimeSpanToFloat(const FTimespan& InTimespan)
{
	int32 Counter = InTimespan.GetSeconds();
	Counter += InTimespan.GetMinutes() * 60;
	Counter += InTimespan.GetHours() * 3600;
	Counter += InTimespan.GetDays() * 86400;
	return Counter;
}

TArray<float> UKaosMathStatics::ExponentialDistributionRandomSampleSize(float Mean, int32 SampleSize, bool bUseLowerBounds, float LowerBounds, bool bUseUpperBounds, float UpperBounds)
{
	constexpr float RegularizationFactor = 1.f;
	TArray<float> OutFloats;

	Mean = FMath::Clamp(Mean, 0.f, 1.f);
	LowerBounds = FMath::Clamp(LowerBounds, 0.f, 1.f);
	UpperBounds = FMath::Clamp(UpperBounds, 0.f, 1.f);

	float MinE = !bUseLowerBounds ? 1 : FMath::Exp((-LowerBounds / Mean));
	float MaxE = !bUseUpperBounds ? 0 : FMath::Exp((-UpperBounds / Mean));

	MaxE = FMath::Clamp(MaxE, 0.f, 1.f);
	MinE = FMath::Clamp(MinE, 0.f, 1.f);

	for (int32 i = 0; i < SampleSize; i++)
	{
		float Result = MinE + (MaxE - MinE) * FMath::FRand();
		Result = -Mean * FMath::LogX(Result, 2.7182818284590451 * ((bUseLowerBounds != false || bUseUpperBounds != false) ? 1 : RegularizationFactor));
		OutFloats.Add(Result);
	}
	return OutFloats;
}

TArray<FVector> UKaosMathStatics::GetEquidistantTargetLocationsInCone(const FVector& InMiddleTargetLocation, const FVector& InCenterLocation, float InDistBetweenTargets, int32 InNumTargets)
{
	const double DistFromTarget = FVector::Dist(InCenterLocation, InMiddleTargetLocation);
	double Angle = 2 * FMath::Asin((InDistBetweenTargets / (2 * DistFromTarget)));
	Angle = FMath::RadiansToDegrees(Angle);

	const int32 NumRotationsForStartPos = InNumTargets / 2;
	FVector RotationVector = InMiddleTargetLocation - InCenterLocation;
	RotationVector = RotationVector.RotateAngleAxis(NumRotationsForStartPos * Angle, FVector::UpVector);
	if (InNumTargets % 2 == 0)
	{
		RotationVector = RotationVector.RotateAngleAxis(Angle / 2, FVector::DownVector);
	}

	TArray<FVector> OutVectors;
	for (int32 i = 0; i < InNumTargets; ++i)
	{
		OutVectors.Add(InCenterLocation + RotationVector.RotateAngleAxis(i * Angle, FVector::DownVector));
	}
	return OutVectors;
}

void UKaosMathStatics::ClampPointWithinCone(const FVector& ConeOrigin, const FVector& ConeDirection, float ConeHalfAngleDegrees, FVector& TargetPoint, bool& bTargetPointAdjusted)
{
	const FVector ToTarget = TargetPoint - ConeOrigin;
	const float TargetDistance = ToTarget.Size();
	if (TargetDistance > KINDA_SMALL_NUMBER)
	{
		const FVector ToTargetDir = ToTarget / TargetDistance;
		const float DotToTarget = ToTargetDir | ConeDirection;
		// Outside cone angle?
		if (FMath::Acos(DotToTarget) > FMath::DegreesToRadians(ConeHalfAngleDegrees))
		{
			// Project along cone axis, then rotate towards target to get the result.
			const FVector AxisPoint = ConeDirection * TargetDistance;
			// Cross product for point directly behind cone would be zero
			if (!FMath::IsNearlyEqual(DotToTarget, -1.0f, SMALL_NUMBER))
			{
				//Get the cross product of ToTargetDir and ConeDirection and normalize it.
				const FVector RotationAxis = (ToTargetDir ^ ConeDirection).GetSafeNormal();
				TargetPoint = ConeOrigin + AxisPoint.RotateAngleAxis(-ConeHalfAngleDegrees, RotationAxis);
			}
			else
			{
				TargetPoint = AxisPoint;
			}
			bTargetPointAdjusted = true;
			return;
		}
	}

	// Either too close to the origin or within the cone so do nothing.
	bTargetPointAdjusted = false;
}

bool UKaosMathStatics::IsInCone(const FVector& ConeOrigin, const FVector& ConeDirection, float ConeHalfAngle, const FVector& Point)
{
	const float Dot = ConeDirection | (Point - ConeOrigin);
	const float CosineSquared = FMath::Pow(Dot, 2) / (FVector::DistSquared(ConeOrigin, Point) * ConeDirection.SizeSquared());

	const float CosineAngleSquared = FMath::Pow(FMath::Cos(ConeHalfAngle / 180.0f * PI), 2);

	return Dot > 0 && CosineSquared >= CosineAngleSquared;
}

float UKaosMathStatics::DistanceBetweenActors(AActor* Actor1, AActor* Actor2)
{
	if (Actor1 && Actor2)
	{
		return FVector::Dist(Actor1->GetActorLocation(), Actor2->GetActorLocation());
	}
	return 0.f;
}

float UKaosMathStatics::DistanceBetweenActors2D(AActor* Actor1, AActor* Actor2)
{
	if (Actor1 && Actor2)
	{
		return FVector::Dist2D(Actor1->GetActorLocation(), Actor2->GetActorLocation());
	}
	return 0.f;
}

bool UKaosMathStatics::IsPointInsidePolygon(const FVector2D& TestPoint, const TArray<FVector2D>& PolygonPoints)
{
	const int NumPoints = PolygonPoints.Num();
	float AngleSum = 0.0f;
	for (int PointIndex = 0; PointIndex < NumPoints; ++PointIndex)
	{
		const FVector2D& VecAB = PolygonPoints[PointIndex] - TestPoint;
		const FVector2D& VecAC = PolygonPoints[(PointIndex + 1) % NumPoints] - TestPoint;
		const float Angle = FMath::Sign(FVector2D::CrossProduct(VecAB, VecAC)) * FMath::Acos(FMath::Clamp(FVector2D::DotProduct(VecAB, VecAC) / (VecAB.Size() * VecAC.Size()), -1.0f, 1.0f));
		AngleSum += Angle;
	}
	return (FMath::Abs(AngleSum) > 0.001f);
}

float UKaosMathStatics::ExponentialDistributionRandom(float Mean, bool bUseLowerBounds, float LowerBounds, bool bUseUpperBounds, float UpperBounds)
{
	constexpr float RegularizationFactor = 1.f;

	const float MinE = !bUseLowerBounds ? 1 : FMath::Exp((-LowerBounds / Mean));
	const float MaxE = !bUseUpperBounds ? 0 : FMath::Exp((-UpperBounds / Mean));

	float Result = MinE + (MaxE - MinE) * FMath::FRand();
	/* 2.7182818284590451 is equivalent to C# Math.E */
	Result = -Mean * FMath::LogX(2.7182818284590451 * ((bUseLowerBounds != false || bUseUpperBounds != false) ? 1 : RegularizationFactor), Result);

	return Result;
}

float UKaosMathStatics::RandomNegateFloat(float InFloat)
{
	return FMath::FRandRange(0.f, 1.f) * -1.f;
}

bool UKaosMathStatics::LineIntersect(const FVector2D& L1Start, const FVector2D& L1End, const FVector2D& L2Start, const FVector2D& L2End, FVector2D& Intersect, float Tolerance)
{
	float Ta = (L2End - L2Start) ^ (L2Start - L1Start);
	float Tb = (L1End - L1Start) ^ (L2Start - L1Start);
	const float Denom = (L2End - L2Start) ^ (L1End - L1Start);

	if (FMath::IsNearlyZero(Ta) && FMath::IsNearlyZero(Tb))
	{
		// Lines are the same
		Intersect = (L2Start + L2End) / 2;
		return true;
	}

	if (FMath::IsNearlyZero(Denom))
	{
		// Lines are parallel
		Intersect = (L2Start + L2End) / 2;
		return false;
	}

	Ta /= Denom;
	Tb /= Denom;

	Intersect = L1Start + Ta * (L1End - L1Start);

	if (Ta >= -Tolerance && Ta <= (1.0f + Tolerance) && Tb >= -Tolerance && Tb <= (1.0f + Tolerance))
	{
		return true;
	}

	return false;
}

float UKaosMathStatics::AngleBetweenVectors(const FVector& VectorA, const FVector& VectorB)
{
	const float Dot = FVector::DotProduct(VectorA.GetSafeNormal(), VectorB.GetSafeNormal());
	return FMath::Acos(Dot);
}
