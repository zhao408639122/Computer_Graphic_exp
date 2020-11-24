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


int mouseCount, polygonFixed;
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

struct node {
	double x, y, dx;
	node(double _x = 0, double _y = 0, double _dx = 0) : x(_x), y(_y), dx(_dx) {}
};
bool cmp(const node& a, const node& b) { return a.x < b.x; }

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

class polygon {
private:
	std::vector<point> Vertices, Draft;
	std::vector<node> NET[SCR_HEIGHT], AET;
	std::vector<float> pointsInside, pointsOutside;
	point* ModifyingPoint;
public:
	void render(Shader& ourShader, unsigned int& VAO, unsigned& VBO) {
		pointsInside.clear();
		pointsOutside.clear();
		if (Vertices.size() < 2) return;
		for (auto i = Draft.begin(); i != Draft.end() && (i + 1) != Draft.end(); i += 2) {
			i->push(pointsInside);
			(i + 1)->push(pointsInside);
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
		polygonFixed = 0;
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

	void Fill() {
		Draft.clear();
		AET.clear();
		for (int i = 0; i < SCR_HEIGHT; ++i) NET[i].clear();

		int uY = SCR_HEIGHT, dY = 0;
		for (auto it : Vertices) {
			uY = std::min(uY, it.y);
			dY = std::max(dY, it.y);
		}
		for (auto i = Vertices.begin(); i != Vertices.end(); ++i) {
			auto k = i + 1;
			if (k == Vertices.end()) k = Vertices.begin();
			int tempX = 0;
			int maxY = std::max(i->y, k->y), minY = std::min(i->y, k->y);
			double tempdx = ((k->x - i->x) * 1.0) / ((k->y - i->y) * 1.0);
			if (i->y == minY) tempX = i->x;
			else tempX = k->x;
			NET[minY].emplace_back(tempX, maxY - 1, tempdx);
		}
		for (int i = uY; i <= dY; ++i) {
			if (!NET[i].empty()) { //add new edge
				for (auto it : NET[i]) AET.push_back(it);
			}
			//fill
			std::sort(AET.begin(), AET.end(), cmp);
			for (auto k = AET.begin(); k != AET.end() && (k + 1) != AET.end(); k += 2) {
				Draft.push_back(point(k->x, i));
				Draft.push_back(point((k + 1)->x, i));
			}
			//update
			for (auto it = AET.begin(); it != AET.end();)
				if (it->y == i) it = AET.erase(it);
				else it++;
			for (auto it = AET.begin(); it != AET.end(); ++it)
				it->x = it->x + it->dx;
		}
	}

	void printSize() {
		std::cout << "SIZE: " << Vertices.size() << std::endl;
	}
}Polygon;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	else if (key == GLFW_KEY_P && action == GLFW_PRESS) {
		Polygon.clear();
		mouseCount = 0;
	}
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		mouseCount ^= 1;
		double xpos, ypos;
		int pos[2];
		glfwGetCursorPos(window, &xpos, &ypos);
		pos[0] = (int)xpos, pos[1] = (int)ypos;
		if (mouseCount & 1) {
			if (polygonFixed && !Polygon.findPoint(point(pos[0], pos[1])))
				mouseCount ^= 1;
		}
		else if (polygonFixed) {
			Polygon.clearModifyingPoint();
		}
		if (!polygonFixed) {
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
	Polygon.updateModifyingPoint(point(pos[0], pos[1]));
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

	Polygon.clear();
	while (!glfwWindowShouldClose(window)) {

		glClearColor(0.05f, 0.0f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		processInput(window);
		Polygon.Fill();
		Polygon.render(ourShader, VAO, VBO);
		//Polygon.printSize();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	glfwTerminate();
	return 0;
}