
#pragma once
#ifndef SCENE_H
#define SCENE_H

#include "stdafx.h"
#include "InputHandler.h"
#include "Camera.h"
#include "Model.h"
#include "CubeModel.h"
#include "Texture.h"

// New files
// Material
// Texture <- stb

// TEMP


class Scene
{
protected:
	ID3D11Device*			dxdevice;
	ID3D11DeviceContext*	dxdevice_context;
	int						window_width;
	int						window_height;

public:

	Scene(
		ID3D11Device* dxdevice,
		ID3D11DeviceContext* dxdevice_context,
		int window_width,
		int window_height);

	virtual void Init() = 0;

	virtual void Update(
		float dt,
		InputHandler* input_handler) = 0;
	
	virtual void Render() = 0;
	
	virtual void Release() = 0;

	virtual void WindowResize(
		int window_width,
		int window_height);
};

class OurTestScene : public Scene
{
	//
	// Constant buffers (CBuffers) for data that is sent to shaders
	//

	// CBuffer for transformation matrices
	ID3D11Buffer* transformation_buffer = nullptr;
	// CBuffer for phong-related positional matrices (light-source and camera position)
	ID3D11Buffer* lightcam_buffer = nullptr;
	// CBuffer for phong components k_a, k_d, k_s and (alpha). respectively, ambient, diffuse, and blank color and shininess.
	ID3D11Buffer* phong_component_buffer = nullptr;
	// 
	// CBuffer client-side definitions
	// These must match the corresponding shader definitions 
	//

	struct TransformationBuffer
	{
		mat4f ModelToWorldMatrix;
		mat4f WorldToViewMatrix;
		mat4f ProjectionMatrix;
	};

	struct LightCamBuffer
	{
		vec4f LightPositionVector;
		vec4f CameraPositionVector;
	};

	struct PhongComponentBuffer
	{

	};

	//
	// Scene content
	//
	Camera* camera;

	QuadModel* quad;
	OBJModel* sponza;
	CubeModel* cube;
	OBJModel* sphere01;
	OBJModel* sphere02;
	OBJModel* sphere03;
	OBJModel* plane;

	// Model-to-world transformation matrices
	mat4f Msponza;
	mat4f Mquad;
	mat4f Mcube;
	mat4f Msphere01;
	mat4f Msphere02;
	mat4f Msphere03;
	mat4f Mplane;

	// World-to-view matrix
	mat4f Mview;
	// Projection matrix
	mat4f Mproj;

	// Misc
	float angle = 0;			// A per-frame updated rotation angle (radians)...
	float angle_vel = fPI / 2;	// ...and its velocity (radians/sec)
	float camera_vel = 5.0f;	// Camera movement velocity in units/s
	float fps_cooldown = 0;
	vec3f pointlight_pos = { 3, 5, 0 };
	vec3f previous_pointlight_pos;
	vec3f previous_camera_pos;

	void InitTransformationBuffer();
	void InitLightCamBuffer();
	void InitPhongComponentBuffer();

	void UpdateTransformationBuffer(
		mat4f ModelToWorldMatrix,
		mat4f WorldToViewMatrix,
		mat4f ProjectionMatrix);

	void UpdateLightCamBuffer(
		vec4f LightPositionVector,
		vec4f CameraPositionVector);
	
	void UpdatePhongComponentBuffer();

public:
	OurTestScene(
		ID3D11Device* dxdevice,
		ID3D11DeviceContext* dxdevice_context,
		int window_width,
		int window_height);

	void Init() override;

	void Update(
		float dt,
		InputHandler* input_handler) override;

	void Render() override;

	void Release() override;

	void WindowResize(
		int window_width,
		int window_height) override;
};

#endif