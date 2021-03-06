#include <iostream>
#include <pcl/point_types.h>
#include <pcl/point_cloud.h>
#include <pcl/io/pcd_io.h>
#include <pcl/filters/passthrough.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/filters/statistical_outlier_removal.h>
#include <pcl/search/kdtree.h>
//#include <pcl/kdtree/kdtree_flann.h>
#include <thrust/execution_policy.h>

#include "mb_cuda/common/point_types.h"
#include "mb_cuda/io/pcl_thrust.h"
#include "mb_cuda/io/host_device.h"
#include "mb_cuda/filters/pass_through.h"
#include "mb_cuda/filters/voxel_grid.h"
#include "mb_cuda/filters/statistical_outlier_removal.h"

#include <boost/timer.hpp>
#include <boost/shared_ptr.hpp>
#include <pcl/visualization/cloud_viewer.h>
#include "grabber/kinect2grabber.h"

typedef pcl::PointXYZRGB pointT;
typedef pcl::PointCloud<pointT> pointCloudT;
typedef pointCloudT::Ptr pointCloudPtr;

void point_setvalue_test();
void pass_through_test();
void getMinMax_test();
void voxel_grid_test();
void statistical_outliers_removal_test();

int main()
{
//  point_setvalue_test();
//  pass_through_test();
//  getMinMax_test();
//  voxel_grid_test();
//  mb_cuda::flann_search();
  statistical_outliers_removal_test();
}

void point_setvalue_test()
{
  std::cout<<std::endl<<"[test]: point_setvalue"<<std::endl;
  mb_cuda::PointXYZRGB p1;
  p1.x=3;
  p1.y=5;
  p1.z=7.9;

  mb_cuda::PointXYZRGB p2=p1;

  std::cout<<"p2.x= "<<p2.x<<" p2.y= "<<p2.y<<" p2.z= "<<p2.z<<std::endl;
}

void pass_through_test()
{
  std::cout<<std::endl<<"[test]: pass_through"<<std::endl;
  std::cout<<"========================================="<<std::endl;

  //prepare the data
  pointCloudPtr cloud(new pointCloudT);
  std::string fname="//home//wangy//dev//3dvision_ws//projects//build//milkbox_cuda//scene_1.pcd";
  pcl::io::loadPCDFile(fname,*cloud);

  //convert the pcl pointcloud to host_vector type
  thrust::host_vector<mb_cuda::PointXYZRGB> host_cloud;
  thrust::device_vector<mb_cuda::PointXYZRGB> device_cloud;
  mb_cuda::pcl_to_thrust(cloud,host_cloud);
  mb_cuda::host_to_device(host_cloud,device_cloud);

  //filter by passthrough
  thrust::device_vector<mb_cuda::PointXYZRGB> d_filtered_cloud;
  boost::timer _timer;
  mb_cuda::pass_through_filter(device_cloud,d_filtered_cloud,'z',0,0.8);
  std::cout<<"thrust passthrough elapsed time: "<<_timer.elapsed()*1000<<" ms"<<std::endl;
  std::cout<<"after filtered, the cloud size is: "<<d_filtered_cloud.size()<<std::endl;

  std::cout<<"pass_through test passed!"<<std::endl;
}

void getMinMax_test()
{
  std::cout<<std::endl<<"[test]: getMinMax"<<std::endl;
  std::cout<<"========================================="<<std::endl;

  //prepare the data
  pointCloudPtr cloud(new pointCloudT);
  std::string fname="//home//wangy//dev//3dvision_ws//projects//build//milkbox_cuda//scene_2.pcd";
  pcl::io::loadPCDFile(fname,*cloud);

  //convert the pcl pointcloud to host_vector type
  thrust::host_vector<mb_cuda::PointXYZRGB> host_cloud;
  thrust::device_vector<mb_cuda::PointXYZRGB> device_cloud;
  mb_cuda::pcl_to_thrust(cloud,host_cloud);
  mb_cuda::host_to_device(host_cloud,device_cloud);
  mb_cuda::pointT min_p,max_p;
  boost::timer _timer;
  mb_cuda::getMinMax3D(device_cloud,min_p,max_p);
  std::cout<<"mb_cuda getMinMax3D elapsed time: "<<_timer.elapsed()*1000<<" ms"<<std::endl;
  std::cout<<"mb_cuda getMinMax3D results:"<<std::endl;
  std::cout<<"min_Pt: "<<min_p.x<<" "<<min_p.y<<" "<<min_p.z<<std::endl;
  std::cout<<"max_pt: "<<max_p.x<<" "<<max_p.y<<" "<<max_p.z<<std::endl;

  Eigen::Vector4f minPt,maxPt;
  pcl::PCLPointCloud2 msg;
  pcl::toPCLPointCloud2<pointT>(*cloud,msg);
  typedef boost::shared_ptr< ::pcl::PCLPointCloud2 const> PCLPointCloud2ConstPtr;
  PCLPointCloud2ConstPtr ptr(new pcl::PCLPointCloud2(msg));
  _timer.restart();
  pcl::getMinMax3D(ptr,0,0,0,minPt,maxPt);
  std::cout<<"pcl getMinMax3D elapsed time: "<<_timer.elapsed()*1000<<" ms"<<std::endl;
  std::cout<<"pcl getMinMax3D results:"<<std::endl;
  std::cout<<"min_Pt: "<<minPt[0]<<" "<<minPt[1]<<" "<<minPt[2]<<std::endl;
  std::cout<<"max_pt: "<<maxPt[0]<<" "<<maxPt[1]<<" "<<maxPt[2]<<std::endl;
}

