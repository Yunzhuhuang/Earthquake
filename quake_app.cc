/** CSci-4611 Assignment 3:  Earthquake
 */

#include "quake_app.h"
#include "config.h"

#include <iostream>
#include <sstream>

// Number of seconds in 1 year (approx.)
const int PLAYBACK_WINDOW = 12 * 28 * 24 * 60 * 60;

using namespace std;


QuakeApp::QuakeApp() : GraphicsApp(1280,720, "Earthquake"),
    playback_scale_(15000000.0), debug_mode_(false)
{
    // Define a search path for finding data files (images and earthquake db)
    search_path_.push_back(".");
    search_path_.push_back("./data");
    search_path_.push_back(DATA_DIR_INSTALL);
    search_path_.push_back(DATA_DIR_BUILD);
    
    quake_db_ = EarthquakeDatabase(Platform::FindFile("earthquakes.txt", search_path_));
    current_time_ = quake_db_.earthquake(quake_db_.min_index()).date().ToSeconds();

 }


QuakeApp::~QuakeApp() {
}


void QuakeApp::InitNanoGUI() {
    // Setup the GUI window
    nanogui::Window *window = new nanogui::Window(screen(), "Earthquake Controls");
    window->setPosition(Eigen::Vector2i(10, 10));
    window->setSize(Eigen::Vector2i(400,200));
    window->setLayout(new nanogui::GroupLayout());
    
    date_label_ = new nanogui::Label(window, "Current Date: MM/DD/YYYY", "sans-bold");
    
    globe_btn_ = new nanogui::Button(window, "Globe");
    globe_btn_->setCallback(std::bind(&QuakeApp::OnGlobeBtnPressed, this));
    globe_btn_->setTooltip("Toggle between map and globe.");
    
    new nanogui::Label(window, "Playback Speed", "sans-bold");
    
    nanogui::Widget *panel = new nanogui::Widget(window);
    panel->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Horizontal,
                                            nanogui::Alignment::Middle, 0, 20));
    
    nanogui::Slider *slider = new nanogui::Slider(panel);
    slider->setValue(0.5f);
    slider->setFixedWidth(120);
    
    speed_box_ = new nanogui::TextBox(panel);
    speed_box_->setFixedSize(Eigen::Vector2i(60, 25));
    speed_box_->setValue("50");
    speed_box_->setUnits("%");
    slider->setCallback(std::bind(&QuakeApp::OnSliderUpdate, this, std::placeholders::_1));
    speed_box_->setFixedSize(Eigen::Vector2i(60,25));
    speed_box_->setFontSize(20);
    speed_box_->setAlignment(nanogui::TextBox::Alignment::Right);
    
    nanogui::Button* debug_btn = new nanogui::Button(window, "Toggle Debug Mode");
    debug_btn->setCallback(std::bind(&QuakeApp::OnDebugBtnPressed, this));
    debug_btn->setTooltip("Toggle displaying mesh triangles and normals (can be slow)");
    
    screen()->performLayout();
}

void QuakeApp::OnLeftMouseDrag(const Point2 &pos, const Vector2 &delta) {
    // Optional: In our demo, we adjust the tilt of the globe here when the
    // mouse is dragged up/down on the screen.
}


void QuakeApp::OnGlobeBtnPressed() {
    // TODO: This is where you can switch between flat earth mode and globe mode
    //increase keypressed once we press the Globe
    keypressed++;
}

void QuakeApp::OnDebugBtnPressed() {
    debug_mode_ = !debug_mode_;
}

void QuakeApp::OnSliderUpdate(float value) {
    speed_box_->setValue(std::to_string((int) (value * 100)));
    playback_scale_ = 30000000.0*value;
}


