#include <igl/readOFF.h>
#include <igl/readOBJ.h>
#include <igl/opengl/glfw/Viewer.h>
#include <igl/png/readPNG.h>
#include <igl/unproject_onto_mesh.h>
#include <stdafx.h>
#include <geodesic_mesh.h>
#include <geodesic_algorithm_exact.h>
#include <geodesic_constants_and_simple_functions.h>

using namespace std;

int main() {
    vector<double> points;//顶点
    vector<unsigned> faces;//面
    vector<int> realIndex;
    Eigen::MatrixXd V;
    Eigen::MatrixXi F;
    int originalVertNum = 0;
    char file_name[255] = { "./kitten_simplified.obj\0" };

    // VTP Load Mesh
    bool success = geodesic::read_mesh_from_file(file_name, points, faces, realIndex, originalVertNum);
    if (!success) {
        cout << "something is wrong with the input file" << endl;
        return 0;
    }
    cout << "Load Mesh Success..." << endl;
    //Load obj
    igl::readOBJ(file_name, V, F);
    igl::opengl::glfw::Viewer viewer;
    Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic> R, G, B, A;
    igl::png::readPNG("./MyColorBar2.png", R, G, B, A);

    const auto update_distance = [&](const int vid) {
        //VTP Build Mesh
        geodesic::Mesh mesh;
        mesh.initialize_mesh_data(points, faces);//创建包括边的内部网络数据结构，初始化作用
        cout << "Build Mesh Success..." << endl;
        geodesic::GeodesicAlgorithmExact algorithm(&mesh);
        cout << "Computing geodesic distance to vertex " << vid << "..." << endl;
        // Propagation
        algorithm.propagate(vid);//从vid开始传播
        // Print Statistics
        cout << endl;
        algorithm.print_statistics();
        //------------------------VTP_Algorithm_End-------------------------------------------------
        Eigen::VectorXd d;
        d.resize(mesh.vertices().size(), 1);
        for (unsigned i = 0; i < mesh.vertices().size(); ++i)
            d[i] = mesh.vertices()[i].geodesic_distance();
        viewer.data().show_texture = true;
        viewer.data().set_texture(R, G, B, A);
        viewer.data().set_data(d);//在颜色图中查找颜色，并进行片段着色器的线性插值着色
    };
    viewer.callback_mouse_down = [&](igl::opengl::glfw::Viewer& viewer, int, int)->bool {
        int fid;
        Eigen::Vector3f bc;
        // Cast a ray in the view direction starting from the mouse position
        double x = viewer.current_mouse_x;
        double y = viewer.core().viewport(3) - viewer.current_mouse_y;
        if (igl::unproject_onto_mesh(Eigen::Vector2f(x, y), viewer.core().view, viewer.core().proj, viewer.core().viewport, V, F, fid, bc)) {
            int max;
            bc.maxCoeff(&max);
            int vid = F(fid, max);
            update_distance(vid);
            return true;
        }
        return false;
    };
    cout << "Click on mesh to define new source.\n" << endl;
    viewer.data().set_mesh(V, F);
    //viewer.data().show_texture = true;
    viewer.data().show_lines = false;
    viewer.launch();
    return 0;
}