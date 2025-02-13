
#include "Scene.h"

Scene::Scene(
	ID3D11Device* dxdevice,
	ID3D11DeviceContext* dxdevice_context,
	int window_width,
	int window_height) :
	dxdevice(dxdevice),
	dxdevice_context(dxdevice_context),
	window_width(window_width),
	window_height(window_height)
{ }

void Scene::WindowResize(
	int window_width,
	int window_height)
{
	this->window_width = window_width;
	this->window_height = window_height;
}

OurTestScene::OurTestScene(
	ID3D11Device* dxdevice,
	ID3D11DeviceContext* dxdevice_context,
	int window_width,
	int window_height) :
	Scene(dxdevice, dxdevice_context, window_width, window_height)
{ 
	InitTransformationBuffer();
	// + init other CBuffers
	InitLightCamBuffer();
	InitPhongComponentBuffer();
}

//
// Called once at initialization
//
void OurTestScene::Init()
{

	camera = new Camera(
		45.0f * fTO_RAD,		// field-of-view (radians)
		(float)window_width / window_height,	// aspect ratio
		1.0f,					// z-near plane (everything closer will be clipped/removed)
		500.0f);				// z-far plane (everything further will be clipped/removed)

	// Move camera to (0,0,5)
	camera->moveTo({ 0, 0, 5 });

	// Create objects
	quad = new QuadModel(dxdevice, dxdevice_context);
	sponza = new OBJModel("assets/crytek-sponza/sponza.obj", dxdevice, dxdevice_context);
	cube = new CubeModel(dxdevice, dxdevice_context);
	sphere01 = new OBJModel("assets/sphere01/sphere01.obj", dxdevice, dxdevice_context);
	sphere02 = new OBJModel("assets/sphere02/sphere02.obj", dxdevice, dxdevice_context);
	sphere03 = new OBJModel("assets/sphere03/sphere03.obj", dxdevice, dxdevice_context);
	//plane = new OBJModel("assets/plane/plane.obj", dxdevice, dxdevice_context);
}

//
// Called every frame
// dt (seconds) is time elapsed since the previous frame
//
void OurTestScene::Update(
	float dt,
	InputHandler* input_handler)
{
	// First-person view-/camera-space movement.
	if (input_handler->IsKeyPressed(Keys::Up) || input_handler->IsKeyPressed(Keys::W))
		camera->move({ 0.0f, 0.0f, -camera_vel * dt});
	if (input_handler->IsKeyPressed(Keys::Down) || input_handler->IsKeyPressed(Keys::S))
		camera->move({ 0.0f, 0.0f, camera_vel * dt });
	if (input_handler->IsKeyPressed(Keys::Right) || input_handler->IsKeyPressed(Keys::D))
		camera->move({ camera_vel * dt, 0.0f, 0.0f });
	if (input_handler->IsKeyPressed(Keys::Left) || input_handler->IsKeyPressed(Keys::A))
		camera->move({ -camera_vel * dt, 0.0f, 0.0f });
	if (input_handler->IsKeyPressed(Keys::K))
		camera->moveTo(vec3f(- 5, 0, 0));

	// Relative mouse movement since last frame
	long mousedx = input_handler->GetMouseDeltaX();
	long mousedy = input_handler->GetMouseDeltaY();

	// Camera rotational control (yaw and pitch)
	if (mousedx != 0)
	{
		printf(std::to_string(mousedx*dt).c_str());
		printf(std::string("\n").c_str());
		camera->rotate(0, mousedx * dt, 0);
	}

	if (mousedy != 0)
	{
		printf(std::to_string(mousedx * dt).c_str());
		printf(std::string("\n").c_str());
		camera->rotate(0, 0, mousedy * dt);
	}


	// Now set/update object transformations
	// This can be done using any sequence of transformation matrices,
	// but the T*R*S order is most common; i.e. scale, then rotate, and then translate.
	// If no transformation is desired, an identity matrix can be obtained 
	// via e.g. Mquad = linalg::mat4f_identity; 

	// Quad model-to-world transformation
	Mquad = mat4f::translation(-3, -1, -2) *			// No translation
		mat4f::rotation(-angle/5, 0.0f, 1.0f, 0.0f) *	// Rotate continuously around the y-axis
		mat4f::scaling(1, 1, 1);				// Scale uniformly to 150%

	// Sponza model-to-world transformation
	Msponza = mat4f::translation(0, -5, 0) *		 // Move down 5 units
		mat4f::rotation(fPI/2, 0.0f, 1.0f, 0.0f) * // Rotate pi/2 radians (90 degrees) around y
		mat4f::scaling(0.05f);						 // The scene is quite large so scale it down to 5%

	// Cube model-to-world transformation
	Mcube = mat4f::translation(-1, -1, -2) *
		mat4f::rotation(-angle/5, 0.0f, 1.0f, 0.0f) *
		mat4f::scaling(1, 1, 1);

	// Sphere01 model-to-world transformation
	Msphere01 = mat4f::translation(-2, 0, -8) *
		mat4f::rotation(-angle/10, 0.0f, 1.0f, 0.0f) *
		mat4f::scaling(1, 1, 1);

	// Sphere02 model-to-world transformation
	Msphere02 = mat4f::rotation(0.0f, angle / 4, 1.0f, 0.0f) * 
		mat4f::translation(7, 0, 0) *
		mat4f::scaling(1, 1, 1);

	// Sphere03 model-to-world transformation
	Msphere03 = mat4f::rotation(-angle / 4, 0.0f, 1.0f, 0.0f) * 
		mat4f::translation(3, 0, 0) *
		mat4f::scaling(0.5f, 0.5f, 0.5f);

	//Mplane = mat4f::translation(-7, 0, -5) *
	//	mat4f::rotation(0.0f, 0.0f, 1.0f, 0.0f) *
	//	mat4f::scaling(1.0f, 1.0f, 1.0f);


	// Increment the rotation angle.
	angle += angle_vel * dt;

	// Print fps
	fps_cooldown -= dt;
	if (fps_cooldown < 0.0)
	{
		std::cout << "fps " << (int)(1.0f / dt) << std::endl;
//		printf("fps %i\n", (int)(1.0f / dt));
		fps_cooldown = 2.0;
	}
}

