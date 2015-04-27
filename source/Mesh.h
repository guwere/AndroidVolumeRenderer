#pragma once


#include <glm/glm.hpp>
#include "Common.h"

#include "SQTTransform.h"

class Shader;

/** @brief The vertex structure to be written to the gpu. Note the resulting packed layout*/
struct Vertex
{
	Vertex();
	Vertex(const glm::vec3 position);
	Vertex(float vx, float vy, float vz, float ts, float tt);
	void operator=(const glm::vec3 &other);
	glm::vec3 m_Position; //!< position in model space
	glm::vec3 m_Normal;//!< normal of vertex in model space / +z-axis in tangent space
	glm::vec2 m_TexCoord; //!< uv coordinates for vertex

};

struct Edge
{
	Edge(){};
	Edge(const glm::vec3 &point1, const glm::vec3 &point2);
	glm::vec3 p1, p2;
};


/**@brief Refer to one of the fragment shaders to see the layout of the material structure. 
The fragment shader for the mesh may use texture and/or material properties */
struct Material
{
	//various material properties needed for lighting
	glm::vec4 ambient;
	glm::vec4 diffuse;
	glm::vec4 specular;
	float shininess;
	float roughness;
	float refraction;
};

struct Shader;

/**@brief Encapsulates the buffers and texture handling for a single mesh.
@todo proper cleanup of buffer object in destructor
*/
struct Mesh
{
public:
	/** Construct a Mesh from vertices, indices and textures
		@param shader is the associated Shader for that Mesh
	*/
	Mesh(const std::vector<Vertex> &vertices,
		const std::vector<GLuint> &indices,
		Shader *shader);

	Mesh();
	virtual ~Mesh();
	/**@brief Deletes buffers*/
	void destroy();

	/**@brief Create a VAO for @param shader from the already loaded vertices */
	virtual void createVAO(const Shader *shader);
	/**@brief Writes material properties, textures and draws the triangles. Defaults to GL_TRIANGLES*/
	virtual void render() const;	
	/**@brief Writes material properties, textures and draws the triangles*/
	virtual void render(GLenum mode, bool drawElements) const;

	void renderWireframe() const;
	std::vector<Vertex> getTransformedVertices(const glm::mat4 &transformMatrix) const;
	std::vector<glm::vec3> getTransformedPositions(const glm::mat4 &transformMatrix) const;
	std::vector<Edge> getTransformedEdges(const glm::mat4 &transformMatrix) const;

	void generateCube(Shader *shader);
	void generatePlane(Shader *shader);


public:
	SQTTransform transform;
	std::vector<Vertex> m_Vertices;
	std::vector<Edge> m_Edges;
	std::vector<GLuint> m_Indices;
	Material m_Material;

protected:
	GLuint m_VAO;
	GLuint m_VBO;//!< single VAO initialised with subbuffer data
	GLuint m_EBO; //!< index buffer
	float m_diagonalLength;
};


