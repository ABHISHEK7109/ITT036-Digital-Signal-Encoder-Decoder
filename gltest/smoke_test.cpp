// OpenGL smoke test: draw a pure green line, read it back with glReadPixels.
// Needs only GLFW (all GL calls used are OpenGL 1.1 -> no GLAD/GLEW required).
#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
#endif
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

int main() {
    if (!glfwInit()) { std::cerr << "glfwInit failed\n"; return 1; }
    GLFWwindow* win = glfwCreateWindow(400, 200, "ITT036 smoke test", nullptr, nullptr);
    if (!win) { std::cerr << "window creation failed\n"; glfwTerminate(); return 1; }
    glfwMakeContextCurrent(win);

    // Use the real framebuffer size (differs from window size on HiDPI screens)
    int fbW, fbH;
    glfwGetFramebufferSize(win, &fbW, &fbH);
    std::cout << "Framebuffer: " << fbW << " x " << fbH << "\n";

    glViewport(0, 0, fbW, fbH);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, fbW, 0, fbH, -1, 1);          // 1 unit = 1 pixel
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_LINE_SMOOTH);               // no anti-aliasing: exact colours
    glDisable(GL_BLEND);
    glClearColor(1, 1, 1, 1);                // white background
    glClear(GL_COLOR_BUFFER_BIT);
    
    glLineWidth(3.0f);
    glColor3f(0.0f, 1.0f, 0.0f);             // pure green
    glBegin(GL_LINES);
      glVertex2f(50.0f,  fbH / 2.0f);
      glVertex2f(350.0f, fbH / 2.0f);
    glEnd();
    glFinish();

    // Read the pixels back from the framebuffer we just drew into
    std::vector<unsigned char> px(fbW * fbH * 3);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadBuffer(GL_BACK);
    glReadPixels(0, 0, fbW, fbH, GL_RGB, GL_UNSIGNED_BYTE, px.data());

    auto at = [&](int x, int y) {            // y measured from bottom (OpenGL convention)
        const unsigned char* p = &px[(y * fbW + x) * 3];
        std::cout << "pixel(" << x << "," << y << ") = ("
                  << int(p[0]) << "," << int(p[1]) << "," << int(p[2]) << ")\n";
    };
    at(200, fbH / 2);      // on the line  -> expect (0,255,0)
    at(200, fbH / 2 + 20); // off the line -> expect (255,255,255)

    glfwSwapBuffers(win);
    glfwDestroyWindow(win);
    glfwTerminate();
    return 0;
}