#include<glad/glad.h>
#include<GLFW\glfw3.h>
#include<iostream>
#include<vector>
#include<algorithm>
#include<assert.h>
#include"shader.h"
#define sqr(x) (x)*(x)
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

const char* vertexShaderSource = "D:/work/CG/cg/cg/shader.vs";
const char* fragmentShaderSource = "D:/work/CG/cg/cg/shader.fs";


int mouseCount, lineControl, windowControl, windowInit;
float vertices[100010];

float transX(int x) {
	return (float)((float)(2 * x) - SCR_WIDTH) / (SCR_WIDTH * 1.0f);
}

float transY(int x) {
	return (float)(SCR_HEIGHT - (float)(2 * x)) / (SCR_HEIGHT * 1.0f);
}


struct point {
	int x, y;
	point(int _x = 0, int _y = 0) : x(_x), y(_y) {}
	void set(int _x, int _y) {
		x = _x, y = _y;
	}
	void push(std::vector<float>& vec) {
		vec.push_back(transX(x));
		vec.push_back(transY(y));
		vec.push_back(0.0);
	}
};

inline int dis(int xpos0, int ypos0, int xpos1, int ypos1) {
	return sqr(xpos0 - xpos1) + sqr(ypos0 - ypos1);
}

int judgeCursorPos(int xpos0, int ypos0, int xpos1, int ypos1) {
	//std::cout << "sqrx: " << sqr(xpos0 - xpos1) << ", sqry: " << sqr(ypos0 - ypos1) << std::endl;
	return (sqr(xpos0 - xpos1) + sqr(ypos0 - ypos1) <= 25);
}
int judgeCursorPos(point a, point b) {
	int xpos0 = a.x, xpos1 = b.x, ypos0 = a.y, ypos1 = b.y;
	//std::cout << "sqrx: " << sqr(xpos0 - xpos1) << ", sqry: " << sqr(ypos0 - ypos1) << std::endl;
	return (sqr(xpos0 - xpos1) + sqr(ypos0 - ypos1) <= 25);
}

class square {
private:
	std::vector<float> squareRender;
public:
	int  lB, rB, dB, uB;
	point pt[4];
	point init[2];
	void changeSize(int x1, int y1, int x2, int y2) {
		lB = std::min(x1, x2);
		rB = std::max(x1, x2);
		uB = std::min(y1, y2);
		dB = std::max(y1, y2);
		pt[0].set(lB, uB), pt[1].set(rB, uB);
		pt[2].set(lB, dB), pt[3].set(rB, dB);
	}

	void initSize() {
		changeSize(init[0].x, init[0].y, init[1].x, init[1].y);
	}

	void render(Shader& ourShader, unsigned int& VAO, unsigned& VBO) {
		squareRender.clear();
		for (int i = 0; i < 3; ++i) pt[i].push(squareRender);
		for (int i = 1; i < 4; ++i) pt[i].push(squareRender);
		GLfloat* temp = squareRender.data();
		int size = squareRender.size();

		ourShader.setVec4("ourColor", 0.78f, 1.0f, 1.0f, 1.0f);
		ourShader.use();
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size * sizeof(GLfloat), temp);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, size / 3);
	}
	int inside(int x, int y) {
		return ((x >= lB) && (x <= rB) && (y >= uB) && (y <= dB));
	}
}clipWindow;

struct line {
	point pt[2];
	line(int x1 = 0, int y1 = 0, int x2 = 0, int y2 = 0) {
		pt[0].set(x1, y1);
		pt[1].set(x2, y2);
	}
	void render(std::vector<float>& Inside, std::vector<float>& Outside) {
		int x1 = pt[0].x, x2 = pt[1].x, y1 = pt[0].y, y2 = pt[1].y;
		int dx = x2 - x1, dy = y2 - y1;
		square* W = &clipWindow;
		if (clipWindow.inside(x1, y1) && clipWindow.inside(x2, y2)) {
			point(x1, y1).push(Inside);
			point(x2, y2).push(Inside);
			return;
		}
		int p[5], q[5];
		p[1] = -dx, q[1] = x1 - W->lB, p[2] = dx, q[2] = W->rB - x1;
		p[3] = -dy, q[3] = y1 - W->uB, p[4] = dy, q[4] = W->dB - y1;
		int converge = 1;
		float umax = 0, umin = 1;
		for (int i = 1; i <= 4; ++i) {
			if (p[i] == 0) {
				if (q[i] < 0) {
					converge = 0;
					break;
				}
			}
			else {
				if (p[i] < 0)
					umax = std::max(float(q[i]) / float(p[i]), umax);
				else
					umin = std::min(float(q[i]) / float(p[i]), umin);
			}
		}
		//std::cout << "umax: " << umax << ", umin: " << umin << std::endl;
		if (umax > umin) converge = 0;
		if (!converge) {
			point(x1, y1).push(Outside);
			point(x2, y2).push(Outside);
		}
		else {
			if (clipWindow.inside(x1, y1)) {
				point b1(x1 + umin * dx, y1 + umin * dy), b2(x1 + umax * dx, y1 + umax * dy);
				int dis1 = dis(b1.x, b1.y, x2, y2), dis2 = dis(b2.x, b2.y, x2, y2);
				if (dis1 > dis2) std::swap(b1, b2);
				point(x1, y1).push(Inside);
				b1.push(Inside);
				b1.push(Outside);
				point(x2, y2).push(Outside);
				//std::cout << "111111111" << std::endl;
			}
			else if (clipWindow.inside(x2, y2)) {
				point b1(x1 + umin * dx, y1 + umin * dy), b2(x1 + umax * dx, y1 + umax * dy);
				int dis1 = dis(b1.x, b1.y, x1, y1), dis2 = dis(b2.x, b2.y, x1, y1);
				if (dis1 > dis2) std::swap(b1, b2);
				point(x1, y1).push(Outside);
				b1.push(Outside);
				b1.push(Inside);
				point(x2, y2).push(Inside);
				//std::cout << "111111111" << std::endl;
			}
			else {
				point b1(x1 + umin * dx, y1 + umin * dy), b2(x1 + umax * dx, y1 + umax * dy);
				int dis1 = dis(b1.x, b1.y, x1, y1), dis2 = dis(b2.x, b2.y, x1, y1);
				if (dis1 > dis2) std::swap(b1, b2);
				point(x1, y1).push(Outside);
				b1.push(Outside);
				b1.push(Inside);
				b2.push(Inside);
				b2.push(Outside);
				point(x2, y2).push(Outside);
			}
		}
	}
};

