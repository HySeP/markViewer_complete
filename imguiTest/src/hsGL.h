#ifndef HSGL_H
#define HSGL_H

#include <GL/gl3w.h>
#include <glm/glm.hpp>

#include <vector>

namespace SYE {
	class Shader;
	struct Vertex;
}

class glBase {
	private:
	protected:
		GLuint VAO, VBO;
		glm::mat4 mMatModel = glm::mat4(0.7f);	
		SYE::Shader *pShader;
		std::vector<SYE::Vertex> vertices;
		void setVAO();
	public:
		glBase() {};
		virtual ~glBase() {};

		virtual void render(glm::mat4 view, glm::mat4 proj) {};
};

class glGrid: public glBase {
	private:
	protected:
	public:
		glGrid(
				SYE::Shader *pShader
				, unsigned int rows = 100
				, unsigned int cols = 100
				, float unit = 0.1f
				, glm::vec3 color = glm::vec3(0.7f, 0.7f, 0.7f)
				);
		virtual ~glGrid();

		void render(glm::mat4 view, glm::mat4 proj);
};


enum glObjectType {
	objTriangle
	/*
	KObjRectangle,
	KObjCircle,
	KObEtc,
	*/
};

class HsGlGrid: public glBase {
	private:
	protected:
	public:
		HsGlGrid();
		virtual ~HsGlGrid();
};

class glObject: public glBase {
	private:
		int type;
	protected:
	public:
		glObject();
		virtual ~glObject();

		void initialize(
				SYE::Shader *pShader
				, glObjectType type
				, float unit = 0.1f
				, glm::vec3 color = glm::vec3(0.4f, 0.3f, 0.5f)
				);
		void render(glm::mat4 view, glm::mat4 proj);

};

#endif