void QuakeApp::UpdateSimulation(double dt)  {
    // Advance the current time and loop back to the start if time is past the last earthquake
    current_time_ += playback_scale_ * dt;
    if (current_time_ > quake_db_.earthquake(quake_db_.max_index()).date().ToSeconds()) {
        current_time_ = quake_db_.earthquake(quake_db_.min_index()).date().ToSeconds();
    }
    if (current_time_ < quake_db_.earthquake(quake_db_.min_index()).date().ToSeconds()) {
        current_time_ = quake_db_.earthquake(quake_db_.max_index()).date().ToSeconds();
    }
    
    Date d(current_time_);
    stringstream s;
    s << "Current date: " << d.month()
        << "/" << d.day()
        << "/" << d.year();
    date_label_->setCaption(s.str());
    
    // TODO: Any animation, morphing, rotation of the earth, or other things that should
    // be updated once each frame would go here
    
    //update the earth rotation degree to make it spin
    degree+=45*dt;
    
    //the interval
    float x = 0.0;
    float y = 1.0;
   
    //from plane to sphere
    if(keypressed != 0 && keypressed%2 != 0) {
        float temp = Clamp(a-0.002, x, y);
        if(temp == x) {
            earth_.morph(0.0);
            a = 0.0;
        }
        else {
            a -= 0.002;
            earth_.morph(a);
        }
    }
    //from sphere to plane
    else if(keypressed != 0 && keypressed%2 == 0) {
        float temp = Clamp(a+0.002, x, y);
        if(temp == y) {
            earth_.morph(1.0);
            a = 1.0;
        }
        else {
            a += 0.002;
            earth_.morph(a);
        }
    }
    //initial position
    else {
        earth_.morph(1.0);
        a = 1.0;
    }
    
}


void QuakeApp::InitOpenGL() {
    // Set up the camera in a good position to see the entire earth in either mode
    proj_matrix_ = Matrix4::Perspective(60.0f, aspect_ratio(), 0.1f, 50.0f);
    view_matrix_ = Matrix4::LookAt(Point3(0,0,3.5), Point3(0,0,0), Vector3(0,1,0));
    glClearColor(0.0, 0.0, 0.0, 1);
    
    // Initialize the earth object
    earth_.Init(search_path_);

    // Initialize the texture used for the background image
    stars_tex_.InitFromFile(Platform::FindFile("iss006e40544.png", search_path_));
}


void QuakeApp::DrawUsingOpenGL() {
    quick_shapes_.DrawFullscreenTexture(Color(1,1,1), stars_tex_);
    
    // You can leave this as the identity matrix and we will have a fine view of
    // the earth.  If you want to add any rotation or other animation of the
    // earth, the model_matrix is where you would apply that.
    Matrix4 model_matrix;
    //the roration matrix
    Matrix4 R = Matrix4::RotationY(GfxMath::ToRadians(degree));
    
    //initial plane mesh when we don't press Global
    if(keypressed == 0) {
        //the earthquake on the plane
        plane_earthquake();
        earth_.Draw(model_matrix, view_matrix_, proj_matrix_);
    }
    else{
        //from sphere to plane
        if(keypressed %2 == 0) {
            earth_.Draw(model_matrix, view_matrix_, proj_matrix_);
            if(a == 1.0) {
              plane_earthquake();
            }
        }
        //from plane to sphere
        else {
            if(a == 0.0) {
                model_matrix = R;
             sphere_earthquake(R);
            }
            earth_.Draw(model_matrix, view_matrix_, proj_matrix_);
        }
    }
    
    if (debug_mode_) {
         earth_.DrawDebugInfo(model_matrix, view_matrix_, proj_matrix_);
    }
    
    // TODO: You'll also need to draw the earthquakes.  It's up to you exactly
    // how you wish to do that.
}

//draw the earthquake on plane
void QuakeApp::plane_earthquake() {
    Matrix4 modelMatrix;
    int start = quake_db_.min_index();
    Date d(current_time_);
    Color c;
    int index = quake_db_.FindMostRecentQuake(d);
    
    if(d.SecondsUntil(quake_db_.earthquake(quake_db_.min_index()).date()) <= PLAYBACK_WINDOW) {
        for(int i = start; i <= index; i++) {
            Point3 loc = earth_.LatLongToPlane(quake_db_.earthquake(i).latitude(), quake_db_.earthquake(i).longitude());
            double ratio =  (quake_db_.earthquake(i).magnitude()-quake_db_.min_magnitude())/(quake_db_.max_magnitude()-quake_db_.min_magnitude());
            if(ratio >= 0.7) {
                c =Color(0,0,0);
            }
            else if(ratio >= 0.4) {
                c =Color(1,0,0);
            }
            else{
                c = Color(1,1,0);
            }
            double seconds = d.SecondsUntil(quake_db_.earthquake(i).date());
            Matrix4 T = Matrix4::Translation(loc - Point3(0,0,0));
            Matrix4 S = Matrix4::Scale(Vector3(0.15f, 0.15f, 0.15f)*ratio-Vector3(0.15f, 0.15f, 0.15f)*ratio*seconds/PLAYBACK_WINDOW);
            Matrix4 event = T * S * modelMatrix;
            quick_shapes_.DrawSphere(event, view_matrix_, proj_matrix_, c);
        }
    }
    else {
        Date temp(d.ToSeconds()-PLAYBACK_WINDOW);
        int s = quake_db_.FindMostRecentQuake(temp);
        for(int i = s; i <= index; i++) {
           Point3 loc = earth_.LatLongToPlane(quake_db_.earthquake(i).latitude(), quake_db_.earthquake(i).longitude());
           double ratio =  (quake_db_.earthquake(i).magnitude()-quake_db_.min_magnitude())/(quake_db_.max_magnitude()-quake_db_.min_magnitude());
            if(ratio >= 0.7) {
                c =Color(0,0,0);  //group of largest earthquake of color black
            }
            else if(ratio >= 0.4) {
                c =Color(1,0,0);   //group of second largest earthquake of color red
            }
            else{
                c = Color(1,1,0);     //group of third largest earthquake of color yellow
            }
           double seconds = d.SecondsUntil(quake_db_.earthquake(i).date());
           Matrix4 T = Matrix4::Translation(loc - Point3(0,0,0));
           Matrix4 S = Matrix4::Scale(Vector3(0.15f, 0.15f, 0.15f)*ratio-Vector3(0.15f, 0.15f, 0.15f)*ratio*seconds/PLAYBACK_WINDOW);
           Matrix4 event = T * S * modelMatrix;
           quick_shapes_.DrawSphere(event, view_matrix_, proj_matrix_, c);
      }
    }
}

