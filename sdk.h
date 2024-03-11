#pragma once
#include "win_utils.h"
#include "driver.h"

DWORD_PTR bonearray;
FTransform GetBoneIndex(DWORD_PTR mesh, int index)
{
	int isCached = read<int>(mesh + OFFSETS::BONE_ARRAY_CACHE);
	bonearray = read<DWORD_PTR>(mesh + OFFSETS::BONE_ARRAY);
	if (bonearray == NULL)
	{
		bonearray = read<DWORD_PTR>(mesh + OFFSETS::BONE_ARRAY + 0x10);
	}
	return read<FTransform>(bonearray + (index * 0x60));
}

Vector3 getbone(uintptr_t mesh, int bone_id)
{

	int is_cached = read<int>(mesh + BONE_ARRAY_CACHE);
	uintptr_t bone_array = read<uintptr_t>(mesh + BONE_ARRAY + (is_cached * 0x10));
	FTransform bone = read<FTransform>(bone_array + (bone_id * 0x60));
	FTransform component_to_world = read<FTransform>(mesh + COMPONENT_TO_WORLD);
	D3DMATRIX matrix = MatrixMultiplication(bone.ToMatrixWithScale(), component_to_world.ToMatrixWithScale());
	return Vector3(matrix._41, matrix._42, matrix._43);
}

D3DXMATRIX Matrix(Vector3 rot, Vector3 origin = Vector3(0, 0, 0))
{
	float radPitch = (rot.x * float(M_PI) / 180.f);
	float radYaw = (rot.y * float(M_PI) / 180.f);
	float radRoll = (rot.z * float(M_PI) / 180.f);

	float SP = sinf(radPitch);
	float CP = cosf(radPitch);
	float SY = sinf(radYaw);
	float CY = cosf(radYaw);
	float SR = sinf(radRoll);
	float CR = cosf(radRoll);
	D3DMATRIX matrix;
	matrix.m[0][0] = CP * CY;
	matrix.m[0][1] = CP * SY;
	matrix.m[0][2] = SP;
	matrix.m[0][3] = 0.f;

	matrix.m[1][0] = SR * SP * CY - CR * SY;
	matrix.m[1][1] = SR * SP * SY + CR * CY;
	matrix.m[1][2] = -SR * CP;
	matrix.m[1][3] = 0.f;

	matrix.m[2][0] = -(CR * SP * CY + SR * SY);
	matrix.m[2][1] = CY * SR - CR * SP * SY;
	matrix.m[2][2] = CR * CP;
	matrix.m[2][3] = 0.f;

	matrix.m[3][0] = origin.x;
	matrix.m[3][1] = origin.y;
	matrix.m[3][2] = origin.z;
	matrix.m[3][3] = 1.f;

	return matrix;
}

struct CamewaDescwipsion
{
	float FieldOfView;
	Vector3 Rotation;
	Vector3 Location;
};

CamewaDescwipsion GetViewPoint()
{
	
	CamewaDescwipsion ViewPoint;

	auto location_pointer = read<uintptr_t>( cache::uworld + 0x110);
	auto rotation_pointer = read<uintptr_t>( cache::uworld + 0x120);

	struct FNRot
	{
		double a;
		char pad_0008[24];
		double b;
		char pad_0028[424];
		double c;
	}fnRot;

	fnRot.a = read<double>( rotation_pointer);
	fnRot.b = read<double>( rotation_pointer + 0x20);
	fnRot.c = read<double>( rotation_pointer + 0x1d0);

	ViewPoint.Location = read<Vector3>( location_pointer);
	ViewPoint.Rotation.x = asin(fnRot.c) * (180.0 / M_PI);
	ViewPoint.Rotation.y = ((atan2(fnRot.a * -1, fnRot.b) * (180.0 / M_PI)) * -1) * -1;
	ViewPoint.FieldOfView = read<float>( (uintptr_t)cache::player_controller + 0x394) * 90.f;

	return ViewPoint;
}

Vector3 ProjectWorldToScreen(Vector3 WorldLocation)
{
	CamewaDescwipsion ViewPoint = GetViewPoint();
	D3DMATRIX tempMatrix = Matrix(ViewPoint.Rotation);
	Vector3 vAxisX = Vector3(tempMatrix.m[0][0], tempMatrix.m[0][1], tempMatrix.m[0][2]);
	Vector3 vAxisY = Vector3(tempMatrix.m[1][0], tempMatrix.m[1][1], tempMatrix.m[1][2]);
	Vector3 vAxisZ = Vector3(tempMatrix.m[2][0], tempMatrix.m[2][1], tempMatrix.m[2][2]);
	Vector3 vDelta = WorldLocation - ViewPoint.Location;
	Vector3 vTransformed = Vector3(vDelta.Dot(vAxisY), vDelta.Dot(vAxisZ), vDelta.Dot(vAxisX));
		if (vTransformed.z < 1.f) // payson1337
			vTransformed.z = 1.f;
	return Vector3((Width / 2.0f) + vTransformed.x * (((Width / 2.0f) / tanf(ViewPoint.FieldOfView * (float)M_PI / 360.f))) / vTransformed.z, (Height / 2.0f) - vTransformed.y * (((Width / 2.0f) / tanf(ViewPoint.FieldOfView * (float)M_PI / 360.f))) / vTransformed.z, 0);
}

typedef struct _FNlEntity
{
	uint64_t Actor;
	int ID;
	uint64_t mesh;
}FNlEntity;

std::vector<FNlEntity> entityList;