void voxel_grid_test()
{
  std::cout<<std::endl<<"[test]: voxel_grid"<<std::endl;
  std::cout<<"========================================="<<std::endl;

  //prepare the data
  pointCloudPtr cloud(new pointCloudT);
  std::string fname="//home//wangy//dev//3dvision_ws//projects//build//milkbox_cuda//scene_2.pcd";
  pcl::io::loadPCDFile(fname,*cloud);

  //convert the pcl pointcloud to host_vector type
  thrust::host_vector<mb_cuda::PointXYZRGB> host_cloud;
  thrust::device_vector<mb_cuda::PointXYZRGB> device_cloud;
  mb_cuda::pcl_to_thrust(cloud,host_cloud);
  mb_cuda::host_to_device(host_cloud,device_cloud);

  std::cout<<"cloud size is: "<<cloud->points.size()<<std::endl;

  //fitler by voxel grid
  float leafSize=0.005;
  thrust::device_vector<mb_cuda::PointXYZRGB> d_filtered;
  thrust::device_vector<mb_cuda::PointXYZRGB> d_nonan_cloud;
  boost::timer _timer;
  mb_cuda::removeNansOrIfs(device_cloud,d_nonan_cloud);
  std::cout<<"mb_cuda removeNans elapsed time: "<<_timer.elapsed()*1000<<" ms"<<std::endl;
  _timer.restart();
  mb_cuda::voxel_grid_filter(d_nonan_cloud,d_filtered,leafSize);
  std::cout<<"mb_cuda voxel_grid elapsed time: "<<_timer.elapsed()*1000<<" ms"<<std::endl;
  std::cout<<"after filtered, the cloud size is: "<<d_filtered.size()<<std::endl;
  pointCloudPtr cuda_filtered_cloud(new pointCloudT);
  thrust::host_vector<mb_cuda::PointXYZRGB> h_tmp;
  mb_cuda::device_to_host(d_filtered,h_tmp);
  mb_cuda::thrust_to_pcl(h_tmp,cuda_filtered_cloud);

  //using pcl::voxelgrid to downsample
  pcl::VoxelGrid<pointT> grid;
  pointCloudPtr h_filtered(new pointCloudT);
  grid.setInputCloud(cloud);
  grid.setLeafSize(leafSize,leafSize,leafSize);
  _timer.restart();
  grid.filter(*h_filtered);
  std::cout<<"pcl voxel_grid elapsed time: "<<_timer.elapsed()*1000<<" ms"<<std::endl;
  std::cout<<"after filtered, the cloud size is: "<<h_filtered->points.size()<<std::endl;

  std::cout<<"voxel_grid test passed!"<<std::endl;

  //view
  pcl::visualization::PCLVisualizer viewer("point cloud from kinect2");
  viewer.addPointCloud(cuda_filtered_cloud,"scene_cloud");
  pcl::visualization::PCLVisualizer viewer2("point cloud2 from kinect2");
  viewer2.addPointCloud(h_filtered,"scene_cloud2");
  while(!viewer.wasStopped()){
      viewer.spinOnce();
      viewer2.spinOnce();
    }
}

