#include "LD20Sensor.h"
#include "actors/Actors.h"
#include "Robota.h"


uint16_t LD20Sensor::getType() {
  return SENSOR_LD20_LIDAR;
}

LD20Sensor::LD20Sensor(HardwareSerial *serial) {
  this->serial = serial;
}

void LD20Sensor::init() {
  if (!*serial) {
    serial->begin(230400, SERIAL_8N1, 13, 12);
  } else {
    // Assume it has already been initialised -> do nothing
  }
}

void LD20Sensor::processPacket(uint8_t byte) {
  static const uint16_t pkg_count = sizeof(LiDARMeasureDataType);
  static const uint16_t pkghealth_count = sizeof(LiDARHealthInfoType);
  static const uint16_t pkgmanufac_count = sizeof(LiDARManufactureInfoType);

  switch (state) {
    case HEADER:
      if (byte == PKG_HEADER) {
        buffer[count++] = byte;
        state = VER_LEN;
      } else {
        //do nothing
      }
      break;
    case VER_LEN:
      buffer[count++] = byte;

      if (byte == DATA_PKG_INFO) {
        state = DATA;
      } else if (byte == HEALTH_PKG_INFO) {
        state = DATA_HEALTH;
      } else if (byte == MANUFACT_PKG_INF) {
        state = DATA_MANUFACTURER;
      } else {
        // Invalid packet, just ignore it for now
        // TODO maybe log errors, idk
        state = HEADER;
        count = 0;
      }
      break;
    case DATA:
      buffer[count++] = byte;
      if (count >= pkg_count) {
        memcpy(&pcdpkg_data_, buffer, pkg_count);
        uint8_t crc = calcCRC8(&pcdpkg_data_, pkg_count - 1);
        //TODO crc
        state = HEADER;
        count = 0;
        if (crc == pcdpkg_data_.crc8) {
          float step = (pcdpkg_data_.end_angle - pcdpkg_data_.start_angle) / (POINT_PER_PACK - 1.0);
          for (int i = 0; i < POINT_PER_PACK; i++) {
            uint16_t angle = (uint16_t) (pcdpkg_data_.start_angle + step * i * 100);
            SingleLiDARMeasurement measurement;
            measurement.angle = angle;
            measurement.distance = pcdpkg_data_.point[i].distance;
            measurement.intensity = pcdpkg_data_.point[i].intensity;
            callback(measurement);
          }
        } else {
          return;  //TODO error handling
        }
      } else {
        // do nothing
      }
      break;
    // Ignore those for now
    case DATA_HEALTH:
    case DATA_MANUFACTURER:
      state = HEADER;
      count = 0;
      break;
  }
}

uint8_t LD20Sensor::calcCRC8(const void *data, uint16_t data_len) {
  uint8_t crc = 0;
  while (data_len--) {
    crc = CrcTable[(crc ^ *((uint8_t*) data)) & 0xff];
    data++;
  }
  return crc;
}

void LD20Sensor::tick() {
  while (serial->available()) {
    uint8_t byte = serial->read();
    processPacket(byte);
  }
}

void LD20Sensor::setCallback(void (*callback)(SingleLiDARMeasurement measurement)) {
  this->callback = callback;
}

