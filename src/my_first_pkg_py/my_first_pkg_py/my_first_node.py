#!/usr/bin/env python3
import rclpy  #Ros2 pythonn client library - access to ros2 funcs like nodes, topics etc
from rclpy.node import Node #from node module inside rclpy pkg import Node class

class MyNode(Node):
    def __init__(self):
        super().__init__("py_test")
        self.get_logger().info("Hello World")


def main(args=None):  #defining main func with optional arg. It can be called as main() or main("arg_name")
    rclpy.init(args=args)
    node = MyNode()
    rclpy.spin(node)
    rclpy.shutdown() 

if __name__ == "__main__":
    main()