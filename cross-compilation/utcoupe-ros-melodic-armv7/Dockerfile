FROM arm32v7/ros:melodic-ros-base

RUN mkdir -p /utcoupe/coupe21/ros_ws/src
ENV UTCOUPE_WORKSPACE /utcoupe/coupe21
ENV ROS_LANG_DISABLE=genlisp:geneus

RUN apt-get update
RUN apt-get install wget libsfml-dev libarmadillo-dev sudo qt5-default -qq -y
RUN apt-get install ros-melodic-tf2 ros-melodic-tf2-ros ros-melodic-rviz ros-melodic-diagnostic-updater ros-melodic-roslint ros-melodic-camera-info-manager ros-melodic-rosserial-arduino ros-melodic-rosbridge-suite ros-melodic-tf2-web-republisher ros-melodic-serial ros-melodic-dynamixel-sdk ros-melodic-rosserial-python ros-melodic-tf2-geometry-msgs ros-melodic-urg-c ros-melodic-urg-node -y -qq
RUN rm -rf /var/lib/apt/lists/*

RUN mkdir -p /temp/utcoupe_install
WORKDIR /temp/utcoupe_install
RUN wget https://github.com/utcoupe/coupe21/raw/master/scripts/install_external_nodes.sh && chmod +x install_external_nodes.sh
RUN bash -c "source /opt/ros/melodic/setup.sh && ./install_external_nodes.sh"
RUN rm -rf /tmp/utcoupe*

WORKDIR $UTCOUPE_WORKSPACE/ros_ws
# Tests if the setup works
RUN bash -c "source /opt/ros/melodic/setup.sh && catkin_make"
RUN rm -rf build devel


WORKDIR $UTCOUPE_WORKSPACE/ros_ws
CMD bash -c "echo 'Using '$UTCOUPE_WORKSPACE' as ROS workspace."
