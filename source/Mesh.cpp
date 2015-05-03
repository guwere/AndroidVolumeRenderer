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

Vertex::Vertex(float vx, float vy, float vz, float ts, float tt)
	:m_Position(vx, vy, vz), m_TexCoord(ts, tt)
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
void Mesh::renderWireframe() const
{
	render(GL_LINE_STRIP,true);
}

std::vector<Vertex> Mesh::getTransformedVertices(const glm::mat4 &transformMatrix) const
{
	glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(transformMatrix)));
	std::vector<Vertex> result(m_Vertices.size());
	for(unsigned int i = 0; i < m_Vertices.size(); ++i)
	{
		const Vertex &v = m_Vertices[i];
		result[i].m_Position = glm::vec3(transformMatrix * glm::vec4(m_Vertices[i].m_Position, 1.0f));
		result[i].m_TexCoord = m_Vertices[i].m_TexCoord;
		result[i].m_Normal = normalMatrix * m_Vertices[i].m_Normal;
	}
	return result;
}

std::vector<glm::vec3> Mesh::getTransformedPositions(const glm::mat4 &transformMatrix) const
{
	std::vector<glm::vec3> result(m_Vertices.size());
	for(unsigned int i = 0; i < m_Vertices.size(); ++i)
	{
		const Vertex &v = m_Vertices[i];
		result[i] = glm::vec3(transformMatrix * glm::vec4(m_Vertices[i].m_Position, 1.0f));
	}
	return result;
}

std::vector<Edge> Mesh::getTransformedEdges(const glm::mat4 &transformMatrix) const
{
	std::vector<Edge> result(m_Edges.size());
	for(unsigned int i = 0; i < m_Edges.size(); ++i)
	{
		const Edge &v = m_Edges[i];
		result[i].p1 = glm::vec3(transformMatrix * glm::vec4(m_Edges[i].p1, 1.0f));
		result[i].p2 = glm::vec3(transformMatrix * glm::vec4(m_Edges[i].p2, 1.0f));
	}
	return result;
}

