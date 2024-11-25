import Serial from "./serial.ts";

Serial.OnPacket = () => {
    console.log("paquete");
};
