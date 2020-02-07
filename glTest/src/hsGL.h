#ifndef HSGL_H
#define HSGL_H

#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>

#include <GLFW/glfw3.h>
//GLFWwindow* window;

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <vector>
#include <iostream>
#include <ctime>

class hsGL {
	private:
	protected:
	public:
		hsGL();
		virtual ~hsGL();
		bool CleanVBO(unsigned int vertexbuffer, unsigned int VertexArrayID, unsigned int programID);
		bool makeTri(unsigned int vertexbuffer);
		bool makeGL(void);
};


#endif