//
// Called every frame, after update
//
void OurTestScene::Render()
{
	if (!(previous_pointlight_pos == pointlight_pos)
		|| !(previous_camera_pos == camera->position))
	{
		// Converting the regular 3D light and camera positions to 4D vectors to
		// resolve buffer data-alignment issues.
		vec4f LightPositionVector(
			pointlight_pos.x,
			pointlight_pos.y,
			pointlight_pos.z,
			0);
		vec4f CameraPositionVector(
			camera->position.x,
			camera->position.y,
			camera->position.z,
			0);

		// Bind lightcam_buffer to slot b0 of the PS
		dxdevice_context->PSSetConstantBuffers(0, 1, &lightcam_buffer);
		UpdateLightCamBuffer(LightPositionVector, CameraPositionVector);

		previous_pointlight_pos = pointlight_pos;
		previous_camera_pos = camera->position;
	}

	// Bind transformation_buffer to slot b0 of the VS
	dxdevice_context->VSSetConstantBuffers(0, 1, &transformation_buffer);

	// Obtain the matrices needed for rendering from the camera
	Mview = camera->get_WorldToViewMatrix();
	Mproj = camera->get_ProjectionMatrix();

	// Load matrices + the Quad's transformation to the device and render it
	//UpdateTransformationBuffer(Mquad, Mview, Mproj);
	//quad->Render();

	// Load matrices + Sponza's transformation to the device and render it
	//UpdateTransformationBuffer(Msponza, Mview, Mproj);
	//sponza->Render();

	//UpdateTransformationBuffer(Mcube, Mview, Mproj);
	//cube->Render();

	UpdateTransformationBuffer(Msphere01, Mview, Mproj);
	sphere01->Render();

	UpdateTransformationBuffer(Msphere01 * Msphere02, Mview, Mproj);
	sphere02->Render();

	UpdateTransformationBuffer(Msphere01 * Msphere02 * Msphere03, Mview, Mproj);
	sphere03->Render();

	//UpdateTransformationBuffer(Mplane, Mview, Mproj);
	//plane->Render();
}

