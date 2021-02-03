inline uint8_t crc8(CircularBuffer<uint8_t, 35> &data) {
  unsigned long crc;
  int i, bit;
  uint8_t length = data.size();

  crc = 0x02;
  for ( i = 0 ; i < length ; i++ ) {
    crc ^= data[i];
    for ( bit = 0 ; bit < 8 ; bit++ ) {
      if ( (crc & 0x80) != 0 ) {
        crc <<= 1;
        crc ^= 0x7;
      }
      else {
        crc <<= 1;
      }
    }
  }

  return crc ^ 0x02;
}

inline void ID_request() {
  Q_out.push(0xFE);
  Q_out.push(0xBF);
  Q_out.push(0x01);
  Q_out.push(0x02);
  Q_out.push(0xF1);
  Q_out.push(0x73);

  rs485_send();
}

inline void ID_ack() {
  Q_out.push(id);
  Q_out.push(0xBF);
  Q_out.push(0x03);

  rs485_send();
}

void rs485_send() {
  // The following is not required for the new RS485 chip
  if (AUTO_TX) {

  } else {
    digitalWrite(TX485, HIGH);
    delay(1);
  }


  // Add telegram length
  Q_out.unshift(Q_out.size() + 2);

  // Add CRC
  Q_out.push(crc8(Q_out));

  // Wrap telegram in SOF/EOF
  Q_out.unshift(0x7E);
  Q_out.push(0x7E);

  for (i = 0; i < Q_out.size(); i++)
    Serial.write(Q_out[i]);

  //print_msg(Q_out);

  Serial.flush();

  if (AUTO_TX) {

  } else {
    digitalWrite(TX485, LOW);
  }

  // DEBUG: print_msg(Q_out);
  Q_out.clear();
}
