#include <GLFW/glfw3.h>
#include <stdio.h>

// Funzione di errore callback
void error_callback(int error, const char *description) {
  fprintf(stderr, "GLFW Error (%d): %s\n", error, description);
}

int main() {
  // Imposta la callback per gli errori di GLFW
  glfwSetErrorCallback(error_callback);

  // Inizializzazione di GLFW
  if (!glfwInit()) {
    printf("GLFW initialization failed!\n");
    return 1;
  }

  GLFWwindow *window = glfwCreateWindow(640, 480, "OpenGL Window", NULL, NULL);
  if (!window) {
    printf("GLFW window creation failed!\n");
    glfwTerminate();
    return 1;
  }

  glfwMakeContextCurrent(window);

  glfwSwapInterval(1);

  unsigned char *data = new unsigned char[100 * 100 * 3];
  for (int y = 0; y < 100; ++y) {
    for (int x = 0; x < 100; ++x) {
      data[y * 100 * 3 + x * 3] = 0xff;
      data[y * 100 * 3 + x * 3 + 1] = 0x00;
      data[y * 100 * 3 + x * 3 + 2] = 0x00;
    }
  }

  for (int y = 25; y < 75; ++y) {
    for (int x = 25; x < 75; ++x) {
      data[y * 100 * 3 + x * 3] = 0x00;
      data[y * 100 * 3 + x * 3 + 1] = 0x00;
      data[y * 100 * 3 + x * 3 + 2] = 0xff;
    }
  }

  GLuint tex_handle;
  glGenTextures(1, &tex_handle);
  glBindTexture(GL_TEXTURE_2D, tex_handle);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 100, 100, 0, GL_RGB, GL_UNSIGNED_BYTE,
               data);

  while (!glfwWindowShouldClose(window)) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //*set up orphographic projection
    int window_width, window_height;
    glfwGetFramebufferSize(window, &window_width, &window_height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, window_width, 0, window_height, -1, 1);
    glMatrixMode(GL_MODELVIEW);

    //*rendere whatever you want
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex_handle);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(100, 100);
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(window_width - 100, 100);
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(window_width - 100, window_height - 100);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(100, window_height - 100);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  return 0;
}
