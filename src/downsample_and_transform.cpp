#include <ros/ros.h>
#include <sensor_msgs/PointCloud2.h>
#include <sensor_msgs/point_cloud_conversion.h>
#include <pcl_ros/point_cloud.h>
#include <pcl_ros/transforms.h>
#include <pcl/point_types.h>
#include <pcl/filters/extract_indices.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/kdtree/kdtree.h>
#include <pcl/segmentation/extract_clusters.h>
#include <pcl_conversions/pcl_conversions.h>
#include <tf/transform_listener.h>



class PointCloudTransform{
	private:
		ros::NodeHandle nh;
		ros::NodeHandle nhPrivate;
		tf::TransformListener tflistener;
		ros::Subscriber sub;
		ros::Publisher pub;
		sensor_msgs::PointCloud pc_;

	public:
		PointCloudTransform();
		void Callback(const sensor_msgs::PointCloud2ConstPtr& msg);
		void DownsamplingBoxel(	pcl::PointCloud<pcl::PointXYZRGB>::Ptr pc,
								pcl::PointCloud<pcl::PointXYZRGB>::Ptr pc_);
		void DownsamplingRandom(pcl::PointCloud<pcl::PointXYZRGB>::Ptr pc,
								pcl::PointCloud<pcl::PointXYZRGB>::Ptr pc_, 
								int denominator);
};

PointCloudTransform::PointCloudTransform()
    : nhPrivate("~")
{
	sub = nh.subscribe("/cloud", 1, &PointCloudTransform::Callback, this);
	pub = nh.advertise<sensor_msgs::PointCloud2>("/cloud/downsampled", 1);
}

void PointCloudTransform::Callback(const sensor_msgs::PointCloud2ConstPtr &msg)
{
	std::cout << "downsamoling" << std::endl;
	sensor_msgs::PointCloud2 pc2_trans;
	sensor_msgs::PointCloud2 pc2_out;
	pcl::PointCloud<pcl::PointXYZRGB>::Ptr pcl_cloud (new pcl::PointCloud<pcl::PointXYZRGB>());
	pcl::PointCloud<pcl::PointXYZRGB>::Ptr ds_cloud {new pcl::PointCloud<pcl::PointXYZRGB>()};
    ros::Time time = ros::Time(0);
	try{
		
        tflistener.waitForTransform("/base_link", msg->header.frame_id, time, ros::Duration(4.0));
		pcl_ros::transformPointCloud("/base_link", *msg, pc2_trans, tflistener);
		pcl::fromROSMsg(pc2_trans, *pcl_cloud);

		DownsamplingBoxel(pcl_cloud,ds_cloud);
		// DownsamplingRandom(pcl_cloud,ds_cloud,10);

    	pcl::toROSMsg(*ds_cloud, pc2_out);
		pc2_out.header.frame_id = "/base_link";
		pc2_out.header.stamp = msg->header.stamp;
		pub.publish(pc2_out);
	}
	catch(tf::TransformException ex){
		ROS_ERROR("%s",ex.what());
	}
}


void PointCloudTransform::DownsamplingBoxel(pcl::PointCloud<pcl::PointXYZRGB>::Ptr pc,
											pcl::PointCloud<pcl::PointXYZRGB>::Ptr pc_)
{
	
	pcl::VoxelGrid<pcl::PointXYZRGB> vg;
   	vg.setInputCloud (pc);  
   	vg.setLeafSize (0.02f, 0.02f, 0.02f);
    vg.filter (*pc_);
	
}
void PointCloudTransform::DownsamplingRandom(	pcl::PointCloud<pcl::PointXYZRGB>::Ptr pc,
												pcl::PointCloud<pcl::PointXYZRGB>::Ptr pc_,
												int denominator)
{
	int j=0;
	for(auto i=pc->points.begin();i!=pc->points.end();i++){
		if(!(j%denominator)){
			pc_->points.push_back(*i);
		}
		j++;
	}
}

int main(int argc, char** argv)
{
    ros::init(argc, argv, "downsample_vox_and_transform");
	PointCloudTransform transform;
	ros::spin();
	
	return 0;
}
