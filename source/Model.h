#pragma once
#include "Mesh.h"
#include "SQTTransform.h"

#include <glm/glm.hpp>

struct ShaderProgram;
struct aiScene;
struct aiNode;
class CubeMap;

/**
@brief A class that is used as the template to instantiate Object instances.
@details Each Object contain a pointer to its Model. Many Object instances can have the same underlying Model.
As such Model member variables are not allowed to be changed except the shader that it uses.
once after initialization. A Model owns the Assimp Scene that it loads. Disgards of the scene after construction once 
the relevant data structure have been loaded. Model also own the its Mesh objects so it manages the lifetime of Mesh.
Between the GameWorld -> Object -> Model -> Mesh -> Vertex hierarchy it is Model that associates with a given ShaderProgram.
some of the code is adapted from http://learnopengl.com/#!Model-Loading/Model
*/
class Model
{
public:
	/**@brief Contruct a model by giving the modelTable loaded from lua config file.
		 @details Usually it is the responsibility of ModelManager to initialize and load models
	 */
	Model(const std::string &name, const std::string &modelDir, ShaderProgram *shaderProgram);

	/**Deletes the Mesh objects of the model*/
	virtual ~Model();

protected:

public:

protected:
	//used by subclasses only
	Model(const std::string &name);


};

