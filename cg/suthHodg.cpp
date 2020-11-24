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


int mouseCount, polygonControl, windowControl, windowInit, polygonFixed;
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
	return (sqr(xpos0 - xpos1) + sqr(ypos0 - ypos1) <= 25);
}
int judgeCursorPos(point a, point b) {
	int xpos0 = a.x, xpos1 = b.x, ypos0 = a.y, ypos1 = b.y;
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
		pt[2].set(rB, dB), pt[3].set(lB, dB);
	}

	void initSize() {
		changeSize(init[0].x, init[0].y, init[1].x, init[1].y);
	}

	void render(Shader& ourShader, unsigned int& VAO, unsigned& VBO) {
		squareRender.clear();
		for (int i = 0; i < 3; ++i) pt[i].push(squareRender);
		for (int i = 2; i < 5; ++i) pt[i % 4].push(squareRender);

		GLfloat* temp = squareRender.data();
		int size = squareRender.size();

		ourShader.setVec4("ourColor", 0.05f, 0.25f, 0.9f, 1.0f);
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

point pointIntersect(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4) {
	int num1 = (x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4);
	int num2 = (x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4);
	int den = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
	return point(num1 / den, num2 / den);
}

class polygon {
private:
	std::vector<point> Vertices;
	std::vector<point> Draft;
	std::vector<float> pointsInside, pointsOutside;
	point* ModifyingPoint;
public:
	void clip(point p1, point p2) {
		std::vector<point> Temp;
		int x1 = p1.x, x2 = p2.x, y1 = p1.y, y2 = p2.y;
		int size = Draft.size();
		for (auto i = Draft.begin(); i != Draft.end(); ++i) {
			auto k = i + 1;
			if (k == Draft.end()) k = Draft.begin();
			int i_pos = (x2 - x1) * (i->y - y1) - (y2 - y1) * (i->x - x1);
			int k_pos = (x2 - x1) * (k->y - y1) - (y2 - y1) * (k->x - x1);
			if (i_pos < 0 && k_pos < 0)
				Temp.push_back(point(k->x, k->y));
			else if (i_pos >= 0 && k_pos < 0) {
				Temp.push_back(pointIntersect(x1, y1, x2, y2, i->x, i->y, k->x, k->y));
				Temp.push_back(point(k->x, k->y));
			}
			else if (i_pos < 0 && k_pos >= 0) {
				Temp.push_back(pointIntersect(x1, y1, x2, y2, i->x, i->y, k->x, k->y));
			}
		}
		Draft.clear();
		Draft.assign(Temp.begin(), Temp.end());
	}
	void suthHodgClip() {
		if (Vertices.size() < 3) return;
		Draft.clear();
		Draft.assign(Vertices.begin(), Vertices.end());
		for (int i = 3; i >= 0; --i) {
			int k = (i + 1) % 4;
			clip(clipWindow.pt[k], clipWindow.pt[i]);
		}
	}
	void render(Shader& ourShader, unsigned int& VAO, unsigned& VBO) {
		pointsInside.clear();
		pointsOutside.clear();
		if (Vertices.size() < 2) return;
		for (auto i = Draft.begin(); i != Draft.end(); ++i) {
			auto j = i + 1;
			if (j == Draft.end()) j = Draft.begin();
			i->push(pointsInside);
			j->push(pointsInside);
		}
		for (auto i = Vertices.begin(); i != Vertices.end(); ++i) {
			auto j = i + 1;
			if (j == Vertices.end()) {
				j = Vertices.begin();
				if (!polygonFixed) break;
			}
			i->push(pointsOutside);
			j->push(pointsOutside);
		}
		//render line outside
		ourShader.setVec4("ourColor", 0.0f, 0.7f, 0.4f, 1.0f);
		ourShader.use();
		GLfloat* temp = pointsOutside.data();
		int size = pointsOutside.size();
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size * sizeof(GLfloat), temp);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(VAO);
		glDrawArrays(GL_LINES, 0, size / 3);
		//render line inside
		ourShader.setVec4("ourColor", 1.0f, 0.4f, 0.3f, 1.0f);
		ourShader.use();
		temp = pointsInside.data();
		size = pointsInside.size();
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size * sizeof(GLfloat), temp);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(VAO);
		glDrawArrays(GL_LINES, 0, size / 3);
	}
	void clear() {
		Vertices.clear();
		Draft.clear();
		ModifyingPoint = nullptr;
	}
	void clearModifyingPoint() {
		ModifyingPoint = nullptr;
	}
	int findPoint(point P) {
		for (auto i = Vertices.begin(); i != Vertices.end(); ++i) {
			if (judgeCursorPos(P, *i)) {
				ModifyingPoint = &(*i);
				return 1;
			}
		}
		return 0;
	}

	void pointInput(point P) {
		if (Vertices.size() > 2 && judgeCursorPos(P, Vertices[0])) {
			Vertices.pop_back();
			polygonFixed = 1;
			clearModifyingPoint();
			mouseCount = 0;
		}
		else {
			if (Vertices.size() == 0) {
				Vertices.push_back(P);
				Vertices.push_back(P);
				ModifyingPoint = &Vertices[Vertices.size() - 1];
			}
			else {
				Vertices.push_back(P);
				ModifyingPoint = &Vertices[Vertices.size() - 1];
			}
		}
	}

	void updateModifyingPoint(point P) {
		if (ModifyingPoint == nullptr) return;
		ModifyingPoint->set(P.x, P.y);
	}
}Polygon;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key == GLFW_KEY_S && action == GLFW_PRESS) {
		windowControl = 1, polygonControl = 0;
		mouseCount = 0;
	}
	else if (key == GLFW_KEY_P && action == GLFW_PRESS) {
		polygonControl = 1, windowControl = 0;
		Polygon.clear();
		mouseCount = 0, polygonFixed = 0;
	}
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && (polygonControl || windowControl)) {
		mouseCount ^= 1;
		std::cout << "mouseControl: " << mouseCount << ", windowControl: " << windowControl << ", polygonControl: "
			<< polygonControl << ", windowInited: " << windowInit << std::endl;
		double xpos, ypos;
		int pos[2];
		glfwGetCursorPos(window, &xpos, &ypos);
		pos[0] = (int)xpos, pos[1] = (int)ypos;
		if (mouseCount & 1) {
			if (windowControl) {
				clipWindow.init[0].set(pos[0], pos[1]);
				clipWindow.init[1].set(pos[0], pos[1]);
				windowInit = 1;
			}
			else if (polygonControl && polygonFixed) {
				if (!Polygon.findPoint(point(pos[0], pos[1])))
					mouseCount ^= 1;
			}
		}
		else if (polygonControl && polygonFixed) {
			Polygon.clearModifyingPoint();
		}
		if (polygonControl && !polygonFixed) {
			mouseCount = 0;
			Polygon.pointInput(point(pos[0], pos[1]));
		}
		//Polygon.print();
	}
}

void processInput(GLFWwindow* window) {
	int pos[2];
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	pos[0] = (int)xpos, pos[1] = (int)ypos;
	if (windowControl && mouseCount & 1) {
		clipWindow.init[1].set(pos[0], pos[1]);
		clipWindow.initSize();

	}
	else if (polygonControl) {
		Polygon.updateModifyingPoint(point(pos[0], pos[1]));
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
		if (windowInit) {
			clipWindow.render(ourShader, VAO, VBO);
			Polygon.suthHodgClip();
		}
		Polygon.render(ourShader, VAO, VBO);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	glfwTerminate();
	return 0;
}