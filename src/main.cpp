#include <stdio.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "video_reader.hpp"
#include <iostream>
#include <chrono>
#include <vector>
#include <string>

#include "loggers.hpp"
#include "timer.hpp"

UNIQUE_PTR_LOGGER g_logger = MAKE_UNIQUE_LOGGER("main");

//* callback func
void error_callback(int error, const char* description)
{
	g_logger->error("GLFW Error {}: {}", error, description);
}

//* Shader sources
const char* vertex_shader_source = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

void main()
{
    gl_Position = vec4(aPos, 0.0, 1.0);
    // Flip the y-axis by subtracting from 1.0
    TexCoord = vec2(aTexCoord.x, 1.0 - aTexCoord.y);
}
)";

const char* fragment_shader_source = R"(
#version 330 core
out vec4 FragColor;
in vec2 TexCoord;
uniform sampler2D videoTexture;

void main()
{
    FragColor = texture(videoTexture, TexCoord);
}
)";

//* shader compiling
GLuint compile_shader(GLenum type, const char* source)
{
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);

	//*check errori
	int success;
	char infoLog[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		std::cerr << "ERROR: Shader Compilation Failed\n" << infoLog << std::endl;
	}
	return shader;
}

//* create shader program
GLuint create_shader_program()
{
	GLuint vertex_shader   = compile_shader(GL_VERTEX_SHADER, vertex_shader_source);
	GLuint fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_shader_source);

	GLuint shader_program = glCreateProgram();
	glAttachShader(shader_program, vertex_shader);
	glAttachShader(shader_program, fragment_shader);
	glLinkProgram(shader_program);

	//* check linking
	int program_success;
	char programLog[512];
	glGetProgramiv(shader_program, GL_LINK_STATUS, &program_success);
	if (!program_success)
	{
		glGetProgramInfoLog(shader_program, 512, NULL, programLog);
		std::cerr << "ERROR: Program Linking Failed\n" << programLog << std::endl;
	}
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	return shader_program;
}

//* buffer adn VAO setup
void setup_buffers(GLuint& VAO, GLuint& VBO, GLuint& EBO)
{
	//* data definitions
	const float vertices[]		 = {// Posizioni       // Coordinate Texture
								-1.0F, 1.0F, 0.0F, 1.0F, -1.0F, -1.0F, 0.0F, 0.0F, 1.0F, -1.0F, 1.0F, 0.0F, 1.0F, 1.0F, 1.0F, 1.0F};
	const unsigned int indices[] = {0, 1, 2, 2, 3, 0};

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	//* attribute of vertex
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}

int main()
{
	glfwSetErrorCallback(error_callback);
	if (glfwInit() == 0)
	{
		std::cerr << "GLFW initialization failed!" << std::endl;
		return 1;
	}

	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	GLFWwindow* window = glfwCreateWindow(1920, 1080, "OpenGL Window", NULL, NULL);
	if (window == nullptr)
	{
		std::cerr << "GLFW window creation failed!" << std::endl;
		glfwTerminate();
		return 1;
	}
	glfwMakeContextCurrent(window);
	glewInit();
	glfwSwapInterval(0);

	//* create shader program ad use it
	GLuint shader_program = create_shader_program();
	glUseProgram(shader_program);

	//* load video data
	std::vector<std::string> filenames = {"/mnt/c/Users/astor_dev_03/Desktop/output.mp4"};
	std::vector<VideoReaderState> vr_state(filenames.size());
	for (size_t i = 0; i < filenames.size(); ++i)
	{
		if (!video_reader_open(&vr_state[i], filenames[i].c_str()))
		{
			std::cerr << "Failed to open video: " << filenames[i] << std::endl;
		}
	}

	//* gen video texture
	std::vector<GLuint> tex_handle(filenames.size());
	glGenTextures(static_cast<GLsizei>(filenames.size()), tex_handle.data());
	for (size_t i = 0; i < tex_handle.size(); ++i)
	{
		glBindTexture(GL_TEXTURE_2D, tex_handle[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, vr_state[i].width, vr_state[i].height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}

	//* alloc buffer for each frame
	std::vector<uint8_t*> frame_data(filenames.size());
	for (size_t i = 0; i < filenames.size(); ++i)
	{
		frame_data[i] = new uint8_t[vr_state[i].width * vr_state[i].height * 4];
	}

	//* buffer setup for rendering
	GLuint VAO, VBO, EBO;
	setup_buffers(VAO, VBO, EBO);

	//* var for calc fps
	auto last_time	= std::chrono::high_resolution_clock::now();
	int frame_count = 0;

	while (!glfwWindowShouldClose(window))
	{
		auto frame_start = std::chrono::high_resolution_clock::now();
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		//* update frame video
		for (size_t i = 0; i < vr_state.size(); ++i)
		{
			if (video_reader_read_frame(&vr_state[i], frame_data[i]))
			{
				glBindTexture(GL_TEXTURE_2D, tex_handle[i]);
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, vr_state[i].width, vr_state[i].height, GL_RGBA, GL_UNSIGNED_BYTE, frame_data[i]);
			}
		}

		glUseProgram(shader_program);
		glBindTexture(GL_TEXTURE_2D, tex_handle[0]); // Utilizza il primo video
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		//* calc and print FPS
		frame_count++;
		auto current_time					  = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> elapsed = current_time - last_time;
		if (elapsed.count() >= 0.01) // u
		{
			double fps = frame_count / elapsed.count();
			std::cout << "FPS: " << fps << std::endl;
			frame_count = 0;
			last_time	= current_time;
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Pulizia delle risorse
	for (size_t i = 0; i < vr_state.size(); ++i)
	{
		video_reader_close(&vr_state[i]);
		delete[] frame_data[i];
	}
	glDeleteProgram(shader_program);
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glfwTerminate();
	return 0;
}
