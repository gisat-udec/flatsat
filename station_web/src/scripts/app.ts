import Serial from "./serial.ts";

Serial.OnPacket = (data: DataView) => {
    console.log(data.getUint32(9, true), data.getUint8(13), data.getUint8(14));
};