void OurTestScene::Release()
{
	SAFE_DELETE(quad);
	SAFE_DELETE(sponza);
	SAFE_DELETE(camera);
	SAFE_DELETE(cube);

	SAFE_RELEASE(transformation_buffer);
	// + release other CBuffers
	SAFE_RELEASE(lightcam_buffer);
	SAFE_RELEASE(phong_component_buffer);
}

void OurTestScene::WindowResize(
	int window_width,
	int window_height)
{
	if (camera)
		camera->aspect = float(window_width) / window_height;

	Scene::WindowResize(window_width, window_height);
}

void OurTestScene::InitTransformationBuffer()
{
	HRESULT hr;
	D3D11_BUFFER_DESC MatrixBuffer_desc = { 0 };
	MatrixBuffer_desc.Usage = D3D11_USAGE_DYNAMIC;
	MatrixBuffer_desc.ByteWidth = sizeof(TransformationBuffer);
	MatrixBuffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	MatrixBuffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	MatrixBuffer_desc.MiscFlags = 0;
	MatrixBuffer_desc.StructureByteStride = 0;
	ASSERT(hr = dxdevice->CreateBuffer(&MatrixBuffer_desc, nullptr, &transformation_buffer));
}

void OurTestScene::InitLightCamBuffer()
{
	HRESULT hr;
	D3D11_BUFFER_DESC MatrixBuffer_desc = { 0 };
	MatrixBuffer_desc.Usage = D3D11_USAGE_DYNAMIC;
	MatrixBuffer_desc.ByteWidth = sizeof(lightcam_buffer) * 2;
	MatrixBuffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	MatrixBuffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	MatrixBuffer_desc.MiscFlags = 0;
	MatrixBuffer_desc.StructureByteStride = 0;
	ASSERT(hr = dxdevice->CreateBuffer(&MatrixBuffer_desc, nullptr, &lightcam_buffer));
}

void OurTestScene::InitPhongComponentBuffer()
{
	HRESULT hr;
	D3D11_BUFFER_DESC MatrixBuffer_desc = { 0 };
	MatrixBuffer_desc.Usage = D3D11_USAGE_DYNAMIC;
	MatrixBuffer_desc.ByteWidth = sizeof(phong_component_buffer) * 2;
	MatrixBuffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	MatrixBuffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	MatrixBuffer_desc.MiscFlags = 0;
	MatrixBuffer_desc.StructureByteStride = 0;
	ASSERT(hr = dxdevice->CreateBuffer(&MatrixBuffer_desc, nullptr, &phong_component_buffer));
}

void OurTestScene::UpdateTransformationBuffer(
	mat4f ModelToWorldMatrix,
	mat4f WorldToViewMatrix,
	mat4f ProjectionMatrix)
{
	// Map the resource buffer, obtain a pointer and then write our matrices to it
	D3D11_MAPPED_SUBRESOURCE resource;
	dxdevice_context->Map(transformation_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	TransformationBuffer* matrix_buffer_ = (TransformationBuffer*)resource.pData;
	matrix_buffer_->ModelToWorldMatrix = ModelToWorldMatrix;
	matrix_buffer_->WorldToViewMatrix = WorldToViewMatrix;
	matrix_buffer_->ProjectionMatrix = ProjectionMatrix;
	dxdevice_context->Unmap(transformation_buffer, 0);
}

void OurTestScene::UpdateLightCamBuffer(
	vec4f LightPositionVector,
	vec4f CameraPositionVector)
{
	D3D11_MAPPED_SUBRESOURCE resource;
	dxdevice_context->Map(lightcam_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	LightCamBuffer* matrix_buffer_ = (LightCamBuffer*)resource.pData;
	matrix_buffer_->LightPositionVector = LightPositionVector;
	matrix_buffer_->CameraPositionVector = CameraPositionVector;
	dxdevice_context->Unmap(lightcam_buffer, 0);
}

void OurTestScene::UpdatePhongComponentBuffer()
{
	D3D11_MAPPED_SUBRESOURCE resource;
	dxdevice_context->Map(phong_component_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	PhongComponentBuffer* matrix_buffer_ = (PhongComponentBuffer*)resource.pData;
	// ...
	dxdevice_context->Unmap(phong_component_buffer, 0);
}