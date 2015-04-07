#include "Mesh.h"
#include "Shader.h"
#include <glm/gtc/type_ptr.hpp>

using std::vector;
using std::string;

Vertex::Vertex()
{
}

Vertex::Vertex(const glm::vec3 position)
	:m_Position(position)
{
}

void Vertex::operator=(const glm::vec3 &other)
{
	m_Position = other;
}

Mesh::Mesh(const std::vector<Vertex> &vertices,
	const std::vector<GLuint> &indices,
	Shader *shader)
	:m_Vertices(vertices), m_Indices(indices)
{

}

Mesh::Mesh()
{
}

Mesh::~Mesh()
{
	destroy();
}

void Mesh::destroy()
{
	glDeleteBuffers(1,&m_VBO);
	glDeleteBuffers(1,&m_EBO);
	glDeleteVertexArrays(1,&m_VAO);
}
//code adapted from http://learnopengl.com/#!Model-Loading/Mesh
void Mesh::createVAO(const Shader *shader)
{
	// Create buffers/arrays
	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);
	if(m_Indices.size())
		glGenBuffers(1, &m_EBO);

	glBindVertexArray(m_VAO);
	// Load data into vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(Vertex), &m_Vertices[0], GL_STATIC_DRAW);

	if(m_Indices.size())
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(GLuint), &m_Indices[0], GL_STATIC_DRAW);
	}

	// Set the vertex attribute pointers
	// Vertex Positions
	GLuint position = glGetAttribLocation(shader->m_Id,"position");
	glEnableVertexAttribArray(position);
	glVertexAttribPointer(position, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
	// Vertex Normals
	GLuint normal = glGetAttribLocation(shader->m_Id,"normal");
	glEnableVertexAttribArray(normal);
	glVertexAttribPointer(normal, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, m_Normal));
	// Vertex Texture Coords
	GLuint texCoord = glGetAttribLocation(shader->m_Id,"texCoord");
	glEnableVertexAttribArray(texCoord);
	glVertexAttribPointer(texCoord, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, m_TexCoord));

	glBindVertexArray(0);

}



void Mesh::render(GLenum mode, bool drawElements) const
{
	// Draw mesh
	glBindVertexArray(m_VAO);
	if(drawElements)
		glDrawElements(mode, m_Indices.size(), GL_UNSIGNED_INT, 0);
	else
		glDrawArrays(mode, 0, m_Vertices.size());
	glBindVertexArray(0);

}

void Mesh::render() const
{
	render(GL_TRIANGLES,true);
}


void Mesh::generateCube(Shader *shader)
{

	m_Vertices.push_back(Vertex(glm::vec3(-1.0, -1.0,  1.0)));
	m_Vertices.push_back(Vertex(glm::vec3(1.0, -1.0,  1.0)));
	m_Vertices.push_back(Vertex(glm::vec3(1.0,  1.0,  1.0)));
	m_Vertices.push_back(Vertex(glm::vec3(-1.0,  1.0,  1.0)));

	m_Vertices.push_back(Vertex(glm::vec3(-1.0, -1.0, -1.0)));
	m_Vertices.push_back(Vertex(glm::vec3(1.0, -1.0, -1.0)));
	m_Vertices.push_back(Vertex(glm::vec3(1.0,  1.0, -1.0)));
	m_Vertices.push_back(Vertex(glm::vec3(-1.0,  1.0, -1.0)));

	GLushort cube_elements[] = {
		// front
		0, 1, 2,
		2, 3, 0,
		// top
		1, 5, 6,
		6, 2, 1,
		// back
		7, 6, 5,
		5, 4, 7,
		// bottom
		4, 0, 3,
		3, 7, 4,
		// left
		4, 5, 1,
		1, 0, 4,
		// right
		3, 2, 6,
		6, 7, 3
	};

	m_Indices = vector<GLuint>(cube_elements, &cube_elements[0] + sizeof(cube_elements) / sizeof(GLushort) );
	createVAO(shader);
}

void Mesh::generatePlane(Shader *shader)
{
	m_Vertices.push_back(Vertex(glm::vec3(-1.0, -1.0,  1.0)));
	m_Vertices.push_back(Vertex(glm::vec3(1.0, -1.0,  1.0)));
	m_Vertices.push_back(Vertex(glm::vec3(1.0,  1.0,  1.0)));
	m_Vertices.push_back(Vertex(glm::vec3(-1.0,  1.0,  1.0)));
	GLushort cube_elements[] = {
		// front
		0, 1, 2,
		2, 3, 0

	};
	m_Indices = vector<GLuint>(cube_elements, cube_elements + sizeof(cube_elements) / sizeof(GLuint) );
	createVAO(shader);
}
