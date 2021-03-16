/** CSci-4611 Assignment 3:  Earthquake
 */

#include "earth.h"
#include "config.h"

#include <vector>

// for M_PI constant
#define _USE_MATH_DEFINES
#include <math.h>


Earth::Earth() {
}

Earth::~Earth() {
}

void Earth::Init(const std::vector<std::string> &search_path) {
    // init shader program
    shader_.Init();
    
    // init texture: you can change to a lower-res texture here if needed
    earth_tex_.InitFromFile(Platform::FindFile("earth-2k.png", search_path));


    // TODO: This is where you need to set the vertices and indiceds for earth_mesh_.
    
    std::vector<unsigned int> indices;
    std::vector<Point2> tex_coords;

    //set up plane mesh
    for(int i = 0; i <= nstacks; i++) {
      for(int j = 0; j <= nslices; j++){
          float u = (float)j/(nslices);
          float v= (float)i/(nstacks);
          vertices_plane.push_back(Point3(-1*M_PI + j*2*M_PI/nslices, 0.5*M_PI - i*M_PI/nstacks,0));
          normals_plane.push_back(Vector3(0,0,1));
          tex_coords.push_back(Point2(u,v));
      }
    }
    
    //set up the indices array
    for(int i = 0; i < nstacks; i++) {
      for(int j = 0; j < nslices; j++){
          indices.push_back(i*(nslices+1)+j);
          indices.push_back(i*(nslices+1)+j+(nslices+1));
          indices.push_back(i*(nslices+1)+j+(nslices+1)+1);
          
          indices.push_back(i*(nslices+1)+j);
          indices.push_back(i*(nslices+1)+j+(nslices+1)+1);
          indices.push_back(i*(nslices+1)+j+1);
      }
    }
    
    //sphere mesh
    double r = LatLongToSphere(90.0, 0.0)[1];  //radius for sphere
    //set up sphere mesh
    for(int i = 0; i <=nstacks; i++) {
         for(int j = 0; j <= nslices; j++) {
             float u = (float) j/nslices;
             float v = (float) i/nstacks;
            
            float rad = 2*r*sqrt(v*(1.0-v));
             float angle = -GfxMath::TWO_PI *u - M_PI/2;
             float x = rad*cos(angle);
             float z = rad*sin(angle);
            
             
            vertices_sphere.push_back(Point3(x,r-2*r*v , z));
            normals_sphere.push_back((Point3(x,r-2*r*v,z)-Point3(0, 0,0)).ToUnit());
             
        }
     }
    earth_mesh_.SetIndices(indices);
    earth_mesh_.SetTexCoords(0, tex_coords);
}

//function morph to take care of transfer of vertices
void Earth::morph(float a){
    std::vector<Point3> vertices;
    std::vector<Vector3> normals;
    for(int i = 0; i < (nstacks+1)*(nslices+1); i++) {
        Point3 p = Point3::Lerp(vertices_sphere[i], vertices_plane[i], a);
        Vector3 v = Vector3::Lerp(normals_sphere[i], normals_plane[i], a);
        vertices.push_back(p);
        normals.push_back(v.ToUnit());
    }
    earth_mesh_.SetVertices(vertices);
    earth_mesh_.SetNormals(normals);
    earth_mesh_.UpdateGPUMemory();
}

void Earth::Draw(const Matrix4 &model_matrix, const Matrix4 &view_matrix, const Matrix4 &proj_matrix) {
    // Define a really bright white light.  Lighting is a property of the "shader"
    DefaultShader::LightProperties light;
    light.position = Point3(10,10,10);
    light.ambient_intensity = Color(1,1,1);
    light.diffuse_intensity = Color(1,1,1);
    light.specular_intensity = Color(1,1,1);
    shader_.SetLight(0, light);

    // Adust the material properties, material is a property of the thing
    // (e.g., a mesh) that we draw with the shader.  The reflectance properties
    // affect the lighting.  The surface texture is the key for getting the
    // image of the earth to show up.
    DefaultShader::MaterialProperties mat;
    mat.ambient_reflectance = Color(0.5, 0.5, 0.5);
    mat.diffuse_reflectance = Color(0.75, 0.75, 0.75);
    mat.specular_reflectance = Color(0.75, 0.75, 0.75);
    mat.surface_texture = earth_tex_;

    // Draw the earth mesh using these settings
    if (earth_mesh_.num_triangles() > 0) {
        shader_.Draw(model_matrix, view_matrix, proj_matrix, &earth_mesh_, mat);
    }
}

//convert latitude and longitude to sphere(x,y,z)
Point3 Earth::LatLongToSphere(double latitude, double longitude) const {
    double lat = GfxMath::ToRadians(latitude);
    double lon = GfxMath::ToRadians(longitude);
    double x = cos(lat)*sin(lon);
    double y = sin(lat);
    double z = cos(lat)*cos(lon);
    return Point3(x,y,z);
}
Vector3 Earth::NormalToSphere(double latitude, double longitude) const {
    double lat = GfxMath::ToRadians(latitude);
    double lon = GfxMath::ToRadians(longitude);
    double x = cos(lat)*sin(lon);
    double y = sin(lat);
    double z = cos(lat)*cos(lon);
    return Vector3(x,y,z);
}

//convert latitude and longitude to plane(x,y,z)
Point3 Earth::LatLongToPlane(double latitude, double longitude) const {
    return Point3((longitude/180)*M_PI,(latitude/90)*M_PI/2,0);
}

void Earth::DrawDebugInfo(const Matrix4 &model_matrix, const Matrix4 &view_matrix, const Matrix4 &proj_matrix) {
    // This draws a cylinder for each line segment on each edge of each triangle in your mesh.
    // So it will be very slow if you have a large mesh, but it's quite useful when you are
    // debugging your mesh code, especially if you start with a small mesh.
    for (int t=0; t<earth_mesh_.num_triangles(); t++) {
        std::vector<unsigned int> indices = earth_mesh_.read_triangle_indices_data(t);
        std::vector<Point3> loop;
        loop.push_back(earth_mesh_.read_vertex_data(indices[0]));
        loop.push_back(earth_mesh_.read_vertex_data(indices[1]));
        loop.push_back(earth_mesh_.read_vertex_data(indices[2]));
        quick_shapes_.DrawLines(model_matrix, view_matrix, proj_matrix,
            Color(1,1,0), loop, QuickShapes::LinesType::LINE_LOOP, 0.005f);
    }
}


    
    
    
    
    
  


    
    





