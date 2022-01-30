#ifndef CUBEMODEL_H

#include "Model.h"


class CubeModel : public Model
{
	unsigned nbr_indices = 0;

public:

	CubeModel(
		ID3D11Device* dx3ddevice,
		ID3D11DeviceContext* dx3ddevice_context);

	virtual void Render() const;

	~CubeModel() { }
}; 

#endif // !CUBEMODEL_H