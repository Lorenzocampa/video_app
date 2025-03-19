#include <GLFW/glfw3.h>
#include <stdio.h>

// Funzione di errore callback
void error_callback(int error, const char* description)
{
	fprintf(stderr, "GLFW Error (%d): %s\n", error, description);
}

bool load_frame(const char* filename, int* width, int* height, unsigned char** data);

int main()
{
	glfwSetErrorCallback(error_callback);

	if (!glfwInit())
	{
		printf("GLFW initialization failed!\n");
		return 1;
	}

	GLFWwindow* window = glfwCreateWindow(640, 480, "OpenGL Window", NULL, NULL);
	if (!window)
	{
		printf("GLFW window creation failed!\n");
		glfwTerminate();
		return 1;
	}

	glfwMakeContextCurrent(window);

	glfwSwapInterval(1);

	int frame_width, frame_height;
	unsigned char* frame_data;
	if (!load_frame("/mnt/c/Users/astor_dev_03/Desktop/video.mkv", &frame_width, &frame_height, &frame_data))
	{
		printf("could not load frame!\n");
		return 1;
	};

	GLuint tex_handle;
	glGenTextures(1, &tex_handle);
	glBindTexture(GL_TEXTURE_2D, tex_handle);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame_width, frame_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, frame_data);

	while (!glfwWindowShouldClose(window))
	{
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//*set up orphographic projection
		int window_width, window_height;
		glfwGetFramebufferSize(window, &window_width, &window_height);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, window_width, window_height, 0, -1, 1);
		glMatrixMode(GL_MODELVIEW);

		//*rendere whatever you want
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, tex_handle);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f(200, 200);
		glTexCoord2f(1.0f, 0.0f);
		glVertex2f(200 + frame_width, 200);
		glTexCoord2f(1.0f, 1.0f);
		glVertex2f(200 + frame_width, 200 + frame_height);
		glTexCoord2f(0.0f, 1.0f);
		glVertex2f(200, 200 + frame_height);
		glEnd();
		glDisable(GL_TEXTURE_2D);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	return 0;
}