void Mesh::generateCube(Shader *shader)
{
	// Front face
	//m_Vertices.push_back(Vertex(-1,-1,1, 0,0 ));
	//m_Vertices.push_back(Vertex( 1,-1,1, 1,0 ));
 //   m_Vertices.push_back(Vertex(-1,1,1, 0,1  ));
 //   m_Vertices.push_back(Vertex( 1,1,1, 1,1  ));
	////m_Vertices.push_back(Vertex(// Right face ));
	//m_Vertices.push_back(Vertex(1,-1,1, 0,0  ));
	//m_Vertices.push_back(Vertex(1,-1,-1, 1,0 ));
	//m_Vertices.push_back(Vertex(1,1,1,  0,1  ));
	//m_Vertices.push_back(Vertex(1,1,-1, 1,1  ));
	////m_Vertices.push_back(Vertex(// Back face  ));
	//m_Vertices.push_back(Vertex(1,-1,-1, 0,0 ));
	//m_Vertices.push_back(Vertex(-1,-1,-1, 1,0)); 
	//m_Vertices.push_back(Vertex(1,1,-1,  0,1 ));
	//m_Vertices.push_back(Vertex(-1,1,-1, 1,1 ));
	////m_Vertices.push_back(Vertex(// Left face  ));
	//m_Vertices.push_back(Vertex(-1,-1,-1, 0,0));
	//m_Vertices.push_back(Vertex(-1,-1,1, 1,0 ));
	//m_Vertices.push_back(Vertex(-1,1,-1, 0,1 ));
	//m_Vertices.push_back(Vertex(-1,1,1,1,1  ));
	////m_Vertices.push_back(Vertex(// Bottom face));
	//m_Vertices.push_back(Vertex(-1,-1,-1, 0,0));
	//m_Vertices.push_back(Vertex(1,-1,-1, 1,0));
	//m_Vertices.push_back(Vertex(-1,-1,1, 0,1));
	//m_Vertices.push_back(Vertex(1,-1,1,1,1));
	////m_Vertices.push_back(Vertex(// Top face	  ));
	//m_Vertices.push_back(Vertex(-1,1,1, 0,0  ));
	//m_Vertices.push_back(Vertex(1,1,1, 1,0	  ));
	//m_Vertices.push_back(Vertex(-1,1,-1, 0,1 ));
	//m_Vertices.push_back(Vertex(1,1,-1,1,1  ));


	m_Vertices.push_back(Vertex(glm::vec3(MIN_MESH_X, MIN_MESH_Y, MAX_MESH_Z)));
	m_Vertices.push_back(Vertex(glm::vec3(MAX_MESH_X, MIN_MESH_Y, MAX_MESH_Z)));
	m_Vertices.push_back(Vertex(glm::vec3(MAX_MESH_X, MAX_MESH_Y, MAX_MESH_Z)));
	m_Vertices.push_back(Vertex(glm::vec3(MIN_MESH_X, MAX_MESH_Y, MAX_MESH_Z)));

	m_Vertices.push_back(Vertex(glm::vec3(MIN_MESH_X, MIN_MESH_Y, MIN_MESH_Z)));
	m_Vertices.push_back(Vertex(glm::vec3(MAX_MESH_X, MIN_MESH_Y, MIN_MESH_Z)));
	m_Vertices.push_back(Vertex(glm::vec3(MAX_MESH_X, MAX_MESH_Y, MIN_MESH_Z)));
	m_Vertices.push_back(Vertex(glm::vec3(MIN_MESH_X, MAX_MESH_Y, MIN_MESH_Z)));

	m_diagonalLength = glm::distance(glm::vec3(MIN_MESH_X, MIN_MESH_Y, MIN_MESH_Z), glm::vec3(MAX_MESH_X,  MAX_MESH_Y,  MAX_MESH_Z));

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

	m_Edges.resize(12);
	m_Edges[0] = Edge(m_Vertices[0].m_Position,m_Vertices[1].m_Position);
	m_Edges[1] = Edge(m_Vertices[1].m_Position,m_Vertices[2].m_Position);
	m_Edges[2] = Edge(m_Vertices[2].m_Position,m_Vertices[3].m_Position);
	m_Edges[3] = Edge(m_Vertices[3].m_Position,m_Vertices[0].m_Position);

	m_Edges[4] = Edge(m_Vertices[0].m_Position,m_Vertices[4].m_Position);
	m_Edges[5] = Edge(m_Vertices[1].m_Position,m_Vertices[5].m_Position);
	m_Edges[6] = Edge(m_Vertices[4].m_Position,m_Vertices[5].m_Position);
	m_Edges[7] = Edge(m_Vertices[4].m_Position,m_Vertices[7].m_Position);

	m_Edges[8] = Edge(m_Vertices[5].m_Position,m_Vertices[6].m_Position);
	m_Edges[9] = Edge(m_Vertices[6].m_Position,m_Vertices[7].m_Position);
	m_Edges[10] = Edge(m_Vertices[3].m_Position,m_Vertices[7].m_Position);
	m_Edges[11] = Edge(m_Vertices[2].m_Position,m_Vertices[6].m_Position);


	//the size of the dimensions of the pushed cube is 2; normalized between [0,1] range to match tex coords
	//for(int i = 0; i < m_Vertices.size(); ++i)
	//{
	//	const Vertex &v = m_Vertices[i];;
	//	m_Vertices[i] = Vertex(
	//		(v.m_Position.x + 1.0f) * 0.5f,
	//		(v.m_Position.y + 1.0f) * 0.5f,
	//		(v.m_Position.z + 1.0f) * 0.5f,
	//		v.m_TexCoord.x, v.m_TexCoord.y);
	//}


	
	createVAO(shader);
}

void Mesh::generatePlane(Shader *shader)
{
	GLfloat cube_vertices[] = {
		0,0,0, 0,0,
		1,0,0, 1,0,
		1,1,0, 1,1,
		0,1,0, 0,1

	};
	m_Vertices.push_back(Vertex(glm::vec3(MIN_MESH_X, MIN_MESH_Y, 0)));
	m_Vertices.push_back(Vertex(glm::vec3(MAX_MESH_X, MIN_MESH_Y, 0)));
	m_Vertices.push_back(Vertex(glm::vec3(MAX_MESH_X, MAX_MESH_Y, 0)));
	m_Vertices.push_back(Vertex(glm::vec3(MIN_MESH_X, MAX_MESH_Y, 0)));

	GLushort cube_elements[] = {
		// front
		0, 1, 2,
		2, 3, 0

	};
	m_Indices = vector<GLuint>(cube_elements, cube_elements + sizeof(cube_elements) / sizeof(GLushort) );
	createVAO(shader);
}

Edge::Edge(const glm::vec3 &point1, const glm::vec3 &point2)
	:p1(point1), p2(point2)
{

}
