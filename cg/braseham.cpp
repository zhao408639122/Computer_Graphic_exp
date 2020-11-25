#include<glad/glad.h>
#include<GLFW\glfw3.h>
#include<iostream>
#include<vector>
#include<assert.h>
#include"shader.h"
#define sqr(x) (x)*(x)
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

const char* vertexShaderSource = "./shader.vs";
const char* fragmentShaderSource = "./shader.fs";


int cnt, conF, inpress;
float vertices[100010];
int pos[4];
void braseham(int* f);

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
}

inline int judgeCursonPos(int xpos0, int ypos0, int xpos1, int ypos1) {
	std::cout << "sqrx: " << sqr(xpos0 - xpos1) << ", sqry: " << sqr(ypos0 - ypos1) << std::endl;
	return (sqr(xpos0 - xpos1) + sqr(ypos0 - ypos1) <= 25);
}
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		if (!conF) {
			if (cnt) {
				pos[0] = (int)xpos, pos[1] = (int)ypos;
				cnt ^= 1;
			}
			else {
				pos[2] = (int)xpos, pos[3] = (int)ypos;
				braseham(pos);
				cnt ^= 1;
				conF = 1;
			}
			std::cout << cnt << std::endl;
		}
		else {
			if (judgeCursonPos(pos[0], pos[1], (int)xpos, (int)ypos)) {
				pos[0] = (int)xpos, pos[1] = (int)ypos;
				inpress = 1;
			}
			else if (judgeCursonPos(pos[2], pos[3], (int)xpos, (int)ypos)) {
				pos[2] = (int)xpos, pos[3] = (int)ypos;
				inpress = 2;
			}
		}
	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
		inpress = 0;
}

std::vector<float> line;
void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
		cnt = 1, conF = 0;
		line.clear();
	}
	if ((!cnt) && (!conF)) { // when mouse moving
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		pos[2] = (int)xpos, pos[3] = (int)ypos;
		braseham(pos);
	}
	if (inpress) {
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		if (inpress == 1)
			pos[0] = (int)xpos, pos[1] = (int)ypos;
		else
			pos[2] = (int)xpos, pos[3] = (int)ypos;
		braseham(pos);
	}
}


float transX(int x) {
	return (float)((float)(2 * x) - SCR_WIDTH) / (SCR_WIDTH * 1.0f);
}

float transY(int x) {
	return (float)(SCR_HEIGHT - (float)(2 * x)) / (SCR_HEIGHT * 1.0f);
}

void line_insert(float x, float y) {
	line.push_back(transX(x));
	line.push_back(transY(y));
	line.push_back(1.0f);
}
void braseham(int* f) {
	line.clear();
	int x0 = f[0], y0 = f[1], x1 = f[2], y1 = f[3];
	if (x1 < x0) std::swap(x0, x1), std::swap(y0, y1);
	int dx = x1 - x0, dy = y1 - y0, x = x0, y = y0, e = -dx;
	std::cout << "dx: " << dx << ", dy: " << dy << std::endl;
	int flag = 0;
	if (dy < 0) {
		flag = 2;
		dy = -dy;
	}
	if (dx >= dy) flag += 1;
	else if (dx < dy) {
		flag += 2;
		e = -dy;
		std::swap(dx, dy), std::swap(x, y);
	}
	for (int i = 0; i <= dx; ++i) {
		if (flag == 1) line_insert(x, y);
		else if (flag == 2)	line_insert(y, x);
		else if (flag == 3) line_insert(x, 2 * y0 - y);
		//else if (flag == 4) line_insert(2 * y0 - y, x);
		else if (flag == 4) line_insert(y, 2 * y0 - x);
		x++; e += 2 * dy;
		if (e >= 0) {
			y++; e -= 2 * dx;
		}
	}
	std::cout << "dx: " << dx << ", dy: " << dy << ", flag: " << flag << std::endl;

}
int main() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	int gladErr = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress); //Starts up GLAD
	if (!gladErr) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	cnt = 1;
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); //感知窗口回调函数
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	Shader ourShader(vertexShaderSource, fragmentShaderSource);



	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	printf("%u\n%u", VAO, VBO);
	while (!glfwWindowShouldClose(window)) {

		processInput(window);

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		ourShader.setVec4("ourColor", 1.0f, 1.0f, 1.0f, 1.0f);
		ourShader.use();

		GLfloat* temp = line.data();
		int size = line.size();
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size * sizeof(GLfloat), temp);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindVertexArray(VAO);
		glDrawArrays(GL_POINTS, 0, size / 3);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	glfwTerminate();
	//while (1);
	return 0;
}

