/* \author Aaron Brown */
/* additional implementation by Christopher Anton*/
// Create simple 3d highway enviroment using PCL
// for exploring self-driving car sensors

#include "sensors/lidar.h"
#include "render/render.h"
#include "processPointClouds.h"
// using templates for processPointClouds so also include .cpp to help linker
#include "processPointClouds.cpp"
//#include "opencv2/opencv.hpp"
//#include <opencv2/opencv.hpp>

std::vector<Car> initHighway(bool renderScene, pcl::visualization::PCLVisualizer::Ptr &viewer) 
{

    Car egoCar(Vect3(0, 0, 0), Vect3(4, 2, 2), Color(0, 1, 0), "egoCar");
    Car car1(Vect3(15, 0, 0), Vect3(4, 2, 2), Color(0, 0, 1), "car1");
    Car car2(Vect3(8, -4, 0), Vect3(4, 2, 2), Color(0, 0, 1), "car2");
    Car car3(Vect3(-12, 4, 0), Vect3(4, 2, 2), Color(0, 0, 1), "car3");

    std::vector<Car> cars;
    cars.push_back(egoCar);
    cars.push_back(car1);
    cars.push_back(car2);
    cars.push_back(car3);

    if (renderScene) 
    {
        renderHighway(viewer);
        egoCar.render(viewer);
        car1.render(viewer);
        car2.render(viewer);
        car3.render(viewer);
    }

    return cars;
}


void cityBlock(pcl::visualization::PCLVisualizer::Ptr& viewer, ProcessPointClouds<pcl::PointXYZI>* pointProcessorI, const pcl::PointCloud<pcl::PointXYZI>::Ptr& inputCloud)
{
    // ----------------------------------------------------
    // -----Open 3D viewer and display City Block     -----
    // ----------------------------------------------------
    //Input clud sent for filtering
    pcl::PointCloud<pcl::PointXYZI>::Ptr filterCloud = pointProcessorI->FilterCloud(inputCloud, 0.15 , Eigen::Vector4f (-20, -6, -2, 1), Eigen::Vector4f ( 20, 7, 2, 1));

    //filtered cloud sent for segmenting obstacle and plane clouds
    std::pair<pcl::PointCloud<pcl::PointXYZI>::Ptr, pcl::PointCloud<pcl::PointXYZI>::Ptr> segmentCloud = pointProcessorI->SegmentPlane(filterCloud, 25, 0.3);

    //Obstacle cloud sent for clustring obstacles
    std::vector<pcl::PointCloud<pcl::PointXYZI>::Ptr> cloudClusters = pointProcessorI->Clustering(segmentCloud.first, .44, 3, 1000);

    int clusterId = 0;
    std::vector<Color> colors = {Color(1,0,0), Color(1,1,0), Color(0,0,1), Color(1,1,0), Color(1,0,1), Color(1,1,1)};

    
    for(pcl::PointCloud<pcl::PointXYZI>::Ptr cluster : cloudClusters)
    {
        std::cout << "cluster size ";
        pointProcessorI->numPoints(cluster);
        //render the obstacle clusters
        renderPointCloud(viewer,cluster,"obstCloud"+std::to_string(clusterId),colors[clusterId%colors.size()]);

        Box box = pointProcessorI->BoundingBox(cluster);

        //render a box around each obstacle
        renderBox(viewer,box,clusterId);
        ++clusterId;
    }
    //render road(plane cloud)
    renderPointCloud(viewer, segmentCloud.second, "planeCloud", Color(0, 1, 0));
}


    
void initCamera(CameraAngle setAngle, pcl::visualization::PCLVisualizer::Ptr &viewer) 
{

    viewer->setBackgroundColor(0, 0, 0);

    // set camera position and angle
    viewer->initCameraParameters();
    // distance away in meters
    int distance = 16;

    switch (setAngle) {
        case XY :
            viewer->setCameraPosition(-distance, -distance, distance, 1, 1, 0);
            break;
        case TopDown :
            viewer->setCameraPosition(0, 0, distance, 1, 0, 1);
            break;
        case Side :
            viewer->setCameraPosition(0, -distance, 0, 0, 0, 1);
            break;
        case FPS :
            viewer->setCameraPosition(-15, 0, 0, 0, 0, 1);
    }

    if (setAngle != FPS)
        viewer->addCoordinateSystem(1.0);
}


int main(int argc, char **argv) 
{
    
    /*std::cout << "starting enviroment" << std::endl;

    cout << "OpenCV version : " << CV_VERSION << endl;

    cout << "Major version : " << CV_MAJOR_VERSION << endl;

    cout << "Minor version : " << CV_MINOR_VERSION << endl;

    cout << "Subminor version : " << CV_SUBMINOR_VERSION << endl;*/

    pcl::visualization::PCLVisualizer::Ptr viewer(new pcl::visualization::PCLVisualizer("3D Viewer"));
    CameraAngle setAngle = XY;
    initCamera(setAngle, viewer);

    //Created a  point cloud processor
    ProcessPointClouds<pcl::PointXYZI> pointProcessorI;
    pcl::PointCloud<pcl::PointXYZI>::Ptr inputCloudI;

    std::vector<boost::filesystem::path> stream = pointProcessorI.streamPcd("../src/sensors/data/pcd/data_1");

    auto streamIterator = stream.begin();
        
    while (!viewer->wasStopped()) 
    {

        // Clear Viewer
        viewer->removeAllPointClouds();
        viewer->removeAllShapes();

        // Load PCD and run obstacle detection process
        inputCloudI = pointProcessorI.loadPcd((*streamIterator).string());
        cityBlock(viewer, &pointProcessorI, inputCloudI);

        streamIterator++;
        if(streamIterator==stream.end())
            streamIterator = stream.begin();
    
        viewer->spinOnce();

    }
}