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

  while (!glfwWindowShouldClose(window)) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawPixels(100, 100, GL_RGB, GL_UNSIGNED_BYTE, data);
    glfwSwapBuffers(window);

    glfwPollEvents();

    glfwSwapBuffers(window);
  }

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
