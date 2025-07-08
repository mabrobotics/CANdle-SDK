# MD80 ROS2 Node

This node handles the communication between MAB's MD80 drives in ROS2 environment. The node was designed to act as 
operational endpoint - to control the drives and get information from them, thus it is not capable of configuring the drives, 
for this use `candletool`.

## Principles of operation

The node normally communicates via services for setup, and via topics for regular data transfers.
Services are: 
- /add_md80s
- /zero_md80s
- /set_mode_md80s
- /enable_md80s
- /disable_md80s

Topics subscribed by the node are:
- /md80/motion_command
- /md80/impedance_command
- /md80/velocity_pid_command
- /md80/position_pid_command

Topic published by the node is:
- /md80/joint_states

## Quick startup guide

Please find a detailed startup guide in the [MD80 x CANdle manual](https://www.mabrobotics.pl/servos)