void statistical_outliers_removal_test()
{
  std::cout<<std::endl<<"[test]: statistical_outliers_removal"<<std::endl;
  std::cout<<"========================================="<<std::endl;

  //prepare the data
  pointCloudPtr cloud(new pointCloudT);
  std::string fname="//home//wangy//dev//3dvision_ws//projects//build//milkbox_cuda//scene_1.pcd";
  pcl::io::loadPCDFile(fname,*cloud);

  //convert the pcl pointcloud to host_vector type
  thrust::host_vector<mb_cuda::PointXYZRGB> host_cloud;
  thrust::device_vector<mb_cuda::PointXYZRGB> device_cloud;
  mb_cuda::pcl_to_thrust(cloud,host_cloud);
  mb_cuda::host_to_device(host_cloud,device_cloud);

  std::cout<<"cloud size is: "<<cloud->points.size()<<std::endl;

  //fitler by voxel grid
//  float leafSize=0.005;
//  thrust::device_vector<mb_cuda::PointXYZRGB> d_filtered;
//  thrust::device_vector<mb_cuda::PointXYZRGB> d_nonan_cloud;
//  boost::timer _timer;
//  mb_cuda::removeNansOrIfs(device_cloud,d_nonan_cloud);
//  std::cout<<"mb_cuda removeNans elapsed time: "<<_timer.elapsed()*1000<<" ms"<<std::endl;
//  _timer.restart();
//  mb_cuda::voxel_grid_filter(d_nonan_cloud,d_filtered,leafSize);
//  std::cout<<"mb_cuda voxel_grid elapsed time: "<<_timer.elapsed()*1000<<" ms"<<std::endl;
//  std::cout<<"after filtered, the cloud size is: "<<d_filtered.size()<<std::endl;
//  pointCloudPtr cuda_filtered_cloud(new pointCloudT);
//  thrust::host_vector<mb_cuda::PointXYZRGB> h_tmp;
//  mb_cuda::device_to_host(d_filtered,h_tmp);
//  mb_cuda::thrust_to_pcl(h_tmp,cuda_filtered_cloud);

//  //using pcl::voxelgrid to downsample
//  pcl::VoxelGrid<pointT> grid;
//  pointCloudPtr h_filtered(new pointCloudT);
//  grid.setInputCloud(cloud);
//  grid.setLeafSize(leafSize,leafSize,leafSize);
//  _timer.restart();
//  grid.filter(*h_filtered);
//  std::cout<<"pcl voxel_grid elapsed time: "<<_timer.elapsed()*1000<<" ms"<<std::endl;
//  std::cout<<"after filtered, the cloud size is: "<<h_filtered->points.size()<<std::endl;

  //statistical outliers removal
//  boost::timer _timer;
//  int knn=50;
//  int inliers_num=0;
//  thrust::device_vector<int> inliers;
//  mb_cuda::statistical_outlier_removal(device_cloud,knn,1.0,inliers, inliers_num);
//  std::cout<<"statistical outliers removal elapsed time: "<<_timer.elapsed()*1000<<" ms"<<std::endl;
//  std::cout<<"num of inliers: "<<inliers_num<<std::endl;

  //kdtree from pcl + cuda find inliers
  int mean_k=50;
  int inliers_num=0;
  int pt_num=cloud->points.size();
  thrust::device_vector<int> inliers;

  pcl::KdTreeFLANN<pcl::PointXYZRGB> kdtree;
  kdtree.setInputCloud (cloud);
  thrust::device_ptr<float> dists_ptr=thrust::device_malloc<float>(mean_k*pt_num);
//  std::vector<std::vector<int> > indices_vec(pt_num);
  std::vector<std::vector<float> > dists_vec(pt_num);

  boost::timer _timer1;
  for(int i=0;i<pt_num;++i){
      std::vector<int> indices_vec(mean_k);
      dists_vec[i].resize(mean_k);
      pointT pt=cloud->points[i];
      kdtree.nearestKSearch(pt,mean_k,indices_vec,dists_vec[i]);
      for(int j=0;j<mean_k;++j){
          dists_ptr[i*mean_k+j]=dists_vec[i][j];
        }
    }

  mb_cuda::statistical_outlier_removal(dists_ptr,pt_num,mean_k,2.0,inliers,inliers_num);
  thrust::free(thrust::device, dists_ptr);
  std::cout<<"statistical outliers removal elapsed time: "<<_timer1.elapsed()*1000<<" ms"<<std::endl;
  std::cout<<"num of inliers: "<<inliers_num<<std::endl;



  //using pcl outlier removal
  boost::timer _timer;
  pcl::StatisticalOutlierRemoval<pointT> outlierRemv;
  outlierRemv.setInputCloud(cloud);
  outlierRemv.setMeanK(50);
  outlierRemv.setStddevMulThresh(3.0);
  outlierRemv.filter(*cloud);
  std::cout<<"pcl statistical outliers removal elapsed time: "<<_timer.elapsed()*1000<<" ms"<<std::endl;
  std::cout<<"num of inliers: "<<cloud->points.size()<<std::endl;

}
