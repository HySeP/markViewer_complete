#include "hsGL.h"

#include "hsAssimp.h"

void glBase::setVAO() {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(SYE::Vertex), &vertices[0], GL_STATIC_DRAW);

		// vertex positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SYE::Vertex), (void*)0);
		// vertex colors
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(SYE::Vertex), (void*)offsetof(SYE::Vertex, Color));

	glBindVertexArray(0);
}


glGrid::glGrid( SYE::Shader *pShader, unsigned int rows, unsigned int cols, float unit, glm::vec3 color) {
	this->pShader = pShader;

	glm::vec3	stPos;
	stPos.x = (-float(cols)/2)*unit;
	stPos.y = (-float(rows)/2)*unit;
	stPos.z = 0;

	SYE::Vertex vertex;
	for(unsigned int i=0; i <= cols; i++) {
		vertex.Position.x = stPos.x + (i*unit);
		vertex.Position.y = stPos.y;
		vertex.Position.z = 0;
		vertex.Color = color;
		vertices.push_back(vertex);

		vertex.Position.x = stPos.x + (i*unit);
		vertex.Position.y = -stPos.y;
		vertex.Position.z = 0;
		vertex.Color = color;
		vertices.push_back(vertex);
	}

	for(unsigned int i=0; i <= rows; i++) {

		vertex.Position.x = stPos.x;
		vertex.Position.y = stPos.y + (i*unit);
		vertex.Position.z = 0;
		vertex.Color = color;
		vertices.push_back(vertex);

		vertex.Position.x = -stPos.x;
		vertex.Position.y = stPos.y + (i*unit);
		vertex.Position.z = 0;
		vertex.Color = color;
		vertices.push_back(vertex);
	}

	setVAO();
}

glGrid::~glGrid() {
}

void glGrid::render(glm::mat4 view, glm::mat4 proj) {
	if(pShader!=nullptr) {
		pShader->use();
		pShader->setMat4("view", view);
		pShader->setMat4("projection", proj);
		pShader->setMat4("model", mMatModel);

		glBindVertexArray(VAO);
		glDrawArrays(GL_LINES, 0, vertices.size());
	}
}


glObject::glObject() {
}

glObject::~glObject() {
}

void glObject::initialize(SYE::Shader *pShader, glObjectType type, float unit, glm::vec3 color) {
		this->pShader = pShader;
		this->type = type;

		SYE::Vertex vertex;
		switch(type){
			case objTriangle :
				vertex.Position.x = 0;
				vertex.Position.y = 2*unit;
				vertex.Position.z = 0;
				vertex.Color = color;
				vertices.push_back(vertex);

				vertex.Position.x = 0;
				vertex.Position.y = 0;
				vertex.Position.z = 2*unit;
				vertex.Color = color;
				vertices.push_back(vertex);

				vertex.Position.x = 2*unit;
				vertex.Position.y = 0;
				vertex.Position.z = 0;
				vertex.Color = color;
				vertices.push_back(vertex);
				break;
/*
			case ObjRectangle :
				vertex.Position.x = -2*unit;
				vertex.Position.y = -2*unit;
				vertex.Position.z = 0;
				vertex.Color = color;
				vertices.push_back(vertex);

				vertex.Position.x = 2*unit;
				vertex.Position.y = 2*unit;
				vertex.Position.z = 0*unit;
				vertex.Color = color;
				vertices.push_back(vertex);

				vertex.Position.x = 2*unit;
				vertex.Position.y = 2*unit;
				vertex.Position.z = 4*unit;
				vertex.Color = color;
				vertices.push_back(vertex);

				vertex.Position.x = -2*unit;
				vertex.Position.y = -2*unit;
				vertex.Position.z = 4*unit;
				vertex.Color = color;
				vertices.push_back(vertex);
				break;

			case ObjCircle :
			case ObEtc :
*/
			default :
				break;
		}

		setVAO();
}

void glObject::render(glm::mat4 view, glm::mat4 proj) {
	if(pShader!=nullptr) {
		pShader->use();
		pShader->setMat4("view", view);
		pShader->setMat4("projection", proj);
		pShader->setMat4("model", mMatModel);

		glBindVertexArray(VAO);

		/*
		switch(this->type){
			case objTriangle :
			
			
			case objRectangle :
				// GL_TRIANGLES
				// GL_TRIANGLE_STRIP 
				// GL_TRIANGLE_FAN
				glDrawArrays(GL_TRIANGLE_FAN, 0, vertices.size());
				break;

			case objCircle :
			case obEtc :
				glDrawArrays(GL_LINES, 0, vertices.size());
				break;
			

			default :
				break;
			*/
		}
}

