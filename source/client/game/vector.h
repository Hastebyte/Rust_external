#pragma once
#ifndef VECTOR_H
#define VECTOR_H

#include <math.h>

#define M_PI 3.14159265358979323846264338327950288419716939937510

class vector2
{
public:
	vector2( ) : x( 0.f ), y( 0.f )
	{

	}

	vector2( float _x, float _y ) : x( _x ), y( _y )
	{

	}
	~vector2( )
	{

	}

	float x;
	float y;
};

class vector3
{
public:
	vector3( ) : x( 0.f ), y( 0.f ), z( 0.f )
	{

	}

	vector3( float _x, float _y, float _z ) : x( _x ), y( _y ), z( _z )
	{

	}
	~vector3( )
	{

	}

	float x;
	float y;
	float z;

	inline float Dot( vector3 v )
	{
		return x * v.x + y * v.y + z * v.z;
	}

	inline float Distance( vector3 v )
	{
		return float( sqrtf( powf( v.x - x, 2.0 ) + powf( v.y - y, 2.0 ) + powf( v.z - z, 2.0 ) ) );
	}

	inline float Length( )
	{
		return sqrtf( Dot( *this ) );
	}

	vector3 operator+( vector3 v )
	{
		return vector3( x + v.x, y + v.y, z + v.z );
	}

	vector3 operator-( vector3 v )
	{
		return vector3( x - v.x, y - v.y, z - v.z );
	}

	vector3 operator*( float number ) const
	{
		return vector3( x * number, y * number, z * number );
	}
};

class vector4
{
public:
	vector4( ) : x( 0.f ), y( 0.f ), z( 0.f ), w( 0.f )
	{

	}

	vector4( float _x, float _y, float _z, float _w ) : x( _x ), y( _y ), z( _z ), w( _w )
	{

	}
	~vector4( )
	{

	}

	float x;
	float y;
	float z;
	float w;
};

struct transform_access_ready_only
{
	uintptr_t transform_data;
};

struct transform_data
{
	uintptr_t transform_array;
	uintptr_t transform_indices;
};

struct matrix_34
{
	vector4 vec0;
	vector4 vec1;
	vector4 vec2;
};

#endif