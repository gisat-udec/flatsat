import { createSignal } from "solid-js";
import Serial from "./serial.ts";

interface Client {
    camera_ok: number;
    imu_ok: number;
    ambient_ok: number;
    imu_x: number;
    imu_y: number;
    imu_z: number;
    ambient_temp: number;
    ambient_pressure: number;
    jpeg: string;
}

let clients: Record<number, Client> = {};
export const [clients_signal, setClients_signal] = createSignal(clients);

let jpeg = {
    frame: 0,
    chunks: 0,
    chunk: [] as Uint8Array[],
};

Serial.OnPacket = (data: DataView) => {
    let id = data.getUint32(4, true);
    clients[id] = {
        camera_ok: data.getUint8(8),
        imu_ok: data.getUint8(9),
        ambient_ok: data.getUint8(10),
        imu_x: data.getFloat32(11, true),
        imu_y: data.getFloat32(15, true),
        imu_z: data.getFloat32(19, true),
        ambient_temp: data.getFloat32(23, true),
        ambient_pressure: data.getFloat32(27, true),
        jpeg: "",
    };

    let has_jpeg = data.getUint8(31);
    if (has_jpeg == 1) {
        let frame = data.getUint32(32, true);
        let chunk = data.getUint8(36);
        let chunks = data.getUint8(37);
        if (jpeg.frame != frame) {
            jpeg.frame = frame;
            jpeg.chunks = chunks;
            jpeg.chunk = [];
        }
        jpeg.chunk[chunk] = new Uint8Array(
            data.buffer,
            38,
            data.byteLength - 38,
        );

        if (jpeg.chunk.length == chunks) {
            clients[id].jpeg = window.URL.createObjectURL(
                new Blob([...jpeg.chunk]),
            );
        }
    }

    setClients_signal({ ...clients });
};