//draw the earthquake on sphere
void QuakeApp::sphere_earthquake(Matrix4 R) {
    Matrix4 model_matrix;
    int start = quake_db_.min_index();
    Date d(current_time_);
    Color c;
    int index = quake_db_.FindMostRecentQuake(d);
    
    if(d.SecondsUntil(quake_db_.earthquake(quake_db_.min_index()).date()) <= PLAYBACK_WINDOW) {
        for(int i = start; i <= index; i++) {
            Point3 loc = earth_.LatLongToSphere(quake_db_.earthquake(i).latitude(), quake_db_.earthquake(i).longitude());
            double ratio =  (quake_db_.earthquake(i).magnitude()-quake_db_.min_magnitude())/(quake_db_.max_magnitude()-quake_db_.min_magnitude());
            if(ratio >= 0.7) {   //group of largest earthquake of color black
                c =Color(0,0,0);
            }
            else if(ratio >= 0.4) {
                c =Color(1,0,0);
            }
            else{
                c = Color(1,1,0);
            }
            double seconds = d.SecondsUntil(quake_db_.earthquake(i).date());
            Matrix4 T = Matrix4::Translation(loc - Point3(0,0,0));
            Matrix4 S = Matrix4::Scale(Vector3(0.1f, 0.1f, 0.1f)*ratio-Vector3(0.1f, 0.1f, 0.1f)*ratio*seconds/PLAYBACK_WINDOW);
            Matrix4 event = R * T * S * model_matrix;
            quick_shapes_.DrawSphere(event, view_matrix_, proj_matrix_, c);
        }
    }
    else {
        Date temp(d.ToSeconds()-PLAYBACK_WINDOW);
        int s = quake_db_.FindMostRecentQuake(temp);
        for(int i = s; i <= index; i++) {
            Point3 loc = earth_.LatLongToSphere(quake_db_.earthquake(i).latitude(), quake_db_.earthquake(i).longitude());
           double ratio =  (quake_db_.earthquake(i).magnitude()-quake_db_.min_magnitude())/(quake_db_.max_magnitude()-quake_db_.min_magnitude());
            if(ratio >= 0.7) {
                c =Color(0,0,0);
            }
            else if(ratio >= 0.4) {
                c =Color(1,0,0);
            }
            else{
                c = Color(1,1,0);
            }
           double seconds = d.SecondsUntil(quake_db_.earthquake(i).date());
           Matrix4 T = Matrix4::Translation(loc - Point3(0,0,0));
           Matrix4 S = Matrix4::Scale(Vector3(0.1f, 0.1f, 0.1f)*ratio-Vector3(0.1f, 0.1f, 0.1f)*ratio*seconds/PLAYBACK_WINDOW);
            Matrix4 event =  R * T * S * model_matrix;
           quick_shapes_.DrawSphere(event, view_matrix_, proj_matrix_, c);
      }
    }
}

//helper funciton for make sure x is in range
float QuakeApp::Clamp(float x, float a, float b) {
    if(x <= a)
        return a;
    else if(x >= b)
        return b;
    else
        return x;
}






