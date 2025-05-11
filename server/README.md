Code for the central server

Python Protobuf files are to be generated into a subfolder [protobuf](protobuf)
```bash
SwarmBots$ mkdir -p server/protobuf
SwarmBots$ protoc --proto_path=proto/ --python_out=server/protobuf/ proto/*.proto
```

**Important!** the file [wrapper_pb2](protobuf/wrapper_pb2.py) imports have to be updated to inform of the subfolder location
```python
import protobuf.move_cmd_pb2 as move__cmd__pb2
import protobuf.led_cmd_pb2 as led__cmd__pb2
import protobuf.lidar_data_pb2 as lidar__data__pb2
import protobuf.encoder_data_pb2 as encoder__data__pb2
import protobuf.imu_data_pb2 as imu__data__pb2
```

Python packages:
- websockets
- protobuf
- pygame