class lineController {
private:
	std::vector<line> f;
	std::vector<float> pointsInside, pointsOutside;
	point* modifyingPoint;
public:
	void push(int xpos, int ypos) {
		f.push_back(line(xpos, ypos, xpos, ypos));
		modifyingPoint = &f.back().pt[1];
	}
	void renderAll(Shader& ourShader, unsigned int& VAO, unsigned& VBO) {
		pointsInside.clear();
		pointsOutside.clear();
		for (auto i : f)
			i.render(pointsInside, pointsOutside);

		//render line inside
		ourShader.setVec4("ourColor", 1.0f, 0.4f, 0.3f, 1.0f);
		ourShader.use();
		GLfloat* temp = pointsInside.data();
		int size = pointsInside.size();
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size * sizeof(GLfloat), temp);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(VAO);
		glDrawArrays(GL_LINES, 0, size / 3);

		//render line outside
		ourShader.setVec4("ourColor", 0.0f, 0.7f, 0.4f, 1.0f);
		ourShader.use();
		temp = pointsOutside.data();
		size = pointsOutside.size();
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size * sizeof(GLfloat), temp);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(VAO);
		glDrawArrays(GL_LINES, 0, size / 3);
	}
	bool findPoint(point rhs) {
		for (auto i = f.begin(); i != f.end(); ++i) {
			if (judgeCursorPos(i->pt[0], rhs)) {
				modifyingPoint = &(i->pt[0]);
				return 1;
			}
			else if (judgeCursorPos(i->pt[1], rhs)) {
				modifyingPoint = &(i->pt[1]);
				return 1;
			}
		}
		return 0;
	}
	void clearTempPtr() {
		modifyingPoint = nullptr;
	}

	void updateTemp(int xpos, int ypos) {
		modifyingPoint->set(xpos, ypos);
	}

	//void print() {
	//	std::cout << "X: " << modifyingPoint->x << ", Y: " << modifyingPoint->y << std::endl;
	//}
}lineSet;
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key == GLFW_KEY_S && action == GLFW_PRESS) {
		windowControl = 1, lineControl = 0;
		lineSet.clearTempPtr();
		mouseCount = 0;
	}
	else if (key == GLFW_KEY_L && action == GLFW_PRESS) {
		lineControl = 1, windowControl = 0;
		mouseCount = 0;
	}
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && (lineControl || windowControl)) {
		mouseCount ^= 1;
		if (mouseCount & 1) {
			double xpos, ypos;
			int pos[2];
			glfwGetCursorPos(window, &xpos, &ypos);
			pos[0] = (int)xpos, pos[1] = (int)ypos;
			if (windowControl) {
				clipWindow.init[0].set(pos[0], pos[1]);
				clipWindow.init[1].set(pos[0], pos[1]);
				windowInit = 1;
			}
			else if (lineControl) {
				if (!lineSet.findPoint(point(pos[0], pos[1]))) {
					lineSet.push(pos[0], pos[1]);
				}
			}
		}
		else {
			if (lineControl) {
				lineSet.clearTempPtr();
			}
		}
	}

}

void processInput(GLFWwindow* window) {
	/*
	if ((!cnt) && (!conF)) { // when mouse moving
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		pos[2] = (int)xpos, pos[3] = (int)ypos;
		braseham(pos);
	}
	*/
	int pos[2];
	if (mouseCount & 1) {
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		pos[0] = (int)xpos, pos[1] = (int)ypos;
		if (windowControl) {
			clipWindow.init[1].set(pos[0], pos[1]);
			clipWindow.initSize();
			//std::cout << "X0: " << clipWindow.init[0].x << ", Y0: " << clipWindow.init[0].y << std::endl;
			//std::cout << "X1: " << clipWindow.init[1].x << ", Y1: " << clipWindow.init[1].y << std::endl;

		}
		else if (lineControl) {
			//std::cout << "X: " << pos[0] << ", Y: " << pos[1] << std::endl;
			//lineSet.print();
			lineSet.updateTemp(pos[0], pos[1]);
		}
	}
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

	while (!glfwWindowShouldClose(window)) {

		glClearColor(0.05f, 0.0f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		processInput(window);
		if (windowInit) clipWindow.render(ourShader, VAO, VBO);
		lineSet.renderAll(ourShader, VAO, VBO);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	glfwTerminate();
	//while (1);
	return 0;
}