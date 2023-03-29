#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cstring>

int main (int argc, char **argv)
{
	if (!glfwInit()) {
		return 1;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow *win=glfwCreateWindow(640, 480, "TEST", NULL, NULL);
	if (!win) {
		return 1;
	}
	glfwMakeContextCurrent(win);

	if (!gladLoadGL(glfwGetProcAddress)) {
		return 1;
	}

	if (!GLAD_GL_VERSION_4_3) {
		return 1;
	}

	GLuint cubeTex;
	GLuint fbo;
    	int size = 256;
	unsigned char* white = (unsigned char*)malloc(size*size*4);
	unsigned char* buf = (unsigned char*)malloc(size*size*4);
	memset(white,255,size*size*4);
	printf("%s %s %s\n", glGetString(GL_VENDOR), glGetString(GL_RENDERER), glGetString(GL_VERSION));

	for (int variant=0; variant <5; variant++) {
		glGenTextures(1, &cubeTex);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTex);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		if (variant <3) {
			for (int f=0; f<6; f++) {
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + f, 0, GL_RGBA, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE, (variant==1)?white:NULL);
			}
		} else {
			glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_RGBA8, size,size);
		}
		if (variant == 2 || variant == 4) {
  			glClearTexImage(cubeTex,0,GL_RGBA,GL_UNSIGNED_BYTE,white);
		}

		glBindTexture(GL_TEXTURE_CUBE_MAP,0);

		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, cubeTex, 0);
		glClearColor(1,0,0,1);
		glClear(GL_COLOR_BUFFER_BIT); // clearing a layered framebuffer should clear ALL layers

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTex);
		for (int f=0; f<6; f++) {
			glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + f, 0, GL_RGBA, GL_UNSIGNED_BYTE, buf);
			unsigned char *ptr = buf + ((size/2) * size + size/2)*4;
			printf("variant%d: FACE%d: 0x%02x 0x%02x 0x%02x 0x%02x OK: %d\n",
			       variant, f,ptr[0],ptr[1],ptr[2],ptr[3],
			       (ptr[0] > 250 && ptr[1] < 5 && ptr[2] < 5 && ptr[3] > 250));
		}
		printf("Err: 0x%x\n", glGetError());
		///glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		glClearColor(0,0,0,1);
		glClear(GL_COLOR_BUFFER_BIT);
		glfwSwapBuffers(win);

		glDeleteTextures(1,&cubeTex);
		glDeleteFramebuffers(1,&fbo);
	}

	free(white);
	free(buf);
	return 0;
}

